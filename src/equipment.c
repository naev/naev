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

#include "array.h"
#include "conf.h"
#include "debug.h"
#include "dialogue.h"
#include "escort.h"
#include "gui.h"
#include "hook.h"
#include "info.h"
#include "land.h"
#include "land_outfits.h"
#include "log.h"
#include "map.h"
#include "mission.h"
#include "ndata.h"
#include "nstring.h"
#include "nlua.h"
#include "nlua_tk.h"
#include "ntime.h"
#include "player.h"
#include "player_fleet.h"
#include "pilot_outfit.h"
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

/*
 * equipment stuff
 */
static CstSlotWidget eq_wgt; /**< Equipment widget. */
static double equipment_dir      = 0.; /**< Equipment dir. */
static unsigned int equipment_lastick = 0; /**< Last tick. */
static unsigned int equipment_wid   = 0; /**< Global wid. */
static int equipment_creating    = 0; /**< Whether or not creating. */
static int ship_mode = 0; /**< Ship mode. */
static iar_data_t iar_data[OUTFIT_TABS]; /**< Stored image array positions. */
static Outfit **iar_outfits[OUTFIT_TABS]; /**< Outfits associated with the image array cells. */
static nlua_env autoequip_env = LUA_NOREF; /* Autoequip env. */
static int equipment_outfitMode = 0; /**< Outfit mode for filtering. */

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
static void equipment_toggleFav( unsigned int wid, const char *wgt );
static void equipment_toggleDeploy( unsigned int wid, const char *wgt );
static void equipment_renderColumn( double x, double y, double w, double h,
      const PilotOutfitSlot *lst, const char *txt,
      int selected, Outfit *o, Pilot *p, const CstSlotWidget *wgt );
static void equipment_renderSlots( double bx, double by, double bw, double bh, void *data );
static void equipment_renderMisc( double bx, double by, double bw, double bh, void *data );
static void equipment_renderOverlayColumn( double x, double y, double h,
      PilotOutfitSlot *lst, int mover, CstSlotWidget *wgt );
static void equipment_renderOverlaySlots( double bx, double by, double bw, double bh,
      void *data );
static void equipment_renderShip( double bx, double by, double bw, double bh, void *data );
static int equipment_mouseInColumn( const PilotOutfitSlot *lst, double y, double h, double my );
static int equipment_mouseSlots( unsigned int wid, const SDL_Event* event,
      double x, double y, double w, double h, double rx, double ry, void *data );
/* Misc. */
static char eq_qCol( double cur, double base, int inv );
static int equipment_swapSlot( unsigned int wid, Pilot *p, PilotOutfitSlot *slot );
static void equipment_sellShip( unsigned int wid, const char* str );
static void equipment_renameShip( unsigned int wid, const char *str );
static void equipment_shipMode( unsigned int wid, const char *str );
static void equipment_rightClickShips( unsigned int wid, const char* str );
static void equipment_transChangeShip( unsigned int wid, const char* str );
static void equipment_changeShip( unsigned int wid );
static void equipment_unequipShip( unsigned int wid, const char* str );
static void equipment_autoequipShip( unsigned int wid, const char* str );
static void equipment_filterOutfits( unsigned int wid, const char *str );
static void equipment_rightClickOutfits( unsigned int wid, const char* str );
static void equipment_outfitPopdown( unsigned int wid, const char* str );
static void equipment_changeTab( unsigned int wid, const char *wgt, int old, int tab );
static int equipment_playerAddOutfit( const Outfit *o, int quantity );
static int equipment_playerRmOutfit( const Outfit *o, int quantity );

/**
 * @brief Handles right-click on unequipped outfit.
 *    @param wid Window to update.
 *    @param str Widget name. Must be EQUIPMENT_OUTFITS.
 */
void equipment_rightClickOutfits( unsigned int wid, const char* str )
{
   (void) str;
   Outfit *o;
   int id, active, minimal, n, nfits;
   PilotOutfitSlot* slots;
   Pilot *p;
   OutfitSlotSize size;

   active = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
   id = toolkit_getImageArrayPos( wid, EQUIPMENT_OUTFITS );

   /* Did the user click on background or placeholder cell? */
   if (id < 0 || array_size(iar_outfits[active])==0)
      return;

   o = iar_outfits[active][id];
   if (o==NULL)
      return;

   /* Figure out which slot this stuff fits into */
   switch (o->slot.type) {
      case OUTFIT_SLOT_STRUCTURE:
         slots    = eq_wgt.selected->p->outfit_structure;
         break;
      case OUTFIT_SLOT_UTILITY:
         slots    = eq_wgt.selected->p->outfit_utility;
         break;
      case OUTFIT_SLOT_WEAPON:
         slots    = eq_wgt.selected->p->outfit_weapon;
         break;
      default:
         return;
   }
   n = array_size(slots);

   /* See how many slots it fits into. */
   nfits = 0;
   minimal = n;
   for (int i=0; i<n; i++) {
      /* Must fit the slot. */
      if (!outfit_fitsSlot( o, &slots[i].sslot->slot))
         continue;

      /* Must have valid slot size. */
      if (o->slot.size == OUTFIT_SLOT_SIZE_NA)
         continue;

      minimal = i;
      nfits++;
   }
   /* Only fits in a single slot, so we might as well just swap it. */
   if (nfits==1) {
      eq_wgt.outfit  = o;
      p              = eq_wgt.selected->p;
      /* We have to call once to remove, once to add. */
      if (slots[minimal].outfit != NULL)
         equipment_swapSlot( equipment_wid, p, &slots[minimal] );
      eq_wgt.outfit  = o;
      equipment_swapSlot( equipment_wid, p, &slots[minimal] );

      hooks_run( "equip" ); /* Equipped. */
      return;
   }

   /* See if limit is applied and swap with shared limit slot. */
   if (o->limit != NULL) {
      minimal = n;
      for (int i=0; i<n; i++) {
         /* Must fit the slot. */
         if (!outfit_fitsSlot( o, &slots[i].sslot->slot))
            continue;

         /* Must have valid slot size. */
         if (o->slot.size == OUTFIT_SLOT_SIZE_NA)
            continue;

         /* Must have outfit with limit. */
         if ((slots[i].outfit == NULL) || (slots[i].outfit->limit == NULL))
            continue;

         /* Must share a limit to be able to swap. */
         if (strcmp(slots[i].outfit->limit,o->limit)!=0)
            continue;

         minimal = i;
      }
      if (minimal < n) {
         eq_wgt.outfit  = o;
         p              = eq_wgt.selected->p;
         /* Once to unequip and once to equip. */
         equipment_swapSlot( equipment_wid, p, &slots[minimal] );
         eq_wgt.outfit  = o;
         equipment_swapSlot( equipment_wid, p, &slots[minimal] );
         hooks_run( "equip" ); /* Equipped. */
         return;
      }
   }

   /* Loop through outfit slots of the right type, try to find an empty one */
   size = OUTFIT_SLOT_SIZE_NA;
   minimal = n;
   for (int i=0; i<n; i++) {
      /* Slot full. */
      if (slots[i].outfit != NULL)
         continue;

      /* Must fit the slot. */
      if (!outfit_fitsSlot( o, &slots[i].sslot->slot))
         continue;

      /* Must have valid slot size. */
      if (o->slot.size == OUTFIT_SLOT_SIZE_NA)
         continue;

      /* Search for the smallest slot avaliable. */
      if ((size == OUTFIT_SLOT_SIZE_NA) || (slots[i].sslot->slot.size < size)){
         size = slots[i].sslot->slot.size;
         minimal = i;
      }
   }

   /* Use the chosen one (if any). */
   if (minimal < n) {
      eq_wgt.outfit  = o;
      p              = eq_wgt.selected->p;
      equipment_swapSlot( equipment_wid, p, &slots[minimal] );
      hooks_run( "equip" ); /* Equipped. */
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
      *ew = 180;
   if (eh != NULL)
      *eh = *h - 100;

   /* Calculate custom widget. */
   if (cw != NULL)
      *cw = 120;
   if (ch != NULL)
      *ch = 140 + ((player.fleet_capacity > 0) ? 40 : 0);

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
   int w,h, sw,sh, ow,oh, bw,bh, ew,eh, cw,ch, x,y;
   unsigned int *widptr;

   /* Load the outfit mode. */
   equipment_outfitMode = player.eq_outfitMode;

   /* Mark as generated. */
   land_tabGenerate(LAND_WINDOW_EQUIPMENT);

   /* Set global WID. */
   equipment_wid = wid;
   equipment_creating = 1;

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, &sw, &sh, &ow, &oh,
         &ew, &eh, &cw, &ch, &bw, &bh );

   /* Buttons */
   x = -20+4;
   y = 20;
   window_addButtonKey( wid, x, y,
         bw, bh, "btnCloseEquipment",
         _("Take Off"), land_buttonTakeoff, SDLK_t );
   x -= (15+bw);
   window_addButtonKey( wid, x, y,
         bw, bh, "btnSellShip",
         _("Sell Ship"), equipment_sellShip, SDLK_s );
   x -= (15+bw);
   window_addButtonKey( wid, x, y,
         bw, bh, "btnChangeShip",
         _("Swap Ship"), equipment_transChangeShip, SDLK_p );
   x -= (15+bw);
   window_addButtonKey( wid, x, y,
         bw, bh, "btnUnequipShip",
         _("Unequip"), equipment_unequipShip, SDLK_u );
   x -= (15+bw);
   window_addButtonKey( wid, x, y,
         bw, bh, "btnAutoequipShip",
         _("Autoequip"), equipment_autoequipShip, SDLK_a );

   /* Prepare the outfit array. */
   for (int i=0; i<OUTFIT_TABS; i++) {
      toolkit_initImageArrayData( &iar_data[i] );
      array_free( iar_outfits[i] );
   }
   memset( iar_outfits, 0, sizeof(Outfit**) * OUTFIT_TABS );

   /* Safe defaults. */
   equipment_lastick = SDL_GetTicks();
   equipment_dir     = 0.;

   /* Add ammo. */
   equipment_addAmmo();

   /* text */
   x = 20+sw+20+180+10;
   y = -40;
   window_addText( wid, x, y,
         130, y-20+h-bh, 0, "txtSDesc", &gl_defFont, &cFontGrey, NULL );
   window_addText( wid, x, y,
         w-x-128-30, y-20+h-bh, 0, "txtAcquired", &gl_defFont, NULL, NULL );
   x += 130;
   window_addText( wid, x, y,
         w-x-20-128-20, y-20+h-bh, 0, "txtDDesc", &gl_defFont, NULL, NULL );

   /* Generate lists. */
   window_addText( wid, 30, -20,
         ow, gl_defFont.h, 0, "txtShipTitle", &gl_defFont, NULL, _("Available Ships") );
   window_addText( wid, 30, -40-sh-20,
         ow, gl_defFont.h, 0, "txtOutfitTitle", &gl_defFont, NULL, _("Available Outfits") );

   /* Favourite checkbox, run before genLists. */
   x = -20-(128-cw)/2;
   y = -20-150-ch;
   window_addCheckbox( wid, x, y, cw, 30, "chkFav", _("Favourite"), equipment_toggleFav, 0 );
   if (player.fleet_capacity > 0) {
      y -= 23;
      window_addCheckbox( wid, x, y, cw, 30, "chkDeploy", _("Deployed"), equipment_toggleDeploy, 0 );
   }
   x = -16;
   y -= 23 + 10;
   window_addButtonKey( wid, x, y, 128+8, bh, "btnRenameShip",
         _("Rename"), equipment_renameShip, SDLK_r );
   y -= bh + 10;
   window_addButtonKey( wid, x, y, 128+8, bh, "btnShipMode",
         _("Toggle Display"), equipment_shipMode, SDLK_d );

   /* Generate lists. */
   equipment_genLists( wid );

   /* Slot widget. Designed so that 10 slots barely fit. */
   equipment_slotWidget( wid, 20+sw+15, -40-5, ew, eh, &eq_wgt );
   equipment_slotDeselect( &eq_wgt );
   eq_wgt.canmodify = 1;

   /* Separator. */
   // window_addRect( wid, 20 + sw + 20, -40, 2, h-60, "rctDivider", &cGrey50, 0 );

   /* Custom widget (ship information). */
   window_addCust( wid, -20-(128-cw)/2, -20-150, cw, ch, "cstMisc", 0,
         equipment_renderMisc, NULL, NULL, NULL, NULL );
   window_canFocusWidget( wid, "cstMisc", 0 );

   /* Spinning ship. */
   widptr = malloc(sizeof(unsigned int));
   *widptr = wid;
   window_addRect( wid, -20+4, -40+4, 128+8, 128+8, "rctShip", &cBlack, 1 );
   window_addCust( wid, -20, -40, 128, 128, "cstShip", 0,
         equipment_renderShip, NULL, NULL, NULL, widptr );
   window_custSetDynamic( wid, "cstShip", 1 );
   window_canFocusWidget( wid, "cstShip", 0 );
   window_custAutoFreeData( wid, "cstShip" );

   /* Focus the ships image array. */
   window_setFocus( wid , EQUIPMENT_SHIPS );
   equipment_creating = 0;
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
   equipment_slotDeselect( data );

   /* Create the widget. */
   window_addCust( wid, x, y, w, h, "cstEquipment", 0,
         equipment_renderSlots, equipment_mouseSlots, NULL, NULL, data );
   window_custSetClipping( wid, "cstEquipment", 0 );
   window_custSetOverlay( wid, "cstEquipment", equipment_renderOverlaySlots );
   window_canFocusWidget( wid, "cstEquipment", 0 );
}
/**
 * @brief Renders an outfit column.
 */
static void equipment_renderColumn( double x, double y, double w, double h,
      const PilotOutfitSlot *lst, const char *txt,
      int selected, Outfit *o, Pilot *p, const CstSlotWidget *wgt )
{
   const glColour *c, *dc, *rc;
   glColour bc;

   /* Shouldn't be happening, but let's be nice and not crash. */
   if (!array_size(lst))
      return;

   /* Render text. */
   if ((o != NULL) && (lst[0].sslot->slot.type == o->slot.type))
      c = &cFontGreen;
   else
      c = &cFontWhite;
   gl_printMidRaw( &gl_smallFont, 60.,
         x-15., y+h+10., c, -1., txt );

   /* Iterate for all the slots. */
   for (int i=0; i<array_size(lst); i++) {
      int cantoggle;
      const glTexture *icon;

      /* Skip slots hat are empty and the player can't do anything about. */
      if (lst[i].sslot->locked && !lst[i].sslot->visible && (lst[i].outfit == NULL))
         continue;

      cantoggle = pilot_slotIsToggleable( &lst[i] );

      /* Choose default colour. */
      if (wgt->weapons >= 0) {
         int level = pilot_weapSetInSet( p, wgt->weapons, &lst[i] );
         if (level == 0)
            dc = &cFontRed;
         else if (level == 1)
            dc = &cFontYellow;
         else if (cantoggle)
            dc = &cFontBlue;
         else
            dc = &cFontGrey;
      }
      else
         dc = outfit_slotSizeColour( &lst[i].sslot->slot );

      if (dc == NULL)
         dc = &cGrey60;

      /* Draw background. */
      if ((wgt->weapons >= 0) && (!cantoggle || pilot_weapSetCheck( p, wgt->weapons, &lst[i] )))
         bc = cFontGrey;
      else
         bc = *dc;
      bc.a = 0.4;
      toolkit_drawRect( x, y, w, h, &bc, NULL );

      if (lst[i].outfit != NULL) {
         /* Draw bugger. */
         gl_renderScale( lst[i].outfit->gfx_store,
               x, y, w, h, NULL );
      }
      else if ((o != NULL) &&
            (lst[i].sslot->slot.type == o->slot.type)) {
         /* Render a thick frame with a yes/no colour, and geometric cue. */
         int ok = (!lst[i].sslot->locked && pilot_canEquip( p, &lst[i], o ) == NULL);
         glUseProgram( shaders.status.program );
         glUniform1f( shaders.status.paramf, ok );
         gl_renderShader( x, y, w, h, 0., &shaders.status, NULL, 0 );
      }

      /* Must rechoose colour based on slot properties. */
      rc = dc;
      if (wgt->canmodify) {
         if (lst[i].sslot->locked)
            rc = NULL;
         else if (lst[i].sslot->required)
            rc = &cBrightRed;
         else if (lst[i].sslot->exclusive)
            rc = &cWhite;
         else if (lst[i].sslot->slot.spid != 0)
            rc = &cBlack;
      }

      /* Draw outline. */
      if (i==selected)
         toolkit_drawOutlineThick( x, y, w, h, 5, 7, &cGreen, NULL );
      if (rc != NULL)
         toolkit_drawOutlineThick( x, y, w, h, 1, 3, rc, NULL );

      /* Draw slot iccon if applicable. */
      icon = sp_icon( lst[i].sslot->slot.spid );
      if (icon != NULL) {
            double sw = 14.;
            double sh = 14.;
            double sx = x+w-10.;
            double sy = y+h-10.;

            if (icon->flags & OPENGL_TEX_SDF)
               gl_renderSDF( icon, sx, sy, sw, sh, &cWhite, 0., 1. );
            else
               gl_renderScaleAspect( icon, sx, sy, sw, sh, NULL );
      }

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
static void equipment_calculateSlots( const Pilot *p, double bw, double bh,
      double *w, double *h, int *n, int *m )
{
   double tw, th, s;
   int tm;

   /* Calculate size. */
   tm = MAX( MAX( array_size(p->outfit_weapon), array_size(p->outfit_utility) ), array_size(p->outfit_structure) );
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

   /* Must have selected ship. */
   wgt = (CstSlotWidget*) data;
   if (wgt->selected == NULL)
      return;

   /* Get data. */
   p   = wgt->selected->p;
   selected = wgt->slot;

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;

   /* Draw structure outfits. */
   x  = bx + (tw-w)/2 + 2*tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_structure, _("Structure"),
         selected, wgt->outfit, p, wgt );

   /* Draw systems outfits. */
   selected -= array_size(p->outfit_structure);
   x -= tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_utility, _("Utility"),
         selected, wgt->outfit, p, wgt );

   /* Draw weapon outfits. */
   selected -= array_size(p->outfit_utility);
   x -= tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_weapon, _("Weapon"),
         selected, wgt->outfit, p, wgt );
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

   p = eq_wgt.selected->p;

   /* Base bar properties. */
   w = bw;
   h = 20;
   x = bx;
   y = by + bh - 30 - h;

   /* Fleet capacity. */
   if (player.fleet_capacity > 0) {
      gl_printMidRaw( &gl_smallFont, w, x, y + h + 7, &cFontWhite, -1., _("Fleet Capacity") );

      percent = CLAMP(0., 1., 1.-(double)player.fleet_used / (double)player.fleet_capacity );
      toolkit_drawRect( x, y - 2, w * percent, h + 4, &cBlue, NULL );
      toolkit_drawRect( x + w * percent, y - 2, w * (1.-percent), h + 4, &cBlack, NULL );
      gl_printMid( &gl_smallFont, w,
         x, y + h / 2. - gl_smallFont.h / 2.,
         &cFontWhite, "%d / %d", player.fleet_capacity-player.fleet_used, player.fleet_capacity );

      y -= gl_smallFont.h + 2*h;
   }

   /* Render CPU. */
   gl_printMidRaw( &gl_smallFont, w, x, y + h + 7, &cFontWhite, -1, _("CPU Free") );

   percent = (p->cpu_max > 0) ? CLAMP(0., 1., (double)p->cpu / (double)p->cpu_max) : 0.;
   toolkit_drawRect( x, y - 2, w * percent, h + 4, &cGreen, NULL );
   toolkit_drawRect( x + w * percent, y - 2, w * (1.-percent), h + 4, &cRed, NULL );
   gl_printMid( &gl_smallFont, w, x, y + h / 2. - gl_smallFont.h / 2., &cFontWhite, "%d / %d", p->cpu, p->cpu_max );

   y -= h;

   /* Render mass limit. */
   gl_printMidRaw( &gl_smallFont, w, x, y-3, &cFontWhite, -1., _("Mass Limit Left") );
   y -= gl_smallFont.h + h;

   percent = (p->stats.engine_limit > 0) ? CLAMP(0., 1.,
      (p->stats.engine_limit - p->solid.mass) / p->stats.engine_limit) : 0.;
   toolkit_drawRect( x, y - 2, w * percent, h + 4, &cGreen, NULL );
   toolkit_drawRect( x + w * percent, y - 2, w * (1.-percent), h + 4, &cOrange, NULL );
   gl_printMid( &gl_smallFont, w,
      x, y + h / 2. - gl_smallFont.h / 2.,
      &cFontWhite, "%.0f / %.0f", p->stats.engine_limit - p->solid.mass, p->stats.engine_limit );

   y -= h;
   if (p->stats.engine_limit > 0. && p->solid.mass > p->stats.engine_limit) {
      gl_printMid( &gl_smallFont, w,
         x, y, &cFontRed, _("!! %.0f%% Slower !!"),
         (1. - p->speed / p->speed_base) * 100);
   }
}

/**
 * @brief Renders an outfit column.
 *
 *    @param x X position to render at.
 *    @param y Y position to render at.
 *    @param h Height.
 *    @param lst Array (array.h) of elements.
 *    @param mover Slot for which mouseover is active
 *    @param wgt Widget rendering.
 */
static void equipment_renderOverlayColumn( double x, double y, double h,
      PilotOutfitSlot *lst, int mover, CstSlotWidget *wgt )
{
   const glColour *c;
   glColour tc;
   int text_width, yoff;
   const char *display;

   /* Iterate for all the slots. */
   for (int i=0; i<array_size(lst); i++) {
      int subtitle = 0;

      /* Skip slots hat are empty and the player can't do anything about. */
      if (lst[i].sslot->locked && (lst[i].outfit == NULL))
         continue;

      if (lst[i].outfit != NULL) {
         /* See if needs a subtitle. */
         if ((outfit_isLauncher(lst[i].outfit) ||
                  (outfit_isFighterBay(lst[i].outfit))) &&
                (lst[i].u.ammo.quantity < outfit_amount(lst[i].outfit)))
            subtitle = 1;
      }
      /* Draw bottom. */
      if ((i==mover) || subtitle) {
         int top = 0;
         display = NULL;
         if ((i==mover) && wgt->canmodify) {
            if (lst[i].sslot->locked) {
               top = 1;
               display = _("Locked");
               c = &cFontRed;
            }
            else if (lst[i].outfit != NULL) {
               top = 1;
               display = pilot_canEquip( wgt->selected->p, &lst[i], NULL );
               if (display != NULL)
                  c = &cFontRed;
               else {
                  display = _("Right click to remove");
                  c = &cFontGreen;
               }
            }
            else if ((wgt->outfit != NULL) &&
                  (lst->sslot->slot.type == wgt->outfit->slot.type)) {
               top = 1;
               display = pilot_canEquip( wgt->selected->p, &lst[i], wgt->outfit );
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
   char alt[STRMAX];
   const Outfit *o;
   CstSlotWidget *wgt;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   if (wgt->selected == NULL)
      return;
   p   = wgt->selected->p;

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;

   /* Get selected. */
   mover    = wgt->mouseover;

   /* Render weapon outfits. */
   x  = bx + (tw-w)/2 + 2*tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderOverlayColumn( x, y, h,
         p->outfit_structure, mover, wgt );
   mover    -= array_size(p->outfit_structure);
   x -= tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderOverlayColumn( x, y, h,
         p->outfit_utility, mover, wgt );
   mover    -= array_size(p->outfit_utility);
   x -= tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderOverlayColumn( x, y, h,
         p->outfit_weapon, mover, wgt );

   /* Mouse must be over something. */
   if (wgt->mouseover < 0)
      return;

   /* Get the slot. */
   slot = p->outfits[wgt->mouseover];

   /* For comfortability. */
   o = slot->outfit;

   /* Slot is empty. */
   if (o == NULL) {
      int pos;
      if (slot->sslot->slot.spid) {
         pos = scnprintf( alt, sizeof(alt),
               "#o%s\n", _( sp_display( slot->sslot->slot.spid ) ) );
      }
      else
         pos = 0;
      pos += scnprintf( &alt[pos], sizeof(alt)-pos, _( "#%c%s #%c%s #0slot" ),
            outfit_slotSizeColourFont( &slot->sslot->slot ), _(slotSize( slot->sslot->slot.size )),
            outfit_slotTypeColourFont( &slot->sslot->slot ), _(slotName( slot->sslot->slot.type )) );
      if (slot->sslot->exclusive && (pos < (int)sizeof(alt)))
         pos += scnprintf( &alt[pos], sizeof(alt)-pos,
               _(" [exclusive]") );
      if (slot->sslot->locked && (pos < (int)sizeof(alt)))
         pos += scnprintf( &alt[pos], sizeof(alt)-pos,
               "#r%s#0", _(" [locked]") );
      if (slot->sslot->slot.spid)
         scnprintf( &alt[pos], sizeof(alt)-pos,
               "\n\n%s", _( sp_description( slot->sslot->slot.spid ) ) );
      toolkit_drawAltText( bx + wgt->altx, by + wgt->alty, alt );
      return;
   }

   /* Get text. */
   outfit_altText( alt, sizeof(alt), o, (p==player.p) ? p : NULL );

   /* Display temporary bonuses. */
   if (slot->lua_mem != LUA_NOREF) {
      size_t slen = strlen(alt);
      ss_statsListDesc( slot->lua_stats, &alt[slen], sizeof(alt)-slen, 1 );
   }

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
 *    @param data Unused.
 */
static void equipment_renderShip( double bx, double by,
      double bw, double bh, void *data )
{
   Pilot *p;
   int w, h;
   double px, py, pw, ph, lr, lg, lb, li;
   vec2 v;
   GLint fbo;
   const unsigned int *wid = data;

   /* Must have selected ship. */
   if (eq_wgt.selected == NULL)
      return;
   p = eq_wgt.selected->p;

   /* Don't update if not selected. */
   if (window_isTop(*wid)) {
      unsigned int tick = SDL_GetTicks();
      double dt = (double)(tick - equipment_lastick)/1000.;
      equipment_lastick = tick;
      p->solid.dir += p->turn * dt;
      if (p->solid.dir > 2.*M_PI)
         p->solid.dir = fmod( p->solid.dir, 2.*M_PI );
   }
   else
      equipment_lastick = SDL_GetTicks();
   gl_getSpriteFromDir( &p->tsx, &p->tsy, p->ship->sx, p->ship->sy, p->solid.dir );

   w = p->ship->size;
   h = p->ship->size;

   /* Render ship graphic. */
   if (w > bw) {
      pw = 128.;
      ph = 128.;
   }
   else {
      pw = w;
      ph = h;
   }
   px = bx + (bw-pw)/2;
   py = by + (bh-ph)/2;

   /* Render background. */
   gl_renderRect( px, py, pw, ph, &cBlack );

   /* Use framebuffer to draw, have to use an additional one. */
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
   object_lightGet( &lr, &lg, &lb, &li );
   object_light( 2., 2., 2., 0.8 );
   pilot_renderFramebuffer( p, gl_screen.fbo[4], gl_screen.nw, gl_screen.nh );
   object_light( lr, lg, lb, li );
   glBindFramebuffer( GL_FRAMEBUFFER, fbo );
   gl_renderTextureRaw( gl_screen.fbo_tex[4], 0,
         px, py, pw, ph,
         0, 0, w/(double)gl_screen.nw, h/(double)gl_screen.nh, NULL, 0. );

#ifdef DEBUGGING
   if (debug_isFlag(DEBUG_MARK_EMITTER)) {
      /* Visualize the trail emitters. */
      double dircos, dirsin;
      int i;
      dircos = cos(equipment_dir);
      dirsin = sin(equipment_dir);
      for (i=0; i<array_size(p->ship->trail_emitters); i++) {
         v.x = p->ship->trail_emitters[i].x_engine * dircos -
              p->ship->trail_emitters[i].y_engine * dirsin;
         v.y = p->ship->trail_emitters[i].x_engine * dirsin +
              p->ship->trail_emitters[i].y_engine * dircos +
              p->ship->trail_emitters[i].h_engine;
         v.x *= pw / p->ship->size;
         v.y *= ph / p->ship->size;
         if (p->ship->trail_emitters[i].trail_spec->nebula)
            gl_renderCross(px + pw/2. + v.x, py + ph/2. + v.y*M_SQRT1_2, 2., &cFontBlue);
         else
            gl_renderCross(px + pw/2. + v.x, py + ph/2. + v.y*M_SQRT1_2, 4., &cInert);
      }
   }
#endif /* DEBUGGING */

   if ((eq_wgt.slot >= 0) && p->outfits[eq_wgt.slot]->sslot->slot.type==OUTFIT_SLOT_WEAPON) {
      pilot_getMount( p, p->outfits[eq_wgt.slot], &v );
      px += pw/2.;
      py += ph/2.;
      v.x *= pw / p->ship->size;
      v.y *= ph / p->ship->size;

      /* Render it. */
      glUseProgram(shaders.crosshairs.program);
      glUniform1f(shaders.crosshairs.paramf, 2.);
      gl_renderShader( px+v.x, py+v.y, 7., 7., 0., &shaders.crosshairs, &cRadar_player, 1 );
   }
}
/**
 * @brief Handles a mouse press in column.
 *
 *    @param lst List of items in the column.
 *    @param y Y position of the column.
 *    @param h Height of column.
 *    @param my Mouse press position.
 *    @return Number pressed (or -1 if none).
 */
static int equipment_mouseInColumn( const PilotOutfitSlot *lst, double y, double h, double my )
{
   for (int i=0; i<array_size(lst); i++) {
      /* Skip slots hat are empty and the player can't do anything about. */
      if (lst[i].sslot->locked && (lst[i].outfit == NULL))
         continue;

      /* See if in position. */
      if ((my > y) && (my < y+h+20.))
         return i;
      y -= h+20.;
   }

   return -1.;
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
 *    @param os Array (array.h) of elements in the column.
 *    @param p Pilot to which the elements belong.
 *    @param selected Currently selected element.
 *    @param wgt Slot widget.
 */
static int equipment_mouseColumn( unsigned int wid, const SDL_Event* event,
      double mx, double my, double y, double h, PilotOutfitSlot* os,
      Pilot *p, int selected, CstSlotWidget *wgt )
{
   int ret = equipment_mouseInColumn( os, y, h, my );
   if (ret < 0)
      return 0;

   if (event->type == SDL_MOUSEBUTTONDOWN) {
      /* Normal mouse usage. */
      if (wgt->weapons < 0) {
         if (event->button.button == SDL_BUTTON_LEFT) {
            if (wgt->canmodify) {
               int sel = selected + ret;
               if (wgt->slot==sel)
                  wgt->slot = -1;
               else
                  wgt->slot = sel;
               equipment_regenLists( wid, 1, 0 );
            }
         }
         else if ((event->button.button == SDL_BUTTON_RIGHT) &&
               wgt->canmodify && !os[ret].sslot->locked) {
            equipment_swapSlot( wid, p, &os[ret] );
            hooks_run( "equip" ); /* Equipped. */
         }
      }
      /* Viewing weapon slots. */
      else {
         int level;
         int exists = pilot_weapSetInSet( p, wgt->weapons, &os[ret] );
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
         else
            pilot_weapSetAdd( p, wgt->weapons, &os[ret], level );
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
static int equipment_mouseSlots( unsigned int wid, const SDL_Event* event,
      double mx, double my, double bw, double bh,
      double rx, double ry, void *data )
{
   (void) bw;
   (void) rx;
   (void) ry;
   Pilot *p;
   int selected;
   double x, y;
   double w, h;
   double tw;
   CstSlotWidget *wgt;
   int n, m;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   if (wgt->selected == NULL)
      return 0;
   p   = wgt->selected->p;

   /* Must be left click for now. */
   if ((event->type != SDL_MOUSEBUTTONDOWN) &&
         (event->type != SDL_MOUSEMOTION))
      return 0;

   /* Is covered by something. */
   if (widget_isCovered( wid, "cstEquipment", mx, my )) {
      wgt->mouseover = -1;
      return 0;
   }

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;

   /* Go over each column and pass mouse event. */
   selected = 0;
   x  = (tw-w)/2 + 2*tw;
   y  = bh - (h+20) + (h+20-h)/2 - 10;
   if ((mx > x-10) && (mx < x+w+10)) {
      int ret = equipment_mouseColumn( wid, event, mx, my, y, h,
            p->outfit_structure, p, selected, wgt );
      if (ret)
         return !!(event->type == SDL_MOUSEBUTTONDOWN);
   }
   selected += array_size(p->outfit_structure);
   x -= tw;
   if ((mx > x-10) && (mx < x+w+10)) {
      int ret = equipment_mouseColumn( wid, event, mx, my, y, h,
            p->outfit_utility, p, selected, wgt );
      if (ret)
         return !!(event->type == SDL_MOUSEBUTTONDOWN);
   }
   selected += array_size(p->outfit_utility);
   x -= tw;
   if ((mx > x-10) && (mx < x+w+10)) {
      int ret = equipment_mouseColumn( wid, event, mx, my, y, h,
            p->outfit_weapon, p, selected, wgt );
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
   /* Slot can be changed later in land_refuel() so have to save early. */
   int selslot = eq_wgt.slot;

   /* Remove outfit. */
   if (slot->outfit != NULL) {
      int ret;
      const Outfit *o = slot->outfit;

      /* Must be able to remove. */
      if (pilot_canEquip( eq_wgt.selected->p, slot, NULL ) != NULL)
         return 0;

      /* Remove ammo first. */
      pilot_rmAmmo( eq_wgt.selected->p, slot, slot->u.ammo.quantity );

      /* Remove outfit. */
      ret = pilot_rmOutfit( eq_wgt.selected->p, slot );
      if (ret == 0)
         equipment_playerAddOutfit( o, 1 );
   }
   /* Add outfit. */
   else {
      int ret;
      const Outfit *o = eq_wgt.outfit;
      /* Must have outfit. */
      if (o==NULL)
         return 0;

      /* Must fit slot. */
      if (!outfit_fitsSlot( o, &slot->sslot->slot ))
         return 0;

      /* Must be able to add. */
      if (pilot_canEquip( eq_wgt.selected->p, slot, o ) != NULL)
         return 0;

      /* Add outfit to ship. */
      ret = equipment_playerRmOutfit( o, 1 );
      if (ret == 1) {
         pilot_addOutfitRaw( eq_wgt.selected->p, o, slot );

         /* Recalculate stats. */
         pilot_outfitLInitAll( eq_wgt.selected->p );
         pilot_calcStats( eq_wgt.selected->p ); /* TODO avoid running twice. */
      }

      equipment_addAmmo();
   }

   /* Refuel if necessary. */
   land_refuel();

   /* Recalculate stats. */
   pilot_calcStats( p );
   pilot_healLanded( p );

   /* Redo the outfits thingy, while conserving slot. */
   equipment_regenLists( wid, 1, 1 );
   eq_wgt.slot = selslot;

   /* Update outfits. */
   outfits_updateEquipmentOutfits();

   /* Update weapon sets if needed. */
   if (eq_wgt.selected->p->autoweap) {
      pilot_weaponAuto( eq_wgt.selected->p );
      ws_copy( eq_wgt.selected->weapon_sets, eq_wgt.selected->p->weapon_sets );
   }
   pilot_weaponSafe( eq_wgt.selected->p );

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
   int nship = 0, noutfit = 0;
   double offship = 0, offoutfit = 0;
   char *selship = NULL;
   const char *s = NULL;
   char *focused;

   /* Ignore when creating, should be generated correctly anyway. */
   if (equipment_creating)
      return;

   /* Defaults. */
   nship    = 0;
   offship  = 0.;

   /* Must exist. */
   if (!window_existsID( equipment_wid ))
      return;

   /* Save focus. */
   focused = window_getFocus( wid );

   /* Save positions. */
   if (outfits) {
      noutfit   = toolkit_getImageArrayPos(    wid, EQUIPMENT_OUTFITS );
      offoutfit = toolkit_getImageArrayOffset( wid, EQUIPMENT_OUTFITS );
      window_destroyWidget( wid, EQUIPMENT_OUTFITS );
   }
   if (ships) {
      nship   = toolkit_getImageArrayPos(    wid, EQUIPMENT_SHIPS );
      offship = toolkit_getImageArrayOffset( wid, EQUIPMENT_SHIPS );
      s       = toolkit_getImageArray(       wid, EQUIPMENT_SHIPS );
      selship = strdup( s );
      window_destroyWidget( wid, EQUIPMENT_SHIPS );
   }

   /* Regenerate lists. */
   equipment_genLists( wid );

   /* Restore positions. */
   if (outfits) {
      toolkit_setImageArrayPos(    wid, EQUIPMENT_OUTFITS, noutfit );
      toolkit_setImageArrayOffset( wid, EQUIPMENT_OUTFITS, offoutfit );
      equipment_updateOutfits( wid, NULL );
   }
   if (ships) {
      toolkit_setImageArrayPos(    wid, EQUIPMENT_SHIPS, nship );
      toolkit_setImageArrayOffset( wid, EQUIPMENT_SHIPS, offship );
      /* Try to maintain same ship selected. */
      s = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
      if ((s != NULL) && (strcmp(s,selship)!=0)) {
         int ret = toolkit_setImageArray( wid, EQUIPMENT_SHIPS, selship );
         if (ret != 0) /* Failed to maintain. */
            toolkit_setImageArrayPos( wid, EQUIPMENT_SHIPS, nship );

         /* Update ships. */
         equipment_updateShips( wid, NULL );
      }
      free( selship );
   }

   /* Restore focus. */
   window_setFocus( wid, focused );
   free(focused);
}

/**
 * @brief Makes sure it's valid to sell a ship.
 *    @param shipname Ship being sold.
 */
int equipment_canSellPlayerShip( const char *shipname )
{
   int failure = 0;
   land_errClear();
   if (strcmp( shipname, player.p->name )==0) { /* Already on-board. */
      land_errDialogueBuild( _("You can't sell the ship you're piloting!") );
      failure = 1;
   }

   return !failure;
}

/**
 * @brief Makes sure it's valid to change ships in the equipment view.
 *    @param shipname Ship being changed to.
 */
int equipment_canSwapPlayerShip( const char *shipname )
{
   int diff;
   Pilot *newship;
   const PlayerShip_t *ps = player_getPlayerShip( shipname );
   land_errClear();

   if (strcmp(shipname,player.p->name)==0) { /* Already onboard. */
      land_errDialogueBuild( _("You're already onboard the %s."), shipname );
      return 0;
   }

   /* Ship can't be piloted by player. */
   if (ship_isFlag( ps->p->ship, SHIP_NOPLAYER )) {
      land_errDialogueBuild( _("You can not pilot the %s! The ship can only be used as an escort."), shipname );
      return 0;
   }

   /* Ship can't be set as an escort. */
   if (ship_isFlag( player.p->ship, SHIP_NOESCORT )) {
      land_errDialogueBuild( _("You can not swap ships and set %s as an escort!"), player.p->name );
      return 0;
   }

   newship = ps->p;
   if (ps->deployed)
      diff = 0;
   else
      diff = pilot_cargoUsed(player.p) - pilot_cargoFree(newship) - (pfleet_cargoFree() - pilot_cargoFree(player.p)); /* Has to fit all the cargo. */
   diff = MAX( diff, pilot_cargoUsedMission(player.p) - pilot_cargoFree(newship) ); /* Has to fit all mission cargo. */
   if (diff > 0) { /* Current ship has too much cargo. */
      land_errDialogueBuild( n_(
               "You have %d tonne more cargo than the new ship can hold.",
               "You have %d tonnes more cargo than the new ship can hold.",
               diff),
            diff );
      return 0;
   }
   if (pilot_hasDeployed( player.p )) {
      if (!dialogue_YesNo(_("Recall Fighters"), _("This action will recall your deployed fighters. Is that OK?"))) {
         land_errDialogueBuild( _("You have deployed fighters.") );
         return 0;
      }
      /* Recall fighters. */
      escort_clearDeployed( player.p );
   }
   return 1;
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
      p = eq_wgt.selected->p;

   /* Add ammo to all outfits. */
   pilot_fillAmmo( p );

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
 *    @param name Whether or not to display the pilot name.
 */
int equipment_shipStats( char *buf, int max_len,  const Pilot *s, int dpseps, int name )
{
   int l;
   double eps, dps;

   dps = 0.;
   eps = 0.;
   /* Calculate damage and energy per second. */
   if (dpseps)
      pilot_dpseps( s, &dps, &eps );

   /* Write to buffer. */
   if (name)
      l = scnprintf( buf, max_len, "%s\n", s->name );
   else
      l = 0;
   if (dps > 0.)
      l += scnprintf( &buf[l], (max_len-l),
            _("%.2f DPS [%.2f EPS]\n"), dps, eps );
   if (s->ship->desc_extra != NULL)
      l += scnprintf( &buf[l], (max_len-l), "%s\n", _(s->ship->desc_extra) );
   l += scnprintf( &buf[l], (max_len-l), _("%.0f fuel per jump\n"), s->fuel_consumption );
   l += ss_statsDesc( &s->stats, &buf[l], (max_len-l), 0 );
   return l;
}

/**
 * @brief Handles toggling of the favourite checkbox.
 */
static void equipment_toggleFav( unsigned int wid, const char *wgt )
{
   int state = window_checkboxState( wid, wgt );
   const char *shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   if (strcmp(shipname,player.p->name)==0) { /* no ships */
      player.ps.favourite = state;
   }
   else {
      PlayerShip_t *ps = player_getPlayerShip( shipname );
      ps->favourite = state;
   }

   /* Update ship to reflect changes. */
   equipment_regenLists( wid, 0, 1 );
}

static void equipment_toggleDeploy( unsigned int wid, const char *wgt )
{
   int state = window_checkboxState( wid, wgt );
   const char *shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );

   /* Can't deploy if no capacity. */
   if (player.fleet_capacity <= 0)
      return;

   /* Only if current ship isn't selected try to deploy. */
   if (strcmp(shipname,player.p->name)!=0) {
      PlayerShip_t *ps = player_getPlayerShip( shipname );
      if (state && ship_isFlag( ps->p->ship, SHIP_NOESCORT )) {
         dialogue_msg( _("Invalid Escort"), _("You can not set your ship '%s' as an escort!"), ps->p->name );
         return;
      }
      if (pfleet_toggleDeploy( ps, state ))
         return;
   }
   else
      window_checkboxSet( wid, wgt, 1 ); /* Player is always deployed. */

   /* Update ship to reflect changes. */
   equipment_regenLists( wid, 0, 1 );
}

/**
 * @brief Generates a new ship/outfit lists if needed.
 *
 *    @param wid Parent window id.
 */
static void equipment_genLists( unsigned int wid )
{
   equipment_genShipList( wid );
   equipment_genOutfitList( wid );
}

/**
 * @brief Generates the ship list.
 *    @param wid Window to generate list on.
 */
static void equipment_genShipList( unsigned int wid )
{
   ImageArrayCell *cships;
   int nships;
   int w, h;
   int sw, sh;
   const PlayerShip_t *ps;
   char r[PATH_MAX];
   glTexture *t;
   int iconsize;

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, &sw, &sh, NULL, NULL,
         NULL, NULL, NULL, NULL, NULL, NULL );

   if (widget_exists( wid, EQUIPMENT_SHIPS ))
      return;

   eq_wgt.selected = NULL;
   if (spob_hasService(land_spob, SPOB_SERVICE_SHIPYARD))
      nships   = player_nships()+1;
   else
      nships   = 1;
   cships   = calloc( nships, sizeof(ImageArrayCell) );
   /* Add player's current ship. */
   cships[0].image = gl_dupTexture(player.p->ship->gfx_store);
   cships[0].caption = strdup(player.p->name);
   cships[0].layers = gl_copyTexArray( player.p->ship->gfx_overlays );
   t = gl_newImage( OVERLAY_GFX_PATH"active.webp", 0 );
   cships[0].layers = gl_addTexArray( cships[0].layers, t );
   if (player.ps.favourite) {
      t = gl_newImage( OVERLAY_GFX_PATH"favourite.webp", 0 );
      cships[0].layers = gl_addTexArray( cships[0].layers, t );
   }
   if (player.p->ship->rarity > 0) {
      snprintf( r, sizeof(r), OVERLAY_GFX_PATH"rarity_%d.webp", player.p->ship->rarity );
      t = gl_newImage( r, 0 );
      cships[0].layers = gl_addTexArray( cships[0].layers, t );
   }
   if (spob_hasService(land_spob, SPOB_SERVICE_SHIPYARD)) {
      player_shipsSort();
      ps = player_getShipStack();
      for (int i=1; i<=array_size(ps); i++) {
         cships[i].image = gl_dupTexture( ps[i-1].p->ship->gfx_store );
         cships[i].caption = strdup( ps[i-1].p->name );
         cships[i].layers = gl_copyTexArray( ps[i-1].p->ship->gfx_overlays );
         if (ps[i-1].favourite) {
            t = gl_newImage( OVERLAY_GFX_PATH"favourite.webp", 0 );
            cships[i].layers = gl_addTexArray( cships[i].layers, t );
         }
         if (ps[i-1].deployed) {
            t = gl_newImage( OVERLAY_GFX_PATH"fleet.webp", 0 );
            cships[i].layers = gl_addTexArray( cships[i].layers, t );
         }
         if (ps[i-1].p->ship->rarity > 0) {
            snprintf( r, sizeof(r), OVERLAY_GFX_PATH"rarity_%d.webp", ps[i-1].p->ship->rarity );
            t = gl_newImage( r, 0 );
            cships[i].layers = gl_addTexArray( cships[i].layers, t );
         }
      }
   }
   /* Ship stats in alt text. */
   for (int i=0; i<nships; i++) {
      const Pilot *s  = player_getShip( cships[i].caption );
      int l;
      cships[i].alt = malloc( STRMAX );
      l  = snprintf( &cships[i].alt[0], STRMAX, _("Ship Stats\n") );
      l  = equipment_shipStats( &cships[i].alt[0], STRMAX-l, s, 1, 1 );
      if (l == 0) {
         free( cships[i].alt );
         cships[i].alt = NULL;
      }
   }

   /* Create the image array. */
   iconsize = 96;
   if (!conf.big_icons) {
      if (toolkit_simImageArrayVisibleElements(sw,sh,iconsize,iconsize) < nships)
         iconsize = 80;
      if (toolkit_simImageArrayVisibleElements(sw,sh,iconsize,iconsize) < nships)
         iconsize = 64;
   }
   window_addImageArray( wid, 20, -40,
         sw, sh, EQUIPMENT_SHIPS, iconsize, iconsize,
         cships, nships, equipment_updateShips, equipment_rightClickShips, equipment_transChangeShip );
   toolkit_setImageArrayAccept( wid, EQUIPMENT_SHIPS, equipment_transChangeShip );

   equipment_updateShips(wid, NULL);
}

static int equipment_filter( const Outfit *o ) {
   Pilot *p = (eq_wgt.selected==NULL) ? NULL : eq_wgt.selected->p;
   const PlayerShip_t *ps;

   /* Filter only those that fit slot. */
   if ((p!=NULL) && (eq_wgt.slot >= 0)) {
      const PilotOutfitSlot *pos = p->outfits[ eq_wgt.slot ];
      if (!outfit_fitsSlot( o, &pos->sslot->slot ))
         return 0;
   }

   /* Standard filtering. */
   switch (equipment_outfitMode) {
      case 0:
         return 1;

      case 1: /* Fits any ship of the player. */
         ps = player_getShipStack();
         for (int j=0; j < array_size(ps); j++) {
            const Pilot *pp = ps[j].p;
            for (int i=0; i < array_size(pp->outfits); i++) {
               if (outfit_fitsSlot( o, &pp->outfits[i]->sslot->slot ))
                  return 1;
            }
         }
         return 0;

      case 2: /* Fits currently selected ship. */
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
static int equipment_filterWeapon( const Outfit *o ) {
   return equipment_filter(o) && outfit_filterWeapon(o);
}
static int equipment_filterUtility( const Outfit *o ) {
   return equipment_filter(o) && outfit_filterUtility(o);
}
static int equipment_filterStructure( const Outfit *o ) {
   return equipment_filter(o) && outfit_filterStructure(o);
}
static int equipment_filterCore( const Outfit *o ) {
   return equipment_filter(o) && outfit_filterCore(o);
}

/**
 * @brief Generates the outfit list.
 *    @param wid Window to generate list on.
 */
static void equipment_genOutfitList( unsigned int wid )
{
   int x, y, w, h, ow, oh;
   int ix, iy, iw, ih, barw; /* Input filter. */
   const char *filtertext;
   int (*tabfilters[])( const Outfit *o ) = {
      equipment_filter,
      equipment_filterWeapon,
      equipment_filterUtility,
      equipment_filterStructure,
      equipment_filterCore,
   };
   const char *tabnames[] = {
      _("All"), _(OUTFIT_LABEL_WEAPON), _(OUTFIT_LABEL_UTILITY), _(OUTFIT_LABEL_STRUCTURE), _(OUTFIT_LABEL_CORE)
   };
   int noutfits, active;
   ImageArrayCell *coutfits;
   int iconsize;
   const Pilot *p = (eq_wgt.selected != NULL) ? eq_wgt.selected->p : NULL;

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
      ih = 30;

      ix = ow - iw + 15;
      iy = y + oh - 25 - 1;

#ifdef DEBUGGING
      if (iw <= 60)
         WARN(_("Very little space on equipment outfit tabs!"));
#endif /* DEBUGGING */

      /* Add popdown menu stuff. */
      window_addButton( wid, ix+iw-30, iy, 30, 30, "btnOutfitFilter", NULL, equipment_outfitPopdown );
      window_buttonCustomRender( wid, "btnOutfitFilter", window_buttonCustomRenderGear );
      iw -= 35;

      /* Set text filter. */
      window_addInput( wid, ix, iy, iw, ih, EQUIPMENT_FILTER, 32, 1, NULL );
      inp_setEmptyText( wid, EQUIPMENT_FILTER, _("Filter…") );
      window_setInputCallback( wid, EQUIPMENT_FILTER, equipment_filterOutfits );
   }

   window_tabWinOnChange( wid, EQUIPMENT_OUTFIT_TAB, equipment_changeTab );
   active = window_tabWinGetActive( equipment_wid, EQUIPMENT_OUTFIT_TAB );

   /* Widget must not already exist. */
   if (widget_exists( wid, EQUIPMENT_OUTFITS ))
      return;

   /* Allocate space. */
   noutfits = MAX( 1, player_numOutfits() ); /* This is the most we'll need, probably less due to filtering. */
   array_free( iar_outfits[active] );
   iar_outfits[active] = array_create( Outfit* );

   filtertext = NULL;
   if (widget_exists(equipment_wid, EQUIPMENT_FILTER)) {
      filtertext = window_getInput( equipment_wid, EQUIPMENT_FILTER );
      if (strlen(filtertext) == 0)
         filtertext = NULL;
   }

   /* Get the outfits. */
   noutfits = player_getOutfitsFiltered( (const Outfit***)&iar_outfits[active], tabfilters[active], filtertext );
   coutfits = outfits_imageArrayCells( (const Outfit**)iar_outfits[active], &noutfits, (p==NULL) ? player.p : p, 0 );

   /* Create the actual image array. */
   iw = ow - 6;
   ih = oh - 37;
   iconsize = 96;
   if (!conf.big_icons) {
      if (toolkit_simImageArrayVisibleElements(iw,ih,iconsize,iconsize) < noutfits)
         iconsize = 80;
      if (toolkit_simImageArrayVisibleElements(iw,ih,iconsize,iconsize) < noutfits)
         iconsize = 64;
   }
   window_addImageArray( wid, x + 4, y + 3, iw, ih,
         EQUIPMENT_OUTFITS, iconsize, iconsize,
         coutfits, noutfits,
         equipment_updateOutfits,
         equipment_rightClickOutfits,
         equipment_rightClickOutfits );

   toolkit_setImageArrayAccept( wid, EQUIPMENT_OUTFITS, equipment_rightClickOutfits );

   equipment_updateOutfits(wid, NULL);
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
#define EQ_COMP_I( cur, base, inv ) \
eq_qCol( cur, base, inv ), eq_qSym( cur, base, inv )
/**
 * @brief Updates the player's ship window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateShips( unsigned int wid, const char* str )
{
   (void) str;
   char buf[STRMAX], buf_price[ECON_CRED_STRLEN];
   char errorReport[STRMAX_SHORT], tbuf[64];
   const char *shipname, *acquired;
   char scargo[NUM2STRLEN];
   Pilot *ship;
   PlayerShip_t *ps, *prevship;
   char *nt;
   int onboard, cargo, jumps, favourite, deployed, x, h, spaceworthy;
   int ww, wh, sw, sh, bw, bh, hacquired, hname;
   int wgtw, wgth;
   size_t l = 0;

   equipment_getDim( wid, &ww, &wh, &sw, &sh, NULL, NULL, NULL, NULL, NULL, NULL, &bw, &bh );

   /* Clear defaults. */
   eq_wgt.slot          = -1;
   eq_wgt.mouseover     = -1;
   equipment_lastick    = SDL_GetTicks();

   /* Get the ship. */
   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   if (strcmp(shipname,player.p->name)==0) { /* no ships */
      ps      = &player.ps;
      onboard = 1;
      deployed = 1;
   }
   else {
      ps       = player_getPlayerShip( shipname );
      onboard  = 0;
      deployed = ps->deployed;
   }
   ship        = ps->p;
   favourite   = ps->favourite;
   prevship    = eq_wgt.selected;
   eq_wgt.selected = ps;

   /* update text */
   credits2str( buf_price, player_shipPrice(shipname,0), 2 ); /* sell price */
   cargo = pilot_cargoFree(ship) + pilot_cargoUsed(ship);
   nt = ntime_pretty( pilot_hyperspaceDelay( ship ), 2 );

   /* Get ship error report. */
   spaceworthy = !pilot_reportSpaceworthy( ship, errorReport, sizeof(errorReport));

   jumps = floor(ship->fuel_max / ship->fuel_consumption);

   /* Get acquired text length. */
   x = 20+sw+20+180+10;
   acquired = (ps->acquired) ? ps->acquired : _("You do not remember how you acquired this ship.");
   window_dimWidget( wid, "txtAcquired", &wgtw, &wgth );
   hacquired = gl_printLinesRaw( &gl_defFont, wgtw, acquired );
   window_dimWidget( wid, "txtDDesc", &wgtw, &wgth );
   hname = gl_printLinesRaw( &gl_defFont, wgtw, ship->name );

   /* Helper strings. */
   num2str( scargo, pilot_cargoUsed(ship), 0 );

   /* Destroy intrinsic widget if applicable. */
   if (widget_exists( wid, "iarIntrinsic" ))
      window_destroyWidget( wid, "iarIntrinsic" );

   l += scnprintf( &buf[l], sizeof(buf)-l, "%s", _("Name:") );
   for (int i=0; i<hname-1; i++)
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Model:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Class:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Acquired Date:") );
   for (int i=0; i<hacquired+1; i++)
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
   if (ship_mode==0) {
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Value:") );
      if (player.fleet_capacity > 0)
         l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Fleet Capacity:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Crew:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Mass:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Jump Time:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Accel:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Speed:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Turn:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Time Constant:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Detected at:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Signature:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Stealth at:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Scanning time:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Absorption:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Shield:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Armour:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Energy:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Cargo Space:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Fuel:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Ship Status:") );
   }
   else {
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Time Flown:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Jumped Times:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Landed Times:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Damage Done:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Damage Taken:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _("Ships Destroyed:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n\n%s", _("Intrinsic Outfits:") );
   }
   window_modifyText( wid, "txtSDesc", buf );

   /* Fill the buffer. */
   /* Generic. */
   l = 0;
   l += scnprintf( &buf[l], sizeof(buf)-l, "%s", ship->name );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _(ship->ship->name) );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", _(ship_classDisplay(ship->ship)) );
   if (ps->acquired_date==0)
      snprintf( tbuf, sizeof(tbuf), _("Unknown") );
   else
      ntime_prettyBuf( tbuf, sizeof(tbuf), ps->acquired_date, 2 );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", tbuf );

   h = gl_printHeightRaw( &gl_defFont, wgtw, buf );
   window_moveWidget( wid, "txtAcquired", x, -40-h-6 );
   window_modifyText( wid, "txtAcquired", acquired );

   for (int i=0; i<hacquired+1; i++)
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
   if (ship_mode==0) {
      /* Some core stats. */
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", buf_price );
      if (player.fleet_capacity > 0)
         l += scnprintf( &buf[l], sizeof(buf)-l, "\n%d", ship->ship->points );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n#%c%s%d#0", EQ_COMP( (int)floor(ship->crew), ship->ship->crew, 0 ) );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s#0 %s", num2strU(ship->solid.mass,0), UNIT_MASS );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s average"), nt );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("#%c%s%.0f#0 %s"),
            EQ_COMP( ship->accel, ship->ship->accel, 0 ), UNIT_ACCEL );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("\n#%c%s%.0f#0 %s (max #%c%s%.0f#0 %s)"),
            EQ_COMP( ship->speed, ship->ship->speed, 0 ), UNIT_SPEED,
            EQ_COMP( solid_maxspeed( &ship->solid, ship->speed, ship->accel ),
               solid_maxspeed( &ship->solid, ship->ship->speed, ship->ship->accel), 0 ), UNIT_SPEED );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("\n#%c%s%.0f#0 %s"),
            EQ_COMP( ship->turn*180./M_PI, ship->ship->turn*180./M_PI, 0 ), UNIT_ROTATION );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%.0f%%", ship->stats.time_mod * ship->ship->dt_default*100. );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s %s\n", num2strU(ship->ew_detection,0), UNIT_DISTANCE );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s %s"), num2strU(ship->ew_signature,0), UNIT_DISTANCE );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s %s\n", num2strU(ship->ew_stealth,0), UNIT_DISTANCE );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%.1f %s"), pilot_ewScanTime(ship), UNIT_TIME );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n\n" );
      /* Health. */
      l += scnprintf( &buf[l], sizeof(buf)-l, "#%c%s%.0f%%\n", EQ_COMP( ship->dmg_absorb*100., ship->ship->dmg_absorb*100., 0 ) );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("#%c%s%s#0 %s"),
            EQ_COMP_I( ship->shield_max, ship->ship->shield, 0 ), num2strU(ship->shield_max,0), UNIT_ENERGY );
      l += scnprintf( &buf[l], sizeof(buf)-l, _(" (#%c%s%s#0 %s)"),
            EQ_COMP_I( ship->shield_regen, ship->ship->shield_regen, 0 ), num2strU(ship->shield_regen,1), UNIT_POWER );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("#%c%s%s#0 %s"),
            EQ_COMP_I( ship->armour_max, ship->ship->armour, 0 ), num2strU(ship->armour_max,0), UNIT_ENERGY );
      l += scnprintf( &buf[l], sizeof(buf)-l, _(" (#%c%s%s#0 %s)"),
            EQ_COMP_I( ship->armour_regen, ship->ship->armour_regen, 0 ), num2strU(ship->armour_regen,1), UNIT_POWER );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("#%c%s%s#0 %s"),
            EQ_COMP_I( ship->energy_max, ship->ship->energy, 0 ), num2strU(ship->energy_max,0), UNIT_ENERGY );
      l += scnprintf( &buf[l], sizeof(buf)-l, _(" (#%c%s%s#0 %s)"),
            EQ_COMP_I( ship->energy_regen, ship->ship->energy_regen, 0 ), num2strU(ship->energy_regen,1), UNIT_POWER );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      /* Misc. */
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s / #%c%s%s#0 %s"),
            scargo, EQ_COMP_I( cargo, ship->ship->cap_cargo, 0 ), num2strU(cargo,0), UNIT_MASS );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s %s (%d %s)"),
            num2strU(ship->fuel_max,0), UNIT_UNIT, jumps, n_( "jump", "jumps", jumps ) );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      /*l +=*/ scnprintf( &buf[l], sizeof(buf)-l, "\n#%c%s#0", spaceworthy ? '0' : 'r', errorReport );
   }
   else {
      int destroyed = 0;
      for (int i=0; i<SHIP_CLASS_TOTAL; i++)
         destroyed += ps->ships_destroyed[i];
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s hours"), num2strU(ps->time_played/3600.,1) );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", num2strU(ps->jumped_times,0) );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n%s", num2strU(ps->landed_times,0) );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s %s"), num2strU(ps->dmg_done_shield+ps->dmg_done_armour,0), UNIT_ENERGY );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n" );
      l += scnprintf( &buf[l], sizeof(buf)-l, _("%s %s"), num2strU(ps->dmg_taken_shield+ps->dmg_taken_armour,0), UNIT_ENERGY );
      /*l +=*/ scnprintf( &buf[l], sizeof(buf)-l, "\n%s", num2strU(destroyed,0) );
   }
   window_modifyText( wid, "txtDDesc", buf );

   /* Add intrinsic outfit image array. */
   if (ship_mode!=0) {
      int tx, ty, tw, th;
      ImageArrayCell *cells;
      int ncells = array_size(ship->outfit_intrinsic);
      Outfit const **outfits = (Outfit const**) array_create( Outfit* );
      for (int i=0; i<ncells; i++)
         array_push_back( &outfits, ship->outfit_intrinsic[i].outfit );

      cells = outfits_imageArrayCells( outfits, &ncells, ship, 0 );

      window_posWidget( wid, "txtSDesc", &tx, &ty );
      window_dimWidget( wid, "txtSDesc", &tw, &th );
      ty = -40-window_getTextHeight( wid, "txtSDesc" )-10;
      window_addImageArray( wid, tx, ty, ww-x-128-30-10, ty-20+wh-bh-30, "iarIntrinsic",
         64, 64, cells, ncells, NULL, NULL, NULL );

      array_free(outfits);
   }

   /* Clean up. */
   free( nt );

   /* Set checkboxes. */
   window_checkboxSet( wid, "chkFav", favourite );
   if (player.fleet_capacity > 0)
      window_checkboxSet( wid, "chkDeploy", deployed );

   /* button disabling */
   if (onboard) {
      window_disableButton( wid, "btnSellShip" );
      window_disableButton( wid, "btnChangeShip" );
   }
   else {
      window_enableButton( wid, "btnChangeShip" );
      window_enableButton( wid, "btnSellShip" );
   }

   /* If pilot-dependent outfit filter modes are active, we have to regenerate outfits always. */
   if ((equipment_outfitMode==2) && (eq_wgt.selected != prevship))
      equipment_regenLists( wid, 1, 0 );
}
#undef EQ_COMP
/**
 * @brief Updates the player's outfit list.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateOutfits( unsigned int wid, const char* str )
{
   (void) str;
   int i, active;

   /* Must have outfit. */
   active = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
   i = toolkit_getImageArrayPos( wid, EQUIPMENT_OUTFITS );
   if (i < 0 || array_size(iar_outfits[active])==0) {
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
static void equipment_filterOutfits( unsigned int wid, const char *str )
{
   (void) str;
   equipment_regenLists(wid, 1, 0);
}

/**
 * Functions for the popdown menu (filter outfits by size)
 */

static void equipment_outfitPopdownSelect( unsigned int wid, const char *str )
{
   int m = toolkit_getListPos( wid, str );
   if (m == equipment_outfitMode)
      return;

   equipment_outfitMode = m;
   equipment_regenLists( wid, 1, 0 );
   player.eq_outfitMode = m;
}

static void equipment_outfitPopdownActivate( unsigned int wid, const char *str )
{
   equipment_outfitPopdownSelect( wid, str );
   window_destroyWidget( wid, str );
}

static void equipment_outfitPopdown( unsigned int wid, const char* str )
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
   window_addList( wid, x+w, y-120+h, 350, 120, name, modelist, n, equipment_outfitMode, equipment_outfitPopdownSelect, equipment_outfitPopdownActivate );
   window_setFocus( wid, name );
}

/**
 * @brief Ensures the tab's selected item is reflected in the ship slot list
 *
 *    @param wid Unused.
 *    @param wgt Unused.
 *    @param old Tab changed from.
 *    @param tab Tab changed to.
 */
static void equipment_changeTab( unsigned int wid, const char *wgt, int old, int tab )
{
   (void) wgt;
   iar_data_t old_data;

   toolkit_saveImageArrayData( wid, EQUIPMENT_OUTFITS, &iar_data[old] );

   /* Store the currently-saved positions for the new tab. */
   old_data = iar_data[tab];

   /* Resetting the input will cause the outfit list to be regenerated. */
   if (widget_exists(wid, EQUIPMENT_FILTER))
      window_setInput(wid, EQUIPMENT_FILTER, NULL);
   else
      equipment_regenLists(wid, 1, 0);

   /* Set positions for the new tab. This is necessary because the stored
    * position for the new tab may have exceeded the size of the old tab,
    * resulting in it being clipped. */
   toolkit_loadImageArrayData( wid, EQUIPMENT_OUTFITS, &old_data );

   /* Focus the outfit image array. */
   window_setFocus( wid, EQUIPMENT_OUTFITS );
}

/**
 * @brief Toggles deployed status.
 */
static void equipment_rightClickShips( unsigned int wid, const char *str )
{
   const char *shipname = toolkit_getImageArray( wid, str );
   PlayerShip_t *ps = player_getPlayerShip( shipname );

   /* Must have fleet capacity. */
   if (player.fleet_capacity <= 0)
      return;

   /* Deploy the ship if applicable. */
   if (ps == NULL)
      return;

   /* Don't do anything for player ship. */
   if (strcmp(shipname,player.p->name)==0)
      return;

   if (!ps->deployed && ship_isFlag( ps->p->ship, SHIP_NOESCORT )) {
      dialogue_msg( _("Invalid Escort"), _("You can not set your ship '%s' as an escort!"), ps->p->name );
      return;
   }

   /* Try to deploy. */
   if (pfleet_toggleDeploy( ps, !ps->deployed ))
      return;

   if (player.fleet_capacity > 0)
      window_checkboxSet( wid, "chkDeploy", ps->deployed );

   equipment_regenLists( wid, 0, 1 );
}

/**
 * @brief Changes ship.
 *    @param wid Window player is attempting to change ships in.
 *    @param str Unused.
 */
static void equipment_transChangeShip( unsigned int wid, const char* str )
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
   const char *shipname, *filtertext;
   PlayerShip_t *ps;
   int i;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   ps = player_getPlayerShip( shipname );
   if (ps != NULL) {
      /* Swap deployed status. */
      player.ps.deployed = ps->deployed;
   }

   if (!equipment_canSwapPlayerShip( shipname )) {
      land_errDisplay();
      return;
   }

   /* Store active tab, filter text, and positions for the outfits. */
   i = window_tabWinGetActive( wid, EQUIPMENT_OUTFIT_TAB );
   toolkit_saveImageArrayData( wid, EQUIPMENT_OUTFITS, &iar_data[i] );
   if (widget_exists(wid, EQUIPMENT_FILTER))
      filtertext = window_getInput( equipment_wid, EQUIPMENT_FILTER );
   else
      filtertext = NULL;

   /* Swap ship. */
   player_swapShip( shipname, 1 );
   pilot_healLanded( player.p );

   /* What happens here is the gui gets recreated when the player swaps ship.
    * This causes all the windows to be destroyed and the 'wid' we have here
    * becomes invalid. However, since we store it in a global variable we can
    * recover it and use it instead. */
   wid = equipment_wid;

   /* Restore outfits image array properties. */
   window_tabWinSetActive( wid, EQUIPMENT_OUTFIT_TAB, i );
   toolkit_loadImageArrayData( wid, EQUIPMENT_OUTFITS, &iar_data[i] );
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
static void equipment_unequipShip( unsigned int wid, const char* str )
{
   (void) str;
   Pilot *ship = eq_wgt.selected->p;

   /*
    * Unequipping is disallowed under two conditions. Firstly, the ship may not
    * be unequipped when it has fighters deployed in space. Secondly, it cannot
    * unequip if it's carrying more cargo than the ship normally fits, i.e.
    * by equipping cargo pods.
    */
   if (pilot_cargoUsed(ship) > ship->ship->cap_cargo) {
      dialogue_alert( _("You can't unequip your ship when you have more cargo than it can hold without modifications!") );
      return;
   }
   if (dialogue_YesNo(_("Unequip Ship"), /* confirm */
         _("Are you sure you want to remove all equipment from your ship?"))==0)
      return;
   if (pilot_hasDeployed( ship )) {
      if (!dialogue_YesNo(_("Recall Fighters"), _("This action will recall your deployed fighters. Is that OK?")))
         return;
      /* Recall fighters. */
      escort_clearDeployed( ship );
   }

   /* Remove all outfits. */
   for (int i=0; i<array_size(ship->outfits); i++) {
      int ret;
      PilotOutfitSlot *s = ship->outfits[i];
      const Outfit *o = s->outfit;

      /* Skip null outfits. */
      if (o==NULL)
         continue;

      /* Ignore locked slots. */
      if (s->sslot->locked)
         continue;

      /* Remove ammo first. */
      pilot_rmAmmo( ship, s, pilot_maxAmmoO(ship, o) );

      /* Remove rest. */
      ret = pilot_rmOutfitRaw( ship, s );
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
   if (ship->autoweap) {
      pilot_weaponAuto( ship );
      ws_copy( eq_wgt.selected->weapon_sets, ship->weapon_sets );
   }
   pilot_weaponSafe( ship );

   /* Notify GUI of modification. */
   gui_setShip();
}

/**
 * @brief Does the autoequip magic on the player's ship.
 */
static void equipment_autoequipShip( unsigned int wid, const char* str )
{
   (void) str;
   (void) wid;
   Pilot *ship;
   int doswap;
   const char *curship;
   const char *file = AUTOEQUIP_PATH;

   ship = eq_wgt.selected->p;

   /* Swap ship if necessary. */
   doswap = (ship != player.p);
   if (doswap) {
      curship = player.p->name;
      player_swapShip( ship->name, 0 );
   }

   /* Create the environment */
   if (autoequip_env == LUA_NOREF) {
      /* Read File. */
      size_t bufsize;
      char *buf = ndata_read( file, &bufsize );
      if (buf == NULL) {
         WARN( _("File '%s' not found!"), file );
         return;
      }
      /* New env. */
      autoequip_env = nlua_newEnv();
      nlua_loadStandard( autoequip_env );
      nlua_loadTk( autoequip_env );
      if (nlua_dobufenv(autoequip_env, buf, bufsize, file) != 0) {
         WARN(_("Failed to run '%s': %s"), file, lua_tostring(naevL,-1));
         free(buf);
         lua_pop(naevL,1);
         goto autoequip_cleanup;
      }

      free(buf);
   }

   /* Run func. */
   nlua_getenv( naevL, autoequip_env, "autoequip" );
   if (!lua_isfunction(naevL,-1)) {
      WARN(_("'%s' doesn't have valid required 'autoequip' function!"), file);
      lua_pop(naevL,1);
      goto autoequip_cleanup;
   }
   lua_pushpilot(naevL,player.p->id);
   if (nlua_pcall( autoequip_env, 1, 0 )) {
      WARN(_("'%s' failed to run required 'autoequip' function: %s"), file, lua_tostring(naevL,-1));
      lua_pop(naevL,1);
      goto autoequip_cleanup;
   }

   hooks_run( "equip" ); /* Equipped. */

   /* Clean up. */
autoequip_cleanup:
   if (doswap) {
      player_swapShip( curship, 0 );
      toolkit_setImageArray( equipment_wid, EQUIPMENT_SHIPS, ship->name );
      equipment_updateShips( equipment_wid, NULL );
   }
}

/**
 * @brief Player tries to sell a ship.
 *    @param wid Window player is selling ships in.
 *    @param str Unused.
 */
static void equipment_sellShip( unsigned int wid, const char* str )
{
   (void) str;
   char buf[ECON_CRED_STRLEN], *name;
   credits_t price;
   PlayerShip_t *ps;
   Pilot *p;
   const Ship *s;
   HookParam hparam[3];
   const char *shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );

   if (!equipment_canSellPlayerShip( shipname )) {
      land_errDisplay();
      return;
   }

   /* Calculate price. */
   price = player_shipPrice(shipname,0);
   credits2str( buf, price, 2 );

   /* Check if player really wants to sell. */
   if (!dialogue_YesNo( _("Sell Ship"),
            _("Are you sure you want to sell your ship %s for %s?"),
            shipname, buf))
      return;

   /* Check if deployed and undeploy. */
   ps = player_getPlayerShip( shipname );
   if (ps->deployed) {
      if (pfleet_toggleDeploy( ps, 0 ))
         return;
   }

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
   hparam[0].type    = HOOK_PARAM_SHIP;
   hparam[0].u.ship  = s;
   hparam[1].type    = HOOK_PARAM_STRING;
   hparam[1].u.str   = name;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "ship_sell", hparam );
   land_needsTakeoff( 1 );
   free(name);
}

/**
 * @brief Renames the selected ship.
 *
 *    @param wid Parent window id.
 *    @param str Unused.
 */
static void equipment_renameShip( unsigned int wid, const char *str )
{
   (void) str;
   const char *shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   Pilot *ship = player_getShip(shipname);
   char *newname = dialogue_input( _("Ship Name"), 1, 60,
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
 * @brief Toggles the ship visualization mode.
 */
static void equipment_shipMode( unsigned int wid, const char *str )
{
   ship_mode = 1-ship_mode;
   equipment_updateShips( wid, str );
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
   for (int i=0; i<OUTFIT_TABS; i++)
      array_free( iar_outfits[i] );
   memset( iar_outfits, 0, sizeof(Outfit**) * OUTFIT_TABS );

   equipment_slotDeselect( &eq_wgt );
}

/**
 * @brief Deselects equipment stuff.
 */
void equipment_slotDeselect( CstSlotWidget *wgt )
{
   if (wgt==NULL)
      wgt = &eq_wgt;
   /* Safe defaults. */
   memset( wgt, 0, sizeof(CstSlotWidget) );
   wgt->slot      = -1;
   wgt->mouseover = -1;
   wgt->weapons   = -1;
}
