/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file text.c
 *
 * @brief Text widget.
 */


#include "tk/toolkit_priv.h"


static void fad_render( Widget* fad, double bx, double by );


/**
 * @brief Adds a Fader widget.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 * If width is bigger the height it's horizontal, otherwise it's vertical.
 *
 *    @param wid Window to add to.
 *    @param x X position.
 *    @param y Y position.
 *    @param w Width.
 *    @param h Height.
 *    @param name Internal widget name.
 *    @param min Minimum value.
 *    @param max Maximum value.
 *    @param def Default value.
 *    @param call Callback when fader is modified.
 */
void window_addFader( const unsigned int wid,
                      const int x, const int y, /* position */
                      const int w, const int h, /* size */
                      char* name, const double min, const double max,
                      const double def,
                      void (*call) (unsigned int,char*) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type   = WIDGET_FADER;
   wgt->name   = strdup(name);
   wgt->wdw    = wid;

   /* specific */
   wgt->render          = fad_render;
   /*wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);*/ /**< @todo Let faders focus. */
   wgt->dat.fad.value   = min;
   wgt->dat.fad.min     = min;
   wgt->dat.fad.max     = max;
   wgt->dat.fad.value   = CLAMP(min, max, def);;
   wgt->dat.fad.fptr    = call;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/**
 * @brief Renders a fader
 *
 *    @param fad WIDGET_FADER widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void fad_render( Widget* fad, double bx, double by )
{
   double pos;
   double w,h;
   double tx,ty, tw,th;
   double kx,ky, kw,kh;

   /* Some values. */
   pos = fad->dat.fad.value / (fad->dat.fad.max - fad->dat.fad.min);
   w = fad->w;
   h = fad->h;

   /* Track. */
   tx = bx + fad->x + (h > w ? (w - 5.) / 2 : 0);
   ty = by + fad->y + (h < w ? (h - 5.) / 2 : 0);
   tw = (h < w ? w : 5.);                                           
   th = (h > w ? h : 5.);                                           
   toolkit_drawRect(tx, ty, tw , th, toolkit_colDark, toolkit_colDark);

   /* Knob. */
   kx = bx + fad->x + (h < w ? w * pos - 5. : 0);
   ky = by + fad->y + (h > w ? h * pos - 5. : 0);
   kw = (h < w ? 15. : w);
   kh = (h > w ? 15. : h);

   /* Draw. */
   toolkit_drawRect(kx, ky, kw , kh, toolkit_colDark, toolkit_colLight);
   toolkit_drawOutline(kx, ky, kw , kh, 1., &cBlack, toolkit_colDark);
}


