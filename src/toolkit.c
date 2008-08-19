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

#include "naev.h"
#include "log.h"
#include "pause.h"
#include "opengl.h"
#include "input.h"


#define INPUT_DELAY     500 /**< Delay before starting to repeat. */
#define INPUT_FREQ      100 /**< Interval between repetition. */


/**
 * @typedef WidgetType
 *
 * @brief Represents the widget types.
 */
typedef enum WidgetType_ {
   WIDGET_NULL,
   WIDGET_BUTTON,
   WIDGET_TEXT,
   WIDGET_IMAGE,
   WIDGET_LIST,
   WIDGET_RECT,
   WIDGET_CUST,
   WIDGET_INPUT,
   WIDGET_IMAGEARRAY
} WidgetType;

/**
 * @typedef WidgetStatus
 *
 * @brief Represents widget status.
 *
 * Only really used by buttons.
 */
typedef enum WidgetStatus_ {
   WIDGET_STATUS_NORMAL,
   WIDGET_STATUS_MOUSEOVER,
   WIDGET_STATUS_MOUSEDOWN,
   WIDGET_STATUS_SCROLLING
} WidgetStatus;


/**
 * @struct Widget
 *
 * @brief Represents a widget.
 */
typedef struct Widget_ {
   char* name; /**< Widget's name. */
   WidgetType type; /**< Widget's type. */

   double x; /**< X position within the window. */
   double y; /**< Y position within the window. */
   double w; /**< Widget width. */
   double h; /**< Widget height. */

   WidgetStatus status; /**< Widget status. */

   union {
      struct { /* WIDGET_BUTTON */
         void (*fptr) (char*); /**< Activate callback. */
         char *display; /**< Displayed text. */
         int disabled; /**< 1 if button is disabled, 0 if enabled. */
      } btn; /**< WIDGET_BUTTON */

      struct { /* WIDGET_TEXT */
         char *text; /**< Text to display, using printMid if centered, else printText. */
         glFont* font; /**< Text font. */
         glColour* colour; /**< Text colour. */
         int centered; /**< 1 if text is centered, 0 if it isn't. */
      } txt; /**< WIDGET_TEXT */

      struct { /* WIDGET_IMAGE */
         glTexture* image; /**< Image to display. */
         glColour* colour; /**< Colour to warp to. */
         int border; /**< 1 if widget should have border. */
      } img; /**< WIDGET_IMAGE */

      struct { /* WIDGET_LIST */
         char **options; /**< Pointer to the options. */
         int noptions; /**< Total number of options. */
         int selected; /**< Which option is currently selected. */
         int pos; /** Current topmost option (in view). */
         void (*fptr) (char*); /**< Modify callback - triggered on selection. */
      } lst; /**< WIDGET_LIST */

      struct { /* WIDGET_RECT */
         glColour* colour; /**< Background colour. */
         int border; /**< 1 if widget should have border, 0 if it shouldn't. */
      } rct; /**< WIDGET_RECT */

      struct { /* WIDGET_CUST */
         int border; /**< 1 if widget should have border, 0 if it shouldn't. */
         void (*render) (double bx, double by, double bw, double bh); /**< Function to run when rendering. */
         void (*mouse) (SDL_Event* event, double bx, double by); /**< Function to run when recieving mous events. */
      } cst; /**< WIDGET_CUST */

      struct { /* WIDGET_INPUT */
         char *input; /**< Input buffer. */
         int max; /**< Maximum length. */
         int oneline; /**< Is it a one-liner? no '\n' and friends */
         int view; /**< View position. */
         int pos; /**< Cursor position. */
      } inp; /**< WIDGET_INPUT */

      struct { /* WIDGET_IMAGEARRAY */
         glTexture **images; /**< Image array. */
         char **captions; /**< Corresponding caption array. */
         int nelements; /**< Number of elements. */
         int selected; /**< Currently selected element. */
         double pos; /**< Current y position. */
         int iw; /**< Image width to use. */
         int ih; /**< Image height to use. */
         void (*fptr) (char*); /**< Modify callback - triggered on selection. */
      } iar; /**< WIDGET_IMAGEARRAY */
   } dat; /**< Stores the widget specific data. */
} Widget;


/**
 * @struct Window
 *
 * @brief Represents a graphical window.
 */
typedef struct Window_ {
   unsigned int id; /**< Unique ID. */
   char *name; /**< Window name - should be unique. */

   int hidden; /**< Is window hidden? - @todo use */
   int focus; /**< Current focused widget. */

   void (*accept_fptr)( char* ); /**< Triggered by hitting 'enter' with no widget that catches the keypress. */
   void (*cancel_fptr)( char* ); /**< Triggered by hitting 'escape' with no widget that catches the keypress. */

   double x; /**< X position of the window. */
   double y; /**< Y position of the window. */
   double w; /**< Window width. */
   double h; /**< Window height. */

   Widget *widgets; /**< Widget storage. */
   int nwidgets; /**< Total number of widgets. */
} Window;


static unsigned int genwid = 0; /**< Generates unique window ids, > 0 */


int toolkit = 0; /**< 1 if toolkit is in use, 0 else. */

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
static SDLKey input_key; /**< Current pressed key. */
static unsigned int input_keyTime; /**< Tick pressed. */
static int input_keyCounter; /**< Number of repetitions. */


/*
 * Converts absolute mouse events to relative mouse events.
 */
static double last_x = 0.; /**< Last x mouse position. */
static double last_y = 0.; /**< Last y mouse position. */


/*
 * default outline colours
 */
static glColour* toolkit_colLight = &cGrey90; /**< Light outline colour. */
static glColour* toolkit_col = &cGrey70; /**< Normal outline colour. */
static glColour* toolkit_colDark = &cGrey30; /**< Dark outline colour. */

/*
 * static prototypes
 */
/* widgets */
static Widget* window_newWidget( Window* w );
static void widget_cleanup( Widget *widget );
static Window* window_wget( const unsigned int wid );
static Widget* window_getwgt( const unsigned int wid, char* name );
static void toolkit_setPos( Window *wdw, Widget *wgt, int x, int y );
/* input */
static int toolkit_inputInput( Uint8 type, Widget* inp, SDLKey key );
static void toolkit_mouseEvent( SDL_Event* event );
static int toolkit_keyEvent( SDL_Event* event );
static void toolkit_imgarrMove( Widget* iar, double ry );
/* focus */
static void toolkit_nextFocus (void);
static int toolkit_isFocusable( Widget *wgt );
static void toolkit_triggerFocus (void);
static Widget* toolkit_getFocus (void);
static void toolkit_listScroll( Widget* wgt, int direction );
static void toolkit_listFocus( Widget* lst, double bx, double by );
static void toolkit_imgarrFocus( Widget* iar, double bx, double by );
/* render */
static void window_render( Window* w );
static void toolkit_renderButton( Widget* btn, double bx, double by );
static void toolkit_renderText( Widget* txt, double bx, double by );
static void toolkit_renderImage( Widget* img, double bx, double by );
static void toolkit_renderList( Widget* lst, double bx, double by );
static void toolkit_renderRect( Widget* rct, double bx, double by );
static void toolkit_renderCust( Widget* cst, double bx, double by );
static void toolkit_renderInput( Widget* inp, double bx, double by );
static void toolkit_renderImageArray( Widget* iar, double bx, double by );
static void toolkit_drawOutline( double x, double y,
      double w, double h, double b,
      glColour* c, glColour* lc );
static void toolkit_clip( double x, double y, double w, double h );
static void toolkit_unclip (void);
static void toolkit_drawRect( double x, double y,
      double w, double h, glColour* c, glColour* lc );


/*
 * Sets the internal widget position.
 */
static void toolkit_setPos( Window *wdw, Widget *wgt, int x, int y )
{
   if (x < 0) wgt->x = wdw->w - wgt->w + x;
   else wgt->x = (double) x;
   if (y < 0) wgt->y = wdw->h - wgt->h + y;
   else wgt->y = (double) y;
}


/**
 * @fn void window_addButton( const unsigned int wid,
 *                            const int x, const int y,
 *                            const int w, const int h,
 *                            char* name, char* display,
 *                            void (*call) (char*) )
 *
 * @brief Adds a button widget to a window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the button widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Width of the button.
 *    @param h Height of the button.
 *    @param name Name of the button to use internally.
 *    @param display Text displayed on the button (centered).
 *    @param call Function to call when button is pressed. Parameter passed
 *                is the name of the button.
 */
void window_addButton( const unsigned int wid,
                       const int x, const int y,
                       const int w, const int h,
                       char* name, char* display,
                       void (*call) (char*) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_BUTTON;
   wgt->name = strdup(name);
   
   /* specific */
   wgt->dat.btn.display = strdup(display);
   wgt->dat.btn.disabled = 0; /* initially enabled */
   wgt->dat.btn.fptr = call;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/*
 * adds text to the window
 */
void window_addText( const unsigned int wid,
      const int x, const int y,
      const int w, const int h,
      const int centered, char* name,
      glFont* font, glColour* colour, char* string )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_TEXT;
   wgt->name = strdup(name);

   /* specific */
   if (font==NULL) wgt->dat.txt.font = &gl_defFont;
   else wgt->dat.txt.font = font;
   if (font==NULL) wgt->dat.txt.font = &gl_defFont;
   else wgt->dat.txt.font = font;
   if (colour==NULL) wgt->dat.txt.colour = &cBlack;
   else wgt->dat.txt.colour = colour;
   wgt->dat.txt.centered = centered;
   if (string) wgt->dat.txt.text = strdup(string);
   else wgt->dat.txt.text = NULL;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/*
 * adds a graphic to the window
 */
void window_addImage( const unsigned int wid,
      const int x, const int y,
      char* name, glTexture* image, int border )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_IMAGE;
   wgt->name = strdup(name);

   /* specific */
   wgt->dat.img.image = image;
   wgt->dat.img.border = border;
   wgt->dat.img.colour = NULL; /* normal colour */

   /* position/size */
   wgt->w = (image==NULL) ? 0 : wgt->dat.img.image->sw;
   wgt->h = (image==NULL) ? 0 : wgt->dat.img.image->sh;
   toolkit_setPos( wdw, wgt, x, y );
}


/*
 * adds a list to the window
 */
void window_addList( const unsigned int wid,
      const int x, const int y,
      const int w, const int h,
      char* name, char **items, int nitems, int defitem,
      void (*call) (char*) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_LIST;
   wgt->name = strdup(name);

   /* specific */
   wgt->dat.lst.options = items;
   wgt->dat.lst.noptions = nitems;
   wgt->dat.lst.selected = defitem; /* -1 would be none */
   wgt->dat.lst.pos = 0;
   wgt->dat.lst.fptr = call;
   
   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h - ((h % (gl_defFont.h+2)) + 2);
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/*
 * adds a rectangle to the window
 */
void window_addRect( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, glColour* colour, int border ) /* properties */
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_RECT;
   wgt->name = strdup(name);

   /* specific */
   wgt->dat.rct.colour = colour;
   wgt->dat.rct.border = border;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/*
 * adds a custom widget
 */
void window_addCust( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int border,
      void (*render) (double x, double y, double w, double h),
      void (*mouse) (SDL_Event* event, double x, double y) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_CUST;
   wgt->name = strdup(name);

   /* specific */
   wgt->dat.cst.border = border;
   wgt->dat.cst.render = render;
   wgt->dat.cst.mouse = mouse;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/*
 * adds an input widget
 */
void window_addInput( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int max, const int oneline )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_INPUT;
   wgt->name = strdup(name);

   /* specific */
   wgt->dat.inp.max = max+1;
   wgt->dat.inp.oneline = oneline;
   wgt->dat.inp.pos = 0;
   wgt->dat.inp.view = 0;
   wgt->dat.inp.input = malloc(sizeof(char)*wgt->dat.inp.max);
   memset(wgt->dat.inp.input, 0, wgt->dat.inp.max*sizeof(char));

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Adds an Image Array.
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
      char* name, const int iw, const int ih, /* name and image sizes */
      glTexture** tex, char** caption, int nelem, /* elements */
      void (*call) (char*) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_IMAGEARRAY;
   wgt->name = strdup(name);

   /* specific */
   wgt->dat.iar.images = tex;
   wgt->dat.iar.captions = caption;
   wgt->dat.iar.nelements = nelem;
   wgt->dat.iar.selected = 0;
   wgt->dat.iar.pos = 0;
   wgt->dat.iar.iw = iw;
   wgt->dat.iar.ih = ih;
   wgt->dat.iar.fptr = call;
   
   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}



/*
 * returns pointer to a newly alloced Widget
 */
static Widget* window_newWidget( Window* w )
{
   Widget* wgt = NULL;

   w->nwidgets++;
   w->widgets = realloc( w->widgets,
         sizeof(Widget)*w->nwidgets );
   if (w->widgets == NULL) WARN("Out of Memory");

   wgt = &w->widgets[ w->nwidgets - 1 ]; 

   wgt->type = WIDGET_NULL;
   wgt->status = WIDGET_STATUS_NORMAL;
   return wgt;
}


/*
 * returns the window of id wid
 */
static Window* window_wget( const unsigned int wid )
{
   int i;
   for (i=0; i<nwindows; i++)
      if (windows[i].id == wid)
         return &windows[i];
   DEBUG("Window '%d' not found in windows stack", wid);
   return NULL;
}


/*
 * gets the wgt from the window
 */
static Widget* window_getwgt( const unsigned int wid, char* name )
{
   int i;
   Window *wdw = window_wget(wid);

   for (i=0; i<wdw->nwidgets; i++)
      if (strcmp(wdw->widgets[i].name, name)==0)
         return &wdw->widgets[i];

   WARN("Widget '%s' not found in window '%u'!", name, wid );
   return NULL;
}


/*
 * modifies an existing text string
 */
void window_modifyText( const unsigned int wid,
      char* name, char* newstring )
{
   Widget *wgt = window_getwgt(wid,name);

   if (wgt->dat.txt.text) free(wgt->dat.txt.text);
   wgt->dat.txt.text = (newstring) ?  strdup(newstring) : NULL;
}


/*
 * Gets a widget's position.
 */
void window_posWidget( const unsigned int wid,
      char* name, int *x, int *y )
{
   Widget *wgt = window_getwgt(wid,name);

   (*x) = wgt->x;
   (*y) = wgt->y;
}


/*
 * Moves a widget.
 */
void window_moveWidget( const unsigned int wid,
      char* name, int x, int y )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_getwgt(wid,name);

   toolkit_setPos( wdw, wgt, x, y );
}


/*
 * disables a button
 */
void window_disableButton( const unsigned int wid, char* name )
{
   Widget *wgt = window_getwgt(wid,name);

   if (wgt->type!=WIDGET_BUTTON) {
      DEBUG("Trying to disable a non-button widget '%s'", name);
      return;
   }
   wgt->dat.btn.disabled = 1;
}
/* 
 * enables a button
 */
void window_enableButton( const unsigned int wid, char *name )
{
   Widget *wgt = window_getwgt(wid,name);

   if (wgt->type!=WIDGET_BUTTON) {
      DEBUG("Trying to enable a non-button widget '%s'", name);
      return;
   }
   wgt->dat.btn.disabled = 0;

}


/*
 * modifies an existing image's image
 */
void window_modifyImage( const unsigned int wid,
      char* name, glTexture* image )
{
   Widget *wgt = window_getwgt(wid,name);

   wgt->dat.img.image = image;
}


/*
 * changes an existing image's colour
 */
void window_imgColour( const unsigned int wid,
      char* name, glColour* colour )
{
   Widget *wgt = window_getwgt(wid,name);

   wgt->dat.img.colour = colour;
}


/*
 * gets the image from an image widget
 */
glTexture* window_getImage( const unsigned int wid, char* name )
{
   Widget *wgt = window_getwgt(wid,name);
   return (wgt) ? wgt->dat.img.image : NULL;
}


/*
 * gets the input from an input widget
 */
char* window_getInput( const unsigned int wid, char* name )
{
   Widget *wgt = window_getwgt(wid,name);
   return (wgt) ? wgt->dat.inp.input : NULL;
}



/*
 * checks to see if a window exists
 */
int window_exists( const char* wdwname )
{
   int i;
   for (i=0; i<nwindows; i++)
      if (strcmp(windows[i].name,wdwname)==0)
         return 1; /* exists */
   return 0; /* doesn't exist */
}


/*
 * returns the id of a window
 */
unsigned int window_get( const char* wdwname )
{
   int i;
   for (i=0; i<nwindows; i++)
      if (strcmp(windows[i].name,wdwname)==0)
         return windows[i].id;
   DEBUG("Window '%s' not found in windows stack", wdwname);
   return 0;
}


/*
 * creates a window
 */
unsigned int window_create( char* name,
      const int x, const int y, const int w, const int h )
{
   Window *wdw;

   if (nwindows >= mwindows) { /* at memory limit */
      windows = realloc(windows, sizeof(Window)*(++mwindows));
      if (windows==NULL) WARN("Out of memory");
   }

   const int wid = (++genwid); /* unique id */

   wdw = &windows[nwindows];

   wdw->id = wid;
   wdw->name = strdup(name);

   wdw->hidden = 0;
   wdw->focus = -1;
   wdw->accept_fptr = NULL;
   wdw->cancel_fptr = NULL;

   wdw->w = (double) w;
   wdw->h = (double) h;
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

   /* Widgets */
   wdw->widgets = NULL;
   wdw->nwidgets = 0;

   nwindows++;
   
   if (toolkit==0) { /* toolkit is on */
      SDL_ShowCursor(SDL_ENABLE);
      toolkit = 1; /* enable toolkit */
      pause_game();
      gl_defViewport(); /* Reset the default viewport */
   }

   return wid;
}


/**
 * @fn void window_setAccept( const unsigned int wid, void (*accept)( char* ) )
 *
 * @ brief Sets the default accept function of the window.
 *
 * This function is called whenever 'enter' is pressed and the current widget
 *  does not catch it.  NULL disables the accept function.
 *
 *    @param wid ID of the window to set the accept function.
 *    @param accept Function to trigger when window is "accepted".  Parameter
 *                  passed is window name.
 */
void window_setAccept( const unsigned int wid, void (*accept)( char* ) )
{
   Window *wdw;

   wdw = window_wget( wid );
   if (wdw != NULL) wdw->accept_fptr = accept;
}


/**
 * @fn void window_setCancel( const unsigned int wid, void (*cancel)( char* ) )
 *
 * @brief Sets the default cancel function of the window.
 *
 * This function is called whenever 'escape' is hit and the current widget
 *  does not catch it.  NULL disables the cancel function.
 *
 *    @param wid ID of the window to set cancel function.
 *    @param cancel Function to trigger when window is "cancelled".  Parameter
 *                  passed is window name.
 */
void window_setCancel( const unsigned int wid, void (*cancel)( char* ) )
{
   Window *wdw;

   wdw = window_wget( wid );
   if (wdw != NULL) wdw->cancel_fptr = cancel;
}


/*
 * destroys a widget
 */
static void widget_cleanup( Widget *widget )
{
   int i;

   if (widget->name) free(widget->name);

   switch (widget->type) {
      case WIDGET_BUTTON: /* must clear the button display text */
         if (widget->dat.btn.display) free(widget->dat.btn.display);
         break;

      case WIDGET_TEXT: /* must clear the text */
         if (widget->dat.txt.text) free(widget->dat.txt.text);
         break;

      case WIDGET_LIST: /* must clear the list */
         if (widget->dat.lst.options) {
            for (i=0; i<widget->dat.lst.noptions; i++)
               if (widget->dat.lst.options[i])
                  free(widget->dat.lst.options[i]);
            free( widget->dat.lst.options );
         }
         break;

      case WIDGET_INPUT:
         free(widget->dat.inp.input); /* frees the input buffer */
         break;

      case WIDGET_IMAGEARRAY:
         if (widget->dat.iar.nelements > 0) { /* Free each text individually */
            for (i=0; i<widget->dat.iar.nelements; i++)
               if (widget->dat.iar.captions[i])
                  free(widget->dat.iar.captions[i]);
            /* Free the arrays */
            free( widget->dat.iar.captions );
            free( widget->dat.iar.images );
         }
         break;


      default:
         break;
   }
}


/* 
 * destroys a window
 */
void window_destroy( const unsigned int wid )
{
   int i,j;

   /* destroy the window */
   for (i=0; i<nwindows; i++)
      if (windows[i].id == wid) {
         if (windows[i].name) free(windows[i].name);
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
      toolkit = 0; /* disable toolkit */
      if (paused) unpause_game();
   }
}


/*
 * destroys a widget on a window
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


/* 
 * draws an outline
 * if bc is NULL, colour will be flat
 */
static void toolkit_drawOutline( double x, double y, 
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
static void toolkit_drawRect( double x, double y,
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


/*
 * sets up 2d clipping planes around a rectangle
 */
static void toolkit_clip( double x, double y, double w, double h )
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
static void toolkit_unclip (void)
{
   glDisable(GL_CLIP_PLANE0);
   glDisable(GL_CLIP_PLANE1);
   glDisable(GL_CLIP_PLANE2);
   glDisable(GL_CLIP_PLANE3);
}


/*
 * renders a window
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
   gl_printMid( &gl_defFont, w->w,
         x + (double)SCREEN_W/2.,
         y + w->h - 20. + (double)SCREEN_H/2.,
         &cBlack, w->name );

   /*
    * widgets
    */
   for (i=0; i<w->nwidgets; i++) {

      switch (w->widgets[i].type) {
         case WIDGET_NULL: break;

         case WIDGET_BUTTON:
            toolkit_renderButton( &w->widgets[i], x, y );
            break;

         case WIDGET_TEXT:
            toolkit_renderText( &w->widgets[i], x, y );
            break;

         case WIDGET_IMAGE:
            toolkit_renderImage( &w->widgets[i], x, y );
            break;

         case WIDGET_LIST:
            toolkit_renderList( &w->widgets[i], x, y );
            break;

         case WIDGET_RECT:
            toolkit_renderRect( &w->widgets[i], x, y );
            break;

         case WIDGET_CUST:
            toolkit_renderCust( &w->widgets[i], x, y );
            break;

         case WIDGET_INPUT:
            toolkit_renderInput( &w->widgets[i], x, y );
            break;

         case WIDGET_IMAGEARRAY:
            toolkit_renderImageArray( &w->widgets[i], x, y );
            break;
      }
   }

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


/*
 * renders a button
 */
static void toolkit_renderButton( Widget* btn, double bx, double by )
{
   glColour *c, *dc, *lc;
   double x, y;

   x = bx + btn->x;
   y = by + btn->y;

   /* set the colours */
   if (btn->dat.btn.disabled==1) {
      lc = &cGrey60;
      c = &cGrey20;
      dc = &cGrey40;
   }
   else {
      switch (btn->status) {
         case WIDGET_STATUS_NORMAL:
            lc = &cGrey80;
            c = &cGrey60;
            dc = &cGrey40;
            break;
         case WIDGET_STATUS_MOUSEOVER:
            lc = &cWhite;
            c = &cGrey80;
            dc = &cGrey60;
            break;
         case WIDGET_STATUS_MOUSEDOWN:
            lc = &cGreen;
            c = &cGreen;
            dc = &cGrey40;
            break;
         default:
            break;
      }
   }


   /* shaded base */
   if (btn->dat.btn.disabled==1) {
      toolkit_drawRect( x, y,            btn->w, 0.4*btn->h, dc, NULL );
      toolkit_drawRect( x, y+0.4*btn->h, btn->w, 0.6*btn->h, dc, c );
   }
   else {
      toolkit_drawRect( x, y,            btn->w, 0.6*btn->h, dc, c );
      toolkit_drawRect( x, y+0.6*btn->h, btn->w, 0.4*btn->h, c, NULL );
   }
   
   /* inner outline */
   toolkit_drawOutline( x, y, btn->w, btn->h, 0., lc, c );
   /* outter outline */
   toolkit_drawOutline( x, y, btn->w, btn->h, 1., &cBlack, NULL );

   gl_printMid( NULL, (int)btn->w,
         bx + (double)SCREEN_W/2. + btn->x,
         by + (double)SCREEN_H/2. + btn->y + (btn->h - gl_defFont.h)/2.,
         &cDarkRed, btn->dat.btn.display );
}
/*
 * renders the text
 */
static void toolkit_renderText( Widget* txt, double bx, double by )
{
   if (txt->dat.txt.text==NULL) return;
   
   if (txt->dat.txt.centered)
      gl_printMid( txt->dat.txt.font, txt->w,
            bx + (double)SCREEN_W/2. + txt->x,
            by + (double)SCREEN_H/2. + txt->y,
            txt->dat.txt.colour, txt->dat.txt.text );
   else
      gl_printText( txt->dat.txt.font, txt->w, txt->h,
            bx + (double)SCREEN_W/2. + txt->x,
            by + (double)SCREEN_H/2. + txt->y,
            txt->dat.txt.colour, txt->dat.txt.text );
}
/*
 * renders the image
 */
static void toolkit_renderImage( Widget* img, double bx, double by )
{
   double x,y;

   if (img->dat.img.image == NULL) return;

   x = bx + img->x;
   y = by + img->y;

   /*
    * image
    */
   gl_blitStatic( img->dat.img.image,
         x + (double)SCREEN_W/2.,
         y + (double)SCREEN_H/2.,
         img->dat.img.colour );

   if (img->dat.img.border) {
      /* inner outline (outwards) */
      toolkit_drawOutline( x, y+1, img->dat.img.image->sw-1,
         img->dat.img.image->sh-1, 1., toolkit_colLight, toolkit_col );
      /* outter outline */
      toolkit_drawOutline( x, y+1, img->dat.img.image->sw-1,
            img->dat.img.image->sh-1, 2., toolkit_colDark, NULL );
   }
}


/*
 * renders the list
 */
static void toolkit_renderList( Widget* lst, double bx, double by )
{
   int i;
   double x,y, tx,ty;

   x = bx + lst->x;
   y = by + lst->y;

   /* lst bg */
   toolkit_drawRect( x, y, lst->w, lst->h, &cWhite, NULL );

   /* inner outline */
   toolkit_drawOutline( x, y, lst->w, lst->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outter outline */
   toolkit_drawOutline( x, y, lst->w, lst->h, 1., toolkit_colDark, NULL );

   /* draw selected */
   toolkit_drawRect( x, y - 1. + lst->h -
         (1 + lst->dat.lst.selected - lst->dat.lst.pos)*(gl_defFont.h+2.),
         lst->w, gl_defFont.h + 2., &cHilight, NULL );

   /* draw content */
   tx = (double)SCREEN_W/2. + x + 2.;
   ty = (double)SCREEN_H/2. + y + lst->h - 2. - gl_defFont.h;
   y = ty - 2.;
   for (i=lst->dat.lst.pos; i<lst->dat.lst.noptions; i++) {
      gl_printMax( &gl_defFont, (int)lst->w-4,
            tx, ty, &cBlack, lst->dat.lst.options[i] );
      ty -= 2 + gl_defFont.h;
      if (ty-y > lst->h) break;
   }
}


/*
 * renders a rectangle
 */
static void toolkit_renderRect( Widget* rct, double bx, double by )
{
   double x, y;

   x = bx + rct->x;
   y = by + rct->y;

   if (rct->dat.rct.colour) /* draw rect only if it exists */
      toolkit_drawRect( x, y, rct->w, rct->h, rct->dat.rct.colour, NULL );

   if (rct->dat.rct.border) {
      /* inner outline */
      toolkit_drawOutline( x, y, rct->w, rct->h, 0.,
            toolkit_colLight, toolkit_col );
      /* outter outline */
      toolkit_drawOutline( x, y, rct->w, rct->h, 1.,
            toolkit_colDark, NULL );
   }
}


/*
 * renders a custom widget
 */
static void toolkit_renderCust( Widget* cst, double bx, double by )
{
   double x,y;

   x = bx + cst->x;
   y = by + cst->y;

   if (cst->dat.cst.border) {
      /* inner outline */
      toolkit_drawOutline( x-1, y+1, cst->w+1, cst->h+1, 0.,
            toolkit_colLight, toolkit_col );
      /* outter outline */
      toolkit_drawOutline( x-1, y, cst->w+1, cst->h+1, 1.,
            toolkit_colDark, NULL );
   }

   toolkit_clip( x, y, cst->w, cst->h );
   (*cst->dat.cst.render) ( x, y, cst->w, cst->h );
   toolkit_unclip();
}


/*
 * renders an input widget
 */
static void toolkit_renderInput( Widget* inp, double bx, double by )
{
   double x, y, ty;

   x = bx + inp->x;
   y = by + inp->y;

   /* main background */
   toolkit_drawRect( x, y, inp->w, inp->h, &cWhite, NULL );

   /* center vertically */
   if (inp->dat.inp.oneline) ty = y - (inp->h - gl_smallFont.h)/2.;

   gl_printText( &gl_smallFont, inp->w-10., inp->h,
         x+5. + SCREEN_W/2., ty  + SCREEN_H/2.,
         &cBlack, inp->dat.inp.input + inp->dat.inp.view );

   /* inner outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outter outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 1.,
         toolkit_colDark, NULL );
}


/**
 * @fn static void toolkit_renderImageArray( Widget* iar, double bx, double by )
 *
 * @brief Renders an image array.
 */
static void toolkit_renderImageArray( Widget* iar, double bx, double by )
{
   int i,j;
   double x,y, w,h, xcurs,ycurs;
   double scroll_pos, sx,sy;
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
   /* scrollbar background */
   toolkit_drawRect( x + iar->w - 10., y, 10., iar->h,
         toolkit_colDark, toolkit_col );
   toolkit_drawOutline( x + iar->w - 10., y, 10., iar->h, 1.,
         toolkit_colLight, toolkit_col );
   toolkit_drawOutline( x + iar->w - 10., y, 10., iar->h, 2.,
         toolkit_colDark, NULL );
   /* Bar itself. */
   scroll_pos = iar->dat.iar.pos / (h * (yelem - (int)(iar->h / h)));
   sx = x + iar->w - 10.;
   sy = y + iar-> h - (iar->h - 30.) * scroll_pos - 30.;
   toolkit_drawRect( sx, sy, 10., 30., toolkit_colLight, toolkit_col );
   toolkit_drawOutline( sx, sy, 10., 30., 0., toolkit_colDark, NULL );

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


/*
 * handles input for an input widget
 */
static int toolkit_inputInput( Uint8 type, Widget* inp, SDLKey key )
{
   int n;
   SDLMod mods;

   if (inp->type != WIDGET_INPUT) return 0;

   mods = SDL_GetModState();
   if (inp->dat.inp.oneline && isascii(key)) {

      /* backspace -> delete text */
      if ((type==SDL_KEYDOWN) && (key=='\b') && (inp->dat.inp.pos > 0)) {
         inp->dat.inp.input[ --inp->dat.inp.pos ] = '\0';
         
         if (inp->dat.inp.view > 0) {
            n = gl_printWidth( &gl_smallFont,
                  inp->dat.inp.input + inp->dat.inp.view - 1 );
            if (n+10 < inp->w)
               inp->dat.inp.view--;
         }
      }

      else if ((type==SDL_KEYDOWN) && (inp->dat.inp.pos < inp->dat.inp.max-1)) {
         
         if ((key==SDLK_RETURN) && !inp->dat.inp.oneline)
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = '\n';

         /* upper case characters */
         else if (isalpha(key) && (mods & (KMOD_LSHIFT | KMOD_RSHIFT)))
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = toupper(key);

         /* rest */
         else if (!iscntrl(key))
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = key;

         /* didn't get a useful key */
         else return 0;

         n = gl_printWidth( &gl_smallFont, inp->dat.inp.input+inp->dat.inp.view );
         if (n+10 > inp->w) inp->dat.inp.view++;
         return 1;
      }
   }
   return 0;
}


/*
 * renders the windows
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


/*
 * toolkit input handled here
 * if return is 1, then the input isn't passed along
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


/*
 * input
 */
static int mouse_down = 0;
static void toolkit_mouseEvent( SDL_Event* event )
{
   int i;
   double x,y, rel_x,rel_y;
   Window *w;
   Widget *wgt, *wgt_func;

   /* set mouse button status */
   if (event->type==SDL_MOUSEBUTTONDOWN) mouse_down = 1;
   else if (event->type==SDL_MOUSEBUTTONUP) mouse_down = 0;

   /* absolute positions */
   if (event->type==SDL_MOUSEMOTION) {
      x = (double)event->motion.x;
      y = SCREEN_H - (double)event->motion.y;
      /* Create relative events. */
      rel_x = x - last_x;
      rel_y = y - last_y;
      last_x = x;
      last_y = y;
   }
   else if ((event->type==SDL_MOUSEBUTTONDOWN) || (event->type==SDL_MOUSEBUTTONUP)) {
      x = (double)event->button.x;
      y = SCREEN_H - (double)event->button.y;
   }

   w = &windows[nwindows-1];

   /* always treat button ups to stop hanging states */
   if ((event->type!=SDL_MOUSEBUTTONUP) &&
      ((x < w->x) || (x > (w->x + w->w)) || (y < w->y) || (y > (w->y + w->h))))
      return; /* not in current window */

   /* relative positions */
   x -= w->x;
   y -= w->y;

   wgt_func = NULL;
   for (i=0; i<w->nwidgets; i++) {
      wgt = &w->widgets[i];
      /* widget in range? */
      if ((x > wgt->x) && (x < (wgt->x + wgt->w)) &&
            (y > wgt->y) && (y < (wgt->y + wgt->h))) {
         /* custom widgets take it from here */
         if ((wgt->type==WIDGET_CUST) && wgt->dat.cst.mouse) 
            (*wgt->dat.cst.mouse)( event, x-wgt->x, y-wgt->y );
         else
            switch (event->type) {
               case SDL_MOUSEMOTION:
                  if (!mouse_down)
                     wgt->status = WIDGET_STATUS_MOUSEOVER;
                  else {
                     if (wgt->type == WIDGET_IMAGEARRAY)
                        toolkit_imgarrMove( wgt, rel_y );
                  }
                  break;

               case SDL_MOUSEBUTTONDOWN:
                  wgt->status = WIDGET_STATUS_MOUSEDOWN;

                  /* Handle mouse wheel. */
                  if (event->button.button == SDL_BUTTON_WHEELUP) {
                     toolkit_listScroll( wgt, +1 );
                     break;
                  }
                  if (event->button.button == SDL_BUTTON_WHEELDOWN) {
                     toolkit_listScroll( wgt, -1 );
                     break;
                  }

                  if (toolkit_isFocusable(wgt))
                     w->focus = i;

                  if (wgt->type == WIDGET_LIST) {
                     toolkit_listFocus( wgt, x-wgt->x, y-wgt->y );
                     input_key = 0; /* hack to avoid weird bug with permascroll */
                  }

                  if (wgt->type == WIDGET_IMAGEARRAY)
                     toolkit_imgarrFocus( wgt, x-wgt->x, y-wgt->y );
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
                        else
                           wgt_func = wgt; /* run it at the end in case of close */
                     }
                  }
                  wgt->status = WIDGET_STATUS_NORMAL;
                  break;
            }
      }
      /* otherwise custom widgets can get stuck on mousedown */
      else if ((wgt->type==WIDGET_CUST) &&
            (event->type==SDL_MOUSEBUTTONUP) && wgt->dat.cst.mouse)
            (*wgt->dat.cst.mouse)( event, x-wgt->x, y-wgt->y );
      else if (!mouse_down)
         wgt->status = WIDGET_STATUS_NORMAL;
   }

   /* We trigger this at the end in case it destroys the window that is calling
    * this function.  Otherwise ugly segfaults appear. */
   if (wgt_func) (*wgt_func->dat.btn.fptr)(wgt_func->name);
}


/*
 * handles the key events
 */
static void toolkit_regKey( SDLKey key )
{
   if ((input_key==0) && (input_keyTime==0)) {
      input_key = key;
      input_keyTime = SDL_GetTicks();
      input_keyCounter = 0;
   }
}
static void toolkit_unregKey( SDLKey key )
{
   if (input_key == key) {
      input_key = 0;
      input_keyTime = 0;
      input_keyCounter = 0;
   }
}
static int toolkit_keyEvent( SDL_Event* event )
{
   Window *wdw; 
   Widget *wgt;
   SDLKey key;

   if (nwindows<=0) return 0;

   wdw = &windows[nwindows-1];
   wgt = (wdw->focus != -1) ? &wdw->widgets[ wdw->focus ] : NULL;
   key = event->key.keysym.sym;

   /* hack to simulate key repetition */
   if ((key==SDLK_BACKSPACE) || isalnum(key)) {
      if (event->type==SDL_KEYDOWN) toolkit_regKey(key);
      else if (event->type==SDL_KEYUP) toolkit_unregKey(key);
   }   

   /* handle input widgets */
   if (wgt && (wgt->type==WIDGET_INPUT)) /* grabs all the events it wants */
      if (toolkit_inputInput( event->type, wgt, key)) return 1;

   switch (key) {
      case SDLK_TAB:
         if (event->type == SDL_KEYDOWN)
            toolkit_nextFocus();
         return 1;

      case SDLK_RETURN:
         if (event->type == SDL_KEYDOWN)
            toolkit_triggerFocus();
         return 1;

      case SDLK_ESCAPE:
         if (event->type == SDL_KEYDOWN)
            if (wdw->cancel_fptr != NULL) {
               (*wdw->cancel_fptr)(wdw->name);
               return 1;
            }
         return 0;

      case SDLK_UP:
         if (event->type == SDL_KEYDOWN) {
            toolkit_regKey(SDLK_UP);
            toolkit_listScroll( toolkit_getFocus(), +1 );
         }
         else if (event->type == SDL_KEYUP)
            toolkit_unregKey(SDLK_UP);
         return 0;

      case SDLK_DOWN:
         if (event->type == SDL_KEYDOWN) {
            toolkit_regKey(SDLK_DOWN);
            toolkit_listScroll( toolkit_getFocus(), -1 );
         }
         else if (event->type == SDL_KEYUP)
            toolkit_unregKey(SDLK_DOWN);
         return 0;

      default:
         return 0;
   }
}


/*
 * updates the toolkit input for repeating keys
 */
void toolkit_update (void)
{
   unsigned int t;
   Window *wdw;
   Widget *wgt;

   t = SDL_GetTicks();

   if (input_key == 0) return;

   if (input_keyTime + INPUT_DELAY + input_keyCounter*INPUT_FREQ > t)
      return;

   input_keyCounter++;

   if (nwindows > 0) {
      wdw = &windows[nwindows-1];
      wgt = (wdw->focus >= 0) ? &wdw->widgets[ wdw->focus ] : NULL;
      if (wgt && (wgt->type==WIDGET_INPUT) &&
            (input_key==SDLK_BACKSPACE || isalnum(input_key)))
         toolkit_inputInput( SDL_KEYDOWN, wgt, input_key );
   }

   switch (input_key) {

      case SDLK_UP:
         toolkit_listScroll( toolkit_getFocus(), +1 );
         break;
      case SDLK_DOWN:
         toolkit_listScroll( toolkit_getFocus(), -1 );
         break;

      default:
         break;
   }
}


/*
 * focus next widget
 */
static void toolkit_nextFocus (void)
{
   Window* wdw = &windows[nwindows-1]; /* get active window */

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


/*
 * returns 1 if the window is focusable
 */
static int toolkit_isFocusable( Widget *wgt )
{
   if (wgt==NULL) return 0;

   switch (wgt->type) {
      case WIDGET_BUTTON:
         if (wgt->dat.btn.disabled==1) return 0;
      case WIDGET_LIST:
      case WIDGET_INPUT:
      case WIDGET_IMAGEARRAY:
         return 1;

      default:
         return 0;
   }
}


/*
 * trigger the focused widget
 */
static void toolkit_triggerFocus (void)
{
   Window* wdw;
   Widget* wgt;

   wdw = &windows[nwindows-1];
   if (wdw->focus == -1) return;
   wgt = &wdw->widgets[wdw->focus];

   switch (wgt->type) {

      case WIDGET_BUTTON:
         if (wgt->dat.btn.fptr) (*wgt->dat.btn.fptr)(wgt->name);
         else DEBUG("Toolkit: Button '%s' of Window '%s' "
               "doesn't have a function trigger",
               wgt->name, wdw->name );
         break;

      default:
         if (wdw->accept_fptr) (*wdw->accept_fptr)(wgt->name);
         break;
   }
}


/*
 * tries to scroll up/down by direction
 */
static void toolkit_listScroll( Widget* wgt, int direction )
{
   double w,h;
   int xelem, yelem;
   double hmax;

   if (wgt == NULL) return;

   switch (wgt->type) {

      case WIDGET_LIST:
         wgt->dat.lst.selected -= direction;
         wgt->dat.lst.selected = MAX(0,wgt->dat.lst.selected);
         wgt->dat.lst.selected = MIN(wgt->dat.lst.selected, wgt->dat.lst.noptions-1);
         if (wgt->dat.lst.fptr) (*wgt->dat.lst.fptr)(wgt->name);
         break;

      case WIDGET_IMAGEARRAY:
         /* element dimensions */
         w = wgt->dat.iar.iw + 5.*2.; /* includes border */
         h = wgt->dat.iar.ih + 5.*2. + 2. + gl_smallFont.h;

         /* number of elements */
         xelem = (int)((wgt->w - 10.) / w);
         yelem = (int)wgt->dat.iar.nelements / xelem + 1;

         /* maximum */
         hmax = h * (yelem - (int)(wgt->h / h));
         if (hmax < 0.)
            hmax = 0.;
         
         /* move */
         wgt->dat.iar.pos -= direction * h;

         /* Boundry check. */
         wgt->dat.iar.pos = MAX(wgt->dat.iar.pos, 0.);
         wgt->dat.iar.pos = MIN(wgt->dat.iar.pos, hmax);
         if (wgt->dat.iar.fptr) (*wgt->dat.iar.fptr)(wgt->name);
         break;

      default:
         break;
   }
}


/**
 * @fn char* toolkit_getList( const unsigned int wid, char* name )
 *
 * @brief Gets what is selected currently in a list.
 *
 * List includes Image Arrays.
 */
char* toolkit_getList( const unsigned int wid, char* name )
{
   Widget *wgt = window_getwgt(wid,name);

   if (wgt == NULL) {
      WARN("Widget '%s' not found", name);
      return NULL;
   }

   switch (wgt->type) {
      case WIDGET_LIST:
         if (wgt->dat.lst.selected == -1)
            return NULL;
         return wgt->dat.lst.options[ wgt->dat.lst.selected ];

      case WIDGET_IMAGEARRAY:
         if (wgt->dat.iar.selected == -1)
            return NULL;
         return wgt->dat.iar.captions[ wgt->dat.iar.selected ];

      default:
         return NULL;
   }
}


/*
 * get the position of current item in the list
 */
int toolkit_getListPos( const unsigned int wid, char* name )
{
   Widget *wgt = window_getwgt(wid,name);

   if (wgt == NULL) {
      WARN("Widget '%s' not found", name);
      return -1;
   }

   if ((wgt->type != WIDGET_LIST) || (wgt->dat.lst.selected == -1))
      return -1;

   return wgt->dat.lst.selected;
}


/*
 * mouse event focus on list
 */
static void toolkit_listFocus( Widget* lst, double bx, double by )
{
   (void)bx;
   int i;

   i = (lst->h - by) / (gl_defFont.h + 2.);
   if (i < lst->dat.lst.noptions) { /* shouldn't be out of boundries */
      lst->dat.lst.selected = i;
      toolkit_listScroll( lst, 0 ); /* checks boundries and triggers callback */
   }
}

/**
 * @fn static void toolkit_imgarrFocus( Widget* iar, double bx, double by )
 *
 * @brief Mouse event focus on image array.
 *
 *    @param iar Image Array widget.
 *    @param bx X position click.
 *    @param by Y position click.
 */
static void toolkit_imgarrFocus( Widget* iar, double bx, double by )
{
   int i,j;
   double x,y, w,h, ycurs,xcurs;
   double scroll_pos, hmax;
   int xelem, xspace, yelem;

   /* positions */
   x = bx + iar->x;
   y = by + iar->y;

   /* element dimensions */
   w = iar->dat.iar.iw + 5.*2.; /* includes border */
   h = iar->dat.iar.ih + 5.*2. + 2. + gl_smallFont.h;

   /* number of elements */
   xelem = (int)((iar->w - 10.) / w);
   xspace = (((int)iar->w - 10) % (int)w) / (xelem + 1);
   yelem = (int)iar->dat.iar.nelements / xelem + 1;

   /* Normal click. */
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
               iar->dat.iar.selected = j*xelem + i;
               if (iar->dat.iar.fptr != NULL)
                  (*iar->dat.iar.fptr)(iar->name);
               return;
            }
            xcurs += xspace + w;
         }
         ycurs -= h;
      }
   }
   /* Scrollbar click. */
   else {
      /* Get bar position (center). */
      hmax = h * (yelem - (int)(iar->h / h));
      scroll_pos = iar->dat.iar.pos / hmax;
      y = iar->h - (iar->h - 30.) * scroll_pos - 15.;

      /* Click below the bar. */
      if (by < y-15.)
         toolkit_listScroll( iar, -2 );
      /* Click above the bar. */
      else if (by > y+15.)
         toolkit_listScroll( iar, +2 );
      /* Click on the bar. */
      else
         iar->status = WIDGET_STATUS_SCROLLING;
   }
}


/**
 * @fn static void toolkit_imgarrMove( Widget* iar, double ry )
 *
 * @brief Handles Image Array movement.
 *
 *    @param iar Image Array that has mouse movement.
 *    @param ry Relative Y mouse movement.
 */
static void toolkit_imgarrMove( Widget* iar, double ry )
{
   double w,h;
   int xelem, yelem;
   double hmax;

   if (iar->status == WIDGET_STATUS_SCROLLING) {

      /* element dimensions */
      w = iar->dat.iar.iw + 5.*2.; /* includes border */
      h = iar->dat.iar.ih + 5.*2. + 2. + gl_smallFont.h;

      /* number of elements */
      xelem = (int)((iar->w - 10.) / w);
      yelem = (int)iar->dat.iar.nelements / xelem + 1;

      hmax = h * (yelem - (int)(iar->h / h));

      iar->dat.iar.pos -= ry * hmax / (iar->h - 30.);

      /* Dous boundry checks. */
      toolkit_listScroll(iar, 0);
   }
}



/*
 * returns the focused widget
 */
static Widget* toolkit_getFocus (void)
{
   Window* wdw;
   wdw = &windows[nwindows-1];
   
   if (wdw->focus == -1) return NULL;

   return &wdw->widgets[wdw->focus];
}


/*
 * initializes the toolkit
 */
int toolkit_init (void)
{
   windows = malloc(sizeof(Window)*MIN_WINDOWS);
   nwindows = 0;
   mwindows = MIN_WINDOWS;
   SDL_ShowCursor(SDL_DISABLE);

   return 0;
}


/*
 * exits the toolkit
 */
void toolkit_exit (void)
{
   while (nwindows > 0)
      window_destroy(windows[0].id);
   free(windows);
}
