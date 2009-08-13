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


/*
 * equipment stuff
 */
static Pilot *equipment_selected = NULL; /**< Selected pilot ship. */
static Outfit *equipment_outfit  = NULL; /**< Selected outfit. */
static int equipment_slot        = -1; /**< Selected equipment slot. */
static int equipment_mouseover   = -1; /**< Mouse over slot. */
static double equipment_dir      = 0.; /**< Equipment dir. */
static double equipment_altx     = 0; /**< Alt X text position. */
static double equipment_alty     = 0; /**< Alt Y text position. */
static unsigned int equipment_lastick = 0; /**< Last tick. */
static gl_vbo *equipment_vbo     = NULL; /**< The VBO. */


/*
 * prototypes
 */
static void equipment_renderColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, const char *txt, int selected );
static void equipment_render( double bx, double by, double bw, double bh );
static void equipment_renderOverlayColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, int mover );
static void equipment_renderOverlay( double bx, double by, double bw, double bh );
static void equipment_renderShip( double bx, double by,
      double bw, double bh, double x, double y, Pilot *p );
static int equipment_mouseColumn( double y, double h, int n, double my );
static void equipment_mouse( unsigned int wid, SDL_Event* event,
      double x, double y, double w, double h );
static const char* equipment_canSwap( PilotOutfitSlot *s, Outfit *o, int add );
static int equipment_swapSlot( unsigned int wid, PilotOutfitSlot *slot );
static void equipment_sellShip( unsigned int wid, char* str );
static void equipment_transChangeShip( unsigned int wid, char* str );
static void equipment_changeShip( unsigned int wid );
static void equipment_transportShip( unsigned int wid );
static void equipment_unequipShip( unsigned int wid, char* str );
static unsigned int equipment_transportPrice( char *shipname );


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
   int cw, ch;
   int x, y;
   GLfloat colour[4*4];
   const char *buf;

   /* Create the vbo if necessary. */
   if (equipment_vbo == NULL) {
      equipment_vbo = gl_vboCreateStream( sizeof(GLfloat) * (2+4)*4, NULL );
      for (i=0; i<4; i++) {
         colour[i*4+0] = cRadar_player.r;
         colour[i*4+1] = cRadar_player.g;
         colour[i*4+2] = cRadar_player.b;
         colour[i*4+3] = cRadar_player.a;
      }
      gl_vboSubData( equipment_vbo, sizeof(GLfloat) * 2*4,
            sizeof(GLfloat) * 4*4, colour );
   }

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
   equipment_selected   = NULL;
   equipment_outfit     = NULL;
   equipment_slot       = -1;
   equipment_mouseover  = -1;
   equipment_altx       = 0;
   equipment_alty       = 0;
   equipment_lastick    = SDL_GetTicks();
   equipment_dir        = 0.;

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
         "Swap Ships", equipment_transChangeShip );
   window_addButton( wid, -20 - (20+bw)*3, 20,
         bw, bh, "btnUnequipShip",
         "Unequip", equipment_unequipShip );

   /* text */
   buf = "Name:\n"
         "Ship:\n"
         "Class:\n"
         "Sell price:\n"
         "\n"
         "Mass:\n"
         "Thrust:\n"
         "Speed:\n"
         "Turn:\n"
         "\n"
         "Shield:\n"
         "Armour:\n"
         "Energy:\n"
         "\n"
         "Cargo:\n"
         "Fuel:\n"
         "\n"
         "Transportation:\n"
         "Where:";
   x = 20 + sw + 20 + 180 + 20 + 30;
   y = -210;
   window_addText( wid, x, y,
         100, h+y, 0, "txtSDesc", &gl_smallFont, &cDConsole, buf );
   x += 100;
   window_addText( wid, x, y,
         w - x - 20, h+y, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );

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
   window_custSetClipping( wid, "cstEquipment", 0 );
   window_custSetOverlay( wid, "cstEquipment", equipment_renderOverlay );
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
   gl_printMidRaw( &gl_smallFont, w+10.,
         x + SCREEN_W/2.-5., y+h+10. + SCREEN_H/2.,
         &cBlack, txt );

   /* Iterate for all the slots. */
   for (i=0; i<n; i++) {
      if (lst[i].outfit != NULL) {
         if (i==selected)
            c = &cDConsole;
         else
            c = &cBlack;
         /* Draw background. */
         toolkit_drawRect( x, y, w, h, c, NULL );
         /* Draw bugger. */
         gl_blitScale( lst[i].outfit->gfx_store,
               x + SCREEN_W/2., y + SCREEN_H/2., w, h, NULL );
      }
      else {
         if ((equipment_outfit != NULL) &&
               (lst[i].slot == equipment_outfit->slot)) {
            if (equipment_canSwap( NULL, equipment_outfit, 1 ) != NULL)
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
      toolkit_drawOutline( x, y, w, h, 1., lc, c  );
      toolkit_drawOutline( x, y, w, h, 2., dc, NULL  );
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
   h = 80;
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
         &cBlack, "%.0f / %.0f", p->cpu, p->cpu_max );

   /* Render ship graphic. */
   equipment_renderShip( bx, by, bw, bh, x, y, p );
}


/**
 * @brief Renders an outfit column.
 */
static void equipment_renderOverlayColumn( double x, double y, double w, double h,
      int n, PilotOutfitSlot *lst, int mover )
{
   int i;
   glColour *c, tc;
   int text_width, xoff;
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
         if (i==mover) {
            if (lst[i].outfit != NULL) {
               display = equipment_canSwap( &lst[i], lst[i].outfit, 0 );
               if (display != NULL)
                  c = &cRed;
               else {
                  display = "Right click to remove";
                  c = &cDConsole;
               }
            }
            else if ((equipment_outfit != NULL) &&
                  (lst->slot == equipment_outfit->slot)) {
               display = equipment_canSwap( NULL, equipment_outfit, 1 );
               if (display != NULL)
                  c = &cRed;
               else {
                  display = "Right click to add";
                  c = &cDConsole;
               }
            }
         }
         else {
            if (outfit_isLauncher(lst[i].outfit) ||
                  (outfit_isFighterBay(lst[i].outfit))) {
               if ((lst[i].u.ammo.outfit == NULL) ||
                     (lst[i].u.ammo.quantity == 0)) {
                  display = "Out of ammo.";
                  c = &cRed;
               }
               else if (lst[i].u.ammo.quantity + lst[i].u.ammo.deployed <
                     outfit_amount(lst[i].outfit)) {
                  display = "Low ammo.";
                  c = &cYellow;
               }
            }
         }

         if (display != NULL) {
            text_width = gl_printWidthRaw( &gl_smallFont, display );
            xoff = (text_width - w)/2;
            tc.r = 1.;
            tc.g = 1.;
            tc.b = 1.;
            tc.a = 0.5;
            toolkit_drawRect( x-xoff-5, y - gl_smallFont.h - 5,
                  text_width+10, gl_smallFont.h+5,
                  &tc, NULL );
            gl_printMaxRaw( &gl_smallFont, text_width,
                  x-xoff + SCREEN_W/2., y - gl_smallFont.h -2. + SCREEN_H/2.,
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
static void equipment_renderOverlay( double bx, double by, double bw, double bh )
{
   (void) bw;
   Pilot *p;
   int m, mover;
   double x, y;
   double w, h;
   PilotOutfitSlot *slot;
   char alt[512];
   int pos;
   Outfit *o;

   /* Must have selected ship. */
   if (equipment_selected != NULL) {

      /* Calculate height of outfit widgets. */
      p = equipment_selected;
      m = MAX( MAX( p->outfit_nhigh, p->outfit_nmedium ), p->outfit_nlow );
      h = (bh-30.)/m;
      if (h > 40)
         h = 40;
      w = h;

      /* Get selected. */
      mover    = equipment_mouseover;

      /* Render high outfits. */
      x = bx + 10 + (40-w)/2;
      y = by + bh - 60 + (40-h)/2;
      equipment_renderOverlayColumn( x, y, w, h,
            p->outfit_nhigh, p->outfit_high, mover );
      mover    -= p->outfit_nhigh;
      x = bx + 10 + (40-w)/2 + 60;
      y = by + bh - 60 + (40-h)/2;
      equipment_renderOverlayColumn( x, y, w, h,
            p->outfit_nmedium, p->outfit_medium, mover );
      mover    -= p->outfit_nmedium;
      x = bx + 10 + (40-w)/2 + 120;
      y = by + bh - 60 + (40-h)/2;
      equipment_renderOverlayColumn( x, y, w, h,
            p->outfit_nlow, p->outfit_low, mover );
   }

   /* Mouse must be over something. */
   if (equipment_mouseover < 0)
      return;

   /* Get the slot. */
   p = equipment_selected;
   if (equipment_mouseover < p->outfit_nhigh) {
      slot = &p->outfit_high[equipment_mouseover];
   }
   else if (equipment_mouseover < p->outfit_nhigh + p->outfit_nmedium) {
      slot = &p->outfit_medium[ equipment_mouseover - p->outfit_nhigh ];
   }
   else {
      slot = &p->outfit_low[ equipment_mouseover -
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
         "%s\n",
         o->name,
         o->desc_short );
   if (o->mass > 0.)
      pos += snprintf( &alt[pos], sizeof(alt)-pos,
            "%.0f Tons",
            o->mass );

   /* Draw the text. */
   toolkit_drawAltText( bx + equipment_altx, by + equipment_alty, alt );
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
   GLfloat vertex[2*4];

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
   if ((equipment_slot >=0) && (equipment_slot < p->outfit_nhigh)) {
      p->tsx = sx;
      p->tsy = sy;
      pilot_getMount( p, &p->outfit_high[equipment_slot], &v );
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
      gl_vboSubData( equipment_vbo, 0, sizeof(GLfloat) * (2*4), vertex );
      gl_vboActivateOffset( equipment_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
      gl_vboActivateOffset( equipment_vbo, GL_COLOR_ARRAY,
            sizeof(GLfloat) * 2*4, 4, GL_FLOAT, 0 );
      glDrawArrays( GL_LINES, 0, 4 );
      gl_vboDeactivate();
      glLineWidth( 1. );
   }
   lc = toolkit_colLight;
   c  = toolkit_col;
   dc = toolkit_colDark;
   toolkit_drawOutline( x - 5., y-5., w+10., h+10., 1., lc, c  );
   toolkit_drawOutline( x - 5., y-5., w+10., h+10., 2., dc, NULL  );
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
   (void) bw;
   Pilot *p;
   int m, selected, ret;
   double x, y;
   double w, h;

   /* Must have selected ship. */
   if (equipment_selected == NULL)
      return;

   /* Must be left click for now. */
   if ((event->type != SDL_MOUSEBUTTONDOWN) &&
         (event->type != SDL_MOUSEMOTION))
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
   y = bh - 60 + (40-h)/2 - 10;
   x = 10 + (40-w)/2;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( y, h, p->outfit_nhigh, my );
      if (ret >= 0) {
         if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT)
               equipment_slot = selected + ret;
            else if (event->button.button == SDL_BUTTON_RIGHT)
               equipment_swapSlot( wid, &p->outfit_high[ret] );
         }
         else {
            equipment_mouseover  = selected + ret;
            equipment_altx       = mx;
            equipment_alty       = my;
         }
         return;
      }
   }
   selected += p->outfit_nhigh;
   x = 10 + (40-w)/2 + 60;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( y, h, p->outfit_nmedium, my );
      if (ret >= 0) {
         if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT)
               equipment_slot = selected + ret;
            else if (event->button.button == SDL_BUTTON_RIGHT)
               equipment_swapSlot( wid, &p->outfit_medium[ret] );
         }
         else {
            equipment_mouseover = selected + ret;
            equipment_altx       = mx;
            equipment_alty       = my;
         }
         return;
      }
   }
   selected += p->outfit_nmedium;
   x = 10 + (40-w)/2 + 120;
   if ((mx > x-10) && (mx < x+w+10)) {
      ret = equipment_mouseColumn( y, h, p->outfit_nlow, my );
      if (ret >= 0) {
         if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT)
               equipment_slot = selected + ret;
            else if (event->button.button == SDL_BUTTON_RIGHT)
               equipment_swapSlot( wid, &p->outfit_low[ret] );
         }
         else {
            equipment_mouseover = selected + ret;
            equipment_altx       = mx;
            equipment_alty       = my;
         }
         return;
      }
   }

   /* Not over anything. */
   equipment_mouseover = -1;
}


/**
 * @brief CHecks to see if can swap equipment.
 *
 *    @return NULL if can swap, or error message if can't.
 */
static const char* equipment_canSwap( PilotOutfitSlot *s, Outfit *o, int add )
{
   Pilot *p;

   /* Get targets. */
   p = equipment_selected;

   /* Just in case. */
   if ((p==NULL) || (o==NULL))
      return "Nothing selected.";

   /* Adding outfit. */
   if (add) {
      if ((outfit_cpu(o) > 0) && (p->cpu < outfit_cpu(o)))
         return "Insufficient CPU";

      /* Can't add more then one afterburner. */
      if (outfit_isAfterburner(o) &&
            (p->afterburner != NULL))
         return "Already have an afterburner";

      /* Must not drive some things negative. */
      if (outfit_isMod(o)) {
         /*
          * Movement.
          */
         if (((o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust) < 0) &&
               (fabs(o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust) > p->thrust))
            return "Insufficient thrust";
         if (((o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed) < 0) &&
               (fabs(o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed) > p->speed))
            return "Insufficient speed";
         if (((o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn) < 0) &&
               (fabs(o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn) > p->turn))
            return "Insufficient turn";

         /*
          * Health.
          */
         /* Max. */
         if ((o->u.mod.armour < 0) &&
               (fabs(o->u.mod.armour) > p->armour))
            return "Insufficient armour";
         if ((o->u.mod.shield < 0) &&
               (fabs(o->u.mod.shield) > p->shield))
            return "Insufficient shield";
         if ((o->u.mod.energy < 0) &&
               (fabs(o->u.mod.energy) > p->armour))
            return "Insufficient energy";
         /* Regen. */
         if ((o->u.mod.armour_regen < 0) &&
               (fabs(o->u.mod.armour_regen) > p->armour_regen))
            return "Insufficient energy regeneration";
         if ((o->u.mod.shield_regen < 0) &&
               (fabs(o->u.mod.shield_regen) > p->shield_regen))
            return "Insufficient shield regeneration";
         if ((o->u.mod.energy_regen < 0) &&
               (fabs(o->u.mod.energy_regen) > p->energy_regen))
            return "Insufficient energy regeneration";

         /* 
          * Misc.
          */
         if ((o->u.mod.fuel < 0) &&
               (fabs(o->u.mod.fuel) > p->fuel_max))
            return "Insufficient fuel";
      }
   }
   /* Removing outfit. */
   else {
      if ((outfit_cpu(o) < 0) && (p->cpu < fabs(outfit_cpu(o))))
         return "Lower CPU usage first";

      /* Must not drive some things negative. */
      if (outfit_isMod(o)) {
         /*
          * Movement.
          */
         if (((o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust) > 0) &&
               (o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust > p->thrust))
            return "Increase thrust first";
         if (((o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed) > 0) &&
               (o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed > p->speed))
            return "Increase speed first";
         if (((o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn) > 0) &&
               (o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn > p->turn))
            return "Increase turn first";

         /*
          * Health.
          */
         /* Max. */
         if ((o->u.mod.armour > 0) &&
               (o->u.mod.armour > p->armour))
            return "Increase armour first";
         if ((o->u.mod.shield > 0) &&
               (o->u.mod.shield > p->shield))
            return "Increase shield first";
         if ((o->u.mod.energy > 0) &&
               (o->u.mod.energy > p->energy))
            return "Increase energy first";
         /* Regen. */
         if ((o->u.mod.armour_regen > 0) &&
               (o->u.mod.armour_regen > p->armour_regen))
            return "Lower energy usage first";
         if ((o->u.mod.shield_regen > 0) &&
               (o->u.mod.shield_regen > p->shield_regen))
            return "Lower shield usage first";
         if ((o->u.mod.energy_regen > 0) &&
               (o->u.mod.energy_regen > p->energy_regen))
            return "Lower energy usage first";

         /* 
          * Misc.
          */
         if ((o->u.mod.fuel > 0) &&
               (o->u.mod.fuel > p->fuel_max))
            return "Increase fuel first";
      }
      else if (outfit_isFighterBay(o)) {
         if (s->u.ammo.deployed > 0)
            return "Recall the fighters first";
      }
   }

   /* Can equip. */
   return NULL;
}


/**
 * @brief Swaps an equipment slot.
 */
static int equipment_swapSlot( unsigned int wid, PilotOutfitSlot *slot )
{
   int ret;
   Outfit *o, *ammo;
   int regen;
   int q;

   /* Do not regenerate list by default. */
   regen = 0;

   /* Remove outfit. */
   if (slot->outfit != NULL) {
      o = slot->outfit;

      /* Must be able to remove. */
      if (equipment_canSwap( slot, o, 0 ) != NULL)
         return 0;

      /* Remove ammo first. */
      if (outfit_isLauncher(o) || (outfit_isFighterBay(o))) {
         ammo = slot->u.ammo.outfit;
         q    = pilot_rmAmmo( equipment_selected, slot, slot->u.ammo.quantity );
         if (q > 0)
            player_addOutfit( ammo, q );
      }

      /* Remove outfit. */
      ret = pilot_rmOutfit( equipment_selected, slot );
      if (ret == 0)
         player_addOutfit( o, 1 );

      /* See if should remake. */
      if (player_outfitOwned(o) == 1)
         regen = 1;
   }
   /* Add outfit. */
   else {
      /* Must have outfit. */
      o = equipment_outfit;
      if (o==NULL)
         return 0;

      /* Must fit slot. */
      if (o->slot != slot->slot)
         return 0;

      /* Must be able to add. */
      if (equipment_canSwap( NULL, o, 1 ) != NULL)
         return 0;

      /* Add outfit to ship. */
      ret = player_rmOutfit( o, 1 );
      if (ret == 1)
         pilot_addOutfit( equipment_selected, o, slot );

      equipment_addAmmo();

      /* See if should remake. */
      if (player_outfitOwned(o) == 0)
         regen = 1;
   }

   /* Redo the outfits thingy. */
   if (regen) {
      window_destroyWidget( wid, EQUIPMENT_OUTFITS );
      equipment_genLists( wid );
   }

   /* Update ships. */
   equipment_updateShips( wid, NULL );

   return 0;
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
   if (equipment_selected == NULL)
      p = player;
   else
      p = equipment_selected;

   /* Add ammo to all outfits. */
   for (i=0; i<p->noutfits; i++) {
      o = p->outfits[i]->outfit;

      /* Must be valid outfit. */
      if (o == NULL)
         continue;

      /* Add ammo if able to. */
      if (outfit_isLauncher(o) || (outfit_isFighterBay(o))) {

         /* Get ammo. */
         ammo = outfit_ammo(o);
         q    = player_outfitOwned(ammo);

         /* Add ammo. */
         q = pilot_addAmmo( p, p->outfits[i], ammo, q );

         /* Remove from player. */
         player_rmOutfit( ammo, q );
      }
   }
}
/**
 * @brief Generates a new ship/outfit lists if needed.
 */
void equipment_genLists( unsigned int wid )
{
   int i, l, p;
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
   Outfit *o;

   /* Get window dimensions. */
   window_dimWindow( wid, &w, &h );

   /* Calculate image array dimensions. */
   sw = 200;
   sh = (h - 100)/2;
   ow = sw;
   oh = sh;

   /* Ship list. */
   if (!widget_exists( wid, EQUIPMENT_SHIPS )) {
      equipment_selected = NULL;
      if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         nships   = player_nships()+1;
      else
         nships   = 1;
      sships   = malloc(sizeof(char*)*nships);
      tships   = malloc(sizeof(glTexture*)*nships);
      /* Add player's current ship. */
      sships[0] = strdup(player->name);
      tships[0] = player->ship->gfx_target;
      if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
         player_ships( &sships[1], &tships[1] );
      window_addImageArray( wid, 20, -40,
            sw, sh, EQUIPMENT_SHIPS, 64./96.*128., 64.,
            tships, sships, nships, equipment_updateShips );
   }

   /* Outfit list. */
   if (!widget_exists( wid ,EQUIPMENT_OUTFITS )) {
      equipment_outfit = NULL;
      noutfits = MAX(1,player_numOutfits());
      soutfits = malloc(sizeof(char*)*noutfits);
      toutfits = malloc(sizeof(glTexture*)*noutfits);
      player_getOutfits( soutfits, toutfits );
      window_addImageArray( wid, 20, -40 - sh - 40,
            sw, sh, EQUIPMENT_OUTFITS, 50., 50.,
            toutfits, soutfits, noutfits, equipment_updateOutfits );
      /* Set alt text. */
      if (strcmp(soutfits[0],"None")!=0) {
         alt = malloc( sizeof(char*) * noutfits );
         for (i=0; i<noutfits; i++) {
            o      = outfit_get( soutfits[i] );
            if (o->desc_short == NULL)
               alt[i] = NULL;
            else {
               l = strlen(o->desc_short) + 128;
               alt[i] = malloc( l );
               p = snprintf( alt[i], l,
                     "%s\n"
                     "\n"
                     "%s\n",
                     o->name,
                     o->desc_short );
               if (o->mass > 0.)
                  p += snprintf( &alt[i][p], l-p,
                        "%.0f Tons\n",
                        o->mass );
               p += snprintf( &alt[i][p], l-p,
                     "Quantity %d",
                     player_outfitOwned(o) );
            }
         }
         toolkit_setImageArrayAlt( wid, EQUIPMENT_OUTFITS, alt );
      }
   }

   /* Update window. */
   equipment_updateOutfits(wid, NULL); /* Will update ships also. */
}
/**
 * @brief Updates the player's ship window.
 *    @param wid Window to update.
 *    @param str Unused.
 */
void equipment_updateShips( unsigned int wid, char* str )
{
   (void)str;
   char buf[512], sysname[128], buf2[16], buf3[16];
   char *shipname;
   Pilot *ship;
   char* loc;
   unsigned int price;
   int onboard;
   int cargo;

   /* Clear defaults. */
   equipment_slot       = -1;
   equipment_mouseover  = -1;
   equipment_lastick    = SDL_GetTicks();

   /* Get the ship. */
   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   if (strcmp(shipname,player->name)==0) { /* no ships */
      ship    = player;
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
   equipment_selected = ship;

   /* update text */
   credits2str( buf2, price , 2 ); /* transport */
   credits2str( buf3, player_shipPrice(shipname), 2 ); /* sell price */
   cargo = pilot_cargoFree(ship) + pilot_cargoUsed(ship);
   snprintf( buf, sizeof(buf),
         "%s\n"
         "%s\n"
         "%s\n"
         "%s credits\n"
         "\n"
         "%.0f Tons\n"
         "%.0f MN/ton\n"
         "%.0f M/s\n"
         "%.0f Grad/s\n"
         "\n"
         "%.0f MJ (%.1f MJ/s)\n"
         "%.0f MJ (%.1f MJ/s)\n"
         "%.0f MJ (%.1f MJ/s)\n"
         "\n"
         "%d / %d Tons\n"
         "%.0f / %.0f Units\n"
         "\n"
         "%s credits\n"
         "%s%s",
         /* Generic. */
         ship->name,
         ship->ship->name,
         ship_class(ship->ship),
         buf3,
         /* Movement. */
         ship->solid->mass,
         ship->thrust/ship->solid->mass,
         ship->speed,
         ship->turn,
         /* Health. */
         ship->shield_max, ship->shield_regen,
         ship->armour_max, ship->armour_regen,
         ship->energy_max, ship->energy_regen,
         /* Misc. */
         pilot_cargoUsed(ship), cargo,
         ship->fuel, ship->fuel_max,
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
      equipment_outfit = NULL;
      return;
   }
   equipment_outfit = outfit_get(oname);

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
   char *shipname;
   Pilot *ship;
   char* loc;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
   if (strcmp(shipname,"None")==0) /* no ships */
      return;
   ship  = player_getShip( shipname );
   loc   = player_getLoc( ship->name );

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
   char *shipname, *loc;
   Pilot *newship;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
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
   window_destroyWidget( wid, EQUIPMENT_SHIPS );
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
   Pilot *ship;
   Outfit *o;

   ship = equipment_selected;

   /* Remove all outfits. */
   for (i=0; i<ship->noutfits; i++) {
      o = ship->outfits[i]->outfit;
      ret = pilot_rmOutfit( ship, ship->outfits[i] );
      if (ret==0)
         player_addOutfit( o, 1 );
   }

   /* Regenerate list. */
   window_destroyWidget( wid, EQUIPMENT_OUTFITS );
   equipment_genLists( wid );
}
/**
 * @brief Player tries to sell a ship.
 *    @param wid Window player is selling ships in.
 *    @param str Unused.
 */
static void equipment_sellShip( unsigned int wid, char* str )
{
   (void)str;
   char *shipname, buf[16], *name;
   int price;

   shipname = toolkit_getImageArray( wid, EQUIPMENT_SHIPS );
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
   name = strdup(shipname);
   player->credits += price;
   land_checkAddRefuel();
   player_rmShip( shipname );

   /* Destroy widget - must be before widget. */
   window_destroyWidget( wid, EQUIPMENT_SHIPS );
   equipment_genLists( wid );

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

   price = (unsigned int)(sqrt(ship->solid->mass)*5000.);

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

