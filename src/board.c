/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file board.c
 *
 * @brief Deals with boarding ships.
 */


#include "board.h"

#include "naev.h"

#include "log.h"
#include "pilot.h"
#include "player.h"
#include "toolkit.h"
#include "space.h"
#include "rng.h"
#include "economy.h"
#include "hook.h"
#include "damagetype.h"
#include "nstring.h"


#define BOARDING_WIDTH  300 /**< Boarding window width. */
#define BOARDING_HEIGHT 200 /**< Boarding window height. */

#define BUTTON_WIDTH     50 /**< Boarding button width. */
#define BUTTON_HEIGHT    30 /**< Boarding button height. */


static int board_stopboard = 0; /**< Whether or not to unboard. */
static int board_boarded   = 0;


/*
 * prototypes
 */
static void board_stealCreds( unsigned int wdw, char* str );
static void board_stealCargo( unsigned int wdw, char* str );
static void board_stealFuel( unsigned int wdw, char* str );
static int board_trySteal( Pilot *p );
static int board_fail( unsigned int wdw );
static void board_update( unsigned int wdw );


/**
 * @brief Gets if the player is boarded.
 */
int player_isBoarded (void)
{
   return board_boarded;
}


/**
 * @fn void player_board (void)
 *
 * @brief Attempt to board the player's target.
 *
 * Creates the window on success.
 */
void player_board (void)
{
   Pilot *p;
   unsigned int wdw;
   char c;
   HookParam hparam[2];

   if (player.p->target==PLAYER_ID) {
      player_message("\erYou need a target to board first!");
      return;
   }

   p = pilot_get(player.p->target);
   c = pilot_getFactionColourChar( p );

   if (pilot_isFlag(p,PILOT_NOBOARD)) {
      player_message("\erTarget ship can not be boarded.");
      return;
   }
   else if (pilot_isFlag(p,PILOT_BOARDED)) {
      player_message("\erYour target cannot be boarded again.");
      return;
   }
   else if (!pilot_isDisabled(p) && !pilot_isFlag(p,PILOT_BOARDABLE)) {
      player_message("\erYou cannot board a ship that isn't disabled!");
      return;
   }
   else if (vect_dist(&player.p->solid->pos,&p->solid->pos) >
         p->ship->gfx_space->sw * PILOT_SIZE_APROX) {
      player_message("\erYou are too far away to board your target.");
      return;
   }
   else if ((pow2(VX(player.p->solid->vel)-VX(p->solid->vel)) +
            pow2(VY(player.p->solid->vel)-VY(p->solid->vel))) >
         (double)pow2(MAX_HYPERSPACE_VEL)) {
      player_message("\erYou are going too fast to board the ship.");
      return;
   }
   /* We'll recover it if it's the pilot's ex-escort. */
   else if (p->parent == PLAYER_ID) {
      /* Try to recover. */
      pilot_dock( p, player.p, 0 );
      if (pilot_isFlag(p, PILOT_DELETE )) { /* Hack to see if it boarded. */
         player_message("\epYou recover \eg%s\ep into your fighter bay.", p->name);
         return;
      }
   }

   /* Is boarded. */
   board_boarded = 1;

   /* pilot will be boarded */
   pilot_setFlag(p,PILOT_BOARDED);
   player_message("\epBoarding ship \e%c%s\e0.", c, p->name);

   /* Don't unboard. */
   board_stopboard = 0;

   /*
    * run hook if needed
    */
   hparam[0].type       = HOOK_PARAM_PILOT;
   hparam[0].u.lp.pilot = p->id;
   hparam[1].type       = HOOK_PARAM_SENTINEL;
   hooks_runParam( "board", hparam );
   pilot_runHook(p, PILOT_HOOK_BOARD);

   if (board_stopboard) {
      board_boarded = 0;
      return;
   }

   /*
    * create the boarding window
    */
   wdw = window_create( "Boarding", -1, -1, BOARDING_WIDTH, BOARDING_HEIGHT );

   window_addText( wdw, 20, -30, 120, 60,
         0, "txtCargo", &gl_smallFont, &cDConsole,
         "Credits:\n"
         "Cargo:\n"
         "Fuel:\n"
         );
   window_addText( wdw, 80, -30, 120, 60,
         0, "txtData", &gl_smallFont, &cBlack, NULL );

   window_addButton( wdw, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnStealCredits", "Credits", board_stealCreds);
   window_addButton( wdw, 20+BUTTON_WIDTH+20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnStealCargo", "Cargo", board_stealCargo);
   window_addButton( wdw, 20+2*(BUTTON_WIDTH+20), 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnStealFuel", "Fuel", board_stealFuel);

   window_addButton( wdw, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBoardingClose", "Leave", board_exit );

   board_update(wdw);
}


/**
 * @brief Forces unboarding of the pilot.
 */
void board_unboard (void)
{
   board_stopboard = 1;
}


/**
 * @brief Closes the boarding window.
 *
 *    @param wdw Window triggering the function.
 *    @param str Unused.
 */
void board_exit( unsigned int wdw, char* str )
{
   (void) str;
   window_destroy( wdw );

   /* Is not boarded. */
   board_boarded = 0;
}


/**
 * @brief Attempt to steal the boarded ship's credits.
 *
 *    @param wdw Window triggering the function.
 *    @param str Unused.
 */
static void board_stealCreds( unsigned int wdw, char* str )
{
   (void)str;
   Pilot* p;

   p = pilot_get(player.p->target);

   if (p->credits==0) { /* you can't steal from the poor */
      player_message("\epThe ship has no credits.");
      return;
   }

   if (board_fail(wdw)) return;

   player_modCredits( p->credits );
   p->credits = 0;
   board_update( wdw ); /* update the lack of credits */
   player_message("\epYou manage to steal the ship's credits.");
}


/**
 * @brief Attempt to steal the boarded ship's cargo.
 *
 *    @param wdw Window triggering the function.
 *    @param str Unused.
 */
static void board_stealCargo( unsigned int wdw, char* str )
{
   (void)str;
   int q;
   Pilot* p;

   p = pilot_get(player.p->target);

   if (p->ncommodities==0) { /* no cargo */
      player_message("\epThe ship has no cargo.");
      return;
   }
   else if (pilot_cargoFree(player.p) <= 0) {
      player_message("\erYou have no room for the ship's cargo.");
      return;
   }

   if (board_fail(wdw)) return;

   /** steal as much as possible until full - @todo let player choose */
   q = 1;
   while ((p->ncommodities > 0) && (q!=0)) {
      q = pilot_cargoAdd( player.p, p->commodities[0].commodity,
            p->commodities[0].quantity );
      pilot_cargoRm( p, p->commodities[0].commodity, q );
   }

   board_update( wdw );
   player_message("\epYou manage to steal the ship's cargo.");
}


/**
 * @brief Attempt to steal the boarded ship's fuel.
 *
 *    @param wdw Window triggering the function.
 *    @param str Unused.
 */
static void board_stealFuel( unsigned int wdw, char* str )
{
   (void)str;
   Pilot* p;

   p = pilot_get(player.p->target);

   if (p->fuel <= 0.) { /* no fuel. */
      player_message("\epThe ship has no fuel.");
      return;
   }
   else if (player.p->fuel == player.p->fuel_max) {
      player_message("\erYour ship is at maximum fuel capacity.");
      return;
   }

   if (board_fail(wdw))
      return;

   /* Steal fuel. */
   player.p->fuel += p->fuel;
   p->fuel = 0.;

   /* Make sure doesn't overflow. */
   if (player.p->fuel > player.p->fuel_max) {
      p->fuel      = player.p->fuel - player.p->fuel_max;
      player.p->fuel = player.p->fuel_max;
   }

   board_update( wdw );
   player_message("\epYou manage to steal the ship's fuel.");
}


/**
 * @brief Checks to see if the pilot can steal from its target.
 *
 *    @param p Pilot stealing from its target.
 *    @return 0 if successful, 1 if fails, -1 if fails and kills target.
 */
static int board_trySteal( Pilot *p )
{
   Pilot *target;
   Damage dmg;

   /* Get the target. */
   target = pilot_get(p->target);
   if (target == NULL)
      return 1;

   /* See if was successful. */
   if (RNGF() > (0.5 * (10. + target->crew)/(10. + p->crew)))
      return 0;

   /* Triggered self destruct. */
   if (RNGF() < 0.4) {
      /* Don't actually kill. */
      target->shield = 0.;
      target->armour = 1.;
      /* This will make the boarding ship take the possible faction hit. */
      dmg.type        = dtype_get("normal");
      dmg.damage      = 100.;
      dmg.penetration = 1.;
      dmg.disable     = 0.;
      pilot_hit( target, NULL, p->id, &dmg );
      /* Return ship dead. */
      return -1;
   }

   return 1;
}


/**
 * @brief Checks to see if the hijack attempt failed.
 *
 *    @return 1 on failure to board, otherwise 0.
 */
static int board_fail( unsigned int wdw )
{
   int ret;

   ret = board_trySteal( player.p );

   if (ret == 0)
      return 0;
   else if (ret < 0) /* killed ship. */
      player_message("\epYou have tripped the ship's self-destruct mechanism!");
   else /* you just got locked out */
      player_message("\epThe ship's security system locks %s out.",
            (player.p->ship->crew > 0) ? "your crew" : "you" );

   board_exit( wdw, NULL);
   return 1;
}


/**
 * @brief Updates the boarding window fields.
 *
 *    @param wdw Window to update.
 */
static void board_update( unsigned int wdw )
{
   int i, j;
   char str[PATH_MAX];
   char cred[ECON_CRED_STRLEN];
   Pilot* p;

   p = pilot_get(player.p->target);
   j = 0;

   /* Credits. */
   credits2str( cred, p->credits, 2 );
   j += nsnprintf( &str[j], PATH_MAX, "%s\n", cred );

   /* Commodities. */
   if (p->ncommodities==0)
      j += nsnprintf( &str[j], PATH_MAX-j, "none\n" );
   else {
      for (i=0; i<p->ncommodities; i++)
         j += nsnprintf( &str[j], PATH_MAX-j,
               "%d %s\n",
               p->commodities[i].quantity, p->commodities[i].commodity->name );
   }

   /* Fuel. */
   if (p->fuel <= 0.)
      nsnprintf( &str[j], PATH_MAX-j, "none\n" );
   else
      nsnprintf( &str[j], PATH_MAX-j, "%.0f Units\n", p->fuel );

   window_modifyText( wdw, "txtData", str );
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

   /* Make sure target is sane. */
   target = pilot_get(p->target);
   if (target == NULL) {
      DEBUG("NO TARGET");
      return 0;
   }

   /* Check if can board. */
   if (!pilot_isDisabled(target))
      return 0;
   else if (vect_dist(&p->solid->pos, &target->solid->pos) >
         target->ship->gfx_space->sw * PILOT_SIZE_APROX )
      return 0;
   else if ((pow2(VX(p->solid->vel)-VX(target->solid->vel)) +
            pow2(VY(p->solid->vel)-VY(target->solid->vel))) >
            (double)pow2(MAX_HYPERSPACE_VEL))
      return 0;
   else if (pilot_isFlag(target,PILOT_BOARDED))
      return 0;

   /* Set the boarding flag. */
   pilot_setFlag(target, PILOT_BOARDED);
   pilot_setFlag(p, PILOT_BOARDING);

   /* Set time it takes to board. */
   p->ptimer = 3.;

   return 1;
}


/**
 * @brief Finishes the boarding.
 *
 *    @param p Pilot to finish the boarding.
 */
void pilot_boardComplete( Pilot *p )
{
   int ret;
   Pilot *target;
   credits_t worth;
   char creds[ ECON_CRED_STRLEN ];

   /* Make sure target is sane. */
   target = pilot_get(p->target);
   if (target == NULL)
      return;

   /* In the case of the player take fewer credits. */
   if (pilot_isPlayer(target)) {
      worth = MIN( 0.1*pilot_worth(target), target->credits );
      p->credits       += worth;
      target->credits  -= worth;
      credits2str( creds, worth, 2 );
      player_message( "\e%c%s\e0 has plundered %s credits from your ship!",
            pilot_getFactionColourChar(p), p->name, creds );
   }
   else {
      /* Steal stuff, we only do credits for now. */
      ret = board_trySteal(p);
      if (ret == 0) {
         /* Normally just plunder it all. */
         p->credits += target->credits;
         target->credits = 0.;
      }
   }

   /* Finish the boarding. */
   pilot_rmFlag(p, PILOT_BOARDING);
}


