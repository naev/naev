/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file land_shipyard.c
 *
 * @brief Handles the shipyard at land.
 */


#include "land_shipyard.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "log.h"
#include "player.h"
#include "land.h"
#include "toolkit.h"
#include "tk/toolkit_priv.h"
#include "dialogue.h"


/*
 * Vars.
 */
static Ship* shipyard_selected = NULL; /**< Currently selected shipyard ship. */


/*
 * Helper functions.
 */
static void shipyard_buy( unsigned int wid, char* str );
static void shipyard_trade( unsigned int wid, char* str );
static void shipyard_rmouse( unsigned int wid, char* widget_name );
static void shipyard_renderSlots( double bx, double by, double bw, double bh, void *data );
static void shipyard_renderSlotsRow( double bx, double by, double bw, char *str, ShipOutfitSlot *s, int n );


/**
 * @brief Opens the shipyard window.
 */
void shipyard_open( unsigned int wid )
{
   int i;
   Ship **ships;
   char **sships;
   glTexture **tships;
   int nships;
   int w, h;
   int iw, ih;
   int bw, bh;
   int th;
   int y;
   const char *buf;

   /* Init vars. */
   shipyard_selected = NULL;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   iw = 310 + (w-800);
   ih = h - 60;

   /* Calculate button dimensions. */
   bw = (w - iw - 200) / 2;
   bh = LAND_BUTTON_HEIGHT;

   /* buttons */
   window_addButton( wid, -20, 20,
         bw-20, bh, "btnCloseShipyard",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -20 - bw, 20,
         bw-20, bh, "btnTradeShip",
         "Trade-In", shipyard_trade );
   window_addButton( wid, -20 - bw*2, 20,
         bw-20, bh, "btnBuyShip",
         "Buy", shipyard_buy );

   /* target gfx */
   window_addRect( wid, -41, -50,
         SHIP_TARGET_W, SHIP_TARGET_H, "rctTarget", &cBlack, 0 );
   window_addImage( wid, -40, -50,
         SHIP_TARGET_W, SHIP_TARGET_H, "imgTarget", NULL, 1 );

   /* slot types */
   window_addCust( wid, -20, -160, 148, 70, "cstSlots", 0.,
         shipyard_renderSlots, NULL, NULL );

   /* stat text */
   window_addText( wid, -40, -240, 128, 200, 0, "txtStats",
         &gl_smallFont, &cBlack, NULL );

   /* text */
   buf = "Model:\n"
         "Class:\n"
         "Fabricator:\n"
         "Crew:\n"
         "\n"
         "CPU:\n"
         "Mass:\n"
         "Jump Time:\n"
         "Thrust:\n"
         "Speed:\n"
         "Turn:\n"
         "\n"
         "Absorption:\n"
         "Shield:\n"
         "Armour:\n"
         "Energy:\n"
         "Cargo Space:\n"
         "Fuel:\n"
         "Price:\n"
         "Money:\n"
         "License:\n";
   th = gl_printHeightRaw( &gl_smallFont, 100, buf );
   y  = -55;
   window_addText( wid, 40+iw+20, y,
         100, th, 0, "txtSDesc", &gl_smallFont, &cDConsole, buf );
   window_addText( wid, 40+iw+20+100, y,
         w-(40+iw+20+100)-20, th, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   y -= th + 10;
   window_addText( wid, 20+iw+40, y,
         w-(20+iw+40) - 20, 185, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );

   /* set up the ships to buy/sell */
   ships = tech_getShip( land_planet->tech, &nships );
   if (nships <= 0) {
      sships    = malloc(sizeof(char*));
      sships[0] = strdup("None");
      tships    = malloc(sizeof(glTexture*));
      tships[0] = NULL;
      nships    = 1;
   }
   else {
      sships = malloc(sizeof(char*)*nships);
      tships = malloc(sizeof(glTexture*)*nships);
      for (i=0; i<nships; i++) {
         sships[i] = strdup(ships[i]->name);
         tships[i] = ships[i]->gfx_store;
      }
      free(ships);
   }
   window_addImageArray( wid, 20, 20,
         iw, ih, "iarShipyard", 64./96.*128., 64.,
         tships, sships, nships, shipyard_update, shipyard_rmouse );

   /* write the shipyard stuff */
   shipyard_update(wid, NULL);
}
/**
 * @brief Updates the ships in the shipyard window.
 *    @param wid Window to update the ships in.
 *    @param str Unused.
 */
void shipyard_update( unsigned int wid, char* str )
{
   (void)str;
   char *shipname;
   Ship* ship;
   char buf[PATH_MAX], buf2[ECON_CRED_STRLEN], buf3[ECON_CRED_STRLEN];

   shipname = toolkit_getImageArray( wid, "iarShipyard" );

   /* No ships */
   if (strcmp(shipname,"None")==0) {
      window_modifyImage( wid, "imgTarget", NULL, 0, 0 );
      window_disableButton( wid, "btnBuyShip");
      window_disableButton( wid, "btnTradeShip");
      snprintf( buf, PATH_MAX,
            "None\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "NA\n" );
      window_modifyText( wid, "txtDDesc", buf );
      window_modifyImage( wid, "txtStats", NULL, 0, 0 );
      return;
   }

   ship = ship_get( shipname );
   shipyard_selected = ship;

   /* update image */
   window_modifyImage( wid, "imgTarget", ship->gfx_store, 0, 0 );

   /* update text */
   window_modifyText( wid, "txtStats", ship->desc_stats );
   window_modifyText( wid, "txtDescription", ship->description );
   credits2str( buf2, ship->price, 2 );
   credits2str( buf3, player.p->credits, 2 );
   snprintf( buf, PATH_MAX,
         "%s\n"
         "%s\n"
         "%s\n"
         "%d\n"
         "\n"
         "%.0f teraflops\n"
         "%.0f tons\n"
         "%.1f STU average\n"
         "%.0f kN/ton\n"
         "%.0f m/s\n"
         "%.0f deg/s\n"
         "\n"
         "%.0f%% damage\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f tons\n"
         "%d units\n"
         "%s credits\n"
         "%s credits\n"
         "%s\n",
         ship->name,
         ship_class(ship),
         ship->fabricator,
         ship->crew,
         /* Weapons & Manoeuvrability */
         ship->cpu,
         ship->mass,
         pow( ship->mass, 1./2.5 ) / 5. * (ship->stats.jump_delay/100.+1.), /**< @todo make this more portable. */
         ship->thrust / ship->mass,
         ship->speed,
         ship->turn*180/M_PI,
         /* Misc */
         ship->dmg_absorb*100.,
         ship->shield, ship->shield_regen,
         ship->armour, ship->armour_regen,
         ship->energy, ship->energy_regen,
         ship->cap_cargo,
         ship->fuel,
         buf2,
         buf3,
         (ship->license != NULL) ? ship->license : "None" );
   window_modifyText( wid,  "txtDDesc", buf );

   if (!shipyard_canBuy( shipname ))
      window_disableButton( wid, "btnBuyShip");
   else
      window_enableButton( wid, "btnBuyShip");

   if (!shipyard_canTrade( shipname ))
      window_disableButton( wid, "btnTradeShip");
   else
      window_enableButton( wid, "btnTradeShip");
}

/**
 * @brief Player right-clicks on a ship.
 *    @param wid Window player is buying ship from.
 *    @param widget_name Name of the window. (unused)
 *    @param shipname Name of the ship the player wants to buy. (unused)
 */
static void shipyard_rmouse( unsigned int wid, char* widget_name )
{
    return shipyard_buy(wid, widget_name);
}

/**
 * @brief Player attempts to buy a ship.
 *    @param wid Window player is buying ship from.
 *    @param str Unused.
 */
static void shipyard_buy( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[ECON_CRED_STRLEN];
   Ship* ship;

   shipname = toolkit_getImageArray( wid, "iarShipyard" );
   ship = ship_get( shipname );

   int targetprice = ship->price;

   if (land_errDialogue( shipname, "buy" ))
      return;

   credits2str( buf, targetprice, 2 );
   if (dialogue_YesNo("Are you sure?", /* confirm */
         "Do you really want to spend %s on a new ship?", buf )==0)
      return;

   /* player just gots a new ship */
   if (player_newShip( ship, NULL, 0 ) != 0) {
      /* Player actually aborted naming process. */
      return;
   }
   player_modCredits( -targetprice ); /* ouch, paying is hard */
   land_checkAddRefuel();

   /* Update shipyard. */
   shipyard_update(wid, NULL);
}

/**
 * @brief Makes sure it's sane to buy a ship.
 *    @param shipname Ship being bought.
 */
int shipyard_canBuy ( char *shipname )
{
   Ship* ship;
   ship = ship_get( shipname );
   int failure = 0;

   /* Must have enough credits and the necessary license. */
   if (!player_hasLicense(ship->license)) {
      land_errDialogueBuild( "You lack the %s.", ship->license );
      failure = 1;
   }
   if (!player_hasCredits( ship->price )) {
      char buf[ECON_CRED_STRLEN];
      credits2str( buf, ship->price - player.p->credits, 2 );
      land_errDialogueBuild( "You need %s more credits.", buf);
      failure = 1;
   }
   return !failure;
}

/**
 * @brief Makes sure it's sane to sell a ship.
 *    @param shipname Ship being sold.
 */
int can_sell( char* shipname )
{
   int failure = 0;
   Pilot* ship;
   ship = player_getShip( shipname );
   if (strcmp(shipname,player.p->name)==0) { /* Already onboard. */
      land_errDialogueBuild( "You can't sell the ship you're piloting.", shipname );
      failure = 1;
   }

   return !failure;
}

/**
 * @brief Makes sure it's sane to change ships.
 *    @param shipname Ship being changed to.
 */
int can_swap( char* shipname )
{
   int failure = 0;
   Ship* ship;
   ship = ship_get( shipname );

   if (pilot_cargoUsed(player.p) > ship->cap_cargo) { /* Current ship has too much cargo. */
      land_errDialogueBuild( "You have %g tons more cargo than the new ship can hold.",
            pilot_cargoUsed(player.p) - ship->cap_cargo, ship->name );
      failure = 1;
   }
   if (pilot_hasDeployed(player.p)) { /* Escorts are in space. */
      land_errDialogueBuild( "You can't strand your fighters in space.");
      failure = 1;
   }
   return !failure;
}


/**
 * @brief Makes sure it's sane to buy a ship, trading the old one in simultaneously.
 *    @param shipname Ship being bought.
 */
int shipyard_canTrade( char* shipname )
{
   int failure = 0;
   Ship* ship;
   ship = ship_get( shipname );

   /* Must have the necessary license, enough credits, and be able to swap ships. */
   if (!player_hasLicense(ship->license)) {
      land_errDialogueBuild( "You lack the %s.", ship->license );
      failure = 1;
   }
   if (!player_hasCredits( ship->price - player_shipPrice(player.p->name))) {
      int creditdifference = ship->price - (player_shipPrice(player.p->name) + player.p->credits);
      char buf[ECON_CRED_STRLEN];
      credits2str( buf, creditdifference, 2 );
      land_errDialogueBuild( "You need %s more credits.", buf);
      failure = 1;
   }
   if (!can_swap( shipname ))
      failure = 1;
   return !failure;
}


/**
 * @brief Player attempts to buy a ship, trading the current ship in.
 *    @param wid Window player is buying ship from.
 *    @param str Unused.
 */
static void shipyard_trade( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[ECON_CRED_STRLEN], buf2[ECON_CRED_STRLEN],
         buf3[ECON_CRED_STRLEN], buf4[ECON_CRED_STRLEN];
   Ship* ship;

   shipname = toolkit_getImageArray( wid, "iarShipyard" );
   ship = ship_get( shipname );

   int targetprice = ship->price;
   int playerprice = player_shipPrice(player.p->name);

   if (land_errDialogue( shipname, "trade" ))
      return;

   credits2str( buf, targetprice, 2 );
   credits2str( buf2, playerprice, 2 );
   credits2str( buf3, targetprice - playerprice, 2 );
   credits2str( buf4, playerprice - targetprice, 2 );

   /* Display the correct dialogue depending on the new ship's price versus the player's. */
   if ( targetprice == playerprice ) {
      if (dialogue_YesNo("Are you sure?", /* confirm */
         "Your %s is worth %s, exactly as much as the new ship, so no credits need be exchanged. Are you sure you want to trade your ship in?",
               player.p->ship->name, buf2)==0)
         return;
   }
   else if ( targetprice < playerprice ) {
      if (dialogue_YesNo("Are you sure?", /* confirm */
         "Your %s is worth %s credits, more than the new ship. For your ship, you will get the new %s and %s credits. Are you sure you want to trade your ship in?",
               player.p->ship->name, buf2, ship->name, buf4)==0)
         return;
   }
   else if ( targetprice > playerprice ) {
      if (dialogue_YesNo("Are you sure?", /* confirm */
         "Your %s is worth %s, so the new ship will cost %s credits. Are you sure you want to trade your ship in?",
               player.p->ship->name, buf2, buf3)==0)
         return;
   }

   /* player just gots a new ship */
   if (player_newShip( ship, NULL, 1 ) != 0)
      return; /* Player aborted the naming process. */

   player_modCredits( playerprice - targetprice ); /* Modify credits by the difference between ship values. */

   land_checkAddRefuel();

   /* Update shipyard. */
   shipyard_update(wid, NULL);
}


/**
 * @brief Custom widget render function for the slot widget.
 */
static void shipyard_renderSlots( double bx, double by, double bw, double bh, void *data )
{
   (void) data;
   double x, y, w;
   Ship *ship;

   /* Make sure a valid ship is selected. */
   ship = shipyard_selected;
   if (ship == NULL)
      return;

   y = by + bh;

   /* Draw rotated text. */
   y -= 10;
   gl_print( &gl_smallFont, bx, y, &cBlack, "Slots:" );

   x = bx + 10.;
   w = bw - 10.;

   /* Weapon slots. */
   y -= 20;
   shipyard_renderSlotsRow( x, y, w, "W", ship->outfit_weapon, ship->outfit_nweapon );

   /* Utility slots. */
   y -= 20;
   shipyard_renderSlotsRow( x, y, w, "U", ship->outfit_utility, ship->outfit_nutility );

   /* Structure slots. */
   y -= 20;
   shipyard_renderSlotsRow( x, y, w, "S", ship->outfit_structure, ship->outfit_nstructure );
}


/**
 * @brief Renders a row of ship slots.
 */
static void shipyard_renderSlotsRow( double bx, double by, double bw, char *str, ShipOutfitSlot *s, int n )
{
   (void) bw;
   int i;
   double x;
   glColour *c;

   x = bx;

   /* Print text. */
   gl_print( &gl_smallFont, bx, by, &cBlack, str );

   /* Draw squares. */
   for (i=0; i<n; i++) {
      c = outfit_slotSizeColour( &s[i].slot );
      if (c == NULL)
         c = &cBlack;

      x += 15.;
      toolkit_drawRect( x, by, 10, 10, c, NULL );
   }
}





