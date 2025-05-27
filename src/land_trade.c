/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file land_trade.c
 *
 * @brief Handles the Trading Center at land.
 */
/** @cond */
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "land_trade.h"

#include "array.h"
#include "commodity.h"
#include "economy.h"
#include "hook.h"
#include "land.h"
#include "nstring.h"
#include "player.h"
#include "player_fleet.h"
#include "space.h"
#include "toolkit.h"

typedef struct CommodityItem {
   Commodity *com;
   double     price_mod;
} CommodityItem;

/*
 * Quantity to buy on one click
 */
static int commodity_mod =
   10; /**< Amount you can buy or sell in a single click. */
static CommodityItem *commodity_list = NULL; /* Array.h */

/* Prototypes. */
static void commodity_exchange_genList( unsigned int wid );

static void commodity_exchange_modifiers( unsigned int wid )
{
   int q = commodity_getMod();
   if ( q != commodity_mod ) {
      char buf[STRMAX_SHORT];
      commodity_mod = q;
      snprintf( buf, sizeof( buf ), _( "Buy (%d %s)" ), q, UNIT_MASS );
      window_buttonCaption( wid, "btnCommodityBuy", buf );
      snprintf( buf, sizeof( buf ), _( "Sell (%d %s)" ), q, UNIT_MASS );
      window_buttonCaption( wid, "btnCommoditySell", buf );
      toolkit_rerender();
   }
}

static int commodity_exchange_events( unsigned int wid, SDL_Event *evt )
{
   if ( ( evt->type == SDL_KEYDOWN ) || ( evt->type == SDL_KEYUP ) )
      commodity_exchange_modifiers( wid );
   return 0;
}

/**
 * @brief Opens the local market window.
 */
void commodity_exchange_open( unsigned int wid )
{
   int    w, h, iw, dw, bw, titleHeight, infoHeight;
   char   buf[STRMAX_SHORT];
   size_t l = 0;
   int    q = commodity_getMod();

   /* Mark as generated. */
   land_tabGenerate( LAND_WINDOW_COMMODITY );

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   /* Window size minus right column size minus space on left and right */
   iw = 565 + ( w - LAND_WIDTH );
   dw = w - iw - 60;

   /* buttons */
   bw = MIN( LAND_BUTTON_WIDTH, ( dw - 40 ) / 3 );
   snprintf( buf, sizeof( buf ), _( "Buy (%d %s)" ), q, UNIT_MASS );
   window_addButtonKey( wid, 40 + iw, 20, bw, LAND_BUTTON_HEIGHT,
                        "btnCommodityBuy", buf, commodity_buy, SDLK_b );
   snprintf( buf, sizeof( buf ), _( "Sell (%d %s)" ), q, UNIT_MASS );
   window_addButtonKey( wid, 60 + iw + bw, 20, bw, LAND_BUTTON_HEIGHT,
                        "btnCommoditySell", buf, commodity_sell, SDLK_s );
   window_addButtonKey( wid, 80 + iw + 2 * bw, 20, bw, LAND_BUTTON_HEIGHT,
                        "btnCommodityClose", _( "Take Off" ),
                        land_buttonTakeoff, SDLK_t );

   /* handle multipliers. */
   window_handleEvents( wid, commodity_exchange_events );
   window_setOnFocus( wid, commodity_exchange_modifiers );

   /* store gfx */
   window_addRect( wid, -20, -40, 192, 192, "rctStore", &cBlack, 0 );
   window_addImage( wid, -20, -40, 192, 192, "imgStore", NULL, 1 );

   /* text */
   titleHeight =
      gl_printHeightRaw( &gl_defFont, LAND_BUTTON_WIDTH + 80, _( "None" ) );
   window_addText( wid, 40 + iw, -40, dw, titleHeight, 0, "txtName",
                   &gl_defFont, NULL, _( "None" ) );

   l += scnprintf( &buf[l], sizeof( buf ) - l, "%s",
                   ( player.fleet_capacity > 0 ) ? _( "Your fleet has:" )
                                                 : _( "You have:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s",
                   ( player.fleet_capacity > 0 ) ? _( "Free Space (fleet):" )
                                                 : _( "Free Space:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", _( "Money:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", _( "Market Price:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s",
                   _( "Average price here:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s",
                   _( "Average galactic price:" ) );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", _( "Purchased for:" ) );
   infoHeight = gl_printHeightRaw( &gl_smallFont, LAND_BUTTON_WIDTH + 80, buf );
   window_addText( wid, 40 + iw, -60 - titleHeight, 200, infoHeight, 0,
                   "txtSInfo", &gl_smallFont, &cFontGrey, buf );
   window_addText( wid, 40 + iw + 224, -60 - titleHeight,
                   dw - ( 200 + 20 + 192 ), infoHeight, 0, "txtDInfo",
                   &gl_smallFont, NULL, NULL );

   window_addText( wid, 40 + iw, -80 - titleHeight - infoHeight, dw, 100, 0,
                   "txtDRef", &gl_smallFont, NULL, NULL );
   window_addText(
      wid, 40 + iw, MIN( -80 - titleHeight - infoHeight, -192 - 60 ), dw,
      h - ( 80 + titleHeight + infoHeight ) - ( 40 + LAND_BUTTON_HEIGHT ), 0,
      "txtDesc", &gl_smallFont, NULL, NULL );

   /* Generate list. */
   commodity_exchange_genList( wid );

   /* Set default keyboard focuse to the list */
   window_setFocus( wid, "iarTrade" );
}

static void commodity_exchange_genList( unsigned int wid )
{
   iar_data_t      idat;
   int             exists = widget_exists( wid, "iarTrade" );
   ImageArrayCell *cgoods;
   int             w, h, iw, ih, iconsize, ngoods;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   /* Window size minus right column size minus space on left and right */
   iw = 565 + ( w - LAND_WIDTH );
   ih = h - 60;

   double     *prices = NULL;
   Commodity **tech   = tech_getCommodity( land_spob->tech, &prices );

   if ( exists ) {
      toolkit_saveImageArrayData( wid, "iarTrade", &idat );
      window_destroyWidget( wid, "iarTrade" );
   }

   if ( commodity_list != NULL )
      array_free( commodity_list );
   commodity_list = array_create( CommodityItem );

   /* First add special sellable. */
   PilotCommodity *pclist = pfleet_cargoList();
   for ( int i = 0; i < array_size( pclist ); i++ ) {
      PilotCommodity *pc = &pclist[i];
      if ( pc->id > 0 ) /* Ignore mission stuff. */
         continue;
      if ( !commodity_isFlag( pc->commodity, COMMODITY_FLAG_ALWAYS_CAN_SELL ) )
         continue;
      CommodityItem item = {
         .com       = (Commodity *)pc->commodity,
         .price_mod = 1.,
      };
      array_push_back( &commodity_list, item );
   }

   /* Add spob stuff. */
   for ( int i = 0; i < array_size( land_spob->commodities ); i++ ) {
      Commodity *com = land_spob->commodities[i];
      /* Ignore if already in the list. */
      int found = 0;
      for ( int k = 0; k < array_size( commodity_list ); k++ ) {
         if ( com == commodity_list[k].com ) {
            found = 1;
            break;
         }
      }
      if ( found )
         continue;
      CommodityItem item = {
         .com       = com,
         .price_mod = 1.,
      };
      array_push_back( &commodity_list, item );
   }

   /* Next add local specialties. */
   for ( int i = 0; i < array_size( tech ); i++ ) {
      Commodity *com = tech[i];
      /* Ignore if already in the list. */
      int found = 0;
      for ( int k = 0; k < array_size( commodity_list ); k++ ) {
         if ( com == commodity_list[k].com ) {
            if ( FABS( prices[i] - 1. ) > DOUBLE_TOL )
               commodity_list[k].price_mod =
                  prices[i]; /* Overwrite if necessary. */
            found = 1;
            break;
         }
      }
      if ( found )
         continue;
      CommodityItem item = {
         .com       = com,
         .price_mod = prices[i],
      };
      array_push_back( &commodity_list, item );
   }

   /* Now create the goods. */
   ngoods = array_size( commodity_list );
   if ( ngoods > 0 ) {
      cgoods = calloc( ngoods, sizeof( ImageArrayCell ) );
      for ( int i = 0; i < ngoods; i++ ) {
         const Commodity *com = commodity_list[i].com;
         cgoods[i].image      = gl_dupTexture( com->gfx_store );
         cgoods[i].caption    = strdup( _( com->name ) );
         cgoods[i].quantity   = pfleet_cargoOwned( com );
      }
   } else {
      ngoods            = 1;
      cgoods            = calloc( ngoods, sizeof( ImageArrayCell ) );
      cgoods[0].image   = NULL;
      cgoods[0].caption = strdup( _( "None" ) );
      commodity_list    = NULL;
   }
   array_free( pclist );

   /* set up the goods to buy/sell */
   iconsize = 128;
   if ( !conf.big_icons ) {
      if ( toolkit_simImageArrayVisibleElements( iw, ih, iconsize, iconsize ) <
           ngoods )
         iconsize = 96;
      if ( toolkit_simImageArrayVisibleElements( iw, ih, iconsize, iconsize ) <
           ngoods )
         iconsize = 64;
   }
   window_addImageArray( wid, 20, 20, iw, ih, "iarTrade", iconsize, iconsize,
                         cgoods, ngoods, commodity_update, commodity_update,
                         commodity_update );

   if ( exists )
      toolkit_loadImageArrayData( wid, "iarTrade", &idat );

   array_free( tech );
   array_free( prices );
}

void commodity_exchange_cleanup( void )
{
   array_free( commodity_list );
   commodity_list = NULL;
}

/**
 * @brief Updates the commodity window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void commodity_update( unsigned int wid, const char *str )
{
   (void)str;
   char   buf[STRMAX];
   char   buf_purchase_price[ECON_CRED_STRLEN], buf_credits[ECON_CRED_STRLEN];
   size_t l = 0;
   const Commodity *com;
   credits_t        mean, globalmean;
   double           std, globalstd;
   char buf_mean[ECON_CRED_STRLEN], buf_globalmean[ECON_CRED_STRLEN];
   char buf_std[ECON_CRED_STRLEN], buf_globalstd[ECON_CRED_STRLEN];
   char buf_local_price[ECON_CRED_STRLEN];
   char buf_tonnes_owned[ECON_MASS_STRLEN], buf_tonnes_free[ECON_MASS_STRLEN];
   int  owned, cargo_free;
   int  i = toolkit_getImageArrayPos( wid, "iarTrade" );
   credits2str( buf_credits, player.p->credits, 2 );
   cargo_free = pfleet_cargoFree();
   tonnes2str( buf_tonnes_free, cargo_free );

   if ( i < 0 || array_size( commodity_list ) == 0 ) {
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s", _( "N/A" ) );
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", buf_tonnes_free );
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", buf_credits );
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", "" );
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", _( "N/A" ) );
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", _( "N/A" ) );
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", _( "N/A" ) );
      window_modifyText( wid, "txtDInfo", buf );
      window_modifyText( wid, "txtDesc", _( "No commodities available." ) );
      window_disableButton( wid, "btnCommodityBuy" );
      window_disableButton( wid, "btnCommoditySell" );
      return;
   }
   com              = commodity_list[i].com;
   double price_mod = commodity_list[i].price_mod;

   /* modify image */
   window_modifyImage( wid, "imgStore", com->gfx_store, 192, 192 );

   spob_averageSpobPrice( land_spob, com, &mean, &std );
   credits2str( buf_mean, mean, -1 );
   snprintf( buf_std, sizeof( buf_std ), _( "%.1f ¤" ),
             std ); /* TODO credit2str could learn to do this... */
   economy_getAveragePrice( com, &globalmean, &globalstd );
   credits2str( buf_globalmean, globalmean, -1 );
   snprintf( buf_globalstd, sizeof( buf_globalstd ), _( "%.1f ¤" ),
             globalstd ); /* TODO credit2str could learn to do this... */
   /* modify text */
   buf_purchase_price[0] = '\0';
   owned                 = pfleet_cargoOwned( com );
   if ( owned > 0 )
      credits2str( buf_purchase_price, com->lastPurchasePrice, -1 );
   credits2str( buf_local_price,
                price_mod * spob_commodityPrice( land_spob, com ), -1 );
   tonnes2str( buf_tonnes_owned, owned );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "%s", buf_tonnes_owned );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", buf_tonnes_free );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", buf_credits );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n" );
   l += scnprintf( &buf[l], sizeof( buf ) - l, _( "%s/t" ), buf_local_price );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n" );
   l += scnprintf( &buf[l], sizeof( buf ) - l, _( "%s/t ± %s/t" ), buf_mean,
                   buf_std );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n" );
   l += scnprintf( &buf[l], sizeof( buf ) - l, _( "%s/t ± %s/t" ),
                   buf_globalmean, buf_globalstd );
   l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s", buf_purchase_price );

   window_modifyText( wid, "txtDInfo", buf );
   window_modifyText( wid, "txtName", _( com->name ) );
   l = 0;
   l += scnprintf( &buf[l], sizeof( buf ) - l, "%s", _( com->description ) );
   if ( array_size( com->illegalto ) > 0 ) {
      l += scnprintf( &buf[l], sizeof( buf ) - l, "\n\n%s",
                      _( "Illegalized by the following factions:\n" ) );
      for ( int j = 0; j < array_size( com->illegalto ); j++ ) {
         int f = com->illegalto[j];
         if ( !faction_isKnown( f ) )
            continue;

         l += scnprintf( &buf[l], sizeof( buf ) - l, _( "\n   - %s" ),
                         _( faction_name( f ) ) );
      }
   }
   window_modifyText( wid, "txtDesc", buf );

   /* Add relative price. */
   l = 0;
   if ( commodity_isFlag( com, COMMODITY_FLAG_PRICE_CONSTANT ) ) {
      l += scnprintf( &buf[l], sizeof( buf ) - l, _( "Price is constant." ) );
      window_modifyText( wid, "txtDRef", buf );
   } else if ( com->price_ref != NULL ) {
      char c = '0';
      if ( com->price_mod > 1. )
         c = 'g';
      else if ( com->price_mod < 1. )
         c = 'r';
      l += scnprintf(
         &buf[l], sizeof( buf ) - l,
         _( "Price is based on #%c%.0f%%#0 of the price of #o%s#0." ), c,
         com->price_mod * 100., _( com->price_ref ) );
      window_modifyText( wid, "txtDRef", buf );
   } else
      window_modifyText( wid, "txtDRef", NULL );

   /* Button enabling/disabling */
   if ( commodity_canBuy( com, price_mod ) )
      window_enableButton( wid, "btnCommodityBuy" );
   else
      window_disableButtonSoft( wid, "btnCommodityBuy" );

   if ( commodity_canSell( com ) )
      window_enableButton( wid, "btnCommoditySell" );
   else
      window_disableButtonSoft( wid, "btnCommoditySell" );
}

/**
 * @brief Checks to see if the player can buy a commodity.
 */
int commodity_canBuy( const Commodity *com, double price_mod )
{
   int          failure, incommodities;
   unsigned int q, price;
   char         buf[ECON_CRED_STRLEN];

   land_errClear();
   failure = 0;
   q       = commodity_getMod();
   price   = spob_commodityPrice( land_spob, com ) * q * price_mod;

   if ( !player_hasCredits( price ) ) {
      credits2str( buf, price - player.p->credits, 2 );
      land_errDialogueBuild( _( "You need %s more." ), buf );
      failure = 1;
   }
   if ( pfleet_cargoFree() <= 0 ) {
      land_errDialogueBuild( _( "No cargo space available!" ) );
      failure = 1;
   }

   incommodities = 0;
   for ( int i = 0; i < array_size( commodity_list ); i++ ) {
      if ( commodity_list[i].com == com ) {
         incommodities = 1;
         break;
      }
   }
   if ( !incommodities ) {
      land_errDialogueBuild( _( "%s is not sold here!" ), _( com->name ) );
      failure = 1;
   }

   return !failure;
}

/**
 * @brief Checks to see if a player can sell a commodity.
 */
int commodity_canSell( const Commodity *com )
{
   int failure = 0;
   land_errClear();
   if ( pfleet_cargoOwned( com ) == 0 ) {
      land_errDialogueBuild( _( "You can't sell something you don't have!" ) );
      failure = 1;
   }
   return !failure;
}

/**
 * @brief Buys the selected commodity.
 *    @param wid Window buying from.
 *    @param str Unused.
 */
void commodity_buy( unsigned int wid, const char *str )
{
   (void)str;
   int          i;
   Commodity   *com;
   unsigned int q;
   credits_t    price;
   HookParam    hparam[3];

   /* Get selected. */
   q     = commodity_getMod();
   i     = toolkit_getImageArrayPos( wid, "iarTrade" );
   com   = commodity_list[i].com;
   price = spob_commodityPrice( land_spob, com ) * commodity_list[i].price_mod;

   /* Check stuff. */
   if ( !commodity_canBuy( com, commodity_list[i].price_mod ) ) {
      land_errDisplay();
      return;
   }

   /* Make the buy. */
   q = pfleet_cargoAdd( com, q );
   com->lastPurchasePrice =
      price; /* To show the player how much they paid for it */
   price *= q;
   player_modCredits( -price );
   commodity_update( wid, NULL );
   commodity_exchange_genList( wid );

   /* Run hooks. */
   hparam[0].type        = HOOK_PARAM_COMMODITY;
   hparam[0].u.commodity = com;
   hparam[1].type        = HOOK_PARAM_NUMBER;
   hparam[1].u.num       = q;
   hparam[2].type        = HOOK_PARAM_SENTINEL;
   hooks_runParam( "comm_buy", hparam );
   land_needsTakeoff( 1 );
}

/**
 * @brief Attempts to sell a commodity.
 *    @param wid Window selling commodity from.
 *    @param str Unused.
 */
void commodity_sell( unsigned int wid, const char *str )
{
   (void)str;
   int          i;
   Commodity   *com;
   unsigned int q;
   credits_t    price;
   HookParam    hparam[3];

   /* Get parameters. */
   q     = commodity_getMod();
   i     = toolkit_getImageArrayPos( wid, "iarTrade" );
   com   = commodity_list[i].com;
   price = spob_commodityPrice( land_spob, com ) * commodity_list[i].price_mod;

   /* Check stuff. */
   if ( !commodity_canSell( com ) ) {
      land_errDisplay();
      return;
   }

   /* Remove commodity. */
   q     = pfleet_cargoRm( com, q, 0 );
   price = price * (credits_t)q;
   player_modCredits( price );
   if ( pfleet_cargoOwned( com ) == 0 ) /* None left, set purchase price to
                                           zero, in case missions add cargo. */
      com->lastPurchasePrice = 0;
   commodity_update( wid, NULL );
   commodity_exchange_genList( wid );

   /* Run hooks. */
   hparam[0].type        = HOOK_PARAM_COMMODITY;
   hparam[0].u.commodity = com;
   hparam[1].type        = HOOK_PARAM_NUMBER;
   hparam[1].u.num       = q;
   hparam[2].type        = HOOK_PARAM_SENTINEL;
   hooks_runParam( "comm_sell", hparam );
   land_needsTakeoff( 1 );
}

/**
 * @brief Gets the current modifier status.
 *    @return The amount modifier when buying or selling commodities.
 */
int commodity_getMod( void )
{
   SDL_Keymod mods = SDL_GetModState();
   int        q    = 10;
   if ( mods & ( KMOD_LCTRL | KMOD_RCTRL ) )
      q *= 5;
   if ( mods & ( KMOD_LSHIFT | KMOD_RSHIFT ) )
      q *= 10;
   if ( mods & ( KMOD_LALT | KMOD_RALT ) )
      q = 1;

   return q;
}

/**
 * @brief Renders the commodity buying modifier.
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width to render at.
 *    @param h Height to render at.
 *    @param data Unused.
 */
void commodity_renderMod( double bx, double by, double w, double h, void *data )
{
   (void)data;
   (void)h;
   int  q;
   char buf[8];

   q = commodity_getMod();
   if ( q != commodity_mod ) {
      // commodity_update( land_getWid(LAND_WINDOW_COMMODITY), NULL );
      commodity_mod = q;
   }
   snprintf( buf, sizeof( buf ), "%dx", q );
   gl_printMidRaw( &gl_smallFont, w, bx, by, &cFontWhite, -1, buf );
}
