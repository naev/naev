/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file land.c
 *
 * @brief Handles all the landing menus and actionsv.
 */


#include "land.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "naev.h"
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


/* global/main window */
#define LAND_WIDTH   700 /**< Land window width. */
#define LAND_HEIGHT  600 /**< Land window height. */
#define BUTTON_WIDTH 200 /**< Default button width. */
#define BUTTON_HEIGHT 40 /**< Default button height. */

/* commodity window */
#define COMMODITY_WIDTH 400 /**< Commodity window width. */
#define COMMODITY_HEIGHT 400 /**< Commodity window height. */

/* outfits */
#define OUTFITS_WIDTH   800 /**< Outfits window width. */
#define OUTFITS_HEIGHT  600 /**< Outfits window height. */

/* shipyard */
#define SHIPYARD_WIDTH  800 /**< Shipyard window width. */
#define SHIPYARD_HEIGHT 600 /**< Shipyard window height. */

/* news window */
#define NEWS_WIDTH   400 /**< News window width. */
#define NEWS_HEIGHT  500 /**< News window height. */

/* bar window */
#define BAR_WIDTH    460 /**< Bar window width. */
#define BAR_HEIGHT   300 /**< Bar window height. */

/* mission computer window */
#define MISSION_WIDTH   700 /**< Mission computer window width. */
#define MISSION_HEIGHT  600 /**< Mission computer window height. */


/*
 * we use visited flags to not duplicate missions generated
 */
#define VISITED_LAND          (1<<0) /**< Player already landed. */
#define VISITED_COMMODITY     (1<<1) /**< Player already visited commodities. */
#define VISITED_BAR           (1<<2) /**< Player already visited bar. */
#define VISITED_OUTFITS       (1<<3) /**< Player already visited outfits. */
#define VISITED_SHIPYARD      (1<<4) /**< Player already visited shipyard. */
#define visited(f)            (land_visited |= (f))
#define has_visited(f)        (land_visited & (f))
static unsigned int land_visited = 0; /**< Contains what the player visited. */



/*
 * land variables
 */
int landed = 0;
static unsigned int land_wid = 0; /**< Land window ID. */
Planet* land_planet = NULL;
static glTexture *gfx_exterior = NULL;

/*
 * mission computer stack
 */
static Mission* mission_computer = NULL;
static int mission_ncomputer = 0;

/*
 * player stuff
 */
extern int hyperspace_target;


/*
 * prototypes
 */
/* commodity exchange */
static void commodity_exchange_open (void);
static void commodity_update( unsigned int wid, char* str );
static void commodity_buy( unsigned int wid, char* str );
static void commodity_sell( unsigned int wid, char* str );
/* outfits */
static void outfits_open (void);
static void outfits_update( unsigned int wid, char* str );
static int outfit_canBuy( Outfit* outfit, int q, int errmsg );
static void outfits_buy( unsigned int wid, char* str );
static int outfit_canSell( Outfit* outfit, int q, int errmsg );
static void outfits_sell( unsigned int wid, char* str );
static int outfits_getMod (void);
static void outfits_renderMod( double bx, double by, double w, double h );
/* shipyard */
static void shipyard_open (void);
static void shipyard_update( unsigned int wid, char* str );
static void shipyard_info( unsigned int wid, char* str );
static void shipyard_buy( unsigned int wid, char* str );
/* your ships */
static void shipyard_yours_open( unsigned int parent, char* str );
static void shipyard_yoursUpdate( unsigned int wid, char* str );
static void shipyard_yoursChange( unsigned int wid, char* str );
static void shipyard_yoursSell( unsigned int wid, char* str );
static void shipyard_yoursTransport( unsigned int wid, char* str );
static unsigned int shipyard_yoursTransportPrice( char* shipname );
/* spaceport bar */
static void spaceport_bar_open (void);
/* news */
static void news_open(unsigned int parent, char *str);
/* mission computer */
static void misn_open (void);
static void misn_close( unsigned int wid, char *name );
static void misn_accept( unsigned int wid, char* str );
static void misn_genList( unsigned int wid, int first );
static void misn_update( unsigned int wid, char* str );
/* refuel */
static void land_checkAddRefuel (void);
static unsigned int refuel_price (void);
static void spaceport_refuel( unsigned int wid, char *str );


/*
 * the local market
 */
static void commodity_exchange_open (void)
{
   int i;
   char **goods;
   unsigned int wid;

   /* window */
   wid = window_create( "Commodity Exchange",
         -1, -1, COMMODITY_WIDTH, COMMODITY_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodityClose",
         "Close", window_close );
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
         COMMODITY_HEIGHT-140-BUTTON_HEIGHT, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   /* goods list */
   goods = malloc(sizeof(char*) * land_planet->ncommodities);
   for (i=0; i<land_planet->ncommodities; i++)
      goods[i] = strdup(land_planet->commodities[i]->name);
   window_addList( wid, 20, -40,
         COMMODITY_WIDTH-BUTTON_WIDTH-60, COMMODITY_HEIGHT-80-BUTTON_HEIGHT,
         "lstGoods", goods, land_planet->ncommodities, 0, commodity_update );

   /* update */
   commodity_update(wid, NULL);

   /* Check commodity exchange missions. */
   if (!has_visited(VISITED_COMMODITY)) {
      missions_run(MIS_AVAIL_COMMODITY, land_planet->faction,
            land_planet->name, cur_system->name);
      visited(VISITED_COMMODITY);
   }
}
static void commodity_update( unsigned int wid, char* str )
{
   (void)str;
   char buf[128];
   char *comname;
   Commodity *com;

   comname = toolkit_getList( wid, "lstGoods" );
   com = commodity_get( comname );

   /* modify text */
   snprintf( buf, 128,
         "%d tons\n"
         "%d credits/ton\n"
         "\n"
         "%d tons\n",
         player_cargoOwned( comname ),
         com->medium,
         pilot_cargoFree(player));
   window_modifyText( wid, "txtDInfo", buf );
   window_modifyText( wid, "txtDesc", com->description );

}
static void commodity_buy( unsigned int wid, char* str )
{
   (void)str;
   char *comname;
   Commodity *com;
   int q;

   q = 10;
   comname = toolkit_getList( wid, "lstGoods" );
   com = commodity_get( comname );

   if (player->credits < q * com->medium) {
      dialogue_alert( "Not enough credits!" );
      return;
   }
   else if (pilot_cargoFree(player) <= 0) {
      dialogue_alert( "Not enough free space!" );
      return;
   }

   q = pilot_addCargo( player, com, q );
   player->credits -= q * com->medium;
   land_checkAddRefuel();
   commodity_update(wid, NULL);
}
static void commodity_sell( unsigned int wid, char* str )
{
   (void)str;
   char *comname;
   Commodity *com;
   int q;

   q = 10;
   comname = toolkit_getList( wid, "lstGoods" );
   com = commodity_get( comname );

   q = pilot_rmCargo( player, com, q );
   player->credits += q * com->medium;
   land_checkAddRefuel();
   commodity_update(wid, NULL);
}


/*
 * ze outfits
 */
static void outfits_open (void)
{
   int i;
   Outfit **outfits;
   char **soutfits;
   glTexture **toutfits;
   int noutfits;
   char buf[128];
   unsigned int wid;

   /* create window */
   snprintf(buf,128,"%s - Outfits", land_planet->name );
   wid = window_create( buf, -1, -1,
         OUTFITS_WIDTH, OUTFITS_HEIGHT );
   window_setAccept( wid, outfits_buy ); /* will allow buying from keyboard */

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseOutfits",
         "Close", window_close );
   window_addButton( wid, -40-BUTTON_WIDTH, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnBuyOutfit",
         "Buy", outfits_buy );
   window_addButton( wid, -40-BUTTON_WIDTH, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnSellOutfit",
         "Sell", outfits_sell );

   /* fancy 128x128 image */
   window_addRect( wid, -20, -50, 128, 128, "rctImage", &cBlack, 0 );
   window_addImage( wid, -20-128, -50-128, "imgOutfit", NULL, 1 );

   /* cust draws the modifier */
   window_addCust( wid, -40-BUTTON_WIDTH, 60+2*BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "cstMod", 0, outfits_renderMod, NULL );

   /* the descriptive text */
   window_addText( wid, 40+300+20, -60,
         80, 140, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Name:\n"
         "Type:\n"
         "Owned:\n"
         "\n"
         "Space taken:\n"
         "Free Space:\n"
         "\n"
         "Price:\n"
         "Money:\n" );
   window_addText( wid, 40+300+40+60, -60,
         250, 140, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 20+300+40, -220,
         OUTFITS_WIDTH-400, 180, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );

   /* set up the outfits to buy/sell */
   outfits = outfit_getTech( &noutfits, land_planet->tech, PLANET_TECH_MAX);
   if (noutfits <= 0) { /* No outfits */
      soutfits = malloc(sizeof(char*));
      soutfits[0] = strdup("None");
      toutfits = malloc(sizeof(glTexture*));
      toutfits[0] = NULL;
      noutfits = 1;
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
   window_addImageArray( wid, 20, 40,
         310, OUTFITS_HEIGHT-80, "iarOutfits", 64, 64,
         toutfits, soutfits, noutfits, outfits_update );

   /* write the outfits stuff */
   outfits_update( wid, NULL );

   /* Check outfit missions. */
   if (!has_visited(VISITED_OUTFITS)) {
      missions_run(MIS_AVAIL_OUTFIT, land_planet->faction,
            land_planet->name, cur_system->name);
      visited(VISITED_OUTFITS);
   }
}
static void outfits_update( unsigned int wid, char* str )
{
   (void)str;
   char *outfitname;
   Outfit* outfit;
   char buf[128], buf2[16], buf3[16];

   outfitname = toolkit_getList( wid, "iarOutfits" );
   if (strcmp(outfitname,"None")==0) { /* No outfits */
      window_modifyImage( wid, "imgOutfit", NULL );
      window_disableButton( wid, "btnBuyOutfit" );
      window_disableButton( wid, "btnSellOutfit" );
      snprintf( buf, 128,
            "None\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "%d\n"
            "\n"
            "NA\n"
            "NA\n",
            pilot_freeSpace(player) );
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
   snprintf( buf, 128,
         "%s\n"
         "%s\n"
         "%d\n"
         "\n"
         "%d\n"
         "%d\n"
         "\n"
         "%s credits\n"
         "%s credits\n",
         outfit->name,
         outfit_getType(outfit),
         player_outfitOwned(outfitname),
         outfit->mass,
         pilot_freeSpace(player),
         buf2,
         buf3 );
   window_modifyText( wid,  "txtDDesc", buf );
}
static int outfit_canBuy( Outfit* outfit, int q, int errmsg )
{
   char buf[16];

   /* can player actually fit the outfit? */
   if ((pilot_freeSpace(player) - outfit->mass) < 0) {
      if (errmsg != 0)
         dialogue_alert( "Not enough free space (you need %d more).",
               outfit->mass - pilot_freeSpace(player) );
      return 0;
   }
   /* has too many already */
   else if (player_outfitOwned(outfit->name) >= outfit->max) {
      if (errmsg != 0)
         dialogue_alert( "You can only carry %d of this outfit.",
               outfit->max );
      return 0;
   }
   /* can only have one afterburner */
   else if (outfit_isAfterburner(outfit) && (player->afterburner!=NULL)) {
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

   return 1;
}
static void outfits_buy( unsigned int wid, char* str )
{
   (void)str;
   char *outfitname;
   Outfit* outfit;
   int q;

   outfitname = toolkit_getList( wid, "iarOutfits" );
   outfit = outfit_get( outfitname );

   q = outfits_getMod();

   /* can buy the outfit? */
   if (outfit_canBuy(outfit, q, 1) == 0)
      return;

   player->credits -= outfit->price * pilot_addOutfit( player, outfit,
         MIN(q,outfit->max) );
   land_checkAddRefuel();
   outfits_update(wid, NULL);
}
static int outfit_canSell( Outfit* outfit, int q, int errmsg )
{
   /* has no outfits to sell */
   if (player_outfitOwned(outfit->name) <= 0) {
      if (errmsg != 0)
         dialogue_alert( "You can't sell something you don't have." );
      return 0;
   }
   /* can't sell when you are using it */
   else if (outfit_isMod(outfit) && (pilot_cargoFree(player) < outfit->u.mod.cargo*q)) {
      if (errmsg != 0)
         dialogue_alert( "You currently have cargo in this modification." );
      return 0;
   }
   /* has negative mass and player has no free space */
   else if ((outfit->mass < 0) && (pilot_freeSpace(player) < -outfit->mass)) {
      if (errmsg != 0)
         dialogue_alert( "Get rid of other outfits first.");
      return 0;
   }

   return 1;
}
static void outfits_sell( unsigned int wid, char* str )
{
   (void)str;
   char *outfitname;
   Outfit* outfit;
   int q;

   outfitname = toolkit_getList( wid, "iarOutfits" );
   outfit = outfit_get( outfitname );

   q = outfits_getMod();

   /* has no outfits to sell */
   if (outfit_canSell( outfit, q, 1 ) == 0)
      return;

   player->credits += outfit->price * pilot_rmOutfit( player, outfit, q );
   land_checkAddRefuel();
   outfits_update(wid, NULL);
}
/*
 * returns the current modifier status
 */
static int outfits_getMod (void)
{
   SDLMod mods;
   int q;

   mods = SDL_GetModState();
   q = 1;
   if (mods & (KMOD_LCTRL | KMOD_RCTRL)) q *= 5;
   if (mods & (KMOD_LSHIFT | KMOD_RSHIFT)) q *= 10;

   return q;
}
static void outfits_renderMod( double bx, double by, double w, double h )
{
   (void) h;
   int q;
   char buf[8];

   q = outfits_getMod();
   if (q==1) return;

   snprintf( buf, 8, "%dx", q );
   gl_printMid( &gl_smallFont, w,
         bx + (double)SCREEN_W/2.,
         by + (double)SCREEN_H/2.,
         &cBlack, buf );
}




/*
 * ze shipyard
 */
static void shipyard_open (void)
{
   int i;
   Ship **ships;
   char **sships;
   glTexture **tships;
   int nships;
   char buf[128];
   unsigned int wid;

   /* window creation */
   snprintf( buf, 128, "%s - Shipyard", land_planet->name );
   wid = window_create( buf,
         -1, -1, SHIPYARD_WIDTH, SHIPYARD_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseShipyard",
         "Close", window_close );
   window_addButton( wid, -20, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnYourShips",
         "Your Ships", shipyard_yours_open );
   window_addButton( wid, -40-BUTTON_WIDTH, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnBuyShip",
         "Buy", shipyard_buy );
   window_addButton( wid, -40-BUTTON_WIDTH, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnInfoShip",
         "Info", shipyard_info );

   /* target gfx */
   window_addRect( wid, -40, -50,
         128, 96, "rctTarget", &cBlack, 0 );
   window_addImage( wid, -40-128, -50-96,
         "imgTarget", NULL, 1 );

   /* text */
   window_addText( wid, 40+300+40, -55,
         80, 96, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Name:\n"
         "Class:\n"
         "Fabricator:\n"
         "\n"
         "Price:\n"
         "Money:\n" );
   window_addText( wid, 40+300+40+80, -55,
         130, 96, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 20+310+40, -175,
         SHIPYARD_WIDTH-(20+310+40) - 20, 185, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );

   /* set up the ships to buy/sell */
   ships = ship_getTech( &nships, land_planet->tech, PLANET_TECH_MAX );
   if (nships <= 0) {
      sships = malloc(sizeof(char*));
      sships[0] = strdup("None");
      tships = malloc(sizeof(glTexture*));
      tships[0] = NULL;
      nships = 1;
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
   window_addImageArray( wid, 20, 40,
         310, SHIPYARD_HEIGHT-80, "iarShipyard", 64./96.*128., 64.,
         tships, sships, nships, shipyard_update );

   /* write the shipyard stuff */
   shipyard_update(wid, NULL);

   /* Run available missions. */
   if (!has_visited(VISITED_SHIPYARD)) {
      missions_run(MIS_AVAIL_SHIPYARD, land_planet->faction,
            land_planet->name, cur_system->name);
      visited(VISITED_SHIPYARD);
   }
}
static void shipyard_update( unsigned int wid, char* str )
{
   (void)str;
   char *shipname;
   Ship* ship;
   char buf[80], buf2[16], buf3[16];
   
   shipname = toolkit_getList( wid, "iarShipyard" );

   /* No ships */
   if (strcmp(shipname,"None")==0) {
      window_modifyImage( wid, "imgTarget", NULL );
      window_disableButton( wid, "btnBuyShip");
      window_disableButton( wid, "btnInfoShip");
      snprintf( buf, 80,
            "None\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "NA\n" );
      window_modifyText( wid,  "txtDDesc", buf );
      return;
   }

   ship = ship_get( shipname );

   /* toggle your shipyard */
   if (player_nships()==0) window_disableButton(wid,"btnYourShips");
   else window_enableButton(wid,"btnYourShips");

   /* update image */
   window_modifyImage( wid, "imgTarget", ship->gfx_target );

   /* update text */
   window_modifyText( wid, "txtDescription", ship->description );
   credits2str( buf2, ship->price, 2 );
   credits2str( buf3, player->credits, 2 );
   snprintf( buf, 80,
         "%s\n"
         "%s\n"
         "%s\n"
         "\n"
         "%s credits\n"
         "%s credits\n",
         ship->name,
         ship_class(ship),
         ship->fabricator,
         buf2,
         buf3);
   window_modifyText( wid,  "txtDDesc", buf );

   if (ship->price > player->credits)
      window_disableButton( wid, "btnBuyShip");
   else window_enableButton( wid, "btnBuyShip");
}
static void shipyard_info( unsigned int wid, char* str )
{
   (void)str;
   char *shipname;

   shipname = toolkit_getList( wid, "iarShipyard" );
   ship_view(0, shipname);
}
static void shipyard_buy( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[16];
   Ship* ship;

   shipname = toolkit_getList( wid, "iarShipyard" );
   ship = ship_get( shipname );

   /* Must have enough money. */
   if (ship->price > player->credits) {
      dialogue_alert( "Not enough credits!" );
      return;
   }

   /* we now move cargo to the new ship */
   if (pilot_cargoUsed(player) > ship->cap_cargo) {
      dialogue_alert("You won't have space to move your current cargo onto the new ship.");
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
static void shipyard_yours_open( unsigned int parent, char* str )
{
   (void) str;
   (void) parent;
   char **sships;
   glTexture **tships;
   int nships;
   unsigned int wid;

   /* create window */
   wid = window_create( "Your Ships",
         -1, -1, SHIPYARD_WIDTH, SHIPYARD_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseYourShips",
         "Shipyard", window_close );
   window_addButton( wid, -40-BUTTON_WIDTH, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnChangeShip",
         "Change Ship", shipyard_yoursChange );
   window_addButton( wid, -40-BUTTON_WIDTH, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnTransportShip",
         "Transport Ship", shipyard_yoursTransport );
   window_addButton( wid, -20, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnSellShip",
         "Sell Ship", shipyard_yoursSell );

   /* image */
   window_addRect( wid, -40, -50,
         128, 96, "rctTarget", &cBlack, 0 );
   window_addImage( wid, -40-128, -50-96,
         "imgTarget", NULL, 1 );

   /* text */
   window_addText( wid, 40+300+40, -55,
         100, 200, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Name:\n"
         "Ship:\n"
         "Class:\n"
         "Where:\n"
         "\n"
         "Cargo free:\n"
         "Weapons free:\n"
         "\n"
         "Transportation:\n"
         "Sell price:\n"
         );
   window_addText( wid, 40+300+40+100, -55,
      130, 200, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 40+300+40, -255,
      100, 20, 0, "txtSOutfits", &gl_smallFont, &cDConsole,
      "Outfits:\n"
      );
   window_addText( wid, 40+300+40, -255-gl_smallFont.h-5,
      SHIPYARD_WIDTH-40-300-40-20, 200, 0, "txtDOutfits",
      &gl_smallFont, &cBlack, NULL );

   /* ship list */
   nships = MAX(1,player_nships());
   sships = malloc(sizeof(char*)*nships);
   tships = malloc(sizeof(glTexture*)*nships);
   player_ships( sships, tships );
   window_addImageArray( wid, 20, 40,
         310, SHIPYARD_HEIGHT-80, "lstYourShips", 64./96.*128., 64.,
         tships, sships, nships, shipyard_yoursUpdate );

   shipyard_yoursUpdate(wid, NULL);
}
static void shipyard_yoursUpdate( unsigned int wid, char* str )
{
   (void)str;
   char buf[256], buf2[16], buf3[16], *buf4;
   char *shipname;
   Pilot *ship;
   char* loc;
   unsigned int price;

   shipname = toolkit_getList( wid, "lstYourShips" );
   if (strcmp(shipname,"None")==0) { /* no ships */
      window_disableButton( wid, "btnChangeShip" );
      window_disableButton( wid, "btnTransportShip" );
      window_disableButton( wid, "btnSellShip" );
      return;
   }
   ship = player_getShip( shipname );
   loc = player_getLoc(ship->name);
   price = shipyard_yoursTransportPrice(shipname);

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
         "%d tons\n"
         "\n"
         "%s credits\n"
         "%s credits\n",
         ship->name,
         ship->ship->name,
         ship_class(ship->ship),
         loc,
         pilot_cargoFree(ship),
         pilot_freeSpace(ship),
         buf2,
         buf3);
   window_modifyText( wid, "txtDDesc", buf );

   buf4 = pilot_getOutfits( ship );
   window_modifyText( wid, "txtDOutfits", buf4 );
   free(buf4);

   /* button disabling */
   if (strcmp(land_planet->name,loc)) { /* ship not here */
      window_disableButton( wid, "btnChangeShip" );
      if (price > player->credits)
         window_disableButton( wid, "btnTransportShip" );
      else window_enableButton( wid, "btnTransportShip" );
   }
   else { /* ship is here */
      window_enableButton( wid, "btnChangeShip" );
      window_disableButton( wid, "btnTransportShip" );
   }
   /* If ship is there you can always sell. */
   window_enableButton( wid, "btnSellShip" );
}
static void shipyard_yoursChange( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, *loc;
   Pilot *newship;

   shipname = toolkit_getList( wid, "lstYourShips" );
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

   player_swapShip(shipname);

   /* recreate the window */
   window_destroy(wid);
   shipyard_yours_open(0, NULL);
}
static void shipyard_yoursSell( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[16];
   int price;

   shipname = toolkit_getList( wid, "lstYourShips" );
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

   /* recreate the window */
   window_destroy(wid);
   shipyard_yours_open(0, NULL);
}
static void shipyard_yoursTransport( unsigned int wid, char* str )
{
   (void)str;
   unsigned int price;
   char *shipname, buf[16];

   shipname = toolkit_getList( wid, "lstYourShips" );
   if (strcmp(shipname,"None")==0) { /* no ships */
      dialogue_alert( "You can't transport nothing here!" );
      return;
   }

   price = shipyard_yoursTransportPrice( shipname );
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

   /* update the window to reflect the change */
   shipyard_yoursUpdate(wid, NULL);
}
static unsigned int shipyard_yoursTransportPrice( char* shipname )
{
   char *loc;
   Pilot* ship;
   int price;

   ship = player_getShip(shipname);
   loc = player_getLoc(shipname);
   if (strcmp(loc,land_planet->name)==0) /* already here */
      return 0;

   price = (int)(sqrt(ship->solid->mass)*5000.);

   return price;
}


/*
 * the spaceport bar
 */
static void spaceport_bar_open (void)
{
   unsigned int wid;

   /* window */
   wid = window_create( "Spaceport Bar",
         -1, -1, BAR_WIDTH, BAR_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseBar",
         "Close", window_close );
   window_addButton( wid, 20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnNews",
         "News", news_open);

   /* text */
   window_addText( wid, 20, -50,
         BAR_WIDTH - 40, BAR_HEIGHT - 60 - BUTTON_HEIGHT, 0,
         "txtDescription", &gl_smallFont, &cBlack, land_planet->bar_description );

   if (!has_visited(VISITED_BAR)) {
      missions_run(MIS_AVAIL_BAR, land_planet->faction,
            land_planet->name, cur_system->name);
      visited(VISITED_BAR);
   }
}



/*
 * the planetary news reports
 */
static void news_open( unsigned int parent, char *str )
{
   (void) parent;
   (void) str;
   unsigned int wid;

   /* create window */
   wid = window_create( "News Reports",
         -1, -1, NEWS_WIDTH, NEWS_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseNews",
         "Close", window_close );

   /* text */
   window_addText( wid, 20, 20 + BUTTON_HEIGHT + 20,
         NEWS_WIDTH-40, NEWS_HEIGHT - 20 - BUTTON_HEIGHT - 20 - 20 - 20,
         0, "txtNews", &gl_smallFont, &cBlack,
         "News reporters report that they are on strike!");
}


/*
 * mission computer, cuz missions rock
 */
static void misn_open (void)
{
   unsigned int wid;

   /* create window */
   wid = window_create( "Mission Computer",
         -1, -1, MISSION_WIDTH, MISSION_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseMission",
         "Close", misn_close );
   window_addButton( wid, -20, 40+BUTTON_HEIGHT,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnAcceptMission",
         "Accept", misn_accept );

   /* text */
   window_addText( wid, 300+40, -60,
         300, 40, 0, "txtSReward",
         &gl_smallFont, &cDConsole, "Reward:" );
   window_addText( wid, 300+100, -60,
         240, 40, 0, "txtReward", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 300+40, -100,
         300, MISSION_HEIGHT - BUTTON_WIDTH - 120, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   misn_genList(wid, 1);
}
static void misn_close( unsigned int wid, char *name )
{
   /* Remove computer markers just in case. */
   space_clearComputerMarkers();
   window_close( wid, name );
}
static void misn_accept( unsigned int wid, char* str )
{
   char* misn_name;
   Mission* misn;
   int pos;
   (void)str;

   misn_name = toolkit_getList( wid, "lstMission" );

   if (strcmp(misn_name,"No Missions")==0) return;

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
   }
}
static void misn_genList( unsigned int wid, int first )
{
   int i,j;
   char** misn_names;

   if (!first)
      window_destroyWidget( wid, "lstMission" );

   /* list */
   if (mission_ncomputer > 0) { /* there are missions */
      misn_names = malloc(sizeof(char*) * mission_ncomputer);
      j = 0;
      for (i=0; i<mission_ncomputer; i++)
         if (mission_computer[i].title)
            misn_names[j++] = strdup(mission_computer[i].title);
   }
   if ((mission_ncomputer==0) || (j==0)) { /* no missions */
      if (j==0) free(misn_names);
      misn_names = malloc(sizeof(char*));
      misn_names[0] = strdup("No Missions");
      j = 1;
   }
   window_addList( wid, 20, -40,
         300, MISSION_HEIGHT-60,
         "lstMission", misn_names, j, 0, misn_update );

   misn_update(wid, NULL);
}
static void misn_update( unsigned int wid, char* str )
{
   char *active_misn;
   Mission* misn;

   (void)str;

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
   window_modifyText( wid, "txtReward", misn->reward );
   window_modifyText( wid, "txtDesc", misn->desc );
   window_enableButton( wid, "btnAcceptMission" );
}


/**
 * @fn static int refuel_price (void)
 *
 * @brief Gets how much it will cost to refuel the player.
 *
 *    @return Refuel price.
 */
static unsigned int refuel_price (void)
{
   return (unsigned int)((player->fuel_max - player->fuel)*3);
}


/**
 * @brief Refuels the player.
 *
 *    @param str Unused.
 */
static void spaceport_refuel( unsigned int wid, char *str )
{
   (void)str;

   if (player->credits < refuel_price()) { /* player is out of money after landing */
      dialogue_alert("You seem to not have enough credits to refuel your ship" );
      return;
   }

   player->credits -= refuel_price();
   player->fuel = player->fuel_max;
   window_destroyWidget( wid, "btnRefuel" );
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
   if (widget_exists( land_wid, "btnRefuel" )) {
      window_enableButton( land_wid, "btnRefuel");
   }
   /* Else create it. */
   else {
      credits2str( cred, refuel_price(), 2 );
      snprintf( buf, 32, "Refuel %s", cred );
      window_addButton( land_wid, -20, 20 + 2*(BUTTON_HEIGHT + 20),
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnRefuel",
            buf, spaceport_refuel );
   }
   
   /* Make sure player can click it. */
   if (player->credits < refuel_price())
      window_disableButton( land_wid, "btnRefuel" );
}


/**
 * @brief Opens up all the land dialogue stuff.
 *
 *    @param p Planet to open stuff for.
 */
void land( Planet* p )
{
   glTexture *logo;
   int offset;

   if (landed) return;

   /* Load stuff */
   land_planet = p;
   gfx_exterior = gl_newImage( p->gfx_exterior );
   land_wid = window_create( p->name, -1, -1, LAND_WIDTH, LAND_HEIGHT );
   
   /*
    * Faction logo.
    */
   offset = 20;
   if (land_planet->faction != -1) {
      logo = faction_logoSmall(land_planet->faction);
      if (logo != NULL) {
         window_addImage( land_wid, 440 + (LAND_WIDTH-460-logo->w)/2, -20,
               "imgFaction", logo, 0 );
         offset = 84;
      }
   }

   /*
    * Pretty display.
    */
   window_addImage( land_wid, 20, -40, "imgPlanet", gfx_exterior, 1 );
   window_addText( land_wid, 440, -20-offset,
         LAND_WIDTH-460, LAND_HEIGHT-20-offset-60-BUTTON_HEIGHT*2, 0, 
         "txtPlanetDesc", &gl_smallFont, &cBlack, p->description);

   /*
    * buttons
    */
   /* first column */
   window_addButton( land_wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnTakeoff",
         "Takeoff", (void(*)(unsigned int,char*))takeoff );
   if (planet_hasService(land_planet, PLANET_SERVICE_COMMODITY))
      window_addButton( land_wid, -20, 20 + BUTTON_HEIGHT + 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodity",
            "Commodity Exchange", (void(*)(unsigned int,char*))commodity_exchange_open);
   /* second column */
   if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
      window_addButton( land_wid, -20 - BUTTON_WIDTH - 20, 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnShipyard",
            "Shipyard", (void(*)(unsigned int,char*))shipyard_open);
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS))
      window_addButton( land_wid, -20 - BUTTON_WIDTH - 20, 20 + BUTTON_HEIGHT + 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnOutfits",
            "Outfits", (void(*)(unsigned int,char*))outfits_open);
   /* third column */
   if (planet_hasService(land_planet, PLANET_SERVICE_BASIC)) {
      window_addButton( land_wid, 20, 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnNews",
            "Mission Terminal", (void(*)(unsigned int,char*))misn_open);
      window_addButton( land_wid, 20, 20 + BUTTON_HEIGHT + 20,
            BUTTON_WIDTH, BUTTON_HEIGHT, "btnBar",
            "Spaceport Bar", (void(*)(unsigned int,char*))spaceport_bar_open);
   }


   /* player is now officially landed */
   landed = 1;

   /* Change the music */
   music_choose("land");

   /* Run hooks */
   hooks_run("land");

   /* generate mission computer stuff */
   mission_computer = missions_computer( &mission_ncomputer,
         land_planet->faction, land_planet->name, cur_system->name );

   /* Check land missions. */
   if (!has_visited(VISITED_LAND)) {
      missions_run(MIS_AVAIL_LAND, land_planet->faction,
            land_planet->name, cur_system->name);
      visited(VISITED_LAND);
   }

   /* Add fuel button if needed - AFTER missions pay :). */
   land_checkAddRefuel();
}


/**
 * @brief Makes the player take off if landed.
 */
void takeoff (void)
{
   int sw,sh, i, h;
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
   ntime_inc( RNG( 2*NTIME_UNIT_LENGTH, 3*NTIME_UNIT_LENGTH ) );
   nt = ntime_pretty(0);
   player_message("Taking off from %s on %s.", land_planet->name, nt);
   free(nt);

   /* initialize the new space */
   h = hyperspace_target;
   space_init(NULL);
   hyperspace_target = h;

   /* cleanup */
   save_all(); /* must be before cleaning up planet */
   land_cleanup(); /* Cleanup stuff */
   hooks_run("takeoff"); /* Must be run after cleanup since we don't want the
                            missions to think we are landed. */
   hooks_run("enter");

   /* cleanup mission computer */
   for (i=0; i<mission_ncomputer; i++)
      mission_cleanup( &mission_computer[i] );
   free(mission_computer);
   mission_computer = NULL;
   mission_ncomputer = 0;
}


/**
 * @brief Cleans up some land-related variables.
 */
void land_cleanup (void)
{
   /* Clean up default stuff. */
   land_planet = NULL;
   landed = 0;
   land_visited = 0;
  
   /* Destroy window. */
   if (land_wid > 0) {
      window_destroy(land_wid);
      land_wid = 0;
   }

   /* Clean up possible stray graphic. */
   if (gfx_exterior != NULL) {
      gl_freeTexture( gfx_exterior );
      gfx_exterior = NULL;
   }
}

