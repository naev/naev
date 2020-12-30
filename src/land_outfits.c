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

#include "dialogue.h"
#include "equipment.h"
#include "hook.h"
#include "land_takeoff.h"
#include "log.h"
#include "map.h"
#include "map_find.h"
#include "nstring.h"
#include "outfit.h"
#include "player.h"
#include "player_gui.h"
#include "slots.h"
#include "space.h"
#include "toolkit.h"


#define  OUTFITS_IAR    "iarOutfits"
#define  OUTFITS_TAB    "tabOutfits"
#define  OUTFITS_FILTER "inpFilterOutfits"
#define  OUTFITS_NTABS  6


typedef struct LandOutfitData_ {
   Outfit *outfits;
   int noutfits;
} LandOutfitData;


static iar_data_t *iar_data = NULL; /**< Stored image array positions. */
static Outfit ***iar_outfits = NULL; /**< Outfits associated with the image array cells. */

/* Modifier for buying and selling quantity. */
static int outfits_mod = 1;


/*
 * Helper functions.
 */
static void outfits_getSize( unsigned int wid, int *w, int *h,
      int *iw, int *ih, int *bw, int *bh );
static void outfits_buy( unsigned int wid, char* str );
static void outfits_sell( unsigned int wid, char* str );
static int outfits_getMod (void);
static void outfits_renderMod( double bx, double by, double w, double h, void *data );
static void outfits_rmouse( unsigned int wid, char* widget_name );
static void outfits_find( unsigned int wid, char* str );
static credits_t outfit_getPrice( Outfit *outfit );
static void outfits_genList( unsigned int wid );
static void outfits_changeTab( unsigned int wid, char *wgt, int old, int tab );
static void outfits_onClose( unsigned int wid, char *str );


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
      *iw = 704 + (*w - LAND_WIDTH);
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


/**
 * @brief For when the widget closes.
 */
static void outfits_onClose( unsigned int wid, char *str )
{
   (void) str;
   LandOutfitData *data = window_getData( wid );
   if (data==NULL)
      return;
   free( data->outfits );
   free( data );
}



/**
 * @brief Opens the outfit exchange center window.
 *
 *    @param wid Window ID to open at.
 *    @param outfits Outfit list to sell. Set to NULL if this is the landed player store.
 *    @param noutfits Length of \p outfits.
 */
void outfits_open( unsigned int wid, Outfit **outfits, int noutfits )
{
   int w, h, iw, ih, bw, bh, off;
   LandOutfitData *data = NULL;

   /* Set up window data. */
   if (outfits!=NULL) {
      data           = malloc( sizeof( LandOutfitData ) );
      data->noutfits = noutfits;
      data->outfits  = malloc( data->noutfits * sizeof( Outfit ) );
      memcpy( data->outfits, outfits, data->noutfits * sizeof( Outfit ) );
      window_setData( wid, data );
      window_onClose( wid, outfits_onClose );
   }

   /* Mark as generated. */
   if (outfits==NULL)
      land_tabGenerate(LAND_WINDOW_OUTFITS);

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, &bw, &bh );

   /* Initialize stored positions. */
   if (outfits==NULL) {
      if (iar_data == NULL)
         iar_data = calloc( OUTFITS_NTABS, sizeof(iar_data_t) );
      else
         memset( iar_data, 0, sizeof(iar_data_t) * OUTFITS_NTABS );
   }
   if (iar_outfits == NULL)
      iar_outfits = calloc( OUTFITS_NTABS, sizeof(Outfit**) );
   else {
      for (int i=0; i<OUTFITS_NTABS; i++)
         free( iar_outfits[i] );
      memset( iar_outfits, 0, sizeof(Outfit**) * OUTFITS_NTABS );
   }

   /* will allow buying from keyboard */
   window_setAccept( wid, outfits_buy );

   /* buttons */
   if (data==NULL) {
      window_addButtonKey( wid, off = -20, 20,
            bw, bh, "btnCloseOutfits",
            _("Take Off"), land_buttonTakeoff, SDLK_t );
   }
   else {
      window_addButtonKey( wid, off = -20, 20,
            bw, bh, "btnCloseOutfits",
            _("Close"), window_close, SDLK_t );
   }
   window_addButtonKey( wid, off -= 20+bw, 20,
         bw, bh, "btnSellOutfit",
         _("Sell"), outfits_sell, SDLK_s );
   window_addButtonKey( wid, off -= 20+bw, 20,
         bw, bh, "btnBuyOutfit",
         _("Buy"), outfits_buy, SDLK_b );
   window_addButtonKey( wid, off -= 20+bw, 20,
         bw, bh, "btnFindOutfits",
         _("Find Outfits"), outfits_find, SDLK_f );
   (void)off;

   /* fancy 192x192 image */
   window_addRect( wid, -17, -46, 200, 199, "rctImage", &cBlack, 0 );
   window_addImage( wid, -20, -50, 192, 192, "imgOutfit", NULL, 1 );

   /* cust draws the modifier */
   window_addCust( wid, -40-bw, 60+2*bh,
         bw, bh, "cstMod", 0, outfits_renderMod, NULL, NULL );

   /* the descriptive text */
   window_addText( wid, 20 + iw + 20, -40,
         w - (20 + iw + 20) - 200 - 20, 160, 0, "txtOutfitName", &gl_defFont, NULL, NULL );
   window_addText( wid, 20 + iw + 20, -40 - gl_defFont.h - 30,
         w - (20 + iw + 20) - 200 - 20, 320, 0, "txtDescShort", &gl_smallFont, NULL, NULL );

   window_addText( wid, 20 + iw + 20, 0,
         90, 160, 0, "txtSDesc", &gl_smallFont, NULL,
         _("#nOwned:#0\n"
         "\n"
         "#nSlot:#0\n"
         "#nSize:#0\n"
         "#nMass:#0\n"
         "\n"
         "#nPrice:#0\n"
         "#nMoney:#0\n"
         "#nLicense:#0\n") );
   window_addText( wid, 20 + iw + 20 + 90, 0,
         w - (20 + iw + 20 + 90), 160, 0, "txtDDesc", &gl_smallFont, NULL, NULL );
   window_addText( wid, 20 + iw + 20, 0,
         w-(iw+80), h, /* TODO: Size exactly and resize instead of moving? */
	 0, "txtDescription", &gl_smallFont, NULL, NULL );

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
void outfits_regenList( unsigned int wid, char *str )
{
   (void) str;
   int tab;
   char *focused;
   LandOutfitData *data;

   /* If local or not. */
   data = window_getData( wid );

   /* Must exist. */
   if ((data==NULL) && (land_getWid( LAND_WINDOW_OUTFITS ) == 0))
      return;

   /* Save focus. */
   focused = strdup(window_getFocus(wid));

   /* Save positions. */
   tab = window_tabWinGetActive( wid, OUTFITS_TAB );
   toolkit_saveImageArrayData( wid, OUTFITS_IAR, &iar_data[tab] );
   window_destroyWidget( wid, OUTFITS_IAR );

   outfits_genList( wid );

   /* Restore positions. */
   toolkit_setImageArrayPos(    wid, OUTFITS_IAR, iar_data[tab].pos );
   toolkit_setImageArrayOffset( wid, OUTFITS_IAR, iar_data[tab].offset );
   outfits_update( wid, NULL );

   /* Restore focus. */
   window_setFocus( wid, focused );
   free(focused);
}

/**
 * @brief Generates the outfit list.
 *
 *    @param wid Window to generate the list on.
 */
static void outfits_genList( unsigned int wid )
{
   int (*tabfilters[])( const Outfit *o ) = {
      NULL,
      outfit_filterWeapon,
      outfit_filterUtility,
      outfit_filterStructure,
      outfit_filterCore,
      outfit_filterOther
   };
   const char *tabnames[] = {
      _("All"), _(OUTFIT_LABEL_WEAPON), _(OUTFIT_LABEL_UTILITY), _(OUTFIT_LABEL_STRUCTURE), _(OUTFIT_LABEL_CORE), _("Other")
   };

   int active;
   int fx, fy, fw, fh, barw; /* Input filter. */
   ImageArrayCell *coutfits;
   int noutfits;
   int w, h, iw, ih;
   char *filtertext;
   LandOutfitData *data;
   int iconsize;

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, NULL, NULL );

   /* Create tabbed window. */
   if (!widget_exists( wid, OUTFITS_TAB )) {
      window_addTabbedWindow( wid, 20, 20 + ih - 30, iw, 30,
         OUTFITS_TAB, OUTFITS_NTABS, tabnames, 1 );

      barw = window_tabWinGetBarWidth( wid, OUTFITS_TAB );
      fw = CLAMP(0, 150, iw - barw - 30);
      fh = 20;

      fx = iw - fw;
      fy = ih - (30 - fh) / 2; /* Centered relative to 30 px tab bar */

      /* Only create the filter widget if it will be a reasonable size. */
      if (iw >= 30) {
         window_addInput( wid, fx + 15, fy +1, fw, fh, OUTFITS_FILTER, 32, 1, &gl_smallFont );
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
   free( iar_outfits[active] );
   if (data == NULL) {
      /* Use landed outfits. */
      iar_outfits[active] = tech_getOutfit( land_planet->tech, &noutfits );
   }
   else {
      /* Use custom list. */
      noutfits = data->noutfits;
      iar_outfits[active] = calloc( noutfits, sizeof(Outfit*) );
      memcpy( iar_outfits[active], data->outfits, sizeof(Outfit*)*noutfits );
   }
   noutfits = outfits_filter( iar_outfits[active], noutfits,
         tabfilters[active], filtertext );
   coutfits = outfits_imageArrayCells( iar_outfits[active], &noutfits );

   if (!conf.big_icons && (((iw*ih)/(128*128)) < noutfits))
      iconsize = 96;
   else
      iconsize = 128;
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
void outfits_update( unsigned int wid, char* str )
{
   (void)str;
   int i, active;
   Outfit* outfit;
   char buf[PATH_MAX], buf_price[ECON_CRED_STRLEN], buf_credits[ECON_CRED_STRLEN], buf_license[PATH_MAX];
   double th;
   int iw, ih;
   int w, h;
   double mass;

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, NULL, NULL );

   /* Get and set parameters. */
   active = window_tabWinGetActive( wid, OUTFITS_TAB );
   i = toolkit_getImageArrayPos( wid, OUTFITS_IAR );
   if (i < 0 || iar_outfits[active] == NULL) { /* No outfits */
      window_modifyImage( wid, "imgOutfit", NULL, 192, 192 );
      window_disableButton( wid, "btnBuyOutfit" );
      window_disableButton( wid, "btnSellOutfit" );
      nsnprintf( buf, PATH_MAX,
            _("N/A\n"
            "\n"
            "N/A\n"
            "N/A\n"
            "N/A\n"
            "\n"
            "N/A\n"
            "N/A\n"
            "N/A\n") );
      window_modifyText( wid, "txtDDesc", buf );
      window_modifyText( wid, "txtOutfitName", _("None") );
      window_modifyText( wid, "txtDescShort", NULL );
      window_modifyText( wid, "txtDescription", NULL );
      /* Reposition. */
      th = 64;
      window_moveWidget( wid, "txtSDesc", 20+iw+20, -40-th-30-32 );
      window_moveWidget( wid, "txtDDesc", 20+iw+20+90, -40-th-30-32 );
      window_moveWidget( wid, "txtDescription", 20+iw+20, -240-32);
      return;
   }

   outfit = iar_outfits[active][i];

   /* new image */
   window_modifyImage( wid, "imgOutfit", outfit->gfx_store, 192, 192 );

   if (outfit_canBuy(outfit->name, land_planet) > 0)
      window_enableButton( wid, "btnBuyOutfit" );
   else
      window_disableButtonSoft( wid, "btnBuyOutfit" );

   /* gray out sell button */
   if (outfit_canSell(outfit->name) > 0)
      window_enableButton( wid, "btnSellOutfit" );
   else
      window_disableButtonSoft( wid, "btnSellOutfit" );

   /* new text */
   window_modifyText( wid, "txtDescription", _(outfit->description) );
   price2str( buf_price, outfit_getPrice(outfit), player.p->credits, 2 );
   credits2str( buf_credits, player.p->credits, 2 );

   if (outfit->license == NULL)
      strncpy( buf_license, _("None"), sizeof(buf_license)-1 );
   else if (player_hasLicense( outfit->license ))
      strncpy( buf_license, _(outfit->license), sizeof(buf_license)-1 );
   else
      nsnprintf( buf_license, sizeof(buf_license), "#r%s#0", _(outfit->license) );
   buf_license[ sizeof(buf_license)-1 ] = '\0';

   mass = outfit->mass;
   if ((outfit_isLauncher(outfit) || outfit_isFighterBay(outfit)) &&
         (outfit_ammo(outfit) != NULL)) {
      mass += outfit_amount(outfit) * outfit_ammo(outfit)->mass;
   }

   nsnprintf( buf, PATH_MAX,
         _("%d\n"
         "\n"
         "%s\n"
         "%s\n"
         "%.0f tonnes\n"
         "\n"
         "%s\n"
         "%s\n"
         "%s\n"),
         player_outfitOwned(outfit),
         _(outfit_slotName(outfit)),
         _(outfit_slotSize(outfit)),
         mass,
         buf_price,
         buf_credits,
         buf_license );
   window_modifyText( wid, "txtDDesc", buf );
   window_modifyText( wid, "txtOutfitName", _(outfit->name) );
   window_modifyText( wid, "txtDescShort", outfit->desc_short );
   th = gl_printHeightRaw( &gl_smallFont, w - (20 + iw + 20) - 200 - 20, outfit->desc_short );
   window_moveWidget( wid, "txtSDesc", 20+iw+20, -40-th-30-32 );
   window_moveWidget( wid, "txtDDesc", 20+iw+20+90, -40-th-30-32 );
   th += gl_printHeightRaw( &gl_smallFont, w - (20 + iw + 20) - 200 - 20, buf );
   th = MAX( th, 192 );
   window_moveWidget( wid, "txtDescription", 20+iw+20, -40-th-30-32 );
}


/**
 * @brief Updates the outfitter and equipment outfit image arrays.
 */
void outfits_updateEquipmentOutfits( void )
{
   int ew, ow;

   if (landed && land_doneLoading()) {
      if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS)) {
         ow = land_getWid( LAND_WINDOW_OUTFITS );
         outfits_regenList( ow, NULL );
      }
      else if (!planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         return;
      ew = land_getWid( LAND_WINDOW_EQUIPMENT );
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
static void outfits_changeTab( unsigned int wid, char *wgt, int old, int tab )
{
   (void) wid;
   (void) wgt;
   int pos;
   double offset;

   toolkit_saveImageArrayData( wid, OUTFITS_IAR, &iar_data[old] );

   /* Store the currently-saved positions for the new tab. */
   pos    = iar_data[tab].pos;
   offset = iar_data[tab].offset;

   /* Resetting the input will cause the outfit list to be regenerated. */
   if (widget_exists(wid, OUTFITS_FILTER))
      window_setInput(wid, OUTFITS_FILTER, NULL);
   else
      outfits_regenList( wid, NULL );

   /* Set positions for the new tab. This is necessary because the stored
    * position for the new tab may have exceeded the size of the old tab,
    * resulting in it being clipped. */
   toolkit_setImageArrayPos(    wid, OUTFITS_IAR, pos );
   toolkit_setImageArrayOffset( wid, OUTFITS_IAR, offset );

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
int outfits_filter( Outfit **outfits, int n,
      int(*filter)( const Outfit *o ), char *name )
{
   int i, j;

   j = 0;
   for (i=0; i<n; i++) {
      if ((filter != NULL) && !filter(outfits[i]))
         continue;

      if ((name != NULL) && (nstrcasestr( outfits[i]->name, name ) == NULL))
         continue;

      /* Shift matches downward. */
      outfits[j] = outfits[i];
      j++;
   }

   return j;
}


/**
 * @brief Starts the map find with outfit search selected.
 *    @param wid Window buying outfit from.
 *    @param str Unused.
 */
static void outfits_find( unsigned int wid, char* str )
{
   (void) str;
   map_inputFindType(wid, "outfit");
}


/**
 * @brief Returns the price of an outfit (subject to quantity modifier)
 */
static credits_t outfit_getPrice( Outfit *outfit )
{
   unsigned int q;
   credits_t price;

   q = outfits_getMod();
   price = outfit->price * q;

   return price;
}


/**
 * @brief Generates image array cells corresponding to outfits.
 */
ImageArrayCell *outfits_imageArrayCells( Outfit **outfits, int *noutfits )
{
   int i;
   int l, p;
   double mass;
   const glColour *c;
   ImageArrayCell *coutfits;
   Outfit *o;
   const char *typename;
   glTexture *t;

   /* Allocate. */
   coutfits = calloc( MAX(1,*noutfits), sizeof(ImageArrayCell) );

   if (*noutfits == 0) {
      *noutfits = 1;
      coutfits[0].image = NULL;
      coutfits[0].caption = strdup( _("None") );
   }
   else {
      /* Set alt text. */
      for (i=0; i<*noutfits; i++) {
         o = outfits[i];

         coutfits[i].image = gl_dupTexture( o->gfx_store );
         coutfits[i].caption = strdup( _(o->name) );
         coutfits[i].quantity = player_outfitOwned(o);

         /* Background colour. */
         c = outfit_slotSizeColour( &o->slot );
         if (c == NULL)
            c = &cBlack;
         col_blend( &coutfits[i].bg, c, &cGrey70, 1 );

         /* Short description. */
         if (o->desc_short == NULL)
            coutfits[i].alt = NULL;
         else {
            mass = o->mass;
            if ((outfit_isLauncher(o) || outfit_isFighterBay(o)) &&
                  (outfit_ammo(o) != NULL)) {
               mass += outfit_amount(o) * outfit_ammo(o)->mass;
            }

            l = strlen(o->desc_short) + 128;
            coutfits[i].alt = malloc( l );
            p  = nsnprintf( &coutfits[i].alt[0], l, "%s\n", _(o->name) );
            if (outfit_isProp(o, OUTFIT_PROP_UNIQUE))
               p += nsnprintf( &coutfits[i].alt[p], l-p, _("#oUnique#0\n") );
            if ((o->slot.spid!=0) && (p < l))
               p += nsnprintf( &coutfits[i].alt[p], l-p, _("#oSlot %s#0\n"),
                     _( sp_display( o->slot.spid ) ) );
            if (p < l)
               p += nsnprintf( &coutfits[i].alt[p], l-p, "\n%s", o->desc_short );
            if ((o->mass > 0.) && (p < l))
               nsnprintf( &coutfits[i].alt[p], l-p,
                     _("\n%.0f Tonnes"),
                     mass );
         }

         /* Slot type. */
         if ( (strcmp(outfit_slotName(o), "N/A") != 0)
               && (strcmp(outfit_slotName(o), "NULL") != 0) ) {
            typename       = outfit_slotName(o);
            coutfits[i].slottype = malloc(2);
            coutfits[i].slottype[0] = typename[0];
            coutfits[i].slottype[1] = '\0';
         }

         /* Layers. */
         coutfits[i].layers = gl_copyTexArray( o->gfx_overlays, o->gfx_noverlays, &coutfits[i].nlayers );
         if (o->rarity > 0) {
            t = rarity_texture( o->rarity );
            coutfits[i].layers = gl_addTexArray( coutfits[i].layers, &coutfits[i].nlayers, t );
         }
      }
   }
   return coutfits;
}



/**
 * @brief Checks to see if the player can buy the outfit.
 *    @param name Outfit to buy.
 *    @param planet Where the player is shopping.
 */
int outfit_canBuy( const char *name, Planet *planet )
{
   int failure;
   credits_t price;
   Outfit *outfit;
   char buf[ECON_CRED_STRLEN];

   failure = 0;
   outfit  = outfit_get(name);
   price   = outfit_getPrice(outfit);

   /* Unique. */
   if (outfit_isProp(outfit, OUTFIT_PROP_UNIQUE) && (player_outfitOwnedTotal(outfit)>0)) {
      land_errDialogueBuild( _("You can only own one of this outfit.") );
      return 0;
   }

   /* Map already mapped */
   if ((outfit_isMap(outfit) && map_isMapped(outfit)) ||
         (outfit_isLocalMap(outfit) && localmap_isMapped(outfit))) {
      land_errDialogueBuild( _("You already know of everything this map contains.") );
      return 0;
   }
   /* GUI already owned */
   if (outfit_isGUI(outfit) && player_guiCheck(outfit->u.gui.gui)) {
      land_errDialogueBuild( _("You already own this GUI.") );
      return 0;
   }
   /* Already has license. */
   if (outfit_isLicense(outfit) && player_hasLicense(outfit->name)) {
      land_errDialogueBuild( _("You already have this license.") );
      return 0;
   }
   /* not enough $$ */
   if (!player_hasCredits(price)) {
      credits2str( buf, price - player.p->credits, 2 );
      land_errDialogueBuild( _("You need %s more."), buf);
      failure = 1;
   }
   /* Needs license. */
   if ((!player_hasLicense(outfit->license)) &&
         ((planet == NULL) || (!planet_hasService(planet, PLANET_SERVICE_BLACKMARKET)))) {
      land_errDialogueBuild( _("You need the '%s' license to buy this outfit."),
               _(outfit->license) );
      failure = 1;
   }

   return !failure;
}


/**
 * @brief Player right-clicks on an outfit.
 *    @param wid Window player is buying ship from.
 *    @param widget_name Name of the window. (unused)
 */
static void outfits_rmouse( unsigned int wid, char* widget_name )
{
    outfits_buy( wid, widget_name );
}

/**
 * @brief Attempts to buy the outfit that is selected.
 *    @param wid Window buying outfit from.
 *    @param str Unused.
 */
static void outfits_buy( unsigned int wid, char* str )
{
   (void) str;
   int i, active;
   Outfit* outfit;
   int q;
   HookParam hparam[3];

   active = window_tabWinGetActive( wid, OUTFITS_TAB );
   i = toolkit_getImageArrayPos( wid, OUTFITS_IAR );
   if (i < 0 || iar_outfits[active] == NULL)
      return;

   outfit = iar_outfits[active][i];
   q = outfits_getMod();
   /* Can only get one unique item. */
   if (outfit_isProp(outfit, OUTFIT_PROP_UNIQUE) ||
         outfit_isMap(outfit) || outfit_isLocalMap(outfit) ||
         outfit_isGUI(outfit) || outfit_isLicense(outfit))
      q = MIN(q,1);

   /* can buy the outfit? */
   if (land_errDialogue( outfit->name, "buyOutfit" ))
      return;

   /* Actually buy the outfit. */
   player_modCredits( -outfit->price * player_addOutfit( outfit, q ) );
   outfits_updateEquipmentOutfits();
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = outfit->name;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = q;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "outfit_buy", hparam );
   if (land_takeoff)
      takeoff(1);

   /* Regenerate list. */
   outfits_regenList( wid, NULL );
}
/**
 * @brief Checks to see if the player can sell the selected outfit.
 *    @param name Outfit to try to sell.
 */
int outfit_canSell( const char *name )
{
   int failure;
   Outfit *outfit;

   failure = 0;
   outfit = outfit_get(name);

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
   if (player_outfitOwned(outfit) <= 0) {
      land_errDialogueBuild( _("You can't sell something you don't have!") );
      failure = 1;
   }

   return !failure;
}
/**
 * @brief Attempts to sell the selected outfit the player has.
 *    @param wid Window selling outfits from.
 *    @param str Unused.
 */
static void outfits_sell( unsigned int wid, char* str )
{
   (void)str;
   int i, active;
   Outfit* outfit;
   int q;
   HookParam hparam[3];

   active = window_tabWinGetActive( wid, OUTFITS_TAB );
   i = toolkit_getImageArrayPos( wid, OUTFITS_IAR );
   if (i < 0 || iar_outfits[active] == NULL)
      return;

   outfit      = iar_outfits[active][i];

   q = outfits_getMod();

   /* Check various failure conditions. */
   if (land_errDialogue( outfit->name, "sellOutfit" ))
      return;

   player_modCredits( outfit->price * player_rmOutfit( outfit, q ) );
   outfits_updateEquipmentOutfits();
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = outfit->name;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = q;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "outfit_sell", hparam );
   if (land_takeoff)
      takeoff(1);

   /* Regenerate list. */
   outfits_regenList( wid, NULL );
}
/**
 * @brief Gets the current modifier status.
 *    @return The amount modifier when buying or selling outfits.
 */
static int outfits_getMod (void)
{
   SDL_Keymod mods;
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
 *    @param data Unused.
 */
static void outfits_renderMod( double bx, double by, double w, double h, void *data )
{
   (void) data;
   (void) h;
   int q;
   char buf[8];

   q = outfits_getMod();
   if (q != outfits_mod) {
      outfits_updateEquipmentOutfits();
      outfits_mod = q;
   }
   if (q==1) return; /* Ignore no modifier. */

   nsnprintf( buf, 8, "%dx", q );
   gl_printMidRaw( &gl_smallFont, w, bx, by, &cFontWhite, -1, buf );
}


/**
 * @brief Cleans up outfit globals.
 */
void outfits_cleanup(void)
{
   /* Free stored positions. */
   free(iar_data);
   iar_data = NULL;
   if (iar_outfits != NULL) {
      for (int i=0; i<OUTFITS_NTABS; i++)
         free( iar_outfits[i] );
      free(iar_outfits);
      iar_outfits = NULL;
   }
}
