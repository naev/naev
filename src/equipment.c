/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file equipment.c
 *
 * @brief Handles equipping ships.
 */


#include "equipment.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "log.h"
#include "land.h"
#include "toolkit.h"
#include "dialogue.h"
#include "player.h"
#include "mission.h"
#include "ntime.h"
#include "opengl_vbo.h"
#include "conf.h"
#include "gui.h"
#include "tk/toolkit_priv.h" /* Yes, I'm a bad person, abstractions be damned! */


/* global/main window */
#define BUTTON_WIDTH 200 /**< Default button width. */
#define BUTTON_HEIGHT 40 /**< Default button height. */

#define SHIP_ALT_MAX 256 /**< Maximum ship alt text. */


/*
 * equipment stuff
 */
static CstSlotWidget eq_wgt; /**< Equipment widget. */
static double equipment_dir      = 0.; /**< Equipment dir. */
static unsigned int equipment_lastick = 0; /**< Last tick. */
static gl_vbo *equipment_vbo     = NULL; /**< The VBO. */


/*
 * prototypes
 */
static void equipment_getDim( unsigned int wid, int *w, int *h,
      int *sw, int *sh, int *ow, int *oh,
      int *ew, int *eh,
      int *cw, int *ch, int *bw, int *bh );
/* Widget. */
static void equipment_genLists( unsigned int wid );
static void equipment_renderColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, const char *txt,
      int selected, Outfit *o, Pilot *p );
static void equipment_renderSlots( double bx, double by, double bw, double bh, void *data );
static void equipment_renderMisc( double bx, double by, double bw, double bh, void *data );
static void equipment_renderOverlayColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, int mover, CstSlotWidget *wgt );
static void equipment_renderOverlaySlots( double bx, double by, double bw, double bh,
      void *data );
static void equipment_renderShip( double bx, double by,
      double bw, double bh, double x, double y, Pilot *p );
static int equipment_mouseColumn( double y, double h, int n, double my );
static void equipment_mouseSlots( unsigned int wid, SDL_Event* event,
      double x, double y, double w, double h, void *data );
/* Misc. */
static int equipment_swapSlot( unsigned int wid, Pilot *p, PilotOutfitSlot *slot );
static void equipment_sellShip( unsigned int wid, char* str );
static void equipment_transChangeShip( unsigned int wid, char* str );
static void equipment_changeShip( unsigned int wid );
static void equipment_transportShip( unsigned int wid );
static void equipment_unequipShip( unsigned int wid, char* str );
static unsigned int equipment_transportPrice( char *shipname );
static void equipment_rightClickOutfits( unsigned int wid, char* str );


/**
 * @brief Handles right-click on unequipped outfit.
 *    @param wid Window to update.
 *    @param str Widget name. Must be EQUIPMENT_OUTFITS.
 *    @param clicked_outfit Name of the outfit the user right-clicked on.
 */
void equipment_rightClickOutfits( unsigned int wid, char* str )
{
   (void)str;
   Outfit* o;
   int i;
   int outfit_n;
   PilotOutfitSlot* slots;
   Pilot *p;
   const char* clicked_outfit = toolkit_getImageArray( wid, EQUIPMENT_OUTFITS );

   /* Did the user click on background? */
   if (clicked_outfit == NULL)
      return;

   o = outfit_get(clicked_outfit);
   if (o == NULL)
      return;

   /* Figure out which slot this stuff fits into */
   switch (o->slot.type) {
      case OUTFIT_SLOT_LOW:
         outfit_n = eq_wgt.selected->outfit_nlow;
         slots    = eq_wgt.selected->outfit_low;
         break;
      case OUTFIT_SLOT_MEDIUM:
         outfit_n = eq_wgt.selected->outfit_nmedium;
         slots    = eq_wgt.selected->outfit_medium;
         break;
      case OUTFIT_SLOT_HIGH:
         outfit_n = eq_wgt.selected->outfit_nhigh;
         slots    = eq_wgt.selected->outfit_high;
         break;
      default:
         return;
   }

   /* Loop through outfit slots of the right type, try to find an empty one */
   for (i=0; i < outfit_n; i++) {

      /* Slot full. */
      if (slots[i].outfit != NULL)
         continue;

      /* Must fit the slot. */
      if (!outfit_fitsSlot( o, &slots[i].slot))
         continue;

      /* Bingo! */
      eq_wgt.outfit  = o;
      p              = eq_wgt.selected;
      equipment_swapSlot( wid, p, &slots[i] );
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
   /* Get window dimensions. */
   window_dimWindow( wid, w, h );

   /* Calculate image array dimensions. */
   if (sw != NULL)
      *sw = 200 + (*w-800);
   if (sh != NULL)
      *sh = (*h - 100)/2;
   if (ow != NULL)
      *ow = (sw!=NULL) ? *sw : 0;
   if (oh != NULL)
      *oh = (sh!=NULL) ? *sh : 0;

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
      *bw = (*w - 20 - (sw!=NULL?*sw:0) - 40 - 20 - 60) / 4;
   if (bh != NULL)
      *bh = BUTTON_HEIGHT;
}

/**
 * @brief Opens the player's equipment window.
 */
void equipment_open( unsigned int wid )
{
   int i;
   int w, h;
   int sw, sh;
   int ow, oh;
   int bw, bh;
   int ew, eh;
   int cw, ch;
   int x, y;
   GLfloat colour[4*4];
   const char *buf;

   /* Create the vbo if necessary. */
   if (equipment_vbo == NULL) {
      equipment_vbo = gl_vboCreateStream( (sizeof(GLint)*2 + sizeof(GLfloat)*4)*4, NULL );
      for (i=0; i<4; i++) {
         colour[i*4+0] = cRadar_player.r;
         colour[i*4+1] = cRadar_player.g;
         colour[i*4+2] = cRadar_player.b;
         colour[i*4+3] = cRadar_player.a;
      }
      gl_vboSubData( equipment_vbo, sizeof(GLint)*2*4, sizeof(colour), colour );
   }

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, &sw, &sh, &ow, &oh,
         &ew, &eh, &cw, &ch, &bw, &bh );

   /* Sane defaults. */
   equipment_lastick    = SDL_GetTicks();
   equipment_dir        = 0.;
   eq_wgt.selected      = NULL;

   /* Add ammo. */
   equipment_addAmmo();

   /* buttons */
   window_addButton( wid, -20, 20,
         bw, bh, "btnCloseEquipment",
         "Takeoff", land_buttonTakeoff );
   window_addButton( wid, -20 - (20+bw), 20,
         bw, bh, "btnSellShip",
         "Sell Ship", equipment_sellShip );
   window_addButton( wid, -20 - (20+bw)*2, 20,
         bw, bh, "btnChangeShip",
         "Swap Ship", equipment_transChangeShip );
   window_addButton( wid, -20 - (20+bw)*3, 20,
         bw, bh, "btnUnequipShip",
         "Unequip", equipment_unequipShip );

   /* text */
   buf = "Name:\n"
      "Model:\n"
      "Class:\n"
      "Value:\n"
      "\n"
      "Mass:\n"
      "Jump Time:\n"
      "Thrust:\n"
      "Speed:\n"
      "Turn:\n"
      "\n"
      "Shield:\n"
      "Armour:\n"
      "Energy:\n"
      "Cargo Space:\n"
      "Fuel:\n"
      "\n"
      "Transportation:\n"
      "Where:";
   x = 20 + sw + 20 + 180 + 20 + 30;
   y = -190;
   window_addText( wid, x, y,
         100, h+y, 0, "txtSDesc", &gl_smallFont, &cDConsole, buf );
   x += 100;
   window_addText( wid, x, y,
         w - x - 20, h+y, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );

   /* Generate lists. */
   window_addText( wid, 30, -20,
         130, 200, 0, "txtShipTitle", &gl_smallFont, &cBlack, "Available Ships" );
   window_addText( wid, 30, -40-sh-20,
         130, 200, 0, "txtOutfitTitle", &gl_smallFont, &cBlack, "Available Outfits" );
   equipment_genLists( wid );

   /* Seperator. */
   window_addRect( wid, 20 + sw + 20, -40, 2, h-60, "rctDivider", &cGrey50, 0 );

   /* Slot widget. */
   equipment_slotWidget( wid, 20 + sw + 40, -40, ew, eh, &eq_wgt );
   eq_wgt.canmodify = 1;

   /* Custom widget (ship information). */
   window_addCust( wid, 20 + sw + 40 + ew + 20, -40, cw, ch, "cstMisc", 0,
         equipment_renderMisc, NULL, NULL );
}
void equipment_slotWidget( unsigned int wid,
      double x, double y, double w, double h,
      CstSlotWidget *data )
{
   /* Initialize data. */
   data->selected = NULL;
   data->outfit   = NULL;
   data->slot     = -1;
   data->mouseover = -1;
   data->altx     = 0.;
   data->alty     = 0.;

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
      int selected, Outfit *o, Pilot *p )
{
   int i;
   glColour *c, *dc, bc;

   /* Render text. */
   if ((o != NULL) && (lst[0].slot.type == o->slot.type)) 
      c = &cDConsole;
   else
      c = &cBlack;
   gl_printMidRaw( &gl_smallFont, w+10.,
         x + SCREEN_W/2.-5., y+h+10. + SCREEN_H/2.,
         c, txt );

   /* Iterate for all the slots. */
   for (i=0; i<n; i++) {
      /* Choose colours based on size. */
      if (i==selected) {
         c  = &cGrey80;
         dc = outfit_slotSizeColour( &lst[i].slot );
         if (dc == NULL)
            dc = &cGrey60;
      }
      else {
         c  = toolkit_col;
         dc = outfit_slotSizeColour( &lst[i].slot );
         if (dc == NULL)
            dc = toolkit_colDark;
      }

      /* Draw background. */
      memcpy( &bc, dc, sizeof(bc) );
      bc.a = 0.4;
      if (i==selected)
         c = &cDConsole;
      else
         c = &bc;
      toolkit_drawRect( x, y, w, h, c, NULL );

      if (lst[i].outfit != NULL) {
         /* Draw bugger. */
         gl_blitScale( lst[i].outfit->gfx_store,
               x + SCREEN_W/2., y + SCREEN_H/2., w, h, NULL );
         c = &cBlack; /* Ensures nice uniform outlines. */
      }
      else {
         if ((o != NULL) &&
               (lst[i].slot.type == o->slot.type)) {
            if (pilot_canEquip( p, &lst[i], o, 1 ) != NULL)
               c = &cRed;
            else
               c = &cDConsole;
         }
         else
            c = &cBlack;
         gl_printMidRaw( &gl_smallFont, w,
               x + SCREEN_W/2., y + (h-gl_smallFont.h)/2 + SCREEN_H/2., c, "None" );
      }

      /* Draw outline. */
      toolkit_drawOutlineThick( x, y, w, h, 1, 3, dc, NULL );
      toolkit_drawOutline( x-1, y-1, w+3, h+3, 0, c, c );
      /* Go to next one. */
      y -= h+20;
   }
}
static void equipment_calculateSlots( Pilot *p, double bw, double bh,
      double *w, double *h, int *n, int *m )
{
   double tw, th, s;
   int tm;

   /* Calculate size. */
   tm = MAX( MAX( p->outfit_nhigh, p->outfit_nmedium ), p->outfit_nlow );
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
 */
static void equipment_renderSlots( double bx, double by, double bw, double bh, void *data )
{
   double x, y;
   double w, h;
   double tw, th;
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
   th = bh / (double)m;

   /* Draw high outfits. */
   x  = bx + (tw-w)/2;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nhigh, p->outfit_high, "High", 
         selected, wgt->outfit, wgt->selected );

   /* Draw medium outfits. */
   selected -= p->outfit_nhigh;
   x += tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nmedium, p->outfit_medium, "Medium",
         selected, wgt->outfit, wgt->selected );

   /* Draw low outfits. */
   selected -= p->outfit_nmedium;
   x += tw;
   y  = by + bh - (h+20) + (h+20-h)/2;
   equipment_renderColumn( x, y, w, h,
         p->outfit_nlow, p->outfit_low, "Low",
         selected, wgt->outfit, wgt->selected );
}
/**
 * @brief Renders the custom equipment widget.
 */
static void equipment_renderMisc( double bx, double by, double bw, double bh, void *data )
{
   (void) data;
   Pilot *p;
   double percent;
   double x, y;
   double w, h;
   glColour *lc, *c, *dc;

   /* Must have selected ship. */
   if (eq_wgt.selected == NULL)
      return;

   p = eq_wgt.selected;

   /* Render CPU and energy bars. */
   lc = &cWhite;
   c = &cGrey80;
   dc = &cGrey60;
   w = 30;
   h = 70;
   x = bx + (40-w)/2 + 10;
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
         x - 20 + SCREEN_W/2., y - 10 - gl_smallFont.h + SCREEN_H/2.,
         &cBlack, "%.0f / %.0f", p->cpu, p->cpu_max );

   /* Render ship graphic. */
   equipment_renderShip( bx, by, bw, bh, x, y, p );
}


/**
 * @brief Renders an outfit column.
 * @param mover Slot for which mouseover is active
 */
static void equipment_renderOverlayColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, int mover, CstSlotWidget *wgt )
{
   int i;
   glColour *c, tc;
   int text_width, xoff, yoff, top;
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
               display = pilot_canEquip( wgt->selected, &lst[i], lst[i].outfit, 0 );
               if (display != NULL)
                  c = &cRed;
               else {
                  display = "Right click to remove";
                  c = &cDConsole;
               }
            }
            else if ((wgt->outfit != NULL) &&
                  (lst->slot.type == wgt->outfit->slot.type)) {
               top = 0;
               display = pilot_canEquip( wgt->selected, &lst[i], wgt->outfit, 1 );
               if (display != NULL)
                  c = &cRed;
               else {
                  display = "Right click to add";
                  c = &cDConsole;
               }
            }
         }
         else if (lst[i].outfit != NULL) {
            top = 1;
            if (outfit_isLauncher(lst[i].outfit) ||
                  (outfit_isFighterBay(lst[i].outfit))) {
               if ((lst[i].u.ammo.outfit == NULL) ||
                     (lst[i].u.ammo.quantity == 0)) {
                  if (outfit_isFighterBay(lst[i].outfit))
                     display = "Bay empty";
                  else
                     display = "Out of ammo";
                  c = &cRed;
               }
               else if (lst[i].u.ammo.quantity + lst[i].u.ammo.deployed <
                     outfit_amount(lst[i].outfit)) {
                  if (outfit_isFighterBay(lst[i].outfit))
                     display = "Bay low";
                  else
                     display = "Low ammo";
                  c = &cYellow;
               }
            }
         }

         if (display != NULL) {
            text_width = gl_printWidthRaw( &gl_smallFont, display );
            xoff = -(text_width - w)/2;
            if (top)
               yoff = h + 2;
            else
               yoff = -gl_smallFont.h - 3;
            tc.r = 1.;
            tc.g = 1.;
            tc.b = 1.;
            tc.a = 0.5;
            toolkit_drawRect( x+xoff-5, y -3. + yoff,
                  text_width+10, gl_smallFont.h+5,
                  &tc, NULL );
            gl_printMaxRaw( &gl_smallFont, text_width,
                  x+xoff + SCREEN_W/2., y + SCREEN_H/2. + yoff,
                  c, display );
         }
      }
      /* Go to next one. */
      y -= h+20;
   }
}
/**
 * @brief Renders the equipment overlay.
 */
static void equipment_renderOverlaySlots( double bx, double by, double bw, double bh,
      void *data )
{
   (void) bw;
   Pilot *p;
   int mover;
   double x, y;
   double w, h;
   double tw, th;
   int n, m;
   PilotOutfitSlot *slot;
   char alt[512];
   int pos;
   Outfit *o;
   CstSlotWidget *wgt;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   p   = wgt->selected;

   /* Must have selected ship. */
   if (wgt->selected != NULL) {

      /* Get dimensions. */
      equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
      tw = bw / (double)n;
      th = bh / (double)m;

      /* Get selected. */
      mover    = wgt->mouseover;

      /* Render high outfits. */
      x  = bx + (tw-w)/2;
      y  = by + bh - (h+20) + (h+20-h)/2;
      equipment_renderOverlayColumn( x, y, w, h,
            p->outfit_nhigh, p->outfit_high, mover, wgt );
      mover    -= p->outfit_nhigh;
      x += tw;
      y  = by + bh - (h+20) + (h+20-h)/2;
      equipment_renderOverlayColumn( x, y, w, h,
            p->outfit_nmedium, p->outfit_medium, mover, wgt );
      mover    -= p->outfit_nmedium;
      x += tw;
      y  = by + bh - (h+20) + (h+20-h)/2;
      equipment_renderOverlayColumn( x, y, w, h,
            p->outfit_nlow, p->outfit_low, mover, wgt );
   }

   /* Mouse must be over something. */
   if (wgt->mouseover < 0)
      return;

   /* Get the slot. */
   p = wgt->selected;
   if (wgt->mouseover < p->outfit_nhigh) {
      slot = &p->outfit_high[wgt->mouseover];
   }
   else if (wgt->mouseover < p->outfit_nhigh + p->outfit_nmedium) {
      slot = &p->outfit_medium[ wgt->mouseover - p->outfit_nhigh ];
   }
   else {
      slot = &p->outfit_low[ wgt->mouseover -
         p->outfit_nhigh - p->outfit_nmedium ];
   }

   /* For comfortability. */
   o = slot->outfit;

   /* Slot is empty. */
   if (o == NULL)
      return;

   /* Get text. */
   if (o->desc_short == NULL)
      return;
   pos = snprintf( alt, sizeof(alt),
         "%s\n"
         "\n"
         "%s",
         o->name,
         o->desc_short );
   if (o->mass > 0.)
      pos += snprintf( &alt[pos], sizeof(alt)-pos,
            "\n%.0f Tons",
            o->mass );

   /* Draw the text. */
   toolkit_drawAltText( bx + wgt->altx, by + wgt->alty, alt );
}


/**
 * @brief Renders the ship in the equipment window.
 */
static void equipment_renderShip( double bx, double by,
      double bw, double bh, double x, double y, Pilot* p )
{
   int sx, sy;
   glColour *lc, *c, *dc;
   unsigned int tick;
   double dt;
   double px, py;
   double pw, ph;
   double w, h;
   Vector2d v;
   GLint vertex[2*4];

   tick = SDL_GetTicks();
   dt   = (double)(tick - equipment_lastick)/1000.;
   equipment_lastick = tick;
   equipment_dir += p->turn * M_PI/180. * dt;
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
         px + SCREEN_W/2, py + SCREEN_H/2, sx, sy, pw, ph, NULL );
   if ((eq_wgt.slot >= 0) && (eq_wgt.slot < p->outfit_nhigh)) {
      p->tsx = sx;
      p->tsy = sy;
      pilot_getMount( p, &p->outfit_high[eq_wgt.slot], &v );
      px += pw/2;
      py += ph/2;
      v.x *= pw / p->ship->gfx_space->sw;
      v.y *= ph / p->ship->gfx_space->sh;
      /* Render it. */
      vertex[0] = px + v.x + 0.;
      vertex[1] = py + v.y - 7;
      vertex[2] = vertex[0];
      vertex[3] = py + v.y + 7;
      vertex[4] = px + v.x - 7.;
      vertex[5] = py + v.y + 0.;
      vertex[6] = px + v.x + 7.;
      vertex[7] = vertex[5];
      glLineWidth( 3. );
      gl_vboSubData( equipment_vbo, 0, sizeof(vertex), vertex );
      gl_vboActivateOffset( equipment_vbo, GL_VERTEX_ARRAY, 0, 2, GL_INT, 0 );
      gl_vboActivateOffset( equipment_vbo, GL_COLOR_ARRAY,
            sizeof(vertex), 4, GL_FLOAT, 0 );
      glDrawArrays( GL_LINES, 0, 4 );
      gl_vboDeactivate();
      glLineWidth( 1. );
   }
   lc = toolkit_colLight;
   c  = toolkit_col;
   dc = toolkit_colDark;
   toolkit_drawOutline( x - 5., y-4., w+8., h+2., 1., lc, c  );
   toolkit_drawOutline( x - 5., y-4., w+8., h+2., 2., dc, NULL  );
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
static void equipment_mouseSlots( unsigned int wid, SDL_Event* event,
      double mx, double my, double bw, double bh, void *data )
{
   (void) bw;
   Pilot *p;
   int selected, ret;
   double x, y;
   double w, h;
   double tw, th;
   CstSlotWidget *wgt;
   int n, m;

   /* Get data. */
   wgt = (CstSlotWidget*) data;
   p   = wgt->selected;

   /* Must have selected ship. */
   if (p == NULL)
      return;

   /* Must be left click for now. */
   if ((event->type != SDL_MOUSEBUTTONDOWN) &&
         (event->type != SDL_MOUSEMOTION))
      return;

   /* Get dimensions. */
   equipment_calculateSlots( p, bw, bh, &w, &h, &n, &m );
   tw = bw / (double)n;
   th = bh / (double)m;

   /* Render high outfits. */
   selected = 0;
   x  = (tw-w)/2;
   y  = bh - (h+20) + (h+20-h)/2 - 10;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( y, h, p->outfit_nhigh, my );
      if (ret >= 0) {
         if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT)
               wgt->slot = selected + ret;
            else if ((event->button.button == SDL_BUTTON_RIGHT) &&
                  wgt->canmodify)
               equipment_swapSlot( wid, p, &p->outfit_high[ret] );
         }
         else {
            wgt->mouseover  = selected + ret;
            wgt->altx       = mx;
            wgt->alty       = my;
         }
         return;
      }
   }
   selected += p->outfit_nhigh;
   x += tw;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( y, h, p->outfit_nmedium, my );
      if (ret >= 0) {
         if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT)
               wgt->slot = selected + ret;
            else if ((event->button.button == SDL_BUTTON_RIGHT) &&
                  wgt->canmodify)
               equipment_swapSlot( wid, p, &p->outfit_medium[ret] );
         }
         else {
            wgt->mouseover = selected + ret;
            wgt->altx      = mx;
            wgt->alty      = my;
         }
         return;
      }
   }
   selected += p->outfit_nmedium;
   x += tw;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( y, h, p->outfit_nlow, my );
      if (ret >= 0) {
         if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT)
               wgt->slot = selected + ret;
            else if ((event->button.button == SDL_BUTTON_RIGHT) &&
                  wgt->canmodify)
               equipment_swapSlot( wid, p, &p->outfit_low[ret] );
         }
         else {
            wgt->mouseover = selected + ret;
            wgt->altx      = mx;
            wgt->alty      = my;
         }
         return;
      }
   }

   /* Not over anything. */
   wgt->mouseover = -1;
}


/**
 * @brief Swaps an equipment slot.
 */
static int equipment_swapSlot( unsigned int wid, Pilot *p, PilotOutfitSlot *slot )
{
   int ret;
   Outfit *o, *ammo;
   int q;
   double f;

   /* Remove outfit. */
   if (slot->outfit != NULL) {
      o = slot->outfit;

      /* Must be able to remove. */
      if (pilot_canEquip( eq_wgt.selected, slot, o, 0 ) != NULL)
         return 0;

      /* Remove ammo first. */
      if (outfit_isLauncher(o) || (outfit_isFighterBay(o))) {
         ammo = slot->u.ammo.outfit;
         q    = pilot_rmAmmo( eq_wgt.selected, slot, slot->u.ammo.quantity );
         if (q > 0)
            player_addOutfit( ammo, q );
      }

      /* Handle possible fuel changes. */
      f = eq_wgt.selected->fuel;

      /* Remove outfit. */
      ret = pilot_rmOutfit( eq_wgt.selected, slot );
      if (ret == 0)
         player_addOutfit( o, 1 );

      /* Don't "gain" fuel. */
      eq_wgt.selected->fuel = MIN( eq_wgt.selected->fuel_max, f );
   }
   /* Add outfit. */
   else {
      /* Must have outfit. */
      o = eq_wgt.outfit;
      if (o==NULL)
         return 0;

      /* Must fit slot. */
      if (!outfit_fitsSlot( o, &slot->slot ))
         return 0;

      /* Must be able to add. */
      if (pilot_canEquip( eq_wgt.selected, NULL, o, 1 ) != NULL)
         return 0;

      /* Add outfit to ship. */
      ret = player_rmOutfit( o, 1 );
      if (ret == 1) {
         /* Handle possible fuel changes. */
         f = eq_wgt.selected->fuel;

         /* Add the outfit. */
         pilot_addOutfit( eq_wgt.selected, o, slot );

         /* Don't "gain" fuel. */
         eq_wgt.selected->fuel = MIN( eq_wgt.selected->fuel_max, f );
      }

      equipment_addAmmo();
   }

   /* Recalculate stats. */
   pilot_calcStats( p );

   /* Redo the outfits thingy. */
   equipment_regenLists( wid, 1, 1 );

   return 0;
}


/**
 * @brief Regenerates the equipment window lists.
 */
void equipment_regenLists( unsigned int wid, int outfits, int ships )
{
   int ret;
   int nout, nship;
   double offout, offship;
   char *s, selship[PATH_MAX];

   /* Save positions. */
   if (outfits) {
      nout    = toolkit_getImageArrayPos( wid, EQUIPMENT_OUTFITS );
      offout  = toolkit_getImageArrayOffset( wid, EQUIPMENT_OUTFITS );
      window_destroyWidget( wid, EQUIPMENT_OUTFITS );
   }
   if (ships) {
      nship   = toolkit_getImageArrayPos( wid, EQUIPMENT_SHIPS );
      offship = toolkit_getImageArrayOffset( wid, EQUIPMENT_SHIPS );
      s       = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
      strncpy( selship, s, sizeof(selship) );
      window_destroyWidget( wid, EQUIPMENT_SHIPS );
   }

   /* Regenerate lists. */
   equipment_genLists( wid );

   /* Restore positions. */
   if (outfits) {
      toolkit_setImageArrayPos( wid, EQUIPMENT_OUTFITS, nout );
      toolkit_setImageArrayOffset( wid, EQUIPMENT_OUTFITS, offout );
   }
   if (ships) {
      toolkit_setImageArrayPos( wid, EQUIPMENT_SHIPS, nship );
      toolkit_setImageArrayOffset( wid, EQUIPMENT_SHIPS, offship );
      /* Try to maintain same ship selected. */
      s = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
      if ((s != NULL) && (selship != NULL) && (strcmp(s,selship)!=0)) {
         ret = toolkit_setImageArray( wid, EQUIPMENT_SHIPS, selship );
         if (ret != 0) /* Failed to maintain. */
            toolkit_setImageArrayPos( wid, EQUIPMENT_SHIPS, nship );

         /* Update ships. */
         equipment_updateShips( wid, NULL );
      }
   }
}


/**
 * @brief Adds all the ammo it can to the player.
 */
void equipment_addAmmo (void)
{
   int i;
   Outfit *o, *ammo;
   int q;
   Pilot *p;

   /* Get player. */
   if (eq_wgt.selected == NULL)
      p = player.p;
   else
      p = eq_wgt.selected;

   /* Add ammo to all outfits. */
   for (i=0; i<p->noutfits; i++) {
      o = p->outfits[i]->outfit;

      /* Must be valid outfit. */
      if (o == NULL)
         continue;

      /* Add ammo if able to. */
      ammo = outfit_ammo(o);
      if (ammo == NULL)
         continue;
      q    = player_outfitOwned(ammo);

      /* Add ammo. */
      q = pilot_addAmmo( p, p->outfits[i], ammo, q );

      /* Remove from player. */
      player_rmOutfit( ammo, q );
   }
}


/**
 * @brief Generates a new ship/outfit lists if needed.
 */
static void equipment_genLists( unsigned int wid )
{
   int i, j, l, p;
   char **sships;
   glTexture **tships;
   int nships;
   char **soutfits;
   glTexture **toutfits;
   int noutfits;
   int w, h;
   int sw, sh;
   int ow, oh;
   char **alt;
   char **quantity;
   Outfit *o;
   Pilot *s;
   double mod_energy, mod_damage, mod_shots;
   double eps, dps, shots;
   glColour *bg, *c;

   /* Get dimensions. */
   equipment_getDim( wid, &w, &h, &sw, &sh, &ow, &oh,
         NULL, NULL, NULL, NULL, NULL, NULL );

   /* Ship list. */
   if (!widget_exists( wid, EQUIPMENT_SHIPS )) {
      eq_wgt.selected = NULL;
      if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         nships   = player_nships()+1;
      else
         nships   = 1;
      sships   = malloc(sizeof(char*)*nships);
      tships   = malloc(sizeof(glTexture*)*nships);
      /* Add player's current ship. */
      sships[0] = strdup(player.p->name);
      tships[0] = player.p->ship->gfx_store;
      if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         player_ships( &sships[1], &tships[1] );
      window_addImageArray( wid, 20, -40,
            sw, sh, EQUIPMENT_SHIPS, 64./96.*128., 64.,
            tships, sships, nships, equipment_updateShips, NULL );

      /* Ship stats in alt text. */
      alt   = malloc( sizeof(char*) * nships );
      for (i=0; i<nships; i++) {
         s  = player_getShip( sships[i]);
         alt[i] = malloc( SHIP_ALT_MAX );
         dps = 0.;
         eps = 0.;
         for (j=0; j<s->noutfits; j++) {
            o = s->outfits[j]->outfit;
            if (o==NULL)
               continue;
            switch (o->type) {
               case OUTFIT_TYPE_BOLT:
                  mod_energy = s->stats.energy_forward;
                  mod_damage = s->stats.damage_forward;
                  mod_shots  = 2. - s->stats.firerate_forward;
                  break;
               case OUTFIT_TYPE_TURRET_BOLT:
                  mod_energy = s->stats.energy_turret;
                  mod_damage = s->stats.damage_turret;
                  mod_shots  = 2. - s->stats.firerate_turret;
                  break;
               default:
                  continue;
            }
            shots = 1. / (mod_shots * outfit_delay(o));
            dps  += shots * mod_damage * outfit_damage(o);
            eps  += shots * mod_energy * outfit_energy(o);
         }
         l  = snprintf( alt[i], SHIP_ALT_MAX, "Ship Stats" );
         p  = l;
         if (dps > 0.)
            l += snprintf( &alt[i][l], SHIP_ALT_MAX-l,
                  "\n%.2f DPS [%.2f EPS]", dps, eps );
         if (s->jam_chance > 0.)
            l += snprintf( &alt[i][l], SHIP_ALT_MAX-l,
                  "\n%.0f%% Jam [%.0f Range]",
                  s->jam_chance*100., s->jam_range );
         l += ship_statsDesc( &s->stats, &alt[i][l], SHIP_ALT_MAX-l, 1, 1 );
         if (p == l) {
            free( alt[i] );
            alt[i] = NULL;
         }
      }
      toolkit_setImageArrayAlt( wid, EQUIPMENT_SHIPS, alt );
   }

   /* Outfit list. */
   eq_wgt.outfit = NULL;
   noutfits = MAX(1,player_numOutfits());
   soutfits = malloc(sizeof(char*)*noutfits);
   toutfits = malloc(sizeof(glTexture*)*noutfits);
   bg       = malloc(sizeof(glColour)*noutfits);
   player_getOutfits( soutfits, toutfits );
   if (!widget_exists( wid ,EQUIPMENT_OUTFITS )) {
      window_addImageArray( wid, 20, -40 - sh - 40,
            sw, sh, EQUIPMENT_OUTFITS, 50., 50.,
            toutfits, soutfits, noutfits, equipment_updateOutfits, equipment_rightClickOutfits );

      /* Set alt text. */
      if (strcmp(soutfits[0],"None")!=0) {
         alt      = malloc( sizeof(char*) * noutfits );
         quantity = malloc( sizeof(char*) * noutfits );
         for (i=0; i<noutfits; i++) {
            o      = outfit_get( soutfits[i] );

            /* Background colour. */
            c = outfit_slotSizeColour( &o->slot );
            if (c == NULL)
               c = &cBlack;
            memcpy( &bg[i], c, sizeof(glColour) );
            bg[i].a = 0.5;

            /* Short description. */
            if (o->desc_short == NULL)
               alt[i] = NULL;
            else {
               l = strlen(o->desc_short) + 128;
               alt[i] = malloc( l );
               p = snprintf( alt[i], l,
                     "%s\n"
                     "\n"
                     "%s",
                     o->name,
                     o->desc_short );
               if (o->mass > 0.)
                  p += snprintf( &alt[i][p], l-p,
                        "\n%.0f Tons",
                        o->mass );
            }

            /* Quantity. */
            p = player_outfitOwned(o);
            l = p / 10 + 4;
            quantity[i] = malloc( l );
            snprintf( quantity[i], l, "%d", p );
         }
         toolkit_setImageArrayAlt( wid, EQUIPMENT_OUTFITS, alt );
         toolkit_setImageArrayQuantity( wid, EQUIPMENT_OUTFITS, quantity );
         toolkit_setImageArrayBackground( wid, EQUIPMENT_OUTFITS, bg );
      }
   }

   /* Update window. */
   equipment_updateOutfits(wid, NULL);
   equipment_updateShips(wid, NULL);
}
/**
 * @brief Updates the player's ship window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateShips( unsigned int wid, char* str )
{
   (void)str;
   char buf[512], sysname[128], buf2[32], buf3[32];
   char *shipname;
   Pilot *ship;
   char* loc;
   unsigned int price;
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
      loc     = "Onboard";
      price   = 0;
      onboard = 1;
      sysname[0] = '\0';
   }
   else {
      ship   = player_getShip( shipname );
      loc    = player_getLoc(ship->name);
      price  = equipment_transportPrice( shipname );
      onboard = 0;
      snprintf( sysname, sizeof(sysname), " in the %s system",
            planet_getSystem(loc) );
   }
   eq_wgt.selected = ship;

   /* update text */
   credits2str( buf2, price , 2 ); /* transport */
   credits2str( buf3, player_shipPrice(shipname), 2 ); /* sell price */
   cargo = pilot_cargoFree(ship) + pilot_cargoUsed(ship);
   snprintf( buf, sizeof(buf),
         "%s\n"
         "%s\n"
         "%s\n"
         "%s Credits\n"
         "\n"
         "%.0f Tons\n"
         "%.1f STU Average\n"
         "%.0f KN/Ton\n"
         "%.0f M/s\n"
         "%.0f Grad/s\n"
         "\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f MJ (%.1f MW)\n"
         "%.0f MJ (%.1f MW)\n"
         "%d / %d Tons\n"
         "%.0f / %.0f Units (%d Jumps)\n"
         "\n"
         "%s Credits\n"
         "%s%s",
         /* Generic. */
      ship->name,
      ship->ship->name,
      ship_class(ship->ship),
      buf3,
      /* Movement. */
      ship->solid->mass,
      pilot_hyperspaceDelay( ship ),
      ship->thrust/ship->solid->mass,
      ship->speed,
      ship->turn,
      /* Health. */
      ship->shield_max, ship->shield_regen,
      ship->armour_max, ship->armour_regen,
      ship->energy_max, ship->energy_regen,
      /* Misc. */
      pilot_cargoUsed(ship), cargo,
      ship->fuel, ship->fuel_max, pilot_getJumps(ship),
      /* Transportation. */
      buf2,
      loc, sysname );
   window_modifyText( wid, "txtDDesc", buf );

   /* button disabling */
   if (onboard) {
      window_disableButton( wid, "btnSellShip" );
      window_disableButton( wid, "btnChangeShip" );
   }
   else {
      if (strcmp(land_planet->name,loc)) { /* ship not here */
         window_buttonCaption( wid, "btnChangeShip", "Transport" );
         if (!player_hasCredits( price ))
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
}
/**
 * @brief Updates the player's ship window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateOutfits( unsigned int wid, char* str )
{
   (void) str;
   const char *oname;

   /* Must have outfit. */
   oname = toolkit_getImageArray( wid, EQUIPMENT_OUTFITS );
   if (strcmp(oname,"None")==0) {
      eq_wgt.outfit = NULL;
      return;
   }
   eq_wgt.outfit = outfit_get(oname);

   /* Also update ships. */
   equipment_updateShips(wid, NULL);
}

/**
 * @brief Changes or transport depending on what is active.
 *    @param wid Window player is attempting to change ships in.
 *    @param str Unused.
 */
static void equipment_transChangeShip( unsigned int wid, char* str )
{
   (void) str;
   char *shipname, *loc;
   Pilot *ship;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   ship  = player_getShip( shipname );
   loc   = player_getLoc( shipname );

   if (strcmp(land_planet->name,loc)) /* ship not here */
      equipment_transportShip( wid );
   else
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
   char *shipname;
   Pilot *newship;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   newship = player_getShip(shipname);

   if (error_dialogue( shipname, "swapEquipment" ))
      return;

   /* Swap ship. */
   player_swapShip(shipname);

   /* Regenerate ship widget. */
   equipment_regenLists( wid, 0, 1 );
   /* Focus new ship. */
   toolkit_setImageArrayPos( wid, EQUIPMENT_SHIPS, 0 );
   toolkit_setImageArrayOffset( wid, EQUIPMENT_SHIPS, 0. );
}
/**
 * @brief Player attempts to transport his ship to the planet he is at.
 *    @param wid Window player is trying to transport his ship from.
 */
static void equipment_transportShip( unsigned int wid )
{
   unsigned int price;
   char *shipname, buf[32];

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   if (strcmp(shipname,"None")==0) { /* no ships */
      dialogue_alert( "You can't transport nothing here!" );
      return;
   }

   price = equipment_transportPrice( shipname );
   if (price==0) { /* already here */
      dialogue_alert( "Your ship '%s' is already here.", shipname );
      return;
   }
   else if (!player_hasCredits( price )) { /* not enough money. */
      credits2str( buf, price-player.p->credits, 2 );
      dialogue_alert( "You need %s more credits to transport '%s' here.",
            buf, shipname );
      return;
   }

   /* Obligatory annoying dialogue. */
   credits2str( buf, price, 2 );
   if (dialogue_YesNo("Are you sure?", /* confirm */
            "Do you really want to spend %s transporting your ship %s here?",
            buf, shipname )==0)
      return;

   /* success */
   player_modCredits( -price );
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
   Pilot *ship;
   Outfit *o, *ammo;
   double f;

   ship = eq_wgt.selected;

   /* Handle possible fuel changes. */
   f = eq_wgt.selected->fuel;

   /* Remove all outfits. */
   for (i=0; i<ship->noutfits; i++) {
      o = ship->outfits[i]->outfit;

      /* Skip null outfits. */
      if (o==NULL)
         continue;

      /* Remove ammo first. */
      ammo = outfit_ammo(o);
      if (ammo != NULL) {
         ret = pilot_rmAmmo( ship, ship->outfits[i], outfit_amount(o) );
         player_addOutfit( ammo, ret );
      }

      /* Remove rest. */
      ret = pilot_rmOutfitRaw( ship, ship->outfits[i] );
      if (ret==0)
         player_addOutfit( o, 1 );
   }

   /* Recalculate stats. */
   pilot_calcStats( ship );

   /* Don't "gain" fuel. */
   eq_wgt.selected->fuel = MIN( eq_wgt.selected->fuel_max, f );

   /* Regenerate list. */
   equipment_regenLists( wid, 1, 1 );
}
/**
 * @brief Player tries to sell a ship.
 *    @param wid Window player is selling ships in.
 *    @param str Unused.
 */
static void equipment_sellShip( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[32], *name;
   int price;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );

   if (error_dialogue( shipname, "sell" ))
      return;

   /* Calculate price. */
   price = player_shipPrice(shipname);
   credits2str( buf, price, 2 );

   /* Check if player really wants to sell. */
   if (!dialogue_YesNo( "Sell Ship",
            "Are you sure you want to sell your ship %s for %s credits?", shipname, buf)) {
      return;
   }

   /* Sold. */
   name = strdup(shipname);
   player_modCredits( price );
   land_checkAddRefuel();
   player_rmShip( shipname );

   /* Destroy widget - must be before widget. */
   equipment_regenLists( wid, 0, 1 );

   /* Display widget. */
   dialogue_msg( "Ship Sold", "You have sold your ship %s for %s credits.",
         name, buf );
   free(name);
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

   price = (unsigned int)(sqrt(ship->ship->mass)*5000.);

   return price;
}


/**
 * @brief Cleans up after the equipment stuff.
 */
void equipment_cleanup (void)
{
   /* Destroy the VBO. */
   if (equipment_vbo != NULL)
      gl_vboDestroy( equipment_vbo );
   equipment_vbo = NULL;
}

