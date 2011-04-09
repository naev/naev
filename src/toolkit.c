/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file toolkit.c
 *
 * @brief Handles windows and widgets.
 */


#include "toolkit.h"

#include "naev.h"

#include <stdarg.h>

#include "tk/toolkit_priv.h"

#include "log.h"
#include "pause.h"
#include "opengl.h"
#include "input.h"
#include "nstd.h"
#include "dialogue.h"
#include "conf.h"


#define INPUT_DELAY      conf.repeat_delay /**< Delay before starting to repeat. */
#define INPUT_FREQ       conf.repeat_freq /**< Interval between repetition. */


static unsigned int genwid = 0; /**< Generates unique window ids, > 0 */


static int toolkit_open = 0; /**< 1 if toolkit is in use, 0 else. */
static int toolkit_delayCounter = 0; /**< Horrible hack around secondary loop. */


/*
 * window stuff
 */
#define MIN_WINDOWS  3 /**< Minimum windows to prealloc. */
static Window *windows = NULL; /**< Window linked list, not to be confused with MS windows. */
static int window_dead = 0; /**< There are dead windows lying around. */


/*
 * simulate keypresses when holding
 */
static SDLKey input_key             = 0; /**< Current pressed key. */
static unsigned int input_keyTime   = 0; /**< Tick pressed. */
static int input_keyCounter         = 0; /**< Number of repetitions. */
static char input_text              = 0; /**< Current character. */


/*
 * default outline colours
 */
glColour* toolkit_colLight = &cGrey90; /**< Light outline colour. */
glColour* toolkit_col      = &cGrey70; /**< Normal outline colour. */
glColour* toolkit_colDark  = &cGrey30; /**< Dark outline colour. */


/*
 * VBO
 */
static gl_vbo *toolkit_vbo; /**< Toolkit VBO. */
static GLsizei toolkit_vboColourOffset; /**< Colour offset. */


/*
 * static prototypes
 */
/* input */
static void toolkit_mouseEvent( Window *w, SDL_Event* event );
static void toolkit_mouseEventWidget( Window *w, Widget *wgt,
      Uint32 type, Uint8 button, int x, int y, int rx, int ry );
static int toolkit_keyEvent( Window *wdw, SDL_Event* event );
/* focus */
static void toolkit_focusClear( Window *wdw );
static int toolkit_isFocusable( Widget *wgt );
static Widget* toolkit_getFocus( Window *wdw );
/* render */
static void window_renderBorder( Window* w );
/* Death. */
static void widget_kill( Widget *wgt );
static void window_kill( Window *wdw );
static void toolkit_purgeDead (void);


/**
 * @brief Checks to see if the toolkit is open.
 *
 *    @return 1 if the toolkit is open.
 */
int toolkit_isOpen (void)
{
   return !!toolkit_open;
}


/**
 * @brief Delays the toolkit purge by an iteration, useful for dialogues.
 */
void toolkit_delay (void)
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
void toolkit_setPos( Window *wdw, Widget *wgt, int x, int y )
{
   /* X position. */
   if (x < 0)
      wgt->x = wdw->w - wgt->w + x;
   else
      wgt->x = (double) x;

   /* Y position. */
   if (y < 0)
      wgt->y = wdw->h - wgt->h + y;
   else
      wgt->y = (double) y;
}


/**
 * @brief Allocates room for a new widget.
 *
 *    @param w Window to create widget in.
 *    @return Newly allocated widget.
 */
Widget* window_newWidget( Window* w, const char *name )
{
   Widget *wgt, *wlast, *wtmp;

   /* NULL protection. */
   if (w==NULL)
      return NULL;

   /* Try to find one with the same name first. */
   wlast = NULL;
   for (wgt=w->widgets; wgt!=NULL; wgt=wgt->next) {

      /* Must match name. */
      if (strcmp(name, wgt->name)!=0) {
         wlast = wgt;
         continue;
      }

      /* Should be destroyed. */
      if (!wgt_isFlag( wgt, WGT_FLAG_KILL )) {
         WARN("Trying to create widget '%s' over existing one that hasn't been destroyed",
               name );
         return NULL;
      }

      /* Relink. */
      if (wlast==NULL)
         w->widgets  = wgt->next;
      else
         wlast->next = wgt->next;

      /* Prepare and return this widget. */
      widget_cleanup(wgt);
      break;
   }

   /* Must grow widgets. */
   if (wgt == NULL)
      wgt = malloc( sizeof(Widget) );

   /* Sane defaults. */
   memset( wgt, 0, sizeof(Widget) );
   wgt->type   = WIDGET_NULL;
   wgt->status = WIDGET_STATUS_NORMAL;
   wgt->wdw    = w->id;
   wgt->name   = strdup(name);
   wgt->id     = ++w->idgen;

   /* Set up. */
   wlast = NULL;
   for (wtmp=w->widgets; wtmp!=NULL; wtmp=wtmp->next)
      wlast = wtmp;
   if (wlast == NULL)
      w->widgets  = wgt;
   else
      wlast->next = wgt;

   return wgt;
}


/**
 * @brief Gets a Window by ID.
 *
 *    @param wid ID of the window to get.
 *    @return Window matching wid.
 */
Window* window_wget( const unsigned int wid )
{
   Window *w;
   if (windows == NULL)
      return NULL;
   for (w = windows; w != NULL; w = w->next)
      if (w->id == wid)
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
Widget* window_getwgt( const unsigned int wid, const char* name )
{
   Window *wdw;
   Widget *wgt;

   /* Get the window. */
   wdw = window_wget(wid);
   if (wdw == NULL)
      return NULL;

   /* Find the widget. */
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next)
      if (strcmp(wgt->name, name)==0)
         return wgt;

   WARN("Widget '%s' not found in window '%u'!", name, wid );
   return NULL;
}


/**
 * @brief Gets the dimensions of a window.
 *
 *    @param wid ID of the window to get dimension of.
 *    @param[out] w Width of the window or -1 on error.
 *    @param[out] h Height of the window or -1 on error.
 */
void window_dimWindow( const unsigned int wid, int *w, int *h )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget(wid);
   if (wdw == NULL) {
      *w = -1;
      *h = -1;
      return;
   }

   /* Set dimensions. */
   *w = wdw->w;
   *h = wdw->h;
}

/**
 * @brief Gets a widget's position.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to get position of.
 *    @param[out] x X position of the widget.
 *    @param[out] y Y position of the widget.
 */
void window_posWidget( const unsigned int wid,
      char* name, int *x, int *y )
{
   Widget *wgt;

   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Return position. */
   if (x != NULL)
      (*x) = wgt->x;
   if (y != NULL)
      (*y) = wgt->y;
}


/**
 * @brief Moves a widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to set position to.
 *    @param x New X position to set widget to.
 *    @param y New Y position to set widget to.
 */
void window_moveWidget( const unsigned int wid,
      char* name, int x, int y )
{
   Window *wdw;
   Widget *wgt;

   /* Get window. */
   wdw = window_wget(wid);
   if (wdw == NULL)
      return;

   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Set position. */
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Checks to see if a window exists.
 *
 *    @param wdwname Name of the window to check.
 *    @return 1 if it exists, 0 if it doesn't.
 */
int window_exists( const char* wdwname )
{
   Window *w;
   if (windows == NULL)
      return 0;
   for (w = windows; w != NULL; w = w->next)
      if ((strcmp(w->name,wdwname)==0) && !window_isFlag(w, WINDOW_KILL))
         return 1;
   return 0; /* doesn't exist */
}


/**
 * @brief Gets the ID of a window.
 *
 *    @param wdwname Name of the window to get ID of.
 *    @return ID of the window.
 */
unsigned int window_get( const char* wdwname )
{
   Window *w;
   if (windows == NULL)
      return 0;
   for (w = windows; w != NULL; w = w->next)
      if ((strcmp(w->name,wdwname)==0) && !window_isFlag(w, WINDOW_KILL))
         return w->id;
   return 0;
}


/**
 * @brief Creates a window.
 *
 *    @param name Name of the window to create.
 *    @param x X position of the window (-1 centers).
 *    @param y Y position of the window (-1 centers).
 *    @param w Width of the window (-1 fullscreen).
 *    @param h Height of the window (-1 fullscreen).
 *    @return Newly created window's ID.
 */
unsigned int window_create( const char* name,
      const int x, const int y, const int w, const int h )
{
   Window *wcur, *wlast, *wdw;

   /* Allocate memory. */
   wdw = calloc( 1, sizeof(Window) );

   const int wid = (++genwid); /* unique id */

   /* Create the window. */

   wdw->id           = wid;
   wdw->name         = strdup(name);

   /* Sane defaults. */
   wdw->idgen        = -1;
   wdw->focus        = -1;

   /* Dimensions. */
   wdw->w            = (w == -1) ? SCREEN_W : (double) w;
   wdw->h            = (h == -1) ? SCREEN_H : (double) h;
   if ((w == -1) && (h == -1)) {
      window_setFlag( wdw, WINDOW_FULLSCREEN );
      wdw->x = 0.;
      wdw->y = 0.;
   }
   else {
      /* x pos */
      if (x==-1) /* center */
         wdw->x = (SCREEN_W - wdw->w)/2.;
      else if (x < 0)
         wdw->x = SCREEN_W - wdw->w + (double) x;
      else wdw->x = (double) x;
      /* y pos */
      if (y==-1) /* center */
         wdw->y = (SCREEN_H - wdw->h)/2.;
      else if (y < 0)
         wdw->y = SCREEN_H - wdw->h + (double) y;
      else wdw->y = (double) y;
   }

   if (toolkit_open==0) { /* toolkit is on */
      input_mouseShow();
      toolkit_open = 1; /* enable toolkit */
      pause_game();
      gl_defViewport(); /* Reset the default viewport */
   }

   /* Clear key repeat. */
   toolkit_clearKey();

   /* Add to list. */
   wdw->next = NULL;
   if (windows == NULL)
      windows = wdw;
   else {
      for (wcur = windows; wcur != NULL; wcur = wcur->next) {
         if ((strcmp(wcur->name,name)==0) && !window_isFlag(wcur, WINDOW_KILL) &&
               !window_isFlag(wcur, WINDOW_NOFOCUS))
            WARN("Window with name '%s' already exists!",wcur->name);
         wlast = wcur;
      }
      wlast->next = wdw;
   }

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
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
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
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
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
 *    @param Function to tirgger when window is closed, parameter is window id
 *           and name.
 */
void window_onClose( unsigned int wid, void (*fptr)(unsigned int,char*) )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
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
void window_setAccept( const unsigned int wid, void (*accept)(unsigned int,char*) )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
      return;

   /* Set the accept function. */
   wdw->accept_fptr = accept;
}


/**
 * @brief Sets the default cancel function of the window.
 *
 * This function is called whenever 'escape' is hit and the current widget
 *  does not catch it.  NULL disables the cancel function.
 *
 *    @param wid ID of the window to set cancel function.
 *    @param cancel Function to trigger when window is "cancelled".  Parameter
 *                  passed is window name.
 */
void window_setCancel( const unsigned int wid, void (*cancel)(unsigned int,char*) )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
      return;

   /* Set the cancel function. */
   wdw->cancel_fptr = cancel;
}


/**
 * @brief Sets custom data for a window.
 *
 *    @param wid Window to set custom data of.
 *    @param data Data to set.
 */
void window_setData( unsigned int wid, void *data )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
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
void* window_getData( unsigned int wid )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
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
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
      return;

   if (enable)
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
void window_handleKeys( const unsigned int wid,
      int (*keyhandler)(unsigned int,SDLKey,SDLMod) )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
      return;

   /* Set key event handler function. */
   wdw->keyevent = keyhandler;
}


/**
 * @brief Sets the event handler for the window.
 *
 * This function is called every time the window recieves input.
 */
void window_handleEvents( const unsigned int wid,
      int (*eventhandler)(unsigned int,SDL_Event*) )
{
   Window *wdw;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
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
   if (widget->cleanup != NULL)
      widget->cleanup(widget);

   /* General freeing. */
   if (widget->name)
      free(widget->name);
}


/**
 * @brief Helper function to automatically close the window calling it.
 *
 *    @param wid Window to close.
 *    @param str Unused.
 */
void window_close( unsigned int wid, char *str )
{
   (void) str;

   window_destroy( wid );
}


/**
 * @brief Kills the window.
 *
 *    @param wid ID of window to destroy.
 *    @return 1 if windows still need killing.
 */
void window_destroy( const unsigned int wid )
{
   Window *wdw, *w;
   if (windows == NULL)
      return;
   /* Destroy the window */
   for (wdw = windows; wdw != NULL; wdw = wdw->next) {

      /* Not the window we're looking for. */
      if (wdw->id != wid)
         continue;

      /* Already being killed, skip. */
      if (window_isFlag( wdw, WINDOW_KILL ))
         continue;

      /* Mark children for death. */
      for (w = windows; w != NULL; w = w->next) {
         if (w->parent == wid) {
            window_destroy( w->id );
         }
      }

      /* Mark for death. */
      window_setFlag( wdw, WINDOW_KILL );
      window_dead = 1;

      /* Run the close function first. */
      if (wdw->close_fptr != NULL)
         wdw->close_fptr( wdw->id, wdw->name );
      wdw->close_fptr = NULL;
      break;
   }
}


/**
 * @brief Kills the window.
 *
 *    @param wid ID of window to destroy.
 */
static void window_kill( Window *wdw )
{
   Widget *wgt, *wgtkill;

   /* Run the close function first. */
   if (wdw->close_fptr != NULL)
      wdw->close_fptr( wdw->id, wdw->name );
   wdw->close_fptr = NULL;

   /* Destroy the window. */
   if (wdw->name)
      free(wdw->name);
   wgt = wdw->widgets;
   while (wgt != NULL) {
      wgtkill = wgt;
      wgt = wgtkill->next;
      widget_kill(wgtkill);
   }
   free(wdw);

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
int widget_exists( const unsigned int wid, const char* wgtname )
{
   Window *w = window_wget(wid);
   Widget *wgt;

   /* Get window. */
   if (w==NULL) {
      WARN("window '%d' does not exist", wid);
      return 0;
   }

   /* Check for widget. */
   for (wgt=w->widgets; wgt!=NULL; wgt=wgt->next)
      if (strcmp(wgtname, wgt->name)==0)
         return !wgt_isFlag(wgt, WGT_FLAG_KILL);

   return 0;
}


/**
 * @brief Destroys a widget in a window.
 *
 *    @param wid Window to destroy widget in.
 *    @param wgtname Name of the widget to destroy.
 */
void window_destroyWidget( unsigned int wid, const char* wgtname )
{
   Window *wdw;
   Widget *wgt;

   /* Get the window. */
   wdw = window_wget( wid );
   if (wdw == NULL)
      return;

   /* Get the widget. */
   /* get widget. */
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next) {
      if (strcmp(wgt->name, wgtname)==0) {
         break;
      }
   }
   if (wgt == NULL) {
      WARN("Widget '%s' not found in window '%s'", wgtname, wdw->name );
      return;
   }

   /* Defocus. */
   if (wdw->focus == wgt->id)
      wdw->focus = -1;

   /* There's dead stuff now. */
   window_dead = 1;
   wgt_rmFlag( wgt, WGT_FLAG_FOCUSED );
   wgt_setFlag( wgt, WGT_FLAG_KILL );
}


/**
 * @brief Destroy a widget really.
 */
static void widget_kill( Widget *wgt )
{
   /* Clean up. */
   widget_cleanup(wgt);
   free(wgt);
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
void toolkit_drawOutlineThick( int x, int y, int w, int h, int b,
                          int thick, glColour* c, glColour* lc )
{
   GLshort tri[5][4];
   glColour colours[10];

   /* Set shade model. */
   glShadeModel( (lc==NULL) ? GL_FLAT : GL_SMOOTH );

   x -= b - thick;
   w += 2 * (b - thick);
   y -= b - thick;
   h += 2 * (b - thick);
   lc = lc ? lc : c;

   /* Left-up. */
   tri[0][0]     = x;         /* Inner */
   tri[0][1]     = y;
   tri[0][2]     = x-thick;   /* Outter */
   tri[0][3]     = y-thick;
   colours[0]    = *lc;
   colours[1]    = *lc;

   /* Left-down. */
   tri[1][0]     = x;         /* Inner. */
   tri[1][1]     = y + h;
   tri[1][2]     = x-thick;   /* Outter. */
   tri[1][3]     = y + h+thick;
   colours[2]    = *c;
   colours[3]    = *c;

   /* Right-down. */
   tri[2][0]     = x + w;       /* Inner. */
   tri[2][1]     = y + h;
   tri[2][2]     = x + w+thick; /* Outter. */
   tri[2][3]     = y + h+thick;
   colours[4]    = *c;
   colours[5]    = *c;

   /* Right-up. */
   tri[3][0]     = x + w;       /* Inner. */
   tri[3][1]     = y;
   tri[3][2]     = x + w+thick; /* Outter. */
   tri[3][3]     = y-thick;
   colours[6]    = *lc;
   colours[7]    = *lc;

   /* Left-up. */
   tri[4][0]     = x;         /* Inner */
   tri[4][1]     = y;
   tri[4][2]     = x-thick;   /* Outter */
   tri[4][3]     = y-thick;
   colours[8]    = *lc;
   colours[9]    = *lc;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof(tri), tri );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof(colours), colours );

   /* Set up the VBO. */
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY, 0, 2, GL_SHORT, 0 );
   gl_vboActivateOffset( toolkit_vbo, GL_COLOR_ARRAY,
                         toolkit_vboColourOffset, 4, GL_FLOAT, 0 );

   /* Draw the VBO. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 10 );

   /* Deactivate VBO. */
   gl_vboDeactivate();
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
void toolkit_drawOutline( int x, int y, int w, int h, int b,
                          glColour* c, glColour* lc )
{
   GLshort lines[4][2];
   glColour colours[4];

   /* Set shade model. */
   glShadeModel( (lc==NULL) ? GL_FLAT : GL_SMOOTH );

   x -= b, w += 2 * b;
   y -= b, h += 2 * b;
   lc = lc ? lc : c;

   /* Lines. */
   lines[0][0]   = x - 1;  /* left-up */
   lines[0][1]   = y;
   colours[0]    = *lc;

   lines[1][0]   = x;      /* left-down */
   lines[1][1]   = y + h;
   colours[1]    = *c;

   lines[2][0]   = x + w;  /* right-down */
   lines[2][1]   = y + h;
   colours[2]    = *c;

   lines[3][0]   = x + w;  /* right-up */
   lines[3][1]   = y;
   colours[3]    = *lc;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof(lines), lines );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof(colours), colours );

   /* Set up the VBO. */
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY, 0, 2, GL_SHORT, 0 );
   gl_vboActivateOffset( toolkit_vbo, GL_COLOR_ARRAY,
                         toolkit_vboColourOffset, 4, GL_FLOAT, 0 );

   /* Draw the VBO. */
   glDrawArrays( GL_LINE_LOOP, 0, 4 );

   /* Deactivate VBO. */
   gl_vboDeactivate();
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
void toolkit_drawRect( int x, int y, int w, int h,
                       glColour* c, glColour* lc )
{
   GLshort vertex[4][2];
   glColour colours[4];

   /* Set shade model. */
   glShadeModel( (lc) ? GL_SMOOTH : GL_FLAT );

   lc = lc == NULL ? c : lc;

   /* Set up vertices and colours. */
   vertex[0][0] = x;        /* left-up */
   vertex[0][1] = y;
   colours[0]   = *c;

   vertex[1][0] = x;        /* left-down */
   vertex[1][1] = y + h;
   colours[1]   = *lc;

   vertex[2][0] = x + w;    /* right-up */
   vertex[2][1] = y;
   colours[2]   = *c;

   vertex[3][0] = x + w;    /* right-down */
   vertex[3][1] = y + h;
   colours[3]   = *lc;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof(vertex), vertex );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof(colours), colours );

   /* Set up the VBO. */
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY,
                         0, 2, GL_SHORT, 0 );
   gl_vboActivateOffset( toolkit_vbo, GL_COLOR_ARRAY,
                         toolkit_vboColourOffset, 4, GL_FLOAT, 0 );

   /* Draw the VBO. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Deactivate VBO. */
   gl_vboDeactivate();
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
   double w, h;
   double x, y, o;
   glColour c;
   glColour c2;

   /* Get dimensions. */
   w = 160.;
   h = gl_printHeightRaw( &gl_smallFont, w, alt );
   /* One check to make bigger. */
   if (h > 160.) {
      w = 200;
      h = gl_printHeightRaw( &gl_smallFont, w, alt );
   }

   /* Choose position. */
   x = bx + 10.;
   y = by - h - gl_smallFont.h - 10.;
   if (y < -SCREEN_H/2+10) {
      o  = -(SCREEN_H/2 + y) + 10;
      y += o;
   }

   /* Set colours. */
   c.r = cGrey80.r;
   c.g = cGrey80.g;
   c.b = cGrey80.b;
   c.a = 0.8;
   c2.r = cGrey30.r;
   c2.g = cGrey30.g;
   c2.b = cGrey30.b;
   c2.a = 0.7;
   toolkit_drawRect( x-1, y-5, w+6, h+6, &c2, NULL );
   toolkit_drawRect( x-3, y-3, w+6, h+6, &c, NULL );
   gl_printTextRaw( &gl_smallFont, w, h, x, y, &cBlack, alt );
}


/**
 * @brief Renders a window border.
 *
 *    @param w Window to render
 */
static void window_renderBorder( Window* w )
{
   int i;
   GLshort cx, cy;
   double x, y;
   glColour *lc, *c, *dc, *oc;
   GLshort vertex[31*4];
   GLfloat colours[31*4];

   /* position */
   x = w->x;
   y = w->y;

   /* colours */
   lc = &cGrey90;
   c = &cGrey70;
   dc = &cGrey50;
   oc = &cGrey30;

   /*
    * Case fullscreen.
    */
   if (window_isFlag( w, WINDOW_FULLSCREEN )) {
      /* Background. */
      toolkit_drawRect( x, y,          w->w, 0.6*w->h, dc, c );
      toolkit_drawRect( x, y+0.6*w->h, w->w, 0.4*w->h, c, NULL );
      /* Name. */
      gl_printMidRaw( &gl_defFont, w->w,
            x,
            y + w->h - 20.,
            &cBlack, w->name );
      return;
   }

   /*
    * window shaded bg
    */
   /* main body */
   toolkit_drawRect( x+21, y,          w->w-42., 0.6*w->h, dc, c );
   toolkit_drawRect( x+21, y+0.6*w->h, w->w-42., 0.4*w->h, c, NULL );

   glShadeModel(GL_SMOOTH);
   /* Both sides. */
   gl_vboActivateOffset( toolkit_vbo, GL_COLOR_ARRAY,
         toolkit_vboColourOffset, 4, GL_FLOAT, 0 );
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY, 0, 2, GL_SHORT, 0 );
   /* Colour is shared. */
   colours[0] = c->r;
   colours[1] = c->g;
   colours[2] = c->b;
   colours[3] = c->a;
   for (i=0; i<7; i++) {
      colours[4 + 4*i + 0] = dc->r;
      colours[4 + 4*i + 1] = dc->g;
      colours[4 + 4*i + 2] = dc->b;
      colours[4 + 4*i + 3] = dc->a;
   }
   for (i=0; i<8; i++) {
      colours[32 + 4*i + 0] = c->r;
      colours[32 + 4*i + 1] = c->g;
      colours[32 + 4*i + 2] = c->b;
      colours[32 + 4*i + 3] = c->a;
   }
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset,
         sizeof(GLfloat) * 4*16, colours );
   /* Left side vertex. */
   cx = x;
   cy = y;
   vertex[0]  = cx + 21;
   vertex[1]  = cy + 0.6*w->h;
   vertex[2]  = cx + 21;
   vertex[3]  = cy;
   vertex[4]  = cx + 15;
   vertex[5]  = cy + 1;
   vertex[6]  = cx + 10;
   vertex[7]  = cy + 3;
   vertex[8]  = cx + 6;
   vertex[9]  = cy + 6;
   vertex[10] = cx + 3;
   vertex[11] = cy + 10;
   vertex[12] = cx + 1;
   vertex[13] = cy + 15;
   vertex[14] = cx;
   vertex[15] = cy + 21;
   vertex[16] = cx;
   vertex[17] = cy + 0.6*w->h;
   vertex[18] = cx;
   cy = y + w->h;
   vertex[19] = cy - 21;
   vertex[20] = cx + 1;
   vertex[21] = cy - 15;
   vertex[22] = cx + 3;
   vertex[23] = cy - 10;
   vertex[24] = cx + 6;
   vertex[25] = cy - 6;
   vertex[26] = cx + 10;
   vertex[27] = cy - 3;
   vertex[28] = cx + 15;
   vertex[29] = cy - 1;
   vertex[30] = cx + 21;
   vertex[31] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLshort) * 2*16, vertex );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 16 );
   /* Right side vertex. */
   cx = x + w->w;
   cy = y;
   vertex[0]  = cx - 21;
   vertex[1]  = cy + 0.6*w->h;
   vertex[2]  = cx - 21;
   vertex[3]  = cy;
   vertex[4]  = cx - 15;
   vertex[5]  = cy + 1;
   vertex[6]  = cx - 10;
   vertex[7]  = cy + 3;
   vertex[8]  = cx - 6;
   vertex[9]  = cy + 6;
   vertex[10] = cx - 3;
   vertex[11] = cy + 10;
   vertex[12] = cx - 1;
   vertex[13] = cy + 15;
   vertex[14] = cx;
   vertex[15] = cy + 21;
   vertex[16] = cx;
   vertex[17] = cy + 0.6*w->h;
   vertex[18] = cx;
   cy = y + w->h;
   vertex[19] = cy - 21;
   vertex[20] = cx - 1;
   vertex[21] = cy - 15;
   vertex[22] = cx - 3;
   vertex[23] = cy - 10;
   vertex[24] = cx - 6;
   vertex[25] = cy - 6;
   vertex[26] = cx - 10;
   vertex[27] = cy - 3;
   vertex[28] = cx - 15;
   vertex[29] = cy - 1;
   vertex[30] = cx - 21;
   vertex[31] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLshort) * 2*16, vertex );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 16 );


   /*
    * inner outline
    */
   /* Colour. */
   for (i=0; i<7; i++) {
      colours[4*i + 0] = c->r;
      colours[4*i + 1] = c->g;
      colours[4*i + 2] = c->b;
      colours[4*i + 3] = c->a;
   }
   for (; i<7+16; i++) {
      colours[4*i + 0] = lc->r;
      colours[4*i + 1] = lc->g;
      colours[4*i + 2] = lc->b;
      colours[4*i + 3] = lc->a;
   }
   for (; i<7+16+8; i++) {
      colours[4*i + 0] = c->r;
      colours[4*i + 1] = c->g;
      colours[4*i + 2] = c->b;
      colours[4*i + 3] = c->a;
   }
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset,
         sizeof(GLfloat) * 4*31, colours );
   /* Vertex. */
   /* Left side. */
   cx = x + 1;
   cy = y + 1;
   vertex[0]  = cx + 21;
   vertex[1]  = cy;
   vertex[2]  = cx + 15;
   vertex[3]  = cy + 1;
   vertex[4]  = cx + 10;
   vertex[5]  = cy + 3;
   vertex[6]  = cx + 6;
   vertex[7]  = cy + 6;
   vertex[8]  = cx + 3;
   vertex[9]  = cy + 10;
   vertex[10] = cx + 1;
   vertex[11] = cy + 15;
   vertex[12] = cx;
   vertex[13] = cy + 21;
   vertex[14] = cx;
   vertex[15] = cy + 0.6*w->h - 1;
   cy = y + w->h - 1;
   vertex[16] = cx;
   vertex[17] = cy - 21;
   vertex[18] = cx + 1;
   vertex[19] = cy - 15;
   vertex[20] = cx + 3;
   vertex[21] = cy - 10;
   vertex[22] = cx + 6;
   vertex[23] = cy - 6;
   vertex[24] = cx + 10;
   vertex[25] = cy - 3;
   vertex[26] = cx + 15;
   vertex[27] = cy - 1;
   vertex[28] = cx + 21;
   vertex[29] = cy;
   /* Right side via top. */
   cx = x + w->w - 1;
   cy = y + w->h - 1;
   vertex[30] = cx - 21;
   vertex[31] = cy;
   vertex[32] = cx - 15;
   vertex[33] = cy - 1;
   vertex[34] = cx - 10;
   vertex[35] = cy - 3;
   vertex[36] = cx - 6;
   vertex[37] = cy - 6;
   vertex[38] = cx - 3;
   vertex[39] = cy - 10;
   vertex[40] = cx - 1;
   vertex[41] = cy - 15;
   vertex[42] = cx;
   vertex[43] = cy - 21;
   cy = y + 1;
   vertex[44] = cx;
   vertex[45] = cy + 0.6*w->h - 1;
   vertex[46] = cx;
   vertex[47] = cy + 21;
   vertex[48] = cx - 1;
   vertex[49] = cy + 15;
   vertex[50] = cx - 3;
   vertex[51] = cy + 10;
   vertex[52] = cx - 6;
   vertex[53] = cy + 6;
   vertex[54] = cx - 10;
   vertex[55] = cy + 3;
   vertex[56] = cx - 15;
   vertex[57] = cy + 1;
   vertex[58] = cx - 21;
   vertex[59] = cy;
   cx = x + 1;
   cy = y + 1;
   vertex[60] = cx + 21;
   vertex[61] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLshort) * 2*31, vertex );
   glDrawArrays( GL_LINE_LOOP, 0, 31 );


   /*
    * outter outline
    */
   glShadeModel(GL_FLAT);
   /* Colour. */
   for (i=0; i<31; i++) {
      colours[4*i + 0] = oc->r;
      colours[4*i + 1] = oc->g;
      colours[4*i + 2] = oc->b;
      colours[4*i + 3] = oc->a;
   }
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset,
         sizeof(GLfloat) * 4*31, colours );
   /* Vertex. */
   /* Left side. */
   cx = x;
   cy = y;
   vertex[0]  = cx + 21;
   vertex[1]  = cy;
   vertex[2]  = cx + 15;
   vertex[3]  = cy + 1;
   vertex[4]  = cx + 10;
   vertex[5]  = cy + 3;
   vertex[6]  = cx + 6;
   vertex[7]  = cy + 6;
   vertex[8]  = cx + 3;
   vertex[9]  = cy + 10;
   vertex[10] = cx + 1;
   vertex[11] = cy + 15;
   vertex[12] = cx;
   vertex[13] = cy + 21;
   vertex[14] = cx;
   vertex[15] = cy + 0.6*w->h;
   cy = y + w->h;
   vertex[16] = cx;
   vertex[17] = cy - 21;
   vertex[18] = cx + 1;
   vertex[19] = cy - 15;
   vertex[20] = cx + 3;
   vertex[21] = cy - 10;
   vertex[22] = cx + 6;
   vertex[23] = cy - 6;
   vertex[24] = cx + 10;
   vertex[25] = cy - 3;
   vertex[26] = cx + 15;
   vertex[27] = cy - 1;
   vertex[28] = cx + 21;
   vertex[29] = cy;
   /* Right side via top. */
   cx = x + w->w;
   cy = y + w->h;
   vertex[30] = cx - 21;
   vertex[31] = cy;
   vertex[32] = cx - 15;
   vertex[33] = cy - 1;
   vertex[34] = cx - 10;
   vertex[35] = cy - 3;
   vertex[36] = cx - 6;
   vertex[37] = cy - 6;
   vertex[38] = cx - 3;
   vertex[39] = cy - 10;
   vertex[40] = cx - 1;
   vertex[41] = cy - 15;
   vertex[42] = cx;
   vertex[43] = cy - 21;
   cy = y;
   vertex[44] = cx;
   vertex[45] = cy + 0.6*w->h;
   vertex[46] = cx;
   vertex[47] = cy + 21;
   vertex[48] = cx - 1;
   vertex[49] = cy + 15;
   vertex[50] = cx - 3;
   vertex[51] = cy + 10;
   vertex[52] = cx - 6;
   vertex[53] = cy + 6;
   vertex[54] = cx - 10;
   vertex[55] = cy + 3;
   vertex[56] = cx - 15;
   vertex[57] = cy + 1;
   vertex[58] = cx - 21;
   vertex[59] = cy;
   cx = x;
   cy = y;
   vertex[60] = cx + 21;
   vertex[61] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLshort) * 2*31, vertex );
   glDrawArrays( GL_LINE_LOOP, 0, 31 );

   /* Clean up. */
   gl_vboDeactivate();

   /*
    * render window name
    */
   gl_printMidRaw( &gl_defFont, w->w,
         x,
         y + w->h - 20.,
         &cBlack, w->name );
}


/**
 * @brief Renders a window.
 *
 *    @param w Window to render.
 */
void window_render( Window *w )
{
   double x, y, wid, hei;
   Widget *wgt;

   /* Do not render dead windows. */
   if (window_isFlag( w, WINDOW_KILL ))
      return;

   /* position */
   x = w->x;
   y = w->y;

   /* See if needs border. */
   if (!window_isFlag( w, WINDOW_NOBORDER ))
      window_renderBorder(w);

   /*
    * widgets
    */
   for (wgt=w->widgets; wgt!=NULL; wgt=wgt->next)
      if (wgt->render != NULL)
         wgt->render( wgt, x, y );

   /*
    * focused widget
    */
   if (w->focus != -1) {
      wgt = toolkit_getFocus( w );
      if (wgt == NULL)
         return;
      x  += wgt->x;
      y  += wgt->y;
      wid = wgt->w;
      hei = wgt->h;
      toolkit_drawOutline( x, y, wid, hei, 3, &cBlack, NULL );
   }
}


/**
 * @brief Renders the window overlays.
 *
 *    @param w Window to render overlays of.
 */
void window_renderOverlay( Window *w )
{
   double x, y;
   Widget *wgt;

   /* Do not render dead windows. */
   if (window_isFlag( w, WINDOW_KILL ))
      return;

   /* position */
   x = w->x;
   y = w->y;

   /*
    * overlays
    */
   for (wgt=w->widgets; wgt!=NULL; wgt=wgt->next)
      if (wgt->renderOverlay != NULL)
         wgt->renderOverlay( wgt, x, y );
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
   toolkit_drawRect( x, y, w, h, toolkit_colDark, toolkit_col );
   /* toolkit_drawOutline( x, y, w, h,  0., toolkit_colDark, NULL ); */
   /* toolkit_drawOutline( x, y, w, h, 0., toolkit_colLight, toolkit_col ); */

   /* Bar itself. */
   sy = y + (h - 30.) * (1.-pos);
   toolkit_drawRect( x, sy, w, 30., toolkit_colLight, toolkit_col );
   toolkit_drawOutline( x + 1, sy, w - 1, 30., 0., toolkit_colDark, NULL );
}


/**
 * @brief Renders the windows.
 */
void toolkit_render (void)
{
   Window *w;

   /* Render base. */
   for (w = windows; w!=NULL; w = w->next) {
      if (!window_isFlag(w, WINDOW_NORENDER) &&
               !window_isFlag(w, WINDOW_KILL)) {
         window_render(w);
         window_renderOverlay(w);
      }
   }
}


/**
 * @brief Toolkit input handled here.
 *
 *    @param event Event to handle.
 *    @return 1 if input was used, 0 if it wasn't.
 */
int toolkit_input( SDL_Event* event )
{
   int ret;
   Window *wdw, *wlast;
   Widget *wgt;

   /* Get window that can be focused. */
   wlast = NULL;
   for (wdw = windows; wdw!=NULL; wdw = wdw->next) {
      if (!window_isFlag( wdw, WINDOW_NOINPUT ) &&
            !window_isFlag( wdw, WINDOW_KILL ))
         wlast = wdw;
   }
   if (wlast == NULL)
      return 0;
   wdw = wlast;

   /* See if widget needs event. */
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next) {
      if (wgt_isFlag( wgt, WGT_FLAG_RAWINPUT )) {
         if (wgt->rawevent != NULL) {
            ret = wgt->rawevent( wgt, event );
            if (ret != 0)
               return ret;
         }
      }
   }

   /* Pass event to window. */
   return toolkit_inputWindow( wdw, event, 1 );
}


/**
 * @brief Toolkit window input is handled here.
 */
int toolkit_inputWindow( Window *wdw, SDL_Event *event, int purge )
{
   int ret;
   ret = 0;

   /* Event handler. */
   if (wdw->eventevent != NULL)
      wdw->eventevent( wdw->id, event );

   /* Hack in case window got destroyed in eventevent. */
   if (!window_isFlag(wdw, WINDOW_KILL)) {

      /* Pass it on. */
      switch (event->type) {
         case SDL_MOUSEMOTION:
         case SDL_MOUSEBUTTONDOWN:
         case SDL_MOUSEBUTTONUP:
            toolkit_mouseEvent(wdw, event);
            ret = 1;
            break;

         case SDL_KEYDOWN:
         case SDL_KEYUP:
            ret =  toolkit_keyEvent(wdw, event);
            break;
      }
   }

   /* Clean up the dead if needed. */
   if (purge && !dialogue_isOpen()) { /* Hack, since dialogues use secondary loop. */
      if (toolkit_delayCounter > 0)
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
Uint32 toolkit_inputTranslateCoords( Window *w, SDL_Event *event,
      int *x, int *y, int *rx, int *ry )
{
   /* Extract the position as event. */
   if (event->type==SDL_MOUSEMOTION) {
      *x = event->motion.x;
      *y = event->motion.y;
   }
   else if ((event->type==SDL_MOUSEBUTTONDOWN) || (event->type==SDL_MOUSEBUTTONUP)) {
      *x = event->button.x;
      *y = event->button.y;
   }

   /* Translate offset. */
   gl_windowToScreenPos( x, y, *x, *y );

   /* Transform to relative to window. */
   *x -= w->x;
   *y -= w->y;

   /* Relative only matter if mouse motion. */
   if (event->type==SDL_MOUSEMOTION) {
      *ry = (double)event->motion.yrel * gl_screen.mxscale;
      *rx = (double)event->motion.xrel * gl_screen.myscale;
   }
   else {
      *ry = 0;
      *rx = 0;
   }

   return event->type;
}


/**
 * @brief Handles the mouse events.
 *
 *    @param wdw Window recieving the mouse event.
 *    @param event Mouse event to handle.
 */
static void toolkit_mouseEvent( Window *w, SDL_Event* event )
{
   Widget *wgt;
   Uint32 type;
   Uint8 button;
   int x, y, rx, ry;

   /* Translate mouse coords. */
   type = toolkit_inputTranslateCoords( w, event, &x, &y, &rx, &ry );

   /* Check each widget. */
   for (wgt=w->widgets; wgt!=NULL; wgt=wgt->next) {

      /* custom widgets take it from here */
      if (wgt->type==WIDGET_CUST) {
         if (wgt->dat.cst.mouse)
            wgt->dat.cst.mouse( w->id, event, x-wgt->x, y-wgt->y, wgt->w, wgt->h,
                  wgt->dat.cst.userdata );
      }
      else {
         /* Handle mouse event. */
         if (type == SDL_MOUSEMOTION)
            button = event->motion.state;
         else
            button = event->button.button;
         toolkit_mouseEventWidget( w, wgt, type, button, x, y, rx, ry );
      }
   }
}


/**
 * @brief Handle widget mouse input.
 *
 *    @param w Window to which widget belongs.
 *    @param wgt Widget recieving event.
 *    @param event Event recieved by the window.
 */
static void toolkit_mouseEventWidget( Window *w, Widget *wgt,
      Uint32 type, Uint8 button, int x, int y, int rx, int ry )
{
   int inbounds;

   /* Widget translations. */
   x -= wgt->x;
   y -= wgt->y;

   /* Check inbounds. */
   inbounds = !((x < 0) || (x >= wgt->w) || (y < 0) || (y >= wgt->h));

   /* Regular widgets. */
   switch (type) {
      case SDL_MOUSEMOTION:
         /* Change the status of the widget if mouse isn't down. */

         /* Not scrolling. */
         if (wgt->status != WIDGET_STATUS_SCROLLING) {
            if (inbounds) {
               if (wgt->status != WIDGET_STATUS_MOUSEDOWN)
                  wgt->status = WIDGET_STATUS_MOUSEOVER;
            }
            else
               wgt->status = WIDGET_STATUS_NORMAL;
         }
         else
            inbounds = 1; /* Scrolling is always inbounds. */

         /* If always gets the event. */
         if (wgt_isFlag( wgt, WGT_FLAG_ALWAYSMMOVE ))
            inbounds = 1;

         /* Try to give the event to the widget. */
         if (inbounds && (wgt->mmoveevent != NULL))
            (*wgt->mmoveevent)( wgt, x, y, rx, ry );

         break;

      case SDL_MOUSEBUTTONDOWN:
         if (!inbounds)
            break;

         /* Update the status. */
         if (button == SDL_BUTTON_LEFT)
            wgt->status = WIDGET_STATUS_MOUSEDOWN;

         if (toolkit_isFocusable(wgt)) {
            toolkit_focusClear( w );
            w->focus = wgt->id;
            wgt_setFlag( wgt, WGT_FLAG_FOCUSED );
         }

         /* Try to give the event to the widget. */
         if (wgt->mclickevent != NULL)
            (*wgt->mclickevent)( wgt, button, x, y );
         break;

      case SDL_MOUSEBUTTONUP:
         /* Since basically only buttons are handled here, we ignore
          * it all except the left mouse button. */
         if (button != SDL_BUTTON_LEFT)
            break;

         if (wgt->status==WIDGET_STATUS_MOUSEDOWN) {
            if ((wgt->type==WIDGET_BUTTON) &&
                  (wgt->dat.btn.disabled==0)) {
               if (wgt->dat.btn.fptr==NULL)
                  DEBUG("Toolkit: Button '%s' of Window '%s' "
                        "doesn't have a function trigger",
                        wgt->name, w->name );
               else {
                  (*wgt->dat.btn.fptr)(w->id, wgt->name);
               }
            }
         }

         /* Always goes normal unless is below mouse. */
         if (inbounds)
            wgt->status = WIDGET_STATUS_MOUSEOVER;
         else
            wgt->status = WIDGET_STATUS_NORMAL;

         break;
   }
}


/**
 * @brief Registers a key as down (for key repetition).
 *
 *    @param key Key to register as down.
 */
static void toolkit_regKey( SDLKey key, SDLKey c )
{
   if ((input_key==0) && (input_keyTime==0)) {
      input_key         = key;
      input_keyTime     = SDL_GetTicks();
      input_keyCounter  = 0;
      input_text        = nstd_checkascii(c) ? c : 0;
   }
}
/**
 * @brief Unregisters a key.
 *
 *    @param key Key to unregister.
 */
static void toolkit_unregKey( SDLKey key )
{
   if (input_key == key)
      toolkit_clearKey();
}
/**
 * @brief Clears the registered keys.
 */
void toolkit_clearKey (void)
{
   input_key         = 0;
   input_keyTime     = 0;
   input_keyCounter  = 0;
   input_text        = 0;
}
/**
 * @brief Handles keyboard events.
 *
 *    @param wdw Window recieving the key event.
 *    @param event Keyboard event to handle.
 *    @return 1 if the event is used, 0 if it isn't.
 */
static int toolkit_keyEvent( Window *wdw, SDL_Event* event )
{
   Widget *wgt;
   SDLKey key;
   SDLMod mod;
   char buf[2];

   /* Event info. */
   key = event->key.keysym.sym;
   mod = event->key.keysym.mod;

   /* Hack to simulate key repetition */
   if (event->type == SDL_KEYDOWN)
      toolkit_regKey(key, event->key.keysym.unicode);
   else if (event->type == SDL_KEYUP)
      toolkit_unregKey(key);

   /* See if window is valid. */
   if (wdw == NULL)
      return 0;

   /* Get widget. */
   wgt = toolkit_getFocus( wdw );

   /* We only want keydown from now on. */
   if (event->type != SDL_KEYDOWN)
      return 0;

   /* Trigger event function if exists. */
   if (wgt != NULL) {
      if (wgt->keyevent != NULL) {
         if (wgt->keyevent( wgt, key, mod ))
            return 1;
      }
      if (wgt->textevent != NULL) {
         buf[0] = event->key.keysym.unicode & 0x7f;
         buf[1] = '\0';
         if ((*wgt->textevent)( wgt, buf ))
            return 1;
      }
   }

   /* Handle other cases where event might be used by the window. */
   switch (key) {
      case SDLK_TAB:
         if (mod & (KMOD_LSHIFT | KMOD_RSHIFT))
            toolkit_prevFocus( wdw );
         else
            toolkit_nextFocus( wdw );
         break;

      case SDLK_RETURN:
      case SDLK_KP_ENTER:
         if (wdw->accept_fptr != NULL) {
            wdw->accept_fptr( wdw->id, wdw->name );
            return 1;
         }
         break;

      case SDLK_ESCAPE:
         if (wdw->cancel_fptr != NULL) {
            wdw->cancel_fptr( wdw->id, wdw->name );
            return 1;
         }
         break;

      default:
         break;
   }

   /* Finally the stuff gets passed to the custom key handler if it's defined. */
   if (wdw->keyevent != NULL)
      (*wdw->keyevent)( wdw->id, key, mod );

   return 0;
}


/**
 * @brief Purges the dead windows.
 */
static void toolkit_purgeDead (void)
{
   Window *wdw, *wlast, *wkill;
   Widget *wgt, *wgtlast, *wgtkill;

   /* Only clean up if necessary. */
   if (!window_dead)
      return;

   /* Must be windows. */
   if (windows == NULL)
      return;

   /* Destroy what is needed. */
   wlast = NULL;
   wdw   = windows;
   while (wdw != NULL) {
      if (window_isFlag( wdw, WINDOW_KILL )) {
         /* Save target. */
         wkill = wdw;
         /* Reattach linked list. */
         if (wlast == NULL)
            windows = wdw->next;
         else
            wlast->next = wdw->next;
         wdw = wlast;
         /* Kill target. */
         wkill->next = NULL;
         window_kill( wkill );
      }
      else {
         wgtlast = NULL;
         wgt     = wdw->widgets;
         while (wgt != NULL) {
            if (wgt_isFlag( wgt, WGT_FLAG_KILL )) {
               /* Save target. */
               wgtkill = wgt;
               /* Reattach linked list. */
               if (wgtlast == NULL)
                  wdw->widgets  = wgt->next;
               else
                  wgtlast->next = wgt->next;
               wgt = wgtlast;
               /* Kill target. */
               wgtkill->next = NULL;
               widget_kill( wgtkill );
            }
            /* Save position. */
            wgtlast = wgt;
            if (wgt == NULL)
               wgt = wdw->widgets;
            else
               wgt = wgt->next;
         }
      }
      /* Save position. */
      wlast = wdw;
      if (wdw == NULL)
         wdw = windows;
      else
         wdw = wdw->next;
   }

   /* Nothing left to purge. */
   window_dead = 0;
}


/**
 * @brief Updates the toolkit input for repeating keys.
 */
void toolkit_update (void)
{
   unsigned int t;
   Window *wdw;
   Widget *wgt;
   char buf[2];
   SDL_Event event;
   int ret;

   /* Clean up the dead if needed. */
   if (!dialogue_isOpen()) { /* Hack, since dialogues use secondary loop. */
      if (toolkit_delayCounter > 0)
         toolkit_delayCounter--;
      else
         toolkit_purgeDead();
   }

   /* Killed all the windows. */
   if (windows == NULL) {
      input_mouseHide();
      toolkit_open = 0; /* disable toolkit */
      if (paused)
         unpause_game();
      return; /*  No need to handle anything else. */
   }

   /* Must have a key pressed. */
   if (input_key == 0)
      return;

   t = SDL_GetTicks();

   /* Should be repeating. */
   if (input_keyTime + INPUT_DELAY + input_keyCounter*INPUT_FREQ > t)
      return;

   /* Increment counter. */
   input_keyCounter++;

   /* Check to see what it affects. */
   if (windows != NULL) {
      /* Get the window. */
      wdw = toolkit_getActiveWindow();
      if (wdw == NULL)
         return;


      /* See if widget needs event. */
      for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next) {
         if (wgt_isFlag( wgt, WGT_FLAG_RAWINPUT )) {
            if (wgt->rawevent != NULL) {
               event.type           = SDL_KEYDOWN;
               event.key.state      = SDL_PRESSED;
               event.key.keysym.sym = input_key;
               event.key.keysym.mod = 0;
               ret = wgt->rawevent( wgt, &event );
               if (ret != 0)
                  return;
            }
         }
      }

      /* Handle the focused widget. */
      wgt = toolkit_getFocus( wdw );
      if ((wgt != NULL) && (wgt->keyevent != NULL)) {
         wgt->keyevent( wgt, input_key, 0 );
      }
      if ((input_text != 0) && (wgt != NULL) && (wgt->textevent != NULL)) {
         buf[0] = input_text;
         buf[1] = '\0';
         wgt->textevent( wgt, buf );
      }
   }
}


/**
 * @brief Clears the window focus.
 */
static void toolkit_focusClear( Window *wdw )
{
   Widget *wgt;
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next)
      wgt_rmFlag( wgt, WGT_FLAG_FOCUSED );
}


/**
 * @brief Sanitizes the focus of a window.
 *
 * Makes sure the window has a focusable widget focused.
 */
void toolkit_focusSanitize( Window *wdw )
{
   Widget *wgt;

   /* Clear focus. */
   toolkit_focusClear( wdw );

   /* No focus is always sane. */
   if (wdw->focus == -1)
      return;

   /* Check focused widget. */
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next) {
      if (wdw->focus == wgt->id) {
         /* Not focusable. */
         if (!toolkit_isFocusable(wgt)) {
            wdw->focus = -1;
            toolkit_nextFocus( wdw ); /* Get first focus. */
         }
         else {
            wgt_setFlag( wgt, WGT_FLAG_FOCUSED );
         }
         return;
      }
   }
}


/**
 * @brief Focus next widget.
 */
void toolkit_nextFocus( Window *wdw )
{
   Widget *wgt;
   int next;

   /* Clear focus. */
   toolkit_focusClear( wdw );

   /* See what to focus. */
   next = (wdw->focus == -1);
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next) {
      if (!toolkit_isFocusable(wgt))
         continue;

      if (next) {
         wdw->focus = wgt->id;
         wgt_setFlag( wgt, WGT_FLAG_FOCUSED );
         return;
      }
      else if (wdw->focus == wgt->id)
         next = 1;
   }

   /* Focus nothing. */
   wdw->focus = -1;
   return;
}


/**
 * @brief Focus previous widget.
 */
void toolkit_prevFocus( Window *wdw )
{
   Widget *wgt, *prev;

   /* Clear focus. */
   toolkit_focusClear( wdw );

   /* See what to focus. */
   prev = NULL;
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next) {
      if (!toolkit_isFocusable(wgt))
         continue;

      /* See if we found the current one. */
      if (wdw->focus == wgt->id) {
         if (prev == NULL)
            wdw->focus = -1;
         else {
            wdw->focus = prev->id;
            wgt_setFlag( wgt, WGT_FLAG_FOCUSED );
         }
         return;
      }

      /* Store last focusable widget. */
      prev = wgt;
   }

   /* Focus nothing. */
   if (prev == NULL)
      wdw->focus = -1;
   else {
      wdw->focus = prev->id;
      wgt_setFlag( prev, WGT_FLAG_FOCUSED );
   }
   return;
}


/**
 * @brief Checks to see if a widget is focusable.
 *
 *    @param wgt Widget to check if is focusable.
 *    @return 1 if it's focusable, 0 if it isn't.
 */
static int toolkit_isFocusable( Widget *wgt )
{
   if (wgt==NULL)
      return 0;

   return wgt_isFlag(wgt, WGT_FLAG_CANFOCUS);
}


/**
 * @brief Gets the active window in the toolkit.
 *
 *    @return The active window in the toolkit.
 */
Window* toolkit_getActiveWindow (void)
{
   Window *wdw, *wlast;

   /* Get window that can be focused. */
   wlast = NULL;
   for (wdw = windows; wdw!=NULL; wdw = wdw->next)
      if (!window_isFlag(wdw, WINDOW_NOFOCUS) &&
            !window_isFlag(wdw, WINDOW_KILL))
         wlast = wdw;
   return wlast;
}


/**
 * @brief Gets the focused widget in a window.
 *
 *    @param wdw The window to get the focused widget from.
 *    @return The focused widget.
 */
static Widget* toolkit_getFocus( Window *wdw )
{
   Widget *wgt;

   /* No focus. */
   if (wdw->focus == -1)
      return NULL;

   /* Find focus. */
   for (wgt=wdw->widgets; wgt!=NULL; wgt=wgt->next)
      if (wdw->focus == wgt->id)
         return wgt;

   /* Not found. */
   toolkit_focusClear( wdw );
   wdw->focus = -1;
   return NULL;
}


/**
 * @brief Initializes the toolkit.
 *
 *    @return 0 on success.
 */
int toolkit_init (void)
{
   GLsizei size;

   /* Create the VBO. */
   toolkit_vboColourOffset = sizeof(GLshort) * 2 * 31;
   size = (sizeof(GLshort)*2 + sizeof(GLfloat)*4) * 31;
   toolkit_vbo = gl_vboCreateStream( size, NULL );

   /* DIsable the cursor. */
   input_mouseHide();

   return 0;
}


/**
 * @brief Exits the toolkit.
 */
void toolkit_exit (void)
{
   Window *wdw;

   /* Destroy the windows. */
   while (windows!=NULL) {
      wdw      = windows;
      windows  = windows->next;
      window_kill(wdw);
   }
   free(windows);

   /* Free the VBO. */
   gl_vboDestroy( toolkit_vbo );
   toolkit_vbo = NULL;
}

