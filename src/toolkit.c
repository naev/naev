/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file toolkit.c
 *
 * @brief Handles windows and widgets.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "toolkit.h"

#include "dialogue.h"
#include "input.h"
#include "log.h"
#include "ntracing.h"
#include "opengl.h"
#include "pause.h"
#include "tk/toolkit_priv.h"

#define INPUT_DELAY conf.repeat_delay /**< Delay before starting to repeat. */
#define INPUT_FREQ conf.repeat_freq   /**< Interval between repetition. */

static unsigned int genwid = 0; /**< Generates unique window ids, > 0 */

static int toolkit_needsRender =
   1; /**< Whether or not toolkit needs a render. */
static int toolkit_delayCounter =
   0; /**< Horrible hack around secondary loop. */

/*
 * window stuff
 */
#define MIN_WINDOWS 4 /**< Minimum windows to prealloc. */
static Window *windows =
   NULL; /**< Window linked list, not to be confused with MS windows. */

/*
 * simulate keypresses when holding
 */
static SDL_Keycode input_key = 0; /**< Current pressed key. */
static SDL_Keymod  input_mod = 0; /**< Current pressed modifier. */

/*
 * default outline colours
 */
const glColour *toolkit_colLight = &cGrey50; /**< Light outline colour. */
const glColour *toolkit_col      = &cGrey20; /**< Normal outline colour. */
const glColour *toolkit_colDark  = &cGrey5;  /**< Dark outline colour. */

/*
 * Tab colours
 */
const glColour *tab_active     = &cGrey20; /**< Light outline colour. */
const glColour *tab_activeB    = &cGrey10; /**< Light outline colour. */
const glColour *tab_inactive   = &cGrey15; /**< Normal outline colour. */
const glColour *tab_inactiveB  = &cGrey10; /**< Normal outline colour. */
const glColour *tab_background = &cBlack;  /**< Dark outline colour. */

/*
 * VBO
 */
static gl_vbo *toolkit_vbo;             /**< Toolkit VBO. */
static GLsizei toolkit_vboColourOffset; /**< Colour offset. */

/*
 * static prototypes
 */
/* input */
static int toolkit_mouseEvent( Window *w, SDL_Event *event );
static int toolkit_mouseEventWidget( Window *w, Widget *wgt, SDL_Event *event,
                                     int x, int y, int rx, int ry );
static int toolkit_keyEvent( Window *wdw, SDL_Event *event );
static int toolkit_textEvent( Window *wdw, SDL_Event *event );
/* Focus */
static int  toolkit_isFocusable( const Widget *wgt );
static void toolkit_expose( Window *wdw, int expose );
/* render */
static void window_renderBorder( const Window *w );
/* Death. */
static void widget_kill( Widget *wgt );
static void window_cleanup( Window *wdw );
static void window_remove( Window *wdw );
static void toolkit_purgeDead( void );

/**
 * @brief Checks to see if the toolkit is open.
 *
 *    @return 1 if the toolkit is open.
 */
int toolkit_isOpen( void )
{
   /* Check to see if there is any active window. */
   for ( Window *wdw = windows; wdw != NULL; wdw = wdw->next )
      if ( !window_isFlag( wdw, WINDOW_KILL | WINDOW_NORENDER ) )
         return 1;
   return 0;
}

/**
 * @brief Delays the toolkit purge by an iteration, useful for dialogues.
 */
void toolkit_delay( void )
{
   toolkit_delayCounter++;
}

/**
 * @brief Sets the internal widget position.
 *
 *    @param wdw Window to which the widget belongs.
 *    @param wgt Widget to set position of.
 *    @param x X position to use.
 *    @param y Y position to use.
 */
void toolkit_setPos( const Window *wdw, Widget *wgt, int x, int y )
{
   /* X position. */
   if ( x < 0 )
      wgt->x = wdw->w - wgt->w + x;
   else
      wgt->x = (double)x;

   /* Y position. */
   if ( y < 0 )
      wgt->y = wdw->h - wgt->h + y;
   else
      wgt->y = (double)y;
}

/**
 * @brief Moves a window to the specified coordinates.
 *
 *    @param wdw Window to move.
 *    @param x X position.
 *    @param y Y position.
 */
void toolkit_setWindowPos( Window *wdw, int x, int y )
{
   wdw->xrel = -1.;
   wdw->yrel = -1.;

   window_rmFlag( wdw, WINDOW_CENTERX );
   window_rmFlag( wdw, WINDOW_CENTERY );

   /* x pos */
   if ( x == -1 ) { /* Center */
      wdw->x    = ( gl_screen.nw - wdw->w ) / 2.;
      wdw->xrel = 0.5;
      window_setFlag( wdw, WINDOW_CENTERX );
   } else if ( x < 0 )
      wdw->x = gl_screen.nw - wdw->w + (double)x;
   else
      wdw->x = (double)x;

   /* y pos */
   if ( y == -1 ) { /* Center */
      wdw->y    = ( gl_screen.nh - wdw->h ) / 2.;
      wdw->yrel = 0.5;
      window_setFlag( wdw, WINDOW_CENTERY );
   } else if ( y < 0 )
      wdw->y = gl_screen.nh - wdw->h + (double)y;
   else
      wdw->y = (double)y;
}

/**
 * @brief Moves a window to the specified coordinates.
 *
 *    @param wid ID of the window to move.
 *    @param x X position.
 *    @param y Y position.
 */
void window_move( unsigned int wid, int x, int y )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   toolkit_setWindowPos( wdw, x, y );
}

/**
 * @brief Resizes the window.
 *
 *    @param wid Window ID.
 *    @param w Width to change to (or fullscreen if -1).
 *    @param h Height to change to (or fullscreen if -1).
 */
void window_resize( unsigned int wid, int w, int h )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   wdw->w = ( w == -1 ) ? gl_screen.nw : (double)w;
   wdw->h = ( h == -1 ) ? gl_screen.nh : (double)h;

   if ( ( w == -1 ) && ( h == -1 ) ) {
      window_setFlag( wdw, WINDOW_FULLSCREEN );
      wdw->x = 0.;
      wdw->y = 0.;
      window_setFlag( wdw, WINDOW_CENTERX );
      window_setFlag( wdw, WINDOW_CENTERY );
   } else {
      window_rmFlag( wdw, WINDOW_FULLSCREEN );
      if ( window_isFlag( wdw, WINDOW_CENTERX ) &&
           window_isFlag( wdw, WINDOW_CENTERY ) )
         toolkit_setWindowPos( wdw, -1, -1 );
   }
}

/**
 * @brief Allocates room for a new widget.
 *
 *    @param w Window to create widget in.
 *    @param name Name of the widget to create.
 *    @return Newly allocated widget.
 */
Widget *window_newWidget( Window *w, const char *name )
{
   Widget *wgt, *wlast, *wtmp;
   char   *saved_name = NULL;

   /* NULL protection. */
   if ( w == NULL )
      return NULL;

   /* Try to find one with the same name first. */
   wlast = NULL;
   for ( wgt = w->widgets; wgt != NULL; wgt = wgt->next ) {

      /* Must match name. */
      if ( strcmp( name, wgt->name ) != 0 ) {
         wlast = wgt;
         continue;
      }

      /* Should be destroyed. */
      if ( !wgt_isFlag( wgt, WGT_FLAG_KILL ) ) {
         WARN( _( "Trying to create widget '%s' over existing one that hasn't "
                  "been destroyed" ),
               name );
         return NULL;
      }

      /* Relink. */
      if ( wlast == NULL )
         w->widgets = wgt->next;
      else
         wlast->next = wgt->next;

      /* Prepare and return this widget. */
      saved_name = wgt->name;
      wgt->name  = NULL;
      widget_cleanup( wgt );
      break;
   }

   /* Must grow widgets. */
   if ( wgt == NULL )
      wgt = nmalloc( sizeof( Widget ) );

   /* Safe defaults. */
   memset( wgt, 0, sizeof( Widget ) );
   wgt->type   = WIDGET_NULL;
   wgt->status = WIDGET_STATUS_NORMAL;
   wgt->wdw    = w->id;
   if ( saved_name !=
        NULL ) /* Hack to avoid frees so _getFocus works in the same frame. */
      wgt->name = saved_name;
   else
      wgt->name = strdup( name );
   wgt->id = ++w->idgen;

   /* Set up. */
   wlast = NULL;
   for ( wtmp = w->widgets; wtmp != NULL; wtmp = wtmp->next )
      wlast = wtmp;
   if ( wlast == NULL )
      w->widgets = wgt;
   else
      wlast->next = wgt;

   toolkit_rerender();
   return wgt;
}

/**
 * @brief Gets a Window by ID.
 *
 *    @param wid ID of the window to get.
 *    @return Window matching wid.
 */
Window *window_wget( unsigned int wid )
{
   if ( windows == NULL ) {
      WARN( _( "Window '%u' not found in list!" ), wid );
      return NULL;
   }
   Window *w = window_wgetW( wid );
   if ( w == NULL )
      WARN( _( "Window '%u' not found in list!" ), wid );
   return w;
}

/**
 * @brief Gets a Window by ID, without warning.
 *
 *    @param wid ID of the window to get.
 *    @return Window matching wid.
 */
Window *window_wgetW( unsigned int wid )
{
   for ( Window *w = windows; w != NULL; w = w->next )
      if ( w->id == wid )
         return w;
   return NULL;
}

/**
 * @brief Gets a Window by name, without warning.
 *
 *    @param name Name of the window to get.
 *    @return Window matching wid.
 */
Window *window_wgetNameW( const char *name )
{
   for ( Window *w = windows; w != NULL; w = w->next )
      if ( strcmp( w->name, name ) == 0 )
         return w;
   return NULL;
}

/**
 * @brief Gets a widget from window id and widgetname.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to get.
 *    @return Widget matching name in the window.
 */
Widget *window_getwgt( unsigned int wid, const char *name )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL ) {
      WARN( _( "Widget '%s' not found in window '%u'!" ), name, wid );
      return NULL;
   }

   /* Find the widget. */
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next )
      if ( strcmp( wgt->name, name ) == 0 )
         return wgt;

   WARN( _( "Widget '%s' not found in window '%u'!" ), name, wid );
   return NULL;
}

/**
 * @brief Gets the dimensions of a window.
 *
 *    @param wid ID of the window to get dimension of.
 *    @param[out] w Width of the window or -1 on error.
 *    @param[out] h Height of the window or -1 on error.
 */
void window_dimWindow( unsigned int wid, int *w, int *h )
{
   /* Get the window. */
   const Window *wdw = window_wget( wid );
   if ( wdw == NULL ) {
      *w = -1;
      *h = -1;
      return;
   }

   /* Set dimensions. */
   *w = wdw->w;
   *h = wdw->h;
}

/**
 * @brief Gets the dimensions of a window.
 *
 *    @param wid ID of the window to get dimension of.
 *    @param[out] x X position of the window or -1 on error.
 *    @param[out] y Y position of the window or -1 on error.
 */
void window_posWindow( unsigned int wid, int *x, int *y )
{
   /* Get the window. */
   const Window *wdw = window_wget( wid );
   if ( wdw == NULL ) {
      *x = -1;
      *y = -1;
      return;
   }

   /* Set dimensions. */
   *x = wdw->x;
   *y = wdw->y;
}

/**
 * @brief Gets the dimensions of a widget.
 *
 *    @param wid ID of the window that contains the widget.
 *    @param name Name of the widget to get dimensions of.
 *    @param[out] w Width of the widget or -1 on error.
 *    @param[out] h Height of the widget or -1 on error.
 */
void window_dimWidget( unsigned int wid, const char *name, int *w, int *h )
{
   /* Get widget. */
   const Widget *wgt = window_getwgt( wid, name );
   if ( wgt == NULL ) {
      if ( w != NULL )
         *w = -1;
      if ( h != NULL )
         *h = -1;
      return;
   }

   if ( w != NULL )
      *w = wgt->w;
   if ( h != NULL )
      *h = wgt->h;
}

/**
 * @brief Gets a widget's position.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to get position of.
 *    @param[out] x X position of the widget.
 *    @param[out] y Y position of the widget.
 */
void window_posWidget( unsigned int wid, const char *name, int *x, int *y )
{
   /* Get widget. */
   const Widget *wgt = window_getwgt( wid, name );
   if ( wgt == NULL )
      return;

   /* Return position. */
   if ( x != NULL )
      ( *x ) = wgt->x;
   if ( y != NULL )
      ( *y ) = wgt->y;
}

/**
 * @brief Moves a widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to set position to.
 *    @param x New X position to set widget to.
 *    @param y New Y position to set widget to.
 */
void window_moveWidget( unsigned int wid, const char *name, int x, int y )
{
   Window *wdw;
   Widget *wgt;

   /* Get window. */
   wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Get widget. */
   wgt = window_getwgt( wid, name );
   if ( wgt == NULL )
      return;

   /* Set position. */
   toolkit_setPos( wdw, wgt, x, y );
}

/**
 * @brief Resizes a widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to resize
 *    @param w New width.
 *    @param h New height.
 */
void window_resizeWidget( unsigned int wid, const char *name, int w, int h )
{
   Window *wdw;
   Widget *wgt;

   /* Get window. */
   wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Get widget. */
   wgt = window_getwgt( wid, name );
   if ( wgt == NULL )
      return;

   /* Set position. */
   wgt->w = w;
   wgt->h = h;
}

/**
 * @brief Allows or disallows focusing a widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to resize
 *    @param canfocus Whether or not the widget should be able to be focused.
 */
void window_canFocusWidget( unsigned int wid, const char *name, int canfocus )
{
   Window *wdw;
   Widget *wgt;

   /* Get window. */
   wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Get widget. */
   wgt = window_getwgt( wid, name );
   if ( wgt == NULL )
      return;

   /* Set position. */
   if ( canfocus )
      wgt_setFlag( wgt, WGT_FLAG_CANFOCUS );
   else
      wgt_rmFlag( wgt, WGT_FLAG_CANFOCUS );
}

/**
 * @brief Checks to see if a window is at the top.
 *
 *    @param wid indow ID to see if is top.
 */
int window_isTop( unsigned int wid )
{
   Window *n, *w = window_wget( wid );
   if ( w == NULL )
      return 0;
   n = w->next;
   while ( n != NULL ) {
      if ( !window_isFlag( n, WINDOW_KILL | WINDOW_NORENDER ) )
         return 0;
      n = n->next;
   }
   return 1;
}

/**
 * @brief Checks to see if a widget is covered or not.
 */
int widget_isCovered( unsigned int wid, const char *name, int x, int y )
{
   int     bx, by, rx, ry;
   Widget *wgt = window_getwgt( wid, name );
   if ( wgt == NULL )
      return 0;

   /* Undo transform. */
   bx = x + wgt->x;
   by = y + wgt->y;

   /* Find if the point is covered. */
   for ( Widget *w = wgt->next; w != NULL; w = w->next ) {
      if ( ( wgt->render == NULL ) || wgt_isFlag( w, WGT_FLAG_KILL ) )
         continue;

      rx = bx - w->x;
      ry = by - w->y;
      if ( !( ( rx < 0 ) || ( rx >= w->w ) || ( ry < 0 ) || ( ry >= w->h ) ) )
         return 1;
   }
   return 0;
}

/**
 * @brief Checks to see if a window exists.
 *
 *    @param wdwname Name of the window to check.
 *    @return 1 if it exists, 0 if it doesn't.
 */
int window_exists( const char *wdwname )
{
   if ( windows == NULL )
      return 0;
   for ( Window *w = windows; w != NULL; w = w->next )
      if ( ( strcmp( w->name, wdwname ) == 0 ) &&
           !window_isFlag( w, WINDOW_KILL ) )
         return 1;
   return 0; /* doesn't exist */
}

/**
 * @brief Checks to see if a window with a certain ID exists.
 *
 *    @param wid ID of the window to check.
 *    @return 1 if it exists, 0 if it doesn't.
 */
int window_existsID( unsigned int wid )
{
   if ( windows == NULL )
      return 0;
   for ( Window *w = windows; w != NULL; w = w->next )
      if ( ( w->id == wid ) && !window_isFlag( w, WINDOW_KILL ) )
         return 1;
   return 0; /* doesn't exist */
}

/**
 * @brief Sets the displayname of a window.
 *
 *    @param wid ID of the window.
 *    @param displayname Display name to set to.
 *    @return 0 on success.
 */
int window_setDisplayname( unsigned int wid, const char *displayname )
{
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return -1;
   free( wdw->displayname );
   wdw->displayname = NULL;
   if ( displayname != NULL )
      wdw->displayname = strdup( displayname );
   return 0;
}

/**
 * @brief Sets a window as dynamic, so that it is drawn every frame completely.
 *
 *    @param wid ID of the window.
 *    @param dynamic Whether or not to set as dynamic.
 */
void window_setDynamic( unsigned int wid, int dynamic )
{
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;
   if ( dynamic )
      wdw->flags |= WINDOW_DYNAMIC;
   else
      wdw->flags &= ~WINDOW_DYNAMIC;
}

/**
 * @brief Gets the ID of a window.
 *
 * @note Gets the top window matching the ID first.
 *
 *    @param wdwname Name of the window to get ID of.
 *    @return ID of the window.
 */
unsigned int window_get( const char *wdwname )
{
   Window *last;
   if ( windows == NULL )
      return 0;
   last = NULL;
   for ( Window *w = windows; w != NULL; w = w->next )
      if ( ( strcmp( w->name, wdwname ) == 0 ) &&
           !window_isFlag( w, WINDOW_KILL ) )
         last = w;
   if ( last == NULL )
      return 0;
   return last->id;
}

/**
 * @brief Creates a window.
 *
 *    @param name Window name to use internally - should be unique.
 *    @param displayname Title displayed on the window.
 *    @param x X position of the window (-1 centers).
 *    @param y Y position of the window (-1 centers).
 *    @param w Width of the window (-1 fullscreen).
 *    @param h Height of the window (-1 fullscreen).
 *    @return Newly created window's ID.
 */
unsigned int window_create( const char *name, const char *displayname,
                            const int x, const int y, const int w, const int h )
{
   /* For windows with upper tabs, let tabedWindow handle the border to hide the
    * unactive tabs */
   return window_createFlags( name, displayname, x, y, w, h, 0 );
}

/**
 * @brief Creates a window.
 *
 *    @param name Window name to use internally - should be unique.
 *    @param displayname Title displayed on the window.
 *    @param x X position of the window (-1 centers).
 *    @param y Y position of the window (-1 centers).
 *    @param w Width of the window (-1 fullscreen).
 *    @param h Height of the window (-1 fullscreen).
 *    @param flags Initial flags to set.
 *    @return Newly created window's ID.
 */
unsigned int window_createFlags( const char *name, const char *displayname,
                                 const int x, const int y, const int w,
                                 const int h, unsigned int flags )
{
   Window *wdw = ncalloc( 1, sizeof( Window ) );

   const int wid = ( ++genwid ); /* unique id */

   /* Create the window. */
   wdw->id          = wid;
   wdw->name        = strdup( name );
   wdw->displayname = strdup( displayname );

   /* Safe defaults. */
   wdw->idgen   = -1;
   wdw->focus   = -1;
   wdw->xrel    = -1.;
   wdw->yrel    = -1.;
   wdw->flags   = flags;
   wdw->exposed = !window_isFlag( wdw, WINDOW_NOFOCUS );

   /* Dimensions. */
   wdw->w = ( w == -1 ) ? gl_screen.nw : (double)w;
   wdw->h = ( h == -1 ) ? gl_screen.nh : (double)h;

   if ( ( w == -1 ) && ( h == -1 ) ) {
      window_setFlag( wdw, WINDOW_FULLSCREEN );
      wdw->x = 0.;
      wdw->y = 0.;
   } else
      toolkit_setWindowPos( wdw, x, y );

   if ( !toolkit_isOpen() ) {
      input_mouseShow();
      pause_game();
      gl_defViewport(); /* Reset the default viewport */
   }

   /* Clear key repeat. */
   toolkit_clearKey();

   /* Add to list. */
   wdw->next = NULL;
   if ( windows == NULL )
      windows = wdw;
   else {
      Window *wlast;
      /* Take focus from the old window. */
      if ( wdw->exposed ) {
         Window *wcur = toolkit_getActiveWindow();
         if ( wcur != NULL )
            toolkit_expose( wcur, 0 ); /* wcur is hidden */
      }

      wlast = windows;
      while ( 1 ) {
         if ( ( strcmp( wlast->name, name ) == 0 ) &&
              !window_isFlag( wlast, WINDOW_KILL ) &&
              !window_isFlag( wlast, WINDOW_NOFOCUS ) )
            WARN( _( "Window with name '%s' already exists!" ), wlast->name );

         if ( wlast->next == NULL )
            break;

         wlast = wlast->next;
      }

      wlast->next = wdw;
   }

   toolkit_rerender();
   return wid;
}

/**
 * @brief Sets a window as a window's parent.
 *
 * When a window's parent closes, it closes the window also.
 *
 *    @param wid Window to set as child.
 *    @param parent Window to set as parent.
 */
void window_setParent( unsigned int wid, unsigned int parent )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set the parent. */
   wdw->parent = parent;
}

/**
 * @brief Gets the window's parent.
 *
 *    @param wid Window to get parent of.
 *    @return Parent of the window or 0 on error.
 */
unsigned int window_getParent( unsigned int wid )
{
   /* Get the window. */
   const Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return 0;

   /* Get the parent. */
   return wdw->parent;
}

/**
 * @brief Sets the default close function of the window.
 *
 * This function is called when the window is closed.
 *
 *    @param wid Window to set close function of.
 *    @param fptr Function to trigger when window is closed, parameter is window
 * id and name.
 */
void window_onClose( unsigned int wid,
                     void ( *fptr )( unsigned int, const char * ) )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set the close function. */
   wdw->close_fptr = fptr;
}

/**
 * @brief Sets the default accept function of the window.
 *
 * This function is called whenever 'enter' is pressed and the current widget
 *  does not catch it.  NULL disables the accept function.
 *
 *    @param wid ID of the window to set the accept function.
 *    @param accept Function to trigger when window is "accepted".  Parameter
 *                  passed is window name.
 */
void window_setAccept( unsigned int wid,
                       void ( *accept )( unsigned int, const char * ) )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set the accept function. */
   wdw->accept_fptr = accept;
}

/**
 * @brief Sets the default cancel function of the window.
 *
 * This function is called whenever 'escape' is hit and the current widget
 *  does not catch it. NULL disables the cancel function.
 *
 *    @param wid ID of the window to set cancel function.
 *    @param cancel Function to trigger when window is "cancelled".  Parameter
 *                  passed is window name.
 */
void window_setCancel( unsigned int wid,
                       void ( *cancel )( unsigned int, const char * ) )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set the cancel function. */
   wdw->cancel_fptr = cancel;
}

/**
 * @brief Sets the focus function of the window.
 *
 * This function is called whenever the window is focused or unfocused.
 *
 *    @param wid ID of the window to set cancel function.
 *    @param focus Function to trigger when window focus status changes.
 * Parameter passed is window name.
 */
void window_setOnFocus( unsigned int wid, void ( *focus )( unsigned int ) )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set the cancel function. */
   wdw->focus_fptr = focus;
}

/**
 * @brief Sets custom data for a window.
 *
 *    @param wid Window to set custom data of.
 *    @param data Data to set.
 */
void window_setData( unsigned int wid, void *data )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set data. */
   wdw->udata = data;
}

/**
 * @brief Gets the custom data of a window.
 *
 *    @param wid Window to get custom data of.
 *    @return The custom data or NULL if not applicable.
 */
void *window_getData( unsigned int wid )
{
   /* Get the window. */
   const Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return NULL;

   /* Get data. */
   return wdw->udata;
}

/**
 * @brief Sets or removes the border of a window.
 *
 * Default is enabled.
 *
 *    @param wid ID of the window to enable/disable border.
 *    @param enable Whether or not to enable rendering of the border.
 */
void window_setBorder( unsigned int wid, int enable )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   if ( enable )
      window_rmFlag( wdw, WINDOW_NOBORDER );
   else
      window_setFlag( wdw, WINDOW_NOBORDER );
}

/**
 * @brief Sets the key handler for the window.
 *
 * This function is only called if neither the active widget nor the window
 *  itself grabs the input.
 */
void window_handleKeys( unsigned int wid,
                        int ( *keyhandler )( unsigned int, SDL_Keycode,
                                             SDL_Keymod, int ) )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set key event handler function. */
   wdw->keyevent = keyhandler;
}

/**
 * @brief Sets the event handler for the window.
 *
 * This function is called every time the window receives input.
 */
void window_handleEvents( unsigned int wid,
                          int ( *eventhandler )( unsigned int, SDL_Event * ) )
{
   /* Get the window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Set key event handler function. */
   wdw->eventevent = eventhandler;
}

/**
 * @brief Destroys a widget.
 *
 *    @param widget Widget to destroy.
 */
void widget_cleanup( Widget *widget )
{
   /* Type specific clean up. */
   if ( widget->cleanup != NULL )
      widget->cleanup( widget );

   /* General freeing. */
   free( widget->name );
}

void widget_setStatus( Widget *wgt, WidgetStatus sts )
{
   if ( wgt->status != sts )
      toolkit_rerender();
   wgt->status = sts;
}

/**
 * @brief Closes all open toolkit windows.
 */
void toolkit_closeAll( void )
{
   for ( Window *w = windows; w != NULL; w = w->next )
      window_destroy( w->id );
}

/**
 * @brief Helper function to automatically close the window calling it.
 *
 *    @param wid Window to close.
 *    @param str Unused.
 */
void window_close( unsigned int wid, const char *str )
{
   (void)str;
   window_destroy( wid );
}

/**
 * @brief Kills the window.
 *
 *    @param wid ID of window to destroy.
 */
void window_destroy( unsigned int wid )
{
   if ( windows == NULL )
      return;
   /* Destroy the window */
   for ( Window *wdw = windows; wdw != NULL; wdw = wdw->next ) {
      Window *wactive;

      /* Not the window we're looking for. */
      if ( wdw->id != wid )
         continue;

      /* Already being killed, skip. */
      if ( window_isFlag( wdw, WINDOW_KILL ) )
         continue;

      /* Mark children for death. */
      for ( Window *w = windows; w != NULL; w = w->next )
         if ( w->parent == wid )
            window_destroy( w->id );

      /* Clean up. */
      window_cleanup( wdw );
      window_setFlag( wdw, WINDOW_KILL );
      toolkit_rerender();

      /* Disable text input, etc. */
      toolkit_focusClear( wdw );

      wactive = toolkit_getActiveWindow();
      if ( wactive == NULL )
         break;

      toolkit_expose( wactive, 1 );
      break;
   }

   /* Do focus. */
   for ( Window *w = windows; w != NULL; w = w->next ) {
      if ( !window_isFlag( w, WINDOW_KILL ) && ( w->focus_fptr != NULL ) )
         w->focus_fptr( w->id );
   }
}

/**
 * @brief Kills the window (and children).
 *
 *    @param wdw Window to kill.
 */
void window_kill( Window *wdw )
{
   for ( Window *w = windows; w != NULL; w = w->next )
      if ( w->parent == wdw->id )
         window_kill( w );
   window_cleanup( wdw );
   window_setFlag( wdw, WINDOW_KILL );
   toolkit_rerender();
}

/**
 * @brief Cleans up the window, should be done before window_remove
 *
 *    @param wdw Window to clean up.
 * @see window_remove
 */
static void window_cleanup( Window *wdw )
{
   /* Run the close function first. */
   if ( wdw->close_fptr != NULL )
      wdw->close_fptr( wdw->id, wdw->name );
   wdw->close_fptr = NULL;
}

/**
 * @brief Frees up a window. Make sure to clean up the window first.
 *
 *    @param wdw Window to remove.
 * @see window_cleanup
 */
static void window_remove( Window *wdw )
{
   Widget *wgt;

   /* Destroy the window. */
   free( wdw->name );
   free( wdw->displayname );
   wgt = wdw->widgets;
   while ( wgt != NULL ) {
      Widget *wgtkill = wgt;
      wgt             = wgtkill->next;
      widget_kill( wgtkill );
   }
   nfree( wdw );

   /* Clear key repeat, since toolkit could miss the keyup event. */
   toolkit_clearKey();
}

/**
 * @brief Checks to see if a widget exists.
 *
 *    @param wid Window to check widget in.
 *    @param wgtname Name of the widget to check;
 *    @return 1 if the widget exists.
 */
int widget_exists( unsigned int wid, const char *wgtname )
{
   /* Get window. */
   Window *w = window_wget( wid );
   if ( w == NULL ) {
      WARN( _( "window '%d' does not exist" ), wid );
      return 0;
   }

   /* Check for widget. */
   for ( Widget *wgt = w->widgets; wgt != NULL; wgt = wgt->next )
      if ( strcmp( wgtname, wgt->name ) == 0 )
         return !wgt_isFlag( wgt, WGT_FLAG_KILL );

   return 0;
}

/**
 * @brief Destroys a widget in a window.
 *
 *    @param wid Window to destroy widget in.
 *    @param wgtname Name of the widget to destroy.
 */
void window_destroyWidget( unsigned int wid, const char *wgtname )
{
   Window *wdw;
   Widget *wgt;

   /* Get the window. */
   wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   /* Get the widget. */
   for ( wgt = wdw->widgets; wgt != NULL; wgt = wgt->next )
      if ( strcmp( wgt->name, wgtname ) == 0 )
         break;

   if ( wgt == NULL ) {
      WARN( _( "Widget '%s' not found in window '%s'" ), wgtname,
            wdw->displayname );
      return;
   }

   toolkit_defocusWidget( wdw, wgt );

   /* There's dead stuff now. */
   wgt_rmFlag( wgt, WGT_FLAG_FOCUSED );
   wgt_setFlag( wgt, WGT_FLAG_KILL );
}

/**
 * @brief Destroy a widget really.
 */
static void widget_kill( Widget *wgt )
{
   /* Clean up. */
   widget_cleanup( wgt );
   nfree( wgt );
}

/**
 * @brief Draws an outline.
 *
 * If lc is NULL, colour will be flat.
 *
 *    @param x X position to draw at.
 *    @param y Y position to draw at.
 *    @param w Width.
 *    @param h Height.
 *    @param b Border width.
 *    @param thick Thickness of the border.
 *    @param c Colour.
 *    @param lc Light colour.
 */
void toolkit_drawOutlineThick( int x, int y, int w, int h, int b, int thick,
                               const glColour *c, const glColour *lc )
{
   GLshort  tri[5][4];
   glColour colours[10];

   x -= ( b - thick );
   w += 2 * ( b - thick );
   y -= ( b - thick );
   h += 2 * ( b - thick );
   lc = lc ? lc : c;

   /* Left-up. */
   tri[0][0]  = x; /* Inner */
   tri[0][1]  = y;
   tri[0][2]  = x - thick; /* Outer */
   tri[0][3]  = y - thick;
   colours[0] = *lc;
   colours[1] = *lc;

   /* Left-down. */
   tri[1][0]  = x; /* Inner. */
   tri[1][1]  = y + h;
   tri[1][2]  = x - thick; /* Outer. */
   tri[1][3]  = y + h + thick;
   colours[2] = *c;
   colours[3] = *c;

   /* Right-down. */
   tri[2][0]  = x + w; /* Inner. */
   tri[2][1]  = y + h;
   tri[2][2]  = x + w + thick; /* Outer. */
   tri[2][3]  = y + h + thick;
   colours[4] = *c;
   colours[5] = *c;

   /* Right-up. */
   tri[3][0]  = x + w; /* Inner. */
   tri[3][1]  = y;
   tri[3][2]  = x + w + thick; /* Outer. */
   tri[3][3]  = y - thick;
   colours[6] = *lc;
   colours[7] = *lc;

   /* Left-up. */
   tri[4][0]  = x; /* Inner */
   tri[4][1]  = y;
   tri[4][2]  = x - thick; /* Outer */
   tri[4][3]  = y - thick;
   colours[8] = *lc;
   colours[9] = *lc;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof( tri ), tri );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof( colours ),
                  colours );

   gl_beginSmoothProgram( gl_view_matrix );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex, 0, 2,
                               GL_SHORT, 0 );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex_colour,
                               toolkit_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 10 );
   gl_endSmoothProgram();
}
/**
 * @brief Draws an outline.
 *
 * If lc is NULL, colour will be flat.
 *
 *    @param x X position to draw at.
 *    @param y Y position to draw at.
 *    @param w Width.
 *    @param h Height.
 *    @param b Border width.
 *    @param c Colour.
 *    @param lc Light colour.
 */
void toolkit_drawOutline( int x, int y, int w, int h, int b, const glColour *c,
                          const glColour *lc )
{
   GLshort  lines[4][2];
   glColour colours[4];

   x -= b, w += 2 * b;
   y -= b, h += 2 * b;
   lc = lc ? lc : c;

   /* Lines. */
   lines[0][0] = x; /* left-up */
   lines[0][1] = y;
   colours[0]  = *lc;

   lines[1][0] = x; /* left-down */
   lines[1][1] = y + h;
   colours[1]  = *c;

   lines[2][0] = x + w; /* right-down */
   lines[2][1] = y + h;
   colours[2]  = *c;

   lines[3][0] = x + w; /* right-up */
   lines[3][1] = y;
   colours[3]  = *lc;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof( lines ), lines );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof( colours ),
                  colours );

   gl_beginSmoothProgram( gl_view_matrix );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex, 0, 2,
                               GL_SHORT, 0 );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex_colour,
                               toolkit_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_LOOP, 0, 4 );
   gl_endSmoothProgram();
}

/**
 * @brief Draws a rectangle.
 *
 * If lc is NULL, colour will be flat.
 *
 *    @param x X position to draw at.
 *    @param y Y position to draw at.
 *    @param w Width.
 *    @param h Height.
 *    @param c Colour.
 *    @param lc Light colour.
 */
void toolkit_drawRect( int x, int y, int w, int h, const glColour *c,
                       const glColour *lc )
{
   GLshort  vertex[4][2];
   glColour colours[4];

   lc = lc ? lc : c;

   /* Set up vertices and colours. */
   vertex[0][0] = x; /* left-up */
   vertex[0][1] = y;
   colours[0]   = *c;

   vertex[1][0] = x; /* left-down */
   vertex[1][1] = y + h;
   colours[1]   = *lc;

   vertex[2][0] = x + w; /* right-up */
   vertex[2][1] = y;
   colours[2]   = *c;

   vertex[3][0] = x + w; /* right-down */
   vertex[3][1] = y + h;
   colours[3]   = *lc;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof( vertex ), vertex );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof( colours ),
                  colours );

   gl_beginSmoothProgram( gl_view_matrix );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex, 0, 2,
                               GL_SHORT, 0 );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex_colour,
                               toolkit_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   gl_endSmoothProgram();
}

/**
 * @brief Draws a rectangle.
 *
 *    @param x1 X position of corner 1.
 *    @param y1 Y position to corner 1.
 *    @param x2 X position of corner 2.
 *    @param y2 Y position to corner 2.
 *    @param x3 X position of corner 3.
 *    @param y3 Y position to corner 3.
 *    @param c Colour.
 */
void toolkit_drawTriangle( int x1, int y1, int x2, int y2, int x3, int y3,
                           const glColour *c )
{
   GLshort  vertex[3][2];
   glColour colours[3];

   /* Set up vertices and colours. */
   vertex[0][0] = x1; /* left-up */
   vertex[0][1] = y1;
   colours[0]   = *c;

   vertex[1][0] = x2; /* left-down */
   vertex[1][1] = y2;
   colours[1]   = *c;

   vertex[2][0] = x3; /* right-up */
   vertex[2][1] = y3;
   colours[2]   = *c;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof( vertex ), vertex );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof( colours ),
                  colours );

   gl_beginSmoothProgram( gl_view_matrix );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex, 0, 2,
                               GL_SHORT, 0 );
   gl_vboActivateAttribOffset( toolkit_vbo, shaders.smooth.vertex_colour,
                               toolkit_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 3 );
   gl_endSmoothProgram();
}

/**
 * @brief Draws an alt text.
 *
 *    @param bx X position to draw at.
 *    @param by Y position to draw at.
 *    @param alt Text to draw.
 */
void toolkit_drawAltText( int bx, int by, const char *alt )
{
   double   w, h;
   double   x, y;
   glColour c;
   glColour c2;

   /* We're on top of anything previously drawn. */
   glClear( GL_DEPTH_BUFFER_BIT );

   /* Get dimensions. */
   w = MIN( gl_printWidthRaw( &gl_smallFont, alt ), 400 );
   h = gl_printHeightRaw( &gl_smallFont, w, alt );

   /* Choose position. */
   x = bx + 10.;
   y = by - h - gl_smallFont.h - 10.;
   if ( y < 10. )
      y = 10.;
   if ( x + w + 10. > gl_screen.nw )
      x -= w;

   /* Set colours. */
   c.r  = cGrey20.r;
   c.g  = cGrey20.g;
   c.b  = cGrey20.b;
   c.a  = 0.9;
   c2.r = cGrey10.r;
   c2.g = cGrey10.g;
   c2.b = cGrey10.b;
   c2.a = 0.7;
   // note do we still need two ?
   gl_renderRoundPane( x + 1, y + 1, w + 18, h + 18, ( h + 18 ) / 10,
                       ( h + 18 ) / 10, &c2 );
   gl_renderRoundPane( x, y, w + 18, h + 18, ( h + 18 ) / 10, ( h + 18 ) / 10,
                       &c );
   gl_printTextRaw( &gl_smallFont, w, h, x + 9, y + 9, 0, &cFontWhite, -1.,
                    alt );
}

/**
 * @brief Renders a window border.
 *
 *    @param w Window to render
 */
static void window_renderBorder( const Window *w )
{
   /* Position */
   double x = w->x;
   double y = w->y;
   if ( window_isFlag( w, WINDOW_FULLSCREEN ) ) {
      gl_renderPane( x, y, w->w, w->h, toolkit_col );
   } else {
      gl_renderRoundPane( x, y, w->w, w->h, w->h / 20., w->h / 20.,
                          toolkit_col );
      gl_renderRoundRect( x, y, w->w, w->h, 1, w->h / 20., w->h / 20.,
                          &cGrey70 );
   }
   /**
    * render window name
    */
   if ( !window_isFlag( w, WINDOW_NOTITLE ) )
      gl_printMidRaw( &gl_defFont, w->w, x, y + w->h - 20., &cFontWhite, -1.,
                      w->displayname );
}

/**
 * @brief Renders a window.
 *
 *    @param w Window to render.
 *    @param top Whether or not the window is at the top.
 */
void window_render( Window *w, int top )
{
   /* We're on top of anything previously drawn. */
   glClear( GL_DEPTH_BUFFER_BIT );

   /* See if needs border. */
   if ( !window_isFlag( w, WINDOW_NOBORDER ) ||
        ( window_isFlag( w, WINDOW_TABBED ) &&
          window_isFlag( w, WINDOW_FULLSCREEN ) ) )
      window_renderBorder( w );

   /* Iterate over widgets. */
   for ( Widget *wgt = w->widgets; wgt != NULL; wgt = wgt->next ) {
      if ( wgt->render == NULL )
         continue;
      if ( wgt_isFlag( wgt, WGT_FLAG_KILL ) )
         continue;

      /* Only render non-dynamics. */
      if ( !wgt_isFlag( wgt, WGT_FLAG_DYNAMIC ) || !top )
         wgt->render( wgt, w->x, w->y );
      if ( wgt->id == w->focus ) {
         double wx = w->x + wgt->x;
         double wy = w->y + wgt->y;
         if ( wgt->type == WIDGET_BUTTON )
            gl_renderRoundRect( wx, wy, wgt->w, wgt->h, 2, 10, 10, &cGrey70 );
         else
            gl_renderRect( wx - 2, wy - 2, wgt->w + 4, wgt->h + 4, 2,
                           &cGrey30 );
      }
   }
}

/**
 * @brief Renders the dynamic components of a window.
 */
void window_renderDynamic( Window *w )
{
   /* Iterate over widgets. */
   for ( Widget *wgt = w->widgets; wgt != NULL; wgt = wgt->next ) {
      if ( wgt->render == NULL )
         continue;
      if ( wgt_isFlag( wgt, WGT_FLAG_KILL ) )
         continue;
      if ( wgt_isFlag( wgt, WGT_FLAG_DYNAMIC ) ||
           window_isFlag( w, WINDOW_DYNAMIC ) )
         wgt->render( wgt, w->x, w->y );
      if ( wgt->renderDynamic != NULL )
         wgt->renderDynamic( wgt, w->x, w->y );
   }
}

/**
 * @brief Renders the window overlays.
 *
 *    @param w Window to render overlays of.
 */
void window_renderOverlay( Window *w )
{
   /* Draw overlays. */
   for ( Widget *wgt = w->widgets; wgt != NULL; wgt = wgt->next )
      if ( ( wgt->renderOverlay != NULL ) && !wgt_isFlag( wgt, WGT_FLAG_KILL ) )
         wgt->renderOverlay( wgt, w->x, w->y );
}

/**
 * @brief Draws a scrollbar.
 *
 *    @param x X position of scrollbar.
 *    @param y Y position of scrollbar.
 *    @param w Width of the scrollbar.
 *    @param h Height of the scrollbar.
 *    @param pos Position at [0:1].
 */
void toolkit_drawScrollbar( int x, int y, int w, int h, double pos )
{
   double sy;

   /* scrollbar background */
   gl_renderPane( x, y, w, h, &cGrey10 );

   /* Bar itself. */
   sy = y + ( h - 30. ) * ( 1. - pos );
   gl_renderRoundPane( x, sy, w, 30., w / 3., 30. / 10, toolkit_colLight );
   gl_renderRoundRect( x + 1, sy, w - 2, 30., ( w - 2 ) / 3., 30. / 10, 1,
                       toolkit_colDark );
}

/**
 * @brief Renders the windows.
 */
void toolkit_render( double dt )
{
   (void)dt;
   Window *top = toolkit_getActiveWindow();

   NTracingZone( _ctx, 1 );

   if ( toolkit_needsRender ) {
      toolkit_needsRender = 0;

      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.fbo[3] );
      glClearColor( 0., 0., 0., 0. );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                           GL_ONE_MINUS_SRC_ALPHA );

      /* Render base. */
      for ( Window *w = windows; w != NULL; w = w->next ) {
         if ( window_isFlag( w, WINDOW_NORENDER | WINDOW_KILL ) )
            continue;
         if ( ( w == top ) && window_isFlag( w, WINDOW_DYNAMIC ) )
            continue;

         /* The actual rendering. */
         window_render( w, w == top );
      }

      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );
      glClearColor( 0., 0., 0., 1. );
   }

   /* We can just rendered stored FBO onto the screen. */
   const mat4 ortho = mat4_ortho( 0., 1., 0., 1., 1., -1. );
   const mat4 I     = mat4_identity();
   gl_renderTextureRawH( gl_screen.fbo_tex[3], &ortho, &I, &cWhite );

   /* We render only the active window dynamically, otherwise we wouldn't be
    * able to respect the order. However, since the dynamic stuff is also
    * rendered to the framebuffer below, it shouldn't be too bad. */
   if ( ( top != NULL ) &&
        !window_isFlag( top, WINDOW_NORENDER | WINDOW_KILL ) ) {
      window_renderDynamic( top );
      window_renderOverlay( top );
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Marks the toolkit for needing a full rerender.
 */
void toolkit_rerender( void )
{
   toolkit_needsRender = 1;
}

/**
 * @brief Toolkit input handled here.
 *
 *    @param event Event to handle.
 *    @return 1 if input was used, 0 if it wasn't.
 */
int toolkit_input( SDL_Event *event )
{
   /* Get window that can be focused. */
   Window *wdw = toolkit_getActiveWindow();
   if ( wdw == NULL )
      return 0;

   /* Pass event to window. */
   return toolkit_inputWindow( wdw, event, 1 );
}

/**
 * @brief Toolkit window input is handled here.
 */
int toolkit_inputWindow( Window *wdw, SDL_Event *event, int purge )
{
   int ret;

   /* See if widget needs event. */
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next ) {
      if ( !wgt_isFlag( wgt, WGT_FLAG_RAWINPUT ) )
         continue;
      if ( wgt->rawevent == NULL )
         continue;
      ret = wgt->rawevent( wgt, event );
      if ( ret != 0 ) {
         toolkit_rerender();
         return ret;
      }
   }

   /* Event handler. */
   if ( wdw->eventevent != NULL ) {
      ret = wdw->eventevent( wdw->id, event );
      if ( ret != 0 ) {
         toolkit_rerender();
         return ret;
      }
   }

   /* Hack in case window got destroyed in eventevent. */
   ret = 0;
   if ( !window_isFlag( wdw, WINDOW_KILL ) ) {
      /* Pass it on. */
      switch ( event->type ) {
      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEWHEEL:
         ret |= toolkit_mouseEvent( wdw, event );
         break;

      case SDL_KEYDOWN:
      case SDL_KEYUP:
         ret |= toolkit_keyEvent( wdw, event );
         break;

      case SDL_TEXTINPUT:
         ret |= toolkit_textEvent( wdw, event );
         break;
      case SDL_TEXTEDITING:
         break;
      }
   }
   if ( ret )
      toolkit_rerender();

   /* Clean up the dead if needed. */
   if ( purge &&
        !dialogue_isOpen() ) { /* Hack, since dialogues use secondary loop. */
      if ( toolkit_delayCounter > 0 )
         toolkit_delayCounter--;
      else
         toolkit_purgeDead();
   }

   return ret; /* don't block input */
}

/**
 * @brief Translates the mouse coordinates.
 *
 *    @param w Window to translate coords for.
 *    @param event Event to translate coords.
 *    @param[out] x Resulting X coord in window space.
 *    @param[out] y Resulting Y coord in window space.
 *    @param[out] rx Relative X movement (only valid for motion).
 *    @param[out] ry Relative Y movement (only valid for motion).
 *    @return The type of the event.
 */
Uint32 toolkit_inputTranslateCoords( const Window *w, SDL_Event *event, int *x,
                                     int *y, int *rx, int *ry )
{
   /* Extract the position as event. */
   if ( event->type == SDL_MOUSEMOTION ) {
      *x = event->motion.x;
      *y = event->motion.y;
   } else if ( ( event->type == SDL_MOUSEBUTTONDOWN ) ||
               ( event->type == SDL_MOUSEBUTTONUP ) ) {
      *x = event->button.x;
      *y = event->button.y;
   } else if ( event->type == SDL_MOUSEWHEEL )
      SDL_GetMouseState( x, y );

   /* Translate offset. */
   gl_windowToScreenPos( x, y, *x, *y );

   /* Transform to relative to window. */
   *x -= w->x;
   *y -= w->y;

   /* Relative only matter if mouse motion. */
   if ( event->type == SDL_MOUSEMOTION ) {
      *ry = (double)event->motion.yrel * gl_screen.mxscale;
      *rx = (double)event->motion.xrel * gl_screen.myscale;
   } else {
      *ry = 0;
      *rx = 0;
   }

   return event->type;
}

static int toolkit_mouseEventSingle( Window *w, SDL_Event *event, Widget *wgt,
                                     int x, int y, int rx, int ry )
{
   int ret = 0;
   /* Custom widgets take it from here */
   if ( wgt->type == WIDGET_CUST ) {
      if ( wgt->dat.cst.mouse )
         ret = wgt->dat.cst.mouse( w->id, event, x - wgt->x, y - wgt->y, wgt->w,
                                   wgt->h, rx, ry, wgt->dat.cst.userdata );
   } else
      ret = toolkit_mouseEventWidget( w, wgt, event, x, y, rx, ry );
   return ret;
}
static int toolkit_mouseEventReverse( Window *w, SDL_Event *event, Widget *wgt,
                                      int x, int y, int rx, int ry )
{
   if ( wgt->next != NULL ) {
      int ret = toolkit_mouseEventReverse( w, event, wgt->next, x, y, rx, ry );
      if ( ret )
         return ret;
   }

   return toolkit_mouseEventSingle( w, event, wgt, x, y, rx, ry );
}
/**
 * @brief Handles the mouse events.
 *
 *    @param w Window receiving the mouse event.
 *    @param event Mouse event to handle.
 */
static int toolkit_mouseEvent( Window *w, SDL_Event *event )
{
   int x, y, rx, ry;

   /* Translate mouse coords. */
   toolkit_inputTranslateCoords( w, event, &x, &y, &rx, &ry );

   /* Check each widget. */
   if ( w->widgets != NULL )
      return toolkit_mouseEventReverse( w, event, w->widgets, x, y, rx, ry );
   return 0;
}

/**
 * @brief Handle widget mouse input.
 *
 *    @param w Window to which widget belongs.
 *    @param wgt Widget receiving event.
 *    @param event Event received by the window.
 *    @param x X coordinate in window space.
 *    @param y Y coordinate in window space.
 *    @param rx Relative X movement (only valid for motion).
 *    @param ry Relative Y movement (only valid for motion).
 */
static int toolkit_mouseEventWidget( Window *w, Widget *wgt, SDL_Event *event,
                                     int x, int y, int rx, int ry )
{
   int   ret, inbounds;
   Uint8 button;

   /* Widget translations. */
   x -= wgt->x;
   y -= wgt->y;

   /* Handle mouse event. */
   if ( event->type == SDL_MOUSEMOTION )
      button = event->motion.state;
   else
      button = event->button.button;

   /* Check inbounds. */
   inbounds = !( ( x < 0 ) || ( x >= wgt->w ) || ( y < 0 ) || ( y >= wgt->h ) );

   /* Regular widgets. */
   ret = 0;
   switch ( event->type ) {
   case SDL_MOUSEMOTION:
      /* Change the status of the widget if mouse isn't down. */

      /* Not scrolling. */
      if ( wgt->status != WIDGET_STATUS_SCROLLING ) {
         if ( inbounds ) {
            if ( wgt->status != WIDGET_STATUS_MOUSEDOWN )
               widget_setStatus( wgt, WIDGET_STATUS_MOUSEOVER );
         } else
            widget_setStatus( wgt, WIDGET_STATUS_NORMAL );
      } else
         inbounds = 1; /* Scrolling is always inbounds. */

      /* If always gets the event. */
      if ( wgt_isFlag( wgt, WGT_FLAG_ALWAYSMMOVE ) )
         inbounds = 1;

      /* Try to give the event to the widget. */
      if ( inbounds && ( wgt->mmoveevent != NULL ) )
         ret |= ( *wgt->mmoveevent )( wgt, x, y, rx, ry );

      break;

   case SDL_MOUSEWHEEL:
      if ( !inbounds )
         break;

      /* Try to give the event to the widget. */
      if ( wgt->mwheelevent != NULL )
         ret |= ( *wgt->mwheelevent )( wgt, event->wheel );
      if ( ret )
         toolkit_rerender();

      break;

   case SDL_MOUSEBUTTONDOWN:
      if ( !inbounds )
         break;

      /* Update the status. */
      if ( button == SDL_BUTTON_LEFT )
         widget_setStatus( wgt, WIDGET_STATUS_MOUSEDOWN );

      if ( toolkit_isFocusable( wgt ) ) {
         toolkit_focusClear( w );
         toolkit_focusWidget( w, wgt );
      }

      /* Try to give the event to the widget. */
      if ( wgt->mdoubleclickevent != NULL &&
           input_isDoubleClick( (void *)wgt ) )
         ret |= ( *wgt->mdoubleclickevent )( wgt, button, x, y );
      else if ( wgt->mclickevent != NULL )
         ret |= ( *wgt->mclickevent )( wgt, button, x, y );
      if ( ret ) {
         input_clicked( (void *)wgt );
         toolkit_rerender();
      }
      break;

   case SDL_MOUSEBUTTONUP:
      /* Since basically only buttons are handled here, we ignore
       * it all except the left mouse button. */
      if ( button != SDL_BUTTON_LEFT )
         break;

      if ( wgt->status == WIDGET_STATUS_MOUSEDOWN ) {
         /* Soft-disabled buttons will run anyway. */
         if ( ( wgt->type == WIDGET_BUTTON ) &&
              ( ( wgt->dat.btn.disabled == 0 ) ||
                ( wgt->dat.btn.softdisable ) ) ) {
            if ( wgt->dat.btn.fptr == NULL )
               DEBUG( _( "Toolkit: Button '%s' of Window '%s' "
                         "doesn't have a function trigger" ),
                      wgt->name, w->displayname );
            else {
               ( *wgt->dat.btn.fptr )( w->id, wgt->name );
               ret = 1;
               toolkit_rerender();
            }
         }
      }

      /* Signal scroll done if necessary. */
      if ( ( wgt->status == WIDGET_STATUS_SCROLLING ) &&
           ( wgt->scrolldone != NULL ) )
         wgt->scrolldone( wgt );

      /* Always goes normal unless is below mouse. */
      if ( inbounds )
         widget_setStatus( wgt, WIDGET_STATUS_MOUSEOVER );
      else
         widget_setStatus( wgt, WIDGET_STATUS_NORMAL );

      break;
   }

   return ret;
}

/**
 * @brief Maps modifier keysyms (ctrl, alt, shift) to SDL_Keymods.
 *
 *    @param key Key to convert.
 *    @return The SDL_Keymod corresponding to the key, or 0 if none correspond.
 */
static SDL_Keymod toolkit_mapMod( SDL_Keycode key )
{
   switch ( key ) {
   case SDLK_LCTRL:
      return KMOD_LCTRL;
   case SDLK_RCTRL:
      return KMOD_RCTRL;
   case SDLK_LALT:
      return KMOD_LALT;
   case SDLK_RALT:
      return KMOD_RALT;
   case SDLK_LSHIFT:
      return KMOD_LSHIFT;
   case SDLK_RSHIFT:
      return KMOD_RSHIFT;
   default:
      return 0;
   }
}

/**
 * @brief Registers a key as down (for key repetition).
 *
 *    @param key Key to register as down.
 */
static void toolkit_regKey( SDL_Keycode key )
{
   /* See if our key is in fact a modifier key, and if it is, convert it to a
    * mod. If it is indeed a mod, do not register a new key but add the modifier
    * to the mod mask instead.
    */
   SDL_Keymod mod = toolkit_mapMod( key );
   if ( mod )
      input_mod |= mod;
   /* Don't reset values on repeat keydowns. */
   else
      input_key = key;
}

/**
 * @brief Unregisters a key.
 *
 *    @param key Key to unregister.
 */
static void toolkit_unregKey( SDL_Keycode key )
{
   /* See if our key is in fact a modifier key, and if it is, convert it to a
    * mod. If it is indeed a mod, do not unregister the key but subtract the
    * modifier from the mod mask instead.
    */
   SDL_Keymod mod = toolkit_mapMod( key );
   if ( mod )
      input_mod &= ~mod;
   else
      toolkit_clearKey();
}

/**
 * @brief Clears the registered keys.
 */
void toolkit_clearKey( void )
{
   input_key = 0;
}
/**
 * @brief Handles keyboard events.
 *
 *    @param wdw Window receiving the key event.
 *    @param event Keyboard event to handle.
 *    @return 1 if the event is used, 0 if it isn't.
 */
static int toolkit_keyEvent( Window *wdw, SDL_Event *event )
{
   Widget     *wgt;
   SDL_Keycode key;
   SDL_Keymod  mod;
   int         rep, ret;

   /* Event info. */
   key = event->key.keysym.sym;
   mod = event->key.keysym.mod;
   rep = event->key.repeat;

   /* Hack to simulate key repetition */
   if ( event->type == SDL_KEYDOWN )
      toolkit_regKey( key );
   else if ( event->type == SDL_KEYUP )
      toolkit_unregKey( key );

   /* See if window is valid. */
   if ( wdw == NULL )
      return 0;

   /* Get widget. */
   wgt = toolkit_getFocus( wdw );

   /* We only want keydown from now on. */
   if ( event->type != SDL_KEYDOWN )
      return 0;

   /* Trigger event function if exists. */
   if ( wgt != NULL ) {
      if ( wgt->keyevent != NULL ) {
         ret = wgt->keyevent( wgt, input_key, input_mod, rep );
         if ( ret != 0 )
            return ret;
      }
   }

   if ( input_key != 0 && !rep ) {
      /* Handle button hotkeys. We don't want a held-down key to keep activating
       * buttons, so forbid "repeat". */
      for ( wgt = wdw->widgets; wgt != NULL; wgt = wgt->next ) {
         if ( ( wgt->type == WIDGET_BUTTON ) &&
              ( wgt->dat.btn.key == input_key ) && wgt->keyevent != NULL ) {
            ret = wgt->keyevent( wgt, SDLK_RETURN, input_mod, rep );
            if ( ret != 0 )
               return ret;
         }
      }

      /* Handle other cases where event might be used by the window... and we
       * don't want key-repeat. */
      switch ( key ) {
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
         if ( wdw->accept_fptr != NULL ) {
            wdw->accept_fptr( wdw->id, wdw->name );
            return 1;
         }
         break;

      case SDLK_ESCAPE:
         if ( wdw->cancel_fptr != NULL ) {
            wdw->cancel_fptr( wdw->id, wdw->name );
            return 1;
         }
         break;

      default:
         break;
      }
   }

   /* Finally the stuff gets passed to the custom key handler if it's defined.
    */
   if ( wdw->keyevent != NULL ) {
      ret = ( *wdw->keyevent )( wdw->id, input_key, input_mod, rep );
      if ( ret != 0 )
         return ret;
   }

   /* Placed here so it can be overriden in console for tab completion. */
   if ( key == SDLK_TAB ) {
      if ( mod & ( KMOD_LSHIFT | KMOD_RSHIFT ) )
         toolkit_prevFocus( wdw );
      else
         toolkit_nextFocus( wdw );
      return 1;
   }

   return 0;
}
static int toolkit_textEvent( Window *wdw, SDL_Event *event )
{
   Widget *wgt;

   /* See if window is valid. */
   if ( wdw == NULL )
      return 0;

   /* Get widget. */
   wgt = toolkit_getFocus( wdw );

   /* Trigger event function if exists. */
   if ( ( wgt != NULL ) && ( wgt->textevent != NULL ) ) {
      int ret = ( *wgt->textevent )( wgt, event->text.text );
      if ( ret != 0 )
         return ret;
   }

   return 0;
}

/**
 * @brief Purges the dead windows.
 */
static void toolkit_purgeDead( void )
{
   Window *wdw, *wlast;

   /* Must be windows. */
   if ( windows == NULL )
      return;

   /* Destroy what is needed. */
   wlast = NULL;
   wdw   = windows;
   while ( wdw != NULL ) {
      if ( window_isFlag( wdw, WINDOW_KILL ) ) {
         /* Save target. */
         Window *wkill = wdw;
         /* Clean up while still in list. */
         window_cleanup( wdw );
         /* Reattach linked list. */
         if ( wlast == NULL )
            windows = wdw->next;
         else
            wlast->next = wdw->next;
         wdw = wlast;
         /* Kill target. */
         wkill->next = NULL;
         window_remove( wkill );
      } else {
         Widget *wgtlast = NULL;
         Widget *wgt     = wdw->widgets;
         while ( wgt != NULL ) {
            if ( wgt_isFlag( wgt, WGT_FLAG_KILL ) ) {
               /* Save target. */
               Widget *wgtkill = wgt;
               /* Reattach linked list. */
               if ( wgtlast == NULL )
                  wdw->widgets = wgt->next;
               else
                  wgtlast->next = wgt->next;
               wgt = wgtlast;
               /* Kill target. */
               wgtkill->next = NULL;
               widget_kill( wgtkill );
            }
            /* Save position. */
            wgtlast = wgt;
            if ( wgt == NULL )
               wgt = wdw->widgets;
            else
               wgt = wgt->next;
         }
      }
      /* Save position. */
      wlast = wdw;
      if ( wdw == NULL )
         wdw = windows;
      else
         wdw = wdw->next;
   }
}

/**
 * @brief Updates the toolkit input for repeating keys.
 */
void toolkit_update( void )
{
   /* Clean up the dead if needed. */
   if ( !dialogue_isOpen() ) { /* Hack, since dialogues use secondary loop. */
      if ( toolkit_delayCounter > 0 )
         toolkit_delayCounter--;
      else
         toolkit_purgeDead();
   }

   /* Killed all the windows. */
   if ( !toolkit_isOpen() ) {

      input_mouseHide();
      if ( paused && !player_paused )
         unpause_game();
   }
}

/**
 * @brief Exposes or hides a window and notifies its widgets.
 *
 *    @param wdw Window to change exposure of.
 *    @param expose Whether exposing or hiding.
 */
static void toolkit_expose( Window *wdw, int expose )
{
   if ( expose == wdw->exposed )
      return;
   else
      wdw->exposed = expose;

   if ( expose )
      toolkit_focusSanitize( wdw );
   else
      toolkit_focusClear( wdw );

   if ( wdw->focus != -1 )
      return;

   /* Notify widgets (for tabbed children, etc.) */
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next )
      if ( wgt->exposeevent != NULL )
         wgt->exposeevent( wgt, expose );
}

/**
 * @brief Clears the window focus.
 */
void toolkit_focusClear( Window *wdw )
{
   if ( wdw->focus == -1 )
      return;

   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next )
      toolkit_defocusWidget( wdw, wgt );
}

/**
 * @brief Sanitizes the focus of a window.
 *
 * Makes sure the window has a focusable widget focused.
 */
void toolkit_focusSanitize( Window *wdw )
{
   int focus = wdw->focus;

   /* Clear focus. */
   toolkit_focusClear( wdw );

   /* No focus is always safe. */
   if ( focus == -1 )
      return;

   /* Check focused widget. */
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next ) {
      if ( focus == wgt->id ) {
         /* Not focusable. */
         if ( !toolkit_isFocusable( wgt ) )
            toolkit_nextFocus( wdw ); /* Get first focus. */
         else
            toolkit_focusWidget( wdw, wgt );

         return;
      }
   }
}

/**
 * @brief Focus next widget.
 */
void toolkit_nextFocus( Window *wdw )
{
   int next;

   /* Clear focus. */
   toolkit_focusClear( wdw );

   /* See what to focus. */
   next = ( wdw->focus == -1 );
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next ) {
      if ( !toolkit_isFocusable( wgt ) )
         continue;

      if ( next ) {
         toolkit_focusWidget( wdw, wgt );
         return;
      } else if ( wdw->focus == wgt->id ) {
         next = 1;
      }
   }

   /* Focus nothing. */
   wdw->focus = -1;
   toolkit_rerender();
   return;
}

/**
 * @brief Focus previous widget.
 */
void toolkit_prevFocus( Window *wdw )
{
   Widget *prev;
   int     focus = wdw->focus;

   /* Clear focus. */
   toolkit_focusClear( wdw );

   /* See what to focus. */
   prev = NULL;
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next ) {
      if ( !toolkit_isFocusable( wgt ) )
         continue;

      /* See if we found the current one. */
      if ( focus == wgt->id ) {
         if ( prev != NULL )
            toolkit_focusWidget( wdw, prev );
         return;
      }

      /* Store last focusable widget. */
      prev = wgt;
   }

   /* Focus nothing. */
   if ( prev != NULL )
      toolkit_focusWidget( wdw, prev );

   return;
}

/**
 * @brief Focuses a widget in a window. No-op if it's not focusable or already
 * focused.
 */
void toolkit_focusWidget( Window *wdw, Widget *wgt )
{
   if ( !toolkit_isFocusable( wgt ) || wgt_isFlag( wgt, WGT_FLAG_FOCUSED ) )
      return;

   wdw->focus = wgt->id;
   wgt_setFlag( wgt, WGT_FLAG_FOCUSED );
   if ( wgt->focusGain != NULL )
      wgt->focusGain( wgt );

   toolkit_rerender();
}

/**
 * @brief Defocuses the focused widget in a window. No-op if it's not
 * (de)focusable or already defocused, else voids the window's focus.
 */
void toolkit_defocusWidget( Window *wdw, Widget *wgt )
{
   if ( wdw->focus != wgt->id || !wgt_isFlag( wgt, WGT_FLAG_FOCUSED ) )
      return;

   wdw->focus = -1;
   wgt_rmFlag( wgt, WGT_FLAG_FOCUSED );
   if ( wgt->focusLose != NULL )
      wgt->focusLose( wgt );

   toolkit_rerender();
}

/**
 * @brief Checks to see if a widget is focusable.
 *
 *    @param wgt Widget to check if is focusable.
 *    @return 1 if it's focusable, 0 if it isn't.
 */
static int toolkit_isFocusable( const Widget *wgt )
{
   if ( wgt == NULL )
      return 0;

   return wgt_isFlag( wgt, WGT_FLAG_CANFOCUS );
}

/**
 * @brief Gets the active window in the toolkit.
 *
 *    @return The active window in the toolkit.
 */
Window *toolkit_getActiveWindow( void )
{
   /* Get window that can be focused. */
   Window *wlast = NULL;
   for ( Window *wdw = windows; wdw != NULL; wdw = wdw->next )
      if ( !window_isFlag( wdw, WINDOW_NOFOCUS ) &&
           !window_isFlag( wdw, WINDOW_KILL ) )
         wlast = wdw;
   return wlast;
}

/**
 * @brief Gets the focused widget in a window.
 *
 *    @param wdw The window to get the focused widget from.
 *    @return The focused widget.
 */
Widget *toolkit_getFocus( Window *wdw )
{
   /* No focus. */
   if ( wdw->focus == -1 )
      return NULL;

   /* Find focus. */
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next )
      if ( wdw->focus == wgt->id )
         return wgt;

   /* Not found. */
   toolkit_focusClear( wdw );
   wdw->focus = -1;
   return NULL;
}

/**
 * @brief Sets the focused widget in a window.
 *
 *    @param wid ID of the window to get widget from.
 *    @param wgtname Name of the widget to set focus to,
 *                   or NULL to clear the focus.
 */
void window_setFocus( unsigned int wid, const char *wgtname )
{
   Window *wdw;
   Widget *wgt;

   /* Get window. */
   wdw = window_wget( wid );
   if ( wdw == NULL )
      return;

   toolkit_focusClear( wdw );

   /* Get widget. */
   wgt = wgtname == NULL ? NULL : window_getwgt( wid, wgtname );
   if ( wgt == NULL )
      return;

   toolkit_focusWidget( wdw, wgt );
}

/**
 * @brief Gets the focused widget in a window (does strdup!!).
 *
 *    @param wid ID of the window to get widget from.
 *    @return The focused widget's name (strdup()ed string or NULL).
 */
char *window_getFocus( unsigned int wid )
{
   /* Get window. */
   Window *wdw = window_wget( wid );
   if ( wdw == NULL )
      return NULL;

   /* Find focused widget. */
   for ( Widget *wgt = wdw->widgets; wgt != NULL; wgt = wgt->next )
      if ( wgt->id == wdw->focus )
         return strdup( wgt->name );

   return NULL;
}

/**
 * @brief Raises a window (causes all other windows to appear below it).
 *
 *    @param wid Window to raise.
 */
void window_raise( unsigned int wid )
{
   Window *wdw, *wtmp, *wprev, *wlast;

   wdw = window_wget( wid );

   /* Not found, or already top of the stack. */
   if ( wdw == NULL || wdw->next == NULL )
      return;

   wprev = NULL;
   wlast = NULL;

   for ( wtmp = windows; wtmp != NULL; wtmp = wtmp->next )
      if ( wtmp->next == wdw )
         wprev = wtmp;
      else if ( wtmp->next == NULL )
         wlast = wtmp;

   if ( wprev != NULL )
      wprev->next = wdw->next; /* wdw-1 links to wdw+1 */

   if ( wlast != NULL )
      wlast->next = wdw; /* last links to wdw */

   wdw->next = NULL; /* wdw becomes new last window */

   wtmp = toolkit_getActiveWindow();

   /* No active window, or window is the same. */
   if ( wtmp == NULL || wtmp == wdw )
      return;

   toolkit_expose( wtmp, 0 ); /* wtmp is hidden */
   toolkit_expose( wdw, 1 );  /* wdw is visible */
   toolkit_rerender();
}

/**
 * @brief Lowers a window (causes all other windows to appear above it).
 *
 *    @param wid Window to lower.
 */
void window_lower( unsigned int wid )
{
   Window *wdw, *wtmp, *wprev;

   wdw = window_wget( wid );

   /* Not found, or already bottom of the stack. */
   if ( wdw == NULL || wdw == windows )
      return;

   wprev = NULL;
   for ( wtmp = windows; wtmp != NULL; wtmp = wtmp->next )
      if ( wtmp->next == wdw )
         wprev = wtmp;

   if ( wprev != NULL )
      wprev->next = wdw->next; /* wdw-1 links to wdw+1 */

   wdw->next = windows; /* wdw links to first window */
   windows   = wdw;     /* wdw becomes new first window */

   wtmp = toolkit_getActiveWindow();

   /* No active window, or window is the same. */
   if ( wtmp == NULL || wtmp == wdw )
      return;

   toolkit_expose( wtmp, 1 ); /* wtmp is visible */
   toolkit_expose( wdw, 0 );  /* wdw is hidden */
   toolkit_rerender();
}

/**
 * @brief Repositions windows and their children if resolution changes.
 */
void toolkit_resize( void )
{
   for ( Window *w = windows; w != NULL; w = w->next ) {
      int xdiff, ydiff;

      /* Fullscreen windows must always be full size, though their widgets
       * don't auto-scale. */
      if ( window_isFlag( w, WINDOW_FULLSCREEN ) ) {
         w->w = gl_screen.nw;
         w->h = gl_screen.nh;
         continue;
      }

      /* Skip if position is fixed. */
      if ( w->xrel == -1. && w->yrel == -1. )
         continue;

      xdiff = 0.;
      ydiff = 0.;

      if ( w->xrel != -1. ) {
         int xorig = w->x;
         w->x      = ( gl_screen.nw - w->w ) * w->xrel;
         xdiff     = w->x - xorig;
      }

      if ( w->yrel != -1. ) {
         int yorig = w->y;
         w->y      = ( gl_screen.nh - w->h ) * w->yrel;
         ydiff     = w->y - yorig;
      }

      /* Tabwin children aren't in the stack and must be manually updated. */
      for ( Widget *wgt = w->widgets; wgt != NULL; wgt = wgt->next ) {
         if ( wgt->type != WIDGET_TABBEDWINDOW )
            continue;

         for ( int i = 0; i < wgt->dat.tab.ntabs; i++ ) {
            Window *wtmp = window_wget( wgt->dat.tab.windows[i] );
            wtmp->x += xdiff;
            wtmp->y += ydiff;
         }
      }
   }
   toolkit_rerender();
}

/**
 * @brief Initializes the toolkit.
 *
 *    @return 0 on success.
 */
int toolkit_init( void )
{
   GLsizei size;

   /* Create the VBO. */
   toolkit_vboColourOffset = sizeof( GLshort ) * 2 * 31;
   size        = ( sizeof( GLshort ) * 2 + sizeof( GLfloat ) * 4 ) * 31;
   toolkit_vbo = gl_vboCreateStream( size, NULL );

   /* Disable the cursor. */
   input_mouseHide();

   return 0;
}

/**
 * @brief Exits the toolkit.
 */
void toolkit_exit( void )
{
   /* Destroy the windows. */
   while ( windows != NULL ) {
      Window *wdw = windows;
      windows     = windows->next;
      window_cleanup( wdw );
      window_remove( wdw );
   }

   /* Free the VBO. */
   gl_vboDestroy( toolkit_vbo );
   toolkit_vbo = NULL;
}
