/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file imagearray.c
 *
 * @brief Image array widget.
 */


#include "tk/toolkit_priv.h"


static void iar_render( Widget* iar, double bx, double by );
static void iar_renderOverlay( Widget* iar, double bx, double by );
static int iar_key( Widget* iar, SDLKey key, SDLMod mod );
static int iar_mclick( Widget* iar, int button, int x, int y );
static int iar_mmove( Widget* iar, int x, int y, int rx, int ry );
static void iar_cleanup( Widget* iar );
static int iar_focusImage( Widget* iar, double bx, double by );
static void iar_focus( Widget* iar, double bx, double by );
static void iar_scroll( Widget* iar, int direction );
static Widget *iar_getWidget( const unsigned int wid, const char *name );


/**
 * @brief Adds an Image Array widget.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid Window to add to.
 *    @param x X position.
 *    @param y Y position.
 *    @param w Width.
 *    @param h Height.
 *    @param name Internal widget name.
 *    @param iw Image width to use.
 *    @param ih Image height to use.
 *    @param tex Texture array to use (not freed).
 *    @param caption Caption array to use (freed).
 *    @param nelem Elements in tex and caption.
 *    @param call Callback when modified.
 */
void window_addImageArray( const unsigned int wid,
                           const int x, const int y, /* position */
                           const int w, const int h, /* size */
                           char* name, const int iw, const int ih,
                           glTexture** tex, char** caption, int nelem,
                           void (*call) (unsigned int wdw, char* wgtname) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);

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
   wgt->mmoveevent         = iar_mmove;
   wgt->dat.iar.images     = tex;
   wgt->dat.iar.captions   = caption;
   wgt->dat.iar.nelements  = nelem;
   wgt->dat.iar.selected   = 0;
   wgt->dat.iar.pos        = 0;
   wgt->dat.iar.alt        = -1;
   wgt->dat.iar.iw         = iw;
   wgt->dat.iar.ih         = ih;
   wgt->dat.iar.fptr       = call;
   wgt->dat.iar.xelem      = floor((w - 10.) / (double)(wgt->dat.iar.iw+10));
   wgt->dat.iar.yelem      = (wgt->dat.iar.xelem == 0) ? 0 :
         (int)wgt->dat.iar.nelements / wgt->dat.iar.xelem + 1;

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
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
   int i,j;
   double x,y, w,h, xcurs,ycurs;
   double scroll_pos;
   int xelem, yelem;
   double xspace;
   glColour *c, *dc, *lc;
   int is_selected;

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
   scroll_pos = iar->dat.iar.pos / (h * (yelem - (int)(iar->h / h)));
   toolkit_drawScrollbar( x + iar->w - 10., y, 10., iar->h, scroll_pos );

   /*
    * Main drawing loop.
    */
   toolkit_clip( x, y, iar->w, iar->h );
   ycurs = y + iar->h + (double)SCREEN_H/2. - h + iar->dat.iar.pos;
   for (j=0; j<yelem; j++) {
      xcurs = x + xspace + (double)SCREEN_W/2.;
      for (i=0; i<xelem; i++) {

         /* Out of elements. */
         if ((j*xelem + i) >= iar->dat.iar.nelements)
            break;

         is_selected = (iar->dat.iar.selected == j*xelem + i) ? 1 : 0;

         if (is_selected)
            toolkit_drawRect( xcurs-(double)SCREEN_W/2. + 2.,
                  ycurs-(double)SCREEN_H/2. + 2.,
                  w - 4., h - 4., &cDConsole, NULL );

         /* image */
         if (iar->dat.iar.images[j*xelem + i] != NULL)
            gl_blitScale( iar->dat.iar.images[j*xelem + i],
                  xcurs + 5., ycurs + gl_smallFont.h + 7.,
                  iar->dat.iar.iw, iar->dat.iar.ih, NULL );

         /* caption */
         gl_printMidRaw( &gl_smallFont, iar->dat.iar.iw, xcurs + 5., ycurs + 5.,
                  (is_selected) ? &cBlack : &cWhite,
                  iar->dat.iar.captions[j*xelem + i] );

         /* quantity. */
         if (iar->dat.iar.quantity != NULL) {
            if (iar->dat.iar.quantity[j*xelem + i] != NULL) {
               gl_printMaxRaw( &gl_smallFont, iar->dat.iar.iw,
                     xcurs + 5., ycurs + iar->dat.iar.ih + 7.,
                     &cWhite, iar->dat.iar.quantity[j*xelem + i] );
            }
         }

         /* outline */
         if (is_selected) {
            lc = &cWhite;
            c = &cGrey80;
            dc = &cGrey60;
         }
         else {
            lc = toolkit_colLight;
            c = toolkit_col;
            dc = toolkit_colDark;
         }
         toolkit_drawOutline( xcurs-(double)SCREEN_W/2. + 2.,
               ycurs-(double)SCREEN_H/2. + 2.,
               w - 4., h - 4., 1., lc, c );
         toolkit_drawOutline( xcurs-(double)SCREEN_W/2. + 2.,
               ycurs-(double)SCREEN_H/2. + 2.,
               w - 4., h - 4., 2., dc, NULL );
         xcurs += w + xspace;
      }
      ycurs -= h;
   }
   toolkit_unclip();

   /*
    * Final outline.
    */
   toolkit_drawOutline( x, y+1, iar->w-1, iar->h-1, 1., toolkit_colLight, toolkit_col );
   toolkit_drawOutline( x, y+1, iar->w-1, iar->h-1, 2., toolkit_colDark, NULL );
}


/**
 * @brief Renders the overlay.
 */
static void iar_renderOverlay( Widget* iar, double bx, double by )
{
   /*
    * Draw Alt text if applicable.
    */
   if ((iar->dat.iar.alts != NULL) &&
         (iar->dat.iar.alt >= 0) &&
         (iar->dat.iar.alts[iar->dat.iar.alt] != NULL)) {
      /* Draw alt text. */
      toolkit_drawAltText( bx + iar->x + iar->dat.iar.altx,
            by + iar->y + iar->dat.iar.alty,
            iar->dat.iar.alts[iar->dat.iar.alt]);
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
static int iar_key( Widget* iar, SDLKey key, SDLMod mod )
{
   (void) mod;
   int x,y;
   double h;
   double hmax;
   int yscreen;
   double ypos;

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

   /* Check boundries. */
   iar->dat.iar.selected = CLAMP( 0, iar->dat.iar.nelements-1, iar->dat.iar.selected);

   /* Move if needed. */
   iar_getDim( iar, NULL, &h );
   hmax = h * (iar->dat.iar.yelem - (int)(iar->h / h));
   yscreen = (double)iar->h / h;
   x = iar->dat.iar.selected % iar->dat.iar.xelem;
   y = iar->dat.iar.selected / iar->dat.iar.xelem;
   ypos = y * h;
   /* Below. */
   if (ypos-h < iar->dat.iar.pos)
      iar->dat.iar.pos += (ypos-h) - iar->dat.iar.pos;
   /* Above. */
   if (ypos+2.*h > iar->dat.iar.pos + iar->h)
      iar->dat.iar.pos += (ypos+2.*h) - (iar->dat.iar.pos + iar->h);
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );

   /* Run function pointer if needed. */
   if (iar->dat.iar.fptr)
      iar->dat.iar.fptr( iar->wdw, iar->name);

   return 1;
}


/**
 * @brief Image array widget mouse click handler.
 *
 *    @param iar Widget recieving the event.
 *    @param mclick The mouse click event.
 *    @return 1 if event is used.
 */
static int iar_mclick( Widget* iar, int button, int x, int y )
{
   /* Handle different mouse clicks. */
   switch (button) {
      case SDL_BUTTON_LEFT:
         iar_focus( iar, x, y );
         return 1;
      case SDL_BUTTON_WHEELUP:
         iar_scroll( iar, +1 );
         return 1;
      case SDL_BUTTON_WHEELDOWN:
         iar_scroll( iar, -1 );
         return 1;

      default:
         break;
   }
   return 0;
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
   double w,h;
   int xelem, yelem;
   double hmax;

   if (iar->status == WIDGET_STATUS_SCROLLING) {

      /* element dimensions */
      iar_getDim( iar, &w, &h );

      /* number of elements */
      xelem = iar->dat.iar.xelem;
      yelem = iar->dat.iar.yelem;

      hmax = h * (yelem - (int)(iar->h / h));

      iar->dat.iar.pos += ry * hmax / (iar->h - 30.);

      /* Does boundry checks. */
      iar_scroll( iar, 0 );

      return 1;
   }
   else {
      iar->dat.iar.alt = iar_focusImage( iar, x, y );
      iar->dat.iar.altx = x;
      iar->dat.iar.alty = y;
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
   int i;

   if (iar->dat.iar.nelements > 0) { /* Free each text individually */
      for (i=0; i<iar->dat.iar.nelements; i++) {
         if (iar->dat.iar.captions[i])
            free(iar->dat.iar.captions[i]);
         if (iar->dat.iar.alts && iar->dat.iar.alts[i])
            free(iar->dat.iar.alts[i]);
         if (iar->dat.iar.quantity && iar->dat.iar.quantity[i])
            free(iar->dat.iar.quantity[i]);
      }  
      /* Free the arrays */
      free( iar->dat.iar.captions );
      free( iar->dat.iar.images );
      if (iar->dat.iar.alts)
         free(iar->dat.iar.alts);
      if (iar->dat.iar.quantity)
         free(iar->dat.iar.quantity);
   }
}


/**
 * @brief Tries to scroll a widget up/down by direction.
 *
 *    @param wgt Widget to scroll.
 *    @param direction Direction to scroll.  Positive is up, negative
 *           is down and absolute value is number of elements to scroll.
 */
static void iar_scroll( Widget* iar, int direction )
{
   double w,h;
   int xelem, yelem;
   double hmax;

   if (iar == NULL)
      return;

   /* element dimensions */
   iar_getDim( iar, &w, &h );

   /* number of elements */
   xelem = iar->dat.iar.xelem;
   yelem = iar->dat.iar.yelem;

   /* maximum */
   hmax = h * (yelem - (int)(iar->h / h));
   if (hmax < 0.)
      hmax = 0.;

   /* move */
   iar->dat.iar.pos -= direction * h;

   /* Boundry check. */
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );
   if (iar->dat.iar.fptr)
      iar->dat.iar.fptr( iar->wdw, iar->name );
}


/**
 * @brief See what widget is being focused.
 */
static int iar_focusImage( Widget* iar, double bx, double by )
{
   int i,j;
   double x,y, w,h, ycurs,xcurs;
   int xelem, xspace, yelem;

   /* positions */
   x = bx + iar->x;
   y = by + iar->y;

   /* element dimensions */
   iar_getDim( iar, &w, &h );

   /* number of elements */
   xelem = iar->dat.iar.xelem;
   yelem = iar->dat.iar.yelem;
   xspace = (((int)iar->w - 10) % (int)w) / (xelem + 1);
   if (bx < iar->w - 10.) {

      /* Loop through elements until finding collision. */
      ycurs = iar->h - h + iar->dat.iar.pos;
      for (j=0; j<yelem; j++) {
         xcurs = xspace;
         for (i=0; i<xelem; i++) {
            /* Out of elements. */
            if ((j*xelem + i) >= iar->dat.iar.nelements)
               break;

            /* Check for collision. */
            if ((bx > xcurs) && (bx < xcurs+w-4.) &&
                  (by > ycurs) && (by < ycurs+h-4.)) {
               return j*xelem + i;
            }
            xcurs += xspace + w;
         }
         ycurs -= h;
      }
   }

   return -1;
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
   else {
      /* Get bar position (center). */
      hmax = h * (yelem - (int)(iar->h / h));
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
 * @brief Gets what is selected currently in an Image Array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @return The name of the selected object.
 */
char* toolkit_getImageArray( const unsigned int wid, const char* name )
{
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return NULL;

   /* Nothing selected. */
   if (wgt->dat.iar.selected == -1)
      return NULL;

   return wgt->dat.iar.captions[ wgt->dat.iar.selected ];
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
   wgt->dat.iar.selected = CLAMP( 0, wgt->dat.iar.nelements, pos );

   /* Call callback - dangerous if called from within callback. */
   if (wgt->dat.iar.fptr != NULL)
      wgt->dat.iar.fptr( wgt->wdw, wgt->name );

   return 0;
}


/**
 * @brief Sets the alt text for the images in the image array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @param alt Array of alt text the size of the images in the array.
 *    @return 0 on success.
 */
int toolkit_setImageArrayAlt( const unsigned int wid, const char* name, char **alt )
{
   int i;
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   /* Clean up. */
   if (wgt->dat.iar.alts != NULL) {
      for (i=0; i<wgt->dat.iar.nelements; i++)
         if (wgt->dat.iar.alts[i] != NULL)
            free(wgt->dat.iar.alts[i]);
      free(wgt->dat.iar.alts);
   }

   /* Set. */
   wgt->dat.iar.alts = alt;
   return 0;
}


/**
 * @brief Sets the quantity text for the images in the image array.
 *
 *    @param wid Window where image array is.
 *    @param name Name of the image array.
 *    @param alt Array of alt text the size of the images in the array.
 *    @return 0 on success.
 */
int toolkit_setImageArrayQuantity( const unsigned int wid, const char* name,
      char **quantity )
{
   int i;
   Widget *wgt = iar_getWidget( wid, name );
   if (wgt == NULL)
      return -1;

   /* Clean up. */
   if (wgt->dat.iar.quantity != NULL) {
      for (i=0; i<wgt->dat.iar.nelements; i++)
         if (wgt->dat.iar.quantity[i] != NULL)
            free(wgt->dat.iar.quantity[i]);
      free(wgt->dat.iar.quantity);
   }

   /* Set. */
   wgt->dat.iar.quantity = quantity;
   return 0;
}

