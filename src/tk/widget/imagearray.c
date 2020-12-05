/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file imagearray.c
 *
 * @brief Image array widget.
 */


#include "tk/toolkit_priv.h"

#include <stdlib.h>
#include "nstring.h"

#include "opengl.h"


/* Render. */
static void iar_render( Widget* iar, double bx, double by );
static void iar_renderOverlay( Widget* iar, double bx, double by );
/* Key. */
static int iar_key( Widget* iar, SDL_Keycode key, SDL_Keymod mod );
/* Mouse. */
static int iar_mclick( Widget* iar, int button, int x, int y );
static int iar_mdoubleclick( Widget* iar, int button, int x, int y );
static int iar_mwheel( Widget* lst, SDL_MouseWheelEvent event );
static int iar_mmove( Widget* iar, int x, int y, int rx, int ry );
/* Focus. */
static int iar_focusImage( Widget* iar, double bx, double by );
static void iar_focus( Widget* iar, double bx, double by );
static void iar_scroll( Widget* iar, int direction );
static void iar_centerSelected( Widget *iar );
/* Misc. */
static void iar_setAltTextPos( Widget *iar, double bx, double by );
static Widget *iar_getWidget( const unsigned int wid, const char *name );
static char* toolkit_getNameById( Widget *wgt, int elem );
/* Clean up. */
static void iar_cleanup( Widget* iar );


/**
 * @brief Adds an Image Array widget.
 *
 * Position origin is 0,0 at bottom left. If you use negative X or Y
 *  positions. They actually count from the opposite side in.
 *
 *    @param wid Window to add to.
 *    @param x X position.
 *    @param y Y position.
 *    @param w Width.
 *    @param h Height.
 *    @param name Internal widget name.
 *    @param iw Image width to use.
 *    @param ih Image height to use.
 *    @param img Image widget array to use (not freed).
 *    @param caption Caption array to use (freed).
 *    @param nelem Elements in tex and caption.
 *    @param call Callback when modified.
 */
void window_addImageArray( const unsigned int wid,
                           const int x, const int y, /* position */
                           const int w, const int h, /* size */
                           char* name, const int iw, const int ih,
                           ImageArrayCell *img, int nelem,
                           void (*call) (unsigned int wdw, char* wgtname),
                           void (*rmcall) (unsigned int wdw, char* wgtname),
                           void (*dblcall) (unsigned int wdw, char* wgtname) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_IMAGEARRAY;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   /* specific */
   wgt->render             = iar_render;
   wgt->renderOverlay      = iar_renderOverlay;
   wgt->cleanup            = iar_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->keyevent           = iar_key;
   wgt->mclickevent        = iar_mclick;
   wgt->mdoubleclickevent  = iar_mdoubleclick;
   wgt->mwheelevent        = iar_mwheel;
   wgt->mmoveevent         = iar_mmove;
   wgt_setFlag(wgt, WGT_FLAG_ALWAYSMMOVE);
   wgt->dat.iar.images     = img;
   wgt->dat.iar.nelements  = nelem;
   wgt->dat.iar.selected   = 0;
   wgt->dat.iar.pos        = 0;
   wgt->dat.iar.alt        = -1;
   wgt->dat.iar.altx       = -1;
   wgt->dat.iar.alty       = -1;
   wgt->dat.iar.iw         = iw;
   wgt->dat.iar.ih         = ih;
   wgt->dat.iar.fptr       = call;
   wgt->dat.iar.rmptr      = rmcall;
   wgt->dat.iar.dblptr     = dblcall;
   wgt->dat.iar.xelem      = floor((w - 10.) / (double)(wgt->dat.iar.iw+10));
   wgt->dat.iar.yelem      = (wgt->dat.iar.xelem == 0) ? 0 :
         (int)wgt->dat.iar.nelements / wgt->dat.iar.xelem + 1;

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus( wdw );
}


/**
 * @brief Gets image array effective dimensions.
 */
static void iar_getDim( Widget* iar, double *w, double *h )
{
   if (w != NULL)
      *w = iar->dat.iar.iw + 5.*2.;
   if (h != NULL)
      *h = iar->dat.iar.ih + 5.*2. + 2. + gl_smallFont.h;
}


/**
 * @brief Renders an image array.
 *
 *    @param iar Image array widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void iar_render( Widget* iar, double bx, double by )
{
   int i, j, k, pos;
   double x,y, w,h, xcurs,ycurs;
   double scroll_pos;
   int xelem, yelem;
   double xspace;
   const glColour *dc, *lc;
   glColour fontcolour;
   int is_selected;
   double d;

   /*
    * Calculations.
    */
   /* position */
   x = bx + iar->x;
   y = by + iar->y;

   /* element dimensions */
   iar_getDim( iar, &w, &h );

   /* number of elements */
   xelem = iar->dat.iar.xelem;
   yelem = iar->dat.iar.yelem;
   xspace = (double)(((int)iar->w - 10) % (int)w) / (double)(xelem + 1);

   /* background */
   toolkit_drawRect( x, y, iar->w, iar->h, &cBlack, NULL );

   /*
    * Scrollbar.
    */
   d          = h * (yelem - (int)(iar->h / h));
   if (fabs(d) < 1e-05)
      scroll_pos = 0.;
   else
      scroll_pos = iar->dat.iar.pos / d;
   toolkit_drawScrollbar( x + iar->w - 10., y, 10., iar->h, scroll_pos );

   /*
    * Main drawing loop.
    */
   gl_clipRect( x, y, iar->w, iar->h );
   ycurs = y + iar->h - h + iar->dat.iar.pos;
   for (j=0; j<yelem; j++) {
      xcurs = x + xspace;

      /*  Skip rows that are wholly outside of the viewport. */
      if ((ycurs > y + iar->h) || (ycurs + h < y)) {
         ycurs -= h;
         continue;
      }

      for (i=0; i<xelem; i++) {

         /* Get position. */
         pos = j*xelem + i;

         /* Out of elements. */
         if ((pos) >= iar->dat.iar.nelements)
            break;

         is_selected = (iar->dat.iar.selected == pos) ? 1 : 0;

         fontcolour = cFontWhite;
         /* Draw background. */
         toolkit_drawRect( xcurs + 2.,
               ycurs + 2.,
               w - 5., h - 5., &iar->dat.iar.images[pos].bg, NULL );

         /* image */
         if (iar->dat.iar.images[pos].image != NULL)
            gl_blitScale( iar->dat.iar.images[pos].image,
                  xcurs + 5., ycurs + gl_smallFont.h + 7.,
                  iar->dat.iar.iw, iar->dat.iar.ih, NULL );

         /* layers */
         for (k=0; k<iar->dat.iar.images[pos].nlayers; k++)
            gl_blitScale( iar->dat.iar.images[pos].layers[k],
                  xcurs + 5., ycurs + gl_smallFont.h + 7.,
                  iar->dat.iar.iw, iar->dat.iar.ih, NULL );

         /* caption */
         if (iar->dat.iar.images[pos].caption != NULL)
            gl_printMidRaw( &gl_smallFont, iar->dat.iar.iw, xcurs + 5., ycurs + 5.,
                     &fontcolour, -1., iar->dat.iar.images[pos].caption );

         /* quantity. */
         if (iar->dat.iar.images[pos].quantity > 0) {
            /* Quantity number. */
            gl_printMax( &gl_smallFont, iar->dat.iar.iw,
                  xcurs + 5., ycurs + iar->dat.iar.ih + 4.,
                  &fontcolour, "%d", iar->dat.iar.images[pos].quantity );
         }

         /* Slot type. */
         if (iar->dat.iar.images[pos].slottype != NULL) {
            /* Slot size letter. */
            gl_printMaxRaw( &gl_smallFont, iar->dat.iar.iw,
                  xcurs + iar->dat.iar.iw - 10., ycurs + iar->dat.iar.ih + 4.,
                  &fontcolour, -1., iar->dat.iar.images[pos].slottype );
         }

         /* outline */
         if (is_selected) {
            lc = &cWhite;
            dc = &cGrey60;
         }
         else {
            lc = toolkit_colLight;
            dc = toolkit_col;
         }
         toolkit_drawOutline( xcurs + 2.,
               ycurs + 2.,
               w - 4., h - 4., 1., lc, NULL );
         toolkit_drawOutline( xcurs + 2.,
               ycurs + 2.,
               w - 4., h - 4., 2., dc, NULL );
         xcurs += w + xspace;
      }
      ycurs -= h;
   }
   gl_unclipRect();

   /*
    * Final outline.
    */
   // toolkit_drawOutline( x+1, y+1, iar->w-2, iar->h-2, 1., toolkit_colLight, NULL );
   // toolkit_drawOutline( x+1, y+1, iar->w-2, iar->h-2, 2., toolkit_colDark, NULL );
}


/**
 * @brief Renders the overlay.
 */
static void iar_renderOverlay( Widget* iar, double bx, double by )
{
   double x, y;
   const char *alt;

   /*
    * Draw Alt text if applicable.
    */
   if ((iar->dat.iar.alt >= 0) &&
         (iar->dat.iar.altx != -1) && (iar->dat.iar.alty != -1)) {

      /* Calculate position. */
      x = bx + iar->x + iar->dat.iar.altx;
      y = by + iar->y + iar->dat.iar.alty;

      /* Draw alt text. */
      alt = iar->dat.iar.images[iar->dat.iar.alt].alt;
      if (alt != NULL)
         toolkit_drawAltText( x, y, alt );
   }
}


/**
 * @brief Handles input for an image array widget.
 *
 *    @param iar Image array widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int iar_key( Widget* iar, SDL_Keycode key, SDL_Keymod mod )
{
   (void) mod;

   switch (key) {
      case SDLK_UP:
         iar->dat.iar.selected -= iar->dat.iar.xelem;
         break;
      case SDLK_DOWN:
         iar->dat.iar.selected += iar->dat.iar.xelem;
         break;
      case SDLK_RIGHT:
         iar->dat.iar.selected += 1;
         break;
      case SDLK_LEFT:
         iar->dat.iar.selected -= 1;
         break;

      default:
         return 0;
   }

   /* Check boundaries. */
   iar->dat.iar.selected = CLAMP( 0, iar->dat.iar.nelements-1, iar->dat.iar.selected);

   /* Run function pointer if needed. */
   if (iar->dat.iar.fptr)
      iar->dat.iar.fptr( iar->wdw, iar->name);

   iar_centerSelected( iar );
   return 1;
}


/**
 * @brief Centers on the selection if needed.
 *
 *    @param iar Widget to center on selection.
 */
static void iar_centerSelected( Widget *iar )
{
   int y=0;
   double h;
   double hmax;
   double ypos;

   /* Get dimensions. */
   iar_getDim( iar, NULL, &h );

   /* Ignore fancy stuff if smaller than height. */
   if (h * iar->dat.iar.yelem < iar->h)
      return;

   /* Move if needed. */
   hmax = h * (iar->dat.iar.yelem - (int)(iar->h / h));
   if ( iar->dat.iar.selected >= 0 )
     y = iar->dat.iar.selected / iar->dat.iar.xelem;
   ypos = y * h;
   /* Below. */
   if (ypos < iar->dat.iar.pos)
      iar->dat.iar.pos = ypos;
   /* Above. */
   if (ypos > iar->dat.iar.pos + iar->h - h)
      iar->dat.iar.pos = ypos - h*floor(iar->h/h) + 10.;
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );

   iar_setAltTextPos( iar, iar->dat.iar.altx, iar->dat.iar.alty );
}


/**
 * @brief Image array widget mouse click handler.
 *
 *    @param iar Widget receiving the event.
 *    @return 1 if event is used.
 */
static int iar_mclick( Widget* iar, int button, int x, int y )
{
   /* Handle different mouse clicks. */
   switch (button) {
      case SDL_BUTTON_LEFT:
         iar_focus( iar, x, y );
         return 1;
      case SDL_BUTTON_X1:
         iar_scroll( iar, +1 );
         return 1;
      case SDL_BUTTON_X2:
         iar_scroll( iar, -1 );
         return 1;
      case SDL_BUTTON_RIGHT:
         iar_focus( iar, x, y );
         if (iar->dat.iar.rmptr != NULL)
            iar->dat.iar.rmptr( iar->wdw, iar->name );

         iar_setAltTextPos( iar, x, y );

         return 1;

      default:
         break;
   }
   return 0;
}


/**
 * @brief Image array widget mouse double click handler.
 *
 *    @param iar Widget receiving the event.
 *    @return 1 if event is used.
 */
static int iar_mdoubleclick( Widget* iar, int button, int x, int y )
{
   /* Handle different mouse clicks. */
   switch (button) {
      case SDL_BUTTON_LEFT:
         iar_focus( iar, x, y );
         if (iar->dat.iar.dblptr != NULL)
            iar->dat.iar.dblptr( iar->wdw, iar->name );
         iar_setAltTextPos( iar, x, y );
         return 1;

      default:
         break;
   }
   return 0;
}


/**
 * @brief Handler for mouse wheel events for an image array.
 *
 *    @param iar The widget handling the mouse wheel event.
 *    @param event The event the widget should handle.
 *    @return 1 if the widget uses the event.
 */
static int iar_mwheel( Widget* iar, SDL_MouseWheelEvent event )
{
   if (event.y > 0)
      iar_scroll( iar, +1 );
   else
      iar_scroll( iar, -1 );

   return 1;
}


/**
 * @brief Handles mouse movement for an image array.
 *
 *    @param iar Widget handling the mouse motion.
 *    @param mmove Mouse motion event to handle.
 *    @return 1 if the event is used.
 */
static int iar_mmove( Widget* iar, int x, int y, int rx, int ry )
{
   (void) rx;
   (void) ry;
   double w,h;
   int yelem;
   double hmax;

   if (iar->status == WIDGET_STATUS_SCROLLING) {

      y = CLAMP( 15, iar->h - 15., iar->h - y );

      /* element dimensions */
      iar_getDim( iar, &w, &h );

      /* number of elements */
      yelem = iar->dat.iar.yelem;

      hmax = h * (yelem - (int)(iar->h / h));
      iar->dat.iar.pos = (y - 15.) * hmax / (iar->h - 30.);

      /* Does boundary checks. */
      iar_scroll( iar, 0 );

      return 1;
   }
   else {
      if ((x < 0) || (x >= iar->w) || (y < 0) || (y >= iar->h))
         iar->dat.iar.alt  = -1;
      else
         iar_setAltTextPos( iar, x, y );
   }

   return 0;
}


/**
 * @brief Clean up function for the image array widget.
 *
 *    @param iar Image array widget to clean up.
 */
static void iar_cleanup( Widget* iar )
{
   int i, j;

   if (iar->dat.iar.nelements > 0) { /* Free each text individually */
      for (i=0; i<iar->dat.iar.nelements; i++) {
         if (iar->dat.iar.images[i].image != NULL)
            gl_freeTexture( iar->dat.iar.images[i].image );
         if (iar->dat.iar.images[i].caption != NULL)
            free( iar->dat.iar.images[i].caption );
         if (iar->dat.iar.images[i].alt != NULL)
            free( iar->dat.iar.images[i].alt );
         if (iar->dat.iar.images[i].slottype != NULL)
            free( iar->dat.iar.images[i].slottype );

         for (j=0; j<iar->dat.iar.images[i].nlayers; j++)
            gl_freeTexture( iar->dat.iar.images[i].layers[j] );
         if (iar->dat.iar.images[i].layers != NULL)
            free( iar->dat.iar.images[i].layers );
      }
   }

   /* Free the arrays */
   if (iar->dat.iar.images != NULL)
      free( iar->dat.iar.images );
}


/**
 * @brief Tries to scroll a widget up/down by direction.
 *
 *    @param wgt Widget to scroll.
 *    @param direction Direction to scroll. Positive is up, negative
 *           is down and absolute value is number of elements to scroll.
 */
static void iar_scroll( Widget* iar, int direction )
{
   double w,h;
   int yelem;
   double hmax;

   if (iar == NULL)
      return;

   /* element dimensions */
   iar_getDim( iar, &w, &h );

   /* number of elements */
   yelem = iar->dat.iar.yelem;

   /* maximum */
   hmax = h * (yelem - (int)(iar->h / h));
   if (hmax < 0.)
      hmax = 0.;

   /* move */
   iar->dat.iar.pos -= direction * h;

   /* Boundary check. */
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );
   if (iar->dat.iar.fptr)
      iar->dat.iar.fptr( iar->wdw, iar->name );

   iar_setAltTextPos( iar, iar->dat.iar.altx, iar->dat.iar.alty );
}


/**
 * @brief See what widget is being focused.
 */
static int iar_focusImage( Widget* iar, double bx, double by )
{
   int x, y;
   double w, h;
   int xelem;
   double xspace;

   /* element dimensions */
   iar_getDim( iar, &w, &h );

   /* number of elements */
   xelem = iar->dat.iar.xelem;
   xspace = (double)(((int)iar->w - 10) % (int)w) / (double)(xelem + 1);

   x = bx / (xspace + w);
   y = (iar->h - by + iar->dat.iar.pos) / h;

   /* Reject anything too close to the scroll bar or exceeding nelements. */
   if (y * xelem + x >= iar->dat.iar.nelements || bx >= iar->w - 10.)
      return -1;

   /* Verify that the mouse is on an icon. */
   if ((bx < (x+1) * xspace + x * w) || (bx > (x+1) * (xspace + w) - 4.) ||
         (by > iar->h + iar->dat.iar.pos - y * h - 4.))
      return -1;

   return y * xelem + x;
}



/**
 * @brief Mouse event focus on image array.
 *
 *    @param iar Image Array widget.
 *    @param bx X position click.
 *    @param by Y position click.
 */
static void iar_focus( Widget* iar, double bx, double by )
{
   double y, h;
   double scroll_pos, hmax;
   int yelem;
   int selected;

   /* element dimensions */
   iar_getDim( iar, NULL, &h );

   /* number of elements */
   yelem = iar->dat.iar.yelem;

   /* Test for item click. */
   selected = iar_focusImage( iar, bx, by );
   if (selected >= 0) {
      iar->dat.iar.selected = selected;
      if (iar->dat.iar.fptr != NULL)
         iar->dat.iar.fptr( iar->wdw, iar->name );
   }
   /* Scrollbar click. */
   else if (bx > iar->w - 10.) {
      /* Get bar position (center). */
      hmax = h * (yelem - (int)(iar->h / h));
      if (fabs(hmax) < 1e-05)
         scroll_pos = 0.;
      else
         scroll_pos = iar->dat.iar.pos / hmax;
      y = iar->h - (iar->h - 30.) * scroll_pos - 15.;

      /* Click below the bar. */
      if (by < y-15.)
         iar_scroll( iar, -2 );
      /* Click above the bar. */
      else if (by > y+15.)
         iar_scroll( iar, +2 );
      /* Click on the bar. */
      else
         iar->status = WIDGET_STATUS_SCROLLING;
   }
}


/**
 * @brief Chooses correct alt text for the given coordinates
 *
 */
static void iar_setAltTextPos( Widget *iar, double bx, double by )
{
   iar->dat.iar.alt  = iar_focusImage( iar, bx, by );
   iar->dat.iar.altx = bx;
   iar->dat.iar.alty = by;
}


/**
 * @brief Gets an image array.
 */
static Widget *iar_getWidget( const unsigned int wid, const char *name )
{
   Widget *wgt = window_getwgt(wid,name);

   /* Must be found in stack. */
   if (wgt == NULL) {
      WARN("Widget '%s' not found", name);
      return NULL;
   }

   /* Must be an image array. */
   if (wgt->type != WIDGET_IMAGEARRAY) {
      WARN("Widget '%s' is not an image array.", name);
      return NULL;
   }

   return wgt;
}


/**
 * @brief Gets the name of the element.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @param elem The element in the image array.
 *    @return The name of the selected object.
 */
static char* toolkit_getNameById( Widget *wgt, int elem )
{
   if (wgt == NULL)
      return NULL;

   /* Nothing selected. */
   if (elem == -1)
      return NULL;

   return wgt->dat.iar.images[ elem ].caption;
}


/**
 * @brief Gets what is selected currently in an Image Array.
 *
 *   \warning Oftentimes, UI code will translate or otherwise preprocess
 *            text before populating this widget.
 *            In general, reading back such processed text and trying to
 *            interpret it is ill-advised; it's better to keep the original
 *            list of objects being presented and deal with indices into it.
 *            \see toolkit_getImageArrayPos
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @return The name of the selected object.
 */
char* toolkit_getImageArray( const unsigned int wid, const char* name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL || wgt->dat.iar.selected < 0 )
      return NULL;

   return toolkit_getNameById( wgt, wgt->dat.iar.selected );
}


/**
 * @brief Sets an image array based on value.
 *
 *   \warning If the captions have been translated or otherwise preprocessed,
 *            this function can only find a name that has been transformed the
 *            same way. There may be a more robust solution involving indices.
 *            \see toolkit_setImageArrayPos
 */
int toolkit_setImageArray( const unsigned int wid, const char* name, char* elem )
{
   int i;
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   /* Case NULL. */
   if (elem == NULL) {
      wgt->dat.iar.selected = -1;
      return 0;
   }

   /* Try to find the element. */
   for (i=0; i<wgt->dat.iar.nelements; i++) {
      if (strcmp(elem,wgt->dat.iar.images[i].caption)==0) {
         wgt->dat.iar.selected = i;
         return 0;
      }
   }

   /* Element not found. */
   return -1;
}


/**
 * @brief Gets what is selected currently in an Image Array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @return The position of selected object.
 */
int toolkit_getImageArrayPos( const unsigned int wid, const char* name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   return wgt->dat.iar.selected;
}


/**
 * @brief Gets the Image Array offset.
 */
double toolkit_getImageArrayOffset( const unsigned int wid, const char* name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1.;

   return wgt->dat.iar.pos;
}


/**
 * @brief Sets the Image Array offset.
 */
int toolkit_setImageArrayOffset( const unsigned int wid, const char* name, double off )
{
   double h;
   double hmax;

   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   /* Get dimensions. */
   iar_getDim( wgt, NULL, &h );

   /* Ignore fancy stuff if smaller than height. */
   if (h * wgt->dat.iar.yelem < wgt->h) {
      wgt->dat.iar.pos = 0.;
      return 0;
   }

   /* Move if needed. */
   hmax = h * (wgt->dat.iar.yelem - (int)(wgt->h / h));
   wgt->dat.iar.pos = CLAMP( 0., hmax, off );

   iar_setAltTextPos( wgt, wgt->dat.iar.altx, wgt->dat.iar.alty );

   return 0;
}


/**
 * @brief Sets the active element in the Image Array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @param pos Position to set to.
 *    @return 0 on success.
 */
int toolkit_setImageArrayPos( const unsigned int wid, const char* name, int pos )
{
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   /* Set position. */
   wgt->dat.iar.selected = CLAMP( 0, wgt->dat.iar.nelements-1, pos );

   /* Call callback - dangerous if called from within callback. */
   if (wgt->dat.iar.fptr != NULL)
      wgt->dat.iar.fptr( wgt->wdw, wgt->name );

   iar_centerSelected( wgt );

   return 0;
}


/**
 * @brief Stores several image array attributes.
 *
 *    @param wid Window containing the image array.
 *    @param name Name of the image array widget.
 *    @param iar_data Pointer to an iar_data_t struct.
 *    @return 0 on success.
 */
int toolkit_saveImageArrayData( const unsigned int wid, const char *name,
      iar_data_t *iar_data )
{
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   iar_data->pos    = wgt->dat.iar.selected;
   iar_data->offset = wgt->dat.iar.pos;

   return 0;
}

/**
 * @brief Unsets the selection
 *
 *    @param wid Window containing the image array.
 *    @param name Name of the image array widget.
 */

int toolkit_unsetSelection( const unsigned int wid, const char *name )
{
  Widget *wgt = iar_getWidget( wid, name );

  /* unset the selection */
  wgt->dat.iar.selected = -1;

  return 0;
}
