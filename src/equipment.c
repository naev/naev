/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file equipment.c
 *
 * @brief Handles equipping ships.
 */


/** @cond */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "equipment.h"

#include "conf.h"
#include "dialogue.h"
#include "gui.h"
#include "hook.h"
#include "info.h"
#include "land.h"
#include "land_outfits.h"
#include "land_takeoff.h"
#include "log.h"
#include "map.h"
#include "mission.h"
#include "ndata.h"
#include "nstring.h"
#include "ntime.h"
#include "player.h"
#include "shipstats.h"
#include "slots.h"
#include "tk/toolkit_priv.h" /* Yes, I'm a bad person, abstractions be damned! */
#include "toolkit.h"


/*
 * Image array names.
 */
#define  EQUIPMENT_SHIPS      "iarAvailShips"
#define  EQUIPMENT_OUTFIT_TAB "tabOutfits"
#define  EQUIPMENT_OUTFITS    "iarAvailOutfits"
#define  EQUIPMENT_FILTER     "inpFilterOutfits"
#define  OUTFIT_TABS          5


/* global/main window */
#define BUTTON_WIDTH    200 /**< Default button width. */
#define BUTTON_HEIGHT   40 /**< Default button height. */

#define SHIP_ALT_MAX    512 /**< Maximum ship alt text. */


/*
 * equipment stuff
 */
static CstSlotWidget eq_wgt; /**< Equipment widget. */
static double equipment_dir      = 0.; /**< Equipment dir. */
static unsigned int equipment_lastick = 0; /**< Last tick. */
static unsigned int equipment_wid   = 0; /**< Global wid. */
static iar_data_t *iar_data = NULL; /**< Stored image array positions. */
static Outfit ***iar_outfits = NULL; /**< Outfits associated with the image array cells. */

/* Slot textures */
static glTexture *equip_ico_yes = NULL; /* Green circle */
static glTexture *equip_ico_no  = NULL; /* Red circle with slash */

/*
 * prototypes
 */
/* Creation. */
static void equipment_getDim( unsigned int wid, int *w, int *h,
      int *sw, int *sh, int *ow, int *oh,
      int *ew, int *eh,
      int *cw, int *ch, int *bw, int *bh );
static void equipment_genShipList( unsigned int wid );
static void equipment_genOutfitList( unsigned int wid );
/* Widget. */
static void equipment_genLists( unsigned int wid );
static void equipment_renderColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, const char *txt,
      int selected, Outfit *o, Pilot *p, CstSlotWidget *wgt );
static void equipment_renderSlots( double bx, double by, double bw, double bh, void *data );
static void equipment_renderMisc( double bx, double by, double bw, double bh, void *data );
static void equipment_renderOverlayColumn( double x, double y, double h,
      int n, PilotOutfitSlot *lst, int mover, CstSlotWidget *wgt );
static void equipment_renderOverlaySlots( double bx, double by, double bw, double bh,
      void *data );
static void equipment_renderShip( double bx, double by,
      double bw, double bh, double x, double y, Pilot *p );
static int equipment_mouseInColumn( double y, double h, int n, double my );
static int equipment_mouseSlots( unsigned int wid, SDL_Event* event,
      double x, double y, double w, double h, double rx, double ry, void *data );
/* Misc. */
static char eq_qCol( double cur, double base, int inv );
static int equipment_swapSlot( unsigned int wid, Pilot *p, PilotOutfitSlot *slot );
static void equipment_sellShip( unsigned int wid, char* str );
static void equipment_renameShip( unsigned int wid, char *str );
static void equipment_transChangeShip( unsigned int wid, char* str );
static void equipment_changeShip( unsigned int wid );
static void equipment_unequipShip( unsigned int wid, char* str );
static void equipment_filterOutfits( unsigned int wid, char *str );
static void equipment_rightClickOutfits( unsigned int wid, char* str );
static void equipment_changeTab( unsigned int wid, char *wgt, int old, int tab );
static int equipment_playerAddOutfit( const Outfit *o, int quantity );
static int equipment_playerRmOutfit( const Outfit *o, int quantity );


/**
 * @brief Handles right-click on unequipped outfit.
 *    @param wid Window to update.
 *    @param str Widget name. Must be EQUIPMENT_OUTFITS.
 */
void equipment_rightClickOutfits( unsigned int wid, char* str )
{
   (void)str;
   Outfit* o;
   int i, active;
   int outfit_n;
   PilotOutfitSlot* slots;
   Pilot *p;

   active = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
   i = toolkit_getImageArrayPos( wid, EQUIPMENT_OUTFITS );

   /* Did the user click on background or placeholder cell? */
   if (i < 0 || iar_outfits[active] == NULL)
      return;

   o = iar_outfits[active][i];

   /* Figure out which slot this stuff fits into */
   switch (o->slot.type) {
      case OUTFIT_SLOT_STRUCTURE:
         outfit_n = eq_wgt.selected->outfit_nstructure;
         slots    = eq_wgt.selected->outfit_structure;
         break;
      case OUTFIT_SLOT_UTILITY:
         outfit_n = eq_wgt.selected->outfit_nutility;
         slots    = eq_wgt.selected->outfit_utility;
         break;
      case OUTFIT_SLOT_WEAPON:
         outfit_n = eq_wgt.selected->outfit_nweapon;
         slots    = eq_wgt.selected->outfit_weapon;
         break;
      default:
         return;
   }

   /* Loop through outfit slots of the right type, try to find an empty one */
   for (i=0; i<outfit_n; i++) {

      /* Slot full. */
      if (slots[i].outfit != NULL)
         continue;

      /* Must fit the slot. */
      if (!outfit_fitsSlot( o, &slots[i].sslot->slot))
         continue;

      /* Bingo! */
      eq_wgt.outfit  = o;
      p              = eq_wgt.selected;
      equipment_swapSlot( equipment_wid, p, &slots[i] );
      return;
   }
}

/**
 * @brief Gets the window dimensions.
 */
static void equipment_getDim( unsigned int wid, int *w, int *h,
      int *sw, int *sh, int *ow, int *oh,
      int *ew, int *eh,
      int *cw, int *ch, int *bw, int *bh )
{
   int ssw, ssh;
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Calculate image array dimensions. */
   ssw = 550 + (*w - LAND_WIDTH);
   ssh = (*h - 100);
   if (sw != NULL)
      *sw = ssw;
   if (sh != NULL)
      *sh = (1*ssh)/3;
   if (ow != NULL)
      *ow = ssw;
   if (oh != NULL)
      *oh = (2*ssh)/3;

   /* Calculate slot widget. */
   if (ew != NULL)
      *ew = 180.;
   if (eh != NULL)
      *eh = *h - 100;

   /* Calculate custom widget. */
   if (cw != NULL)
      *cw = *w - 20 - (sw!=NULL?*sw:0) - 20 - (ew!=NULL?*ew:0) - 20.;
   if (ch != NULL)
      *ch = *h - 100;

   /* Calculate button dimensions. */
   if (bw != NULL)
      *bw = (*w - 20 - (sw!=NULL?*sw:0) - 40 - 20 - 60) / 5;
   if (bh != NULL)
      *bh = BUTTON_HEIGHT;
}

/**
 * @brief Opens the player's equipment window.
 */
void equipment_open( unsigned int wid )
{
   int w, h;
   int sw, sh;
   int ow, oh;
   int bw, bh;
   int ew, eh;
   int cw, ch;
   int x, y;
   const char *buf;

   /* Mark as generated. */
   land_tabGenerate(LAND_WINDOW_EQUIPMENT);

   /* Set global WID. */
   equipment_wid = wid;

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, &sw, &sh, &ow, &oh,
         &ew, &eh, &cw, &ch, &bw, &bh );

   /* Prepare the outfit array. */
   if (iar_data == NULL)
      iar_data = calloc( OUTFIT_TABS, sizeof(iar_data_t) );
   else
      memset( iar_data, 0, sizeof(iar_data_t) * OUTFIT_TABS );
   if (iar_outfits == NULL)
      iar_outfits = calloc( OUTFIT_TABS, sizeof(Outfit**) );
   else {
      for (int i=0; i<OUTFIT_TABS; i++)
         free( iar_outfits[i] );
      memset( iar_outfits, 0, sizeof(Outfit**) * OUTFIT_TABS );
   }

   /* Safe defaults. */
   equipment_lastick    = SDL_GetTicks();
   equipment_dir        = 0.;
   eq_wgt.selected      = NULL;

   /* Icons */
   if (equip_ico_yes == NULL)
      equip_ico_yes = gl_newImage( GUI_GFX_PATH"yes.png", 0);
   if (equip_ico_no == NULL)
      equip_ico_no  = gl_newImage( GUI_GFX_PATH"no.png", 0);

   /* Add ammo. */
   equipment_addAmmo();

   /* buttons */
   window_addButtonKey( wid, -20, 20,
         bw, bh, "btnCloseEquipment",
         _("Take Off"), land_buttonTakeoff, SDLK_t );
   window_addButtonKey( wid, -20 - (15+bw), 20,
         bw, bh, "btnRenameShip",
         _("Rename"), equipment_renameShip, SDLK_r );
   window_addButtonKey( wid, -20 - (15+bw)*2, 20,
         bw, bh, "btnSellShip",
         _("Sell Ship"), equipment_sellShip, SDLK_s );
   window_addButtonKey( wid, -20 - (15+bw)*3, 20,
         bw, bh, "btnChangeShip",
         _("Swap Ship"), equipment_transChangeShip, SDLK_p );
   window_addButtonKey( wid, -20 - (15+bw)*4, 20,
         bw, bh, "btnUnequipShip",
         _("Unequip"), equipment_unequipShip, SDLK_u );

   /* text */
   buf = _("#nName:\n#0"
      "#nModel:\n#0"
      "#nClass:\n#0"
      "#nCrew:\n#0"
      "#nValue:\n#0"
      "\n"
      "#nMass:\n#0"
      "#nJump Time:\n#0"
      "#nThrust:\n#0"
      "#nSpeed:\n#0"
      "#nTurn:\n#0"
      "#nTime Constant:\n#0"
      "\n"
      "#nAbsorption:\n#0"
      "#nShield:\n#0"
      "#nArmour:\n#0"
      "#nEnergy:\n#0"
      "#nCargo Space:\n#0"
      "#nFuel:\n#0"
      "\n"
      "#nShip Status:#0");
   x = 20 + sw + 20 + 180 + 20 + 30;
   y = -190;
   window_addText( wid, x, y,
         100, y-20+h-bh, 0, "txtSDesc", &gl_smallFont, NULL, buf );
   x += 150;
   window_addText( wid, x, y,
         w - x - 20, y-20+h-bh, 0, "txtDDesc", &gl_smallFont, NULL, NULL );

   /* Generate lists. */
   window_addText( wid, 30, -20,
         ow, gl_defFont.h, 0, "txtShipTitle", &gl_defFont, NULL, _("Available Ships") );
   window_addText( wid, 30, -40-sh-20,
         ow, gl_defFont.h, 0, "txtOutfitTitle", &gl_defFont, NULL, _("Available Outfits") );
   equipment_genLists( wid );

   /* Separator. */
   // window_addRect( wid, 20 + sw + 20, -40, 2, h-60, "rctDivider", &cGrey50, 0 );

   /* Slot widget. */
   equipment_slotWidget( wid, 20 + sw + 40, -40, ew, eh, &eq_wgt );
   eq_wgt.canmodify = 1;

   /* Custom widget (ship information). */
   window_addCust( wid, 20 + sw + 40 + ew + 20, -40, cw, ch, "cstMisc", 0,
         equipment_renderMisc, NULL, NULL );

   /* Focus the ships image array. */
   window_setFocus( wid , EQUIPMENT_SHIPS );
}


/**
 * @brief Creates the slot widget and initializes it.
 *
 *    @param wid Parent window id.
 *    @param x X position to put it at.
 *    @param y Y position to put it at.
 *    @param w Width.
 *    @param h Height;
 *    @param data Dataset to use.
 */
void equipment_slotWidget( unsigned int wid,
      double x, double y, double w, double h,
      CstSlotWidget *data )
{
   /* Initialize data. */
   memset( data, 0, sizeof(CstSlotWidget) );
   data->slot        = -1;
   data->mouseover   = -1;
   data->weapons     = -1;

   /* Create the widget. */
   window_addCust( wid, x, y, w, h, "cstEquipment", 0,
         equipment_renderSlots, equipment_mouseSlots, data );
   window_custSetClipping( wid, "cstEquipment", 0 );
   window_custSetOverlay( wid, "cstEquipment", equipment_renderOverlaySlots );
}
/**
 * @brief Renders an outfit column.
 */
static void equipment_renderColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, const char *txt,
      int selected, Outfit *o, Pilot *p, CstSlotWidget *wgt )
{
   int i, level;
   const glColour *c, *dc, *rc;
   glColour bc;

   /* Render text. */
   if ((o != NULL) && (lst[0].sslot->slot.type == o->slot.type))
      c = &cFontGreen;
   else
      c = &cFontWhite;
   gl_printMidRaw( &gl_smallFont, 60.,
         x-15., y+h+10., c, -1., txt );

   /* Iterate for all the slots. */
   for (i=0; i<n; i++) {
      /* Choose default colour. */
      if (wgt->weapons >= 0) {
         level = pilot_weapSetCheck( p, wgt->weapons, &lst[i] );
         if (level == 0)
            dc = &cFontRed;
         else if (level == 1)
            dc = &cFontYellow;
         else if (lst[i].active)
            dc = &cFontBlue;
         else
            dc = &cFontGrey;
      }
      else
         dc = outfit_slotSizeColour( &lst[i].sslot->slot );

      /* Choose colours based on size. */
      if ((i==selected) && (dc == NULL))
         dc = &cGrey60;

      /* Draw background. */
      bc = *dc;
      bc.a = 0.4;
      if (i==selected)
         c = &cGreen;
      else
         c = &bc;
      toolkit_drawRect( x, y, w, h, c, NULL );

      if (lst[i].outfit != NULL) {
         /* Draw bugger. */
         gl_blitScale( lst[i].outfit->gfx_store,
               x, y, w, h, NULL );
      }
      else if ((o != NULL) &&
            (lst[i].sslot->slot.type == o->slot.type)) {
         /* Render the appropriate sprite, centered in each slot. */
         if (pilot_canEquip( p, &lst[i], o ) != NULL)
            gl_blitScale( equip_ico_no,
               x + w * .1, y + h * .1, w * .8, h * .8, NULL);
         else
            gl_blitScale( equip_ico_yes,
               x + w * .1, y + h * .1, w * .8, h * .8, NULL);
      }

      /* Must rechoose colour based on slot properties. */
      if (lst[i].sslot->required)
         rc = &cBrightRed;
      else if (lst[i].sslot->slot.spid != 0)
         rc = &cWhite;
      else
         rc = dc;

      /* Draw outline. */
      toolkit_drawOutlineThick( x, y, w, h, 1, 3, rc, NULL );
      // toolkit_drawOutline( x-1, y-1, w+3, h+3, 0, &cBlack, NULL );
      /* Go to next one. */
      y -= h+20;
   }
}


/**
 * @brief Calculates the size the slots need to be for a given window.
 *
 *    @param p Pilot to calculate the slots of.
 *    @param bw Base widget width.
 *    @param bh Base window height.
 *    @param w Width to use.
 *    @param h Height to use.
 *    @param n Number of columns.
 *    @param m Number of rows.
 */
static void equipment_calculateSlots( Pilot *p, double bw, double bh,
      double *w, double *h, int *n, int *m )
{
   double tw, th, s;
   int tm;

   /* Calculate size. */
   tm = MAX( MAX( p->outfit_nweapon, p->outfit_nutility ), p->outfit_nstructure );
   th = bh / (double)tm;
   tw = bw / 3.;
   s  = MIN( th, tw ) - 20.;
   th = s;
   tw = s;

   /* Return. */
   *w = tw;
   *h = th;
   *n = 3;
   *m = tm;
}
/**
 * @brief Renders the equipment slots.
 *
 *    @param bx Base X position of the widget.
 *    @param by Base Y position of the widget.
 *    @param bw Width of the widget.
 *    @param bh Height of the widget.
 *    @param data Custom widget data.
 */
static void equipment_renderSlots( double bx, double by, double bw, double bh, void *data )
{
   double x, y;
   double w, h;
   double tw;
   int n, m;
   CstSlotWidget *wgt;
   Pilot *p;
   int selected;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   p   = wgt->selected;
   selected = wgt->slot;

   /* Must have selected ship. */
   if (p == NULL)
      return;

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;

   /* Draw weapon outfits. */
   x  = bx + (tw-w)/2;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nweapon, p->outfit_weapon, _("Weapon"),
         selected, wgt->outfit, wgt->selected, wgt );

   /* Draw systems outfits. */
   selected -= p->outfit_nweapon;
   x += tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nutility, p->outfit_utility, _("Utility"),
         selected, wgt->outfit, wgt->selected, wgt );

   /* Draw structure outfits. */
   selected -= p->outfit_nutility;
   x += tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nstructure, p->outfit_structure, _("Structure"),
         selected, wgt->outfit, wgt->selected, wgt );
}
/**
 * @brief Renders the custom equipment widget.
 *
 *    @param bx Base X position of the widget.
 *    @param by Base Y position of the widget.
 *    @param bw Width of the widget.
 *    @param bh Height of the widget.
 *    @param data Custom widget data.
 */
static void equipment_renderMisc( double bx, double by, double bw, double bh, void *data )
{
   (void) data;
   Pilot *p;
   double percent;
   double x, y;
   double w, h;

   /* Must have selected ship. */
   if (eq_wgt.selected == NULL)
      return;

   p = eq_wgt.selected;

   /* Render CPU and energy bars. */
   w = 120;
   h = 20;
   x = bx + 50.;
   y = by + bh - 30 - h;

   gl_printMidRaw( &gl_smallFont, w,
      x, y + h + 10., &cFontWhite, -1, _("CPU Free") );

   percent = (p->cpu_max > 0) ? CLAMP(0., 1., (float)p->cpu / (float)p->cpu_max) : 0.;
   toolkit_drawRect( x, y - 2, w * percent, h + 4, &cGreen, NULL );
   toolkit_drawRect( x + w * percent, y - 2, w * (1.-percent), h + 4, &cRed, NULL );
   gl_printMid( &gl_smallFont, w,
      x, y + h / 2. - gl_smallFont.h / 2.,
      &cFontWhite, "%d / %d", p->cpu, p->cpu_max );

   y -= h;

   gl_printMidRaw( &gl_smallFont, w,
      x, y, &cFontWhite, -1., _("Mass Limit Left") );

   y -= gl_smallFont.h + h;

   percent = (p->stats.engine_limit > 0) ? CLAMP(0., 1.,
      (p->stats.engine_limit - p->solid->mass) / p->stats.engine_limit) : 0.;
   toolkit_drawRect( x, y - 2, w * percent, h + 4, &cGreen, NULL );
   toolkit_drawRect( x + w * percent, y - 2, w * (1.-percent), h + 4, &cOrange, NULL );
   gl_printMid( &gl_smallFont, w,
      x, y + h / 2. - gl_smallFont.h / 2.,
      &cFontWhite, "%.0f / %.0f", p->stats.engine_limit - p->solid->mass, p->stats.engine_limit );

   if (p->stats.engine_limit > 0. && p->solid->mass > p->stats.engine_limit) {
      y -= h;
      gl_printMid( &gl_smallFont, w,
         x, y, &cFontRed, _("!! %.0f%% Slower !!"),
         (1. - p->speed / p->speed_base) * 100);
   }

   x += w/2. + 50;

   /* Render ship graphic. */
   equipment_renderShip( bx, by, bw, bh, x, y, p );
}


/**
 * @brief Renders an outfit column.
 *
 *    @param x X position to render at.
 *    @param y Y position to render at.
 *    @param h Height.
 *    @param n Number of elements.
 *    @param lst List of elements.
 *    @param mover Slot for which mouseover is active
 *    @param wgt Widget rendering.
 */
static void equipment_renderOverlayColumn( double x, double y, double h,
      int n, PilotOutfitSlot *lst, int mover, CstSlotWidget *wgt )
{
   int i;
   const glColour *c;
   glColour tc;
   int text_width, yoff, top;
   const char *display;
   int subtitle;

   /* Iterate for all the slots. */
   for (i=0; i<n; i++) {
      subtitle = 0;
      if (lst[i].outfit != NULL) {
         /* See if needs a subtitle. */
         if ((outfit_isLauncher(lst[i].outfit) ||
                  (outfit_isFighterBay(lst[i].outfit))) &&
               ((lst[i].u.ammo.outfit == NULL) ||
                (lst[i].u.ammo.quantity < outfit_amount(lst[i].outfit))))
            subtitle = 1;
      }
      /* Draw bottom. */
      if ((i==mover) || subtitle) {
         display = NULL;
         if ((i==mover) && (wgt->canmodify)) {
            if (lst[i].outfit != NULL) {
               top = 1;
               display = pilot_canEquip( wgt->selected, &lst[i], NULL );
               if (display != NULL)
                  c = &cFontRed;
               else {
                  display = _("Right click to remove");
                  c = &cFontGreen;
               }
            }
            else if ((wgt->outfit != NULL) &&
                  (lst->sslot->slot.type == wgt->outfit->slot.type)) {
               top = 0;
               display = pilot_canEquip( wgt->selected, &lst[i], wgt->outfit );
               if (display != NULL)
                  c = &cFontRed;
               else {
                  display = _("Right click to add");
                  c = &cFontGreen;
               }
            }
         }
         else if (lst[i].outfit != NULL) {
            top = 1;
         }

         if (display != NULL) {
            text_width = gl_printWidthRaw( &gl_smallFont, display );
            if (top)
               yoff = h + 2;
            else
               yoff = -gl_smallFont.h - 3;
            tc.r = 0.;
            tc.g = 0.;
            tc.b = 0.;
            tc.a = 0.9;
            toolkit_drawRect( x, y -5. + yoff,
                  text_width+60, gl_smallFont.h+10,
                  &tc, NULL );
            gl_printMaxRaw( &gl_smallFont, text_width,
                  x+5, y + yoff,
                  c, -1., display );
         }
      }
      /* Go to next one. */
      y -= h+20;
   }
}
/**
 * @brief Renders the equipment overlay.
 *
 *    @param bx Base X position of the widget.
 *    @param by Base Y position of the widget.
 *    @param bw Width of the widget.
 *    @param bh Height of the widget.
 *    @param data Custom widget data.
 */
static void equipment_renderOverlaySlots( double bx, double by, double bw, double bh,
      void *data )
{
   (void) bw;
   Pilot *p;
   int mover;
   double x, y;
   double w, h;
   double tw;
   int n, m;
   PilotOutfitSlot *slot;
   char alt[1024];
   int pos;
   Outfit *o;
   CstSlotWidget *wgt;
   double mass;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   p   = wgt->selected;

   /* Must have selected ship. */
   if (wgt->selected == NULL)
      return;

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;

   /* Get selected. */
   mover    = wgt->mouseover;

   /* Render weapon outfits. */
   x  = bx + (tw-w)/2;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderOverlayColumn( x, y, h,
         p->outfit_nweapon, p->outfit_weapon, mover, wgt );
   mover    -= p->outfit_nweapon;
   x += tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderOverlayColumn( x, y, h,
         p->outfit_nutility, p->outfit_utility, mover, wgt );
   mover    -= p->outfit_nutility;
   x += tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderOverlayColumn( x, y, h,
         p->outfit_nstructure, p->outfit_structure, mover, wgt );

   /* Mouse must be over something. */
   if (wgt->mouseover < 0)
      return;

   /* Get the slot. */
   if (wgt->mouseover < p->outfit_nweapon)
      slot = &p->outfit_weapon[wgt->mouseover];
   else if (wgt->mouseover < p->outfit_nweapon + p->outfit_nutility)
      slot = &p->outfit_utility[ wgt->mouseover - p->outfit_nweapon ];
   else
      slot = &p->outfit_structure[ wgt->mouseover -
         p->outfit_nweapon - p->outfit_nutility ];

   /* For comfortability. */
   o = slot->outfit;

   /* Slot is empty. */
   if (o == NULL) {
      if (slot->sslot->slot.spid == 0)
         return;

      pos = nsnprintf( alt, sizeof(alt),
            "#o%s", _( sp_display( slot->sslot->slot.spid ) ) );
      if (slot->sslot->slot.exclusive && (pos < (int)sizeof(alt)))
         pos += nsnprintf( &alt[pos], sizeof(alt)-pos,
               _(" [exclusive]") );
      if (pos < (int)sizeof(alt))
         nsnprintf( &alt[pos], sizeof(alt)-pos,
               "\n\n%s", _( sp_description( slot->sslot->slot.spid ) ) );
      toolkit_drawAltText( bx + wgt->altx, by + wgt->alty, alt );
      return;
   }

   mass = o->mass;
   if ((outfit_isLauncher(o) || outfit_isFighterBay(o)) &&
         (outfit_ammo(o) != NULL)) {
      mass += outfit_amount(o) * outfit_ammo(o)->mass;
   }

   /* Get text. */
   if (o->desc_short == NULL)
      return;
   pos = nsnprintf( alt, sizeof(alt),
         "%s",
         _(o->name) );
   if (outfit_isProp(o, OUTFIT_PROP_UNIQUE))
      pos += nsnprintf( &alt[pos], sizeof(alt)-pos, _("\n#oUnique#0") );
   if ((o->slot.spid!=0) && (pos < (int)sizeof(alt)))
      pos += nsnprintf( &alt[pos], sizeof(alt)-pos, _("\n#oSlot %s#0"),
            _( sp_display( o->slot.spid ) ) );
   if (pos < (int)sizeof(alt))
      pos += nsnprintf( &alt[pos], sizeof(alt)-pos, "\n\n%s", o->desc_short );
   if ((o->mass > 0.) && (pos < (int)sizeof(alt)))
      nsnprintf( &alt[pos], sizeof(alt)-pos,
            ngettext("\n%.0f Tonne", "\n%.0f Tonnes", mass),
            mass );

   /* Draw the text. */
   toolkit_drawAltText( bx + wgt->altx, by + wgt->alty, alt );
}


/**
 * @brief Renders the ship in the equipment window.
 *
 *    @param bx Base X position of the widget.
 *    @param by Base Y position of the widget.
 *    @param bw Width of the widget.
 *    @param bh Height of the widget.
 *    @param x X position to render at.
 *    @param y Y position to render at.
 *    @param p Pilot to render.
 */
static void equipment_renderShip( double bx, double by,
      double bw, double bh, double x, double y, Pilot* p )
{
   int sx, sy;
   unsigned int tick;
   double dt;
   double px, py;
   double pw, ph;
   double w, h;
   Vector2d v;

   tick = SDL_GetTicks();
   dt   = (double)(tick - equipment_lastick)/1000.;
   equipment_lastick = tick;
   equipment_dir += p->turn * dt;
   if (equipment_dir > 2*M_PI)
      equipment_dir = fmod( equipment_dir, 2*M_PI );
   gl_getSpriteFromDir( &sx, &sy, p->ship->gfx_space, equipment_dir );

   /* Render ship graphic. */
   if (p->ship->gfx_space->sw > 128) {
      pw = 128;
      ph = 128;
   }
   else {
      pw = p->ship->gfx_space->sw;
      ph = p->ship->gfx_space->sh;
   }
   w  = 128;
   h  = 128;
   px = (x+30) + (bx+bw - (x+30) - pw)/2;
   py = by + bh - 30 - h + (h-ph)/2 + 30;
   x  = (x+30) + (bx+bw - (x+30) - w)/2;
   y  = by + bh - 30 - h + 30;
   toolkit_drawRect( x-5, y-5, w+10, h+10, &cBlack, NULL );
   gl_blitScaleSprite( p->ship->gfx_space,
         px, py, sx, sy, pw, ph, NULL );
   if ((eq_wgt.slot >= 0) && (eq_wgt.slot < p->outfit_nweapon)) {
      p->tsx = sx;
      p->tsy = sy;
      pilot_getMount( p, &p->outfit_weapon[eq_wgt.slot], &v );
      px += pw/2;
      py += ph/2;
      v.x *= pw / p->ship->gfx_space->sw;
      v.y *= ph / p->ship->gfx_space->sh;

      /* Render it. */
      glLineWidth( 3. );
      gl_renderCross(px + v.x, py + v.y, 7, &cRadar_player);
      glLineWidth( 1. );
   }
   toolkit_drawOutline( x - 4., y-4., w+7., h+2., 1., toolkit_colLight, NULL  );
   toolkit_drawOutline( x - 4., y-4., w+7., h+2., 2., toolkit_colDark, NULL  );
}
/**
 * @brief Handles a mouse press in column.
 *
 *    @param y Y position of the column.
 *    @param h Height of column.
 *    @param n Number of elements in column.
 *    @param my Mouse press position.
 *    @return Number pressed (or -1 if none).
 */
static int equipment_mouseInColumn( double y, double h, int n, double my )
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
 * @brief Handles a mouse press in a column.
 *
 *    @param wid Parent window id.
 *    @param event Mouse input event.
 *    @param mx Mouse X event position.
 *    @param my Mouse Y event position.
 *    @param y Y position of the column.
 *    @param h Height of the column.
 *    @param n Number of elements in the column.
 *    @param os Pointer to elements in the column.
 *    @param p Pilot to which the elements belong.
 *    @param selected Currently selected element.
 *    @param wgt Slot widget.
 */
static int equipment_mouseColumn( unsigned int wid, SDL_Event* event,
      double mx, double my, double y, double h, int n, PilotOutfitSlot* os,
      Pilot *p, int selected, CstSlotWidget *wgt )
{
   int ret, exists, level;

   ret = equipment_mouseInColumn( y, h, n, my );
   if (ret < 0)
      return 0;

   if (event->type == SDL_MOUSEBUTTONDOWN) {
      /* Normal mouse usage. */
      if (wgt->weapons < 0) {
         if (event->button.button == SDL_BUTTON_LEFT)
            wgt->slot = selected + ret;
         else if ((event->button.button == SDL_BUTTON_RIGHT) &&
               wgt->canmodify)
            equipment_swapSlot( wid, p, &os[ret] );
      }
      /* Viewing weapon slots. */
      else {
         /* See if it exists. */
         exists = pilot_weapSetCheck( p, wgt->weapons, &os[ret] );
         /* Get the level of the selection. */
         if (event->button.button == SDL_BUTTON_LEFT)
            level = 0;
         else if (event->button.button == SDL_BUTTON_RIGHT)
            level = 1;
         else
            return 0; /* We ignore this type of click. */
         /* See if we should add it or remove it. */
         if (exists==level)
            pilot_weapSetRm( p, wgt->weapons, &os[ret] );
         else {
            /* This is a bloody awful place to do this. I hate it. HATE!. */
            /* Case active outfit, convert the weapon group to active outfit. */
            if ((os->sslot->slot.type == OUTFIT_SLOT_STRUCTURE) ||
                  (os->sslot->slot.type == OUTFIT_SLOT_UTILITY)) {
               pilot_weapSetRmSlot( p, wgt->weapons, OUTFIT_SLOT_WEAPON );
               pilot_weapSetAdd( p, wgt->weapons, &os[ret], 0 );
               pilot_weapSetType( p, wgt->weapons, WEAPSET_TYPE_ACTIVE );
            }
            /* Case change weapon groups or active weapon. */
            else {
               pilot_weapSetRmSlot( p, wgt->weapons, OUTFIT_SLOT_STRUCTURE );
               pilot_weapSetRmSlot( p, wgt->weapons, OUTFIT_SLOT_UTILITY );
               if (pilot_weapSetTypeCheck( p, wgt->weapons) == WEAPSET_TYPE_CHANGE)
                  pilot_weapSetType( p, wgt->weapons, WEAPSET_TYPE_CHANGE );
               else {
                  pilot_weapSetType( p, wgt->weapons, WEAPSET_TYPE_WEAPON );
                  level = 0;
               }
               pilot_weapSetAdd( p, wgt->weapons, &os[ret], level );
            }
         }
         p->autoweap = 0; /* Disable autoweap. */
         info_update(); /* Need to update weapons. */
      }
   }
   else {
      wgt->mouseover  = selected + ret;
      wgt->altx       = mx;
      wgt->alty       = my;
   }

   return 1;
}
/**
 * @brief Does mouse input for the custom equipment widget.
 *
 *    @param wid Parent window id.
 *    @param event Mouse input event.
 *    @param mx Mouse X event position.
 *    @param my Mouse Y event position.
 *    @param bw Base window width.
 *    @param bh Base window height.
 *    @param rx Relative X movement (only valid for motion).
 *    @param ry Relative Y movement (only valid for motion).
 *    @param data Custom widget data.
 */
static int equipment_mouseSlots( unsigned int wid, SDL_Event* event,
      double mx, double my, double bw, double bh,
      double rx, double ry, void *data )
{
   (void) bw;
   (void) rx;
   (void) ry;
   Pilot *p;
   int selected, ret;
   double x, y;
   double w, h;
   double tw;
   CstSlotWidget *wgt;
   int n, m;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   p   = wgt->selected;

   /* Must have selected ship. */
   if (p == NULL)
      return 0;

   /* Must be left click for now. */
   if ((event->type != SDL_MOUSEBUTTONDOWN) &&
         (event->type != SDL_MOUSEMOTION))
      return 0;

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;

   /* Render weapon outfits. */
   selected = 0;
   x  = (tw-w)/2;
   y  = bh - (h+20) + (h+20-h)/2 - 10;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( wid, event, mx, my, y, h,
            p->outfit_nweapon, p->outfit_weapon, p, selected, wgt );
      if (ret)
         return !!(event->type == SDL_MOUSEBUTTONDOWN);
   }
   selected += p->outfit_nweapon;
   x += tw;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( wid, event, mx, my, y, h,
            p->outfit_nutility, p->outfit_utility, p, selected, wgt );
      if (ret)
         return !!(event->type == SDL_MOUSEBUTTONDOWN);
   }
   selected += p->outfit_nutility;
   x += tw;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( wid, event, mx, my, y, h,
            p->outfit_nstructure, p->outfit_structure, p, selected, wgt );
      if (ret)
         return !!(event->type == SDL_MOUSEBUTTONDOWN);
   }

   /* Not over anything. */
   wgt->mouseover = -1;
   return 0;
}


/**
 * @brief Swaps an equipment slot.
 *
 *    @param wid Parent window id.
 *    @param p Pilot swapping slots.
 *    @param slot Slot to swap.
 */
static int equipment_swapSlot( unsigned int wid, Pilot *p, PilotOutfitSlot *slot )
{
   int ret;
   Outfit *o, *ammo;

   /* Remove outfit. */
   if (slot->outfit != NULL) {
      o = slot->outfit;

      /* Must be able to remove. */
      if (pilot_canEquip( eq_wgt.selected, slot, NULL ) != NULL)
         return 0;

      /* Remove ammo first. */
      ammo = outfit_ammo(o);
      if ( ammo != NULL )
         pilot_rmAmmo( eq_wgt.selected, slot, slot->u.ammo.quantity );

      /* Remove outfit. */
      ret = pilot_rmOutfit( eq_wgt.selected, slot );
      if (ret == 0)
         equipment_playerAddOutfit( o, 1 );
   }
   /* Add outfit. */
   else {
      /* Must have outfit. */
      o = eq_wgt.outfit;
      if (o==NULL)
         return 0;

      /* Must fit slot. */
      if (!outfit_fitsSlot( o, &slot->sslot->slot ))
         return 0;

      /* Must be able to add. */
      if (pilot_canEquip( eq_wgt.selected, slot, o ) != NULL)
         return 0;

      /* Add outfit to ship. */
      ret = equipment_playerRmOutfit( o, 1 );
      if (ret == 1) {
         pilot_addOutfitRaw( eq_wgt.selected, o, slot );

         /* Recalculate stats. */
         pilot_calcStats( eq_wgt.selected );
      }

      equipment_addAmmo();
   }

   /* Refuel if necessary. */
   land_refuel();

   /* Recalculate stats. */
   pilot_calcStats( p );
   pilot_healLanded( p );

   /* Redo the outfits thingy. */
   equipment_regenLists( wid, 1, 1 );

   /* Update outfits. */
   outfits_updateEquipmentOutfits();

   /* Update weapon sets if needed. */
   if (eq_wgt.selected->autoweap)
      pilot_weaponAuto( eq_wgt.selected );
   pilot_weaponSafe( eq_wgt.selected );

   /* Notify GUI of modification. */
   gui_setShip();

   return 0;
}


/**
 * @brief Regenerates the equipment window lists.
 *
 *    @param wid Window to regenerate lists.
 *    @param outfits Whether or not should regenerate outfits list.
 *    @param ships Whether or not to regenerate ships list.
 */
void equipment_regenLists( unsigned int wid, int outfits, int ships )
{
   int i, ret;
   int nship;
   double offship;
   char *s, selship[PATH_MAX];
   char *focused;

   /* Default.s */
   nship    = 0;
   offship  = 0.;

   /* Must exist. */
   if (!window_existsID( equipment_wid ))
      return;

   /* Save focus. */
   focused = window_getFocus( wid );

   /* Save positions. */
   if (outfits) {
      i = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
      toolkit_saveImageArrayData( wid, EQUIPMENT_OUTFITS, &iar_data[i] );
      window_destroyWidget( wid, EQUIPMENT_OUTFITS );
   }
   if (ships) {
      nship   = toolkit_getImageArrayPos(    wid, EQUIPMENT_SHIPS );
      offship = toolkit_getImageArrayOffset( wid, EQUIPMENT_SHIPS );
      s       = toolkit_getImageArray(       wid, EQUIPMENT_SHIPS );
      strncpy( selship, s, sizeof(selship)-1 );
      selship[PATH_MAX-1] = '\0'; /* Just in case. */
      window_destroyWidget( wid, EQUIPMENT_SHIPS );
   }

   /* Regenerate lists. */
   equipment_genLists( wid );

   /* Restore positions. */
   if (outfits) {
      toolkit_setImageArrayPos(    wid, EQUIPMENT_OUTFITS, iar_data[i].pos );
      toolkit_setImageArrayOffset( wid, EQUIPMENT_OUTFITS, iar_data[i].offset );
      equipment_updateOutfits( wid, NULL );
   }
   if (ships) {
      toolkit_setImageArrayPos(    wid, EQUIPMENT_SHIPS, nship );
      toolkit_setImageArrayOffset( wid, EQUIPMENT_SHIPS, offship );
      /* Try to maintain same ship selected. */
      s = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
      if ((s != NULL) && (strcmp(s,selship)!=0)) {
         ret = toolkit_setImageArray( wid, EQUIPMENT_SHIPS, selship );
         if (ret != 0) /* Failed to maintain. */
            toolkit_setImageArrayPos( wid, EQUIPMENT_SHIPS, nship );

         /* Update ships. */
         equipment_updateShips( wid, NULL );
      }
   }

   /* Restore focus. */
   window_setFocus( wid, focused );
   free(focused);
}


/**
 * @brief Adds all the ammo it can to the player.
 */
void equipment_addAmmo (void)
{
   Pilot *p;

   /* Get player. */
   if (eq_wgt.selected == NULL)
      p = player.p;
   else
      p = eq_wgt.selected;

   /* Add ammo to all outfits. */
   pilot_fillAmmo( p );

   /* Update weapon sets if needed. */
   if (p->autoweap)
      pilot_weaponAuto( p );
   pilot_weaponSafe( p );

   /* Notify GUI of modification. */
   gui_setShip();
}


/**
 * @brief Creates and allocates a string containing the ship stats.
 *
 *    @param buf Buffer to write to.
 *    @param max_len Maximum length of the string to allocate.
 *    @param s Pilot to get stats of.
 *    @param dpseps Whether or not to display dps and eps.
 */
int equipment_shipStats( char *buf, int max_len,  const Pilot *s, int dpseps )
{
   int j, l;
   Outfit *o;
   double mod_energy, mod_damage, mod_shots;
   double eps, dps, shots;
   const Damage *dmg;

   dps = 0.;
   eps = 0.;
   /* Calculate damage and energy per second. */
   if (dpseps) {
      for (j=0; j<s->noutfits; j++) {
         o = s->outfits[j]->outfit;
         if (o==NULL)
            continue;
         switch (o->type) {
            case OUTFIT_TYPE_BOLT:
               mod_energy = s->stats.fwd_energy;
               mod_damage = s->stats.fwd_damage;
               mod_shots  = 2. - s->stats.fwd_firerate;
               break;
            case OUTFIT_TYPE_TURRET_BOLT:
               mod_energy = s->stats.tur_energy;
               mod_damage = s->stats.tur_damage;
               mod_shots  = 2. - s->stats.tur_firerate;
               break;
            case OUTFIT_TYPE_LAUNCHER:
            case OUTFIT_TYPE_TURRET_LAUNCHER:
               mod_energy = 1.;
               mod_damage = s->stats.launch_damage;
               mod_shots  = 2. - s->stats.launch_rate;
               break;
            case OUTFIT_TYPE_BEAM:
            case OUTFIT_TYPE_TURRET_BEAM:
               /* Special case due to continuous fire. */
               dps += outfit_damage(o)->damage;
               eps += outfit_energy(o);

               continue;
            default:
               continue;
         }
         shots = 1. / (mod_shots * outfit_delay(o));

         /* Special case: Ammo-based weapons. */
         if (outfit_isLauncher(o))
            dmg = outfit_damage(o->u.lau.ammo);
         else
            dmg = outfit_damage(o);
         dps  += shots * mod_damage * dmg->damage;
         eps  += shots * mod_energy * MAX( outfit_energy(o), 0. );
      }
   }

   /* Write to buffer. */
   l = 0;
   if (dps > 0.)
      l += nsnprintf( &buf[l], (max_len-l),
            _("%s%.2f DPS [%.2f EPS]"), (l!=0)?"\n":"", dps, eps );
   l += ss_statsDesc( &s->stats, &buf[l], (max_len-l), l );
   return l;
}


/**
 * @brief Generates a new ship/outfit lists if needed.
 *
 *    @param wid Parent window id.
 */
static void equipment_genLists( unsigned int wid )
{
   /* Ship list. */
   equipment_genShipList( wid );

   /* Outfit list. */
   equipment_genOutfitList( wid );

   /* Update window. */
   equipment_updateOutfits(wid, NULL);
   equipment_updateShips(wid, NULL);
}


/**
 * @brief Generates the ship list.
 *    @param wid Window to generate list on.
 */
static void equipment_genShipList( unsigned int wid )
{
   int i, l, n;
   ImageArrayCell *cships;
   int nships;
   int w, h;
   int sw, sh;
   Pilot *s;
   const PlayerShip_t *ps;
   char r[PATH_MAX];
   glTexture *t;

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, &sw, &sh, NULL, NULL,
         NULL, NULL, NULL, NULL, NULL, NULL );

   /* Ship list. */
   if (!widget_exists( wid, EQUIPMENT_SHIPS )) {
      eq_wgt.selected = NULL;
      if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         nships   = player_nships()+1;
      else
         nships   = 1;
      cships   = calloc( nships, sizeof(ImageArrayCell) );
      /* Add player's current ship. */
      cships[0].image = gl_dupTexture(player.p->ship->gfx_store);
      cships[0].caption = strdup(player.p->name);
      cships[0].layers = gl_copyTexArray( player.p->ship->gfx_overlays, player.p->ship->gfx_noverlays, &cships[0].nlayers );
      t = gl_newImage( OVERLAY_GFX_PATH"active.png", OPENGL_TEX_MIPMAPS );
      cships[0].layers = gl_addTexArray( cships[0].layers, &cships[0].nlayers, t );
      if (player.p->ship->rarity > 0) {
         nsnprintf( r, sizeof(r), OVERLAY_GFX_PATH"rarity_%d.png", player.p->ship->rarity );
         t = gl_newImage( r, OPENGL_TEX_MIPMAPS );
         cships[0].layers = gl_addTexArray( cships[0].layers, &cships[0].nlayers, t );
      }
      if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD)) {
         player_shipsSort();
         ps = player_getShipStack( &n );
         for (i=1; i<n+1; i++) {
            cships[i].image = gl_dupTexture( ps[i-1].p->ship->gfx_store );
            cships[i].caption = strdup( ps[i-1].p->name );
            cships[i].layers = gl_copyTexArray( ps[i-1].p->ship->gfx_overlays, ps[i-1].p->ship->gfx_noverlays, &cships[i].nlayers );
            if (ps[i-1].p->ship->rarity > 0) {
               nsnprintf( r, sizeof(r), OVERLAY_GFX_PATH"rarity_%d.png", ps[i-1].p->ship->rarity );
               t = gl_newImage( r, OPENGL_TEX_MIPMAPS );
               cships[i].layers = gl_addTexArray( cships[i].layers, &cships[i].nlayers, t );
            }
         }
      }
      /* Ship stats in alt text. */
      for (i=0; i<nships; i++) {
         s        = player_getShip( cships[i].caption );
         cships[i].alt = malloc( SHIP_ALT_MAX );
         l        = nsnprintf( &cships[i].alt[0], SHIP_ALT_MAX, _("Ship Stats\n") );
         l        = equipment_shipStats( &cships[i].alt[0], SHIP_ALT_MAX-l, s, 1 );
         if (l == 0) {
            free( cships[i].alt );
            cships[i].alt = NULL;
         }
      }

      window_addImageArray( wid, 20, -40,
            sw, sh, EQUIPMENT_SHIPS, 96., 96.,
            cships, nships, equipment_updateShips, NULL, equipment_transChangeShip );
   }
}


/**
 * @brief Generates the outfit list.
 *    @param wid Window to generate list on.
 */
static void equipment_genOutfitList( unsigned int wid )
{
   int x, y, w, h, ow, oh;
   int ix, iy, iw, ih, barw; /* Input filter. */
   char *filtertext;
   int (*tabfilters[])( const Outfit *o ) = {
      NULL,
      outfit_filterWeapon,
      outfit_filterUtility,
      outfit_filterStructure,
      outfit_filterCore
   };
   const char *tabnames[] = {
      _("All"), _(OUTFIT_LABEL_WEAPON), _(OUTFIT_LABEL_UTILITY), _(OUTFIT_LABEL_STRUCTURE), _(OUTFIT_LABEL_CORE)
   };

   int noutfits, active;
   ImageArrayCell *coutfits;

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, NULL, NULL, &ow, &oh,
         NULL, NULL, NULL, NULL, NULL, NULL );

   /* Deselect. */
   eq_wgt.outfit = NULL;

   /* Calculate position. */
   x = 20;
   y = 20;

   /* Create tabbed window. */
   if (!widget_exists( wid, EQUIPMENT_OUTFIT_TAB )) {
      window_addTabbedWindow( wid, x, y + oh - 30, ow, 30,
            EQUIPMENT_OUTFIT_TAB, OUTFIT_TABS, tabnames, 1 );

      barw = window_tabWinGetBarWidth( wid, EQUIPMENT_OUTFIT_TAB );

      iw = CLAMP(0, 150, ow - barw - 30);
      ih = 20;

      ix = ow - iw;
      iy = oh - (30 - ih) / 2; /* Centered relative to 30 px tab bar */

      /* Only create the filter widget if it will be a reasonable size. */
      if (iw >= 30) {
         window_addInput( wid, ix+15, iy+1, iw, ih, EQUIPMENT_FILTER, 32, 1, &gl_smallFont );
         window_setInputCallback( wid, EQUIPMENT_FILTER, equipment_filterOutfits );
      }
   }

   window_tabWinOnChange( wid, EQUIPMENT_OUTFIT_TAB, equipment_changeTab );
   active = window_tabWinGetActive( equipment_wid, EQUIPMENT_OUTFIT_TAB );

   /* Widget must not already exist. */
   if (widget_exists( wid, EQUIPMENT_OUTFITS ))
      return;

   /* Allocate space. */
   noutfits = MAX( 1, player_numOutfits() ); /* This is the most we'll need, probably less due to filtering. */
   free( iar_outfits[active] );
   iar_outfits[active] = calloc( noutfits, sizeof(Outfit*) );

   filtertext = NULL;
   if (widget_exists(equipment_wid, EQUIPMENT_FILTER)) {
      filtertext = window_getInput( equipment_wid, EQUIPMENT_FILTER );
      if (strlen(filtertext) == 0)
         filtertext = NULL;
   }

   /* Get the outfits. */
   noutfits = player_getOutfitsFiltered( iar_outfits[active], tabfilters[active], filtertext );
   coutfits = outfits_imageArrayCells( iar_outfits[active], &noutfits );

   /* Create the actual image array. */
   window_addImageArray( wid, x + 4, y + 3, ow - 6, oh - 37,
         EQUIPMENT_OUTFITS, 96., 96.,
         coutfits, noutfits,
         equipment_updateOutfits,
         equipment_rightClickOutfits,
         equipment_rightClickOutfits );
}


/**
 * @brief Gets the colour for comparing a current value vs a ship base value.
 */
static char eq_qCol( double cur, double base, int inv )
{
   if (cur > 1.2*base)
      return (inv) ? 'r' : 'g';
   else if (cur < 0.8*base)
      return (inv) ? 'g' : 'r';
   return '0';
}


/**
 * @brief Gets the symbol for comparing a current value vs a ship base value.
 */
static const char* eq_qSym( double cur, double base, int inv )
{
   if (cur > 1.2*base)
      return (inv) ? "!! " : "";
   else if (cur < 0.8*base)
      return (inv) ? "" : "!! ";
   return "";
}


#define EQ_COMP( cur, base, inv ) \
eq_qCol( cur, base, inv ), eq_qSym( cur, base, inv ), cur
/**
 * @brief Updates the player's ship window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateShips( unsigned int wid, char* str )
{
   (void)str;
   char buf[STRMAX], buf2[ECON_CRED_STRLEN];
   char errorReport[STRMAX_SHORT];
   char *shipname;
   Pilot *ship;
   char *nt;
   int onboard;
   int cargo;

   /* Clear defaults. */
   eq_wgt.slot          = -1;
   eq_wgt.mouseover     = -1;
   equipment_lastick    = SDL_GetTicks();

   /* Get the ship. */
   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   if (strcmp(shipname,player.p->name)==0) { /* no ships */
      ship    = player.p;
      onboard = 1;
   }
   else {
      ship   = player_getShip( shipname );
      onboard = 0;
   }
   eq_wgt.selected = ship;

   /* update text */
   credits2str( buf2, player_shipPrice(shipname), 2 ); /* sell price */
   cargo = pilot_cargoFree(ship) + pilot_cargoUsed(ship);
   nt = ntime_pretty( pilot_hyperspaceDelay( ship ), 2 );

   /* Get ship error report. */
   pilot_reportSpaceworthy( ship, errorReport, sizeof(errorReport));

   /* Fill the buffer. */
   nsnprintf( buf, sizeof(buf),
         _("%s\n"
         "%s\n"
         "%s\n"
         "#%c%s%.0f#0\n"
         "%s\n"
         "\n"
         "%.0f#0 tonnes\n"
         "%s average\n"
         "#%c%s%.0f#0 kN/tonne\n"
         "#%c%s%.0f#0 m/s (max #%c%s%.0f#0 m/s)\n"
         "#%c%s%.0f#0 deg/s\n"
         "%.0f%%\n"
         "\n"
         "#%c%s%.0f%%\n"
         "#%c%s%.0f#0 MJ (#%c%s%.1f#0 MW)\n"
         "#%c%s%.0f#0 MJ (#%c%s%.1f#0 MW)\n"
         "#%c%s%.0f#0 MJ (#%c%s%.1f#0 MW)\n"
         "%d / #%c%s%d#0 tonnes\n"
         "%d units (%d jumps)\n"
         "\n"
         "#%c%s#0"),
         /* Generic. */
      ship->name,
      _(ship->ship->name),
      _(ship_class(ship->ship)),
      EQ_COMP( ship->crew, ship->ship->crew, 0 ),
      buf2,
      /* Movement. */
      ship->solid->mass,
      nt,
      EQ_COMP( ship->thrust/ship->solid->mass, ship->ship->thrust/ship->ship->mass, 0 ),
      EQ_COMP( ship->speed, ship->ship->speed, 0 ),
      EQ_COMP( solid_maxspeed( ship->solid, ship->speed, ship->thrust ),
            solid_maxspeed( ship->solid, ship->ship->speed, ship->ship->thrust), 0 ),
      EQ_COMP( ship->turn*180./M_PI, ship->ship->turn*180./M_PI, 0 ),
      ship->ship->dt_default * 100,
      /* Health. */
      EQ_COMP( ship->dmg_absorb * 100, ship->ship->dmg_absorb * 100, 0 ),
      EQ_COMP( ship->shield_max, ship->ship->shield, 0 ),
      EQ_COMP( ship->shield_regen, ship->ship->shield_regen, 0 ),
      EQ_COMP( ship->armour_max, ship->ship->armour, 0 ),
      EQ_COMP( ship->armour_regen, ship->ship->armour_regen, 0 ),
      EQ_COMP( ship->energy_max, ship->ship->energy, 0 ),
      EQ_COMP( ship->energy_regen, ship->ship->energy_regen, 0 ),
      /* Misc. */
      pilot_cargoUsed(ship), EQ_COMP( cargo, ship->ship->cap_cargo, 0 ),
      ship->fuel_max, ship->fuel_max / ship->fuel_consumption,
      pilot_checkSpaceworthy(ship) ? 'r' : '0', errorReport );
   window_modifyText( wid, "txtDDesc", buf );

   /* Clean up. */
   free( nt );

   /* button disabling */
   if (onboard) {
      window_disableButton( wid, "btnSellShip" );
      window_disableButton( wid, "btnChangeShip" );
   }
   else {\
      window_enableButton( wid, "btnChangeShip" );
      window_enableButton( wid, "btnSellShip" );
   }
}
#undef EQ_COMP
/**
 * @brief Updates the player's outfit list.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateOutfits( unsigned int wid, char* str )
{
   (void) wid;
   (void) str;
   int i, active;

   /* Must have outfit. */
   active = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
   i = toolkit_getImageArrayPos( wid, EQUIPMENT_OUTFITS );
   if (i < 0 || iar_outfits[active] == NULL) {
      eq_wgt.outfit = NULL;
      return;
   }

   eq_wgt.outfit = iar_outfits[active][i];
}

/**
 * @brief Handles text input in the filter input widget.
 *    @param wid Window containing the widget.
 *    @param str Unused.
 */
static void equipment_filterOutfits( unsigned int wid, char *str )
{
   (void) str;
   equipment_regenLists(wid, 1, 0);
}

/**
 * @brief Ensures the tab's selected item is reflected in the ship slot list
 *
 *    @param wid Unused.
 *    @param wgt Unused.
 *    @param old Tab changed from.
 *    @param tab Tab changed to.
 */
static void equipment_changeTab( unsigned int wid, char *wgt, int old, int tab )
{
   (void) wid;
   (void) wgt;
   int pos;
   double offset;

   toolkit_saveImageArrayData( wid, EQUIPMENT_OUTFITS, &iar_data[old] );

   /* Store the currently-saved positions for the new tab. */
   pos    = iar_data[tab].pos;
   offset = iar_data[tab].offset;

   /* Resetting the input will cause the outfit list to be regenerated. */
   if (widget_exists(wid, EQUIPMENT_FILTER))
      window_setInput(wid, EQUIPMENT_FILTER, NULL);
   else
      equipment_regenLists(wid, 1, 0);

   /* Set positions for the new tab. This is necessary because the stored
    * position for the new tab may have exceeded the size of the old tab,
    * resulting in it being clipped. */
   toolkit_setImageArrayPos(    wid, EQUIPMENT_OUTFITS, pos );
   toolkit_setImageArrayOffset( wid, EQUIPMENT_OUTFITS, offset );

   /* Focus the outfit image array. */
   window_setFocus( wid, EQUIPMENT_OUTFITS );
}

/**
 * @brief Changes ship.
 *    @param wid Window player is attempting to change ships in.
 *    @param str Unused.
 */
static void equipment_transChangeShip( unsigned int wid, char* str )
{
   (void) str;

   equipment_changeShip( wid );

   /* update the window to reflect the change */
   equipment_updateShips( wid, NULL );
}
/**
 * @brief Player attempts to change ship.
 *    @param wid Window player is attempting to change ships in.
 */
static void equipment_changeShip( unsigned int wid )
{
   char *shipname, *filtertext;
   int i;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );

   if (land_errDialogue( shipname, "swapEquipment" ))
      return;

   /* Store active tab, filter text, and positions for the outfits. */
   i = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
   toolkit_saveImageArrayData( wid, EQUIPMENT_OUTFITS, &iar_data[i] );
   if (widget_exists(wid, EQUIPMENT_FILTER))
      filtertext = window_getInput( equipment_wid, EQUIPMENT_FILTER );
   else
      filtertext = NULL;

   /* Swap ship. */
   player_swapShip( shipname );
   pilot_healLanded( player.p );

   /* What happens here is the gui gets recreated when the player swaps ship.
    * This causes all the windows to be destroyed and the 'wid' we have here
    * becomes invalid. However, since we store it in a global variable we can
    * recover it and use it instead. */
   wid = equipment_wid;

   /* Restore outfits image array properties. */
   window_tabWinSetActive( wid, EQUIPMENT_OUTFIT_TAB, i );
   toolkit_setImageArrayPos(    wid, EQUIPMENT_OUTFITS, iar_data[i].pos );
   toolkit_setImageArrayOffset( wid, EQUIPMENT_OUTFITS, iar_data[i].offset );
   if (widget_exists(wid, EQUIPMENT_FILTER))
      window_setInput(wid, EQUIPMENT_FILTER, filtertext);

   /* Regenerate ship widget. */
   equipment_regenLists( wid, 0, 1 );

   /* Focus new ship. */
   toolkit_setImageArrayPos(    wid, EQUIPMENT_SHIPS, 0 );
   toolkit_setImageArrayOffset( wid, EQUIPMENT_SHIPS, 0. );
}



/**
 * @brief Unequips the player's ship.
 *
 *    @param wid Window id.
 *    @param str of widget.
 */
static void equipment_unequipShip( unsigned int wid, char* str )
{
   (void) str;
   int ret;
   int i;
   Pilot *ship;
   Outfit *o, *ammo;

   ship = eq_wgt.selected;

   /*
    * Unequipping is disallowed under two conditions. Firstly, the ship may not
    * be unequipped when it has fighters deployed in space. Secondly, it cannot
    * unequip if it's carrying more cargo than the ship normally fits, i.e.
    * by equipping cargo pods.
    */
   for (i=0; i<ship->noutfits; i++) {
      /* Must have outfit. */
      if (ship->outfits[i]->outfit == NULL)
         continue;
      /* Must be fighter bay. */
      if (!outfit_isFighterBay( ship->outfits[i]->outfit))
         continue;
      /* Must not have deployed fighters. */
      if (ship->outfits[i]->u.ammo.deployed > 0) {
         dialogue_alert( _("You can not unequip your ship while you have deployed fighters!") );
         return;
      }
   }
   if (pilot_cargoUsed( ship ) > ship->ship->cap_cargo) {
      dialogue_alert( _("You can not unequip your ship when you have more cargo than it can hold without modifications!") );
      return;
   }

   if (dialogue_YesNo(_("Unequip Ship"), /* confirm */
         _("Are you sure you want to remove all equipment from your ship?"))==0)
      return;

   /* Remove all outfits. */
   for (i=0; i<ship->noutfits; i++) {
      o = ship->outfits[i]->outfit;

      /* Skip null outfits. */
      if (o==NULL)
         continue;

      /* Remove ammo first. */
      ammo = outfit_ammo(o);
      if (ammo != NULL) {
         pilot_rmAmmo( ship, ship->outfits[i], outfit_amount(o) );
      }

      /* Remove rest. */
      ret = pilot_rmOutfitRaw( ship, ship->outfits[i] );
      if (ret==0)
         equipment_playerAddOutfit( o, 1 );
   }

   /* Recalculate stats. */
   pilot_calcStats( ship );
   pilot_healLanded( ship );

   /* Regenerate list. */
   equipment_regenLists( wid, 1, 1 );

   /* Regenerate outfits. */
   outfits_updateEquipmentOutfits();

   /* Update weapon sets if needed. */
   if (ship->autoweap)
      pilot_weaponAuto( ship );
   pilot_weaponSafe( ship );

   /* Notify GUI of modification. */
   gui_setShip();
}
/**
 * @brief Player tries to sell a ship.
 *    @param wid Window player is selling ships in.
 *    @param str Unused.
 */
static void equipment_sellShip( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[ECON_CRED_STRLEN], *name;
   credits_t price;
   Pilot *p;
   Ship *s;
   HookParam hparam[3];

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );

   if (land_errDialogue( shipname, "sell" ))
      return;

   /* Calculate price. */
   price = player_shipPrice(shipname);
   credits2str( buf, price, 2 );

   /* Check if player really wants to sell. */
   if (!dialogue_YesNo( _("Sell Ship"),
            _("Are you sure you want to sell your ship %s for %s?"),
            shipname, buf))
      return;

   /* Store ship type. */
   p = player_getShip( shipname );
   s = p->ship;

   /* Sold. */
   name = strdup(shipname);
   player_modCredits( price );
   player_rmShip( shipname );

   /* Destroy widget - must be before widget. */
   equipment_regenLists( wid, 0, 1 );

   /* Display widget. */
   dialogue_msg( _("Ship Sold"),
         _("You have sold your ship %s for %s."), name, buf );

   /* Run hook. */
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = s->name;
   hparam[1].type    = HOOK_PARAM_STRING;
   hparam[1].u.str   = name;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "ship_sell", hparam );
   if (land_takeoff)
      takeoff(1);
   free(name);
}


/**
 * @brief Renames the selected ship.
 *
 *    @param wid Parent window id.
 *    @param str Unused.
 */
static void equipment_renameShip( unsigned int wid, char *str )
{
   (void)str;
   Pilot *ship;
   char *shipname, *newname;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   ship = player_getShip(shipname);
   newname = dialogue_input( _("Ship Name"), 1, 60,
         _("Please enter a new name for your %s:"), _(ship->ship->name) );

   /* Player cancelled the dialogue. */
   if (newname == NULL)
      return;

   /* Must not have same name. */
   if (player_hasShip(newname)) {
      dialogue_msg( _("Name Collision"),
            _("Please do not give the ship the same name as another of your ships."));
      free(newname);
      return;
   }

   free (ship->name);
   ship->name = newname;

   /* Destroy widget - must be before widget. */
   equipment_regenLists( wid, 0, 1 );
}


/**
 * @brief Wrapper to only add unique outfits.
 */
static int equipment_playerAddOutfit( const Outfit *o, int quantity )
{
   if (outfit_isProp(o,OUTFIT_PROP_UNIQUE) && (player_outfitOwned(o)>0))
      return 1;
   return player_addOutfit(o,quantity);
}


/**
 * @brief Wrapper to only remove unique outfits.
 */
static int equipment_playerRmOutfit( const Outfit *o, int quantity )
{
   if (outfit_isProp(o,OUTFIT_PROP_UNIQUE))
      return 1;
   return player_rmOutfit(o,quantity);
}


/**
 * @brief Cleans up after the equipment stuff.
 */
void equipment_cleanup (void)
{
   /* Free stored positions. */
   free(iar_data);
   iar_data = NULL;
   if (iar_outfits != NULL) {
      for (int i=0; i<OUTFIT_TABS; i++)
         free( iar_outfits[i] );
      free(iar_outfits);
      iar_outfits = NULL;
   }

   /* Free icons. */
   gl_freeTexture(equip_ico_yes);
   equip_ico_yes = NULL;
   gl_freeTexture(equip_ico_no);
   equip_ico_no = NULL;
}

