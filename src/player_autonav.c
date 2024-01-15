/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player_autonav.c
 *
 * @brief Contains all the player autonav related stuff.
 */
/** @cond */
#include <time.h>

#include "naev.h"
/** @endcond */

#include "player.h"

#include "ai.h"
#include "array.h"
#include "board.h"
#include "conf.h"
#include "map.h"
#include "pause.h"
#include "pilot.h"
#include "pilot_ew.h"
#include "player.h"
#include "map_overlay.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_vec2.h"
#include "nlua_pilot.h"
#include "nlua_spob.h"
#include "nlua_system.h"
#include "sound.h"
#include "space.h"
#include "toolkit.h"

static nlua_env autonav_env   = LUA_NOREF; /**< Autonav environment. */
static int func_system        = LUA_NOREF;
static int func_spob          = LUA_NOREF;
static int func_pilot         = LUA_NOREF;
static int func_board         = LUA_NOREF;
static int func_pos           = LUA_NOREF;
static int func_reset         = LUA_NOREF;
static int func_end           = LUA_NOREF;
static int func_abort         = LUA_NOREF;
static int func_think         = LUA_NOREF;
static int func_update        = LUA_NOREF;
static int func_enter         = LUA_NOREF;

/*
 * Prototypes.
 */
static int player_autonavSetup (void);

int player_autonavInit (void)
{
   nlua_env env = nlua_newEnv();
   nlua_loadStandard( env );
   nlua_loadAI( env );

   size_t bufsize;
   char *buf = ndata_read( AUTONAV_PATH, &bufsize );
   if (nlua_dobufenv(env, buf, bufsize, AUTONAV_PATH) != 0) {
      WARN( _("Error loading file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check"),
            AUTONAV_PATH, lua_tostring(naevL,-1));
      free(buf);
      return -1;
   }
   free(buf);
   autonav_env = env;

   func_system = nlua_refenvtype( env, "autonav_system", LUA_TFUNCTION );
   func_spob   = nlua_refenvtype( env, "autonav_spob",   LUA_TFUNCTION );
   func_pilot  = nlua_refenvtype( env, "autonav_pilot",  LUA_TFUNCTION );
   func_board  = nlua_refenvtype( env, "autonav_board",  LUA_TFUNCTION );
   func_pos    = nlua_refenvtype( env, "autonav_pos",    LUA_TFUNCTION );
   func_reset  = nlua_refenvtype( env, "autonav_reset",  LUA_TFUNCTION );
   func_end    = nlua_refenvtype( env, "autonav_end",    LUA_TFUNCTION );
   func_abort  = nlua_refenvtype( env, "autonav_abort",  LUA_TFUNCTION );
   func_think  = nlua_refenvtype( env, "autonav_think",  LUA_TFUNCTION );
   func_update = nlua_refenvtype( env, "autonav_update", LUA_TFUNCTION );
   func_enter  = nlua_refenvtype( env, "autonav_enter",  LUA_TFUNCTION );

   return 0;
}

/**
 * @brief Resets the game speed.
 */
void player_autonavResetSpeed (void)
{
   player_resetSpeed();
}

/**
 * @brief Starts autonav.
 */
void player_autonavStart (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return;

   if ((player.p->nav_hyperspace == -1) && (player.p->nav_spob== -1))
      return;
   else if ((player.p->nav_spob != -1) && !player_getHypPreempt()) {
      player_autonavSpob( cur_system->spobs[ player.p->nav_spob ]->name, 0 );
      return;
   }

   if (player.p->fuel < player.p->fuel_consumption) {
      player_message("#r%s#0",_("Not enough fuel to jump for autonav."));
      return;
   }

   if (pilot_isFlag( player.p, PILOT_NOJUMP)) {
      player_message("#r%s#0",_("Hyperspace drive is offline."));
      return;
   }

   if (player_autonavSetup())
      return;

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_system );
   if (nlua_pcall( autonav_env, 0, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
   player.autonav = AUTONAV_JUMP;
}

/**
 * @brief Prepares the player to enter autonav.
 *
 *    @return 0 on success, -1 on failure (disabled, etc.)
 */
static int player_autonavSetup (void)
{
   /* Not under manual control or disabled. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
         pilot_isDisabled(player.p))
      return -1;

   /* Autonav is mutually-exclusive with other autopilot methods. */
   player_restoreControl( PINPUT_AUTONAV, NULL );
   player_setFlag(PLAYER_AUTONAV);

   return 0;
}

static int autonav_ending = 0;
/**
 * @brief Ends the autonav.
 */
void player_autonavEnd (void)
{
   /* Don't allow recursive end chaining. */
   if (autonav_ending)
      return;

   autonav_ending = 1;
   player_rmFlag(PLAYER_AUTONAV);
   ovr_autonavClear();
   player_accelOver();

   /* End it. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_end );
   if (nlua_pcall( autonav_env, 0, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }

   autonav_ending = 0;
}

/**
 * @brief Starts autonav and closes the window.
 */
void player_autonavStartWindow( unsigned int wid, const char *str )
{
   (void) str;
   player_hyperspacePreempt( 1 );
   player_autonavStart();
   window_destroy( wid );
}

/**
 * @brief Starts autonav with a local position destination.
 */
void player_autonavPos( double x, double y )
{
   vec2 pos;

   if (player_autonavSetup())
      return;

   vec2_cset( &pos, x, y );
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_pos );
   lua_pushvector( naevL, pos );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
   player.autonav = AUTONAV_POS;
}

/**
 * @brief Starts autonav with a spob destination.
 */
void player_autonavSpob( const char *name, int tryland )
{
   const Spob *spb;

   if (player_autonavSetup())
      return;

   spb = spob_get( name );

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_spob );
   lua_pushspob( naevL, spob_index(spb) );
   lua_pushboolean( naevL, tryland );
   if (nlua_pcall( autonav_env, 2, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
   player.autonav = AUTONAV_SPOB;

}

/**
 * @brief Starts autonav with a pilot to follow.
 */
void player_autonavPil( unsigned int p )
{
   const Pilot *pilot = pilot_get( p );
   int inrange  = pilot_inRangePilot( player.p, pilot, NULL );
   if (player_autonavSetup() || !inrange)
      return;

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_pilot );
   lua_pushpilot( naevL, p );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
   player.autonav = AUTONAV_PILOT;
}

/**
 * @brief Starts autonav with a pilot to board.
 */
void player_autonavBoard( unsigned int p )
{
   const Pilot *pilot = pilot_get( p );
   int inrange  = pilot_inRangePilot( player.p, pilot, NULL );
   if (player_autonavSetup() || !inrange)
      return;

   /* Detected fuzzy, can't board. */
   if (inrange!=1) {
      player_autonavPil( p );
      return;
   }

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_board );
   lua_pushpilot( naevL, p );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
   player.autonav = AUTONAV_PILOT;
}

/**
 * @brief Aborts autonav.
 *
 *    @param reason Human-readable string describing abort condition.
 */
void player_autonavAbort( const char *reason )
{
   /* No point if player is beyond aborting. */
   if ((player.p==NULL) || pilot_isFlag(player.p, PILOT_HYPERSPACE))
      return;

   /* Not under autonav. */
   if (!player_isFlag(PLAYER_AUTONAV))
      return;

   /* Cooldown (handled later) may be script-initiated and we don't
    * want to make it player-abortable while under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   /* Run Lua function. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_abort );
   if (reason != NULL)
      lua_pushstring( naevL, reason );
   else
      lua_pushnil( naevL );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }

   /* Break possible hyperspacing. */
   if (pilot_isFlag(player.p, PILOT_HYP_PREP)) {
      pilot_hyperspaceAbort(player.p);
      player_message("#o%s#0",_("Autonav: aborting hyperspace sequence."));
   }
}

void player_autonavReset( double s )
{
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_reset );
   lua_pushnumber( naevL, s );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
}

/**
 * @brief Handles autonav thinking.
 *
 *    @param pplayer Player doing the thinking.
 *    @param dt Current delta tick.
 */
void player_thinkAutonav( Pilot *pplayer, double dt )
{
   AIMemory oldmem;

   ai_thinkSetup( dt );
   oldmem = ai_setPilot( pplayer ); /* Uses AI functionality. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_think );
   lua_pushnumber( naevL, dt );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
   ai_unsetPilot( oldmem );
   ai_thinkApply( pplayer );
}

/**
 * @brief Updates the player's autonav.
 *
 *    @param dt Current delta tick (should be real delta tick, not game delta tick).
 */
void player_updateAutonav( double dt )
{
   const double dis_dead = 1.0; /* Time it takes to start ramping up. */
   const double dis_mod  = 2.0;
   //const double dis_max  = 5.0;
   //const double dis_ramp = 3.0;

   if (paused || (player.p==NULL) ||
         pilot_isFlag(player.p, PILOT_DEAD) ||
         player_isFlag( PLAYER_CINEMATICS ))
      return;

   /* TODO maybe handle in autonav? */
   /* We handle disabling here. */
   if (pilot_isFlag(player.p, PILOT_DISABLED)) {
      static double tc_mod  = 1.0;
      /* It is somewhat like:
       *        /------------\        4x
       *       /              \
       * -----/                \----- 1x
       *
       * <---><-><----------><-><--->
       *   5   6     X        6   5    Real time
       *   5   15    X        15  5    Game time
       *
       * For triangles we have to add the rectangle and triangle areas.
       */
      double tc_base = player_dt_default() * player.speed;
      double dis_max = MAX( 5., player.p->dtimer / 10. );
      double dis_ramp = (dis_max-tc_base) / dis_mod;

      double time_left = player.p->dtimer - player.p->dtimer_accum;
      /* Initial deadtime. */
      if ((player.p->dtimer_accum < dis_dead) || (time_left < dis_dead))
         tc_mod = tc_base;
      else {
         /* Ramp down. */
         if (time_left < dis_dead + (dis_max-tc_base)*dis_ramp/2. + tc_base*dis_ramp)
            tc_mod = MAX( tc_base, tc_mod - dis_mod*dt );
         /* Ramp up. */
         else
            tc_mod = MIN( dis_max, tc_mod + dis_mod*dt );
      }
      pause_setSpeed( tc_mod );
      sound_setSpeed( tc_mod / player_dt_default() );
      return;
   }

   /* Must be autonaving. */
   if (!player_isFlag(PLAYER_AUTONAV))
      return;

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_update );
   lua_pushnumber( naevL, dt );
   if (nlua_pcall( autonav_env, 1, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
}

void player_autonavEnter (void)
{
   /* Must be autonaving. */
   if (!player_isFlag(PLAYER_AUTONAV))
      return;

   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_enter );
   if (nlua_pcall( autonav_env, 0, 0 )) {
      WARN("%s",lua_tostring(naevL,-1));
      lua_pop(naevL, 1);
   }
}
