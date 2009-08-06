/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file land.c
 *
 * @brief Handles all the landing menus and actionsv.
 */


#include "land.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "log.h"
#include "toolkit.h"
#include "dialogue.h"
#include "player.h"
#include "rng.h"
#include "music.h"
#include "economy.h"
#include "hook.h"
#include "mission.h"
#include "ntime.h"
#include "save.h"
#include "music.h"
#include "map.h"
#include "news.h"
#include "escort.h"
#include "event.h"
#include "tk/toolkit_priv.h" /* Yes, I'm a bad person, abstractions be damned! */


/* global/main window */
#define LAND_WIDTH   800 /**< Land window width. */
#define LAND_HEIGHT  600 /**< Land window height. */
#define BUTTON_WIDTH 200 /**< Default button width. */
#define BUTTON_HEIGHT 40 /**< Default button height. */
#define PORTRAIT_WIDTH 200
#define PORTRAIT_HEIGHT 150

/* news window */
#define NEWS_WIDTH   400 /**< News window width. */
#define NEWS_HEIGHT  500 /**< News window height. */


/*
 * we use visited flags to not duplicate missions generated
 */
#define VISITED_LAND          (1<<0) /**< Player already landed. */
#define VISITED_COMMODITY     (1<<1) /**< Player already visited commodities. */
#define VISITED_BAR           (1<<2) /**< Player already visited bar. */
#define VISITED_OUTFITS       (1<<3) /**< Player already visited outfits. */
#define VISITED_SHIPYARD      (1<<4) /**< Player already visited shipyard. */
#define visited(f)            (land_visited |= (f)) /**< Mark place is visited. */
#define has_visited(f)        (land_visited & (f)) /**< Check if player has visited. */
static unsigned int land_visited = 0; /**< Contains what the player visited. */


/*
 * The window interfaces.
 */
#define LAND_NUMWINDOWS          7 /**< Number of land windows. */
#define LAND_WINDOW_MAIN         0 /**< Main window. */
#define LAND_WINDOW_BAR          1 /**< Bar window. */
#define LAND_WINDOW_MISSION      2 /**< Mission computer window. */
#define LAND_WINDOW_OUTFITS      3 /**< Outfits window. */
#define LAND_WINDOW_SHIPYARD     4 /**< Shipyard window. */
#define LAND_WINDOW_EQUIPMENT    5 /**< Equipment window. */
#define LAND_WINDOW_COMMODITY    6 /**< Commodity window. */



/*
 * land variables
 */
int landed = 0; /**< Is player landed. */
static unsigned int land_wid = 0; /**< Land window ID. */
static const char *land_windowNames[LAND_NUMWINDOWS] = {
   "Landing Main",
   "Spaceport Bar",
   "Mission",
   "Outfits",
   "Shipyard",
   "Equipment",
   "Commodity"
};
static int land_windowsMap[LAND_NUMWINDOWS]; /**< Mapping of windows. */
static unsigned int *land_windows = NULL; /**< Landed window ids. */
Planet* land_planet = NULL; /**< Planet player landed at. */
static glTexture *gfx_exterior = NULL; /**< Exterior graphic of the landed planet. */
 
/*
 * mission computer stack
 */
static Mission* mission_computer = NULL; /**< Missions at the computer. */
static int mission_ncomputer = 0; /**< Number of missions at the computer. */

/*
 * mission bar stack
 */
static Mission* mission_bar = NULL; /**< Missions at the spaceport bar. */
static int mission_nbar = 0; /**< Number of missions at the spaceport bar. */
static glTexture *mission_portrait = NULL; /**< Mission portrait. */

/*
 * player stuff
 */
extern int hyperspace_target; /**< from player.c */
static int last_window = 0; /**< Default window. */

/*
 * equipment stuff
 */
static Pilot *equipment_selected = NULL; /**< Selected pilot ship. */
static int equipment_slot = -1; /**< Selected equipment slot. */


/*
 * prototypes
 */
static unsigned int land_getWid( int window );
static void land_createMainTab( unsigned int wid );
static void land_cleanupWindow( unsigned int wid, char *name );
static void land_buttonTakeoff( unsigned int wid, char *unused );
static void land_changeTab( unsigned int wid, char *wgt, int tab );
/* commodity exchange */
static void commodity_exchange_open( unsigned int wid );
static void commodity_update( unsigned int wid, char* str );
static void commodity_buy( unsigned int wid, char* str );
static void commodity_sell( unsigned int wid, char* str );
/* outfits */
static void outfits_open( unsigned int wid );
static void outfits_update( unsigned int wid, char* str );
static int outfit_canBuy( Outfit* outfit, int q, int errmsg );
static void outfits_buy( unsigned int wid, char* str );
static int outfit_canSell( Outfit* outfit, int q, int errmsg );
static void outfits_sell( unsigned int wid, char* str );
static int outfits_getMod (void);
static void outfits_renderMod( double bx, double by, double w, double h );
/* shipyard */
static void shipyard_open( unsigned int wid );
static void shipyard_update( unsigned int wid, char* str );
static void shipyard_info( unsigned int wid, char* str );
static void shipyard_buy( unsigned int wid, char* str );
/* equipment */
static void equipment_open( unsigned int wid );
static void equipment_renderColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, const char *txt, int selected );
static void equipment_render( double bx, double by, double bw, double bh );
static int equipment_mouseColumn( double y, double h, int n, double my );
static void equipment_mouse( unsigned int wid, SDL_Event* event,
      double x, double y, double w, double h );
static int equipment_swapSlot( unsigned int wid, PilotOutfitSlot *slot );
static void equipment_genLists( unsigned int wid );
static void equipment_update( unsigned int wid, char* str );
static void equipment_sellShip( unsigned int wid, char* str );
static void equipment_transChangeShip( unsigned int wid, char* str );
static void equipment_changeShip( unsigned int wid );
static void equipment_transportShip( unsigned int wid );
static void equipment_unequipShip( unsigned int wid, char* str );
static unsigned int equipment_transportPrice( char *shipname );
/* spaceport bar */
static void spaceport_bar_getDim( int wid,
      int *w, int *h, int *iw, int *ih, int *bw, int *bh );
static void spaceport_bar_open( unsigned int wid );
static int spaceport_bar_genList( unsigned int wid );
static void spaceport_bar_update( unsigned int wid, char* str );
static void spaceport_bar_close( unsigned int wid, char* str );
static void spaceport_bar_approach( unsigned int wid, char* str );
static int news_load (void);
/* mission computer */
static void misn_open( unsigned int wid );
static void misn_close( unsigned int wid, char *name );
static void misn_accept( unsigned int wid, char* str );
static void misn_genList( unsigned int wid, int first );
static void misn_update( unsigned int wid, char* str );
/* refuel */
static void land_checkAddRefuel (void);
static unsigned int refuel_price (void);
static void spaceport_refuel( unsigned int wid, char *str );
/* external */
extern unsigned int economy_getPrice( const Commodity *com,
      const StarSystem *sys, const Planet *p ); /**< from economy.c */


/**
 * @brief Opens the local market window.
 */
static void commodity_exchange_open( unsigned int wid )
{
   int i;
   char **goods;
   int w, h;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodityClose",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -40-((BUTTON_WIDTH-20)/2), 20*2 + BUTTON_HEIGHT,
         (BUTTON_WIDTH-20)/2, BUTTON_HEIGHT, "btnCommodityBuy",
         "Buy", commodity_buy );
   window_addButton( wid, -20, 20*2 + BUTTON_HEIGHT,
         (BUTTON_WIDTH-20)/2, BUTTON_HEIGHT, "btnCommoditySell",
         "Sell", commodity_sell );

   /* text */
   window_addText( wid, -20, -40, BUTTON_WIDTH, 60, 0,
         "txtSInfo", &gl_smallFont, &cDConsole, 
         "You have:\n"
         "Market price:\n"
         "\n"
         "Free Space:\n" );
   window_addText( wid, -20, -40, BUTTON_WIDTH/2, 60, 0,
         "txtDInfo", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, -40, -120, BUTTON_WIDTH-20,
         h-140-BUTTON_HEIGHT, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   /* goods list */
   goods = malloc(sizeof(char*) * land_planet->ncommodities);
   for (i=0; i<land_planet->ncommodities; i++)
      goods[i] = strdup(land_planet->commodities[i]->name);
   window_addList( wid, 20, -40,
         w-BUTTON_WIDTH-60, h-80-BUTTON_HEIGHT,
         "lstGoods", goods, land_planet->ncommodities, 0, commodity_update );

   /* update */
   commodity_update(wid, NULL);
}
/**
 * @brief Updates the commodity window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
static void commodity_update( unsigned int wid, char* str )
{
   (void)str;
   char buf[PATH_MAX];
   char *comname;
   Commodity *com;

   comname = toolkit_getList( wid, "lstGoods" );
   com = commodity_get( comname );

   /* modify text */
   snprintf( buf, PATH_MAX,
         "%d tons\n"
         "%d credits/ton\n"
         "\n"
         "%d tons\n",
         player_cargoOwned( comname ),
         economy_getPrice(com, cur_system, land_planet),
         pilot_cargoFree(player));
   window_modifyText( wid, "txtDInfo", buf );
   window_modifyText( wid, "txtDesc", com->description );
}
/**
 * @brief Buys the selected commodity.
 *    @param wid Window buying from.
 *    @param str Unused.
 */
static void commodity_buy( unsigned int wid, char* str )
{
   (void)str;
   char *comname;
   Commodity *com;
   unsigned int q, price;

   q = 10;
   comname = toolkit_getList( wid, "lstGoods" );
   com = commodity_get( comname );
   price = economy_getPrice(com, cur_system, land_planet);

   if (player->credits < q * price) {
      dialogue_alert( "Not enough credits!" );
      return;
   }
   else if (pilot_cargoFree(player) <= 0) {
      dialogue_alert( "Not enough free space!" );
      return;
   }

   q = pilot_addCargo( player, com, q );
   player->credits -= q * price;
   land_checkAddRefuel();
   commodity_update(wid, NULL);
}
/**
 * @brief Attempts to sell a commodity.
 *    @param wid Window selling commodity from.
 *    @param str Unused.
 */
static void commodity_sell( unsigned int wid, char* str )
{
   (void)str;
   char *comname;
   Commodity *com;
   unsigned int q, price;

   q = 10;
   comname = toolkit_getList( wid, "lstGoods" );
   com = commodity_get( comname );
   price = economy_getPrice(com, cur_system, land_planet);

   q = pilot_rmCargo( player, com, q );
   player->credits += q * price;
   land_checkAddRefuel();
   commodity_update(wid, NULL);
}


/**
 * @brief Opens the outfit exchange center window.
 */
static void outfits_open( unsigned int wid )
{
   int i;
   Outfit **outfits;
   char **soutfits;
   glTexture **toutfits;
   int noutfits;
   int w, h;
   int iw, ih;
   int bw, bh;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   iw = 310;
   ih = h - 60;

   /* Calculate button dimensions. */
   bw = (w - iw - 80) / 2;
   bh = BUTTON_HEIGHT;

   /* will allow buying from keyboard */
   window_setAccept( wid, outfits_buy );

   /* buttons */
   window_addButton( wid, -20, 20,
         bw, bh, "btnCloseOutfits",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -40-bw, 40+bh,
         bw, bh, "btnBuyOutfit",
         "Buy", outfits_buy );
   window_addButton( wid, -40-bw, 20,
         bw, bh, "btnSellOutfit",
         "Sell", outfits_sell );

   /* fancy 128x128 image */
   window_addRect( wid, -20, -50, 128, 128, "rctImage", &cBlack, 0 );
   window_addImage( wid, -20-128, -50-128, "imgOutfit", NULL, 1 );

   /* cust draws the modifier */
   window_addCust( wid, -40-bw, 60+2*bh,
         bw, bh, "cstMod", 0, outfits_renderMod, NULL );

   /* the descriptive text */
   window_addText( wid, 40+300+20, -60,
         80, 160, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Name:\n"
         "Type:\n"
         "Owned:\n"
         "\n"
         "Mass:\n"
         "\n"
         "Price:\n"
         "Money:\n"
         "License:\n" );
   window_addText( wid, 40+300+40+60, -60,
         250, 160, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 20+300+40, -240,
         w-400, 180, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );

   /* set up the outfits to buy/sell */
   outfits = outfit_getTech( &noutfits, land_planet->tech, PLANET_TECH_MAX);
   if (noutfits <= 0) { /* No outfits */
      soutfits    = malloc(sizeof(char*));
      soutfits[0] = strdup("None");
      toutfits    = malloc(sizeof(glTexture*));
      toutfits[0] = NULL;
      noutfits    = 1;
   }
   else {
      /* Create the outfit arrays. */
      soutfits = malloc(sizeof(char*)*noutfits);
      toutfits = malloc(sizeof(glTexture*)*noutfits);
      for (i=0; i<noutfits; i++) {
         soutfits[i] = strdup(outfits[i]->name);
         toutfits[i] = outfits[i]->gfx_store;
      }
      free(outfits);
   }
   window_addImageArray( wid, 20, 20,
         iw, ih, "iarOutfits", 64, 64,
         toutfits, soutfits, noutfits, outfits_update );

   /* write the outfits stuff */
   outfits_update( wid, NULL );
}
/**
 * @brief Updates the outfits in the outfit window.
 *    @param wid Window to update the outfits in.
 *    @param str Unused.
 */
static void outfits_update( unsigned int wid, char* str )
{
   (void)str;
   char *outfitname;
   Outfit* outfit;
   char buf[PATH_MAX], buf2[16], buf3[16];

   outfitname = toolkit_getImageArray( wid, "iarOutfits" );
   if (strcmp(outfitname,"None")==0) { /* No outfits */
      window_modifyImage( wid, "imgOutfit", NULL );
      window_disableButton( wid, "btnBuyOutfit" );
      window_disableButton( wid, "btnSellOutfit" );
      snprintf( buf, PATH_MAX,
            "None\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "\n"
            "NA\n"
            "NA\n"
            "NA\n" );
      window_modifyText( wid,  "txtDDesc", buf );
      return;
   }

   outfit = outfit_get( outfitname );

   /* new image */
   window_modifyImage( wid, "imgOutfit", outfit->gfx_store );

   if (outfit_canBuy(outfit,1,0) > 0)
      window_enableButton( wid, "btnBuyOutfit" );
   else
      window_disableButton( wid, "btnBuyOutfit" );

   /* gray out sell button */
   if (outfit_canSell(outfit,1,0) > 0)
      window_enableButton( wid, "btnSellOutfit" );
   else
      window_disableButton( wid, "btnSellOutfit" );

   /* new text */
   window_modifyText( wid, "txtDescription", outfit->description );
   credits2str( buf2, outfit->price, 2 );
   credits2str( buf3, player->credits, 2 );
   snprintf( buf, PATH_MAX,
         "%s\n"
         "%s\n"
         "%d\n"
         "\n"
         "%.0f tons\n"
         "\n"
         "%s credits\n"
         "%s credits\n"
         "%s\n",
         outfit->name,
         outfit_getType(outfit),
         (outfit_isLicense(outfit)) ?
               player_hasLicense(outfit->name) :
               player_outfitOwned(outfit),
         outfit->mass,
         buf2,
         buf3,
         (outfit->license != NULL) ? outfit->license : "None" );
   window_modifyText( wid,  "txtDDesc", buf );
}
/**
 * @brief Checks to see if the player can buy the outfit.
 *    @param outfit Outfit to buy.
 *    @param q Quantity to buy.
 *    @param errmsg Should alert the player?
 */
static int outfit_canBuy( Outfit* outfit, int q, int errmsg )
{
   char buf[16];

   /* can only have one afterburner */
   if (outfit_isAfterburner(outfit) && (player->afterburner!=NULL)) {
      if (errmsg != 0)
         dialogue_alert( "You can only have one afterburner." );
      return 0;
   }
   /* takes away cargo space but you don't have any */
   else if (outfit_isMod(outfit) && (outfit->u.mod.cargo < 0)
         && (pilot_cargoFree(player) < -outfit->u.mod.cargo)) {
      if (errmsg != 0)
         dialogue_alert( "You need to empty your cargo first." );
      return 0;
   }
   /* not enough $$ */
   else if (q*outfit->price > player->credits) {
      if (errmsg != 0) {
         credits2str( buf, q*outfit->price - player->credits, 2 );
         dialogue_alert( "You need %s more credits.", buf);
      }
      return 0;
   }
   /* Map already mapped */
   else if (outfit_isMap(outfit) && map_isMapped(NULL,outfit->u.map.radius)) {
      if (errmsg != 0)
         dialogue_alert( "You already own this map." );
      return 0;
   }
   /* Already has license. */
   else if (outfit_isLicense(outfit) && player_hasLicense(outfit->name)) {
      if (errmsg != 0)
         dialogue_alert( "You already have this license." );
      return 0;
   }
   /* Needs license. */
   else if ((outfit->license != NULL) && !player_hasLicense(outfit->license)) {
      if (errmsg != 0)
         dialogue_alert( "You need the '%s' license to buy this outfit.",
               outfit->license );
      return 0;
   }

   return 1;
}
/**
 * @brief Attempts to buy the outfit that is selected.
 *    @param wid Window buying outfit from.
 *    @param str Unused.
 */
static void outfits_buy( unsigned int wid, char* str )
{
   (void) str;
   char *outfitname;
   Outfit* outfit;
   int q;

   outfitname = toolkit_getImageArray( wid, "iarOutfits" );
   outfit = outfit_get( outfitname );

   q = outfits_getMod();

   /* can buy the outfit? */
   if (outfit_canBuy(outfit, q, 1) == 0)
      return;

   player->credits -= outfit->price * player_addOutfit( outfit, q );
   land_checkAddRefuel();
   outfits_update(wid, NULL);
}
/**
 * @brief Checks to see if the player can sell the selected outfit.
 *    @param outfit Outfit to try to sell.
 *    @param q Quantity to try to sell.
 *    @param errmsg Should alert player?
 */
static int outfit_canSell( Outfit* outfit, int q, int errmsg )
{
   (void) q;
   /* has no outfits to sell */
   if (player_outfitOwned(outfit) <= 0) {
      if (errmsg != 0)
         dialogue_alert( "You can't sell something you don't have." );
      return 0;
   }

   return 1;
}
/**
 * @brief Attempts to sell the selected outfit the player has.
 *    @param wid Window selling outfits from.
 *    @param str Unused.
 */
static void outfits_sell( unsigned int wid, char* str )
{
   (void)str;
   char *outfitname;
   Outfit* outfit;
   int q;

   outfitname = toolkit_getImageArray( wid, "iarOutfits" );
   outfit = outfit_get( outfitname );

   q = outfits_getMod();

   /* has no outfits to sell */
   if (outfit_canSell( outfit, q, 1 ) == 0)
      return;

   player->credits += outfit->price * player_rmOutfit( outfit, q );
   land_checkAddRefuel();
   outfits_update(wid, NULL);
}
/**
 * @brief Gets the current modifier status.
 *    @return The amount modifier when buying or selling outfits.
 */
static int outfits_getMod (void)
{
   SDLMod mods;
   int q;

   mods = SDL_GetModState();
   q = 1;
   if (mods & (KMOD_LCTRL | KMOD_RCTRL))
      q *= 5;
   if (mods & (KMOD_LSHIFT | KMOD_RSHIFT))
      q *= 10;

   return q;
}
/**
 * @brief Renders the outfit buying modifier.
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width to render at.
 *    @param h Height to render at.
 */
static void outfits_renderMod( double bx, double by, double w, double h )
{
   (void) h;
   int q;
   char buf[8];

   q = outfits_getMod();
   if (q==1) return; /* Ignore no modifier. */

   snprintf( buf, 8, "%dx", q );
   gl_printMid( &gl_smallFont, w,
         bx + (double)SCREEN_W/2.,
         by + (double)SCREEN_H/2.,
         &cBlack, buf );
}




/**
 * @brief Opens the shipyard window.
 */
static void shipyard_open( unsigned int wid )
{
   int i;
   Ship **ships;
   char **sships;
   glTexture **tships;
   int nships;
   int w, h;
   int iw, ih;
   int bw, bh;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   iw = 310;
   ih = h - 60;

   /* Calculate button dimensions. */
   bw = (w - iw - 80) / 2;
   bh = BUTTON_HEIGHT;

   /* buttons */
   window_addButton( wid, -20, 20,
         bw, bh, "btnCloseShipyard",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -40-bw, 20,
         bw, bh, "btnBuyShip",
         "Buy", shipyard_buy );
   window_addButton( wid, -40-bw, 40+bh,
         bw, bh, "btnInfoShip",
         "Info", shipyard_info );

   /* target gfx */
   window_addRect( wid, -40, -50,
         128, 96, "rctTarget", &cBlack, 0 );
   window_addImage( wid, -40-128, -50-96,
         "imgTarget", NULL, 1 );

   /* text */
   window_addText( wid, 40+iw+20, -55,
         80, 128, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Name:\n"
         "Class:\n"
         "Fabricator:\n"
         "\n"
         "Price:\n"
         "Money:\n"
         "License:\n" );
   window_addText( wid, 40+iw+20+80, -55,
         130, 128, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 20+310+40, -55-128-20,
         w-(20+310+40) - 20, 185, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );

   /* set up the ships to buy/sell */
   ships = ship_getTech( &nships, land_planet->tech, PLANET_TECH_MAX );
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
         tships[i] = ships[i]->gfx_target;
      }
      free(ships);
   }
   window_addImageArray( wid, 20, 20,
         iw, ih, "iarShipyard", 64./96.*128., 64.,
         tships, sships, nships, shipyard_update );

   /* write the shipyard stuff */
   shipyard_update(wid, NULL);
}
/**
 * @brief Updates the ships in the shipyard window.
 *    @param wid Window to update the ships in.
 *    @param str Unused.
 */
static void shipyard_update( unsigned int wid, char* str )
{
   (void)str;
   char *shipname;
   Ship* ship;
   char buf[PATH_MAX], buf2[16], buf3[16];
   
   shipname = toolkit_getImageArray( wid, "iarShipyard" );

   /* No ships */
   if (strcmp(shipname,"None")==0) {
      window_modifyImage( wid, "imgTarget", NULL );
      window_disableButton( wid, "btnBuyShip");
      window_disableButton( wid, "btnInfoShip");
      snprintf( buf, PATH_MAX,
            "None\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "NA\n"
            "NA\n" );
      window_modifyText( wid,  "txtDDesc", buf );
      return;
   }

   ship = ship_get( shipname );

   /* update image */
   window_modifyImage( wid, "imgTarget", ship->gfx_target );

   /* update text */
   window_modifyText( wid, "txtDescription", ship->description );
   credits2str( buf2, ship->price, 2 );
   credits2str( buf3, player->credits, 2 );
   snprintf( buf, PATH_MAX,
         "%s\n"
         "%s\n"
         "%s\n"
         "\n"
         "%s credits\n"
         "%s credits\n"
         "%s\n",
         ship->name,
         ship_class(ship),
         ship->fabricator,
         buf2,
         buf3,
         (ship->license != NULL) ? ship->license : "None" );
   window_modifyText( wid,  "txtDDesc", buf );

   if (ship->price > player->credits)
      window_disableButton( wid, "btnBuyShip");
   else window_enableButton( wid, "btnBuyShip");
}
/**
 * @brief Opens the ship's information window.
 *    @param wid Window to find out selected ship.
 *    @param str Unused.
 */
static void shipyard_info( unsigned int wid, char* str )
{
   (void)str;
   char *shipname;

   shipname = toolkit_getImageArray( wid, "iarShipyard" );
   ship_view(0, shipname);
}
/**
 * @brief Player attempts to buy a ship.
 *    @param wid Window player is buying ship from.
 *    @param str Unused.
 */
static void shipyard_buy( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[16];
   Ship* ship;

   shipname = toolkit_getImageArray( wid, "iarShipyard" );
   ship = ship_get( shipname );

   /* Must have enough money. */
   if (ship->price > player->credits) {
      dialogue_alert( "Not enough credits!" );
      return;
   }
   else if (pilot_hasDeployed(player)) {
      dialogue_alert( "You can't leave your fighters stranded. Recall them before buying a new ship." );
      return;
   }

   /* Must have license. */
   if ((ship->license != NULL) && !player_hasLicense(ship->license)) {
      dialogue_alert( "You do not have the '%s' license required to buy this ship.",
            ship->license);
      return;
   }

   /* we now move cargo to the new ship */
   if (pilot_cargoUsed(player) > ship->cap_cargo) {
      dialogue_alert("You won't have enough space to move your current cargo into the new ship.");
      return; 
   }
   credits2str( buf, ship->price, 2 );
   if (dialogue_YesNo("Are you sure?", /* confirm */
         "Do you really want to spend %s on a new ship?", buf )==0)
      return;

   /* player just gots a new ship */
   if (player_newShip( ship, player->solid->pos.x, player->solid->pos.y,
         0., 0., player->solid->dir ) != 0) {
      /* Player actually aborted naming process. */
      return;
   }
   player->credits -= ship->price; /* ouch, paying is hard */
   land_checkAddRefuel();

   shipyard_update(wid, NULL);
}


/**
 * @brief Opens the player's equipment window.
 */
static void equipment_open( unsigned int wid )
{
   int w, h;
   int sw, sh;
   int ow, oh;
   int bw, bh;
   int cw, ch;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   sw = 200;
   sh = (h - 100)/2;
   ow = sw;
   oh = sh;

   /* Calculate custom widget. */
   cw = w - 20 - sw - 20;
   ch = h - 100;

   /* Calculate button dimensions. */
   bw = (w - 20 - sw - 40 - 20 - 60) / 4;
   bh = BUTTON_HEIGHT;

   /* Sane defaults. */
   equipment_selected = NULL;
   equipment_slot = -1;

   /* buttons */
   window_addButton( wid, -20, 20,
         bw, bh, "btnCloseEquipment",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -20 - (20+bw), 20,
         bw, bh, "btnSellShip",
         "Sell Ship", equipment_sellShip );
   window_addButton( wid, -20 - (20+bw)*2, 20,
         bw, bh, "btnChangeShip",
         "Swap Ships", equipment_transChangeShip );
   window_addButton( wid, -20 - (20+bw)*3, 20,
         bw, bh, "btnUnequipShip",
         "Unequip", equipment_unequipShip );

   /* image */
   window_addRect( wid, -30, -50,
         128, 96, "rctTarget", &cBlack, 0 );
   window_addImage( wid, -30-128, -50-96,
         "imgTarget", NULL, 1 );

   /* text */
   window_addText( wid, 20 + sw + 20 + 180 + 20 + 30, -230,
         100, 200, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Name:\n"
         "Ship:\n"
         "Class:\n"
         "Where:\n"
         "\n"
         "Cargo free:\n"
         "\n"
         "Transportation:\n"
         "Sell price:\n"
         );
   window_addText( wid, 20 + sw + 20 + 180 + 20 + 130, -230,
      130, 200, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );

   /* Generate lists. */
   window_addText( wid, 30, -20,
         130, 200, 0, "txtShipTitle", &gl_smallFont, &cBlack, "Available Ships" );
   window_addText( wid, 30, -40-sw-40-20,
         130, 200, 0, "txtOutfitTitle", &gl_smallFont, &cBlack, "Available Outfits" );
   equipment_genLists( wid );

   /* Seperator. */
   window_addRect( wid, 20 + sw + 20, -40, 2, h-60, "rctDivider", &cBlack, 0 );

   /* Custom widget. */
   window_addCust( wid, 20 + sw + 40, -40, cw, ch, "cstEquipment", 0,
         equipment_render, equipment_mouse );
}
/**
 * @brief Renders an outfit column.
 */
static void equipment_renderColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, const char *txt, int selected )
{
   int i;
   glColour *lc, *c, *dc;

   /* Render text. */
   gl_printMidRaw( &gl_smallFont, w,
         x + SCREEN_W/2., y+h+10. + SCREEN_H/2.,
         &cBlack, txt );

   for (i=0; i<n; i++) {
      if (lst[i].outfit != NULL) {
         /* Draw background. */
         toolkit_drawRect( x, y, w, h,
               (i==selected) ? &cDConsole : &cBlack, NULL );
         /* Draw bugger. */
         gl_blitScale( lst[i].outfit->gfx_store,
               x + SCREEN_W/2., y + SCREEN_H/2., w, h, NULL );
      }
      else {
         gl_printMidRaw( &gl_smallFont, w,
               x + SCREEN_W/2., y + (h-gl_smallFont.h)/2 + SCREEN_H/2.,
               (i==selected) ? &cDConsole : &cBlack, "None" );
      }
      /* Draw outline. */
      if (i==selected) {
         lc = &cWhite;
         c = &cGrey80;
         dc = &cGrey60;
      }
      else {
         lc = toolkit_colLight;
         c = toolkit_col;
         dc = toolkit_colDark;
      }
      toolkit_drawOutline( x - 2., y-2., w+4., h+4., 1., lc, c  );
      toolkit_drawOutline( x - 2., y-2., w+4., h+4., 2., dc, NULL  );
      /* Go to next one. */
      y -= h+20;
   }
}
/**
 * @brief Renders the custom equipment widget.
 */
static void equipment_render( double bx, double by, double bw, double bh )
{
   Pilot *p;
   int m, selected;
   double percent;
   double x, y;
   double w, h;
   glColour *lc, *c, *dc;

   /* Must have selected ship. */
   if (equipment_selected == NULL)
      return;

   /* Calculate height of outfit widgets. */
   p = equipment_selected;
   m = MAX( MAX( p->outfit_nhigh, p->outfit_nmedium ), p->outfit_nlow );
   h = (bh-30.)/m;
   if (h > 40)
      h = 40;
   w = h;

   /* Get selected. */
   selected = equipment_slot;

   /* Render high outfits. */
   x = bx + 10 + (40-w)/2;
   y = by + bh - 60 + (40-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nhigh, p->outfit_high, "High", selected );
   selected -= p->outfit_nhigh;
   x = bx + 10 + (40-w)/2 + 60;
   y = by + bh - 60 + (40-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nmedium, p->outfit_medium, "Medium", selected );
   selected -= p->outfit_nmedium;
   x = bx + 10 + (40-w)/2 + 120;
   y = by + bh - 60 + (40-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nlow, p->outfit_low, "Low", selected );

   /* Render CPU and energy bars. */
   lc = &cWhite;
   c = &cGrey80;
   dc = &cGrey60;
   w = 30;
   h = 100;
   x = bx + 10 + (40-w)/2 + 180 + 30;
   y = by + bh - 30 - h;
   percent = (p->cpu_max > 0.) ? p->cpu / p->cpu_max : 0.;
   gl_printMidRaw( &gl_smallFont, w,
         x + SCREEN_W/2., y + h + gl_smallFont.h + 10. + SCREEN_H/2.,
         &cBlack, "CPU" );
   toolkit_drawRect( x, y, w, h*percent, &cGreen, NULL );
   toolkit_drawRect( x, y+h*percent, w, h*(1.-percent), &cRed, NULL );
   toolkit_drawOutline( x, y, w, h, 1., lc, c  );
   toolkit_drawOutline( x, y, w, h, 2., dc, NULL  );
   gl_printMid( &gl_smallFont, 70,
         x - 20 + SCREEN_W/2., y - 20 - gl_smallFont.h + SCREEN_H/2.,
         &cBlack, "%.1f / %.1f", p->cpu, p->cpu_max );
}
/**
 * @brief Handles a mouse press in column.
 */
static int equipment_mouseColumn( double y, double h, int n, double my )
{
   int i;

   for (i=0; i<n; i++) {
      if ((my > y) && (my < y+h+20))
         return i;
      y -= h+20;
   }

   return -1;
}
/**
 * @brief Does mouse input for the custom equipment widget.
 */
static void equipment_mouse( unsigned int wid, SDL_Event* event,
      double mx, double my, double bw, double bh )
{
   Pilot *p;
   int m, selected, ret;
   double x, y;
   double w, h;

   /* Must have selected ship. */
   if (equipment_selected == NULL)
      return;

   /* Must be left click for now. */
   if (event->type != SDL_MOUSEBUTTONDOWN)
      return;

   /* Calculate height of outfit widgets. */
   p = equipment_selected;
   m = MAX( MAX( p->outfit_nhigh, p->outfit_nmedium ), p->outfit_nlow );
   h = (bh-30.)/m;
   if (h > 40)
      h = 40;
   w = h;

   /* Render high outfits. */
   selected = 0;
   x = 10 + (40-w)/2;
   if ((mx > x-10) && (mx < x+w+10)) {
      y = bh - 60 + (40-h)/2;
      ret = equipment_mouseColumn( y, h, p->outfit_nhigh, my );
      if (ret >= 0) {
         if (event->button.button == SDL_BUTTON_LEFT)
            equipment_slot = selected + ret;
         else
            equipment_swapSlot( wid, &p->outfit_high[ret] );
         return;
      }
   }
   selected += p->outfit_nhigh;
   x = 10 + (40-w)/2 + 60;
   if ((mx > x-10) && (mx < x+w+10)) {
      y = bh - 60 + (40-h)/2;
      ret = equipment_mouseColumn( y, h, p->outfit_nmedium, my );
      if (ret >= 0) {
         if (event->button.button == SDL_BUTTON_LEFT)
            equipment_slot = selected + ret;
         else
            equipment_swapSlot( wid, &p->outfit_medium[ret] );
         return;
      }
   }
   selected += p->outfit_nmedium;
   x = 10 + (40-w)/2 + 120;
   if ((mx > x-10) && (mx < x+w+10)) {
      y = bh - 60 + (40-h)/2;
      ret = equipment_mouseColumn( y, h, p->outfit_nlow, my );
      if (ret >= 0) {
         if (event->button.button == SDL_BUTTON_LEFT)
            equipment_slot = selected + ret;
         else
            equipment_swapSlot( wid, &p->outfit_low[ret] );
         return;
      }
   }
}
/**
 * @brief Swaps an equipment slot.
 */
static int equipment_swapSlot( unsigned int wid, PilotOutfitSlot *slot )
{
   int ret;
   const char *oname;
   Outfit *o;


   /* Remove outfit. */
   if (slot->outfit != NULL) {
      o = slot->outfit;
      ret = pilot_rmOutfit( player, slot );
      if (ret == 0)
         player_addOutfit( o, 1 );
   }
   /* Add outfit. */
   else {
      /* Must have outfit. */
      oname = toolkit_getImageArray( wid, "iarAvailOutfits" );
      if (strcmp(oname,"None")==0) {
         return 0;
      }
      o = outfit_get(oname);

      /* Must fit slot. */
      if (o->slot != slot->slot)
         return 0;

      /* Remove outfit. */
      ret = player_rmOutfit( o, 1 );
      if (ret == 1)
         pilot_addOutfit( player, o, slot );
   }

   /* Redo the outfits thingy. */
   window_destroyWidget( wid, "iarAvailOutfits" );
   equipment_genLists( wid );
   return 0;
}
/**
 * @brief Generates a new ship/outfit lists if needed.
 */
static void equipment_genLists( unsigned int wid )
{
   char **sships;
   glTexture **tships;
   int nships;
   char **soutfits;
   glTexture **toutfits;
   int noutfits;
   int w, h;
   int sw, sh;
   int ow, oh;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   sw = 200;
   sh = (h - 100)/2;
   ow = sw;
   oh = sh;

   /* Ship list. */
   if (!widget_exists( wid, "iarAvailShips" )) {
      nships   = player_nships()+1;
      sships   = malloc(sizeof(char*)*nships);
      tships   = malloc(sizeof(glTexture*)*nships);
      /* Add player's current ship. */
      sships[0] = strdup(player->name);
      tships[0] = player->ship->gfx_target;
      player_ships( &sships[1], &tships[1] );
      window_addImageArray( wid, 20, -40,
            sw, sh, "iarAvailShips", 64./96.*128., 64.,
            tships, sships, nships, equipment_update );
   }

   /* Outfit list. */
   if (!widget_exists( wid ,"iarAvailOutfits" )) {
      noutfits = MAX(1,player_numOutfits());
      soutfits = malloc(sizeof(char*)*noutfits);
      toutfits = malloc(sizeof(glTexture*)*noutfits);
      player_getOutfits( soutfits, toutfits );
      window_addImageArray( wid, 20, -40 - sh - 40,
            sw, sh, "iarAvailOutfits", 50., 50.,
            toutfits, soutfits, noutfits, NULL );
   }

   /* Update window. */
   equipment_update(wid, NULL);
}
/**
 * @brief Updates the player's ship window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
static void equipment_update( unsigned int wid, char* str )
{
   (void)str;
   char buf[256], buf2[16], buf3[16];
   char *shipname;
   Pilot *ship;
   char* loc;
   unsigned int price;

   shipname = toolkit_getImageArray( wid, "iarAvailShips" );
   if (strcmp(shipname,player->name)==0) { /* no ships */
      window_disableButton( wid, "btnSellShip" );
      window_disableButton( wid, "btnChangeShip" );
      ship = player;
      loc = "Onboard";
      price = 0;
   }
   else {
      window_enableButton( wid, "btnSellShip" );
      window_enableButton( wid, "btnChangeShip" );
      ship  = player_getShip( shipname );
      loc   = player_getLoc(ship->name);
      price = equipment_transportPrice( shipname );
   }
   equipment_selected = ship;

   /* update image */
   window_modifyImage( wid, "imgTarget", ship->ship->gfx_target );

   /* update text */
   credits2str( buf2, price , 2 ); /* transport */
   credits2str( buf3, player_shipPrice(shipname), 2 ); /* sell price */
   snprintf( buf, 256,
         "%s\n"
         "%s\n"
         "%s\n"
         "%s\n"
         "\n"
         "%d tons\n"
         "\n"
         "%s credits\n"
         "%s credits\n",
         ship->name,
         ship->ship->name,
         ship_class(ship->ship),
         loc,
         pilot_cargoFree(ship),
         buf2,
         buf3);
   window_modifyText( wid, "txtDDesc", buf );

   /* button disabling */
   if (strcmp(land_planet->name,loc)) { /* ship not here */
      window_buttonCaption( wid, "btnChangeShip", "Transport" );
      if (price > player->credits)
         window_disableButton( wid, "btnChangeShip" );
      else
         window_enableButton( wid, "btnChangeShip" );
   }
   else { /* ship is here */
      window_buttonCaption( wid, "btnChangeShip", "Swap Ship" );
      window_enableButton( wid, "btnChangeShip" );
   }
   /* If ship is there you can always sell. */
   window_enableButton( wid, "btnSellShip" );
}
/**
 * @brief Changes or transport depending on what is active.
 *    @param wid Window player is attempting to change ships in.
 *    @param str Unused.
 */
static void equipment_transChangeShip( unsigned int wid, char* str )
{
   (void) str;
   char *shipname;
   Pilot *ship;
   char* loc;

   shipname = toolkit_getImageArray( wid, "iarAvailShips" );
   if (strcmp(shipname,"None")==0) /* no ships */
      return;
   ship  = player_getShip( shipname );
   loc   = player_getLoc( ship->name );

   if (strcmp(land_planet->name,loc)) /* ship not here */
      equipment_transportShip( wid );
   else
      equipment_changeShip( wid );

   /* update the window to reflect the change */
   equipment_update( wid, NULL );
}
/**
 * @brief Player attempts to change ship.
 *    @param wid Window player is attempting to change ships in.
 */
static void equipment_changeShip( unsigned int wid )
{
   char *shipname, *loc;
   Pilot *newship;

   shipname = toolkit_getImageArray( wid, "iarAvailShips" );
   newship = player_getShip(shipname);
   if (strcmp(shipname,"None")==0) { /* no ships */
      dialogue_alert( "You need another ship to change ships!" );
      return;
   }
   loc = player_getLoc(shipname);

   if (strcmp(loc,land_planet->name)) {
      dialogue_alert( "You must transport the ship to %s to be able to get in.",
            land_planet->name );
      return;
   }
   else if (pilot_cargoUsed(player) > pilot_cargoFree(newship)) {
      dialogue_alert( "You won't be able to fit your current cargo in the new ship." );
      return;
   }
   else if (pilot_hasDeployed(player)) {
      dialogue_alert( "You can't leave your fighters stranded. Recall them before changing ships." );
      return;
   }

   /* Swap ship. */
   player_swapShip(shipname);

   /* Destroy widget. */
   window_destroyWidget( wid, "iarAvailShips" );
   equipment_genLists( wid );
}
/**
 * @brief Player attempts to transport his ship to the planet he is at.
 *    @param wid Window player is trying to transport his ship from.
 */
static void equipment_transportShip( unsigned int wid )
{
   unsigned int price;
   char *shipname, buf[16];

   shipname = toolkit_getImageArray( wid, "iarAvailShips" );
   if (strcmp(shipname,"None")==0) { /* no ships */
      dialogue_alert( "You can't transport nothing here!" );
      return;
   }

   price = equipment_transportPrice( shipname );
   if (price==0) { /* already here */
      dialogue_alert( "Your ship '%s' is already here.", shipname );
      return;
   }
   else if (player->credits < price) { /* not enough money */
      credits2str( buf, price-player->credits, 2 );
      dialogue_alert( "You need %d more credits to transport '%s' here.",
            buf, shipname );
      return;
   }

   /* success */
   player->credits -= price;
   land_checkAddRefuel();
   player_setLoc( shipname, land_planet->name );
}
/**
 * @brief Unequips the player's ship.
 */
static void equipment_unequipShip( unsigned int wid, char* str )
{
   (void) str;
   int ret;
   int i;
   char *shipname;
   Pilot *ship;
   Outfit *o;

   shipname = toolkit_getImageArray( wid, "iarAvailShips" );
   if (strcmp(shipname,"None")==0) /* no ships */
      return;
   ship  = player_getShip( shipname );

   /* Remove all outfits. */
   for (i=0; i<ship->noutfits; i++) {
      o = ship->outfits[i]->outfit;
      ret = pilot_rmOutfit( ship, ship->outfits[i] );
      if (ret==0)
         player_addOutfit( o, 1 );
   }
}
/**
 * @brief Player tries to sell a ship.
 *    @param wid Window player is selling ships in.
 *    @param str Unused.
 */
static void equipment_sellShip( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[16];
   int price;

   shipname = toolkit_getImageArray( wid, "iarAvailShips" );
   if (strcmp(shipname,"None")==0) { /* no ships */
      dialogue_alert( "You can't sell nothing!" );
      return;
   }

   /* Calculate price. */
   price = player_shipPrice(shipname);
   credits2str( buf, price, 2 );

   /* Check if player really wants to sell. */
   if (!dialogue_YesNo( "Sell Ship",
         "Are you sure you want to sell your ship %s for %s credits?", shipname, buf)) {
      return;
   }

   /* Sold. */
   player->credits += price;
   land_checkAddRefuel();
   player_rmShip( shipname );
   dialogue_msg( "Ship Sold", "You have sold your ship %s for %s credits.",
         shipname, buf );

   /* Destroy widget. */
   window_destroyWidget( wid, "iarAvailShips" );
   equipment_genLists( wid );
}
/**
 * @brief Gets the ship's transport price.
 *    @param shipname Name of the ship to get the transport price.
 *    @return The price to transport the ship to the current planet.
 */
static unsigned int equipment_transportPrice( char* shipname )
{
   char *loc;
   Pilot* ship;
   unsigned int price;

   ship = player_getShip(shipname);
   loc = player_getLoc(shipname);
   if (strcmp(loc,land_planet->name)==0) /* already here */
      return 0;

   price = (unsigned int)(sqrt(ship->solid->mass)*5000.);

   return price;
}


/**
 * @brief Gets the dimensions of the spaceport bar window.
 */
static void spaceport_bar_getDim( int wid,
      int *w, int *h, int *iw, int *ih, int *bw, int *bh )
{
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Calculate dimensions of portraits. */
   *iw = 300;
   *ih = *h - 60;

   /* Calculate button dimensions. */
   *bw = (*w - *iw - 80)/2;
   *bh = BUTTON_HEIGHT;
}
/**
 * @brief Opens the spaceport bar window.
 */
static void spaceport_bar_open( unsigned int wid )
{
   int w, h, iw, ih, bw, bh, dh, th;

   /* Set window functions. */
   window_onClose( wid, spaceport_bar_close );

   /* Get dimensions. */
   spaceport_bar_getDim( wid, &w, &h, &iw, &ih, &bw, &bh );
   dh = gl_printHeightRaw( &gl_smallFont, w - iw - 60, land_planet->bar_description );

   /* Buttons */
   window_addButton( wid, -20, 20,
         bw, bh, "btnCloseBar",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -20 - bw - 20, 20,
         bw, bh, "btnApproach",
         "Approach", spaceport_bar_approach );

   /* Bar description. */
   window_addText( wid, iw + 40, -40,
         w - iw - 60, dh, 0,
         "txtDescription", &gl_smallFont, &cBlack,
         land_planet->bar_description );

   /* Add portrait text. */
   th = -40 - dh - 40;
   window_addText( wid, iw + 40, th,
         w - iw - 60, gl_defFont.h, 1,
         "txtPortrait", &gl_defFont, &cDConsole, NULL );

   /* Add mission description text. */
   th -= 20 + PORTRAIT_HEIGHT + 20 + 20;
   window_addText( wid, iw + 60, th,
         w - iw - 100, h + th - (2*bh+60), 0,
         "txtMission", &gl_smallFont, &cBlack, NULL );

   /* Generate the mission list. */
   spaceport_bar_genList( wid );
}

/**
 * @brief Generates the misison list for the bar.
 *
 *    @param wid Window to create mission list for.
 */
static int spaceport_bar_genList( unsigned int wid )
{
   int i;
   glTexture **portraits;
   char **names;
   int w, h, iw, ih, bw, bh;
   int n;

   /* Get dimensions. */
   spaceport_bar_getDim( wid, &w, &h, &iw, &ih, &bw, &bh );

   /* Destroy widget if already exists. */
   if (widget_exists( wid, "iarMissions" ))
      window_destroyWidget( wid, "iarMissions" );

   /* Set up missions. */
   if (mission_portrait == NULL)
      mission_portrait = gl_newImage( "gfx/portraits/none.png", 0 );
   if (mission_nbar <= 0) {
      n            = 1;
      portraits    = malloc(sizeof(glTexture*));
      portraits[0] = mission_portrait;
      names        = malloc(sizeof(char*));
      names[0]     = strdup("News");
   }
   else {
      n            = mission_nbar+1;
      portraits    = malloc( sizeof(glTexture*) * n );
      portraits[0] = mission_portrait;
      names        = malloc( sizeof(char*) * n );
      names[0]     = strdup("News");
      for (i=0; i<mission_nbar; i++) {
         names[i+1]     = (mission_bar[i].npc != NULL) ?
            strdup( mission_bar[i].npc ) : NULL;
         portraits[i+1] = mission_bar[i].portrait;
      }
   }
   window_addImageArray( wid, 20, -40,
         iw, ih, "iarMissions", 64, 48,
         portraits, names, n, spaceport_bar_update );

   /* write the outfits stuff */
   spaceport_bar_update( wid, NULL );

   return 0;
}
/**
 * @brief Updates the missions in the spaceport bar.
 *    @param wid Window to update the outfits in.
 *    @param str Unused.
 */
static void spaceport_bar_update( unsigned int wid, char* str )
{
   (void) str;
   int pos;
   int w, h, iw, ih, bw, bh, dh;

   /* Get dimensions. */
   spaceport_bar_getDim( wid, &w, &h, &iw, &ih, &bw, &bh );
   dh = gl_printHeightRaw( &gl_smallFont, w - iw - 60, land_planet->bar_description );

   /* Get array. */
   pos = toolkit_getImageArrayPos( wid, "iarMissions" );

   /* See if is news. */
   if (pos==0) { /* News selected. */
      if (!widget_exists(wid, "cstNews")) {
         /* Destroy portrait. */
         if (widget_exists(wid, "imgPortrait")) {
            window_destroyWidget(wid, "imgPortrait");
         }

         /* Disable button. */
         window_disableButton( wid, "btnApproach" );

         /* Clear text. */
         window_modifyText(  wid, "txtPortrait", NULL );
         window_modifyText(  wid, "txtMission",  NULL );

         /* Create news. */
         news_widget( wid, iw + 60, -40 - (40 + dh),
               w - iw - 100, h - 40 - (dh+20) - 40 - bh - 20 );
      }
      return;
   }

   /* Shift to ignore news now. */
   pos--;

   /* Destroy news widget if needed. */
   if (widget_exists(wid, "cstNews")) {
      window_destroyWidget( wid, "cstNews" );
   }

   /* Create widgets if needed. */
   if (!widget_exists(wid, "imgPortrait")) {
      window_addImage( wid, iw + 40 + (w-iw-60-PORTRAIT_WIDTH)/2,
            -(40 + dh + 40 + gl_defFont.h + 20 + PORTRAIT_HEIGHT),
            "imgPortrait", NULL, 1 );
   }

   /* Enable button. */
   window_enableButton( wid, "btnApproach" );

   /* Set portrait. */
   window_modifyText(  wid, "txtPortrait", mission_bar[pos].npc );
   window_modifyImage( wid, "imgPortrait", mission_bar[pos].portrait );

   /* Set mission description. */
   window_modifyText(  wid, "txtMission",  mission_bar[pos].desc );
}
/**
 * @brief Closes the mission computer window.
 *    @param wid Window to close.
 *    @param name Unused.
 */
static void spaceport_bar_close( unsigned int wid, char *name )
{
   (void) wid;
   (void) name;

   if (mission_portrait != NULL)
      gl_freeTexture(mission_portrait);
   mission_portrait = NULL;
}
/**
 * @brief Approaches guy in mission computer.
 */
static void spaceport_bar_approach( unsigned int wid, char *str )
{
   (void) str;
   Mission* misn;
   int pos;
   int i;

   /* Get position. */
   pos = toolkit_getImageArrayPos( wid, "iarMissions" );

   /* Should never happen, but in case news is selected */
   if (pos == 0)
      return;

   /* Ignore news. */
   pos--;

   /* Make sure player can accept the mission. */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data == NULL) break;
   if (i >= MISSION_MAX) {
      dialogue_alert("You have too many active missions.");
      return;
   }

   /* Get mission. */
   misn = &mission_bar[pos];
   if (mission_accept( misn )) { /* successs in accepting the mission */
      memmove( &mission_bar[pos], &mission_bar[pos+1],
            sizeof(Mission) * (mission_nbar-pos-1) );
      mission_nbar--;

      spaceport_bar_genList(wid);
   }

   /* Reset markers. */
   mission_sysMark();
}
/**
 * @brief Loads the news.
 *
 * @return 0 on success.
 */
static int news_load (void)
{
   news_generate( NULL, 10 );
   return 0;
}



/**
 * @brief Opens the mission computer window.
 */
static void misn_open( unsigned int wid )
{
   char *buf;
   int w, h;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Set window functions. */
   window_onClose( wid, misn_close );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseMission",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -20, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnAcceptMission",
         "Accept", misn_accept );

   /* text */
   window_addText( wid, w/2 + 10, -60,
         w/2 - 30, 20, 0,
         "txtSDate", NULL, &cDConsole, "Date:" );
   buf = ntime_pretty(0);
   window_addText( wid, w/2 + 70, -60,
         w/2 - 90, 20, 0,
         "txtDate", NULL, &cBlack, buf );
   free(buf);
   window_addText( wid, w/2 + 10, -100,
         w/2 - 30, 20, 0,
         "txtSReward", &gl_smallFont, &cDConsole, "Reward:" );
   window_addText( wid, w/2 + 70, -100,
         w/2 - 90, 20, 0,
         "txtReward", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, w/2 + 10, -120,
         w/2 - 30, h/2-90, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   /* map */
   map_show( wid, 20, 20,
         w/2 - 30, h/2 - 35, 0.75 );

   misn_genList(wid, 1);
}
/**
 * @brief Closes the mission computer window.
 *    @param wid Window to close.
 *    @param name Unused.
 */
static void misn_close( unsigned int wid, char *name )
{
   (void) wid;
   (void) name;

   /* Remove computer markers just in case. */
   space_clearComputerMarkers();
}
/**
 * @brief Accepts the selected mission.
 *    @param wid Window of the mission computer.
 *    @param str Unused.
 */
static void misn_accept( unsigned int wid, char* str )
{
   (void) str;
   char* misn_name;
   Mission* misn;
   int pos;
   int i;

   misn_name = toolkit_getList( wid, "lstMission" );

   /* Make sure you have missions. */
   if (strcmp(misn_name,"No Missions")==0)
      return;

   /* Make sure player can accept the mission. */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data == NULL) break;
   if (i >= MISSION_MAX) {
      dialogue_alert("You have too many active missions.");
      return;
   }

   if (dialogue_YesNo("Accept Mission",
         "Are you sure you want to accept this mission?")) {
      pos = toolkit_getListPos( wid, "lstMission" );
      misn = &mission_computer[pos];
      if (mission_accept( misn )) { /* successs in accepting the mission */
         memmove( &mission_computer[pos], &mission_computer[pos+1],
               sizeof(Mission) * (mission_ncomputer-pos-1) );
         mission_ncomputer--;
         misn_genList(wid, 0);
      }

      /* Reset markers. */
      mission_sysMark();
   }
}
/**
 * @brief Generates the mission list.
 *    @param wid Window to generate the mission list for.
 *    @param first Is it the first time generated?
 */
static void misn_genList( unsigned int wid, int first )
{
   int i,j;
   char** misn_names;
   int w,h;

   if (!first)
      window_destroyWidget( wid, "lstMission" );

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* list */
   j = 1; /* make sure we don't accidently free the memory twice. */
   if (mission_ncomputer > 0) { /* there are missions */
      misn_names = malloc(sizeof(char*) * mission_ncomputer);
      j = 0;
      for (i=0; i<mission_ncomputer; i++)
         if (mission_computer[i].title != NULL)
            misn_names[j++] = strdup(mission_computer[i].title);
   }
   if ((mission_ncomputer==0) || (j==0)) { /* no missions. */
      if (j==0)
         free(misn_names);
      misn_names = malloc(sizeof(char*));
      misn_names[0] = strdup("No Missions");
      j = 1;
   }
   window_addList( wid, 20, -40,
         w/2 - 30, h/2 - 35,
         "lstMission", misn_names, j, 0, misn_update );
}
/**
 * @brief Updates the mission list.
 *    @param wid Window of the mission computer.
 *    @param str Unused.
 */
static void misn_update( unsigned int wid, char* str )
{
   (void) str;
   char *active_misn;
   Mission* misn;

   active_misn = toolkit_getList( wid, "lstMission" );
   if (strcmp(active_misn,"No Missions")==0) {
      window_modifyText( wid, "txtReward", "None" );
      window_modifyText( wid, "txtDesc",
            "There are no missions available here." );
      window_disableButton( wid, "btnAcceptMission" );
      return;
   }

   misn = &mission_computer[ toolkit_getListPos( wid, "lstMission" ) ];
   mission_sysComputerMark( misn );
   if (misn->sys_marker != NULL)
      map_center( misn->sys_marker );
   window_modifyText( wid, "txtReward", misn->reward );
   window_modifyText( wid, "txtDesc", misn->desc );
   window_enableButton( wid, "btnAcceptMission" );
}


/**
 * @brief Gets how much it will cost to refuel the player.
 *    @return Refuel price.
 */
static unsigned int refuel_price (void)
{
   return (unsigned int)((player->fuel_max - player->fuel)*3);
}


/**
 * @brief Refuels the player.
 *    @param wid Land window.
 *    @param str Unused.
 */
static void spaceport_refuel( unsigned int wid, char *str )
{
   (void)str;

   if (player->credits < refuel_price()) { /* player is out of money after landing */
      dialogue_alert("You seem to not have enough credits to refuel your ship." );
      return;
   }

   player->credits -= refuel_price();
   player->fuel = player->fuel_max;
   window_destroyWidget( wid, "btnRefuel" );
   window_destroyWidget( wid, "txtRefuel" );
}


/**
 * @brief Checks if should add the refuel button and does if needed.
 */
static void land_checkAddRefuel (void)
{
   char buf[32], cred[16];

   /* Check to see if fuel conditions are met. */
   if (!planet_hasService(land_planet, PLANET_SERVICE_BASIC))
      return;

   /* Full fuel. */
   if (player->fuel >= player->fuel_max)
      return;

   /* Just enable button if it exists. */
   if (widget_exists( land_windows[0], "btnRefuel" )) {
      window_enableButton( land_windows[0], "btnRefuel");
      credits2str( cred, player->credits, 2 );
      snprintf( buf, 32, "Credits: %s", cred );
      window_modifyText( land_windows[0], "txtRefuel", buf );
   }
   /* Else create it. */
   else {
      /* Refuel button. */
      credits2str( cred, refuel_price(), 2 );
      snprintf( buf, 32, "Refuel %s", cred );
      window_addButton( land_windows[0], -20, 20 + 2*(BUTTON_HEIGHT + 20),
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnRefuel",
            buf, spaceport_refuel );
      /* Player credits. */
      credits2str( cred, player->credits, 2 );
      snprintf( buf, 32, "Credits: %s", cred );
      window_addText( land_windows[0], -20, 20 + 3*(BUTTON_HEIGHT + 20),
            BUTTON_WIDTH, gl_smallFont.h, 1, "txtRefuel",
            &gl_smallFont, &cBlack, buf );
   }
   
   /* Make sure player can click it. */
   if (player->credits < refuel_price())
      window_disableButton( land_windows[0], "btnRefuel" );
}


/**
 * @brief Wrapper for takeoff mission button.
 *
 *    @param wid Window causing takeoff.
 *    @param unused Unused.
 */
static void land_buttonTakeoff( unsigned int wid, char *unused )
{
   (void) wid;
   (void) unused;
   /* We'll want the time delay. */
   takeoff(1);
}


/**
 * @brief Cleans up the land window.
 *
 *    @param wid Window closing.
 *    @param name Unused.
 */
static void land_cleanupWindow( unsigned int wid, char *name )
{
   (void) wid;
   (void) name;

   /* Clean up possible stray graphic. */
   if (gfx_exterior != NULL) {
      gl_freeTexture( gfx_exterior );
      gfx_exterior = NULL;
   }
}


/**
 * @brief Gets the WID of a window by type.
 *
 *    @param window Type of window to get wid (LAND_WINDOW_MAIN, ...).
 *    @return 0 on error, otherwise the wid of the window.
 */
static unsigned int land_getWid( int window )
{
   if (land_windowsMap[window] == -1)
      return 0;
   return land_windows[ land_windowsMap[window] ];
}


/**
 * @brief Opens up all the land dialogue stuff.
 *    @param p Planet to open stuff for.
 */
void land( Planet* p )
{
   int i, j;
   const char *names[LAND_NUMWINDOWS];

   /* Do not land twice. */
   if (landed)
      return;

   /* Stop player sounds. */
   player_stopSound();

   /* Load stuff */
   land_planet = p;
   gfx_exterior = gl_newImage( p->gfx_exterior, 0 );
   land_wid = window_create( p->name, -1, -1, LAND_WIDTH, LAND_HEIGHT );
   window_onClose( land_wid, land_cleanupWindow );

   /* Generate computer missions. */
   mission_computer = missions_genList( &mission_ncomputer,
         land_planet->faction, land_planet->name, cur_system->name,
         MIS_AVAIL_COMPUTER );

   /* Generate spaceport bar missions. */
   mission_bar = missions_genList( &mission_nbar,
         land_planet->faction, land_planet->name, cur_system->name,
         MIS_AVAIL_BAR );

   /* Generate the news. */
   if (planet_hasService(land_planet, PLANET_SERVICE_BASIC))
      news_load();

   /* Set window map to invald. */
   for (i=0; i<LAND_NUMWINDOWS; i++)
      land_windowsMap[i] = -1;

   /* See what is available. */
   j = 0;
   land_windowsMap[LAND_WINDOW_MAIN] = j;
   names[j++] = land_windowNames[LAND_WINDOW_MAIN];
   if (planet_hasService(land_planet, PLANET_SERVICE_BASIC)) {
      land_windowsMap[LAND_WINDOW_BAR] = j;
      names[j++] = land_windowNames[LAND_WINDOW_BAR];
      land_windowsMap[LAND_WINDOW_MISSION] = j;
      names[j++] = land_windowNames[LAND_WINDOW_MISSION];
   }
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS)) {
      land_windowsMap[LAND_WINDOW_OUTFITS] = j;
      names[j++] = land_windowNames[LAND_WINDOW_OUTFITS];
   }
   if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD)) {
      land_windowsMap[LAND_WINDOW_SHIPYARD] = j;
      names[j++] = land_windowNames[LAND_WINDOW_SHIPYARD];
      land_windowsMap[LAND_WINDOW_EQUIPMENT] = j;
      names[j++] = land_windowNames[LAND_WINDOW_EQUIPMENT];
   }
   if (planet_hasService(land_planet, PLANET_SERVICE_COMMODITY)) {
      land_windowsMap[LAND_WINDOW_COMMODITY] = j;
      names[j++] = land_windowNames[LAND_WINDOW_COMMODITY];
   }

   /* Create tabbed window. */
   land_windows = window_addTabbedWindow( land_wid, -1, -1, -1, -1, "tabLand", j, names );

   /* Go to last open tab. */
   if (land_windowsMap[ last_window ] != -1)
      window_tabWinSetActive( land_wid, "tabLand", land_windowsMap[ last_window ] );
   window_tabWinOnChange( land_wid, "tabLand", land_changeTab );

   /* Create each tab. */
   land_createMainTab( land_getWid(LAND_WINDOW_MAIN) );
   if (planet_hasService(land_planet, PLANET_SERVICE_BASIC)) {
      spaceport_bar_open( land_getWid(LAND_WINDOW_BAR) );
      misn_open( land_getWid(LAND_WINDOW_MISSION) );
   }
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS))
      outfits_open( land_getWid(LAND_WINDOW_OUTFITS) );
   if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD)) {
      shipyard_open( land_getWid(LAND_WINDOW_SHIPYARD) );
      equipment_open( land_getWid(LAND_WINDOW_EQUIPMENT) );
   }
   if (planet_hasService(land_planet, PLANET_SERVICE_COMMODITY))
      commodity_exchange_open( land_getWid(LAND_WINDOW_COMMODITY) );

   /* player is now officially landed */
   landed = 1;

   /* Change the music */
   music_choose("land");

   /* Run hooks, run after music in case hook wants to change music. */
   hooks_run("land");

   /* Check land missions. */
   if (!has_visited(VISITED_LAND)) {
      missions_run(MIS_AVAIL_LAND, land_planet->faction,
            land_planet->name, cur_system->name);
      visited(VISITED_LAND);
   }

   /* Add fuel button if needed - AFTER missions pay :). */
   land_checkAddRefuel();

   /* Mission forced take off. */
   if (landed == 0) {
      landed = 1; /* ugly hack to make takeoff not complain. */
      takeoff(0);
   }
}


/**
 * @brief Creates the main tab.
 *
 *    @param wid Window to create main tab.
 */
static void land_createMainTab( unsigned int wid )
{
   glTexture *logo;
   int offset;
   int w,h;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /*
    * Faction logo.
    */
   offset = 20;
   if (land_planet->faction != -1) {
      logo = faction_logoSmall(land_planet->faction);
      if (logo != NULL) {
         window_addImage( wid, 440 + (w-460-logo->w)/2, -20,
               "imgFaction", logo, 0 );
         offset = 84;
      }
   }

   /*
    * Pretty display.
    */
   window_addImage( wid, 20, -40, "imgPlanet", gfx_exterior, 1 );
   window_addText( wid, 440, -20-offset,
         w-460, h-20-offset-60-BUTTON_HEIGHT*2, 0, 
         "txtPlanetDesc", &gl_smallFont, &cBlack, land_planet->description);

   /*
    * buttons
    */
   /* first column */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnTakeoff",
         "Takeoff", land_buttonTakeoff );
}


/**
 * @brief Saves the last place the player was.
 *
 *    @param wid Unused.
 *    @param wgt Unused.
 *    @param tab Tab changed to.
 */
static void land_changeTab( unsigned int wid, char *wgt, int tab )
{
   int i;
   (void) wid;
   (void) wgt;
   unsigned int w;

   for (i=0; i<LAND_NUMWINDOWS; i++) {
      if (land_windowsMap[i] == tab) {
         last_window = i;
         w = land_getWid( i );

         /* Must regenerate outfits. */
         switch (i) {
            case LAND_WINDOW_OUTFITS:
               outfits_update( w, NULL );
               break;
            case LAND_WINDOW_SHIPYARD:
               shipyard_update( w, NULL );
               break;
            case LAND_WINDOW_BAR:
               spaceport_bar_update( w, NULL );
               break;
            case LAND_WINDOW_MISSION:
               misn_update( w, NULL );
               break;
            case LAND_WINDOW_COMMODITY:
               commodity_update( w, NULL );
               break;
            case LAND_WINDOW_EQUIPMENT:
               window_destroyWidget( w, "iarAvailOutfits" );
               equipment_genLists( w );
               break;

            default:
               break;
         }

         /* Clear markers if closing Mission Computer. */
         if (i != LAND_WINDOW_MISSION) {
            space_clearComputerMarkers();
         }

         break;
      }
   }
}


/**
 * @brief Makes the player take off if landed.
 *
 *    @param delay Whether or not to have time pass as if the player landed normally.
 */
void takeoff( int delay )
{
   int sw,sh, h;
   char *nt;

   if (!landed) return;

   /* ze music */
   music_choose("takeoff");

   /* to randomize the takeoff a bit */
   sw = land_planet->gfx_space->w;
   sh = land_planet->gfx_space->h;

   /* no longer authorized to land */
   player_rmFlag(PLAYER_LANDACK);

   /* set player to another position with random facing direction and no vel */
   player_warp( land_planet->pos.x + RNG(-sw/2,sw/2),
         land_planet->pos.y + RNG(-sh/2,sh/2) );
   vect_pset( &player->solid->vel, 0., 0. );
   player->solid->dir = RNG(0,359) * M_PI/180.;

   /* heal the player */
   player->armour = player->armour_max;
   player->shield = player->shield_max;
   player->energy = player->energy_max;

   /* time goes by, triggers hook before takeoff */
   if (delay)
      ntime_inc( RNG( 2*NTIME_UNIT_LENGTH, 3*NTIME_UNIT_LENGTH ) );
   nt = ntime_pretty(0);
   player_message("Taking off from %s on %s.", land_planet->name, nt);
   free(nt);

   /* initialize the new space */
   h = hyperspace_target;
   space_init(NULL);
   hyperspace_target = h;

   /* cleanup */
   if (save_all() < 0) { /* must be before cleaning up planet */
      dialogue_alert( "Failed to save game!  You should exit and check the log to see what happened and then file a bug report!" );
   }
   land_cleanup(); /* Cleanup stuff */
   hooks_run("takeoff"); /* Must be run after cleanup since we don't want the
                            missions to think we are landed. */
   player_addEscorts();
   hooks_run("enter");
   events_trigger( EVENT_TRIGGER_ENTER );
}


/**
 * @brief Cleans up some land-related variables.
 */
void land_cleanup (void)
{
   int i;

   /* Clean up default stuff. */
   land_planet    = NULL;
   landed         = 0;
   land_visited   = 0;
  
   /* Destroy window. */
   if (land_wid > 0)
      window_destroy(land_wid);
   land_wid       = 0;

   /* Clean up possible stray graphic. */
   if (gfx_exterior != NULL)
      gl_freeTexture( gfx_exterior );
   gfx_exterior   = NULL;

   /* Clean up mission computer. */
   for (i=0; i<mission_ncomputer; i++)
      mission_cleanup( &mission_computer[i] );
   if (mission_computer != NULL)
      free(mission_computer);
   mission_computer  = NULL;
   mission_ncomputer = 0;

   /* Clean up bar missions. */
   for (i=0; i<mission_nbar; i++)
      mission_cleanup( &mission_bar[i] );
   if (mission_bar != NULL)
      free(mission_bar);
   mission_bar    = NULL;
   mission_nbar   = 0;

   /* Set to sane defaults. */
   equipment_selected = NULL;
}

