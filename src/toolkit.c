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


#define INPUT_DELAY     500 /**< Delay before starting to repeat. */
#define INPUT_FREQ       30 /**< Interval between repetition. */


static unsigned int genwid = 0; /**< Generates unique window ids, > 0 */


static int toolkit_open = 0; /**< 1 if toolkit is in use, 0 else. */

/* 
 * window stuff
 */
#define MIN_WINDOWS  3 /**< Minimum windows to prealloc. */
static Window *windows = NULL; /**< Window stacks, not to be confused with MS windows. */
static int nwindows = 0; /**< Number of windows in the stack. */
static int mwindows = 0; /**< Allocated windows in the stack. */


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
static int toolkit_keyEvent( Window *wdw, SDL_Event* event );
/* focus */
static int toolkit_isFocusable( Widget *wgt );
static Widget* toolkit_getFocus( Window *wdw );


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
Widget* window_newWidget( Window* w )
{
   Widget* wgt = NULL;

   /* Grow widget list. */
   w->nwidgets++;
   w->widgets = realloc( w->widgets,
         sizeof(Widget)*w->nwidgets );
   if (w->widgets == NULL)
      WARN("Out of Memory");

   /* Set sane defaults. */
   wgt = &w->widgets[ w->nwidgets - 1 ]; 
   memset( wgt, 0, sizeof(Widget) );
   wgt->type = WIDGET_NULL;
   wgt->status = WIDGET_STATUS_NORMAL;

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
   int i;
   for (i=0; i<nwindows; i++)
      if (windows[i].id == wid)
         return &windows[i];
   DEBUG("Window '%d' not found in windows stack", wid);
   return NULL;
}


/**
 * @brief Gets a widget from window id and widgetname.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to get.
 *    @return Widget matching name in the window.
 */
Widget* window_getwgt( const unsigned int wid, char* name )
{
   int i;
   Window *wdw;

   /* Get the window. */
   wdw = window_wget(wid);
   if (wdw == NULL)
      return NULL;

   /* Find the widget. */
   for (i=0; i<wdw->nwidgets; i++)
      if (strcmp(wdw->widgets[i].name, name)==0)
         return &wdw->widgets[i];

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
   int i;
   for (i=0; i<nwindows; i++)
      if (strcmp(windows[i].name,wdwname)==0)
         return 1; /* exists */
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
   int i;
   for (i=0; i<nwindows; i++)
      if (strcmp(windows[i].name,wdwname)==0)
         return windows[i].id;
   return 0;
}


/**
 * @brief Creates a window.
 *
 *    @param name Name of the window to create.
 *    @param x X position of the window (-1 centers).
 *    @param y Y position of the window (-1 centers).
 *    @param w Width of the window.
 *    @param h Height of the window.
 *    @return Newly created window's ID.
 */
unsigned int window_create( const char* name,
      const int x, const int y, const int w, const int h )
{
   Window *wdw;

   if (nwindows >= mwindows) { /* at memory limit */
      windows = realloc(windows, sizeof(Window)*(++mwindows));
      if (windows==NULL) WARN("Out of memory");
   }

   const int wid = (++genwid); /* unique id */

   /* Create the window. */
   wdw = &windows[nwindows];
   memset( wdw, 0, sizeof(Window) );

   wdw->id           = wid;
   wdw->name         = strdup(name);

   /* Sane defaults. */
   wdw->focus        = -1;

   /* Dimensions. */
   wdw->w            = (double) w;
   wdw->h            = (double) h;
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

   nwindows++;
   
   if (toolkit_open==0) { /* toolkit is on */
      SDL_ShowCursor(SDL_ENABLE);
      toolkit_open = 1; /* enable toolkit */
      pause_game();
      gl_defViewport(); /* Reset the default viewport */
   }

   /* Clear key repeat. */
   toolkit_clearKey();

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
 * @brief Destroys a widget.
 *
 *    @param widget Widget to destroy.
 */
void widget_cleanup( Widget *widget )
{
   /* General freeing. */
   if (widget->name)
      free(widget->name);

   /* Type specific clean up. */
   if (widget->cleanup != NULL)
      (*widget->cleanup)(widget);
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
 * @brief Destroys a window.
 *
 *    @param wid ID of window to destroy.
 */
void window_destroy( const unsigned int wid )
{
   int i,j;

   /* Destroy children first. */
   for (i=0; i<nwindows; i++) {
      if (windows[i].parent == wid) {
         window_destroy( windows[i].id );
         i = -1; /* Ugly hack in case a window gets destroyed midway. */
      }
   }

   /* Destroy the window */
   for (i=0; i<nwindows; i++)
      if (windows[i].id == wid) {

         /* Run the close function first. */
         if (windows[i].close_fptr != NULL)
            (*windows[i].close_fptr) (windows[i].id, windows[i].name);

         /* Destroy the window. */
         if (windows[i].name)
            free(windows[i].name);
         for (j=0; j<windows[i].nwidgets; j++)
            widget_cleanup(&windows[i].widgets[j]);
         free(windows[i].widgets);
         break;
      }

   if (i==nwindows) { /* Not found */
      WARN("Window of id '%u' not found in window stack!", wid);
      return;
   }

   /* move other windows down a layer */
   for ( ; i<(nwindows-1); i++)
      windows[i] = windows[i+1];

   nwindows--;
   if (nwindows==0) { /* no windows left */
      SDL_ShowCursor(SDL_DISABLE);
      toolkit_open = 0; /* disable toolkit */
      if (paused) unpause_game();
   }

   /* Clear key repeat, since toolkit could miss the keyup event. */
   toolkit_clearKey();
}


/**
 * @brief Checks to see if a widget exists.
 *
 *    @param wid Window to check widget in.
 *    @param wgtname Name of the widget to check;
 */
int widget_exists( const unsigned int wid, const char* wgtname )
{
   Window *w = window_wget(wid);
   int i;

   /* Get window. */
   if (w==NULL) {
      WARN("window '%d' does not exist", wid);
      return -1;
   }

   /* Check for widget. */
   for (i=0; i<w->nwidgets; i++)
      if (strcmp(wgtname,w->widgets[i].name)==0)
         return 1;

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
   Window *w = window_wget(wid);
   int i;

   if (w==NULL) {
      WARN("window '%d' does not exist", wid);
      return;
   }

   for (i=0; i<w->nwidgets; i++)
      if (strcmp(wgtname,w->widgets[i].name)==0)
         break;
   if (i >= w->nwidgets) {
      DEBUG("widget '%s' not found in window %d", wgtname, wid);
      return;
   }
 
   if (w->focus == i) w->focus = -1;
   widget_cleanup(&w->widgets[i]);
   if (i < w->nwidgets-1) /* not last widget */
      memmove(&w->widgets[i], &w->widgets[i+1],
            sizeof(Widget) * (w->nwidgets-i-1) );
   (w->nwidgets)--;
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
void toolkit_drawOutline( double x, double y, 
      double w, double h, double b,
      glColour* c, glColour* lc )
{
   int i;
   static GLfloat lines[9*2], colours[9*4];

   /* Set shade model. */
   glShadeModel( (lc==NULL) ? GL_FLAT : GL_SMOOTH );

   /* Lines. */
   /* x */
   lines[4]  = x;
   lines[0]  = lines[4] - b;
   lines[2]  = lines[0];
   lines[6]  = lines[4] + w;
   lines[8]  = lines[6] + b;
   lines[10] = lines[8];
   lines[12] = lines[6];
   lines[14] = lines[4];
   lines[16] = lines[0];
   /* y */
   lines[1]  = y;
   lines[3]  = lines[1] + h;
   lines[5]  = lines[3] + b;
   lines[7]  = lines[5];
   lines[9]  = lines[3];
   lines[11] = lines[1];
   lines[13] = lines[1] - b;
   lines[15] = lines[13];
   lines[17] = lines[1];

   /* Colours. */
   if (!lc) {
      for (i=0; i<9; i++) {
         colours[4*i + 0] = c->r;
         colours[4*i + 1] = c->g;
         colours[4*i + 2] = c->b;
         colours[4*i + 3] = c->a;
      }
   }
   else {
      colours[0] = lc->r;
      colours[1] = lc->g;
      colours[2] = lc->b;
      colours[3] = lc->a;
      for (i=0; i<4; i++) {
         colours[4 + 4*i + 0] = c->r;
         colours[4 + 4*i + 1] = c->g;
         colours[4 + 4*i + 2] = c->b;
         colours[4 + 4*i + 3] = c->a;
      }
      for (i=0; i<4; i++) {
         colours[20 + 4*i + 0] = lc->r;
         colours[20 + 4*i + 1] = lc->g;
         colours[20 + 4*i + 2] = lc->b;
         colours[20 + 4*i + 3] = lc->a;
      }
   }

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLfloat) * 2*9, lines );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof(GLfloat) * 4*9, colours );

   /* Set up the VBO. */
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( toolkit_vbo, GL_COLOR_ARRAY,
         toolkit_vboColourOffset, 4, GL_FLOAT, 0 );

   /* Draw the VBO. */
   glDrawArrays( GL_LINE_STRIP, 0, 9 );

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
void toolkit_drawRect( double x, double y,
      double w, double h, glColour* c, glColour* lc )
{
   GLfloat vertex[2*4], colours[4*4];

   /* Set shade model. */
   glShadeModel( (lc) ? GL_SMOOTH : GL_FLAT );

   /* Set up the vertex. */
   vertex[0] = x;
   vertex[1] = y;
   vertex[2] = vertex[0];
   vertex[3] = vertex[1] + h;
   vertex[4] = vertex[0] + w;
   vertex[5] = vertex[1];
   vertex[6] = vertex[4];
   vertex[7] = vertex[3];

   /* Set up the colours. */
   if (lc == NULL)
      lc = c;
   colours[0]  = c->r;
   colours[1]  = c->g;
   colours[2]  = c->b;
   colours[3]  = c->a;
   colours[4]  = lc->r;
   colours[5]  = lc->g;
   colours[6]  = lc->b;
   colours[7]  = lc->a;
   colours[8]  = c->r;
   colours[9]  = c->g;
   colours[10] = c->b;
   colours[11] = c->a;
   colours[12] = lc->r;
   colours[13] = lc->g;
   colours[14] = lc->b;
   colours[15] = lc->a;

   /* Upload to the VBO. */
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLfloat) * 2*4, vertex );
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset, sizeof(GLfloat) * 4*4, colours );

   /* Set up the VBO. */
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( toolkit_vbo, GL_COLOR_ARRAY,
         toolkit_vboColourOffset, 4, GL_FLOAT, 0 );

   /* Draw the VBO. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Deactivate VBO. */
   gl_vboDeactivate();
}


/**
 * @brief Sets up 2d clipping planes around a rectangle.
 *
 *    @param x X position of the rectangle.
 *    @param y Y position of the rectangle.
 *    @param w Width of the rectangle.
 *    @param h Height of the rectangle.
 */
void toolkit_clip( double x, double y, double w, double h )
{
   glScissor( x + SCREEN_W/2, y + SCREEN_H/2, w, h );
   glEnable( GL_SCISSOR_TEST );
}
/**
 * @brief Clears the 2d clipping planes.
 */
void toolkit_unclip (void)
{
   glDisable( GL_SCISSOR_TEST );
   glScissor( 0, 0, gl_screen.rw, gl_screen.rh );
}


/**
 * @brief Renders a window.
 *
 *    @param w Window to render.
 */
void window_render( Window* w )
{
   int i;
   GLfloat cx, cy;
   double x, y, wid, hei;
   glColour *lc, *c, *dc, *oc;
   GLfloat vertex[31*4], colours[31*4];

   /* position */
   x = w->x - (double)SCREEN_W/2.;
   y = w->y - (double)SCREEN_H/2.;

   /* colours */
   lc = &cGrey90;
   c = &cGrey70;
   dc = &cGrey50;
   oc = &cGrey30;

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
   gl_vboActivateOffset( toolkit_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   /* Colour is shared. */
   colours[0] = c->r;
   colours[1] = c->g;
   colours[2] = c->r;
   colours[3] = c->a;
   for (i=0; i<7; i++) {
      colours[4 + 4*i + 0] = dc->r;
      colours[4 + 4*i + 1] = dc->g;
      colours[4 + 4*i + 2] = dc->r;
      colours[4 + 4*i + 3] = dc->a;
   }
   for (i=0; i<8; i++) {
      colours[32 + 4*i + 0] = c->r;
      colours[32 + 4*i + 1] = c->g;
      colours[32 + 4*i + 2] = c->r;
      colours[32 + 4*i + 3] = c->a;
   }
   gl_vboSubData( toolkit_vbo, toolkit_vboColourOffset,
         sizeof(GLfloat) * 4*16, colours );
   /* Left side vertex. */
   cx = x;
   cy = y;
   vertex[0]  = cx + 21.;
   vertex[1]  = cy + 0.6*w->h;
   vertex[2]  = cx + 21.;
   vertex[3]  = cy;
   vertex[4]  = cx + 15.;
   vertex[5]  = cy + 1.;
   vertex[6]  = cx + 10.;
   vertex[7]  = cy + 3.;
   vertex[8]  = cx + 6.;
   vertex[9]  = cy + 6.;
   vertex[10] = cx + 3.;
   vertex[11] = cy + 10.;
   vertex[12] = cx + 1.;
   vertex[13] = cy + 15.;
   vertex[14] = cx;
   vertex[15] = cy + 21.;
   vertex[16] = cx;
   vertex[17] = cy + 0.6*w->h;
   vertex[18] = cx;
   cy = y + w->h;
   vertex[19] = cy - 21.;
   vertex[20] = cx + 1.;
   vertex[21] = cy - 15.;
   vertex[22] = cx + 3.;
   vertex[23] = cy - 10.;
   vertex[24] = cx + 6.;
   vertex[25] = cy - 6.;
   vertex[26] = cx + 10.;
   vertex[27] = cy - 3.;
   vertex[28] = cx + 15.;
   vertex[29] = cy - 1.;
   vertex[30] = cx + 21.;
   vertex[31] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLfloat) * 2*16, vertex );
   glDrawArrays( GL_POLYGON, 0, 16 );
   /* Right side vertex. */
   cx = x + w->w;
   cy = y;
   vertex[0]  = cx - 21.;
   vertex[1]  = cy + 0.6*w->h;
   vertex[2]  = cx - 21.;
   vertex[3]  = cy;
   vertex[4]  = cx - 15.;
   vertex[5]  = cy + 1.;
   vertex[6]  = cx - 10.;
   vertex[7]  = cy + 3.;
   vertex[8]  = cx - 6.;
   vertex[9]  = cy + 6.;
   vertex[10] = cx - 3.;
   vertex[11] = cy + 10.;
   vertex[12] = cx - 1.;
   vertex[13] = cy + 15.;
   vertex[14] = cx;
   vertex[15] = cy + 21.;
   vertex[16] = cx;
   vertex[17] = cy + 0.6*w->h;
   vertex[18] = cx;
   cy = y + w->h;
   vertex[19] = cy - 21.;
   vertex[20] = cx - 1.;
   vertex[21] = cy - 15.;
   vertex[22] = cx - 3.;
   vertex[23] = cy - 10.;
   vertex[24] = cx - 6.;
   vertex[25] = cy - 6.;
   vertex[26] = cx - 10.;
   vertex[27] = cy - 3.;
   vertex[28] = cx - 15.;
   vertex[29] = cy - 1.;
   vertex[30] = cx - 21.;
   vertex[31] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLfloat) * 2*16, vertex );
   glDrawArrays( GL_POLYGON, 0, 16 );


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
   cx = x + 1.;
   cy = y + 1.;
   vertex[0]  = cx + 21.;
   vertex[1]  = cy;
   vertex[2]  = cx + 15.;
   vertex[3]  = cy + 1.;
   vertex[4]  = cx + 10.;
   vertex[5]  = cy + 3.;
   vertex[6]  = cx + 6.;
   vertex[7]  = cy + 6.;
   vertex[8]  = cx + 3.;
   vertex[9]  = cy + 10.;
   vertex[10] = cx + 1.;
   vertex[11] = cy + 15.;
   vertex[12] = cx;
   vertex[13] = cy + 21.;
   vertex[14] = cx;
   vertex[15] = cy + 0.6*w->h - 1.;
   cy = y + w->h - 1.;
   vertex[16] = cx;
   vertex[17] = cy - 21.;
   vertex[18] = cx + 1.;
   vertex[19] = cy - 15.;
   vertex[20] = cx + 3.;
   vertex[21] = cy - 10.;
   vertex[22] = cx + 6.;
   vertex[23] = cy - 6.;
   vertex[24] = cx + 10.;
   vertex[25] = cy - 3.;
   vertex[26] = cx + 15.;
   vertex[27] = cy - 1.;
   vertex[28] = cx + 21.;
   vertex[29] = cy;
   /* Right side via top. */
   cx = x + w->w - 1.;
   cy = y + w->h - 1.;
   vertex[30] = cx - 21.;
   vertex[31] = cy;
   vertex[32] = cx - 15.;
   vertex[33] = cy - 1.;
   vertex[34] = cx - 10.;
   vertex[35] = cy - 3.;
   vertex[36] = cx - 6.;
   vertex[37] = cy - 6.;
   vertex[38] = cx - 3.;
   vertex[39] = cy - 10.;
   vertex[40] = cx - 1.;
   vertex[41] = cy - 15.;
   vertex[42] = cx;
   vertex[43] = cy - 21.;
   cy = y + 1.;
   vertex[44] = cx;
   vertex[45] = cy + 0.6*w->h - 1.;
   vertex[46] = cx;
   vertex[47] = cy + 21.;
   vertex[48] = cx - 1.;
   vertex[49] = cy + 15.;
   vertex[50] = cx - 3.;
   vertex[51] = cy + 10.;
   vertex[52] = cx - 6.;
   vertex[53] = cy + 6.;
   vertex[54] = cx - 10.;
   vertex[55] = cy + 3.;
   vertex[56] = cx - 15.;
   vertex[57] = cy + 1.;
   vertex[58] = cx - 21.;
   vertex[59] = cy;
   cx = x + 1.;
   cy = y + 1.;
   vertex[60] = cx + 21.;
   vertex[61] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLfloat) * 2*31, vertex );
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
   vertex[0]  = cx + 21.;
   vertex[1]  = cy;
   vertex[2]  = cx + 15.;
   vertex[3]  = cy + 1.;
   vertex[4]  = cx + 10.;
   vertex[5]  = cy + 3.;
   vertex[6]  = cx + 6.;
   vertex[7]  = cy + 6.;
   vertex[8]  = cx + 3.;
   vertex[9]  = cy + 10.;
   vertex[10] = cx + 1.;
   vertex[11] = cy + 15.;
   vertex[12] = cx;
   vertex[13] = cy + 21.;
   vertex[14] = cx;
   vertex[15] = cy + 0.6*w->h;
   cy = y + w->h;
   vertex[16] = cx;
   vertex[17] = cy - 21.;
   vertex[18] = cx + 1.;
   vertex[19] = cy - 15.;
   vertex[20] = cx + 3.;
   vertex[21] = cy - 10.;
   vertex[22] = cx + 6.;
   vertex[23] = cy - 6.;
   vertex[24] = cx + 10.;
   vertex[25] = cy - 3.;
   vertex[26] = cx + 15.;
   vertex[27] = cy - 1.;
   vertex[28] = cx + 21.;
   vertex[29] = cy;
   /* Right side via top. */
   cx = x + w->w;
   cy = y + w->h;
   vertex[30] = cx - 21.;
   vertex[31] = cy;
   vertex[32] = cx - 15.;
   vertex[33] = cy - 1.;
   vertex[34] = cx - 10.;
   vertex[35] = cy - 3.;
   vertex[36] = cx - 6.;
   vertex[37] = cy - 6.;
   vertex[38] = cx - 3.;
   vertex[39] = cy - 10.;
   vertex[40] = cx - 1.;
   vertex[41] = cy - 15.;
   vertex[42] = cx;
   vertex[43] = cy - 21.;
   cy = y;
   vertex[44] = cx;
   vertex[45] = cy + 0.6*w->h;
   vertex[46] = cx;
   vertex[47] = cy + 21.;
   vertex[48] = cx - 1.;
   vertex[49] = cy + 15.;
   vertex[50] = cx - 3.;
   vertex[51] = cy + 10.;
   vertex[52] = cx - 6.;
   vertex[53] = cy + 6.;
   vertex[54] = cx - 10.;
   vertex[55] = cy + 3.;
   vertex[56] = cx - 15.;
   vertex[57] = cy + 1.;
   vertex[58] = cx - 21.;
   vertex[59] = cy;
   cx = x;
   cy = y;
   vertex[60] = cx + 21.;
   vertex[61] = cy;
   gl_vboSubData( toolkit_vbo, 0, sizeof(GLfloat) * 2*31, vertex );
   glDrawArrays( GL_LINE_LOOP, 0, 31 );

   /* Clean up. */
   gl_vboDeactivate();

   /*
    * render window name
    */
   gl_printMidRaw( &gl_defFont, w->w,
         x + (double)SCREEN_W/2.,
         y + w->h - 20. + (double)SCREEN_H/2.,
         &cBlack, w->name );

   /*
    * widgets
    */
   for (i=0; i<w->nwidgets; i++)
      if (w->widgets[i].render != NULL)
         w->widgets[i].render( &w->widgets[i], x, y );

   /*
    * focused widget
    */
   if (w->focus != -1) {
      x += w->widgets[w->focus].x;
      y += w->widgets[w->focus].y;
      wid = w->widgets[w->focus].w;
      hei = w->widgets[w->focus].h;
      toolkit_drawOutline( x, y, wid, hei, 3, &cBlack, NULL );
   }
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
void toolkit_drawScrollbar( double x, double y, double w, double h, double pos )
{
   double sy;

   /* scrollbar background */
   toolkit_drawRect( x, y, w, h, toolkit_colDark, toolkit_col );
   toolkit_drawOutline( x, y, w, h, 1., toolkit_colLight, toolkit_col );
   toolkit_drawOutline( x, y, w, h, 2., toolkit_colDark, NULL );

   /* Bar itself. */
   sy = y + (h - 30.) * (1.-pos);
   toolkit_drawRect( x, sy, w, 30., toolkit_colLight, toolkit_col );
   toolkit_drawOutline( x, sy, w, 30., 0., toolkit_colDark, NULL );
}


/**
 * @brief Renders the windows.
 */
void toolkit_render (void)
{
   int i;

   for (i=0; i<nwindows; i++)
      if (!window_isFlag(&windows[i], WINDOW_NORENDER))
         window_render(&windows[i]);
}


/**
 * @brief Toolkit input handled here.
 *
 *    @param event Event to handle.
 *    @return 1 if input was used, 0 if it wasn't.
 */
int toolkit_input( SDL_Event* event )
{
   int ret, i;
   Window *wdw;
   Widget *wgt;

   /* Get window that can be focused. */
   for (i=nwindows-1; i>=0; i--)
      if (!window_isFlag(&windows[i], WINDOW_NOINPUT))
         break;
   if (i < 0)
      return 0;
   wdw = &windows[i];

   /* See if widget needs event. */
   if (wdw != NULL) {
      for (i=0; i<wdw->nwidgets; i++) {
         wgt = &wdw->widgets[i];
         if (wgt_isFlag( wgt, WGT_FLAG_RAWINPUT )) {
            if (wgt->rawevent != NULL) {
               ret = wgt->rawevent( wgt, event );
               if (ret != 0)
                  return ret;
            }
         }
      }
   }

   /* Pass event to window. */
   return toolkit_inputWindow( wdw, event );
}


/**
 * @brief Toolkit window input is handled here.
 */
int toolkit_inputWindow( Window *wdw, SDL_Event *event )
{
   switch (event->type) {
      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
         toolkit_mouseEvent(wdw, event);
         return 1; /* block input */

      case SDL_KEYDOWN:
      case SDL_KEYUP:
         return toolkit_keyEvent(wdw, event);

   }
   return 0; /* don't block input */
}


/**
 * @brief Handles the mouse events.
 *
 *    @param wdw Window recieving the mouse event.
 *    @param event Mouse event to handle.
 */
static void toolkit_mouseEvent( Window *w, SDL_Event* event )
{
   int i;
   double x,y;
   Widget *wgt;

   /* Extract the position as event. */
   if (event->type==SDL_MOUSEMOTION) {
      x = (double)event->motion.x;
      y = (double)gl_screen.rh - (double)event->motion.y;
   }
   else if ((event->type==SDL_MOUSEBUTTONDOWN) || (event->type==SDL_MOUSEBUTTONUP)) {
      x = (double)event->button.x;
      y = (double)gl_screen.rh - (double)event->button.y;
   }

   /* Handle possible window scaling. */
   x *= gl_screen.mxscale;
   y *= gl_screen.myscale;

   /* Transform to relative to window. */
   x -= w->x;
   y -= w->y;

   /* Check if inbounds (always handle mouseup). */
   if ((event->type!=SDL_MOUSEBUTTONUP) &&
      ((x < 0.) || (x > w->w) || (y < 0.) || (y > w->h)))
      return; /* not in current window */

   /* Update the event. */
   if (event->type==SDL_MOUSEMOTION) {
      event->motion.x = x;
      event->motion.y = y;
      event->motion.yrel = (double)event->motion.yrel * gl_screen.mxscale;
      event->motion.xrel = (double)event->motion.xrel * gl_screen.myscale;
   }
   else if ((event->type==SDL_MOUSEBUTTONDOWN) || (event->type==SDL_MOUSEBUTTONUP)) {
      event->button.x = x;
      event->button.y = y;
   }

   for (i=0; i<w->nwidgets; i++) {
      wgt = &w->widgets[i];
      /* widget in range? */
      if ((x > wgt->x) && (x < (wgt->x + wgt->w)) &&
            (y > wgt->y) && (y < (wgt->y + wgt->h))) {
         /* custom widgets take it from here */
         if ((wgt->type==WIDGET_CUST) && wgt->dat.cst.mouse) 
            (*wgt->dat.cst.mouse)( w->id, event, x-wgt->x, y-wgt->y, wgt->w, wgt->h );
         else
            switch (event->type) {
               case SDL_MOUSEMOTION:
                  /* Change the status of the widget if mouse isn't down. */
                  if (!(event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)))
                     wgt->status = WIDGET_STATUS_MOUSEOVER;

                  /* Do a coordinate change for the event. */
                  event->motion.x -= wgt->x;
                  event->motion.y -= wgt->y;

                  /* Try to give the event to the widget. */
                  if (wgt->mmoveevent != NULL)
                     if ((*wgt->mmoveevent)( wgt, (SDL_MouseMotionEvent*) event ))
                        return;

                  /* Undo coordinate change. */
                  event->motion.x += wgt->x;
                  event->motion.y += wgt->y;
                  break;

               case SDL_MOUSEBUTTONDOWN:
                  /* Update the status. */
                  if (event->button.button == SDL_BUTTON_LEFT)
                     wgt->status = WIDGET_STATUS_MOUSEDOWN;

                  if (toolkit_isFocusable(wgt))
                     w->focus = i;

                  /* Do a coordinate change for the event. */
                  event->button.x -= wgt->x;
                  event->button.y -= wgt->y;

                  /* Try to give the event to the widget. */
                  if (wgt->mclickevent != NULL)
                     if ((*wgt->mclickevent)( wgt, (SDL_MouseButtonEvent*) event ))
                        return;

                  /* Undo coordinate change. */
                  event->button.x += wgt->x;
                  event->button.y += wgt->y;
                  break;

               case SDL_MOUSEBUTTONUP:
                  /* Since basically only buttons are handled here, we ignore
                   * it all except the left mouse button. */
                  if (event->button.button != SDL_BUTTON_LEFT)
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
                           return;
                        }
                     }
                  }
                  wgt->status = WIDGET_STATUS_NORMAL;
                  break;
            }
      }
      /* otherwise custom widgets can get stuck on mousedown */
      else if ((wgt->type==WIDGET_CUST) &&
            (event->type==SDL_MOUSEBUTTONUP) && wgt->dat.cst.mouse)
         (*wgt->dat.cst.mouse)( w->id, event, x-wgt->x, y-wgt->y, wgt->w, wgt->h );
      else
         wgt->status = WIDGET_STATUS_NORMAL;
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

   /* hack to simulate key repetition */
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
         if ((*wgt->keyevent)( wgt, key, mod ))
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
         toolkit_nextFocus();
         break;

      case SDLK_RETURN:
         if (wdw->accept_fptr != NULL) {
            (*wdw->accept_fptr)( wdw->id, wdw->name );
            return 1;
         }
         break;

      case SDLK_ESCAPE:
         if (wdw->cancel_fptr != NULL) {
            (*wdw->cancel_fptr)( wdw->id, wdw->name );
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
 * @brief Updates the toolkit input for repeating keys.
 */
void toolkit_update (void)
{
   unsigned int t;
   Window *wdw;
   Widget *wgt;
   char buf[2];

   t = SDL_GetTicks();

   /* Must have a key pressed. */
   if (input_key == 0)
      return;

   /* Should be repeating. */
   if (input_keyTime + INPUT_DELAY + input_keyCounter*INPUT_FREQ > t)
      return;

   /* Increment counter. */
   input_keyCounter++;

   /* Check to see what it affects. */
   if (nwindows > 0) {
      wdw = toolkit_getActiveWindow();
      if (wdw == NULL)
         return;
      wgt = toolkit_getFocus( wdw );
      if ((wgt != NULL) && (wgt->keyevent != NULL))
         wgt->keyevent( wgt, input_key, 0 );
      if ((input_text != 0) && (wgt != NULL) && (wgt->textevent != NULL)) {
         buf[0] = input_text;
         buf[1] = '\0';
         wgt->textevent( wgt, buf );
      }
   }
}


/**
 * @brief Focus next widget.
 */
void toolkit_nextFocus (void)
{
   Window *wdw;

   wdw = toolkit_getActiveWindow();
   if (wdw == NULL)
      return;

   if (wdw->nwidgets==0) /* special case no widgets */
      wdw->focus = -1;
   else if (wdw->focus+1 >= wdw->nwidgets)
      wdw->focus = -1;
   else if ((++wdw->focus+1) && /* just increment */
         toolkit_isFocusable(&wdw->widgets[wdw->focus]) )
      return;
   else
      toolkit_nextFocus();
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
   int i;

   /* Get window that can be focused. */
   for (i=nwindows-1; i>=0; i--)
      if (!window_isFlag(&windows[i], WINDOW_NOFOCUS))
         return &windows[i];

   /* No window found. */
   return NULL;
}


/**
 * @brief Gets the focused widget in a window.
 *
 *    @param wdw The window to get the focused widget from.
 *    @return The focused widget.
 */
static Widget* toolkit_getFocus( Window *wdw )
{
   if (wdw->focus == -1)
      return NULL;

   return &wdw->widgets[wdw->focus];
}


/**
 * @brief Initializes the toolkit.
 *
 *    @return 0 on success.
 */
int toolkit_init (void)
{
   GLsizei size;

   /* Allocate some windows. */
   windows = malloc(sizeof(Window)*MIN_WINDOWS);
   nwindows = 0;
   mwindows = MIN_WINDOWS;

   /* Create the VBO. */
   toolkit_vboColourOffset = sizeof(GLfloat) * 2 * 31;
   size = sizeof(GLfloat) * (2+4) * 31;
   toolkit_vbo = gl_vboCreateStream( size, NULL );

   /* DIsable the cursor. */
   SDL_ShowCursor(SDL_DISABLE);

   return 0;
}


/**
 * @brief Exits the toolkit.
 */
void toolkit_exit (void)
{
   /* Destroy the windows. */
   while (nwindows > 0)
      window_destroy(windows[0].id);
   free(windows);

   /* Free the VBO. */
   gl_vboDestroy( toolkit_vbo );
   toolkit_vbo = NULL;
}
