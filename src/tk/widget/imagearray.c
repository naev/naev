/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file imagearray.c
 *
 * @brief Image array widget.
 */
/** @cond */
#include <stdlib.h>
/** @endcond */

#include "array.h"
#include "nstring.h"
#include "opengl.h"
#include "tk/toolkit_priv.h"

/* Render. */
static void iar_render( Widget *iar, double bx, double by );
static void iar_renderOverlay( Widget *iar, double bx, double by );
/* Key. */
static int iar_key( Widget *iar, SDL_Keycode key, SDL_Keymod mod,
                    int isrepeat );
/* Mouse. */
static int iar_mclick( Widget *iar, int button, int x, int y );
static int iar_mdoubleclick( Widget *iar, int button, int x, int y );
static int iar_mwheel( Widget *lst, SDL_MouseWheelEvent event );
static int iar_mmove( Widget *iar, int x, int y, int rx, int ry );
/* Focus. */
static int  iar_focusImage( Widget *iar, double bx, double by );
static void iar_focus( Widget *iar, double bx, double by );
static void iar_scroll( Widget *iar, int direction );
static void iar_centerSelected( Widget *iar );
/* Misc. */
static void        iar_updateSpacing( Widget *iar );
static double      iar_maxPos( Widget *iar );
static void        iar_setAltTextPos( Widget *iar, double bx, double by );
static Widget     *iar_getWidget( unsigned int wid, const char *name );
static const char *toolkit_getNameById( Widget *wgt, int elem );
/* Clean up. */
static void iar_cleanup( Widget *iar );

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
 *    @param rmcall Callback when right-clicked.
 *    @param dblcall Callback when selection is double-clicked.
 */
void window_addImageArray(
   unsigned int wid, const int x, const int y, /* position */
   const int w, const int h,                   /* size */
   const char *name, const int iw, const int ih, ImageArrayCell *img, int nelem,
   void ( *call )( unsigned int wdw, const char *wgtname ),
   void ( *rmcall )( unsigned int wdw, const char *wgtname ),
   void ( *dblcall )( unsigned int wdw, const char *wgtname ) )
{
   Window *wdw = window_wget( wid );
   Widget *wgt = window_newWidget( wdw, name );
   if ( wgt == NULL )
      return;

   /* generic */
   wgt->type = WIDGET_IMAGEARRAY;

   /* position/size */
   wgt->w = (double)w;
   wgt->h = (double)h;
   toolkit_setPos( wdw, wgt, x, y );

   /* specific */
   wgt->render        = iar_render;
   wgt->renderOverlay = iar_renderOverlay;
   wgt->cleanup       = iar_cleanup;
   wgt_setFlag( wgt, WGT_FLAG_CANFOCUS );
   wgt->keyevent          = iar_key;
   wgt->mclickevent       = iar_mclick;
   wgt->mdoubleclickevent = iar_mdoubleclick;
   wgt->mwheelevent       = iar_mwheel;
   wgt->mmoveevent        = iar_mmove;
   wgt_setFlag( wgt, WGT_FLAG_ALWAYSMMOVE );
   wgt->dat.iar.images    = img;
   wgt->dat.iar.nelements = nelem;
   wgt->dat.iar.selected  = 0;
   wgt->dat.iar.pos       = 0;
   wgt->dat.iar.alt       = -1;
   wgt->dat.iar.altx      = -1;
   wgt->dat.iar.alty      = -1;
   wgt->dat.iar.iwref     = iw;
   wgt->dat.iar.ihref     = ih;
   wgt->dat.iar.zoom      = 1.0;
   wgt->dat.iar.mx        = 0;
   wgt->dat.iar.my        = 0;
   wgt->dat.iar.fptr      = call;
   wgt->dat.iar.rmptr     = rmcall;
   wgt->dat.iar.dblptr    = dblcall;
   iar_updateSpacing( wgt );

   if ( wdw->focus == -1 ) /* initialize the focus */
      toolkit_nextFocus( wdw );
}

static void iar_updateSpacing( Widget *iar )
{
   double w           = iar->w;
   int    nelem       = iar->dat.iar.nelements;
   double zoom        = iar->dat.iar.zoom;
   iar->dat.iar.iw    = round( iar->dat.iar.iwref * zoom );
   iar->dat.iar.ih    = round( iar->dat.iar.ihref * zoom );
   iar->dat.iar.xelem = floor( ( w - 10. ) / (double)( iar->dat.iar.iw + 10 ) );
   iar->dat.iar.yelem =
      ( iar->dat.iar.xelem == 0 ) ? 0 : ( nelem - 1 ) / iar->dat.iar.xelem + 1;
}

/**
 * @brief Gets image array effective dimensions.
 */
static void iar_getDim( Widget *iar, double *w, double *h, double *xspace,
                        double *yspace )
{
   double _w, _h, _space;
   _w     = iar->dat.iar.iw + 5. * 2.;
   _h     = iar->dat.iar.ih + 5. * 2. + 2. + gl_smallFont.h;
   _space = ( (int)iar->w - 10 ) % (int)_w;
   _space /= ( iar->dat.iar.xelem + 1 );
   if ( w != NULL )
      *w = _w;
   if ( h != NULL )
      *h = _h;
   if ( xspace != NULL )
      *xspace = _space; /* Justify columns with ~equal spacing */
   if ( yspace != NULL )
      *yspace = round( _space ); /* Make row spacing precisely equal. */
}

/**
 * @brief Renders an image array.
 *
 *    @param iar Image array widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void iar_render( Widget *iar, double bx, double by )
{
   int             pos;
   double          x, y, w, h, xcurs, ycurs, xspace, yspace;
   double          scroll_pos;
   int             xelem, yelem;
   const glColour *dc, *lc;
   const glColour *fontcolour, *bgcolour;
   int             is_selected;
   double          hmax;

   /*
    * Calculations.
    */
   /* position */
   x = bx + iar->x;
   y = by + iar->y;

   /* element dimensions */
   iar_getDim( iar, &w, &h, &xspace, &yspace );

   /* number of elements */
   xelem = iar->dat.iar.xelem;
   yelem = iar->dat.iar.yelem;

   /* background */
   gl_renderPane( x, y, iar->w, iar->h, &cBlack );

   /*
    * Scrollbar.
    */
   hmax = iar_maxPos( iar );
   if ( hmax == 0. )
      scroll_pos = 0.;
   else
      scroll_pos = iar->dat.iar.pos / hmax;
   toolkit_drawScrollbar( x + iar->w - 12., y + 2, 10., iar->h - 4,
                          scroll_pos );

   /*
    * Main drawing loop.
    */
   gl_clipRect( x, y, iar->w, iar->h );
   for ( int j = 0; j < yelem; j++ ) {
      ycurs =
         floor( y + iar->h - ( j + 1 ) * ( h + yspace ) + iar->dat.iar.pos );
      /*  Skip rows that are wholly outside of the viewport. */
      if ( ( ycurs > y + iar->h ) || ( ycurs + h < y ) )
         continue;

      for ( int i = 0; i < xelem; i++ ) {
         ImageArrayCell *cell;

         xcurs = floor( x + i * w + ( i + 0.5 ) * xspace );

         /* Get position. */
         pos = j * xelem + i;

         /* Out of elements. */
         if ( pos >= iar->dat.iar.nelements )
            break;

         cell = &iar->dat.iar.images[pos];

         is_selected = ( iar->dat.iar.selected == pos ) ? 1 : 0;

         if ( is_selected ) {
            fontcolour = &cWhite;
            bgcolour   = toolkit_col;
         } else {
            fontcolour = &cFontWhite;
            bgcolour   = &cell->bg;
            if ( bgcolour->a <= 0. )
               bgcolour = toolkit_colDark;
         }
         /* Draw background. */
         gl_renderPane( xcurs, ycurs, w, h, bgcolour );

         /* image */
         if ( cell->image != NULL ) {
            if ( ( cell->image->sw < iar->dat.iar.iw ) &&
                 ( cell->image->sh < iar->dat.iar.ih ) ) {
               double offx, offy;
               offx = ( iar->dat.iar.iw - cell->image->sw ) * 0.5;
               offy = ( iar->dat.iar.iw - cell->image->sh ) * 0.5;
               gl_renderStatic( cell->image, xcurs + 5. + offx,
                                ycurs + gl_smallFont.h + 7. + offy, NULL );
            } else
               gl_renderScaleAspect( cell->image, xcurs + 5.,
                                     ycurs + gl_smallFont.h + 7.,
                                     iar->dat.iar.iw, iar->dat.iar.ih, NULL );
         }

         /* layers */
         for ( int k = 0; k < array_size( cell->layers ); k++ ) {

            if ( cell->layers[k] != NULL )
               gl_renderScaleAspect( cell->layers[k], xcurs + 5.,
                                     ycurs + gl_smallFont.h + 7.,
                                     iar->dat.iar.iw, iar->dat.iar.ih, NULL );
         }

         /* caption */
         if ( cell->caption != NULL )
            gl_printMidRaw( &gl_smallFont, iar->dat.iar.iw, xcurs + 5.,
                            ycurs + 5., fontcolour, -1., cell->caption );

         /* quantity. */
         if ( cell->quantity > 0 ) {
            /* Quantity number. */
            gl_printMax( &gl_smallFont, iar->dat.iar.iw, xcurs + 5.,
                         ycurs + iar->dat.iar.ih + 4., fontcolour, "%d",
                         cell->quantity );
         }

         /* Slot type. */
         if ( cell->sloticon != NULL ) {
            double sw = 18.;
            double sh = 18.;
            double sx = xcurs + iar->dat.iar.iw - 10.;
            double sy = ycurs + iar->dat.iar.ih + 2.;

            if ( cell->sloticon->flags & OPENGL_TEX_SDF )
               gl_renderSDF( cell->sloticon, sx, sy, sw, sh, &cWhite, 0., 1. );
            else
               gl_renderScaleAspect( cell->sloticon, sx, sy, sw, sh, NULL );
         } else if ( cell->slottype != NULL ) {
            /* Slot size letter. */
            gl_printMaxRaw(
               &gl_smallFont, iar->dat.iar.iw, xcurs + iar->dat.iar.iw - 10.,
               ycurs + iar->dat.iar.ih + 4., fontcolour, -1., cell->slottype );
         }

         /* outline */
         if ( is_selected ) {
            lc = &cWhite;
            dc = &cGrey60;
         } else {
            lc = toolkit_colLight;
            dc = toolkit_col;
         }
         gl_renderRect( xcurs, ycurs, w - 2., h - 2., 2., dc );
         gl_renderRect( xcurs, ycurs, w, h, 1., lc );
      }
   }
   gl_unclipRect();
}

/**
 * @brief Renders the overlay.
 */
static void iar_renderOverlay( Widget *iar, double bx, double by )
{
   double      x, y;
   const char *alt;

   /*
    * Draw Alt text if applicable.
    */
   if ( ( iar->dat.iar.alt >= 0 ) && ( iar->dat.iar.altx != -1 ) &&
        ( iar->dat.iar.alty != -1 ) ) {

      /* Calculate position. */
      x = bx + iar->x + iar->dat.iar.altx;
      y = by + iar->y + iar->dat.iar.alty;

      /* Draw alt text. */
      alt = iar->dat.iar.images[iar->dat.iar.alt].alt;
      if ( alt != NULL )
         toolkit_drawAltText( x, y, alt );
   }
}

/**
 * @brief Handles input for an image array widget.
 *
 *    @param iar Image array widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @param isrepeat Whether or not the key is repeating.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int iar_key( Widget *iar, SDL_Keycode key, SDL_Keymod mod, int isrepeat )
{
   (void)mod;
   (void)isrepeat;

   switch ( key ) {
   case SDLK_KP_PLUS:
   case SDLK_PLUS:
      iar->dat.iar.zoom *= 1.1;
      iar->dat.iar.pos *= 1.1;
      iar_updateSpacing( iar );
      iar_scroll( iar, 0 ); /* Does boundary checks. */
      break;
   case SDLK_KP_MINUS:
   case SDLK_MINUS:
      iar->dat.iar.zoom *= 1.0 / 1.1;
      iar->dat.iar.pos *= 1.0 / 1.1;
      iar_updateSpacing( iar );
      iar_scroll( iar, 0 ); /* Does boundary checks. */
      break;
   case SDLK_PAGEUP:
   case SDLK_UP:
      iar->dat.iar.selected -= iar->dat.iar.xelem;
      break;
   case SDLK_PAGEDOWN:
   case SDLK_DOWN:
      iar->dat.iar.selected += iar->dat.iar.xelem;
      break;
   case SDLK_RIGHT:
      iar->dat.iar.selected += 1;
      break;
   case SDLK_LEFT:
      iar->dat.iar.selected -= 1;
      break;

   case SDLK_RETURN:
   case SDLK_KP_ENTER:
      if ( iar->dat.iar.accept != NULL ) {
         iar->dat.iar.accept( iar->wdw, iar->name );
         return 1;
      }
      FALLTHROUGH;

   default:
      return 0;
   }

   /* Check boundaries. */
   iar->dat.iar.selected =
      CLAMP( 0, iar->dat.iar.nelements - 1, iar->dat.iar.selected );

   /* Run function pointer if needed. */
   if ( iar->dat.iar.fptr )
      iar->dat.iar.fptr( iar->wdw, iar->name );

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
   int    y;
   double h, hmax, yspace;

   /* Get dimensions. */
   iar_getDim( iar, NULL, &h, NULL, &yspace );
   hmax = iar_maxPos( iar );

   /* Move if needed. */
   if ( hmax == 0. || iar->dat.iar.selected < 0 )
      return;

   y                = iar->dat.iar.selected / iar->dat.iar.xelem;
   iar->dat.iar.pos = CLAMP( ( y + 1 ) * ( h + yspace ) - ( iar->h - yspace ),
                             y * ( h + yspace ), iar->dat.iar.pos );
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );

   iar_setAltTextPos( iar, iar->dat.iar.altx, iar->dat.iar.alty );
}

/**
 * @brief Image array widget mouse click handler.
 *
 *    @param iar Widget receiving the event.
 *    @return 1 if event is used.
 */
static int iar_mclick( Widget *iar, int button, int x, int y )
{
   /* Handle different mouse clicks. */
   switch ( button ) {
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
      if ( iar->dat.iar.rmptr != NULL )
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
 *    @TODO It would be more precise to record the iar_focus result for
 *          every click, so rapidly clicking an icon, out of bounds, and
 *          the same icon wouldn't register as a double-click.
 */
static int iar_mdoubleclick( Widget *iar, int button, int x, int y )
{
   /* Update mouse position. */
   iar->dat.iar.mx = x;
   iar->dat.iar.my = y;

   /* Handle different mouse clicks. */
   iar_setAltTextPos( iar, x, y );
   switch ( button ) {
   case SDL_BUTTON_LEFT:
      if ( iar->dat.iar.dblptr != NULL && iar->dat.iar.selected >= 0 &&
           iar->dat.iar.selected == iar_focusImage( iar, x, y ) ) {
         iar->dat.iar.dblptr( iar->wdw, iar->name );
         return 1;
      }

   default:
      break;
   }
   return iar_mclick( iar, button, x, y );
}

/**
 * @brief Handler for mouse wheel events for an image array.
 *
 *    @param iar The widget handling the mouse wheel event.
 *    @param event The event the widget should handle.
 *    @return 1 if the widget uses the event.
 */
static int iar_mwheel( Widget *iar, SDL_MouseWheelEvent event )
{
   if ( SDL_GetModState() & ( KMOD_LCTRL | KMOD_RCTRL ) ) {
      double zoom;
      if ( event.y > 0 )
         zoom = 1.1;
      else
         zoom = 1.0 / 1.1;
      iar->dat.iar.zoom *= zoom;
      iar->dat.iar.pos *= zoom;
      iar_updateSpacing( iar );
      iar_scroll( iar, 0 ); /* Does boundary checks. */
   } else {
      if ( event.y > 0 )
         iar_scroll( iar, +1 );
      else if ( event.y < 0 )
         iar_scroll( iar, -1 );
   }

   return 1;
}

/**
 * @brief Handles mouse movement for an image array.
 *
 *    @param iar Widget handling the mouse motion.
 *    @param mmove Mouse motion event to handle.
 *    @return 1 if the event is used.
 */
static int iar_mmove( Widget *iar, int x, int y, int rx, int ry )
{
   (void)rx;
   (void)ry;

   /* Update mouse position. */
   iar->dat.iar.mx = x;
   iar->dat.iar.my = y;

   if ( iar->status == WIDGET_STATUS_SCROLLING ) {
      double hmax      = iar_maxPos( iar );
      y                = CLAMP( 15, iar->h - 15., iar->h - y );
      iar->dat.iar.pos = ( y - 15. ) * hmax / ( iar->h - 30. );

      /* Does boundary checks. */
      iar_scroll( iar, 0 );

      return 1;
   } else {
      if ( ( x < 0 ) || ( x >= iar->w ) || ( y < 0 ) || ( y >= iar->h ) )
         iar->dat.iar.alt = -1;
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
static void iar_cleanup( Widget *iar )
{
   for ( int i = 0; i < iar->dat.iar.nelements; i++ ) {
      ImageArrayCell *cell = &iar->dat.iar.images[i];
      gl_freeTexture( cell->image );
      free( cell->caption );
      free( cell->alt );
      free( cell->slottype );

      for ( int j = 0; j < array_size( cell->layers ); j++ )
         gl_freeTexture( cell->layers[j] );
      array_free( cell->layers );
   }
   free( iar->dat.iar.images );
}

/**
 * @brief Tries to scroll a widget up/down by direction.
 *
 *    @param wgt Widget to scroll.
 *    @param direction Direction to scroll. Positive is up, negative
 *           is down and absolute value is number of elements to scroll.
 */
static void iar_scroll( Widget *iar, int direction )
{
   double h, yspace;
   double hmax;

   if ( iar == NULL )
      return;

   /* element dimensions */
   iar_getDim( iar, NULL, &h, NULL, &yspace );

   /* maximum */
   hmax = iar_maxPos( iar );
   if ( hmax < 0. )
      hmax = 0.;

   /* move */
   iar->dat.iar.pos -= direction * ( h + yspace );

   /* Boundary check. */
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );
   if ( iar->dat.iar.fptr )
      iar->dat.iar.fptr( iar->wdw, iar->name );

   if ( direction != 0 )
      iar_mmove( iar, iar->dat.iar.mx, iar->dat.iar.my, 0, 0 );
}

/**
 * @brief Return the widget's maximum y position (.pos); this is 0 if all
 * content fits.
 */
static double iar_maxPos( Widget *iar )
{
   double h, yspace, hmax;
   iar_getDim( iar, NULL, &h, NULL, &yspace );
   hmax = ( h + yspace ) * iar->dat.iar.yelem + yspace - iar->h;
   return hmax < 1e-05 ? 0. : hmax;
}

/**
 * @brief See what widget is being focused.
 */
static int iar_focusImage( Widget *iar, double bx, double by )
{
   int    ix, iy;
   double w, h, xspace, yspace, gx, gy;
   int    xelem;

   /* element dimensions */
   iar_getDim( iar, &w, &h, &xspace, &yspace );

   /* number of elements */
   xelem = iar->dat.iar.xelem;

   /* Coordinates within the grid relative to element #0 */
   gx = bx;
   gy = iar->h - by + iar->dat.iar.pos;

   ix = gx / ( xspace + w );
   iy = gy / ( yspace + h );

   /* Reject anything too close to the scroll bar or exceeding nelements. */
   if ( iy * xelem + ix >= iar->dat.iar.nelements || bx >= iar->w - 10. )
      return -1;

   /* Verify that the mouse is on an icon. */
   if ( ( gx < ( ix + 1 ) * xspace + ix * w ) ||
        ( gx > ( ix + 1 ) * ( xspace + w ) - 4. ) ||
        ( gy < ( iy + 1 ) * yspace + iy * h ) ||
        ( gy > ( iy + 1 ) * ( yspace + h ) - 4. ) )
      return -1;

   return iy * xelem + ix;
}

/**
 * @brief Mouse event focus on image array.
 *
 *    @param iar Image Array widget.
 *    @param bx X position click.
 *    @param by Y position click.
 */
static void iar_focus( Widget *iar, double bx, double by )
{
   /* Test for item click. */
   int selected = iar_focusImage( iar, bx, by );
   if ( selected >= 0 ) {
      iar->dat.iar.selected = selected;
      if ( iar->dat.iar.fptr != NULL )
         iar->dat.iar.fptr( iar->wdw, iar->name );
   }
   /* Scrollbar click. */
   else if ( bx > iar->w - 10. ) {
      double scroll_pos, hmax, y;
      /* Get bar position (center). */
      hmax = iar_maxPos( iar );
      if ( hmax == 0. )
         scroll_pos = 0.;
      else
         scroll_pos = iar->dat.iar.pos / hmax;
      y = iar->h - ( iar->h - 30. ) * scroll_pos - 15.;

      /* Click below the bar. */
      if ( by < y - 15. )
         iar_scroll( iar, -2 );
      /* Click above the bar. */
      else if ( by > y + 15. )
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
static Widget *iar_getWidget( unsigned int wid, const char *name )
{
   Widget *wgt = window_getwgt( wid, name );

   /* Must be found in stack. */
   if ( wgt == NULL ) {
      WARN( "Widget '%s' not found", name );
      return NULL;
   }

   /* Must be an image array. */
   if ( wgt->type != WIDGET_IMAGEARRAY ) {
      WARN( "Widget '%s' is not an image array.", name );
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
static const char *toolkit_getNameById( Widget *wgt, int elem )
{
   if ( wgt == NULL )
      return NULL;

   /* Nothing selected. */
   if ( elem == -1 )
      return NULL;

   return wgt->dat.iar.images[elem].caption;
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
const char *toolkit_getImageArray( unsigned int wid, const char *name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL || wgt->dat.iar.selected < 0 )
      return NULL;

   return toolkit_getNameById( wgt, wgt->dat.iar.selected );
}

/*
 * @brief Sets an image array based on value.
 *
 *   \warning If the captions have been translated or otherwise preprocessed,
 *            this function can only find a name that has been transformed the
 *            same way. There may be a more robust solution involving indices.
 *            \see toolkit_setImageArrayPos
 */
int toolkit_setImageArray( unsigned int wid, const char *name,
                           const char *elem )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   /* Case NULL. */
   if ( elem == NULL ) {
      wgt->dat.iar.selected = -1;
      return 0;
   }

   /* Try to find the element. */
   for ( int i = 0; i < wgt->dat.iar.nelements; i++ ) {
      if ( strcmp( elem, wgt->dat.iar.images[i].caption ) == 0 ) {
         wgt->dat.iar.selected = i;
         iar_centerSelected( wgt );
         return 0;
      }
   }

   /* Element not found. */
   return -1;
}

/**
 * @brief Gets the zoom level of an image array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @return The zoom of selected object.
 */
double toolkit_getImageArrayZoom( unsigned int wid, const char *name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1.;

   return wgt->dat.iar.zoom;
}

/**
 * @brief Sets the zoom level of an image array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @param zoom Zoom to set.
 *    @return 0 on success
 */
int toolkit_setImageArrayZoom( unsigned int wid, const char *name, double zoom )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   wgt->dat.iar.zoom = zoom;
   iar_updateSpacing( wgt );
   return 0;
}

/**
 * @brief Gets what is selected currently in an Image Array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @return The position of selected object.
 */
int toolkit_getImageArrayPos( unsigned int wid, const char *name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   return wgt->dat.iar.selected;
}

/**
 * @brief Gets the Image Array offset.
 */
double toolkit_getImageArrayOffset( unsigned int wid, const char *name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1.;

   return wgt->dat.iar.pos;
}

/**
 * @brief Sets the Image Array offset.
 */
int toolkit_setImageArrayOffset( unsigned int wid, const char *name,
                                 double off )
{
   double hmax;

   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   /* Get dimensions. */
   hmax = iar_maxPos( wgt );

   /* Ignore fancy stuff if smaller than height. */
   if ( hmax == 0. ) {
      wgt->dat.iar.pos = 0.;
      return 0;
   }

   /* Move if needed. */
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
int toolkit_setImageArrayPos( unsigned int wid, const char *name, int pos )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   /* Set position. */
   wgt->dat.iar.selected = CLAMP( 0, wgt->dat.iar.nelements - 1, pos );

   /* Call callback - dangerous if called from within callback. */
   if ( wgt->dat.iar.fptr != NULL )
      wgt->dat.iar.fptr( wgt->wdw, wgt->name );

   iar_centerSelected( wgt );

   return 0;
}

/**
 * @brief Stores several image array attributes.
 *
 *    @param wid Window containing the image array.
 *    @param name Name of the image array widget.
 *    @param iar_data Pointer to an iar_data_t struct to save to.
 *    @return 0 on success.
 */
int toolkit_saveImageArrayData( unsigned int wid, const char *name,
                                iar_data_t *iar_data )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   iar_data->pos    = wgt->dat.iar.selected;
   iar_data->offset = wgt->dat.iar.pos;
   iar_data->zoom   = wgt->dat.iar.zoom;

   return 0;
}

/**
 * @brief Loads several image array attributes.
 *
 *    @param wid Window containing the image array.
 *    @param name Name of the image array widget.
 *    @param iar_data Pointer to an iar_data_t struct to load.
 *    @return 0 on success.
 */
int toolkit_loadImageArrayData( unsigned int wid, const char *name,
                                const iar_data_t *iar_data )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return -1;

   wgt->dat.iar.selected = iar_data->pos;
   wgt->dat.iar.pos      = iar_data->offset;
   wgt->dat.iar.zoom     = iar_data->zoom;
   iar_updateSpacing( wgt ); /* Potentially can be necessary if zoom changes. */

   return 0;
}

/**
 * @brief Initializes the image array data.
 */
void toolkit_initImageArrayData( iar_data_t *iar_data )
{
   iar_data->pos    = 0;
   iar_data->offset = 0;
   iar_data->zoom   = 1.0;
}

/**
 * @brief Unsets the selection
 *
 *    @param wid Window containing the image array.
 *    @param name Name of the image array widget.
 */

int toolkit_unsetSelection( unsigned int wid, const char *name )
{
   Widget *wgt = iar_getWidget( wid, name );

   /* unset the selection */
   wgt->dat.iar.selected = -1;

   return 0;
}

/**
 * @brief Sets the accept function of an Image Array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @param fptr Accept function to set.
 */
void toolkit_setImageArrayAccept( unsigned int wid, const char *name,
                                  void ( *fptr )( unsigned int, const char * ) )
{
   Widget *wgt = iar_getWidget( wid, name );
   if ( wgt == NULL )
      return;
   wgt->dat.iar.accept = fptr;
}

/**
 * @brief Gets the number of visible elements in an image array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @return The number of totally visible elements.
 */
int toolkit_getImageArrayVisibleElements( unsigned int wid, const char *name )
{
   Widget *iar = iar_getWidget( wid, name );
   if ( iar == NULL )
      return -1;
   return toolkit_simImageArrayVisibleElements( iar->w, iar->h, iar->dat.iar.iw,
                                                iar->dat.iar.ih );
}

/**
 * @brief Simulates the number of visible elements in an image array.
 *
 *    @param w Width.
 *    @param h Height.
 *    @param iw Image width to use.
 *    @param ih Image height to use.
 */
int toolkit_simImageArrayVisibleElements( int w, int h, int iw, int ih )
{
   int xelem = floor( ( w - 10 ) / ( iw + 10 ) );
   int yelem = floor( ( h - 10 ) / ( ih + 10 + 2 + gl_smallFont.h ) );
   return xelem * yelem;
}
