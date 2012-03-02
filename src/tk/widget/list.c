/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file list.c
 *
 * @brief List widget.
 */


#include "tk/toolkit_priv.h"

#include <stdlib.h>
#include "nstring.h"


static void lst_render( Widget* lst, double bx, double by );
static int lst_key( Widget* lst, SDLKey key, SDLMod mod );
static int lst_mclick( Widget* lst, int button, int x, int y );
static int lst_mmove( Widget* lst, int x, int y, int rx, int ry );
static void lst_cleanup( Widget* lst );

static Widget *lst_getWgt( const unsigned int wid, const char* name );
static int lst_focus( Widget* lst, double bx, double by );
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
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_LIST;

   /* specific */
   wgt->render             = lst_render;
   wgt->cleanup            = lst_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->keyevent           = lst_key;
   wgt->mclickevent        = lst_mclick;
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
      toolkit_nextFocus( wdw );

   if (defitem >= 0 && call)
      call(wid, name);
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
   toolkit_drawRect( x, y, lst->w, lst->h, &cGrey90, NULL );

   /* inner outline */
   toolkit_drawOutline( x, y, lst->w, lst->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outer outline */
   toolkit_drawOutline( x, y, lst->w, lst->h, 1., toolkit_colDark, NULL );

   /* Draw scrollbar. */
   if (lst->dat.lst.height > 0) {
      /* We need to make room for list. */
      w -= 11.;

      scroll_pos  = (double)(lst->dat.lst.pos * (2 + gl_defFont.h));
      scroll_pos /= (double)lst->dat.lst.height - lst->h;
      /* XXX lst->h is off by one */
      toolkit_drawScrollbar( x + lst->w - 12. + 1, y -1, 12., lst->h + 2, scroll_pos );
   }

   /* draw selected */
   toolkit_drawRect( x, y - 1. + lst->h -
         (1 + lst->dat.lst.selected - lst->dat.lst.pos)*(gl_defFont.h+2.),
         w-1, gl_defFont.h + 2., &cHilight, NULL );

   /* draw content */
   tx = x + 2.;
   ty = y + lst->h - 2. - gl_defFont.h;
   miny = ty - lst->h + 2 + gl_defFont.h;
   y = ty - 2.;
   w -= 4;
   for (i=lst->dat.lst.pos; i<lst->dat.lst.noptions; i++) {
      gl_printMaxRaw( &gl_defFont, (int)w,
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
 * @brief Handler for mouse click events for the list widget.
 *
 *    @param lst The widget handling the mouse click event.
 *    @param mclick The event the widget should handle.
 *    @return 1 if the widget uses the event.
 */
static int lst_mclick( Widget* lst, int button, int x, int y )
{
   switch (button) {
      case SDL_BUTTON_LEFT:
         lst_focus( lst, x, y );
         return 1;
      case SDL_BUTTON_WHEELUP:
         lst_scroll( lst, +5 );
         return 1;
      case SDL_BUTTON_WHEELDOWN:
         lst_scroll( lst, -5 );
         return 1;

      default:
         break;
   }
   return 0;
}


/**
 * @brief Handles a mouse click focusing on widget.
 *
 *    @param lst Widget to focus.
 *    @param bx Base x click.
 *    @param by Base y click.
 *    @return 1 if event was used.
 */
static int lst_focus( Widget* lst, double bx, double by )
{
   int i;
   double y, w;
   double scroll_pos;

   /* Get the actual width. */
   w = lst->w;
   if (lst->dat.lst.height > 0)
      w -= 10.;

   if (bx < w) {
      i = lst->dat.lst.pos + (lst->h - by) / (gl_defFont.h + 2.);
      if (i < lst->dat.lst.noptions) { /* shouldn't be out of boundaries */
         lst->dat.lst.selected = i;
         lst_scroll( lst, 0 ); /* checks boundaries and triggers callback */
      }
   }
   else {
      /* Get bar position (center). */
      scroll_pos  = (double)(lst->dat.lst.pos * (2 + gl_defFont.h));
      scroll_pos /= (double)lst->dat.lst.height - lst->h;
      y = (lst->h - 30.) * (1.-scroll_pos) + 15.;

      /* Click below the bar. */
      if (by < y-15.)
         lst_scroll( lst, -5 );
      /* Click above the bar. */
      else if (by > y+15.)
         lst_scroll( lst, +5 );
      /* Click on the bar. */
      else
         lst->status = WIDGET_STATUS_SCROLLING;
   }

   return 1;
}


/**
 * @brief Handles List movement.
 *
 *    @param lst List that has mouse movement.
 *    @param mmove Mouse movement event.
 *    @return 1 if movement was used, 0 if movement wasn't used.
 */
static int lst_mmove( Widget* lst, int x, int y, int rx, int ry )
{
   (void) x;
   (void) rx;
   (void) ry;
   int psel;
   double p;
   int h;

   /* Handle the scrolling if scrolling. */
   if (lst->status == WIDGET_STATUS_SCROLLING) {
      /* Make sure Y inbounds. */
      y = CLAMP( 15., lst->h-15., lst->h - y );

      h = lst->h / (2 + gl_defFont.h) - 1;

      /* Save previous position. */
      psel = lst->dat.lst.pos;

      /* Find absolute position. */
      p  = (y - 15. ) / (lst->h - 30.) * (lst->dat.lst.height - lst->h);
      p /= (2 + gl_defFont.h);
      lst->dat.lst.pos = CLAMP( 0, lst->dat.lst.noptions, (int)ceil(p) );

      /* Does boundary checks. */
      lst->dat.lst.selected = CLAMP( lst->dat.lst.pos,
            lst->dat.lst.pos+h, lst->dat.lst.selected );

      /* Run change if position changed. */
      if (lst->dat.lst.selected != psel)
         if (lst->dat.lst.fptr)
            lst->dat.lst.fptr( lst->wdw, lst->name );

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
   int pos;

   if (lst == NULL)
      return;

   lst->dat.lst.selected -= direction;

   /* boundary check. */
   lst->dat.lst.selected = CLAMP( 0, lst->dat.lst.noptions-1, lst->dat.lst.selected);

   /* see if we have to scroll. */
   pos = (lst->dat.lst.selected - lst->dat.lst.pos);
   if (pos < 0) {
      lst->dat.lst.pos += pos;
      if (lst->dat.lst.pos < 0)
         lst->dat.lst.pos = 0;
   }
   else if (2 + (pos+1) * (gl_defFont.h + 2) > lst->h)
      lst->dat.lst.pos += (2 + (pos+1) * (gl_defFont.h + 2) - lst->h) / (gl_defFont.h + 2);

   if (lst->dat.lst.fptr)
      lst->dat.lst.fptr( lst->wdw, lst->name );
}


/**
 * @brief Gets the list widget.
 */
static Widget *lst_getWgt( const unsigned int wid, const char* name )
{
   Widget *wgt = window_getwgt(wid,name);

   /* Must be in stack. */
   if (wgt == NULL) {
      WARN("Widget '%s' not found", name);
      return NULL;
   }

   /* Must be a list. */
   if (wgt->type != WIDGET_LIST) {
      WARN("Widget '%s' is not a list", name);
      return NULL;
   }

   return wgt;
}


/**
 * @brief Gets what is selected currently in a list.
 *
 * List includes Image Arrays.
 */
char* toolkit_getList( const unsigned int wid, const char* name )
{
   Widget *wgt = lst_getWgt( wid, name );
   if (wgt == NULL)
      return NULL;

   /* Nothing selected. */
   if (wgt->dat.lst.selected == -1)
      return NULL;

   return wgt->dat.lst.options[ wgt->dat.lst.selected ];
}


/**
 * @brief Sets the list value by name.
 */
char* toolkit_setList( const unsigned int wid, const char* name, char* value )
{
   int i;
   Widget *wgt = lst_getWgt( wid, name );
   if ((wgt == NULL) || (value==NULL))
      return NULL;

   for (i=0; i<wgt->dat.lst.noptions; i++) {
      if (strcmp(wgt->dat.lst.options[i],value)==0) {
         wgt->dat.lst.selected = i;
         lst_scroll( wgt, 0 ); /* checks boundaries and triggers callback */
         return value;
      }
   }

   return NULL;
}


/**
 * @brief Sets the list value by position.
 */
char* toolkit_setListPos( const unsigned int wid, const char* name, int pos )
{
   Widget *wgt = lst_getWgt( wid, name );
   if (wgt == NULL)
      return NULL;

   /* Set by pos. */
   wgt->dat.lst.selected = CLAMP( 0, wgt->dat.lst.noptions-1, pos );
   lst_scroll( wgt, 0 ); /* checks boundaries and triggers callback */
   return wgt->dat.lst.options[ wgt->dat.lst.selected ];
}


/**
 * @brief Get the position of current item in the list.
 *
 *    @param wid Window identifier where the list is.
 *    @param name Name of the list.
 *    @return The position in the list or -1 on error.
 */
int toolkit_getListPos( const unsigned int wid, const char* name )
{
   Widget *wgt = lst_getWgt( wid, name );
   if (wgt == NULL)
      return -1;

   return wgt->dat.lst.selected;
}


/**
 * @brief Gets the offset of a list.
 */
int toolkit_getListOffset( const unsigned int wid, const char* name )
{
   Widget *wgt = lst_getWgt( wid, name );
   if (wgt == NULL)
      return -1;

   return wgt->dat.lst.pos;
}


/**
 * @brief Sets the offset of a list.
 */
int toolkit_setListOffset( const unsigned int wid, const char* name, int off )
{
   Widget *wgt = lst_getWgt( wid, name );
   if (wgt == NULL)
      return -1;

   wgt->dat.lst.pos = off;
   return 0;
}



