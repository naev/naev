/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file tabwin.c
 *
 * @brief Tabbed window widget.
 */

/** @cond */
#include <stdlib.h>
/** @endcond */

#include "../../input.h" /* Hack for now. */
#include "font.h"
#include "tk/toolkit_priv.h"
#include "toolkit.h"

#define TAB_HEIGHT 35
#define TAB_HMARGIN 3
#define TAB_HPADDING 15

/*
 * Prototypes.
 */
static void    tab_expose( Widget *tab, int exposed );
static int     tab_mouse( Widget *tab, SDL_Event *event );
static int     tab_key( Widget *tab, SDL_Event *event );
static int     tab_raw( Widget *tab, SDL_Event *event );
static int     tab_scroll( Widget *tab, int dir );
static void    tab_render( Widget *tab, double bx, double by );
static void    tab_renderDynamic( Widget *tab, double bx, double by );
static void    tab_renderOverlay( Widget *tab, double bx, double by );
static void    tab_cleanup( Widget *tab );
static int     tab_getBarWidth( const Widget *wgt );
static Widget *tab_getWgt( unsigned int wid, const char *tab );

/**
 * @brief Creates a widget that hijacks a window and creates many children
 * window.
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
 *    @param tabpos Position to set up the tabs at. 0: ; 1: Bottom Centered; 2:
 * Top centered
 *    @return List of created windows.
 */
unsigned int *window_addTabbedWindow( unsigned int wid, const int x,
                                      const int y,              /* position */
                                      const int w, const int h, /* size */
                                      const char *name, int ntabs,
                                      const char **tabnames, int tabpos )
{
   int     wx, wy, ww, wh;
   Window *wdw;
   Widget *wgt;

   /* Create the Widget. */
   wdw = window_wget( wid );
   wgt = window_newWidget( wdw, name );
   if ( wgt == NULL )
      return NULL;

   /* generic */
   wgt->type = WIDGET_TABBEDWINDOW;

   /* specific */
   wgt_setFlag( wgt, WGT_FLAG_RAWINPUT );
   wgt->exposeevent           = tab_expose;
   wgt->rawevent              = tab_raw;
   wgt->render                = tab_render;
   wgt->renderDynamic         = tab_renderDynamic;
   wgt->renderOverlay         = tab_renderOverlay;
   wgt->cleanup               = tab_cleanup;
   wgt->dat.tab.parent_window = wid;
   wgt->dat.tab.ntabs         = ntabs;
   wgt->dat.tab.tabpos        = tabpos;
   wgt->dat.tab.font          = &gl_smallFont;

   /* position/size */
   wgt->x = (double)( x < 0 ) ? 0. : x;
   wgt->y = (double)( y < 0 ) ? 0. : y;
   wgt->w = (double)( w < 0 ) ? wdw->w : w;
   wgt->h = (double)( h < 0 ) ? wdw->h : h;

   /* Calculate window position and size. */
   wx = wdw->x + wgt->x;
   wy = wdw->y + wgt->y;
   ww = wgt->w;
   wh = wgt->h;
   if ( tabpos == 1 ) {
      // wy -= TAB_HEIGHT / 3;
      wy += 2 * TAB_HEIGHT / 3;
      // wh -= TAB_HEIGHT;
   } else if ( tabpos == 2 ) {
      // wy -= TAB_HEIGHT / 3;
      wh -= TAB_HEIGHT;

   } else if ( tabpos == 0 ) {
      wh -= TAB_HEIGHT;
   } else
      WARN( _( "Tab position '%d' parameter does not make sense" ), tabpos );

   /* Copy tab information. */
   wgt->dat.tab.tabnames = malloc( sizeof( char * ) * ntabs );
   wgt->dat.tab.windows  = malloc( sizeof( unsigned int ) * ntabs );
   wgt->dat.tab.namelen  = malloc( sizeof( int ) * ntabs );
   for ( int i = 0; i < ntabs; i++ ) {
      /* Get name and length. */
      wgt->dat.tab.tabnames[i] = strdup( tabnames[i] );
      wgt->dat.tab.namelen[i] =
         gl_printWidthRaw( wgt->dat.tab.font, wgt->dat.tab.tabnames[i] );
      /* Create windows with flags.
       * Parent window handles events for the children.
       */
      int flags =
         WINDOW_NOFOCUS | WINDOW_NORENDER | WINDOW_NOINPUT | WINDOW_NOTITLE;
      /* For windows with upper tabs, keep borders to let them overwrite hiden
       * tabs */
      if ( !window_isFlag( wdw, WINDOW_TABBED ) )
         flags |= WINDOW_NOBORDER;

      wgt->dat.tab.windows[i] =
         window_createFlags( tabnames[i], tabnames[i], wx, wy, ww, wh, flags );
   }

   /* Return list of windows. */
   return wgt->dat.tab.windows;
}

/**
 * @brief Handles scrolling on a tabbed window's tab bar.
 *
 *    @param tab Widget being scrolled on.
 *    @param dir Direction to scroll in.
 *    @return Index of the newly-selected tab.
 */
static int tab_scroll( Widget *tab, int dir )
{
   int new;

   if ( dir > 0 )
      new = ( tab->dat.tab.active + 1 ) % tab->dat.tab.ntabs;
   else {
      /* Wrap manually to avoid undefined behaviour. */
      if ( tab->dat.tab.active == 0 )
         new = tab->dat.tab.ntabs - 1;
      else
         new = ( tab->dat.tab.active - 1 ) % tab->dat.tab.ntabs;
   }

   return new;
}

/**
 * @brief Handles focus restoring for a tabbed window's children.
 *
 *    @param tab Tabbed window widget.
 */
static void tab_expose( Widget *tab, int exposed )
{
   Window *wdw = window_wget( tab->dat.tab.windows[tab->dat.tab.active] );
   if ( wdw == NULL )
      return;

   /* Re-focus widgets if visible. */
   if ( exposed )
      toolkit_focusSanitize( wdw );
   /* Clear focus (disables text input, etc.) */
   else
      toolkit_focusClear( wdw );
}

/**
 * @brief Handles input for an tabbed window widget.
 *
 *    @param tab Tabbed Window widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int tab_raw( Widget *tab, SDL_Event *event )
{
   Window *wdw;
   int     ret;

   /* First handle event internally. */
   ret = 0;
   if ( event->type == SDL_MOUSEBUTTONDOWN )
      ret = tab_mouse( tab, event );
   else if ( event->type == SDL_KEYDOWN )
      ret = tab_key( tab, event );

   /* Took the event. */
   if ( ret )
      return ret;

   /* Give event to window. */
   wdw = window_wget( tab->dat.tab.windows[tab->dat.tab.active] );
   if ( wdw == NULL ) {
      WARN( _( "Active window in window '%s' not found in stack." ),
            tab->name );
      return 0;
   }

   /* Give the active window the input. */
   return toolkit_inputWindow( wdw, event, 0 );
}

/**
 * @brief Handles mouse events.
 */
static int tab_mouse( Widget *tab, SDL_Event *event )
{
   int     i, p, old, change;
   Window *parent;
   int     x, y, rx, ry;

   /* Get parent window. */
   parent = window_wget( tab->wdw );
   if ( parent == NULL )
      return 0;

   /* Convert to window space. */
   toolkit_inputTranslateCoords( parent, event, &x, &y, &rx, &ry );

   /* Translate to widget space. */
   x -= tab->x;
   y -= tab->y;

   /* Since it's at the top we have to translate down. */
   if ( tab->dat.tab.tabpos == 0 )
      y -= ( tab->h - TAB_HEIGHT );

   /* Make sure event is in the TAB HEIGHT area. */
   if ( ( y >= TAB_HEIGHT ) || ( y < 0 ) )
      return 0;

   /* Handle event. */
   p = TAB_HMARGIN;
   for ( i = 0; i < tab->dat.tab.ntabs; i++ ) {
      /* Too far left, won't match any tabs. */
      if ( x < p )
         break;

      p += ( TAB_HPADDING * 2 ) + TAB_HMARGIN + tab->dat.tab.namelen[i];

      /* Too far right, try next tab. */
      if ( x >= p )
         continue;

      old = tab->dat.tab.active;

      /* Mark as active. */
      change = -1;
      if ( event->button.button == SDL_BUTTON_X1 )
         change = tab_scroll( tab, -1 );
      else if ( event->button.button == SDL_BUTTON_X2 )
         change = tab_scroll( tab, 1 );
      else
         tab->dat.tab.active = i;

      if ( change != -1 )
         tab->dat.tab.active = change;

      /* Create event. */
      if ( tab->dat.tab.onChange != NULL )
         tab->dat.tab.onChange( tab->wdw, tab->name, old, tab->dat.tab.active );
      toolkit_rerender();
      break;
   }

   return 0;
}

/**
 * @brief Handles key events.
 */
#define CHECK_CHANGE( n, v )                                                   \
   bind_key = input_getKeybind( n, NULL, &bind_mod );                          \
   if ( ( key == bind_key ) &&                                                 \
        ( ( bind_mod == NMOD_ANY ) || ( mod == bind_mod ) ) )                  \
   change = v
static int tab_key( Widget *tab, SDL_Event *event )
{
   int         change;
   SDL_Keycode key, bind_key;
   SDL_Keymod  mod, bind_mod;
   Window     *wdw;
   Widget     *wgt;
   int         ret;

   /* Get window and focused widget. */
   wdw = window_wget( tab->dat.tab.windows[tab->dat.tab.active] );
   wgt = toolkit_getFocus( wdw );
   if ( ( wgt != NULL ) && ( wgt->textevent != NULL ) )
      return 0;

   /* Event info. */
   key = event->key.keysym.sym;
   mod = input_translateMod( event->key.keysym.mod );

   /* Handle tab changing. */
   change = -1;
   CHECK_CHANGE( KST_WEAPSET1, 0 );
   CHECK_CHANGE( KST_WEAPSET2, 1 );
   CHECK_CHANGE( KST_WEAPSET3, 2 );
   CHECK_CHANGE( KST_WEAPSET4, 3 );
   CHECK_CHANGE( KST_WEAPSET5, 4 );
   CHECK_CHANGE( KST_WEAPSET6, 5 );
   CHECK_CHANGE( KST_WEAPSET7, 6 );
   CHECK_CHANGE( KST_WEAPSET8, 7 );
   CHECK_CHANGE( KST_WEAPSET9, 8 );
   CHECK_CHANGE( KST_WEAPSET0, 9 );

   /* Window. */
   ret = 0;

   /* Handle keypresses. */
   switch ( key ) {
   case SDLK_TAB:
      if ( mod & NMOD_CTRL ) {
         if ( mod & NMOD_SHIFT ) {
            /* Wrap manually to avoid undefined behaviour. */
            if ( tab->dat.tab.active == 0 )
               change = tab->dat.tab.ntabs - 1;
            else
               change = ( tab->dat.tab.active - 1 ) % tab->dat.tab.ntabs;
         } else
            change = ( tab->dat.tab.active + 1 ) % tab->dat.tab.ntabs;
      } else {
         /* This is entirely backwards, but it's working around existing widget
          * placement. */
         if ( mod & NMOD_SHIFT )
            toolkit_nextFocus( wdw );
         else
            toolkit_prevFocus( wdw );
      }
      ret = 1;
      break;

   default:
      break;
   }

   /* Switch to the selected tab if it exists. */
   if ( change >= 0 && change < tab->dat.tab.ntabs ) {
      int old             = tab->dat.tab.active;
      tab->dat.tab.active = change;
      /* Create event. */
      if ( tab->dat.tab.onChange != NULL )
         tab->dat.tab.onChange( tab->wdw, tab->name, old, tab->dat.tab.active );
      ret = 1;
   }
   if ( ret )
      toolkit_rerender();

   return ret;
}
#undef CHECK_CHANGE

/**
 * @brief Renders a button widget.
 *
 *    @param tab WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void tab_render( Widget *tab, double bx, double by )
{
   int x, y, w, h, dy = 0;
   /* Get window. */
   Window *wdw = window_wget( tab->dat.tab.windows[tab->dat.tab.active] );
   if ( wdw == NULL ) {
      WARN( _( "Active window in widget '%s' not found in stack." ),
            tab->name );
      return;
   }
   h                     = wdw->h < 800 ? 40 : wdw->h / 20;
   w                     = tab_getBarWidth( tab );
   Window *parent_window = window_wget( tab->dat.tab.parent_window );
   int     isFullscreen  = window_isFlag( parent_window, WINDOW_FULLSCREEN );

   /* Render tabs ontop. */
   if ( tab->dat.tab.tabpos == 1 ) {
      /* Center tabs */
      tab->x = wdw->w / 2 - w / 2;
      dy     = 1;
   } else if ( tab->dat.tab.tabpos == 2 ) {
      tab->x = wdw->w / 2 - w / 2;
      tab->y = wdw->h - TAB_HEIGHT / 3.;
      dy     = -1;
   }
   y = by + tab->y;
   x = bx + tab->x;
   if ( tab->dat.tab.tabpos == 0 ) {
      y += tab->h - h;
   }
   /* Iterate through tabs */
   x += TAB_HMARGIN;
   int len, x_active = 0, len_active = 0, y_active = y + dy * 5;
   int i;
   int y_text = TAB_HEIGHT - tab->dat.tab.font->h;
   for ( i = 0; i < tab->dat.tab.ntabs; i++ ) {
      /* Draw contents rect */
      len = tab->dat.tab.namelen[i] + ( TAB_HPADDING * 2 );
      if ( i != tab->dat.tab.active ) {
         gl_renderRoundPane( x, y, len, TAB_HEIGHT, 10, 10, tab_inactive );

         /* Draw text. */
         gl_printRaw( tab->dat.tab.font, x + TAB_HPADDING,
                      y - ( isFullscreen ? 0 : dy ) * y_text / 6. + y_text / 2.,
                      &cFontWhite, -1., tab->dat.tab.tabnames[i] );
         gl_renderRoundRect( x, y, len, TAB_HEIGHT, 2, 10, 10,
                             toolkit_colLight );
      } else {

         if ( isFullscreen ) {
            gl_renderRoundPane( x, y_active, len, TAB_HEIGHT, 10, 10,
                                tab_active );
            gl_printRaw( tab->dat.tab.font, x + TAB_HPADDING,
                         y_active + y_text / 2., &cFontWhite, -1.,
                         tab->dat.tab.tabnames[i] );
            gl_renderRoundRect( x, y_active, len, TAB_HEIGHT, 2, 10, 10,
                                &cGrey70 );
         } else {
            gl_renderRoundRect( x, y_active, len, TAB_HEIGHT, 3, 10, 10,
                                &cGrey70 );
         }
         x_active   = x;
         len_active = len;
      }
      /* Go to next line. */
      x += len + TAB_HMARGIN;
   }
   /* Render the active window. */

   if ( isFullscreen ) {
      window_setFlag( wdw, WINDOW_NOBORDER );
      window_setFlag( wdw, WINDOW_FULLSCREEN );
   }

   window_render( wdw, wgt_isFlag( tab, WGT_FLAG_FOCUSED ) );

   if ( !isFullscreen ) {
      i = tab->dat.tab.active;
      gl_renderRoundPane( x_active + 3, y_active + 3, len_active - 6,
                          TAB_HEIGHT - 6, 5, 5, tab_active );
      gl_printRaw( tab->dat.tab.font, x_active + TAB_HPADDING,
                   y_active + y_text / 2., &cFontWhite, -1.,
                   tab->dat.tab.tabnames[i] );
   } else {
      window_rmFlag( wdw, WINDOW_NOBORDER );
      window_rmFlag( wdw, WINDOW_FULLSCREEN );
   }

   /* Draw text. */
}

/**
 * @brief Renders a button widget.
 *
 *    @param tab WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void tab_renderDynamic( Widget *tab, double bx, double by )
{
   (void)bx;
   (void)by;
   /* Get window. */
   Window *wdw = window_wget( tab->dat.tab.windows[tab->dat.tab.active] );
   if ( wdw == NULL ) {
      WARN( _( "Active window in widget '%s' not found in stack." ),
            tab->name );
      return;
   }

   /* Render the active window. */
   window_renderDynamic( wdw );
}

/**
 * @brief Renders a button widget overlay.
 *
 *    @param tab WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void tab_renderOverlay( Widget *tab, double bx, double by )
{
   (void)bx;
   (void)by;
   Window *wdw = window_wget( tab->dat.tab.windows[tab->dat.tab.active] );
   if ( wdw == NULL ) {
      WARN( _( "Active window in widget '%s' not found in stack." ),
            tab->name );
      return;
   }

   /* Render overlay. */
   window_renderOverlay( wdw );
}

/**
 * @brief Clean up function for the button widget.
 *
 *    @param tab Tabbed Window to clean up.
 */
static void tab_cleanup( Widget *tab )
{
   for ( int i = 0; i < tab->dat.tab.ntabs; i++ ) {
      Window *w = window_wget( tab->dat.tab.windows[i] );
      if ( w )
         window_kill( w );
      free( tab->dat.tab.tabnames[i] );
   }
   free( tab->dat.tab.tabnames );
   free( tab->dat.tab.windows );
   free( tab->dat.tab.namelen );
}

/**
 * @brief Gets the widget.
 */
static Widget *tab_getWgt( unsigned int wid, const char *tab )
{
   Widget *wgt = window_getwgt( wid, tab );

   /* Must be found in stack. */
   if ( wgt == NULL ) {
      WARN( _( "Widget '%s' not found" ), tab );
      return NULL;
      ;
   }

   /* Must be an image array. */
   if ( wgt->type != WIDGET_TABBEDWINDOW ) {
      WARN( _( "Widget '%s' is not an image array." ), tab );
      return NULL;
   }

   return wgt;
}

/**
 * @brief Sets the active tab.
 *
 *    @param wid Window to which tabbed window belongs.
 *    @param tab Name of the tabbed window.
 *    @param active tab to set active.
 */
int window_tabWinSetActive( unsigned int wid, const char *tab, int active )
{
   int old;

   Widget *wgt = tab_getWgt( wid, tab );
   if ( wgt == NULL )
      return -1;

   old = wgt->dat.tab.active;

   /* Set active window. */
   wgt->dat.tab.active = active;

   /* Create event. */
   if ( wgt->dat.tab.onChange != NULL )
      wgt->dat.tab.onChange( wid, wgt->name, old, wgt->dat.tab.active );
   toolkit_rerender();

   return 0;
}

/**
 * @brief Gets the active tab.
 *
 *    @param wid Window to which tabbed window belongs.
 *    @param tab Name of the tabbed window.
 *    @return The ID of the active tab.
 */
int window_tabWinGetActive( unsigned int wid, const char *tab )
{
   Widget *wgt = tab_getWgt( wid, tab );
   if ( wgt == NULL )
      return -1;

   /* Get active window. */
   return wgt->dat.tab.active;
}

/**
 * @brief Sets the onChange function callback.
 *
 *    @param wid Window to which tabbed window belongs.
 *    @param tab Name of the tabbed window.
 *    @param onChange Callback to use (NULL disables).
 */
int window_tabWinOnChange( unsigned int wid, const char *tab,
                           void ( *onChange )( unsigned int, const char *, int,
                                               int ) )
{
   Widget *wgt = tab_getWgt( wid, tab );
   if ( wgt == NULL )
      return -1;

   /* Set on change function. */
   wgt->dat.tab.onChange = onChange;

   return 0;
}

/**
 * @brief Changes the font used by a tabbed window widget.
 *
 *    @param wid Window to which tabbed window belongs.
 *    @param tab Name of the tabbed window.
 *    @param font Font to set to.
 *    @return 0 on success.
 */
int window_tabSetFont( unsigned int wid, const char *tab, const glFont *font )
{
   Widget *wgt = tab_getWgt( wid, tab );
   if ( wgt == NULL )
      return -1;

   wgt->dat.tab.font = font;
   for ( int i = 0; i < wgt->dat.tab.ntabs; i++ )
      wgt->dat.tab.namelen[i] =
         gl_printWidthRaw( wgt->dat.tab.font, wgt->dat.tab.tabnames[i] );

   return 0;
}

/**
 * @brief Gets the tab windows children windows.
 *
 *    @param wid Window to which tabbed window belongs.
 *    @param tab Name of the tabbed window.
 *    @return The children windows.
 */
unsigned int *window_tabWinGet( unsigned int wid, const char *tab )
{
   Widget *wgt = tab_getWgt( wid, tab );
   if ( wgt == NULL )
      return NULL;
   return wgt->dat.tab.windows;
}

/**
 * @brief Gets the total width of all tabs in a tabbed window.
 *
 *    @param wid Window to which tabbed window belongs.
 *    @param tab Name of the tabbed window.
 *    @return Bar width in pixels
 */
int window_tabWinGetBarWidth( unsigned int wid, const char *tab )
{
   const Widget *wgt = tab_getWgt( wid, tab );
   return tab_getBarWidth( wgt );
}

/**
 * @brief \see window_tabWinGetBarWidth
 */
int tab_getBarWidth( const Widget *wgt )
{
   int w;

   if ( wgt == NULL )
      return 0;

   w = TAB_HMARGIN;
   for ( int i = 0; i < wgt->dat.tab.ntabs; i++ )
      w += ( TAB_HMARGIN + TAB_HPADDING ) + wgt->dat.tab.namelen[i] +
           ( TAB_HPADDING );

   return w;
}

/**
 * @brief Sets the tab window name.
 */
int window_tabWinSetTabName( unsigned int wid, const char *tab, int id,
                             const char *name )
{
   const Widget *wgt = tab_getWgt( wid, tab );
   free( wgt->dat.tab.tabnames[id] );
   wgt->dat.tab.tabnames[id] = strdup( name );
   return 0;
}
