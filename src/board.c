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


#define BOARDING_WIDTH  300 /**< Boarding window width. */
#define BOARDING_HEIGHT 200 /**< Boarding window height. */

#define BUTTON_WIDTH     50 /**< Boarding button width. */
#define BUTTON_HEIGHT    30 /**< Boarding button height. */


/*
 * prototypes
 */
static void board_exit( unsigned int wdw, char* str );
static void board_stealCreds( unsigned int wdw, char* str );
static void board_stealCargo( unsigned int wdw, char* str );
static void board_stealFuel( unsigned int wdw, char* str );
static int board_fail( unsigned int wdw );
static void board_update( unsigned int wdw );


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

   if (player->target==PLAYER_ID) {
      player_message("You need a target to board first!");
      return;
   }

   p = pilot_get(player->target);

   if (!pilot_isDisabled(p)) {
      player_message("You cannot board a ship that isn't disabled!");
      return;
   }
   else if (vect_dist(&player->solid->pos,&p->solid->pos) >
         p->ship->gfx_space->sw * PILOT_SIZE_APROX) {
      player_message("You are too far away to board your target.");
      return;
   }
   else if ((pow2(VX(player->solid->vel)-VX(p->solid->vel)) +
            pow2(VY(player->solid->vel)-VY(p->solid->vel))) >
         (double)pow2(MAX_HYPERSPACE_VEL)) {
      player_message("You are going too fast to board the ship.");
      return;
   }
   else if (pilot_isFlag(p,PILOT_BOARDED)) {
      player_message("Your target cannot be boarded again.");
      return;
   };

   /* pilot will be boarded */
   pilot_setFlag(p,PILOT_BOARDED); 
   player_message("Boarding ship %s.", p->name);


   /*
    * Tweak some values.
    */
   p->fuel = (double)RNG(100, p->fuel_max);


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
         "btnStealCargo", "Fuel", board_stealFuel);

   window_addButton( wdw, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBoardingClose", "Leave", board_exit );

   board_update(wdw);

   /*
    * run hook if needed
    */
   pilot_runHook(p, PILOT_HOOK_BOARD);
}

/**
 * @brief Closes the boarding window.
 *
 *    @param wdw Window triggering the function.
 *    @param str Unused.
 */
static void board_exit( unsigned int wdw, char* str )
{
   (void) str;
   window_destroy( wdw );
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

   p = pilot_get(player->target);

   if (p->credits==0) { /* you can't steal from the poor */
      player_message("The ship has no credits.");
      return;
   }

   if (board_fail(wdw)) return;

   player->credits += p->credits;
   p->credits = 0;
   board_update( wdw ); /* update the lack of credits */
   player_message("You manage to steal the ship's credits.");
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

   p = pilot_get(player->target);

   if (p->ncommodities==0) { /* no cargo */
      player_message("The ship has no cargo.");
      return;
   }
   else if (pilot_cargoFree(player) <= 0) {
      player_message("You have no room for cargo.");
      return;
   }

   if (board_fail(wdw)) return;

   /** steal as much as possible until full - @todo let player choose */
   q = 1;
   while ((p->ncommodities > 0) && (q!=0)) {
      q = pilot_addCargo( player, p->commodities[0].commodity,
            p->commodities[0].quantity );
      pilot_rmCargo( p, p->commodities[0].commodity, q );
   }

   board_update( wdw );
   player_message("You manage to steal the ship's cargo.");
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

   p = pilot_get(player->target);

   if (player->fuel <= 0.) { /* no fuel. */
      player_message("The ship has no fuel.");
      return;
   }
   else if (player->fuel == player->fuel_max) {
      player_message("Your ship is at maximum fuel capacity.");
      return;
   }

   if (board_fail(wdw)) return;

   /* Steal fuel. */
   player->fuel += p->fuel;
   p->fuel = 0.;

   /* Make sure doesn't overflow. */
   if (player->fuel > player->fuel_max) {
      p->fuel = player->fuel_max - player->fuel;
      player->fuel = player->fuel_max;
   }

   board_update( wdw );
   player_message("You manage to steal the ship's fuel.");
}


/**
 * @brief Checks to see if the hijack attempt failed.
 *
 *    @return 1 on failure to board, otherwise 0.
 */
static int board_fail( unsigned int wdw )
{
   Pilot* p;

   p = pilot_get(player->target);

   /* fail chance */
   if (RNGF() > (0.5 * 
            (10. + (double)p->ship->crew)/(10. + (double)player->ship->crew)))
      return 0;

   if (RNGF() < 0.4) { /* 40% of instadeath */
      p->armour = -1.;
      player_message("You have tripped the ship's self destruct mechanism!");
   }
   else /* you just got locked out */
      player_message("The ship's security system locks %s out.",
            (player->ship->crew > 0) ? "your crew" : "you" );

   board_exit( wdw, NULL);
   return 1;
}


/**
 * @brief Updates the boarding window fields.
 */
static void board_update( unsigned int wdw )
{
   int i, len;
   char str[128], buf[32];
   char cred[10];
   Pilot* p;

   p = pilot_get(player->target);

   /* Credits. */
   credits2str( cred, p->credits, 2 );
   snprintf( str, 128,
         "%s\n", cred );
   len = strlen(str);

   /* Commodities. */
   if (p->ncommodities==0) {
      strncat( str, "none\n", 128-len );
      len = strlen(str);
   }
   else {
      for (i=0; i<p->ncommodities; i++) {
         snprintf( buf, 32,
               "%d %s\n",
               p->commodities[i].quantity, p->commodities[i].commodity->name );
         strncat( str, buf, 128-len );
         len = strlen(str);
      }
   }

   /* Fuel. */
   if (p->fuel <= 0.)
      strncat( str, "none\n", 128-len );
   else {
      snprintf( buf, 32, "%.0f Units\n", p->fuel );
      strncat( str, buf, 128-len );
   }
   len = strlen(str);

   window_modifyText( wdw, "txtData", str ); 
}
