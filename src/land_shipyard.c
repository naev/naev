/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file land_shipyard.c
 *
 * @brief Handles the shipyard at land.
 */


/** @cond */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "land_shipyard.h"

#include "array.h"
#include "dialogue.h"
#include "hook.h"
#include "land_takeoff.h"
#include "log.h"
#include "map_find.h"
#include "ndata.h"
#include "nstring.h"
#include "player.h"
#include "space.h"
#include "tk/toolkit_priv.h"
#include "toolkit.h"


#define  SHIP_GFX_W     256
#define  SHIP_GFX_H     256


/*
 * Vars.
 */
static Ship **shipyard_list = NULL; /**< Array (array.h): Available ships, valid when the shipyard image-array widget is. */
static Ship* shipyard_selected = NULL; /**< Currently selected shipyard ship. */
static glTexture *shipyard_comm = NULL; /**< Current comm image. */


/*
 * Helper functions.
 */
static void shipyard_buy( unsigned int wid, char* str );
static void shipyard_trade( unsigned int wid, char* str );
static void shipyard_rmouse( unsigned int wid, char* widget_name );
static void shipyard_renderSlots( double bx, double by, double bw, double bh, void *data );
static void shipyard_renderSlotsRow( double bx, double by, double bw, const char *str, ShipOutfitSlot *s );
static void shipyard_find( unsigned int wid, char* str );


/**
 * @brief Opens the shipyard window.
 */
void shipyard_open( unsigned int wid )
{
   int i;
   ImageArrayCell *cships;
   int nships;
   int w, h, sw, sh;
   int iw, ih;
   int bw, bh, padding, off;
   int th;
   int y;
   const char *buf;
   glTexture *t;
   int iconsize;

   /* Mark as generated. */
   land_tabGenerate(LAND_WINDOW_SHIPYARD);

   /* Init vars. */
   shipyard_cleanup();

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   iw = 440 + (w - LAND_WIDTH);
   ih = h - 60;

   /* Ship image dimensions. */
   sw = SHIP_GFX_W;
   sh = SHIP_GFX_H;

   /* Left padding + per-button padding * nbuttons */
   padding = 40 + 20 * 4;

   /* Calculate button dimensions. */
   bw = (w - iw - padding) / 4;
   bh = LAND_BUTTON_HEIGHT;

   /* buttons */
   window_addButtonKey( wid, off = -20, 20,
         bw, bh, "btnCloseShipyard",
         _("Take Off"), land_buttonTakeoff, SDLK_t );
   window_addButtonKey( wid, off -= 20+bw, 20,
         bw, bh, "btnTradeShip",
         _("Trade-In"), shipyard_trade, SDLK_r );
   window_addButtonKey( wid, off -= 20+bw, 20,
         bw, bh, "btnBuyShip",
         _("Buy"), shipyard_buy, SDLK_b );
   window_addButtonKey( wid, off -= 20+bw, 20,
         bw, bh, "btnFindShips",
         _("Find Ships"), shipyard_find, SDLK_f );

   /* target gfx */
   window_addRect( wid, -40+4, -40+4, sw+8, sh+8, "rctTarget", &cBlack, 1 );
   window_addImage( wid, -40, -40, sw, sh, "imgTarget", NULL, 0);

   /* slot types */
   window_addCust( wid, -20, -sh-50, sw-10, 80, "cstSlots", 0.,
         shipyard_renderSlots, NULL, NULL );

   /* stat text */
   window_addText( wid, -4, -sw-50-70-20, sw, -sh-60-70-20+h-bh, 0, "txtStats",
         &gl_smallFont, NULL, NULL );

   /* text */
   buf = _("Model:\n"
         "Class:\n"
         "Fabricator:\n"
         "Crew:\n"
         "\n"
         "CPU:\n"
         "Mass:\n"
         "Thrust:\n"
         "Speed:\n"
         "Turn:\n"
         "Time Constant:\n"
         "\n"
         "Absorption:\n"
         "Shield:\n"
         "Armour:\n"
         "Energy:\n"
         "Cargo Space:\n"
         "Fuel:\n"
         "Fuel Use:\n"
         "Price:\n"
         "Money:\n"
         "License:\n");
   th = gl_printHeightRaw( &gl_defFont, 106, buf );
   y  = -35;
   window_addText( wid, 20+iw+20, y,
         128, th, 0, "txtSDesc", &gl_defFont, &cFontGrey, buf );
   window_addText( wid, 20+iw+20+128, y,
         w-sw-40-(20+iw+20+128), th, 0, "txtDDesc", &gl_defFont, NULL, NULL );
   y -= th;
   window_addText( wid, 20+iw+20, y,
         w-(20+iw+20) - (sw+40), y-20+h-bh, 0, "txtDescription",
         &gl_defFont, NULL, NULL );

   /* set up the ships to buy/sell */
   shipyard_list = tech_getShip( land_planet->tech );
   nships = array_size( shipyard_list );
   cships = calloc( MAX(1,nships), sizeof(ImageArrayCell) );
   if (nships <= 0) {
      cships[0].image = NULL;
      cships[0].caption = strdup(_("None"));
      nships    = 1;
   }
   else {
      for (i=0; i<nships; i++) {
         cships[i].caption = strdup( _(shipyard_list[i]->name) );
         cships[i].image = gl_dupTexture(shipyard_list[i]->gfx_store);
         cships[i].layers = gl_copyTexArray( shipyard_list[i]->gfx_overlays, &cships[i].nlayers );
         if (shipyard_list[i]->rarity > 0) {
            t = rarity_texture( shipyard_list[i]->rarity );
            cships[i].layers = gl_addTexArray( cships[i].layers, &cships[i].nlayers, t );
         }
      }
   }

   iconsize = 128;
   if (!conf.big_icons) {
      if (toolkit_simImageArrayVisibleElements(iw,ih,iconsize,iconsize) < nships)
         iconsize = 96;
      /*
      if (toolkit_simImageArrayVisibleElements(iw,ih,iconsize,iconsize) < nships)
         iconsize = 64;
      */
   }
   window_addImageArray( wid, 20, 20,
         iw, ih, "iarShipyard", iconsize, iconsize,
         cships, nships, shipyard_update, shipyard_rmouse, NULL );

   /* write the shipyard stuff */
   shipyard_update(wid, NULL);
   /* Set default keyboard focuse to the list */
   window_setFocus( wid , "iarShipyard" );
}
/**
 * @brief Updates the ships in the shipyard window.
 *    @param wid Window to update the ships in.
 *    @param str Unused.
 */
void shipyard_update( unsigned int wid, char* str )
{
   (void)str;
   int i;
   Ship* ship;
   char buf[PATH_MAX], buf2[ECON_CRED_STRLEN], buf3[ECON_CRED_STRLEN], buf_license[PATH_MAX];
   char smass[NUM2STRLEN];
   double aspect, gw, gh;

   i = toolkit_getImageArrayPos( wid, "iarShipyard" );

   /* No ships */
   if (i < 0 || array_size(shipyard_list) == 0) {
      gl_freeTexture( shipyard_comm );
      shipyard_comm = NULL;
      window_modifyImage( wid, "imgTarget", NULL, 0, 0 );
      window_disableButton( wid, "btnBuyShip");
      window_disableButton( wid, "btnTradeShip");
      snprintf( buf, sizeof(buf),
            _("None\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "N/A\n") );
      window_modifyImage( wid, "imgTarget", NULL, 0, 0 );
      window_modifyText( wid, "txtStats", NULL );
      window_modifyText( wid, "txtDescription", NULL );
      window_modifyText( wid, "txtDDesc", buf );
      return;
   }

   ship = shipyard_list[i];
   shipyard_selected = ship;

   /* update image */
   gl_freeTexture( shipyard_comm );
   shipyard_comm = NULL;
   shipyard_comm = ship_loadCommGFX( ship );
   aspect = shipyard_comm->w / shipyard_comm->h;
   gw = MIN( shipyard_comm->w, SHIP_GFX_W );
   gh = MIN( shipyard_comm->h, SHIP_GFX_H );
   if (aspect > 1.)
      gh /= aspect;
   else
      gw /= aspect;
   window_destroyWidget( wid, "imgTarget" );
   window_addImage( wid, -40-(SHIP_GFX_W-gw)/2, -30-(SHIP_GFX_H-gh)/2, gw, gh, "imgTarget", NULL, 0 );
   window_modifyImage( wid, "imgTarget", shipyard_comm, gw, gh );

   /* update text */
   window_modifyText( wid, "txtStats", ship->desc_stats );
   window_modifyText( wid, "txtDescription", _(ship->description) );
   price2str( buf2, ship_buyPrice(ship), player.p->credits, 2 );
   credits2str( buf3, player.p->credits, 2 );
   num2str( smass, ship->mass, 0 );

   if (ship->license == NULL)
      strncpy( buf_license, _("None"), sizeof(buf_license)-1 );
   else if (player_hasLicense( ship->license ))
      strncpy( buf_license, _(ship->license), sizeof(buf_license)-1 );
   else
      snprintf( buf_license, sizeof(buf_license), "#r%s#0", _(ship->license) );

   snprintf( buf, sizeof(buf),
         _("%s\n"
         "%s\n"
         "%s\n"
         "%d\n"
         "\n"
         "%.0f %s\n"
         "%s %s\n"
         "%.0f kN/tonne\n"
         "%.0f m/s\n"
         "%.0f deg/s\n"
         "%.0f%%\n"
         "\n"
         "%.0f%% damage\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f %s\n"
         "%d %s\n"
         "%d %s\n"
         "%s\n"
         "%s\n"
         "%s\n"),
         _(ship->name),
         _(ship_classDisplay(ship)),
         _(ship->fabricator),
         ship->crew,
         /* Weapons & Manoeuvrability */
         ship->cpu, n_( "teraflop", "teraflops", ship->cpu ),
         smass, n_( "tonne", "tonnes", ship->mass ),
         ship->thrust,
         ship->speed,
         ship->turn*180/M_PI,
         ship->dt_default*100.,
         /* Misc */
         ship->dmg_absorb*100.,
         ship->shield, ship->shield_regen,
         ship->armour, ship->armour_regen,
         ship->energy, ship->energy_regen,
         ship->cap_cargo, n_( "tonne", "tonnes", ship->cap_cargo ),
         ship->fuel, n_( "unit", "units", ship->fuel ),
         ship->fuel_consumption, n_( "unit", "units", ship->fuel_consumption ),
         buf2,
         buf3,
         buf_license );
   window_modifyText( wid,  "txtDDesc", buf );

   if (!shipyard_canBuy( ship->name, land_planet ))
      window_disableButtonSoft( wid, "btnBuyShip");
   else
      window_enableButton( wid, "btnBuyShip");

   if (!shipyard_canTrade( ship->name ))
      window_disableButtonSoft( wid, "btnTradeShip");
   else
      window_enableButton( wid, "btnTradeShip");
}


/**
 * @brief Cleans up shipyard data.
 */
void shipyard_cleanup (void)
{
   array_free( shipyard_list );
   shipyard_list = NULL;
   shipyard_selected = NULL;
   gl_freeTexture( shipyard_comm );
   shipyard_comm = NULL;
}


/**
 * @brief Starts the map find with ship search selected.
 *    @param wid Window buying outfit from.
 *    @param str Unused.
 */
static void shipyard_find( unsigned int wid, char* str )
{
   (void) str;
   map_inputFindType(wid, "ship");
}


/**
 * @brief Player right-clicks on a ship.
 *    @param wid Window player is buying ship from.
 *    @param widget_name Name of the window. (unused)
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
   int i;
   char buf[ECON_CRED_STRLEN];
   Ship* ship;
   HookParam hparam[2];

   i = toolkit_getImageArrayPos( wid, "iarShipyard" );
   if (i < 0 || array_size(shipyard_list) == 0)
      return;

   ship = shipyard_list[i];

   credits_t targetprice = ship_buyPrice(ship);

   if (land_errDialogue( ship->name, "buyShip" ))
      return;

   credits2str( buf, targetprice, 2 );
   if (dialogue_YesNo(_("Are you sure?"), /* confirm */
         _("Do you really want to spend %s on a new ship?"), buf )==0)
      return;

   /* player just got a new ship */
   if (player_newShip( ship, NULL, 0, 0 ) == NULL) {
      /* Player actually aborted naming process. */
      return;
   }
   player_modCredits( -targetprice ); /* ouch, paying is hard */

   /* Update shipyard. */
   shipyard_update(wid, NULL);

   /* Run hook. */
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = ship->name;
   hparam[1].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "ship_buy", hparam );
   if (land_takeoff)
      takeoff(1);
}

/**
 * @brief Makes sure it's valid to buy a ship.
 *    @param shipname Ship being bought.
 *    @param planet Where the player is shopping.
 */
int shipyard_canBuy( const char *shipname, Planet *planet )
{
   Ship* ship;
   ship = ship_get( shipname );
   int failure = 0;
   credits_t price;

   price = ship_buyPrice(ship);

   /* Must have enough credits and the necessary license. */
   if ((!player_hasLicense(ship->license)) &&
         ((planet == NULL) || (!planet_hasService(planet, PLANET_SERVICE_BLACKMARKET)))) {
      land_errDialogueBuild( _("You lack the %s."), _(ship->license) );
      failure = 1;
   }
   if (!player_hasCredits( price )) {
      char buf[ECON_CRED_STRLEN];
      credits2str( buf, price - player.p->credits, 2 );
      land_errDialogueBuild( _("You need %s more."), buf);
      failure = 1;
   }
   return !failure;
}

/**
 * @brief Makes sure it's valid to sell a ship.
 *    @param shipname Ship being sold.
 */
int can_sell( const char *shipname )
{
   int failure = 0;
   if (strcmp( shipname, player.p->name )==0) { /* Already on-board. */
      land_errDialogueBuild( _("You can't sell the ship you're piloting!") );
      failure = 1;
   }

   return !failure;
}

/**
 * @brief Makes sure it's valid to change ships.
 *    @param shipname Ship being changed to.
 */
int can_swap( const char *shipname )
{
   int failure = 0;
   Ship* ship;
   ship = ship_get( shipname );
   double diff;

   if (pilot_cargoUsed(player.p) > ship->cap_cargo) { /* Current ship has too much cargo. */
      diff = pilot_cargoUsed(player.p) - ship->cap_cargo;
      land_errDialogueBuild( n_(
               "You have %g tonne more cargo than the new ship can hold.",
               "You have %g tonnes more cargo than the new ship can hold.",
               diff ),
            diff );
      failure = 1;
   }
   if (pilot_hasDeployed(player.p)) { /* Escorts are in space. */
      land_errDialogueBuild( _("You can't strand your fighters in space.") );
      failure = 1;
   }
   return !failure;
}


/**
 * @brief Makes sure it's valid to buy a ship, trading the old one in simultaneously.
 *    @param shipname Ship being bought.
 */
int shipyard_canTrade( const char *shipname )
{
   int failure = 0;
   Ship* ship;
   ship = ship_get( shipname );
   credits_t price;

   price = ship_buyPrice( ship );

   /* Must have the necessary license, enough credits, and be able to swap ships. */
   if (!player_hasLicense(ship->license)) {
      land_errDialogueBuild( _("You lack the %s."), _(ship->license) );
      failure = 1;
   }
   if (!player_hasCredits( price - player_shipPrice(player.p->name))) {
      credits_t creditdifference = price - (player_shipPrice(player.p->name) + player.p->credits);
      char buf[ECON_CRED_STRLEN];
      credits2str( buf, creditdifference, 2 );
      land_errDialogueBuild( _("You need %s more."), buf);
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
   int i;
   char buf[ECON_CRED_STRLEN], buf2[ECON_CRED_STRLEN],
         buf3[ECON_CRED_STRLEN], buf4[ECON_CRED_STRLEN];
   Ship* ship;

   i = toolkit_getImageArrayPos( wid, "iarShipyard" );
   if (i < 0 || shipyard_list == NULL)
      return;

   ship = shipyard_list[i];

   credits_t targetprice = ship_buyPrice(ship);
   credits_t playerprice = player_shipPrice(player.p->name);

   if (land_errDialogue( ship->name, "tradeShip" ))
      return;

   credits2str( buf, targetprice, 2 );
   credits2str( buf2, playerprice, 2 );
   credits2str( buf3, targetprice - playerprice, 2 );
   credits2str( buf4, playerprice - targetprice, 2 );

   /* Display the correct dialogue depending on the new ship's price versus the player's. */
   if ( targetprice == playerprice ) {
      if (dialogue_YesNo(_("Are you sure?"), /* confirm */
         _("Your %s is worth %s, exactly as much as the new ship, so no credits need be exchanged. Are you sure you want to trade your ship in?"),
               _(player.p->ship->name), buf2)==0)
         return;
   }
   else if ( targetprice < playerprice ) {
      if (dialogue_YesNo(_("Are you sure?"), /* confirm */
         _("Your %s is worth %s, more than the new ship. For your ship, you will get the new %s and %s. Are you sure you want to trade your ship in?"),
               _(player.p->ship->name), buf2, _(ship->name), buf4)==0)
         return;
   }
   else if ( targetprice > playerprice ) {
      if (dialogue_YesNo(_("Are you sure?"), /* confirm */
         _("Your %s is worth %s, so the new ship will cost %s. Are you sure you want to trade your ship in?"),
               _(player.p->ship->name), buf2, buf3)==0)
         return;
   }

   /* player just got a new ship */
   if (player_newShip( ship, NULL, 1, 0 ) == NULL)
      return; /* Player aborted the naming process. */

   player_modCredits( playerprice - targetprice ); /* Modify credits by the difference between ship values. */

   land_refuel();

   /* The newShip call will trigger a loadGUI that will recreate the land windows. Therefore the land ID will
    * be void. We must reload in in order to properly update it again.*/
   wid = land_getWid(LAND_WINDOW_SHIPYARD);

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
   y -= 10+5;
   gl_print( &gl_smallFont, bx, y, &cFontWhite, _("Slots:") );

   x = bx + 10.;
   w = bw - 10.;

   /* Weapon slots. */
   y -= 20;
   shipyard_renderSlotsRow( x, y, w, _(OUTFIT_LABEL_WEAPON), ship->outfit_weapon );

   /* Utility slots. */
   y -= 20;
   shipyard_renderSlotsRow( x, y, w, _(OUTFIT_LABEL_UTILITY), ship->outfit_utility );

   /* Structure slots. */
   y -= 20;
   shipyard_renderSlotsRow( x, y, w, _(OUTFIT_LABEL_STRUCTURE), ship->outfit_structure );
}


/**
 * @brief Renders a row of ship slots.
 */
static void shipyard_renderSlotsRow( double bx, double by, double bw, const char *str, ShipOutfitSlot *s )
{
   (void) bw;
   int i;
   double x;
   const glColour *c;

   x = bx;

   /* Print text. */
   gl_printMidRaw( &gl_smallFont, 30, bx-15, by, &cFontWhite, -1, str );

   /* Draw squares. */
   for (i=0; i<array_size(s); i++) {
      c = outfit_slotSizeColour( &s[i].slot );
      if (c == NULL)
         c = &cBlack;

      x += 15.;
      toolkit_drawRect( x, by, 10, 10, c, NULL );

      /* Add colour stripe depending on required/exclusiveness. */
      if (s[i].required)
         toolkit_drawTriangle( x, by, x+10, by+10, x, by+10, &cBrightRed );
      else if (s[i].exclusive)
         toolkit_drawTriangle( x, by, x+10, by+10, x, by+10, &cWhite );
      else if (s[i].slot.spid != 0)
         toolkit_drawTriangle( x, by, x+10, by+10, x, by+10, &cBlack );

      gl_renderRectEmpty( x, by, 10, 10, &cBlack );
   }
}
