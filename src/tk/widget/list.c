/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file list.c
 *
 * @brief List widget.
 */


#include "tk/toolkit_priv.h"


static void lst_render( Widget* lst, double bx, double by );
static int lst_key( Widget* lst, SDLKey key, SDLMod mod ); 
static int lst_mmove( Widget* lst, SDL_MouseMotionEvent *mmove );
static void lst_cleanup( Widget* lst );

static void lst_scroll( Widget* lst, int direction );


/**
 * @brief Adds a list widget to a window.
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
 *    @param items Items in the list (will be freed automatically).
 *    @param nitems Number of items in items parameter.
 *    @param defitem Default item to select.
 *    @param call Function to call when new item is selected. Parameter passed
 *                is the name of the list.
 */
void window_addList( const unsigned int wid,
                     const int x, const int y,
                     const int w, const int h,
                     char* name, char **items, int nitems, int defitem,
                     void (*call) (unsigned int wdw, char* wgtname) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type   = WIDGET_LIST;
   wgt->name   = strdup(name);
   wgt->wdw    = wid;

   /* specific */
   wgt->render             = lst_render;
   wgt->cleanup            = lst_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->keyevent           = lst_key;
   wgt->mmoveevent         = lst_mmove;
   wgt->dat.lst.options    = items;
   wgt->dat.lst.noptions   = nitems;
   wgt->dat.lst.selected   = defitem; /* -1 would be none */
   wgt->dat.lst.pos        = 0;
   wgt->dat.lst.fptr       = call;
   
   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h - ((h % (gl_defFont.h+2)) - 2);
   toolkit_setPos( wdw, wgt, x, y );

   /* check if needs scrollbar. */
   if (2 + (nitems * (gl_defFont.h + 2)) > (int)wgt->h)
      wgt->dat.lst.height = (2 + gl_defFont.h) * nitems + 2;
   else
      wgt->dat.lst.height = 0;

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/**
 * @brief Renders a list widget.
 *
 *    @param lst List widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void lst_render( Widget* lst, double bx, double by )
{
   int i;
   double x,y, tx,ty, miny;
   double w, scroll_pos;

   w = lst->w;
   x = bx + lst->x;
   y = by + lst->y;

   /* lst bg */
   toolkit_drawRect( x, y, lst->w, lst->h, &cWhite, NULL );

   /* inner outline */
   toolkit_drawOutline( x, y, lst->w, lst->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outter outline */
   toolkit_drawOutline( x, y, lst->w, lst->h, 1., toolkit_colDark, NULL );

   /* Draw scrollbar. */
   if (lst->dat.lst.height > 0) {
      /* We need to make room for list. */
      w -= 10.;
      
      scroll_pos  = (double)(lst->dat.lst.pos * (2 + gl_defFont.h));
      scroll_pos /= (double)lst->dat.lst.height - lst->h;
      toolkit_drawScrollbar( x + lst->w - 10., y, 10., lst->h, scroll_pos );
   }

   /* draw selected */
   toolkit_drawRect( x, y - 1. + lst->h -
         (1 + lst->dat.lst.selected - lst->dat.lst.pos)*(gl_defFont.h+2.),
         w, gl_defFont.h + 2., &cHilight, NULL );

   /* draw content */
   tx = (double)SCREEN_W/2. + x + 2.;
   ty = (double)SCREEN_H/2. + y + lst->h - 2. - gl_defFont.h;
   miny = ty - lst->h + 2 + gl_defFont.h;
   y = ty - 2.;
   w -= 4;
   for (i=lst->dat.lst.pos; i<lst->dat.lst.noptions; i++) {
      gl_printMax( &gl_defFont, (int)w,
            tx, ty, &cBlack, lst->dat.lst.options[i] );
      ty -= 2 + gl_defFont.h;

      /* Check if out of bounds. */
      if (ty < miny)
         break;
   }
}


/**
 * @brief Handles input for a list widget.
 *
 *    @param lst List widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int lst_key( Widget* lst, SDLKey key, SDLMod mod )
{  
   (void) mod;
   
   switch (key) {
      case SDLK_UP:
         lst_scroll( lst, +1 );
         return 1;
      case SDLK_DOWN:
         lst_scroll( lst, -1 );
         return 1;

      default:
         break;
   }
   
   return 0;
}


/**
 * @brief Handles List movement.
 *
 *    @param lst List that has mouse movement.
 *    @param mmove Mouse movement event.
 *    @return 1 if movement was used, 0 if movement wasn't used.
 */
static int lst_mmove( Widget* lst, SDL_MouseMotionEvent *mmove )
{
   Window *wdw;
   int psel;
   double p;
   int h;

   /* Ignore mouse downs. */
   if (!(mmove->state & SDL_BUTTON(1)))
      return 0;

   /* Handle the scrolling if scrolling. */
   if (lst->status == WIDGET_STATUS_SCROLLING) {
      h = lst->h / (2 + gl_defFont.h) - 1;

      /* Save previous position. */
      psel = lst->dat.lst.pos;

      /* Find absolute position. */
      p  = (lst->h - mmove->y) / lst->h * (lst->dat.lst.height - lst->h);
      p /= (2 + gl_defFont.h);
      lst->dat.lst.pos = (int)round(p);

      /* Does boundry checks. */
      lst->dat.lst.selected = CLAMP( lst->dat.lst.pos,
            lst->dat.lst.pos+h, lst->dat.lst.selected );            

      /* Run change if position changed. */
      if (lst->dat.lst.selected != psel)
         if (lst->dat.lst.fptr) {
            wdw = toolkit_getActiveWindow(); /* get active window */
            (*lst->dat.lst.fptr)(wdw->id,lst->name);
         }

      return 1;
   }

   return 0;
}


/**
 * @brief Clean up function for the list widget.
 *
 *    @param lst List widget to clean up.
 */
static void lst_cleanup( Widget* lst )
{
   int i;

   if (lst->dat.lst.options) {
      for (i=0; i<lst->dat.lst.noptions; i++)
         if (lst->dat.lst.options[i])
            free(lst->dat.lst.options[i]);
      free( lst->dat.lst.options );
   }
}


/**
 * @brief Scrolls a list widget up/down.
 *
 *    @param lst List to scroll.
 *    @param direction Direction to scroll.  Positive is up, negative
 *           is down and absolute value is number of elements to scroll.
 */
static void lst_scroll( Widget* lst, int direction )
{
   Window *wdw;
   int pos;

   if (lst == NULL) return;

   wdw = toolkit_getActiveWindow();
          
   lst->dat.lst.selected -= direction;

   /* boundry check. */
   lst->dat.lst.selected = CLAMP( 0, lst->dat.lst.noptions-1, lst->dat.lst.selected);

   /* see if we have to scroll. */
   pos = (lst->dat.lst.selected - lst->dat.lst.pos);
   if (pos < 0) {
      lst->dat.lst.pos += pos;
      if (lst->dat.lst.pos < 0)
         lst->dat.lst.pos = 0;
   }
   else if (2 + (pos+1) * (gl_defFont.h + 2) > lst->h) {
      lst->dat.lst.pos += (2 + (pos+1) * (gl_defFont.h + 2) - lst->h) / (gl_defFont.h + 2);
   }

   if (lst->dat.lst.fptr)
      (*lst->dat.lst.fptr)(wdw->id,lst->name);
}

