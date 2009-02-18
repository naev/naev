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
static int board_trySteal( Pilot *p );
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

   if (p->fuel <= 0.) { /* no fuel. */
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
 * @brief Checks to see if the pilot can steal from it's target.
 *
 *    @param p Pilot stealing from it's target.
 *    @return 0 if successful, 1 if fails, -1 if fails and kills target.
 */
static int board_trySteal( Pilot *p )
{
   Pilot *target;

   /* Get the target. */
   target = pilot_get(p->target);
   if (target == NULL)
      return 1;

   /* See if was successful. */
   if (RNGF() > (0.5 * 
            (10. + (double)target->ship->crew)/(10. + (double)p->ship->crew)))
      return 0;

   /* Triggered self destruct. */
   if (RNGF() < 0.4) {
      /* Don't actually kill. */
      target->armour = 1.;
      /* This will make the boarding ship take the possible faction hit. */
      pilot_hit( target, NULL, p->id, DAMAGE_TYPE_KINETIC, 100. );
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

   ret = board_trySteal( player );

   if (ret == 0)
      return 0;
   else if (ret < 0) /* killed ship. */
      player_message("You have tripped the ship's self destruct mechanism!");
   else /* you just got locked out */
      player_message("The ship's security system locks %s out.",
            (player->ship->crew > 0) ? "your crew" : "you" );

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
   char cred[10];
   Pilot* p;

   p = pilot_get(player->target);
   j = 0;

   /* Credits. */
   credits2str( cred, p->credits, 2 );
   j += snprintf( &str[j], PATH_MAX-j, "%s\n", cred );

   /* Commodities. */
   if (p->ncommodities==0)
      j += snprintf( &str[j], PATH_MAX-j, "none\n" );
   else {
      for (i=0; i<p->ncommodities; i++)
         j += snprintf( &str[j], PATH_MAX-j,
               "%d %s\n",
               p->commodities[i].quantity, p->commodities[i].commodity->name );
   }

   /* Fuel. */
   if (p->fuel <= 0.)
      j += snprintf( &str[j], PATH_MAX-j, "none\n" );
   else
      j += snprintf( &str[j], PATH_MAX-j, "%.0f Units\n", p->fuel );

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

   /* Make sure target is sane. */
   target = pilot_get(p->target);
   if (target == NULL)
      return;

   /* Steal stuff, we only do credits for now. */
   ret = board_trySteal(p);
   if (ret == 0) {
      p->credits += target->credits;
      target->credits = 0.;
   }

   /* Finish the boarding. */
   pilot_rmFlag(p, PILOT_BOARDING);
}


