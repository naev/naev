/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file fader.c
 *
 * @brief Fader widget.
 */


#include "tk/toolkit_priv.h"


static void fad_render( Widget* fad, double bx, double by );
static int fad_mclick( Widget* fad, int button, int x, int y );
static int fad_mmove( Widget* fad, int x, int y, int rx, int ry );
static int fad_key( Widget* fad, SDL_Keycode key, SDL_Keymod mod );
static void fad_setValue( Widget *fad, double value );
static void fad_scrolldone( Widget *wgt );


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
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_FADER;

   /* specific */
   wgt->render          = fad_render;
   /*wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);*/ /**< @todo Let faders focus. */
   wgt->mclickevent     = fad_mclick;
   wgt->mmoveevent      = fad_mmove;
   wgt->keyevent        = fad_key;
   wgt->scrolldone      = fad_scrolldone;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->dat.fad.min     = min;
   wgt->dat.fad.max     = max;
   wgt->dat.fad.value   = CLAMP(min, max, def);
   wgt->dat.fad.fptr    = call;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus( wdw );
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
   pos = (fad->dat.fad.value-fad->dat.fad.min) / (fad->dat.fad.max-fad->dat.fad.min);
   w = fad->w;
   h = fad->h;

   /* Track. */
   tx = bx + fad->x + (h > w ? (w - 5.) / 2 : 0);
   ty = by + fad->y + (h < w ? (h - 5.) / 2 : 0);
   tw = (h < w ? w : 5.);
   th = (h > w ? h : 5.);
   toolkit_drawRect(tx, ty, tw , th, toolkit_colLight, NULL);

   /* Knob. */
   kx = bx + fad->x + (h < w ? w * pos - 5. : 0);
   ky = by + fad->y + (h > w ? h * pos - 5. : 0);
   kw = (h < w ? 15. : w);
   kh = (h > w ? 15. : h);

   /* Draw. */
   toolkit_drawRect(kx, ky, kw, kh, toolkit_colLight, NULL);
   toolkit_drawOutline(kx + 1, ky, kw - 1, kh - 1, 1., toolkit_colDark, NULL);
}


/**
 * @brief handles mouse movements over the fader.
 *
 *    @param fad The fader widget handling the mouse movements.
 *    @param mmove The event being generated.
 */
static int fad_mmove( Widget* fad, int x, int y, int rx, int ry )
{
   double d;
   (void) rx;
   (void) ry;

   /* Must be scrolling. */
   if (fad->status != WIDGET_STATUS_SCROLLING)
      return 0;

   /* Set the fader value. */
   d = (fad->w > fad->h) ? (double)x / fad->w : (double)y / fad->h;
   fad_setValue(fad, d);

   return 1;
}


/**
 * @brief Handles fader mouse clicks.
 */
static int fad_mclick( Widget* fad, int button, int x, int y )
{
   int kx, ky, kw, kh;
   double pos;

   /* Only handle left mouse button. */
   if (button != SDL_BUTTON_LEFT)
      return 0;

   /* Get position. */
   pos = (fad->dat.fad.value-fad->dat.fad.min) / (fad->dat.fad.max-fad->dat.fad.min);

   /* Knob. */
   if (fad->h < fad->w) {
      kx = fad->w * pos - 5;
      kw = 15;

      /* Out of bounds, jump the knob. */
      if ((x < kx) || (x >= kx + kw))
         fad_setValue(fad, (double)x / fad->w );
   }
   else {
      ky = fad->h * pos - 5;
      kh = 15;

      /* Out of bounds, jump the knob. */
      if ((y < ky) || (y >= ky + kh))
         fad_setValue(fad, (double)y / fad->h );
   }
   /* Always scroll. */
   fad->status = WIDGET_STATUS_SCROLLING;

   return 0;
}


/**
 * @brief Handles input for a fader widget.
 *
 *    @param fad Fader widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int fad_key( Widget* fad, SDL_Keycode key, SDL_Keymod mod )
{
   (void) mod;
   int ret;
   double cur;

   /* Current value. */
   cur = (fad->dat.fad.value - fad->dat.fad.min) / (fad->dat.fad.max - fad->dat.fad.min);

   /* Handle keypresses. */
   ret = 0;
   switch (key) {
      case SDLK_RIGHT:
      case SDLK_UP:
         fad_setValue( fad, cur+0.05 );
         break;

      case SDLK_LEFT:
      case SDLK_DOWN:
         fad_setValue( fad, cur-0.05 );
         break;

      default:
         break;
   }

   return ret;
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
 * @brief Changes fader value.
 *
 *    @param fad Fader to set value of in per one [0:1].
 *    @param value Value to set fader to.
 */
static void fad_setValue( Widget *fad, double value )
{
   /* Set value. */
   fad->dat.fad.value  = value * (fad->dat.fad.max - fad->dat.fad.min);
   fad->dat.fad.value += fad->dat.fad.min;

   /* Safety check. */
   fad->dat.fad.value = CLAMP( fad->dat.fad.min, fad->dat.fad.max,
         fad->dat.fad.value );

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
 * @brief Sets a fader widget's boundaries.
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

   /* Set the fader boundaries. */
   wgt->dat.fad.min = min;
   wgt->dat.fad.max = max;

   /* Set the value. */
   fad_setValue(wgt, wgt->dat.fad.value );
}

/**
 * @brief Internal callback.
 */
static void fad_scrolldone( Widget *wgt )
{
   if (wgt->dat.fad.scrolldone != NULL)
      wgt->dat.fad.scrolldone( wgt->wdw, wgt->name );
}


/**
 * @brief Sets the scroll done callback for a fader.
 *
 *    @param wid Window to which the fader belongs.
 *    @param name Name of the fader.
 *    @param func Function to call when scrolling is done.
 */
void window_faderScrollDone( const unsigned int wid,
      char *name, void (*func)(unsigned int,char*) )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Not setting scroll done function callback for non-fader widget '%s'.", name);
      return;
   }

   wgt->dat.fad.scrolldone = func;
}

