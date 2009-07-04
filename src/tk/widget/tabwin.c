/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file tabwin.c
 *
 * @brief Tabbed window widget.
 */


#include "tk/toolkit_priv.h"

#include "toolkit.h"
#include "font.h"


static int tab_mouse( Widget* tab, SDL_Event *event );
static int tab_raw( Widget* tab, SDL_Event *event );
static void tab_render( Widget* tab, double bx, double by );
static void tab_cleanup( Widget* tab );


/**
 * @brief Adds a button widget to a window.
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
 *    @param ntabs Number of tabs in the widget.
 *    @param tabnames Name of the tabs in the widget.
 *    @return List of created windows.
 */
unsigned int* window_addTabbedWindow( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      const char* name, int ntabs, const char **tabnames )
{
   int i;
   Window *wdw, *wtmp;
   Widget *wgt;
   
   wdw = window_wget(wid);
   wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_TABBEDWINDOW;
   wgt->name = strdup(name);
   wgt->wdw  = wid;
   
   /* specific */
   wgt_setFlag( wgt, WGT_FLAG_RAWINPUT );
   wgt->rawevent           = tab_raw;
   wgt->render             = tab_render;
   wgt->cleanup            = tab_cleanup;
   wgt->dat.tab.ntabs      = ntabs;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   /* Copy tab information. */
   wgt->dat.tab.tabnames   = malloc( sizeof(char *) * ntabs );
   wgt->dat.tab.windows    = malloc( sizeof(unsigned int) * ntabs );
   wgt->dat.tab.namelen    = malloc( sizeof(int) * ntabs );
   for (i=0; i<ntabs; i++) {
      /* Get name and length. */
      wgt->dat.tab.tabnames[i] = strdup( tabnames[i] );
      wgt->dat.tab.namelen[i]  = gl_printWidthRaw( &gl_smallFont,
            wgt->dat.tab.tabnames[i] );
      /* Create windows. */
      wgt->dat.tab.windows[i] = window_create( tabnames[i],
            wdw->x + x, wdw->y + y + 20., wdw->w, wdw->h - 20. );
      wtmp = window_wget( wgt->dat.tab.windows[i] );
      /* Set flags. */
      window_setFlag( wtmp, WINDOW_NOFOCUS );
      window_setFlag( wtmp, WINDOW_NORENDER );
      window_setFlag( wtmp, WINDOW_NOINPUT );
   }

   /* Return list of windows. */
   return wgt->dat.tab.windows;
}


/**
 * @brief Handles input for an button widget.
 *
 *    @param tab Tabbed Window widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int tab_raw( Widget* tab, SDL_Event *event )
{
   Window *wdw;

   /* First handle event internally. */
   if (event->type == SDL_MOUSEBUTTONDOWN)
      tab_mouse( tab, event );


   /* Give event to window. */
   wdw = window_wget( tab->dat.tab.windows[ tab->dat.tab.active ] );
   if (wdw == NULL) {
      WARN("Active window in widget '%s' not found in stack.", tab->name);
      return 0;
   }

   /* Give the active window the input. */
   toolkit_inputWindow( wdw, event );
   return 0; /* Never block event. */
}


/**
 * @brief Handles mouse events.
 */
static int tab_mouse( Widget* tab, SDL_Event *event )
{
   int i, p;
   Window *parent;
   int x, y, rx, ry;
   Uint8 type;

   /* Get parrent window. */
   parent = window_wget( tab->wdw );
   if (parent == NULL)
      return 0;

   /* Convert to window space. */
   type = toolkit_inputTranslateCoords( parent, event, &x, &y, &rx, &ry );

   /* Translate to widget space. */
   x += parent->w - tab->x;
   y += parent->h - tab->y;

   /* Make sure event is in bottom 20 pixels. */
   if ((y>=20) || (y<0))
      return 0;

   /* Handle event. */
   p = 0;
   for (i=0; i<tab->dat.tab.ntabs; i++) {
      p += 10 + tab->dat.tab.namelen[i];
      /* Mark as active. */
      if (x < p) {
         tab->dat.tab.active = i;
         break;
      }
   }

   return 0;
}


/**
 * @brief Renders a button widget.
 *
 *    @param tab WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void tab_render( Widget* tab, double bx, double by )
{
   int i, x, h;
   Window *wdw;
   glColour *c, *lc;

   /** Get window. */
   wdw = window_wget( tab->dat.tab.windows[ tab->dat.tab.active ] );
   if (wdw == NULL) {
      WARN("Active window in widget '%s' not found in stack.", tab->name);
      return;
   }

   /* Render the active window. */
   window_render( wdw );

   /* Render tabs ontop. */
   x = 0;
   h = 20;
   for (i=0; i<tab->dat.tab.ntabs; i++) {
      if (i==tab->dat.tab.active) {
         lc = toolkit_colLight;
         c  = toolkit_col;
      }
      else {
         lc = toolkit_col;
         c  = toolkit_colDark;
      }
      /* Draw border. */
      toolkit_drawRect( bx+x, by+0, tab->dat.tab.namelen[i] + 10, h, c, lc );
      toolkit_drawOutline( bx+x, by+0, tab->dat.tab.namelen[i] + 10, h, 1., &cBlack, c );
      /* Draw text. */
      gl_printRaw( &gl_smallFont, bx+x + 5 + SCREEN_W/2,
            by + (h-gl_smallFont.h)/2 + SCREEN_H/2, &cBlack,
            tab->dat.tab.tabnames[i] );

      /* Go to next line. */
      x += 10 + tab->dat.tab.namelen[i];
   }
}


/**
 * @brief Clean up function for the button widget.
 *
 *    @param tab Tabbed Window to clean up.
 */
static void tab_cleanup( Widget *tab )
{
   int i;
   for (i=0; i<tab->dat.tab.ntabs; i++) {
      free( tab->dat.tab.tabnames[i] );
      window_destroy( tab->dat.tab.windows[i] );
   }
   if (tab->dat.tab.tabnames != NULL)
      free( tab->dat.tab.tabnames );
   if (tab->dat.tab.windows != NULL)
      free( tab->dat.tab.windows );
   if (tab->dat.tab.namelen != NULL)
      free( tab->dat.tab.namelen );
}
