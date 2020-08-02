/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file rect.c
 *
 * @brief Rectangle widget.
 */


#include "tk/toolkit_priv.h"


static void rct_render( Widget* rct, double bx, double by );


/**
 * @brief Adds a rectangle widget to a window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 *    @param name Name of the widget to use internally.
 *    @param colour Colour of the rectangle.
 *    @param border Whether or not it should have a border.
 */
void window_addRect( const unsigned int wid,
                     const int x, const int y, /* position */
                     const int w, const int h, /* size */
                     char* name, const glColour* colour, int border )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_RECT;

   /* specific */
   wgt->render          = rct_render;
   if (colour != NULL) {
      wgt->dat.rct.colour  = *colour;
      wgt->dat.rct.fill    = 1;
   }
   else
      wgt->dat.rct.fill    = 0;
   wgt->dat.rct.border  = border;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Renders a rectangle widget.
 *
 *    @param wct Rectangle widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void rct_render( Widget* rct, double bx, double by )
{
   double x, y;

   x = bx + rct->x;
   y = by + rct->y;

   if (rct->dat.rct.fill) /* draw rect only if it exists */
      toolkit_drawRect( x, y, rct->w, rct->h, &rct->dat.rct.colour, NULL );

   if (rct->dat.rct.border) {
      /* inner outline */
      toolkit_drawOutline( x, y, rct->w, rct->h, 0.,
            toolkit_colLight, NULL );
      /* outer outline */
      toolkit_drawOutline( x, y, rct->w, rct->h, 1.,
            toolkit_colDark, NULL );
   }
}

