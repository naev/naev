/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file land_outfits.c
 *
 * @brief Handles all the landing menus and actions.
 */


#include "land_outfits.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"
#include <math.h>

#include "log.h"
#include "map.h"
#include "equipment.h"
#include "land.h"
#include "outfit.h"
#include "player.h"
#include "player_gui.h"
#include "toolkit.h"
#include "dialogue.h"


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


/**
 * @brief Gets the size of the outfits window.
 */
static void outfits_getSize( unsigned int wid, int *w, int *h,
      int *iw, int *ih, int *bw, int *bh )
{
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Calculate image array dimensions. */
   if (iw != NULL)
      *iw = 310 + (*w-800);
   if (ih != NULL)
      *ih = *h - 60;

   /* Calculate button dimensions. */
   if (bw != NULL)
      *bw = (*w - (iw!=NULL?*iw:0) - 100) / 3;
   if (bh != NULL)
      *bh = LAND_BUTTON_HEIGHT;
}



/**
 * @brief Opens the outfit exchange center window.
 */
void outfits_open( unsigned int wid )
{
   int i;
   Outfit **outfits;
   char **soutfits;
   glTexture **toutfits;
   int noutfits;
   int w, h;
   int iw, ih;
   int bw, bh;
   glColour *bg, blend;
   const glColour *c;
   char **slottype;
   const char *slotname;

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, &bw, &bh );

   /* will allow buying from keyboard */
   window_setAccept( wid, outfits_buy );

   /* buttons */
   window_addButtonKey( wid, -20, 20,
         bw, bh, "btnCloseOutfits",
         "Take Off", land_buttonTakeoff, SDLK_t );
   window_addButtonKey( wid, -40-bw, 20,
         bw, bh, "btnSellOutfit",
         "Sell", outfits_sell, SDLK_s );
   window_addButtonKey( wid, -60-bw*2, 20,
         bw, bh, "btnBuyOutfit",
         "Buy", outfits_buy, SDLK_b );

   /* fancy 128x128 image */
   window_addRect( wid, 19 + iw + 20, -50, 128, 129, "rctImage", &cBlack, 0 );
   window_addImage( wid, 20 + iw + 20, -50-128, 0, 0, "imgOutfit", NULL, 1 );

   /* cust draws the modifier */
   window_addCust( wid, -40-bw, 60+2*bh,
         bw, bh, "cstMod", 0, outfits_renderMod, NULL, NULL );

   /* the descriptive text */
   window_addText( wid, 20 + iw + 20 + 128 + 20, -60,
         320, 160, 0, "txtOutfitName", &gl_defFont, &cBlack, NULL );
   window_addText( wid, 20 + iw + 20 + 128 + 20, -60 - gl_defFont.h - 20,
         320, 160, 0, "txtDescShort", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 20 + iw + 20, -60-128-10,
         60, 160, 0, "txtSDesc", &gl_smallFont, &cDConsole,
         "Owned:\n"
         "\n"
         "Slot:\n"
         "Size:\n"
         "Mass:\n"
         "\n"
         "Price:\n"
         "Money:\n"
         "License:\n" );
   window_addText( wid, 20 + iw + 20 + 60, -60-128-10,
         250, 160, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
   window_addText( wid, 20 + iw + 20, -60-128-10-160,
         w-(iw+80), 180, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );

   /* set up the outfits to buy/sell */
   outfits = tech_getOutfit( land_planet->tech, &noutfits );
   if (noutfits <= 0) { /* No outfits */
      soutfits    = malloc(sizeof(char*));
      soutfits[0] = strdup("None");
      toutfits    = malloc(sizeof(glTexture*));
      toutfits[0] = NULL;
      noutfits    = 1;
      bg          = NULL;
      slottype    = NULL;
   }
   else {
      /* Create the outfit arrays. */
      soutfits = malloc(sizeof(char*)*noutfits);
      toutfits = malloc(sizeof(glTexture*)*noutfits);
      bg       = malloc(sizeof(glColour)*noutfits);
      slottype = malloc(sizeof(char*)*noutfits );
      for (i=0; i<noutfits; i++) {
         soutfits[i] = strdup(outfits[i]->name);
         toutfits[i] = outfits[i]->gfx_store;

         /* Background colour. */
         c = outfit_slotSizeColour( &outfits[i]->slot );
         if (c == NULL)
            c = &cBlack;
         col_blend( &blend, c, &cGrey70, 0.4 );
         memcpy( &bg[i], &blend, sizeof(glColour) );

         /* Get slot name. */
         slotname = outfit_slotName(outfits[i]);
         if ((strcmp(slotname,"NA") != 0) && (strcmp(slotname,"NULL") != 0)) {
            slottype[i]    = malloc( 2 );
            slottype[i][0] = outfit_slotName(outfits[i])[0];
            slottype[i][1] = '\0';
         }
         else
            slottype[i] = NULL;
      }
      free(outfits);
   }
   window_addImageArray( wid, 20, 20,
         iw, ih, "iarOutfits", 64, 64,
         toutfits, soutfits, noutfits, outfits_update, outfits_rmouse );

   /* write the outfits stuff */
   outfits_update( wid, NULL );
   outfits_updateQuantities( wid );
   toolkit_setImageArraySlotType( wid, "iarOutfits", slottype );
   toolkit_setImageArrayBackground( wid, "iarOutfits", bg );
}
/**
 * @brief Updates the quantity counter for the outfits.
 *
 *    @param wid Window to update counters of.
 */
void outfits_updateQuantities( unsigned int wid )
{
   Outfit **outfits, *o;
   int noutfits;
   char **quantity;
   int len, owned;
   int i;

   /* Get outfits. */
   outfits = tech_getOutfit( land_planet->tech, &noutfits );
   if (noutfits <= 0)
      return;

   quantity = malloc(sizeof(char*)*noutfits);
   for (i=0; i<noutfits; i++) {
      o = outfits[i];
      owned = player_outfitOwned(o);
      len = owned / 10 + 4;
      if (owned >= 1) {
         quantity[i] = malloc( len );
         nsnprintf( quantity[i], len, "%d", owned );
      }
      else
         quantity[i] = NULL;
   }
   free(outfits);
   toolkit_setImageArrayQuantity( wid, "iarOutfits", quantity );
}
/**
 * @brief Updates the outfits in the outfit window.
 *    @param wid Window to update the outfits in.
 *    @param str Unused.
 */
void outfits_update( unsigned int wid, char* str )
{
   (void)str;
   char *outfitname;
   Outfit* outfit;
   char buf[PATH_MAX], buf2[ECON_CRED_STRLEN], buf3[ECON_CRED_STRLEN];
   double th;
   int iw, ih;
   int w, h;

   /* Get dimensions. */
   outfits_getSize( wid, &w, &h, &iw, &ih, NULL, NULL );

   /* Get and set parameters. */
   outfitname = toolkit_getImageArray( wid, "iarOutfits" );
   if (strcmp(outfitname,"None")==0) { /* No outfits */
      window_modifyImage( wid, "imgOutfit", NULL, 0, 0 );
      window_disableButton( wid, "btnBuyOutfit" );
      window_disableButton( wid, "btnSellOutfit" );
      nsnprintf( buf, PATH_MAX,
            "NA\n"
            "\n"
            "NA\n"
            "NA\n"
            "NA\n"
            "\n"
            "NA\n"
            "NA\n"
            "NA\n" );
      window_modifyText( wid, "txtDDesc", buf );
      window_modifyText( wid, "txtOutfitName", "None" );
      window_modifyText( wid, "txtDescShort", NULL );
      /* Reposition. */
      window_moveWidget( wid, "txtSDesc", 20+iw+20, -60 );
      window_moveWidget( wid, "txtDDesc", 20+iw+20+60, -60 );
      window_moveWidget( wid, "txtDescription", 20+iw+40, -240 );
      return;
   }

   outfit = outfit_get( outfitname );

   /* new image */
   window_modifyImage( wid, "imgOutfit", outfit->gfx_store, 0, 0 );

   if (outfit_canBuy(outfitname) > 0)
      window_enableButton( wid, "btnBuyOutfit" );
   else
      window_disableButtonSoft( wid, "btnBuyOutfit" );

   /* gray out sell button */
   if (outfit_canSell(outfitname) > 0)
      window_enableButton( wid, "btnSellOutfit" );
   else
      window_disableButtonSoft( wid, "btnSellOutfit" );

   /* new text */
   window_modifyText( wid, "txtDescription", outfit->description );
   credits2str( buf2, outfit->price, 2 );
   credits2str( buf3, player.p->credits, 2 );
   nsnprintf( buf, PATH_MAX,
         "%d\n"
         "\n"
         "%s\n"
         "%s\n"
         "%.0f tons\n"
         "\n"
         "%s credits\n"
         "%s credits\n"
         "%s\n",
         player_outfitOwned(outfit),
         outfit_slotName(outfit),
         outfit_slotSize(outfit),
         outfit->mass,
         buf2,
         buf3,
         (outfit->license != NULL) ? outfit->license : "None" );
   window_modifyText( wid, "txtDDesc", buf );
   window_modifyText( wid, "txtOutfitName", outfit->name );
   window_modifyText( wid, "txtDescShort", outfit->desc_short );
   th = MAX( 128, gl_printHeightRaw( &gl_smallFont, 320, outfit->desc_short ) );
   window_moveWidget( wid, "txtSDesc", 40+iw+20, -60-th-20 );
   window_moveWidget( wid, "txtDDesc", 40+iw+20+60, -60-th-20 );
   th += gl_printHeightRaw( &gl_smallFont, 250, buf );
   window_moveWidget( wid, "txtDescription", 20+iw+40, -60-th-20 );
}


/**
 * @brief Updates the outfitter and equipment outfit image arrays.
 */
void outfits_updateEquipmentOutfits( void )
{
   int ew, ow;

   if (landed && land_doneLoading()) {
      if planet_hasService(land_planet, PLANET_SERVICE_OUTFITS) {
         ow = land_getWid( LAND_WINDOW_OUTFITS );
         outfits_update(ow, NULL);
         outfits_updateQuantities(ow);
      }
      else if (!planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         return;
      ew = land_getWid( LAND_WINDOW_EQUIPMENT );
      equipment_addAmmo();
      equipment_regenLists( ew, 1, 0 );
   }
}


/**
 * @brief Checks to see if the player can buy the outfit.
 *    @param outfit Outfit to buy.
 */
int outfit_canBuy( char *name )
{
   int failure;
   unsigned int q, price;
   Outfit *outfit;
   char buf[ECON_CRED_STRLEN];

   failure = 0;
   q       = outfits_getMod();
   outfit  = outfit_get(name);
   price   = outfit->price * q;

   /* takes away cargo space but you don't have any */
   if (outfit_isMod(outfit) && (outfit->u.mod.cargo < 0)
         && (pilot_cargoFree(player.p) < -outfit->u.mod.cargo)) {
      land_errDialogueBuild( "You need to empty your cargo first." );
      failure = 1;
   }
   /* Map already mapped */
   if ((outfit_isMap(outfit) && map_isMapped(outfit)) ||
         (outfit_isLocalMap(outfit) && localmap_isMapped(outfit))) {
      land_errDialogueBuild( "You already own this map." );
      failure = 1;
      return 0;
   }
   /* GUI already owned */
   if (outfit_isGUI(outfit) && player_guiCheck(outfit->u.gui.gui)) {
      land_errDialogueBuild( "You already own this GUI." );
      return 0;
   }
   /* Already has license. */
   if (outfit_isLicense(outfit) && player_hasLicense(outfit->name)) {
      land_errDialogueBuild( "You already have this license." );
      return 0;
   }
   /* not enough $$ */
   if (!player_hasCredits( q*outfit->price )) {
      credits2str( buf, price - player.p->credits, 2 );
      land_errDialogueBuild( "You need %s more credits.", buf);
      failure = 1;
   }
   /* Needs license. */
   if (!player_hasLicense(outfit->license)) {
      land_errDialogueBuild( "You need the '%s' license to buy this outfit.",
               outfit->license );
      failure = 1;
   }

   return !failure;
}


/**
 * @brief Player right-clicks on an outfit.
 *    @param wid Window player is buying ship from.
 *    @param widget_name Name of the window. (unused)
 *    @param shipname Name of the ship the player wants to buy. (unused)
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
   char *outfitname;
   Outfit* outfit;
   int q;

   outfitname = toolkit_getImageArray( wid, "iarOutfits" );
   outfit = outfit_get( outfitname );

   q = outfits_getMod();

   /* can buy the outfit? */
   if (land_errDialogue( outfitname, "buyOutfit" ))
      return;

   /* Actually buy the outfit. */
   player_modCredits( -outfit->price * player_addOutfit( outfit, q ) );
   land_checkAddRefuel();
   outfits_updateEquipmentOutfits();
}
/**
 * @brief Checks to see if the player can sell the selected outfit.
 *    @param outfit Outfit to try to sell.
 */
int outfit_canSell( char *name )
{
   int failure;
   Outfit *outfit;

   failure = 0;
   outfit = outfit_get(name);

   /* Map check. */
   if (outfit_isMap(outfit) || outfit_isLocalMap(outfit)) {
      land_errDialogueBuild("You can't sell a map.");
      failure = 1;
   }

   /* GUI check. */
   if (outfit_isGUI(outfit)) {
      land_errDialogueBuild("You can't sell a GUI.");
      failure = 1;
   }

   /* License check. */
   if (outfit_isLicense(outfit)) {
      land_errDialogueBuild("You can't sell a license.");
      failure = 1;
   }

   /* has no outfits to sell */
   if (player_outfitOwned(outfit) <= 0) {
      land_errDialogueBuild( "You can't sell something you don't have!" );
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
   char *outfitname;
   Outfit* outfit;
   int q;

   outfitname  = toolkit_getImageArray( wid, "iarOutfits" );
   outfit      = outfit_get( outfitname );

   q = outfits_getMod();

   /* Check various failure conditions. */
   if (land_errDialogue( outfitname, "sellOutfit" ))
      return;

   player_modCredits( outfit->price * player_rmOutfit( outfit, q ) );
   land_checkAddRefuel();
   outfits_updateEquipmentOutfits();
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
   gl_printMid( &gl_smallFont, w, bx, by, &cBlack, buf );
}


