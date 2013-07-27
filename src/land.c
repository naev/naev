/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file land.c
 *
 * @brief Handles all the landing menus and actions.
 */


#include "land.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"
#include <math.h>

#include "land_outfits.h"
#include "land_shipyard.h"

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
#include "map.h"
#include "news.h"
#include "escort.h"
#include "event.h"
#include "conf.h"
#include "gui.h"
#include "equipment.h"
#include "npc.h"
#include "camera.h"
#include "menu.h"
#include "ndata.h"


/* global/main window */
#define LAND_WIDTH   800 /**< Land window width. */
#define LAND_HEIGHT  600 /**< Land window height. */
#define PORTRAIT_WIDTH 200
#define PORTRAIT_HEIGHT 150


/*
 * we use visited flags to not duplicate missions generated
 */
#define VISITED_LAND          (1<<0) /**< Player already landed. */
#define VISITED_COMMODITY     (1<<1) /**< Player already visited commodities. */
#define VISITED_BAR           (1<<2) /**< Player already visited bar. */
#define VISITED_OUTFITS       (1<<3) /**< Player already visited outfits. */
#define VISITED_SHIPYARD      (1<<4) /**< Player already visited shipyard. */
#define VISITED_EQUIPMENT     (1<<5) /**< Player already visited equipment. */
#define VISITED_MISSION       (1<<6) /**< Player already visited mission computer. */
#define visited(f)            (land_visited |= (f)) /**< Mark place is visited. */
#define has_visited(f)        (land_visited & (f)) /**< Check if player has visited. */
static unsigned int land_visited = 0; /**< Contains what the player visited. */


/*
 * land variables
 */
int landed = 0; /**< Is player landed. */
static int land_takeoff = 0; /**< Takeoff. */
int land_loaded = 0; /**< Finished loading? */
static unsigned int land_wid = 0; /**< Land window ID. */
static int land_regen = 0; /**< Whether or not regenning. */
static const char *land_windowNames[LAND_NUMWINDOWS] = {
   "Landing Main",
   "Spaceport Bar",
   "Missions",
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
 * Bar stuff.
 */
static glTexture *mission_portrait = NULL; /**< Mission portrait. */

/*
 * player stuff
 */
static int last_window = 0; /**< Default window. */
int commodity_mod = 1;


/*
 * Error handling.
 */
static char errorlist[512];
static char errorreason[512];
static int errorappend;
static char *errorlist_ptr;

extern void refresh_pl_prices(Planet *pl); /* refresh the prices of a planet */

/*
 * prototypes
 */
static void land_createMainTab( unsigned int wid );
static void land_cleanupWindow( unsigned int wid, char *name );
static void land_changeTab( unsigned int wid, char *wgt, int tab );
/* commodity exchange */
static void commodity_exchange_open( unsigned int wid );
static void commodity_update( unsigned int wid, char* str );
static void commodity_buy( unsigned int wid, char* str );
static void commodity_sell( unsigned int wid, char* str );
static int commodity_canBuy( char *name );
static int commodity_canSell( char *name );
static void commodity_renderMod( double bx, double by, double w, double h, void *data );
/* spaceport bar */
static void bar_getDim( int wid,
      int *w, int *h, int *iw, int *ih, int *bw, int *bh );
static void bar_open( unsigned int wid );
static int bar_genList( unsigned int wid );
static void bar_update( unsigned int wid, char* str );
static void bar_close( unsigned int wid, char* str );
static void bar_approach( unsigned int wid, char* str );
static int news_load (void);
/* mission computer */
static void misn_open( unsigned int wid );
static void misn_close( unsigned int wid, char *name );
static void misn_accept( unsigned int wid, char* str );
static void misn_genList( unsigned int wid, int first );
static void misn_update( unsigned int wid, char* str );
/* refuel */
static credits_t refuel_price (void);
static void spaceport_refuel( unsigned int wid, char *str );
static void land_toggleRefuel( unsigned int wid, char *name );


/**
 * @brief Queue a takeoff.
 */
void land_queueTakeoff (void)
{
   land_takeoff = 1;
}


/**
 * @brief Check to see if finished loading.
 */
int land_doneLoading (void)
{
   if (landed && land_loaded)
      return 1;
   return 0;
}


/**
 * @brief Opens the local market window.
 */
static void commodity_exchange_open( unsigned int wid )
{
   int i, ngoods;
   char **goods;
   int w, h;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* buttons */
   window_addButtonKey( wid, -20, 20,
         LAND_BUTTON_WIDTH, LAND_BUTTON_HEIGHT, "btnCommodityClose",
         "Take Off", land_buttonTakeoff, SDLK_t );
   window_addButtonKey( wid, -40-((LAND_BUTTON_WIDTH-20)/2), 20*2 + LAND_BUTTON_HEIGHT,
         (LAND_BUTTON_WIDTH-20)/2, LAND_BUTTON_HEIGHT, "btnCommodityBuy",
         "Buy", commodity_buy, SDLK_b );
   window_addButtonKey( wid, -20, 20*2 + LAND_BUTTON_HEIGHT,
         (LAND_BUTTON_WIDTH-20)/2, LAND_BUTTON_HEIGHT, "btnCommoditySell",
         "Sell", commodity_sell, SDLK_s );

      /* cust draws the modifier */ 
   window_addCust( wid, -40-((LAND_BUTTON_WIDTH-20)/2)+90, 60 + 2*LAND_BUTTON_HEIGHT,
         (LAND_BUTTON_WIDTH-20)+10, LAND_BUTTON_HEIGHT*2, "cstMod", 0, commodity_renderMod, NULL, NULL );

   /* text */
   window_addText( wid, -20, -40, LAND_BUTTON_WIDTH, 110, 0,
         "txtSInfo", &gl_smallFont, &cDConsole,
         "You have:\n"
         "Market Price:\n"
         "\n"
         "Free Space:\n" );
   window_addText( wid, -20, -40, LAND_BUTTON_WIDTH/2+10, 100, 0,
         "txtDInfo", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, -40, -140, LAND_BUTTON_WIDTH-20,
         h-140-LAND_BUTTON_HEIGHT, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   /* goods list */
   if (land_planet->ncommodities > 0) {
      goods = malloc(sizeof(char*) * land_planet->ncommodities);
      for (i=0; i<land_planet->ncommodities; i++)
         goods[i] = strdup(land_planet->commodities[i]->name);
      ngoods = land_planet->ncommodities;
   }
   else {
      goods    = malloc( sizeof(char*) );
      goods[0] = strdup("None");
      ngoods   = 1;
   }
   window_addList( wid, 20, -40,
         w-LAND_BUTTON_WIDTH-60, h-80-LAND_BUTTON_HEIGHT,
         "lstGoods", goods, ngoods, 0, commodity_update );
   /* Set default keyboard focuse to the list */
   window_setFocus( wid , "lstGoods" );
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
   if ((comname==NULL) || (strcmp( comname, "None" )==0)) {
      nsnprintf( buf, PATH_MAX,
         "NA Tons\n"
         "NA Credits/Ton\n"
         "\n"
         "NA Tons\n" );
      window_modifyText( wid, "txtDInfo", buf );
      window_modifyText( wid, "txtDesc", "No outfits available." );
      window_disableButton( wid, "btnCommodityBuy" );
      window_disableButton( wid, "btnCommoditySell" );
      return;
   }

   /* modify text */
   com = commodity_get( comname );
   nsnprintf( buf, PATH_MAX,
         "%d Tons\n"
         "%"CREDITS_PRI"\n   Credits/Ton"
         "\n"
         "%d Tons\n",
         pilot_cargoOwned( player.p, comname ),
         planet_commodityPrice( land_planet, com ),
         pilot_cargoFree(player.p));
   window_modifyText( wid, "txtDInfo", buf );
   window_modifyText( wid, "txtDesc", com->description );

   /* Button enabling/disabling */
   if (commodity_canBuy( comname ))
      window_enableButton( wid, "btnCommodityBuy" );
   else
      window_disableButtonSoft( wid, "btnCommodityBuy" );

   if (commodity_canSell( comname ))
      window_enableButton( wid, "btnCommoditySell" );
   else
      window_disableButtonSoft( wid, "btnCommoditySell" );
}

static int commodity_canBuy( char *name )
{
   int failure;
   Commodity *com;
   char buf[ECON_CRED_STRLEN];

   failure = 0;
   com = commodity_get( name );

   if (strcmp(name, "None")==0){
      printf("commodity is \"None\", returning\n");
      return 0;
   }
   if (!player_hasCredits( planet_commodityPrice(land_planet, com) )) {
      credits2str( buf, planet_commodityPrice(land_planet, com) - player.p->credits, 2 );
      land_errDialogueBuild("You need %s more credits.", buf );
      failure = 1;
   }
   if (pilot_cargoFree(player.p) == 0) {
      land_errDialogueBuild("You don't have any space!");
      failure = 1;
   }

   return !failure;
}

static int commodity_canSell( char *name )
{
   int failure;
   Commodity *com;

   failure = 0;
   com = commodity_get( name );

   if (pilot_cargoOwned( player.p, name ) <= 0) {
      land_errDialogueBuild("You don't have any %s", com->name );
      failure = 1;
   }

   return !failure;
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
   unsigned int q;
   credits_t price;
   HookParam hparam[3];

   comname = toolkit_getList( wid, "lstGoods" );

   /* Check if you can make the transaction */
   if (land_errDialogue( comname, "buyCommodity" ))
      return;

   /* Get selected. */
   com   = commodity_get( comname );
   q = commodity_getMod();
   q = (q*planet_commodityPrice(land_planet, com) < player_modCredits(0) ) ? q : player_modCredits(0)/(credits_t)planet_commodityPrice(land_planet, com);
   q = (q <= (unsigned int) pilot_cargoFree(player.p)) ? q : (unsigned int) pilot_cargoFree(player.p);
   price = q*planet_commodityPrice(land_planet, com);

   /* Make the buy. */
   q = pilot_cargoAdd( player.p, com, q );
   player_modCredits( -price );
   land_checkAddRefuel();
   commodity_update(wid, NULL);

   /* Run hooks. */
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = comname;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = q;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "comm_buy", hparam );
   if (land_takeoff)
      takeoff(1);
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
   unsigned int q;
   credits_t price;
   HookParam hparam[3];

   comname = toolkit_getList( wid, "lstGoods" );

   /* Check if you can make the transaction. */
   if (land_errDialogue( comname, "sellCommodity" ))
      return;

   /* Get parameters. */
   q = (commodity_getMod() < pilot_cargoOwned( player.p, comname )) ? commodity_getMod() : pilot_cargoOwned( player.p, comname );
   com   = commodity_get( comname );
   price = q*planet_commodityPrice(land_planet, com);

   /* Remove commodity. */
   q = pilot_cargoRm( player.p, com, q );
   player_modCredits( price );
   land_checkAddRefuel();

   commodity_update(wid, NULL);

   /* Run hooks. */
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = comname;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = q;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "comm_sell", hparam );
   if (land_takeoff)
      takeoff(1);
}

/**
 * @brief Gets the current modifier status.
 *    @return The amount modifier when buying or selling commodities.
 */
int commodity_getMod (void)
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
 * @brief Renders the commodity buying modifier.
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width to render at.
 *    @param h Height to render at.
 */
static void commodity_renderMod( double bx, double by, double w, double h, void *data )
{
   (void) data;
   (void) h;
   int bq, sq; /* buy q, sell q */
   char buf[64];
   credits_t price_tobuy;
   credits_t price_tosell;
   char *comm_name = toolkit_getList( land_getWid(LAND_WINDOW_COMMODITY), "lstGoods" );

      /* amount buying is max( num can afford, available space, modifier  ) */
   if (strcmp(comm_name, "None")!=0){
      Commodity *comm = commodity_get( comm_name );
      bq = commodity_getMod();
      bq = (bq*planet_commodityPrice(land_planet, comm) < player_modCredits(0) ) ? bq : (int)(player_modCredits(0)/planet_commodityPrice(land_planet, comm));
      bq = (bq <= pilot_cargoFree(player.p)) ? bq : pilot_cargoFree(player.p);
      sq =  (commodity_getMod() < pilot_cargoOwned( player.p, comm_name )) ? commodity_getMod() : pilot_cargoOwned( player.p, comm_name );
      if (bq != commodity_mod) {
         commodity_update( land_getWid(LAND_WINDOW_COMMODITY), NULL );
         commodity_mod = bq;
      }
      price_tobuy = bq*planet_commodityPrice(land_planet, comm);
      price_tosell = sq*planet_commodityPrice(land_planet, comm);
   } 
   else
      bq = sq = price_tobuy = price_tosell = 0;

   nsnprintf( buf, 64, "buy:%d tons, sell:%d tons",bq,sq);
   gl_printMid( &gl_smallFont, w, bx, by+35, &cBlack, buf );
   nsnprintf( buf, 64, "buy price:%lli", price_tobuy );
   gl_printMid( &gl_smallFont, w, bx, by+20, &cBlack, buf );
   nsnprintf( buf, 64, "sell price:%lli", price_tosell );
   gl_printMid( &gl_smallFont, w, bx, by+5, &cBlack, buf );
}

/**
 * @brief Makes sure it's sane to change ships in the equipment view.
 *    @param shipname Ship being changed to.
 */
int can_swapEquipment( char* shipname )
{
   int failure = 0;
   char *loc = player_getLoc(shipname);
   Pilot *newship;
   newship = player_getShip(shipname);

   if (strcmp(shipname,player.p->name)==0) { /* Already onboard. */
      land_errDialogueBuild( "You're already onboard the %s.", shipname );
      failure = 1;
   }
   if (strcmp(loc,land_planet->name)) { /* Ship isn't here. */
      dialogue_alert( "You must transport the ship to %s to be able to get in.",
            land_planet->name );
      failure = 1;
   }
   if (pilot_cargoUsed(player.p) > (pilot_cargoFree(newship) + pilot_cargoUsed(newship))) { /* Current ship has too much cargo. */
      land_errDialogueBuild( "You have %d tons more cargo than the new ship can hold.",
            pilot_cargoUsed(player.p) - pilot_cargoFree(newship), shipname );
      failure = 1;
   }
   if (pilot_hasDeployed(player.p)) { /* Escorts are in space. */
      land_errDialogueBuild( "You can't strand your fighters in space.");
      failure = 1;
   }
   return !failure;
}


/**
 * @brief Generates error dialogues used by several landing tabs.
 *    @param name Name of the ship, outfit or commodity being acted upon.
 *    @param type Type of action.
 */
int land_errDialogue( char* name, char* type )
{
   errorlist_ptr = NULL;
   if (strcmp(type,"tradeShip")==0)
      shipyard_canTrade( name );
   else if (strcmp(type,"buyShip")==0)
      shipyard_canBuy( name );
   else if (strcmp(type,"swapEquipment")==0)
      can_swapEquipment( name );
   else if (strcmp(type,"swap")==0)
      can_swap( name );
   else if (strcmp(type,"sellShip")==0)
      can_sell( name );
   else if (strcmp(type,"buyOutfit")==0)
      outfit_canBuy( name );
   else if (strcmp(type,"sellOutfit")==0)
      outfit_canSell( name );
   else if (strcmp(type,"buyCommodity")==0)
      commodity_canBuy( name );
   else if (strcmp(type,"sellCommodity")==0)
      commodity_canSell( name );
   if (errorlist_ptr != NULL) {
      dialogue_alert( "%s", errorlist );
      return 1;
   }
   return 0;
}

/**
 * @brief Generates error dialogues used by several landing tabs.
 *    @param fmt String with printf-like formatting
 */
void land_errDialogueBuild( const char *fmt, ... )
{
   va_list ap;

   if (fmt == NULL)
      return;
   else { /* get the message */
      va_start(ap, fmt);
      vsnprintf(errorreason, 512, fmt, ap);
      va_end(ap);
   }

   if (errorlist_ptr == NULL) /* Initialize on first run. */
      errorappend = nsnprintf( errorlist, sizeof(errorlist), "%s", errorreason );
   else /* Append newest error to the existing list. */
      nsnprintf( &errorlist[errorappend],  sizeof(errorlist)-errorappend, "\n%s", errorreason );
   errorlist_ptr = errorlist;
}


/**
 * @brief Gets the dimensions of the spaceport bar window.
 */
static void bar_getDim( int wid,
      int *w, int *h, int *iw, int *ih, int *bw, int *bh )
{
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Calculate dimensions of portraits. */
   *iw = 300 + (*w - 800);
   *ih = *h - 60;

   /* Calculate button dimensions. */
   *bw = (*w - *iw - 80)/2;
   *bh = LAND_BUTTON_HEIGHT;
}
/**
 * @brief Opens the spaceport bar window.
 */
static void bar_open( unsigned int wid )
{
   int w, h, iw, ih, bw, bh, dh, th;

   /* Set window functions. */
   window_onClose( wid, bar_close );

   /* Get dimensions. */
   bar_getDim( wid, &w, &h, &iw, &ih, &bw, &bh );
   dh = gl_printHeightRaw( &gl_smallFont, w - iw - 60, land_planet->bar_description );

   /* Buttons */
   window_addButtonKey( wid, -20, 20,
         bw, bh, "btnCloseBar",
         "Take Off", land_buttonTakeoff, SDLK_t );
   window_addButtonKey( wid, -20 - bw - 20, 20,
         bw, bh, "btnApproach",
         "Approach", bar_approach, SDLK_a );

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
   bar_genList( wid );
   /* Set default keyboard focuse to the list */
   window_setFocus( wid , "iarMissions" );
}

/**
 * @brief Generates the mission list for the bar.
 *
 *    @param wid Window to create mission list for.
 */
static int bar_genList( unsigned int wid )
{
   glTexture **portraits;
   char **names, *focused;
   int w, h, iw, ih, bw, bh;
   int n;

   /* Get dimensions. */
   bar_getDim( wid, &w, &h, &iw, &ih, &bw, &bh );

   /* Save focus. */
   focused = strdup(window_getFocus(wid));

   /* Destroy widget if already exists. */
   if (widget_exists( wid, "iarMissions" ))
      window_destroyWidget( wid, "iarMissions" );

   /* We sort just in case. */
   npc_sort();

   /* Set up missions. */
   if (mission_portrait == NULL)
      mission_portrait = gl_newImage( PORTRAIT_GFX_PATH"news.png", 0 );
   n = npc_getArraySize();
   if (n <= 0) {
      n            = 1;
      portraits    = malloc(sizeof(glTexture*));
      portraits[0] = mission_portrait;
      names        = malloc(sizeof(char*));
      names[0]     = strdup("News");
   }
   else {
      n            = n+1;
      portraits    = malloc( sizeof(glTexture*) * n );
      portraits[0] = mission_portrait;
      npc_getTextureArray( &portraits[1], n-1 );
      names        = malloc( sizeof(char*) * n );
      names[0]     = strdup("News");
      npc_getNameArray( &names[1], n-1 );
   }
   window_addImageArray( wid, 20, -40,
         iw, ih, "iarMissions", 100, 75,
         portraits, names, n, bar_update, NULL );

   /* write the outfits stuff */
   bar_update( wid, NULL );

   /* Restore focus. */
   window_setFocus( wid, focused );
   free(focused);

   return 0;
}
/**
 * @brief Regenerates the bar list.
 */
void bar_regen (void)
{
   if (!landed)
      return;
   bar_genList( land_getWid(LAND_WINDOW_BAR) );
}
/**
 * @brief Updates the missions in the spaceport bar.
 *    @param wid Window to update the outfits in.
 *    @param str Unused.
 */
static void bar_update( unsigned int wid, char* str )
{
   (void) str;
   int pos;
   int w, h, iw, ih, bw, bh, dh;

   /* Get dimensions. */
   bar_getDim( wid, &w, &h, &iw, &ih, &bw, &bh );
   dh = gl_printHeightRaw( &gl_smallFont, w - iw - 60, land_planet->bar_description );

   /* Get array. */
   pos = toolkit_getImageArrayPos( wid, "iarMissions" );

   /* See if is news. */
   if (pos==0) { /* News selected. */
      /* Destroy news widget if needed. */
      if (widget_exists(wid, "cstNews"))
         window_destroyWidget( wid, "cstNews" );

      /* Destroy portrait. */
      if (widget_exists(wid, "imgPortrait"))
         window_destroyWidget(wid, "imgPortrait");

      /* Disable button. */
      window_disableButton( wid, "btnApproach" );

      /* Clear text. */
      window_modifyText(  wid, "txtPortrait", NULL );
      window_modifyText(  wid, "txtMission",  NULL );

      /* Create news. */
      news_widget( wid, iw + 60, -40 - (40 + dh),
            w - iw - 100, h - 40 - (dh+20) - 40 - bh - 20 );
      return;
   }

   /* Shift to ignore news now. */
   pos--;

   /* Destroy news widget if needed. */
   if (widget_exists(wid, "cstNews"))
      window_destroyWidget( wid, "cstNews" );

   /* Create widgets if needed. */
   if (!widget_exists(wid, "imgPortrait"))
      window_addImage( wid, iw + 40 + (w-iw-60-PORTRAIT_WIDTH)/2,
            -(40 + dh + 40 + gl_defFont.h + 20 + PORTRAIT_HEIGHT),
            0, 0, "imgPortrait", NULL, 1 );

   /* Enable button. */
   window_enableButton( wid, "btnApproach" );

   /* Set portrait. */
   window_modifyText(  wid, "txtPortrait", npc_getName( pos ) );
   window_modifyImage( wid, "imgPortrait", npc_getTexture( pos ), 0, 0 );

   /* Set mission description. */
   window_modifyText(  wid, "txtMission", npc_getDesc( pos ));
}
/**
 * @brief Closes the mission computer window.
 *    @param wid Window to close.
 *    @param name Unused.
 */
static void bar_close( unsigned int wid, char *name )
{
   (void) wid;
   (void) name;

   /* Must not be regenerating. */
   if (land_regen) {
      land_regen--;
      return;
   }

   if (mission_portrait != NULL)
      gl_freeTexture(mission_portrait);
   mission_portrait = NULL;
}
/**
 * @brief Approaches guy in mission computer.
 */
static void bar_approach( unsigned int wid, char *str )
{
   (void) str;
   int pos, n;

   /* Get position. */
   pos = toolkit_getImageArrayPos( wid, "iarMissions" );

   /* Should never happen, but in case news is selected */
   if (pos == 0)
      return;

   /* Ignore news. */
   pos--;

   n = npc_getArraySize();
   npc_approach( pos );
   bar_genList( wid ); /* Always just in case. */
   if (n == npc_getArraySize())
      toolkit_setImageArrayPos( wid, "iarMissions", pos+1 );

   /* Reset markers. */
   mission_sysMark();

   /* Mission forced take off. */
   if (land_takeoff)
      takeoff(0);
}
/**
 * @brief Loads the news.
 *
 * @return 0 on success.
 */
static int news_load (void)
{
   generate_news(faction_name(land_planet->faction));
   return 0;
}



/**
 * @brief Opens the mission computer window.
 */
static void misn_open( unsigned int wid )
{
   int w, h;
   int y;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Set window functions. */
   window_onClose( wid, misn_close );

   /* buttons */
   window_addButtonKey( wid, -20, 20,
         LAND_BUTTON_WIDTH,LAND_BUTTON_HEIGHT, "btnCloseMission",
         "Take Off", land_buttonTakeoff, SDLK_t );
   window_addButtonKey( wid, -20, 40+LAND_BUTTON_HEIGHT,
         LAND_BUTTON_WIDTH,LAND_BUTTON_HEIGHT, "btnAcceptMission",
         "Accept Mission", misn_accept, SDLK_a );

   /* text */
   y = -60;
   window_addText( wid, w/2 + 10, y,
         w/2 - 30, 40, 0,
         "txtSDate", NULL, &cDConsole,
         "Date:\n"
         "Free Space:");
   window_addText( wid, w/2 + 110, y,
         w/2 - 90, 40, 0,
         "txtDate", NULL, &cBlack, NULL );
   y -= 2 * gl_defFont.h + 50;
   window_addText( wid, w/2 + 10, y,
         w/2 - 30, 20, 0,
         "txtSReward", &gl_smallFont, &cDConsole, "Reward:" );
   window_addText( wid, w/2 + 70, y,
         w/2 - 90, 20, 0,
         "txtReward", &gl_smallFont, &cBlack, NULL );
   y -= 20;
   window_addText( wid, w/2 + 10, y,
         w/2 - 30, h/2-90, 0,
         "txtDesc", &gl_smallFont, &cBlack, NULL );

   /* map */
   map_show( wid, 20, 20,
         w/2 - 30, h/2 - 35, 0.75 );

   misn_genList(wid, 1);
   /* Set default keyboard focuse to the list */
   window_setFocus( wid , "lstMission" );
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
   int i, ret;

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
      ret = mission_accept( misn );
      if ((ret==0) || (ret==2) || (ret==-1)) { /* success in accepting the mission */
         if (ret==-1)
            mission_cleanup( &mission_computer[pos] );
         memmove( &mission_computer[pos], &mission_computer[pos+1],
               sizeof(Mission) * (mission_ncomputer-pos-1) );
         mission_ncomputer--;

         /* Regenerate list. */
         misn_genList(wid, 0);
         /* Add position persistancey after a mission has been accepted */
         /* NOTE: toolkit_setListPos protects us from a bad position by clamping */
         toolkit_setListPos( wid, "lstMission", pos-1 ); /*looks better without the -1, makes more sense with*/
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
   char** misn_names, *focused;
   int w,h;

   /* Save focus. */
   focused = strdup(window_getFocus(wid));

   if (!first)
      window_destroyWidget( wid, "lstMission" );

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* list */
   j = 1; /* make sure we don't accidentally free the memory twice. */
   misn_names = NULL;
   if (mission_ncomputer > 0) { /* there are missions */
      misn_names = malloc(sizeof(char*) * mission_ncomputer);
      j = 0;
      for (i=0; i<mission_ncomputer; i++)
         if (mission_computer[i].title != NULL)
            misn_names[j++] = strdup(mission_computer[i].title);
   }
   if ((misn_names==NULL) || (mission_ncomputer==0) || (j==0)) { /* no missions. */
      if (j==0)
         free(misn_names);
      misn_names = malloc(sizeof(char*));
      misn_names[0] = strdup("No Missions");
      j = 1;
   }
   window_addList( wid, 20, -40,
         w/2 - 30, h/2 - 35,
         "lstMission", misn_names, j, 0, misn_update );

   /* Restore focus. */
   window_setFocus( wid, focused );
   free(focused);
   /* duplicateed the save focus functionaility from the bar */
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
   char txt[256], *buf;

   /* Clear computer markers. */
   space_clearComputerMarkers();

   /* Update date stuff. */
   buf = ntime_pretty( 0, 2 );
   nsnprintf( txt, sizeof(txt), "%s\n%d Tons", buf, player.p->cargo_free );
   free(buf);
   window_modifyText( wid, "txtDate", txt );

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
   if (misn->markers != NULL)
      map_center( system_getIndex( misn->markers[0].sys )->name );
   window_modifyText( wid, "txtReward", misn->reward );
   window_modifyText( wid, "txtDesc", misn->desc );
   window_enableButton( wid, "btnAcceptMission" );
}


/**
 * @brief Gets how much it will cost to refuel the player.
 *    @return Refuel price.
 */
static credits_t refuel_price (void)
{
   return (credits_t)((player.p->fuel_max - player.p->fuel)*3);
}


/**
 * @brief Refuels the player.
 *    @param wid Land window.
 *    @param str Unused.
 */
static void spaceport_refuel( unsigned int wid, char *str )
{
   (void)str;
   credits_t price;

   price = refuel_price();

   if (!player_hasCredits( price )) { /* player is out of money after landing */
      dialogue_alert("You seem to not have enough credits to refuel your ship." );
      return;
   }

   player_modCredits( -price );
   player.p->fuel      = player.p->fuel_max;
   if (widget_exists( land_windows[0], "btnRefuel" )) {
      window_destroyWidget( wid, "btnRefuel" );
      window_destroyWidget( wid, "txtRefuel" );
   }
}


/**
 * @brief Checks if should add the refuel button and does if needed.
 */
void land_checkAddRefuel (void)
{
   char buf[ECON_CRED_STRLEN], cred[ECON_CRED_STRLEN];
   unsigned int w;

   /* Check to see if fuel conditions are met. */
   if (!planet_hasService(land_planet, PLANET_SERVICE_REFUEL)) {
      if (!widget_exists( land_windows[0], "txtRefuel" ))
         window_addText( land_windows[0], -20, 20 + (LAND_BUTTON_HEIGHT + 20) + 20,
                  200, gl_defFont.h, 1, "txtRefuel",
                  &gl_defFont, &cBlack, "No refueling services." );
      return;
   }

   /* Full fuel. */
   if (player.p->fuel >= player.p->fuel_max) {
      if (widget_exists( land_windows[0], "btnRefuel" ))
         window_destroyWidget( land_windows[0], "btnRefuel" );
      if (widget_exists( land_windows[0], "txtRefuel" ))
         window_destroyWidget( land_windows[0], "txtRefuel" );
      return;
   }

   /* Autorefuel. */
   if (conf.autorefuel) {
      spaceport_refuel( land_windows[0], "btnRefuel" );
      w = land_getWid( LAND_WINDOW_EQUIPMENT );
      if (w > 0)
         equipment_updateShips( w, NULL ); /* Must update counter. */
      if (player.p->fuel >= player.p->fuel_max)
         return;
   }

   /* Just enable button if it exists. */
   if (widget_exists( land_windows[0], "btnRefuel" )) {
      window_enableButton( land_windows[0], "btnRefuel");
      credits2str( cred, player.p->credits, 2 );
      nsnprintf( buf, sizeof(buf), "Credits: %s", cred );
      window_modifyText( land_windows[0], "txtRefuel", buf );
   }
   /* Else create it. */
   else {
      /* Refuel button. */
      credits2str( cred, refuel_price(), 2 );
      nsnprintf( buf, sizeof(buf), "Refuel %s", cred );
      window_addButton( land_windows[0], -20, 20 + (LAND_BUTTON_HEIGHT + 20),
            LAND_BUTTON_WIDTH,LAND_BUTTON_HEIGHT, "btnRefuel",
            buf, spaceport_refuel );
      /* Player credits. */
      credits2str( cred, player.p->credits, 2 );
      nsnprintf( buf, sizeof(buf), "Credits: %s", cred );
      window_addText( land_windows[0], -20, 20 + 2*(LAND_BUTTON_HEIGHT + 20),
            LAND_BUTTON_WIDTH, gl_smallFont.h, 1, "txtRefuel",
            &gl_smallFont, &cBlack, buf );
   }

   /* Make sure player can click it. */
   if (!player_hasCredits( refuel_price() ))
      window_disableButton( land_windows[0], "btnRefuel" );
}


/**
 * @brief Wrapper for takeoff mission button.
 *
 *    @param wid Window causing takeoff.
 *    @param unused Unused.
 */
void land_buttonTakeoff( unsigned int wid, char *unused )
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

   /* Must not be regenerating. */
   if (land_regen) {
      land_regen--;
      return;
   }

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
unsigned int land_getWid( int window )
{
   if (land_windowsMap[window] == -1)
      return 0;
   return land_windows[ land_windowsMap[window] ];
}


/**
 * @brief Recreates the land windows.
 *
 *    @param load Is loading game?
 *    @param changetab Should it change to the last open tab?
 */
void land_genWindows( int load, int changetab )
{
   int i, j;
   const char *names[LAND_NUMWINDOWS];
   int w, h;
   Planet *p;
   int regen;

   /* Destroy old window if exists. */
   if (land_wid > 0) {
      land_regen = 2; /* Mark we're regenning. */
      window_destroy(land_wid);
   }
   land_loaded = 0;

   /* Get planet. */
   p     = land_planet;
   regen = landed;

   /* Create window. */
   if ((gl_screen.rw < 1024) || (gl_screen.rh < 768)) {
      w = -1; /* Fullscreen. */
      h = -1;
   }
   else {
      w = 800 + 0.5 * (SCREEN_W - 800);
      h = 600 + 0.5 * (SCREEN_H - 600);
   }
   land_wid = window_create( p->name, -1, -1, w, h );
   window_onClose( land_wid, land_cleanupWindow );

   /* Set window map to invalid. */
   for (i=0; i<LAND_NUMWINDOWS; i++)
      land_windowsMap[i] = -1;

   /* See what is available. */
   j = 0;
   /* Main. */
   land_windowsMap[LAND_WINDOW_MAIN] = j;
   names[j++] = land_windowNames[LAND_WINDOW_MAIN];
   /* Bar. */
   if (planet_hasService(land_planet, PLANET_SERVICE_BAR)) {
      land_windowsMap[LAND_WINDOW_BAR] = j;
      names[j++] = land_windowNames[LAND_WINDOW_BAR];
   }
   /* Missions. */
   if (planet_hasService(land_planet, PLANET_SERVICE_MISSIONS)) {
      land_windowsMap[LAND_WINDOW_MISSION] = j;
      names[j++] = land_windowNames[LAND_WINDOW_MISSION];
   }
   /* Outfits. */
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS)) {
      land_windowsMap[LAND_WINDOW_OUTFITS] = j;
      names[j++] = land_windowNames[LAND_WINDOW_OUTFITS];
   }
   /* Shipyard. */
   if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD)) {
      land_windowsMap[LAND_WINDOW_SHIPYARD] = j;
      names[j++] = land_windowNames[LAND_WINDOW_SHIPYARD];
   }
   /* Equipment. */
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS) ||
         planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD)) {
      land_windowsMap[LAND_WINDOW_EQUIPMENT] = j;
      names[j++] = land_windowNames[LAND_WINDOW_EQUIPMENT];
   }
   /* Commodity. */
   if (planet_hasService(land_planet, PLANET_SERVICE_COMMODITY)) {
      land_windowsMap[LAND_WINDOW_COMMODITY] = j;
      names[j++] = land_windowNames[LAND_WINDOW_COMMODITY];
   }

   /* Create tabbed window. */
   land_windows = window_addTabbedWindow( land_wid, -1, -1, -1, -1, "tabLand", j, names, 0 );

   /*
    * Order here is very important:
    *
    *  1) Create main tab - must have decent background.
    *  2) Set landed, play music and run land hooks - so hooks run well.
    *  3) Generate missions - so that campaigns are fluid.
    *  4) Create other tabs - lists depend on NPC and missions.
    */

   /* 1) Create main tab. */
   land_createMainTab( land_getWid(LAND_WINDOW_MAIN) );

   /* 2) Set as landed and run hooks. */
   if (!regen) {
      landed = 1;
      music_choose("land"); /* Must be before hooks in case hooks change music. */
      if (!load) {
         hooks_run("land");
      }
      events_trigger( EVENT_TRIGGER_LAND );

      /* 3) Generate computer and bar missions. */
      if (planet_hasService(land_planet, PLANET_SERVICE_MISSIONS))
         mission_computer = missions_genList( &mission_ncomputer,
               land_planet->faction, land_planet->name, cur_system->name,
               MIS_AVAIL_COMPUTER );
      if (planet_hasService(land_planet, PLANET_SERVICE_BAR))
         npc_generate(); /* Generate bar npc. */
   }

   /* 4) Create other tabs. */
   /* Basic - bar + missions */
   if (planet_hasService(land_planet, PLANET_SERVICE_BAR))
      bar_open( land_getWid(LAND_WINDOW_BAR) );
   if (planet_hasService(land_planet, PLANET_SERVICE_MISSIONS))
      misn_open( land_getWid(LAND_WINDOW_MISSION) );
   /* Outfits. */
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS))
      outfits_open( land_getWid(LAND_WINDOW_OUTFITS) );
   /* Shipyard. */
   if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
      shipyard_open( land_getWid(LAND_WINDOW_SHIPYARD) );
   /* Equipment. */
   if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS) ||
         planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
      equipment_open( land_getWid(LAND_WINDOW_EQUIPMENT) );
   /* Commodity. */
   if (planet_hasService(land_planet, PLANET_SERVICE_COMMODITY))
      commodity_exchange_open( land_getWid(LAND_WINDOW_COMMODITY) );

   if (!regen) {
      /* Reset markers if needed. */
      mission_sysMark();

      /* Check land missions. */
      if (!has_visited(VISITED_LAND)) {
         missions_run(MIS_AVAIL_LAND, land_planet->faction,
               land_planet->name, cur_system->name);
         visited(VISITED_LAND);
      }
   }

   /* Go to last open tab. */
   window_tabWinOnChange( land_wid, "tabLand", land_changeTab );
   if (changetab && land_windowsMap[ last_window ] != -1)
      window_tabWinSetActive( land_wid, "tabLand", land_windowsMap[ last_window ] );

   /* Add fuel button if needed - AFTER missions pay :). */
   land_checkAddRefuel();

   /* Finished loading. */
   land_loaded = 1;
}


/**
 * @brief Sets the land window tab.
 *
 *    @param Tab to set like LAND_WINDOW_COMMODITY.
 *    @return 0 on success.
 */
int land_setWindow( int window )
{
   if (land_windowsMap[ window ] < 0)
      return -1;
   window_tabWinSetActive( land_wid, "tabLand", land_windowsMap[window] );
   return 0;
}


/**
 * @brief Opens up all the land dialogue stuff.
 *    @param p Planet to open stuff for.
 *    @param load Whether or not loading the game.
 */
void land( Planet* p, int load )
{

   /* Do not land twice. */
   if (landed)
      return;

   /* Resets the player's heat. */
   pilot_heatReset( player.p );

   /* Heal the player so GUI shows player at full everything. */
   pilot_healLanded( player.p );

   /* Stop player sounds. */
   player_soundStop();

   /* Load stuff */
   land_planet = p;
   gfx_exterior = gl_newImage( p->gfx_exterior, 0 );

   /* Generate the news. */
   if (planet_hasService(land_planet, PLANET_SERVICE_BAR))
      news_load();
   /* refresh economy prices */
   econ_updateprices();

   /* Clear the NPC. */
   npc_clear();

   /* Create all the windows. */
   land_genWindows( load, 0 );

   /* Hack so that load can run player.takeoff(). */
   if (load)
      hooks_run( "load" );

   /* Mission forced take off. */
   if (land_takeoff)
      takeoff(0);
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
               0, 0, "imgFaction", logo, 0 );
         offset = 84;
      }
   }

   /*
    * Pretty display.
    */
   window_addImage( wid, 20, -40, 0, 0, "imgPlanet", gfx_exterior, 1 );
   window_addText( wid, 440, -20-offset,
         w-460, h-20-offset-60-LAND_BUTTON_HEIGHT*2, 0,
         "txtPlanetDesc", &gl_smallFont, &cBlack, land_planet->description);

   /*
    * buttons
    */
   /* first column */
   window_addButtonKey( wid, -20, 20,
         LAND_BUTTON_WIDTH, LAND_BUTTON_HEIGHT, "btnTakeoff",
         "Take Off", land_buttonTakeoff, SDLK_t );

   /*
    * Checkboxes.
    */
   window_addCheckbox( wid, -20, 20 + 2*(LAND_BUTTON_HEIGHT + 20) + 40,
         175, 20, "chkRefuel", "Automatic Refuel",
         land_toggleRefuel, conf.autorefuel );
   land_toggleRefuel( wid, "chkRefuel" );

   if (!land_planet->ncommodities)
      return;
}


/**
 * @brief Refuel was toggled.
 */
static void land_toggleRefuel( unsigned int wid, char *name )
{
   conf.autorefuel = window_checkboxState( wid, name );
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
   const char *torun_hook;
   unsigned int to_visit;

   /* Sane defaults. */
   torun_hook = NULL;
   to_visit   = 0;

   /* Find what switched. */
   for (i=0; i<LAND_NUMWINDOWS; i++) {
      if (land_windowsMap[i] == tab) {
         last_window = i;
         w = land_getWid( i );

         /* Must regenerate outfits. */
         switch (i) {
            case LAND_WINDOW_MAIN:
               land_checkAddRefuel();
               break;
            case LAND_WINDOW_OUTFITS:
               outfits_update( w, NULL );
               outfits_updateQuantities( w );
               to_visit   = VISITED_OUTFITS;
               torun_hook = "outfits";
               break;
            case LAND_WINDOW_SHIPYARD:
               shipyard_update( w, NULL );
               to_visit   = VISITED_SHIPYARD;
               torun_hook = "shipyard";
               break;
            case LAND_WINDOW_BAR:
               bar_update( w, NULL );
               to_visit   = VISITED_BAR;
               torun_hook = "bar";
               break;
            case LAND_WINDOW_MISSION:
               misn_update( w, NULL );
               to_visit   = VISITED_MISSION;
               torun_hook = "mission";
               break;
            case LAND_WINDOW_COMMODITY:
               commodity_update( w, NULL );
               to_visit   = VISITED_COMMODITY;
               torun_hook = "commodity";
               break;
            case LAND_WINDOW_EQUIPMENT:
               equipment_updateShips( w, NULL );
               equipment_updateOutfits( w, NULL );
               to_visit   = VISITED_EQUIPMENT;
               torun_hook = "equipment";
               break;

            default:
               break;
         }

         /* Clear markers if closing Mission Computer. */
         if (i != LAND_WINDOW_MISSION)
            space_clearComputerMarkers();

         break;
      }
   }

   /* Check land missions - always run hooks. */
   /*if ((to_visit != 0) && !has_visited(to_visit)) {*/
   {
      /* Run hooks, run after music in case hook wants to change music. */
      if (torun_hook != NULL)
         if (hooks_run( torun_hook ) > 0)
            bar_genList( land_getWid(LAND_WINDOW_BAR) );

      visited(to_visit);

      if (land_takeoff)
         takeoff(1);
   }
}


/**
 * @brief Makes the player take off if landed.
 *
 *    @param delay Whether or not to have time pass as if the player landed normally.
 */
void takeoff( int delay )
{
   int h;
   char *nt;
   double a, r;

   if (!landed)
      return;

   /* Player's ship is not able to fly. */
   if (!player_canTakeoff()) {
      char message[512];
      pilot_reportSpaceworthy( player.p, message, sizeof(message) );
      dialogue_msg( "Ship not fit for flight", message );
      return;
   }

   /* Clear queued takeoff. */
   land_takeoff = 0;

   /* Refuel if needed. */
   land_checkAddRefuel();

   /* In case we had paused messy sounds. */
   sound_stopAll();

   /* ze music */
   music_choose("takeoff");

   /* to randomize the takeoff a bit */
   a = RNGF() * 2. * M_PI;
   r = RNGF() * land_planet->radius;

   /* no longer authorized to land */
   player_rmFlag(PLAYER_LANDACK);
   pilot_rmFlag(player.p,PILOT_LANDING); /* No longer landing. */

   /* set player to another position with random facing direction and no vel */
   player_warp( land_planet->pos.x + r * cos(a), land_planet->pos.y + r * sin(a) );
   vect_pset( &player.p->solid->vel, 0., 0. );
   player.p->solid->dir = RNGF() * 2. * M_PI;
   cam_setTargetPilot( player.p->id, 0 );

   /* heal the player */
   player.p->armour = player.p->armour_max;
   player.p->shield = player.p->shield_max;
   player.p->energy = player.p->energy_max;
   player.p->stimer = 0.;

   /* initialize the new space */
   h = player.p->nav_hyperspace;
   space_init(NULL);
   player.p->nav_hyperspace = h;

   /* cleanup */
   if (save_all() < 0) /* must be before cleaning up planet */
      dialogue_alert( "Failed to save game! You should exit and check the log to see what happened and then file a bug report!" );

   /* time goes by, triggers hook before takeoff */
   if (delay)
      ntime_inc( ntime_create( 0, 1, 0 ) ); /* 1 STP */
   nt = ntime_pretty( 0, 2 );
   player_message("\epTaking off from %s on %s.", land_planet->name, nt);
   free(nt);

   /* Hooks and stuff. */
   land_cleanup(); /* Cleanup stuff */
   hooks_run("takeoff"); /* Must be run after cleanup since we don't want the
                            missions to think we are landed. */
   if (menu_isOpen(MENU_MAIN))
      return;
   player_addEscorts();
   hooks_run("enter");
   if (menu_isOpen(MENU_MAIN))
      return;
   events_trigger( EVENT_TRIGGER_ENTER );
   missions_run( MIS_AVAIL_SPACE, -1, NULL, NULL );
   if (menu_isOpen(MENU_MAIN))
      return;
   player.p->ptimer = PILOT_TAKEOFF_DELAY;
   pilot_setFlag( player.p, PILOT_TAKEOFF );
   pilot_setThrust( player.p, 0. );
   pilot_setTurn( player.p, 0. );
}


/**
 * @brief Cleans up some land-related variables.
 */
void land_cleanup (void)
{
   int i;

   /* Clean up default stuff. */
   land_regen     = 0;
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
   npc_freeAll();
}


/**
 * @brief Exits all the landing stuff.
 */
void land_exit (void)
{
   land_cleanup();
   equipment_cleanup();
}


