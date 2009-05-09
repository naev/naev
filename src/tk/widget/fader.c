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
static int fad_mmove( Widget* fad, SDL_MouseMotionEvent *mmove );
static void fad_setValue( Widget *fad, double value );


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
   wgt->mmoveevent      = fad_mmove;
   wgt->dat.fad.value   = min;
   wgt->dat.fad.min     = min;
   wgt->dat.fad.max     = max;
   wgt->dat.fad.value   = CLAMP(min, max, def);
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


/**
 * @brief handles mouse movements over the fader.
 *
 *    @param fad The fader widget handling the mouse movements.
 *    @param mmove The event being generated.
 */
static int fad_mmove( Widget* fad, SDL_MouseMotionEvent* mmove )
{
   double d;

   /* Must be dragging mouse. */
   if (!(mmove->state & SDL_BUTTON(1)))
      return 0;

   /* Set the fader value. */
   d = (fad->w > fad->h) ? mmove->xrel / fad->w : mmove->yrel / fad->h;
   fad_setValue(fad, fad->dat.fad.value + d);

   return 1;
}


/**
 * @brief Gets value of fader widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 */
double window_getFaderValue( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return 0.;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Trying to get fader value from non-fader widget '%s'.", name);
      return 0.;
   }

   /* Return the value. */
   return (wgt) ? wgt->dat.fad.value : 0.;
}


/**
 * @brief Changes fader value
 *
 *    @param fad Fader to set value of.
 *    @param value Value to set fader to.
 */
static void fad_setValue( Widget *fad, double value )
{
   /* Sanity check and value set. */
   fad->dat.fad.value = CLAMP( fad->dat.fad.min, fad->dat.fad.max, value );

   /* Run function if needed. */
   if (fad->dat.fad.fptr != NULL)
      (*fad->dat.fad.fptr)(fad->wdw, fad->name);
}


/**
 * @brief Sets a fader widget's value.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 *    @para value Value to set fader to.
 */
void window_faderValue( const unsigned int wid,
      char* name, double value )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Not setting fader value on non-fader widget '%s'.", name);
      return;
   }

   /* Set fader value. */
   fad_setValue(wgt, value);
}


/**
 * @brief Sets a fader widget's boundries.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 *    @param min Minimum fader value.
 *    @param max Maximum fader value.
 */
void window_faderBounds( const unsigned int wid,
      char* name, double min, double max )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Not setting fader value on non-fader widget '%s'.", name);
      return;
   }

   /* Set the fader boundries. */
   wgt->dat.fad.min = min;
   wgt->dat.fad.max = max;

   /* Set the value. */
   fad_setValue(wgt, wgt->dat.fad.value );
}
