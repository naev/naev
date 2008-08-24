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


/*
 * externs
 */
extern unsigned int player_target;


static unsigned int board_wid = 0; /**< Boarding window identifier. */


/*
 * prototypes
 */
static void board_exit( char* str );
static void board_stealCreds( char* str );
static void board_stealCargo( char* str );
static int board_fail (void);
static void board_update (void);


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

   if (player_target==PLAYER_ID) {
      player_message("You need a target to board first!");
      return;
   }

   p = pilot_get(player_target);

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
    * create the boarding window
    */
   board_wid = window_create( "Boarding", -1, -1, BOARDING_WIDTH, BOARDING_HEIGHT );

   window_addText( board_wid, 20, -30, 120, 60,
         0, "txtCargo", &gl_smallFont, &cDConsole,
         "Credits:\n"
         "Cargo:\n"
         );
   window_addText( board_wid, 80, -30, 120, 60,
         0, "txtData", &gl_smallFont, &cBlack, NULL );

   window_addButton( board_wid, 20, 20, 50, 30, "btnStealCredits",
         "Credits", board_stealCreds);
   window_addButton( board_wid, 90, 20, 50, 30, "btnStealCargo",
         "Cargo", board_stealCargo);

   window_addButton( board_wid, -20, 20, 50, 30, "btnBoardingClose",
         "Leave", board_exit );

   board_update();

   /*
    * run hook if needed
    */
   pilot_runHook(p, PILOT_HOOK_BOARD);
}

/**
 * @fn static void board_exit( char* str )
 *
 * @brief Closes the boarding window.
 *
 *    @param str Unused.
 */
static void board_exit( char* str )
{
   (void)str;
   window_destroy( window_get("Boarding") );
}


/**
 * @fn static void board_stealCreds( char* str )
 *
 * @brief Attempt to steal the boarded ship's credits.
 *
 *    @param str Unused.
 */
static void board_stealCreds( char* str )
{
   (void)str;
   Pilot* p;

   p = pilot_get(player_target);

   if (p->credits==0) { /* you can't steal from the poor */
      player_message("The ship has no credits.");
      return;
   }

   if (board_fail()) return;

   player->credits += p->credits;
   p->credits = 0;
   board_update(); /* update the lack of credits */
   player_message("You manage to steal the ship's credits.");
}


/**
 * @fn static void board_stealCargo( char* str )
 *
 * @brief Attempt to steal the bearded ship's cargo.
 *
 *    @param str Unused.
 */
static void board_stealCargo( char* str )
{
   (void)str;
   int q;
   Pilot* p;

   p = pilot_get(player_target);

   if (p->ncommodities==0) { /* no cargo */
      player_message("The ship has no cargo.");
      return;
   }
   else if (pilot_cargoFree(player) <= 0) {
      player_message("You have no room for cargo.");
      return;
   }

   if (board_fail()) return;

   /** steal as much as possible until full - @todo let player choose */
   q = 1;
   while ((p->ncommodities > 0) && (q!=0)) {
      q = pilot_addCargo( player, p->commodities[0].commodity,
            p->commodities[0].quantity );
      pilot_rmCargo( p, p->commodities[0].commodity, q );
   }

   board_update();
   player_message("You manage to steal the ship's cargo.");
}


/**
 * @fn static int board_fail (void)
 *
 * @brief Checks to see if the hijack attempt failed.
 *
 *    @return 1 on failure to board, otherwise 0.
 */
static int board_fail (void)
{
   Pilot* p;

   p = pilot_get(player_target);

   /* fail chance */
   if (RNG(0,100) > (int)(50. * 
            (10. + (double)p->ship->crew)/(10. + (double)player->ship->crew)))
      return 0;

   if (RNG(0,2)==0) { /* 33% of instadeath */
      p->armour = -1.;
      player_message("You have tripped the ship's self destruct mechanism!");
   }
   else /* you just got locked out */
      player_message("The ship's security system locks you out.");

   board_exit(NULL);
   return 1;
}


/**
 * @fn static void board_update (void)
 *
 * @brief Updates the boarding window fields.
 */
static void board_update (void)
{
   int i;
   char str[128], buf[32];
   char cred[10];
   Pilot* p;

   p = pilot_get(player_target);

   credits2str( cred, p->credits, 2 );

   snprintf( str, 11,
         "%s\n", cred );
   if (p->ncommodities==0)
      strncat( str, "none", 10 );
   else {
      for (i=0; i<p->ncommodities; i++) {
         snprintf( buf, 32, 
               "%d %s\n",
               p->commodities[i].quantity, p->commodities[i].commodity->name );
         strncat( str, buf, 32 );
      }
   }

   window_modifyText( board_wid, "txtData", str ); 
}
