/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file board.c
 *
 * @brief Deals with boarding ships.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "board.h"

#include "array.h"
#include "commodity.h"
#include "damagetype.h"
#include "hook.h"
#include "log.h"
#include "nstring.h"
#include "ndata.h"
#include "nlua.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "space.h"
#include "toolkit.h"

#define BOARDING_WIDTH  380 /**< Boarding window width. */
#define BOARDING_HEIGHT 200 /**< Boarding window height. */

#define BUTTON_WIDTH     50 /**< Boarding button width. */
#define BUTTON_HEIGHT    30 /**< Boarding button height. */

static int board_stopboard = 0; /**< Whether or not to unboard. */
static int board_boarded   = 0; /**< Whether or not the player is boarded. */
static nlua_env board_env  = LUA_NOREF; /**< Lua environment to do the boarding. */

/**
 * @brief Gets if the player is boarded.
 */
int player_isBoarded (void)
{
   return board_boarded;
}

int board_hook( void *data )
{
   HookParam hparam[2];
   Pilot *p = (Pilot*) data;

   /* Don't allow boarding own escorts. */
   if (pilot_isWithPlayer( p )) {
      p->stress       = 0.;
      p->dtimer_accum = 0;
      pilot_updateDisable(p, 0);
      player_message(_("#gYou reactivate the systems of %s."), p->name);
      return 0;
   }

   /*
    * run hook if needed
    */
   hparam[0].type = HOOK_PARAM_PILOT;
   hparam[0].u.lp = PLAYER_ID;
   hparam[1].type = HOOK_PARAM_SENTINEL;
   pilot_runHookParam(p, PILOT_HOOK_BOARD, hparam, 1);
   pilot_runHookParam(p, PILOT_HOOK_BOARD_ALL, hparam, 1);
   hparam[0].u.lp = p->id;
   hooks_runParam( "board", hparam );
   pilot_runHookParam(player.p, PILOT_HOOK_BOARDING, hparam, 1);

   /* Run outfit stuff. */
   pilot_outfitLOnboard( player.p, p );

   if (board_stopboard) {
      board_boarded = 0;
      return 0;
   }

   /* Set up environment first time. */
   if (board_env == LUA_NOREF) {
      board_env = nlua_newEnv();
      nlua_loadStandard( board_env );

      size_t bufsize;
      char *buf = ndata_read( BOARD_PATH, &bufsize );
      if (nlua_dobufenv(board_env, buf, bufsize, BOARD_PATH) != 0) {
         WARN( _("Error loading file: %s\n"
             "%s\n"
             "Most likely Lua file has improper syntax, please check"),
               BOARD_PATH, lua_tostring(naevL,-1));
         board_boarded = 0;
         free(buf);
         return -1;
      }
      free(buf);
   }

   /* Run Lua. */
   nlua_getenv(naevL, board_env,"board");
   lua_pushpilot(naevL, p->id);
   if (nlua_pcall(board_env, 1, 0)) { /* error has occurred */
      WARN( _("Board: '%s'"), lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
   board_boarded = 0;
   return 0;
}

/**
 * @brief Sees if the pilot can board a pilot.
 */
int player_canBoard( int noisy )
{
   Pilot *p;

   /* Not disabled. */
   if (pilot_isDisabled(player.p))
      return PLAYER_BOARD_IMPOSSIBLE;

   if (player.p->target==PLAYER_ID) {
      /* We don't try to find far away targets, only nearest and see if it matches.
       * However, perhaps looking for first boardable target within a certain range
       * could be more interesting. */
      pilot_getNearestPosPilot( player.p, &p, player.p->solid.pos.x, player.p->solid.pos.y, 1 );
      if ((p == NULL) ||
            (!pilot_isDisabled(p) && !pilot_isFlag(p,PILOT_BOARDABLE)) ||
            pilot_isFlag(p,PILOT_NOBOARD)) {
         if (noisy)
            player_message( "#r%s", _("You need a target to board first!") );
         return PLAYER_BOARD_IMPOSSIBLE;
      }
      player_targetSet( p->id );
   }
   else
      p = pilot_getTarget( player.p );

   /* More checks. */
   if (pilot_isFlag(p,PILOT_NOBOARD)) {
      if (noisy)
         player_message( "#r%s", _("Target ship can not be boarded.") );
      return PLAYER_BOARD_IMPOSSIBLE;
   }
   else if (!pilot_isDisabled(p) && !pilot_isFlag(p,PILOT_BOARDABLE)) {
      if (noisy)
         player_message( "#r%s", _("You cannot board a ship that isn't disabled!") );
      return PLAYER_BOARD_IMPOSSIBLE;
   }
   else if (pilot_isFlag(p,PILOT_BOARDED)) {
      if (noisy)
         player_message( "#r%s", _("Your target cannot be boarded again.") );
      return PLAYER_BOARD_IMPOSSIBLE;
   }

   return PLAYER_BOARD_OK;
}

/**
 * @brief Attempt to board the player's target.
 *
 * Creates the window on success.
 */
int player_tryBoard( int noisy )
{
   Pilot *p;
   char c;

   if (player_canBoard( noisy )==PLAYER_BOARD_IMPOSSIBLE)
      return PLAYER_BOARD_IMPOSSIBLE;

   /* Should have a pilot target by now or failed. */
   p = pilot_getTarget( player.p );
   c = pilot_getFactionColourChar( p );

   if (vec2_dist(&player.p->solid.pos,&p->solid.pos) >
         p->ship->gfx_space->sw * PILOT_SIZE_APPROX) {
      if (noisy)
         player_message( "#r%s", _("You are too far away to board your target.") );
      return PLAYER_BOARD_RETRY;
   }
   else if (vec2_dist2( &player.p->solid.vel, &p->solid.vel ) > pow2(MAX_HYPERSPACE_VEL)) {
      if (noisy)
         player_message( "#r%s", _("You are going too fast to board the ship.") );
      return PLAYER_BOARD_RETRY;
   }

   /* Handle fighters. */
   if (pilot_isFlag(p, PILOT_CARRIED) && (p->dockpilot == PLAYER_ID)) {
      if (pilot_dock( p, player.p )) {
         WARN(_("Unable to recover fighter."));
         return PLAYER_BOARD_IMPOSSIBLE;
      }
      player_message(_("#oYou recover your %s fighter."), p->name);
      player.p->ptarget = NULL; /* Have to clear target cache. */
      return PLAYER_BOARD_OK;
   }

   /* Set speed to target's speed. */
   vec2_cset(&player.p->solid.vel, VX(p->solid.vel), VY(p->solid.vel));

   /* Is boarded. */
   board_boarded = 1;

   /* Mark pilot as boarded only if it isn't being active boarded. */
   if (!pilot_isFlag(p,PILOT_BOARDABLE))
      pilot_setFlag(p,PILOT_BOARDED);
   player_message(_("#oBoarding ship #%c%s#0."), c, p->name);

   /* Don't unboard. */
   board_stopboard = 0;

   hook_addFunc( board_hook, p, "safe" );
   return PLAYER_BOARD_OK;
}

/**
 * @brief Forces unboarding of the pilot.
 */
void board_unboard (void)
{
   board_stopboard = 1;
}

/**
 * @brief Has a pilot attempt to board another pilot.
 *
 *    @param p Pilot doing the boarding.
 *    @return 1 if target was boarded.
 */
int pilot_board( Pilot *p )
{
   Pilot *target;
   HookParam hparam[2];

   /* Make sure target is valid. */
   target = pilot_getTarget( p );
   if (target == NULL) {
      DEBUG("NO TARGET");
      return 0;
   }

   /* Check if can board. */
   if (!pilot_isDisabled(target))
      return 0;
   else if (vec2_dist(&p->solid.pos, &target->solid.pos) >
         target->ship->gfx_space->sw * PILOT_SIZE_APPROX )
      return 0;
   else if (vec2_dist2( &p->solid.vel, &target->solid.vel ) > pow2(MAX_HYPERSPACE_VEL))
      return 0;
   else if (pilot_isFlag(target,PILOT_BOARDED))
      return 0;

   /* Set speed to target's speed. */
   vec2_cset(&p->solid.vel, VX(target->solid.vel), VY(target->solid.vel));

   /* Set the boarding flag. */
   pilot_setFlag(target, PILOT_BOARDED);
   pilot_setFlag(p, PILOT_BOARDING);

   /* Set time it takes to board. */
   p->ptimer = 3.;

   /* Run pilot board hook. */
   hparam[0].type = HOOK_PARAM_PILOT;
   hparam[0].u.lp = p->id;
   hparam[1].type = HOOK_PARAM_SENTINEL;
   pilot_runHookParam(target, PILOT_HOOK_BOARD_ALL, hparam, 1);
   hparam[0].u.lp = target->id;
   pilot_runHookParam(p, PILOT_HOOK_BOARDING, hparam, 1);

   /* Run outfit stuff. */
   pilot_outfitLOnboard( p, target );

   return 1;
}

/**
 * @brief Finishes the boarding.
 *
 *    @param p Pilot to finish the boarding.
 */
void pilot_boardComplete( Pilot *p )
{
   /* Make sure target is valid. */
   Pilot *target = pilot_getTarget( p );
   if (target == NULL)
      return;

   /* In the case of the player take fewer credits. */
   if (pilot_isPlayer(target)) {
      char creds[ ECON_CRED_STRLEN ];
      credits_t worth   = MIN( 0.1*pilot_worth(target,0), target->credits );
      p->credits       += worth * p->stats.loot_mod;
      target->credits  -= worth;
      credits2str( creds, worth, 2 );
      player_message(
            _("#%c%s#0 has plundered %s from your ship!"),
            pilot_getFactionColourChar(p), p->name, creds );
   }
   else {
      /* Steal stuff, we only do credits for now. */
      p->credits += target->credits * p->stats.loot_mod;
      target->credits = 0.;
   }

   /* Finish the boarding. */
   pilot_rmFlag(p, PILOT_BOARDING);
}
