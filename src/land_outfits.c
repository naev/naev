/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file land_outfits.c
 *
 * @brief Handles all the landing menus and actions.
 */
/** @cond */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "land_outfits.h"

#include "array.h"
#include "cond.h"
#include "dialogue.h"
#include "equipment.h"
#include "hook.h"
#include "log.h"
#include "map.h"
#include "map_find.h"
#include "nstring.h"
#include "nlua.h"
#include "outfit.h"
#include "pilot_outfit.h"
#include "player.h"
#include "player_gui.h"
#include "slots.h"
#include "space.h"
#include "toolkit.h"
#include "utf8.h"

#define  OUTFITS_IAR    "iarOutfits"
#define  OUTFITS_TAB    "tabOutfits"
#define  OUTFITS_FILTER "inpFilterOutfits"
#define  OUTFITS_NTABS  7

/**
 * @brief Data for handling the widget. */
typedef struct LandOutfitData_ {
   const Outfit **outfits; /**< Override list of outfits, otherwise uses landed_spob. */
   int blackmarket;        /**< Whether on not is considered a blackmarket. */
} LandOutfitData;

static iar_data_t iar_data[OUTFITS_NTABS]; /**< Stored image array positions. */
static Outfit **iar_outfits[OUTFITS_NTABS]; /**< C-array of Arrays: Outfits associated with the image array cells. */
static int outfit_Mode = 0; /**< Outfit mode for filtering. */
static PlayerOutfit_t *outfits_sold = NULL; /**< List of the outfits the player sold so they can buy them back. */

/* Modifier for buying and selling quantity. */
static int outfits_mod = 1;

/*
 * Helper functions.
 */
static void outfits_getSize( unsigned int wid, int *w, int *h,
      int *iw, int *ih, int *bw, int *bh );
static void outfits_buy( unsigned int wid, const char *str );
static void outfits_sell( unsigned int wid, const char *str );
static int outfits_getMod (void);
static void outfits_rmouse( unsigned int wid, const char* widget_name );
static void outfits_find( unsigned int wid, const char *str );
static const char *outfit_getPrice( const Outfit *outfit, credits_t *price, int *canbuy, int *cansell );
static void outfit_Popdown( unsigned int wid, const char* str );
static void outfits_genList( unsigned int wid );
static void outfits_changeTab( unsigned int wid, const char *wgt, int old, int tab );
static void outfits_onClose( unsigned int wid, const char *str );
static void outfit_modifiers( unsigned int wid );
static int outfit_events( unsigned int wid, SDL_Event *evt );

/**
 * @brief Gets the size of the outfits window.
 */
static void outfits_getSize( unsigned int wid, int *w, int *h,
      int *iw, int *ih, int *bw, int *bh )
{
   int padding;

   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Calculate image array dimensions. */
   if (iw != NULL)
      *iw = 680 + (*w - LAND_WIDTH);
   if (ih != NULL)
      *ih = *h - 60;

   /* Left padding + per-button padding * nbuttons */
   padding = 40 + 20 * 4;

   /* Calculate button dimensions. */
   if (bw != NULL)
      *bw = (*w - (iw!=NULL?*iw:0) - padding) / 4;
   if (bh != NULL)
      *bh = LAND_BUTTON_HEIGHT;
}

static void outfit_modifiers( unsigned int wid )
{
   int q = outfits_getMod();
   if (q != outfits_mod) {
      outfits_updateEquipmentOutfits();
      outfits_mod = q;
      if (q==1) {
         window_buttonCaption( wid, "btnBuyOutfit", _("Buy") );
         window_buttonCaption( wid, "btnSellOutfit", _("Sell") );
      }
      else {
         char buf[STRMAX_SHORT];
         snprintf( buf, sizeof(buf), _("Buy (%dx)"), q );
         window_buttonCaption( wid, "btnBuyOutfit", buf );
         snprintf( buf, sizeof(buf), _("Sell (%dx)"), q );
         window_buttonCaption( wid, "btnSellOutfit", buf );
      }
      toolkit_rerender();
   }
}

/**
 * Used to force rerenders.
 */
static int outfit_events( unsigned int wid, SDL_Event *evt )
{
   (void) evt; /* For constness warning. */
   if ((evt->type==SDL_KEYDOWN) || (evt->type==SDL_KEYUP))
      outfit_modifiers( wid );
   return 0;
}

/**
 * @brief For when the widget closes.
 */
static void outfits_onClose( unsigned int wid, const char *str )
{
   (void) str;
   LandOutfitData *data = window_getData( wid );
   if (data != NULL) {
      array_free( data->outfits );
      free( data );
   }
}

/**
 * @brief Opens the outfit exchange center window.
 *
 *    @param wid Window ID to open at.
 *    @param outfits Array (array.h): Outfits to sell. Will be freed.
 *                   Set to NULL if this is the landed player store.
 *    @param blackmarket Whether or not the outfit seller is a black market.
 */
void outfits_open( unsigned int wid, const Outfit **outfits, int blackmarket )
{
   int w, h, iw, ih, bw, bh, off;
   LandOutfitData *data = NULL;

   /* Clear sold outfits. */
   array_free( outfits_sold );
   outfits_sold = NULL;

   /* initialize the outfit mode. */
   outfit_Mode = 0;
   data = malloc( sizeof( LandOutfitData ) );
   data->outfits = outfits;
   data->blackmarket = blackmarket;
   window_setData( wid, data );
   window_onClose( wid, outfits_onClose );

   /* Mark as generated. */
   if (outfits==NULL)
      land_tabGenerate(LAND_WINDOW_OUTFITS);

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, &bw, &bh );

   /* Initialize stored positions. */
   for (int i=0; i<OUTFITS_NTABS; i++) {
      toolkit_initImageArrayData( &iar_data[i] );
      array_free( iar_outfits[i] );
   }
   memset( iar_outfits, 0, sizeof(Outfit**) * OUTFITS_NTABS );

   /* will allow buying from keyboard */
   window_setAccept( wid, outfits_buy );

   /* handle multipliers. */
   window_handleEvents( wid, outfit_events );
   window_setOnFocus( wid, outfit_modifiers );

   /* buttons */
   off = -20;
   if (data->outfits==NULL) {
      window_addButtonKey( wid, off, 20,
            bw, bh, "btnCloseOutfits",
            _("Take Off"), land_buttonTakeoff, SDLK_t );
   }
   else {
      window_addButtonKey( wid, off, 20,
            bw, bh, "btnCloseOutfits",
            _("Close"), window_close, SDLK_t );
   }
   off -= 20+bw;
   window_addButtonKey( wid, off, 20,
         bw, bh, "btnSellOutfit",
         _("Sell"), outfits_sell, SDLK_s );
   off -= 20+bw;
   window_addButtonKey( wid, off, 20,
         bw, bh, "btnBuyOutfit",
         _("Buy"), outfits_buy, SDLK_b );
   off -= 20+bw;
   window_addButtonKey( wid, off, 20,
         bw, bh, "btnFindOutfits",
         _("Find Outfits"), outfits_find, SDLK_f );

   /* fancy 256x256 image */
   window_addRect( wid, -40+4, -40+4, 264, 264, "rctImage", &cBlack, 1 );
   window_addImage( wid, -40, -40, 256, 256, "imgOutfit", NULL, 0 );

   /* the descriptive text */
   window_addText( wid, 20 + iw + 20, -40,
         w - (20 + iw + 20) - 264 - 40, 160, 0, "txtOutfitName", &gl_defFont, NULL, NULL );
   window_addText( wid, 20 + iw + 20, -40 - gl_defFont.h*2. - 30,
         w - (20 + iw + 20) - 264 - 40, 400, 0, "txtDescShort", &gl_defFont, NULL, NULL );

   window_addText( wid, 20 + iw + 20, 0,
         90, 160, 0, "txtSDesc", &gl_defFont, &cFontGrey, NULL );
   window_addText( wid, 20 + iw + 20 + 90, 0,
         w - (20 + iw + 20 + 90), 160, 0, "txtDDesc", &gl_defFont, NULL, NULL );
   window_addText( wid, 20 + iw + 20, 0,
         w-(iw+80), h, /* TODO: Size exactly and resize instead of moving? */
         0, "txtDescription", &gl_defFont, NULL, NULL );

   /* Create the image array. */
   outfits_genList( wid );

   /* Set default keyboard focus to the list */
   window_setFocus( wid , OUTFITS_IAR );
}

/**
 * @brief Regenerates the outfit list.
 *
 *   @param wid Window to generate the list on.
 *   @param str Unused.
 */
void outfits_regenList( unsigned int wid, const char *str )
{
   (void) str;
   int tab;
   char *focused;
   LandOutfitData *data;

   /* If local or not. */
   data = window_getData( wid );

   /* Must exist. */
   if ((data->outfits==NULL) && (land_getWid( LAND_WINDOW_OUTFITS ) == 0))
      return;

   /* Save focus. */
   focused = window_getFocus( wid );

   /* Save positions. */
   tab = window_tabWinGetActive( wid, OUTFITS_TAB );
   toolkit_saveImageArrayData( wid, OUTFITS_IAR, &iar_data[tab] );
   window_destroyWidget( wid, OUTFITS_IAR );

   outfits_genList( wid );

   /* Restore positions. */
   toolkit_loadImageArrayData( wid, OUTFITS_IAR, &iar_data[tab] );
   outfits_update( wid, NULL );

   /* Restore focus. */
   window_setFocus( wid, focused );
   free(focused);
}

/**
 * Ad-hoc filter functions.
 */

static int outfitLand_filter( const Outfit *o ) {
   Pilot *p;
   const PlayerShip_t *ps;

   switch (outfit_Mode) {
      case 0:
         return 1;

      case 1: /* Fits any ship of the player. */
         ps = player_getShipStack();
         for (int j=0; j < array_size(ps); j++) {
            p = ps[j].p;
            for (int i=0; i < array_size(p->outfits); i++) {
               if (outfit_fitsSlot( o, &p->outfits[i]->sslot->slot ))
                  return 1;
            }
         }
         return 0;

      case 2: /* Fits currently selected ship. */
         p = player.p;
         if (p==NULL)
            return 1;
         for (int i=0; i < array_size(p->outfits); i++) {
            if (outfit_fitsSlot( o, &p->outfits[i]->sslot->slot ))
               return 1;
         }
         return 0;

      case 3:
         return (o->slot.size==OUTFIT_SLOT_SIZE_LIGHT);
      case 4:
         return (o->slot.size==OUTFIT_SLOT_SIZE_MEDIUM);
      case 5:
         return (o->slot.size==OUTFIT_SLOT_SIZE_HEAVY);
   }
   return 1;
}
static int outfitLand_filterWeapon( const Outfit *o ) {
   return outfitLand_filter(o) && outfit_filterWeapon(o);
}
static int outfitLand_filterUtility( const Outfit *o ) {
   return outfitLand_filter(o) && outfit_filterUtility(o);
}
static int outfitLand_filterStructure( const Outfit *o ) {
   return outfitLand_filter(o) && outfit_filterStructure(o);
}
static int outfitLand_filterCore( const Outfit *o ) {
   return outfitLand_filter(o) && outfit_filterCore(o);
}

/**
 * @brief Generates the outfit list.
 *
 *    @param wid Window to generate the list on.
 */
static void outfits_genList( unsigned int wid )
{
   int (*tabfilters[])( const Outfit *o ) = {
      outfitLand_filter,
      outfitLand_filterWeapon,
      outfitLand_filterUtility,
      outfitLand_filterStructure,
      outfitLand_filterCore,
      outfit_filterOther,
      outfitLand_filter,
   };

   int active;
   ImageArrayCell *coutfits;
   int noutfits;
   int w, h, iw, ih;
   const char *filtertext;
   LandOutfitData *data;
   int iconsize;

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, NULL, NULL );

   /* Create tabbed window. */
   if (!widget_exists( wid, OUTFITS_TAB )) {
      int fx, fy, fw, fh, barw; /* Input filter. */
      const char *tabnames[] = {
         _("All"), _(OUTFIT_LABEL_WEAPON), _(OUTFIT_LABEL_UTILITY), _(OUTFIT_LABEL_STRUCTURE), _(OUTFIT_LABEL_CORE), _("#rOther"), _("Owned"),
      };

      window_addTabbedWindow( wid, 20, 20 + ih - 30, iw, 30,
            OUTFITS_TAB, OUTFITS_NTABS, tabnames, 1 );

      barw = window_tabWinGetBarWidth( wid, OUTFITS_TAB );
      fw = CLAMP(0, 150, iw - barw - 30);
      fh = 30;

      fx = iw - fw + 15;
      fy = ih - 25 -1 + 20;

      /* Only create the filter widgets if it will be a reasonable size. */
      if (iw >= 65) {
         /* Add popdown menu stuff. */
         window_addButton( wid, fx+fw-30, fy, 30, 30, "btnOutfitFilter", NULL, outfit_Popdown );
         window_buttonCustomRender( wid, "btnOutfitFilter", window_buttonCustomRenderGear );
         fw -= 35;

         /* Set text filter. */
         window_addInput( wid, fx, fy, fw, fh, OUTFITS_FILTER, 32, 1, &gl_defFont );
         inp_setEmptyText( wid, OUTFITS_FILTER, _("Filter…") );
         window_setInputCallback( wid, OUTFITS_FILTER, outfits_regenList );
      }
   }

   window_tabWinOnChange( wid, OUTFITS_TAB, outfits_changeTab );
   active = window_tabWinGetActive( wid, OUTFITS_TAB );

   /* Widget must not already exist. */
   if (widget_exists( wid, OUTFITS_IAR ))
      return;

   filtertext = NULL;
   if (widget_exists( wid, OUTFITS_FILTER )) {
      filtertext = window_getInput( wid, OUTFITS_FILTER );
      if (strlen(filtertext) == 0)
         filtertext = NULL;
   }

   /* Set up the outfits to buy/sell */
   data = window_getData( wid );
   array_free( iar_outfits[active] );

   if (active==6) {
      /* Show player their owned outfits. */
      const PlayerOutfit_t *po = player_getOutfits();
      iar_outfits[active] = array_create( Outfit* );
      for (int i=0; i<array_size(po); i++)
         array_push_back( &iar_outfits[active], (Outfit*) po[i].o );
      /* Also add stuff they sold. */
      for (int i=0; i<array_size(outfits_sold); i++) {
         PlayerOutfit_t *os = &outfits_sold[i];
         int found = 0;
         for (int j=0; j<array_size(po); j++) {
            if (po[j].o == os->o) {
               found = 1;
               break;
            }
         }
         if (!found)
            array_push_back( &iar_outfits[active], (Outfit*) os->o );
      }
      qsort( iar_outfits[active], array_size(iar_outfits[active]), sizeof(Outfit*), outfit_compareTech );
   }
   else {
      /* Use custom list; default to landed outfits. */
      iar_outfits[active] = (data->outfits!=NULL) ? array_copy( Outfit*, data->outfits ) : tech_getOutfit( land_spob->tech );
   }
   noutfits = outfits_filter( (const Outfit**)iar_outfits[active], array_size(iar_outfits[active]), tabfilters[active], filtertext );
   coutfits = outfits_imageArrayCells( (const Outfit**)iar_outfits[active], &noutfits, player.p, 1 );

   iconsize = 128;
   if (!conf.big_icons) {
      if (toolkit_simImageArrayVisibleElements(iw,ih-34,iconsize,iconsize) < noutfits)
         iconsize = 96;
      if (toolkit_simImageArrayVisibleElements(iw,ih-34,iconsize,iconsize) < noutfits)
         iconsize = 64;
   }
   window_addImageArray( wid, 20, 20,
         iw, ih - 34, OUTFITS_IAR, iconsize, iconsize,
         coutfits, noutfits, outfits_update, outfits_rmouse, NULL );

   /* write the outfits stuff */
   outfits_update( wid, NULL );
}

/**
 * @brief Updates the outfits in the outfit window.
 *    @param wid Window to update the outfits in.
 *    @param str Unused.
 */
void outfits_update( unsigned int wid, const char *str )
{
   (void) str;
   int i, active;
   Outfit* outfit;
   char buf[STRMAX], lbl[STRMAX], buf_credits[ECON_CRED_STRLEN], buf_mass[ECON_MASS_STRLEN];
   const char *buf_price;
   credits_t price;
   size_t l = 0, k = 0;
   int iw, ih, w, h, blackmarket, canbuy, cansell, sw, th;
   double mass;
   const char *summary;

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, NULL, NULL );

   /* See if black market. */
   const LandOutfitData *data = window_getData( wid );
   blackmarket = data->blackmarket;

   /* Set up keys. */
   k += scnprintf( &lbl[k], sizeof(lbl)-k, "%s", _("Owned:") );
   k += scnprintf( &lbl[k], sizeof(lbl)-k, "\n%s", _("Mass:") );
   k += scnprintf( &lbl[k], sizeof(lbl)-k, "\n%s", _("Price:") );
   k += scnprintf( &lbl[k], sizeof(lbl)-k, "\n%s", _("Money:") );

   /* Get and set parameters. */
   active = window_tabWinGetActive( wid, OUTFITS_TAB );
   i = toolkit_getImageArrayPos( wid, OUTFITS_IAR );
   if (i < 0 || array_size(iar_outfits[active]) == 0) { /* No outfits */
      window_modifyImage( wid, "imgOutfit", NULL, 256, 256 );
      window_disableButton( wid, "btnBuyOutfit" );
      window_disableButton( wid, "btnSellOutfit" );
      l += scnprintf( &buf[l], sizeof(buf)-l, "%s", _("N/A") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("N/A") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("N/A") );
      /*l +=*/ scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("N/A") );
      window_modifyText( wid, "txtSDesc", buf );
      window_modifyText( wid, "txtDDesc", buf );
      window_modifyText( wid, "txtOutfitName", _("None") );
      window_modifyText( wid, "txtDescShort", NULL );
      window_modifyText( wid, "txtDescription", NULL );
      /* Reposition. */
      th = 260;
      window_moveWidget( wid, "txtSDesc", 20+iw+20, -40-th-30-32 );
      window_moveWidget( wid, "txtDDesc", 20+iw+20+90, -40-th-30-32 );
      window_moveWidget( wid, "txtDescription", 20+iw+20, -240-32);
      return;
   }

   i = MIN( i, array_size(iar_outfits[active])-1 );
   outfit = iar_outfits[active][i];

   /* new image */
   window_modifyImage( wid, "imgOutfit", outfit->gfx_store, 256, 256 );

   /* new text */
   if (outfit->slot.type == OUTFIT_SLOT_INTRINSIC) {
      scnprintf( buf, sizeof(buf), "%s\n\n#o%s#0",
            _(outfit->desc_raw),
            _("This is an intrinsic outfit that will be directly equipped on the current ship and can not be moved."));
      window_modifyText( wid, "txtDescription", buf );
   }
   else
      window_modifyText( wid, "txtDescription", _(outfit->desc_raw) );
   buf_price = outfit_getPrice( outfit, &price, &canbuy, &cansell );
   credits2str( buf_credits, player.p->credits, 2 );

   /* gray out sell button */
   if ((outfit_canSell(outfit) > 0) && cansell)
      window_enableButton( wid, "btnSellOutfit" );
   else
      window_disableButtonSoft( wid, "btnSellOutfit" );

   if ((outfit_canBuy( outfit, wid ) > 0) && canbuy) {
      window_enableButton( wid, "btnBuyOutfit" );
   }
   else
      window_disableButtonSoft( wid, "btnBuyOutfit" );

   mass = outfit->mass;
   if (outfit_isLauncher(outfit))
      mass += outfit_amount(outfit) * outfit->u.lau.ammo_mass;
   else if (outfit_isFighterBay(outfit))
      mass += outfit_amount(outfit) * outfit->u.bay.ship_mass;
   tonnes2str( buf_mass, (int)round( mass ) );

   outfit_getNameWithClass( outfit, buf, sizeof(buf) );
   window_modifyText( wid, "txtOutfitName", buf );
   window_dimWidget( wid, "txtOutfitName", &sw, NULL );
   th = gl_printHeightRaw( &gl_defFont, sw, buf )+gl_defFont.h/2;
   l = 0;
   l += scnprintf( &buf[l], sizeof(buf)-l, "%d", player_outfitOwned(outfit) );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", buf_mass );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", buf_price );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", buf_credits );
   if (outfit->license) {
      int meets_reqs = player_hasLicense( outfit->license );
      k += scnprintf( &lbl[k], sizeof(lbl)-k, "\n%s", _("License:") );
      if (blackmarket)
         l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s#0", _("Not Necessary (Blackmarket)") );
      else
         l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s%s#0", meets_reqs ? "" : "#r", _(outfit->license) );
   }
   if (outfit->cond) {
      int meets_reqs = 0;
      if (land_spob != NULL)
         meets_reqs = cond_check(outfit->cond);
      /*k +=*/ scnprintf( &lbl[k], sizeof(lbl)-k, "\n%s", _("Requires:") );
      if (blackmarket)
         /*l +=*/ scnprintf( &buf[l], sizeof(buf)-l, "\n%s#0", _("Not Necessary (Blackmarket)") );
      else
         /*l +=*/ scnprintf( &buf[l], sizeof(buf)-l, "\n%s%s#0", meets_reqs ? "" : "#r", _(outfit->condstr) );
   }
   window_modifyText( wid, "txtSDesc", lbl );
   window_modifyText( wid, "txtDDesc", buf );
   summary = pilot_outfitSummary( player.p, outfit, 0 );
   window_modifyText( wid, "txtDescShort", summary );
   window_moveWidget( wid, "txtDescShort", 20+iw+20, -40-th );
   window_dimWidget( wid, "txtDescShort", &sw, NULL );
   th += gl_printHeightRaw( &gl_defFont, sw, summary );
   th = MAX( th+gl_defFont.h/2, 240 );
   window_moveWidget( wid, "txtSDesc", 20+iw+20, -40-th );
   window_moveWidget( wid, "txtDDesc", 20+iw+20+90, -40-th );
   window_dimWidget( wid, "txtDDesc", &sw, NULL );
   th += gl_printHeightRaw( &gl_defFont, sw, buf );
   th = MAX( th+gl_defFont.h/2, 256+10 );
   window_moveWidget( wid, "txtDescription", 20+iw+20, -40-th );
}

/**
 * @brief Updates the outfitter and equipment outfit image arrays.
 */
void outfits_updateEquipmentOutfits( void )
{
   if (landed && land_doneLoading()) {
      if (spob_hasService(land_spob, SPOB_SERVICE_OUTFITS)) {
         int ow = land_getWid( LAND_WINDOW_OUTFITS );
         outfits_regenList( ow, NULL );
      }
      else if (!spob_hasService(land_spob, SPOB_SERVICE_SHIPYARD))
         return;

      int ew = land_getWid( LAND_WINDOW_EQUIPMENT );
      equipment_addAmmo();
      equipment_regenLists( ew, 1, 0 );
   }
}

/**
 * @brief Ensures the tab's selected item is reflected in the ship slot list
 *
 *    @param wid Unused.
 *    @param wgt Unused.
 *    @param old Tab changed from.
 *    @param tab Tab changed to.
 */
static void outfits_changeTab( unsigned int wid, const char *wgt, int old, int tab )
{
   (void) wgt;
   iar_data_t old_data;

   toolkit_saveImageArrayData( wid, OUTFITS_IAR, &iar_data[old] );

   /* Store the currently-saved positions for the new tab. */
   old_data = iar_data[tab];

   /* Resetting the input will cause the outfit list to be regenerated. */
   if (widget_exists(wid, OUTFITS_FILTER))
      window_setInput(wid, OUTFITS_FILTER, NULL);
   else
      outfits_regenList( wid, NULL );

   /* Set positions for the new tab. This is necessary because the stored
    * position for the new tab may have exceeded the size of the old tab,
    * resulting in it being clipped. */
   toolkit_loadImageArrayData( wid, OUTFITS_IAR, &old_data );

   /* Focus the outfit image array. */
   window_setFocus( wid, OUTFITS_IAR );
}

/**
 * @brief Applies a filter function and string to a list of outfits.
 *
 *    @param outfits Array of outfits to filter.
 *    @param n Number of outfits in the array.
 *    @param filter Filter function to run on each outfit.
 *    @param name Name fragment that each outfit name must contain.
 *    @return Number of outfits.
 */
int outfits_filter( const Outfit **outfits, int n,
      int(*filter)( const Outfit* ), const char *name )
{
   int j = 0;
   for (int i=0; i<n; i++) {
      const Outfit *o = outfits[i];

      /* First run filter. */
      if ((filter != NULL) && !filter(outfits[i]))
         continue;

      /* Try to match name somewhere. */
      if (name!=NULL) {
         if ((SDL_strcasestr(_(o->name), name)==NULL) &&
               (SDL_strcasestr(_(outfit_getType(o)), name)==NULL))
            continue;
      }

      /* Shift matches downward. */
      outfits[j++] = o;
   }

   return j;
}

/**
 * @brief Starts the map find with outfit search selected.
 *    @param wid Window buying outfit from.
 *    @param str Unused.
 */
static void outfits_find( unsigned int wid, const char *str )
{
   (void) str;
   map_inputFindType(wid, "outfit");
}

/**
 * @brief Returns the price of an outfit (subject to quantity modifier)
 */
static const char *outfit_getPrice( const Outfit *outfit, credits_t *price, int *canbuy, int *cansell )
{
   static char pricestr[STRMAX_SHORT];
   unsigned int q = outfits_getMod();
   if (outfit->lua_price == LUA_NOREF) {
      price2str( pricestr, outfit->price * q, player.p->credits, 2 );
      *price = outfit->price * q;
      *canbuy = 1;
      *cansell = 1;
      return pricestr;
   }
   const char *str;

   lua_rawgeti(naevL, LUA_REGISTRYINDEX, outfit->lua_price);
   lua_pushinteger(naevL, q);
   if (nlua_pcall( outfit->lua_env, 1, 3 )) {   /* */
      WARN(_("Outfit '%s' failed to run '%s':\n%s"),outfit->name,"price",lua_tostring(naevL,-1));
      *price = 0;
      *canbuy = 0;
      *cansell = 0;
      lua_pop(naevL, 1);
      return pricestr;
   }

   str = luaL_checkstring( naevL, -3 );
   strncpy( pricestr, str, sizeof(pricestr)-1 );
   *price = 0;
   *canbuy = lua_toboolean( naevL, -2 );
   *cansell = lua_toboolean( naevL, -1 );
   lua_pop(naevL, 3);

   return pricestr;
}

/**
 * @brief Computes the alt text for an outfit.
 */
int outfit_altText( char *buf, int n, const Outfit *o, const Pilot *plt )
{
   int p;
   double mass;

   /* Compute total mass of launcher and ammo if necessary. */
   mass = o->mass;
   if (outfit_isLauncher(o))
      mass += outfit_amount(o) * o->u.lau.ammo_mass;
   else if (outfit_isFighterBay(o))
      mass += outfit_amount(o) * o->u.bay.ship_mass;

   p  = outfit_getNameWithClass( o, buf, n );
   p += scnprintf( &buf[p], n-p, "\n" );
   if (outfit_isProp(o, OUTFIT_PROP_UNIQUE))
      p += scnprintf( &buf[p], n-p, "#o%s#0\n", _("Unique") );
   if (o->limit != NULL)
      p += scnprintf( &buf[p], n-p, "#r%s#0\n", _("Only 1 of type per ship") );
   if (o->slot.spid != 0)
      p += scnprintf( &buf[p], n-p, "#o%s#0\n",
            _(sp_display( o->slot.spid) ) );
   p += scnprintf( &buf[p], n-p, "%s", pilot_outfitSummary( plt, o, 0 ) );
   if ((o->mass > 0.) && (p < n)) {
      char buf_mass[ECON_MASS_STRLEN];
      tonnes2str( buf_mass, (int)round( mass ) );
      scnprintf( &buf[p], n-p, "\n%s", buf_mass );
   }
   return 0;
}

/**
 * @brief Generates image array cells corresponding to outfits.
 */
ImageArrayCell *outfits_imageArrayCells( const Outfit **outfits, int *noutfits, const Pilot *p, int store )
{
   ImageArrayCell *coutfits = calloc( MAX(1,*noutfits), sizeof(ImageArrayCell) );

   if (*noutfits == 0) {
      *noutfits = 1;
      coutfits[0].image = NULL;
      coutfits[0].caption = strdup( _("None") );
   }
   else {
      /* Set alt text. */
      for (int i=0; i<*noutfits; i++) {
         const glColour *c;
         glTexture *t;
         const Outfit *o = outfits[i];

         coutfits[i].image = gl_dupTexture( o->gfx_store );
         coutfits[i].caption = strdup( _(o->name) );
         if (!store && outfit_isProp(o, OUTFIT_PROP_UNIQUE))
            coutfits[i].quantity = -1; /* Don't display. */
         else
            coutfits[i].quantity = player_outfitOwned(o);
         coutfits[i].sloticon = sp_icon( o->slot.spid );

         /* Background colour. */
         c = outfit_slotSizeColour( &o->slot );
         if (c == NULL)
            c = &cBlack;
         col_blend( &coutfits[i].bg, c, &cGrey70, 1 );

         /* Short description. */
         coutfits[i].alt = strdup( pilot_outfitSummary( p, o, 1 ) );

         /* Slot type. */
         if ( (strcmp(outfit_slotName(o), "N/A") != 0)
               && (strcmp(outfit_slotName(o), "NULL") != 0) ) {
            size_t sz = 0;
            const char *typename = _(outfit_slotName(o));
            u8_inc( typename, &sz );
            coutfits[i].slottype = malloc( sz+1 );
            memcpy( coutfits[i].slottype, typename, sz );
            coutfits[i].slottype[sz] = '\0';
         }

         /* Layers. */
         coutfits[i].layers = gl_copyTexArray( o->gfx_overlays );
         if (o->rarity > 0) {
            t = rarity_texture( o->rarity );
            coutfits[i].layers = gl_addTexArray( coutfits[i].layers, t );
         }
      }
   }
   return coutfits;
}

/**
 * Functions for the popdown menu (filter outfits by size)
 */

static void outfit_PopdownSelect( unsigned int wid, const char *str )
{
   int m = toolkit_getListPos( wid, str );
   if (m == outfit_Mode)
      return;

   outfit_Mode = m;
   outfits_regenList( wid, NULL );
}

static void outfit_PopdownActivate( unsigned int wid, const char *str )
{
   outfit_PopdownSelect( wid, str );
   window_destroyWidget( wid, str );
}

static void outfit_Popdown( unsigned int wid, const char* str )
{
   const char *name = "lstOutfitPopdown";
   const char *modes[] = {
      N_("Show all outfits"),
      N_("Show only outfits equipable on any of your ships"),
      N_("Show only outfits equipable on current ship"),
      N_("Show only light outfits"),
      N_("Show only medium outfits"),
      N_("Show only heavy outfits"),
   };
   char **modelist;
   const size_t n = sizeof(modes) / sizeof(const char*);
   int x, y, w, h;

   if (widget_exists( wid, name )) {
      window_destroyWidget( wid, name );
      return;
   }

   modelist = malloc(sizeof(modes));
   for (size_t i=0; i<n; i++)
      modelist[i] = strdup( _(modes[i]) );

   window_dimWidget( wid, str, &w, &h );
   window_posWidget( wid, str, &x, &y );
   window_addList( wid, x+w, y-120+h, 350, 120, name, modelist, n, outfit_Mode, outfit_PopdownSelect, outfit_PopdownActivate );
   window_setFocus( wid, name );
}

static int outfit_isSold( const Outfit *outfit, int wid )
{
   LandOutfitData *data = NULL;
   if (wid>=0)
      data = window_getData( wid );
   if ((data!=NULL) && (data->outfits!=NULL)) {
      for (int i=0; i<array_size(data->outfits); i++) {
         if (data->outfits[i]==outfit)
            return 1;
      }
      return 0;
   }
   else if ((land_spob!=NULL) && tech_checkOutfit( land_spob->tech, outfit ))
      return 1;
   return 0;
}

/**
 * @brief Checks to see if the player can buy the outfit.
 *    @param outfit Outfit to buy.
 *    @param wid Window ID of the outfitter (or -1 to ignore).
 */
int outfit_canBuy( const Outfit *outfit, int wid )
{
   int failure, canbuy, cansell;
   credits_t price;
   LandOutfitData *data = NULL;
   if (wid>=0)
      data = window_getData( wid );
   int blackmarket = (data!=NULL) ? data->blackmarket : 0;
   const Outfit *omap = outfit_get( LOCAL_MAP_NAME );

   land_errClear();
   failure = 0;
   outfit_getPrice( outfit, &price, &canbuy, &cansell );

   /* Special exception for local map. */
   if (outfit!=omap) {
      int sold = 0;
      /* See if the player previously sold it. */
      for (int i=0; i<array_size(outfits_sold); i++) {
         if (outfits_sold[i].o == outfit ) {
            sold = outfits_sold[i].q;
            break;
         }
      }

      /* Not sold at planet. */
      if (!sold && !outfit_isSold( outfit, wid )) {
         land_errDialogueBuild( _("Outfit not sold here!") );
         return 0;
      }
   }

   /* Unique. */
   if (outfit_isProp(outfit, OUTFIT_PROP_UNIQUE) && (player_outfitOwnedTotal(outfit)>0)) {
      land_errDialogueBuild( _("You can only own one of this outfit.") );
      return 0;
   }

   /* Intrinsic. */
   if (outfit->slot.type==OUTFIT_SLOT_INTRINSIC) {
      if (pilot_hasOutfitLimit( player.p, outfit->limit )) {
         land_errDialogueBuild( _("You can only equip one of this outfit type.") );
         return 0;
      }
   }

   /* Map already mapped */
   if ((outfit_isMap(outfit) && map_isUseless(outfit)) ||
         (outfit_isLocalMap(outfit) && localmap_isUseless(outfit))) {
      land_errDialogueBuild( _("You already know of everything this map contains.") );
      return 0;
   }
   /* GUI already owned */
   if (outfit_isGUI(outfit) && player_guiCheck(outfit->u.gui.gui)) {
      land_errDialogueBuild( _("You already own this GUI.") );
      return 0;
   }
   /* Already has license. */
   if (outfit_isLicense(outfit) && player_hasLicense(outfit->u.lic.provides)) {
      land_errDialogueBuild( _("You already have this license.") );
      return 0;
   }
   /* Not enough $$ */
   if (!player_hasCredits(price)) {
      char buf[ECON_CRED_STRLEN];
      credits2str( buf, price - player.p->credits, 2 );
      land_errDialogueBuild( _("You need %s more."), buf);
      failure = 1;
   }
   /* Needs license. */
   if (!blackmarket && !player_hasLicense(outfit->license)) {
      land_errDialogueBuild( _("You need the '%s' license to buy this outfit."),
               _(outfit->license) );
      failure = 1;
   }
   /* Needs requirements. */
   if (!blackmarket && (outfit->cond!=NULL) && !cond_check(outfit->cond)) {
      land_errDialogueBuild( "%s", _(outfit->condstr) );
      failure = 1;
   }
   /* Custom condition failed. */
   if (!canbuy) {
      land_errDialogueBuild( _("You lack the resources to buy this outfit.") );
      failure = 1;
   }

   return !failure;
}

/**
 * @brief Player right-clicks on an outfit.
 *    @param wid Window player is buying ship from.
 *    @param widget_name Name of the window. (unused)
 */
static void outfits_rmouse( unsigned int wid, const char* widget_name )
{
    outfits_buy( wid, widget_name );
}

/**
 * @brief Attempts to buy the outfit that is selected.
 *    @param wid Window buying outfit from.
 *    @param str Unused.
 */
static void outfits_buy( unsigned int wid, const char *str )
{
   (void) str;
   int i, active;
   Outfit* outfit;
   int q;
   PlayerOutfit_t *sold = NULL;
   HookParam hparam[3];

   active = window_tabWinGetActive( wid, OUTFITS_TAB );
   i = toolkit_getImageArrayPos( wid, OUTFITS_IAR );
   if (i < 0 || array_size(iar_outfits[active]) == 0)
      return;

   outfit = iar_outfits[active][i];
   q = outfits_getMod();
   /* Can only get one unique item. */
   if (outfit_isProp(outfit, OUTFIT_PROP_UNIQUE) ||
         (outfit->slot.type==OUTFIT_SLOT_INTRINSIC) ||
         outfit_isMap(outfit) || outfit_isLocalMap(outfit) ||
         outfit_isGUI(outfit) || outfit_isLicense(outfit))
      q = MIN(q,1);

   /* Can buy the outfit? */
   if (!outfit_canBuy( outfit, wid )) {
      land_errDisplay();
      return;
   }

   /* See if the player previously sold it, and limit it to that amount. */
   for (int j=0; j<array_size(outfits_sold); j++) {
      if (outfits_sold[j].o == outfit) {
         sold = &outfits_sold[j];
         q = MIN( q, outfits_sold[j].q );
         break;
      }
   }

   /* Give dialogue when trying to buy intrinsic. */
   if (outfit->slot.type==OUTFIT_SLOT_INTRINSIC)
      if (!dialogue_YesNo( _("Buy Intrinsic Outfit?"), _("Are you sure you wish to buy '%s'? It will be automatically equipped on your current ship '%s'."), _(outfit->name), player.p->name ))
         return;

   /* Try Lua. */
   if (outfit->lua_buy != LUA_NOREF) {
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, outfit->lua_buy);
      lua_pushinteger(naevL, q);
      if (nlua_pcall( outfit->lua_env, 1, 2 )) {   /* */
         WARN(_("Outfit '%s' failed to run '%s':\n%s"),outfit->name,"price",lua_tostring(naevL,-1));
         lua_pop(naevL, 1);
      }

      int bought = lua_toboolean(naevL,-2);

      if (!bought) {
         dialogue_alert( "%s", lua_tostring(naevL,-1) );
         lua_pop(naevL, 2);
         return;
      }
      q = luaL_checkinteger(naevL,-1);
      player_addOutfit( outfit, q );

      lua_pop(naevL, 2);
   }
   else
      player_modCredits( -outfit->price * player_addOutfit( outfit, q ) );

   /* Actually buy the outfit. */
   outfits_updateEquipmentOutfits();
   hparam[0].type    = HOOK_PARAM_OUTFIT;
   hparam[0].u.outfit= outfit;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = q;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "outfit_buy", hparam );
   land_needsTakeoff( 1 );

   /* Mark it that it was sold. */
   if (sold != NULL)
      sold->q -= q;

   /* Regenerate list. */
   outfits_regenList( wid, NULL );
}
/**
 * @brief Checks to see if the player can sell the selected outfit.
 *    @param outfit Outfit to try to sell.
 */
int outfit_canSell( const Outfit *outfit )
{
   int failure = 0;
   int canbuy, cansell;
   credits_t price;

   land_errClear();
   outfit_getPrice( outfit, &price, &canbuy, &cansell );

   /* Unique item. */
   if (outfit_isProp(outfit, OUTFIT_PROP_UNIQUE)) {
      land_errDialogueBuild(_("You can't sell a unique outfit."));
      failure = 1;
   }
   /* Map check. */
   if (outfit_isMap(outfit) || outfit_isLocalMap(outfit)) {
      land_errDialogueBuild(_("You can't sell a map."));
      failure = 1;
   }
   /* GUI check. */
   if (outfit_isGUI(outfit)) {
      land_errDialogueBuild(_("You can't sell a GUI."));
      failure = 1;
   }
   /* License check. */
   if (outfit_isLicense(outfit)) {
      land_errDialogueBuild(_("You can't sell a license."));
      failure = 1;
   }
   /* has no outfits to sell */
   if (!pilot_hasIntrinsic(player.p,outfit) && (player_outfitOwned(outfit) <= 0)) {
      land_errDialogueBuild( _("You can't sell something you don't have!") );
      failure = 1;
   }
   /* Custom condition failed. */
   if (!cansell) {
      land_errDialogueBuild(_("You are unable to sell this outfit!"));
      failure = 1;
   }

   return !failure;
}
/**
 * @brief Attempts to sell the selected outfit the player has.
 *    @param wid Window selling outfits from.
 *    @param str Unused.
 */
static void outfits_sell( unsigned int wid, const char *str )
{
   (void)str;
   int i, active;
   Outfit* outfit;
   int q;
   HookParam hparam[3];

   active = window_tabWinGetActive( wid, OUTFITS_TAB );
   i = toolkit_getImageArrayPos( wid, OUTFITS_IAR );
   if (i < 0 || array_size(iar_outfits[active]) == 0)
      return;

   outfit = iar_outfits[active][i];

   q = outfits_getMod();

   /* Check various failure conditions. */
   if (!outfit_canSell( outfit )) {
      land_errDisplay();
      return;
   }

   /* Try Lua. */
   if (outfit->lua_sell != LUA_NOREF) {
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, outfit->lua_sell);
      lua_pushinteger(naevL, q);
      if (nlua_pcall( outfit->lua_env, 1, 2 )) {   /* */
         WARN(_("Outfit '%s' failed to run '%s':\n%s"),outfit->name,"price",lua_tostring(naevL,-1));
         lua_pop(naevL, 1);
      }

      int bought = lua_toboolean(naevL,-2);

      if (!bought) {
         dialogue_alert( "%s", lua_tostring(naevL,-1) );
         lua_pop(naevL, 2);
         return;
      }
      q = luaL_checkinteger(naevL,-1);

      lua_pop(naevL, 2);
   }
   else {
      q = player_rmOutfit( outfit, q );
      player_modCredits( outfit->price * q );
   }

   outfits_updateEquipmentOutfits();
   hparam[0].type    = HOOK_PARAM_OUTFIT;
   hparam[0].u.outfit= outfit;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = q;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "outfit_sell", hparam );
   land_needsTakeoff( 1 );

   /* Update sold outfits list. */
   if (!outfit_isSold( outfit, wid )) {
      int found = 0;
      if (outfits_sold==NULL)
         outfits_sold = array_create( PlayerOutfit_t );
      for (int j=0; j<array_size(outfits_sold); j++) {
         PlayerOutfit_t *poi = &outfits_sold[j];
         if (poi->o == outfit ) {
            poi->q += q;
            found = 1;
            break;
         }
      }
      if (!found) {
         PlayerOutfit_t po = {
            .o = outfit,
            .q = q,
         };
         array_push_back( &outfits_sold, po );
      }
   }

   /* Regenerate list. */
   outfits_regenList( wid, NULL );
}
/**
 * @brief Gets the current modifier status.
 *    @return The amount modifier when buying or selling outfits.
 */
static int outfits_getMod (void)
{
   SDL_Keymod mods = SDL_GetModState();
   int q = 1;
   if (mods & (KMOD_LCTRL | KMOD_RCTRL))
      q *= 5;
   if (mods & (KMOD_LSHIFT | KMOD_RSHIFT))
      q *= 10;
   return q;
}

/**
 * @brief Cleans up outfit globals.
 */
void outfits_cleanup(void)
{
   /* Free stored positions. */
   for (int i=0; i<OUTFITS_NTABS; i++)
      array_free( iar_outfits[i] );
   memset( iar_outfits, 0, sizeof(Outfit**) * OUTFITS_NTABS );
}
