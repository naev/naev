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
static int iar_key( Widget* iar, SDLKey key, SDLMod mod );
static int iar_mclick( Widget* iar, SDL_MouseButtonEvent *mclick );
static void iar_cleanup( Widget* iar );
static void iar_scroll( Widget* iar, int direction );


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
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type   = WIDGET_IMAGEARRAY;                                   
   wgt->name   = strdup(name);
   wgt->wdw    = wid;

   /* specific */
   wgt->render             = iar_render;
   wgt->cleanup            = iar_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->keyevent           = iar_key;
   wgt->mclickevent        = iar_mclick;
   wgt->dat.iar.images     = tex;
   wgt->dat.iar.captions   = caption;
   wgt->dat.iar.nelements  = nelem;
   wgt->dat.iar.selected   = 0;
   wgt->dat.iar.pos        = 0;
   wgt->dat.iar.iw         = iw;
   wgt->dat.iar.ih         = ih;
   wgt->dat.iar.fptr       = call;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
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
   int xelem,yelem, xspace;
   glColour *c, *dc, *lc;
   int is_selected;

   /*
    * Calculations.
    */
   /* position */
   x = bx + iar->x;
   y = by + iar->y;
                                                                    
   /* element dimensions */                                         
   w = iar->dat.iar.iw + 5.*2.; /* includes border */
   h = iar->dat.iar.ih + 5.*2. + 2. + gl_smallFont.h;

   /* number of elements */
   xelem = (int)((iar->w - 10.) / w);
   xspace = (((int)iar->w - 10) % (int)w) / (xelem + 1);
   yelem = (int)iar->dat.iar.nelements / xelem + 1;

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
         gl_printMid( &gl_smallFont, iar->dat.iar.iw, xcurs + 5., ycurs + 5.,
                  (is_selected) ? &cBlack : &cWhite,
                  iar->dat.iar.captions[j*xelem + i] );

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

   switch (key) {
      case SDLK_UP:
         iar_scroll( iar, +1 );
         return 1;
      case SDLK_DOWN:
         iar_scroll( iar, -1 );
         return 1;

      default:
         break;
   }
   return 0;
}


/**
 * @brief Image array widget mouse click handler.
 *
 *    @param iar Widget recieving the event.
 *    @param mclick The mouse click event.
 *    @return 1 if event is used.
 */
static int iar_mclick( Widget* iar, SDL_MouseButtonEvent *mclick )
{
   switch (mclick->button) {
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
 * @brief Clean up function for the image array widget.
 *
 *    @param iar Image array widget to clean up.
 */
static void iar_cleanup( Widget* iar )
{
   int i;

   if (iar->dat.iar.nelements > 0) { /* Free each text individually */
      for (i=0; i<iar->dat.iar.nelements; i++)
         if (iar->dat.iar.captions[i])
            free(iar->dat.iar.captions[i]);
      /* Free the arrays */
      free( iar->dat.iar.captions );
      free( iar->dat.iar.images );
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
   Window *wdw;
   
   if (iar == NULL) return;
   
   wdw = toolkit_getActiveWindow();
   
   /* element dimensions */
   w = iar->dat.iar.iw + 5.*2.; /* includes border */
   h = iar->dat.iar.ih + 5.*2. + 2. + gl_smallFont.h;

   /* number of elements */
   xelem = (int)((iar->w - 10.) / w);
   yelem = (int)iar->dat.iar.nelements / xelem + 1;

   /* maximum */
   hmax = h * (yelem - (int)(iar->h / h));
   if (hmax < 0.)
      hmax = 0.;

   /* move */
   iar->dat.iar.pos -= direction * h;

   /* Boundry check. */
   iar->dat.iar.pos = CLAMP( 0., hmax, iar->dat.iar.pos );
   if (iar->dat.iar.fptr)
      (*iar->dat.iar.fptr)(wdw->id,iar->name);
}

