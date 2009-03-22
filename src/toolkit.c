/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file toolkit.c
 *
 * @brief Handles windows and widgets.
 */


#include "toolkit.h"

#include <stdarg.h>

#include "tk/toolkit_priv.h"

#include "naev.h"
#include "log.h"
#include "pause.h"
#include "opengl.h"
#include "input.h"
#include "nstd.h"


#define INPUT_DELAY     500 /**< Delay before starting to repeat. */
#define INPUT_FREQ      100 /**< Interval between repetition. */


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


/*
 * default outline colours
 */
glColour* toolkit_colLight = &cGrey90; /**< Light outline colour. */
glColour* toolkit_col      = &cGrey70; /**< Normal outline colour. */
glColour* toolkit_colDark  = &cGrey30; /**< Dark outline colour. */

/*
 * static prototypes
 */
/* input */
static void toolkit_mouseEvent( SDL_Event* event );
static int toolkit_keyEvent( SDL_Event* event );
/* focus */
static int toolkit_isFocusable( Widget *wgt );
static Widget* toolkit_getFocus( Window *wdw );
/* render */
static void window_render( Window* w );


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

   wdw = &windows[nwindows];

   wdw->id           = wid;
   wdw->name         = strdup(name);

   /* Sane defaults. */
   wdw->hidden       = 0;
   wdw->focus        = -1;
   wdw->parent       = 0;
   wdw->close_fptr   = NULL;
   wdw->accept_fptr  = NULL;
   wdw->cancel_fptr  = NULL;

   /* Widgets */
   wdw->widgets      = NULL;
   wdw->nwidgets     = 0;

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
   glShadeModel( (lc==NULL) ? GL_FLAT : GL_SMOOTH );
   if (!lc) COLOUR(*c);
   glBegin(GL_LINE_LOOP);
      /* left */
      if (lc) COLOUR(*lc);
      glVertex2d( x - b,      y         );
      if (lc) COLOUR(*c);
      glVertex2d( x - b,      y + h     );
      /* top */
      glVertex2d( x,          y + h + b );
      glVertex2d( x + w,      y + h + b );
      /* right */
      glVertex2d( x + w + b, y + h      );
      if (lc) COLOUR(*lc);
      glVertex2d( x + w + b, y          );
      /* bottom */
      glVertex2d( x + w,      y - b     );
      glVertex2d( x,          y - b     );
      glVertex2d( x - b,      y         );
   glEnd(); /* GL_LINES */
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
   glShadeModel( (lc) ? GL_SMOOTH : GL_FLAT );
   glBegin(GL_QUADS);

      COLOUR(*c);
      glVertex2d( x,     y     );
      glVertex2d( x + w, y     );

      COLOUR( (lc) ? *lc : *c );
      glVertex2d( x + w, y + h );
      glVertex2d( x,     y + h );

   glEnd(); /* GL_QUADS */
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
   GLdouble ctop[4] = { 0.,  1., 0., -y  };
   GLdouble cbot[4] = { 0., -1., 0., y+h };
   GLdouble clef[4] = {  1., 0., 0., -x  };
   GLdouble crig[4] = { -1., 0., 0., x+w };

   glClipPlane(GL_CLIP_PLANE0, ctop);
   glClipPlane(GL_CLIP_PLANE1, cbot);
   glClipPlane(GL_CLIP_PLANE2, clef);
   glClipPlane(GL_CLIP_PLANE3, crig);

   glEnable(GL_CLIP_PLANE0);
   glEnable(GL_CLIP_PLANE1);
   glEnable(GL_CLIP_PLANE2);
   glEnable(GL_CLIP_PLANE3);
}
/**
 * @brief Clears the 2d clipping planes.
 */
void toolkit_unclip (void)
{
   glDisable(GL_CLIP_PLANE0);
   glDisable(GL_CLIP_PLANE1);
   glDisable(GL_CLIP_PLANE2);
   glDisable(GL_CLIP_PLANE3);
}


/**
 * @brief Renders a window.
 *
 *    @param w Window to render.
 */
static void window_render( Window* w )
{
   int i;
   double x, y, wid, hei;
   glColour *lc, *c, *dc, *oc;

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
   /* left side */
   glBegin(GL_POLYGON);
      COLOUR(*c);
      glVertex2d( x + 21., y + 0.6*w->h ); /* center */
      COLOUR(*dc);
      glVertex2d( x + 21., y       );
      glVertex2d( x + 15., y + 1.  );
      glVertex2d( x + 10., y + 3.  );
      glVertex2d( x + 6.,  y + 6.  );
      glVertex2d( x + 3.,  y + 10. );
      glVertex2d( x + 1.,  y + 15. );
      glVertex2d( x,       y + 21. );
      COLOUR(*c);
      glVertex2d( x,       y + 0.6*w->h ); /* infront of center */
      glVertex2d( x,       y + w->h - 21. );
      glVertex2d( x + 1.,  y + w->h - 15. );
      glVertex2d( x + 3.,  y + w->h - 10. );
      glVertex2d( x + 6.,  y + w->h - 6.  );
      glVertex2d( x + 10., y + w->h - 3.  );
      glVertex2d( x + 15., y + w->h - 1.  );
      glVertex2d( x + 21., y + w->h       );
   glEnd(); /* GL_POLYGON */
   /* right side */
   glBegin(GL_POLYGON);
      COLOUR(*c);
      glVertex2d( x + w->w - 21., y + 0.6*w->h ); /* center */
      COLOUR(*dc);
      glVertex2d( x + w->w - 21., y       );
      glVertex2d( x + w->w - 15., y + 1.  );
      glVertex2d( x + w->w - 10., y + 3.  );
      glVertex2d( x + w->w - 6.,  y + 6.  );
      glVertex2d( x + w->w - 3.,  y + 10. );
      glVertex2d( x + w->w - 1.,  y + 15. );
      glVertex2d( x + w->w,       y + 21. );
      COLOUR(*c);
      glVertex2d( x + w->w,       y + 0.6*w->h ); /* infront of center */
      glVertex2d( x + w->w,       y + w->h - 21. );
      glVertex2d( x + w->w - 1.,  y + w->h - 15. );
      glVertex2d( x + w->w - 3.,  y + w->h - 10. );
      glVertex2d( x + w->w - 6.,  y + w->h - 6.  );
      glVertex2d( x + w->w - 10., y + w->h - 3.  );
      glVertex2d( x + w->w - 15., y + w->h - 1.  );
      glVertex2d( x + w->w - 21., y + w->h       );
   glEnd(); /* GL_POLYGON */


   /* 
    * inner outline
    */
   glShadeModel(GL_SMOOTH);
   glBegin(GL_LINE_LOOP);
      /* left side */
      COLOUR(*c);
      glVertex2d( x + 21.+1., y+1.       );
      glVertex2d( x + 15.+1., y + 1.+1.  );
      glVertex2d( x + 10.+1., y + 3.+1.  );
      glVertex2d( x + 6.+1.,  y + 6.+1.  );
      glVertex2d( x + 3.+1.,  y + 10.+1. );
      glVertex2d( x + 1.+1.,  y + 15.+1. );
      glVertex2d( x+1.,       y + 21.+1. );
      COLOUR(*lc);
      glVertex2d( x+1.,       y + 0.6*w->h ); /* infront of center */
      glVertex2d( x+1.,       y + w->h - 21.-1. );
      glVertex2d( x + 1.+1.,  y + w->h - 15.-1. );
      glVertex2d( x + 3.+1.,  y + w->h - 10.-1. );
      glVertex2d( x + 6.+1.,  y + w->h - 6.-1.  );
      glVertex2d( x + 10.+1., y + w->h - 3.-1.  );
      glVertex2d( x + 15.+1., y + w->h - 1.-1.  );
      glVertex2d( x + 21.+1., y + w->h-1.       );
      /* switch to right via top */
      glVertex2d( x + w->w - 21.-1., y + w->h-1.       );
      glVertex2d( x + w->w - 15.-1., y + w->h - 1.-1.  );
      glVertex2d( x + w->w - 10.-1., y + w->h - 3.-1.  );
      glVertex2d( x + w->w - 6.-1.,  y + w->h - 6.-1.  );
      glVertex2d( x + w->w - 3.-1.,  y + w->h - 10.-1. );
      glVertex2d( x + w->w - 1.-1.,  y + w->h - 15.-1. );
      glVertex2d( x + w->w-1.,       y + w->h - 21.-1. );
      glVertex2d( x + w->w-1.,       y + 0.6*w->h ); /* infront of center */
      COLOUR(*c);
      glVertex2d( x + w->w-1.,       y + 21.+1. );
      glVertex2d( x + w->w - 1.-1.,  y + 15.+1. );
      glVertex2d( x + w->w - 3.-1.,  y + 10.+1. );
      glVertex2d( x + w->w - 6.-1.,  y + 6.+1.  );
      glVertex2d( x + w->w - 10.-1., y + 3.+1.  );
      glVertex2d( x + w->w - 15.-1., y + 1.+1.  );
      glVertex2d( x + w->w - 21.-1., y+1.       );
      glVertex2d( x + 21.+1., y+1.       ); /* back to beginning */
   glEnd(); /* GL_LINE_LOOP */


   /*
    * outter outline
    */
   glShadeModel(GL_FLAT);
   glBegin(GL_LINE_LOOP);
      /* left side */
      COLOUR(*oc);
      glVertex2d( x + 21., y       );
      glVertex2d( x + 15., y + 1.  );
      glVertex2d( x + 10., y + 3.  );
      glVertex2d( x + 6.,  y + 6.  );
      glVertex2d( x + 3.,  y + 10. );
      glVertex2d( x + 1.,  y + 15. );
      glVertex2d( x,       y + 21. );
      glVertex2d( x,       y + 0.6*w->h ); /* infront of center */
      glVertex2d( x,       y + w->h - 21. );
      glVertex2d( x + 1.,  y + w->h - 15. );
      glVertex2d( x + 3.,  y + w->h - 10. );
      glVertex2d( x + 6.,  y + w->h - 6.  );
      glVertex2d( x + 10., y + w->h - 3.  );
      glVertex2d( x + 15., y + w->h - 1.  );
      glVertex2d( x + 21., y + w->h       );
      /* switch to right via top */
      glVertex2d( x + w->w - 21., y + w->h       );
      glVertex2d( x + w->w - 15., y + w->h - 1.  );
      glVertex2d( x + w->w - 10., y + w->h - 3.  );
      glVertex2d( x + w->w - 6.,  y + w->h - 6.  );
      glVertex2d( x + w->w - 3.,  y + w->h - 10. );
      glVertex2d( x + w->w - 1.,  y + w->h - 15. );
      glVertex2d( x + w->w,       y + w->h - 21. );
      glVertex2d( x + w->w,       y + 0.6*w->h ); /* infront of center */
      glVertex2d( x + w->w,       y + 21. );
      glVertex2d( x + w->w - 1.,  y + 15. );
      glVertex2d( x + w->w - 3.,  y + 10. );
      glVertex2d( x + w->w - 6.,  y + 6.  );
      glVertex2d( x + w->w - 10., y + 3.  );
      glVertex2d( x + w->w - 15., y + 1.  );
      glVertex2d( x + w->w - 21., y       );
      glVertex2d( x + 21., y       ); /* back to beginning */
   glEnd(); /* GL_LINE_LOOP */

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

   if (gl_has(OPENGL_AA_LINE)) glEnable(GL_LINE_SMOOTH);
   if (gl_has(OPENGL_AA_POLYGON)) glEnable(GL_POLYGON_SMOOTH);

   for (i=0; i<nwindows; i++)
      window_render(&windows[i]);
   
   if (gl_has(OPENGL_AA_LINE)) glDisable(GL_LINE_SMOOTH);
   if (gl_has(OPENGL_AA_POLYGON)) glDisable(GL_POLYGON_SMOOTH);
}


/**
 * @brief Toolkit input handled here.
 *
 *    @param event Event to handle.
 *    @return 1 if input was used, 0 if it wasn't.
 */
int toolkit_input( SDL_Event* event )
{
   switch (event->type) {
      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
         toolkit_mouseEvent(event);
         return 1; /* block input */

      case SDL_KEYDOWN:
      case SDL_KEYUP:
         return toolkit_keyEvent(event);

   }
   return 0; /* don't block input */
}


/**
 * @brief Handles the mouse events.
 *
 *    @param event Mouse event to handle.
 */
static void toolkit_mouseEvent( SDL_Event* event )
{
   int i;
   double x,y;
   Window *w;
   Widget *wgt;

   /* Get the window. */
   w = toolkit_getActiveWindow();

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
static void toolkit_regKey( SDLKey key )
{
   if ((input_key==0) && (input_keyTime==0)) {
      input_key         = key;
      input_keyTime     = SDL_GetTicks();
      input_keyCounter  = 0;
   }
}
/**
 * @brief Unregisters a key.
 *
 *    @param key Key to unregister.
 */
static void toolkit_unregKey( SDLKey key )
{
   if (input_key == key) {
      input_key         = 0;
      input_keyTime     = 0;
      input_keyCounter  = 0;
   }
}
/**
 * @brief Clears the registered keys.
 */
void toolkit_clearKey (void)
{
   input_key         = 0;
   input_keyTime     = 0;
   input_keyCounter  = 0;
}
/**
 * @brief Handles keyboard events.
 *
 *    @param event Keyboard event to handle.
 *    @return 1 if the event is used, 0 if it isn't.
 */
static int toolkit_keyEvent( SDL_Event* event )
{
   Window *wdw; 
   Widget *wgt;
   SDLKey key;
   SDLMod mod;
   char buf[2];

   /* Needs to have at least one window. */
   if (nwindows<=0)
      return 0;

   /* Get event and key. */
   wdw = toolkit_getActiveWindow();
   wgt = toolkit_getFocus( wdw );
   key = event->key.keysym.sym;
   mod = event->key.keysym.mod;

   /* hack to simulate key repetition */
   if (event->type == SDL_KEYDOWN)
      toolkit_regKey(key);
   else if (event->type == SDL_KEYUP)
      toolkit_unregKey(key);

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
            (*wdw->accept_fptr)(wdw->id,wdw->name);
            return 1;
         }
         break;

      case SDLK_ESCAPE:
         if (wdw->cancel_fptr != NULL) {
            (*wdw->cancel_fptr)(wdw->id,wdw->name);
            return 1;
         }
         break;

      default:
         break;
   }

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
      wgt = toolkit_getFocus( wdw );
      if ((wgt != NULL) && (wgt->keyevent != NULL))
         wgt->keyevent( wgt, input_key, 0 );
   }
}


/**
 * @brief Focus next widget.
 */
void toolkit_nextFocus (void)
{
   Window *wdw;

   wdw = toolkit_getActiveWindow();

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
   return &windows[nwindows-1];
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
   windows = malloc(sizeof(Window)*MIN_WINDOWS);
   nwindows = 0;
   mwindows = MIN_WINDOWS;
   SDL_ShowCursor(SDL_DISABLE);

   return 0;
}


/**
 * @brief Exits the toolkit.
 */
void toolkit_exit (void)
{
   while (nwindows > 0)
      window_destroy(windows[0].id);
   free(windows);
}
