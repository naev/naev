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
#include "nstd.h"


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
   WIDGET_IMAGEARRAY,
   WIDGET_FADER
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


typedef struct WidgetButtonData_ { /* WIDGET_BUTTON */
   void (*fptr) (unsigned int,char*); /**< Activate callback. */
   char *display; /**< Displayed text. */
   int disabled; /**< 1 if button is disabled, 0 if enabled. */
} WidgetButtonData; /**< WIDGET_BUTTON */

typedef struct WidgetTextData_ { /* WIDGET_TEXT */
   char *text; /**< Text to display, using printMid if centered, else printText. */
   glFont* font; /**< Text font. */
   glColour* colour; /**< Text colour. */
   int centered; /**< 1 if text is centered, 0 if it isn't. */
} WidgetTextData; /**< WIDGET_TEXT */

typedef struct WidgetImageData_{ /* WIDGET_IMAGE */
   glTexture* image; /**< Image to display. */
   glColour* colour; /**< Colour to warp to. */
   int border; /**< 1 if widget should have border. */
} WidgetImageData; /**< WIDGET_IMAGE */

typedef struct WidgetListData_ { /* WIDGET_LIST */
   char **options; /**< Pointer to the options. */
   int noptions; /**< Total number of options. */
   int selected; /**< Which option is currently selected. */
   int pos; /** Current topmost option (in view). */
   void (*fptr) (unsigned int,char*); /**< Modify callback - triggered on selection. */
   int height; /**< Real height. */
} WidgetListData; /**< WIDGET_LIST */

typedef struct WidgetRectData_{ /* WIDGET_RECT */
   glColour* colour; /**< Background colour. */
   int border; /**< 1 if widget should have border, 0 if it shouldn't. */
} WidgetRectData; /**< WIDGET_RECT */

typedef struct WidgetCustData_ { /* WIDGET_CUST */
   int border; /**< 1 if widget should have border, 0 if it shouldn't. */
   void (*render) (double bx, double by, double bw, double bh); /**< Function to run when rendering. */
   void (*mouse) (unsigned int wid, SDL_Event* event, double bx, double by); /**< Function to run when recieving mous events. */
} WidgetCustData; /**< WIDGET_CUST */

typedef struct WidgetInputData_ { /* WIDGET_INPUT */
   char *input; /**< Input buffer. */
   int max; /**< Maximum length. */
   int oneline; /**< Is it a one-liner? no '\n' and friends */
   int view; /**< View position. */
   int pos; /**< Cursor position. */
} WidgetInputData; /**< WIDGET_INPUT */

typedef struct WidgetImageArrayData_ { /* WIDGET_IMAGEARRAY */
   glTexture **images; /**< Image array. */
   char **captions; /**< Corresponding caption array. */
   int nelements; /**< Number of elements. */
   int selected; /**< Currently selected element. */
   double pos; /**< Current y position. */
   int iw; /**< Image width to use. */
   int ih; /**< Image height to use. */
   void (*fptr) (unsigned int,char*); /**< Modify callback - triggered on selection. */
} WidgetImageArrayData; /**< WIDGET_IMAGEARRAY */

typedef struct WidgetFaderData_{ /* WIDGET_FADER */
   double value; /**< Current value. */
   double min;   /**< Minimum value. */
   double max;   /**< Maximum value. */
   void (*fptr) (unsigned int,char*); /**< Modify callback - triggered on value change. */
} WidgetFaderData; /**< WIDGET_FADER */
/**
 * @struct Widget
 *
 * @brief Represents a widget.
 */
typedef struct Widget_ {
   /* Basic properties. */
   char* name; /**< Widget's name. */
   WidgetType type; /**< Widget's type. */

   /* Inheritance. */
   unsigned int wdw; /**< Widget's parent window. */

   /* Position and dimensions. */
   double x; /**< X position within the window. */
   double y; /**< Y position within the window. */
   double w; /**< Widget width. */
   double h; /**< Widget height. */

   /* Event abstraction. */
   void (*keyevent) ( SDLKey k, SDLMod m ); /**< Key event handler function for the widget. */
   void (*mouseevent) ( double x, double y, double rx, double ry ); /**< Mouse event handler function for the widget. */

   /* Misc. routines. */
   void (*render) ( struct Widget_ *wgt, double x, double y ); /**< Render function for the widget. */

   /* Status of the widget. */
   WidgetStatus status; /**< Widget status. */

   /* Type specific data (defined by type). */
   union {
      WidgetButtonData btn; /**< WIDGET_BUTTON */
      WidgetTextData txt; /**< WIDGET_TEXT */
      WidgetImageData img; /**< WIDGET_IMAGE */
      WidgetListData lst; /**< WIDGET_LIST */
      WidgetRectData rct; /**< WIDGET_RECT */
      WidgetCustData cst; /**< WIDGET_CUST */
      WidgetInputData inp; /**< WIDGET_INPUT */
      WidgetImageArrayData iar; /**< WIDGET_IMAGEARRAY */
      WidgetFaderData fad; /**< WIDGET_FADER */
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

   void (*accept_fptr)(unsigned int,char*); /**< Triggered by hitting 'enter' with no widget that catches the keypress. */
   void (*cancel_fptr)(unsigned int,char*); /**< Triggered by hitting 'escape' with no widget that catches the keypress. */

   /* Position and dimensions. */
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
static void toolkit_listMove( Widget* lst, double ay );
static void toolkit_imgarrMove( Widget* iar, double ry );
static void toolkit_clearKey (void);
/* focus */
static void toolkit_nextFocus (void);
static int toolkit_isFocusable( Widget *wgt );
static void toolkit_triggerFocus (void);
static Widget* toolkit_getFocus (void);
static void toolkit_listScroll( Widget* wgt, int direction );
static void toolkit_listFocus( Widget* lst, double bx, double by );
static void toolkit_imgarrFocus( Widget* iar, double bx, double by );
static void toolkit_faderSetValue(Widget *fad, double value);
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
static void toolkit_renderFader( Widget* fad, double bx, double by );
static void toolkit_drawOutline( double x, double y,
      double w, double h, double b,
      glColour* c, glColour* lc );
static void toolkit_drawScrollbar( double x, double y, double w, double h, double pos );
static void toolkit_clip( double x, double y, double w, double h );
static void toolkit_unclip (void);
static void toolkit_drawRect( double x, double y,
      double w, double h, glColour* c, glColour* lc );


/**
 * @brief Sets the internal widget position.
 *
 *    @param wdw Window to which the widget belongs.
 *    @param wgt Widget to set position of.
 *    @param x X position to use.
 *    @param y Y position to use.
 */
static void toolkit_setPos( Window *wdw, Widget *wgt, int x, int y )
{
   if (x < 0) wgt->x = wdw->w - wgt->w + x;
   else wgt->x = (double) x;
   if (y < 0) wgt->y = wdw->h - wgt->h + y;
   else wgt->y = (double) y;
}


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
 *    @param display Text displayed on the button (centered).
 *    @param call Function to call when button is pressed. Parameter passed
 *                is the name of the button.
 */
void window_addButton( const unsigned int wid,
                       const int x, const int y,
                       const int w, const int h,
                       char* name, char* display,
                       void (*call) (unsigned int wgt, char* wdwname) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_BUTTON;
   wgt->name = strdup(name);
   wgt->wdw = wid;
   
   /* specific */
   wgt->render = toolkit_renderButton;
   wgt->dat.btn.display = strdup(display);
   wgt->dat.btn.disabled = 0; /* initially enabled */
   wgt->dat.btn.fptr = call;
   if (wgt->dat.btn.fptr == NULL) /* Disable if function is NULL. */
      wgt->dat.btn.disabled = 1;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/**
 * @brief Adds a text widget to the window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Maximum width of the text.
 *    @param h Maximum height of the text.
 *    @param centered Whether text should be centered.
 *    @param name Name of the widget to use internally.
 *    @param font Font to use (NULL is default).
 *    @param colour Colour to use (NULL is default).
 *    @param string Text to display.
 */
void window_addText( const unsigned int wid,
                     const int x, const int y,
                     const int w, const int h,
                     const int centered, char* name,
                     glFont* font, glColour* colour, const char* string )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_TEXT;
   wgt->name = strdup(name);
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderText;
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


/**
 * @brief Adds an image widget to the window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param name Name of the widget to use internally.
 *    @param image Image to use.
 *    @param border Whether to use a border.
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
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderImage;
   wgt->dat.img.image = image;
   wgt->dat.img.border = border;
   wgt->dat.img.colour = NULL; /* normal colour */

   /* position/size */
   wgt->w = (image==NULL) ? 0 : wgt->dat.img.image->sw;
   wgt->h = (image==NULL) ? 0 : wgt->dat.img.image->sh;
   toolkit_setPos( wdw, wgt, x, y );
}


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
   wgt->type = WIDGET_LIST;
   wgt->name = strdup(name);
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderList;
   wgt->dat.lst.options = items;
   wgt->dat.lst.noptions = nitems;
   wgt->dat.lst.selected = defitem; /* -1 would be none */
   wgt->dat.lst.pos = 0;
   wgt->dat.lst.fptr = call;
   
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
 * @brief Adds a rectangle widget to a window.
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
 *    @param colour Colour of the rectangle.
 *    @param border Whether or not it should have a border.
 */
void window_addRect( const unsigned int wid,
                     const int x, const int y, /* position */
                     const int w, const int h, /* size */
                     char* name, glColour* colour, int border )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_RECT;
   wgt->name = strdup(name);
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderRect;
   wgt->dat.rct.colour = colour;
   wgt->dat.rct.border = border;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Adds a custom widget to a window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 * You are in charge of the rendering and handling mouse input for this widget.
 *  Mouse events outside the widget position won't be passed on.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 *    @param name Name of the widget to use internally.
 *    @param border Whether or not it should have a border.
 *    @param render Render function, passes the position and dimensions of the
 *                  widget as parameters.
 *    @param mouse Mouse function, passes the window id, event and position as
 *                 parameters.
 */
void window_addCust( const unsigned int wid,
                     const int x, const int y, /* position */
                     const int w, const int h, /* size */
                     char* name, const int border,
                     void (*render) (double x, double y, double w, double h),
                     void (*mouse) (unsigned int wid, SDL_Event* event,
                                    double x, double y) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_CUST;
   wgt->name = strdup(name);
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderCust;
   wgt->dat.cst.border = border;
   wgt->dat.cst.render = render;
   wgt->dat.cst.mouse = mouse;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Adds an input widget to a window.
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
 *    @param max Max amount of characters that can be written.
 *    @param oneline Whether widget should only be one line long.
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
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderInput;
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
   wgt->type = WIDGET_IMAGEARRAY;
   wgt->name = strdup(name);
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderImageArray;
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


/**
 * @brief Adds a Fader widget.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 * If width is bigger the height it's horizontal, otherwise it's vertical.
 *
 *    @param wid Window to add to.
 *    @param x X position.
 *    @param y Y position.
 *    @param w Width.
 *    @param h Height.
 *    @param name Internal widget name.
 *    @param min Minimum value.
 *    @param max Maximum value.
 *    @param def Default value.
 *    @param call Callback when fader is modified.
 */
void window_addFader( const unsigned int wid,
                      const int x, const int y, /* position */
                      const int w, const int h, /* size */
                      char* name, const double min, const double max,
                      const double def,
                      void (*call) (unsigned int,char*) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_FADER;
   wgt->name = strdup(name);
   wgt->wdw = wid;

   /* specific */
   wgt->render = toolkit_renderFader;
   wgt->dat.fad.value = min;
   wgt->dat.fad.min = min;
   wgt->dat.fad.max = max;
   wgt->dat.fad.value = MAX(min,MIN(max,def));
   wgt->dat.fad.fptr = call;
   
   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/**
 * @brief Allocates room for a new widget.
 *
 *    @param w Window to create widget in.
 *    @return Newly allocated widget.
 */
static Widget* window_newWidget( Window* w )
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
static Window* window_wget( const unsigned int wid )
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
static Widget* window_getwgt( const unsigned int wid, char* name )
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
 * @brief Modifies an existing text widget.
 *
 *    @param wid Window to which the text widget belongs.
 *    @param name Name of the text widget.
 *    @param newstring String to set for the text widget.
 */
void window_modifyText( const unsigned int wid,
      char* name, char* newstring )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check type. */
   if (wgt->type != WIDGET_TEXT) {
      WARN("Not modifying text on non-text widget '%s'.", name);
      return;
   }

   /* Set text. */
   if (wgt->dat.txt.text)
      free(wgt->dat.txt.text);
   wgt->dat.txt.text = (newstring) ?  strdup(newstring) : NULL;
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
 * @brief Disables a button.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to disable.
 */
void window_disableButton( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check type. */
   if (wgt->type != WIDGET_BUTTON) {
      DEBUG("Trying to disable a non-button widget '%s'", name);
      return;
   }

   /* Disable button. */
   wgt->dat.btn.disabled = 1;
}


/**
 * @brief Enables a button.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to enable.
 */
void window_enableButton( const unsigned int wid, char *name )
{
   Widget *wgt;
  
   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check type. */
   if (wgt->type != WIDGET_BUTTON) {
      DEBUG("Trying to enable a non-button widget '%s'", name);
      return;
   }

   /* Enable button. */
   wgt->dat.btn.disabled = 0;

}


/**
 * Modifies an existing image's image.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to modify image of.
 *    @param image New image to set.
 */
void window_modifyImage( const unsigned int wid,
      char* name, glTexture* image )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Not modifying image on non-image widget '%s'.", name);
      return;
   }

   /* Set the image. */
   wgt->dat.img.image = image;
}


/**
 * Modifies an existing image's colour.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget to modify image colour of.
 *    @param colour New colour to use.
 */
void window_imgColour( const unsigned int wid,
      char* name, glColour* colour )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Not modifying image on non-image widget '%s'.", name);
      return;
   }

   /* Set the colour. */
   wgt->dat.img.colour = colour;
}


/**
 * @brief Sets a fader widget's value.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 *    @para value Value to set fader to.
 */
void window_faderValue( const unsigned int wid,
      char* name, double value )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Not setting fader value on non-fader widget '%s'.", name);
      return;
   }

   /* Set fader value. */
   toolkit_faderSetValue(wgt, value);
}


/**
 * @brief Sets a fader widget's boundries.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 *    @param min Minimum fader value.
 *    @param max Maximum fader value.
 */
void window_faderBounds( const unsigned int wid,
      char* name, double min, double max )
{
   double value;
   Widget *wgt;
  
   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Not setting fader value on non-fader widget '%s'.", name);
      return;
   }

   /* Set the fader boundries. */
   wgt->dat.fad.min = min;
   wgt->dat.fad.max = max;
  
   /* Set the value. */
   value = MAX(MIN(value, wgt->dat.fad.max), wgt->dat.fad.min);
   toolkit_faderSetValue(wgt, value);
}


/**
 * @brief Gets the image from an image widget
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 */
glTexture* window_getImage( const unsigned int wid, char* name )
{
   Widget *wgt;
  
   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Check the type. */
   if (wgt->type != WIDGET_IMAGE) {
      WARN("Trying to get image from non-image widget '%s'.", name);
      return NULL;
   }

   /* Get the value. */
   return (wgt) ? wgt->dat.img.image : NULL;
}


/**
 * @brief Gets the input from an input widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 */
char* window_getInput( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Check the type. */
   if (wgt->type != WIDGET_INPUT) {
      WARN("Trying to get input from non-input widget '%s'.", name);
      return NULL;
   }

   /* Get the value. */
   return (wgt) ? wgt->dat.inp.input : NULL;
}

/**
 * @brief Gets value of fader widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 */
double window_getFaderValue( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return 0.;

   /* Check the type. */
   if (wgt->type != WIDGET_FADER) {
      WARN("Trying to get fader value from non-fader widget '%s'.", name);
      return 0.;
   }

   /* Return the value. */
   return (wgt) ? wgt->dat.fad.value : 0.;
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

   wdw->id = wid;
   wdw->name = strdup(name);

   /* Sane defaults. */
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
 * @ rief Sets the default accept function of the window.
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
static void widget_cleanup( Widget *widget )
{
   int i;

   /* General freeing. */
   if (widget->name)
      free(widget->name);

   /* Type specific. */
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


/**
 * @brief Sets up 2d clipping planes around a rectangle.
 *
 *    @param x X position of the rectangle.
 *    @param y Y position of the rectangle.
 *    @param w Width of the rectangle.
 *    @param h Height of the rectangle.
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
/**
 * @brief Clears the 2d clipping planes.
 */
static void toolkit_unclip (void)
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
   gl_printMid( &gl_defFont, w->w,
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
 * @brief Renders a button widget.
 *
 *    @param btn WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
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


/**
 * @brief Renders a text widget.
 *
 *    @param txt Text widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
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


/**
 * @brief Renders a image widget.
 *
 *    @param img Image widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
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


/**
 * @brief Renders a list widget.
 *
 *    @param lst List widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void toolkit_renderList( Widget* lst, double bx, double by )
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
 * @brief Renders a rectangle widget.
 *
 *    @param wct Rectangle widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
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


/**
 * @brief Renders a custom widget.
 *
 *    @param cst Custom widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
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


/**
 * @brief Renders a input widget.
 *
 *    @param inp Input widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
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
 * @brief Renders an image array.
 *
 *    @param iar Image array widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void toolkit_renderImageArray( Widget* iar, double bx, double by )
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
 * @brief Draws a scrollbar.
 *
 *    @param x X position of scrollbar.
 *    @param y Y position of scrollbar.
 *    @param w Width of the scrollbar.
 *    @param h Height of the scrollbar.
 *    @param pos Position at [0:1].
 */
static void toolkit_drawScrollbar( double x, double y, double w, double h, double pos )
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
 * @brief Renders a fader
 *
 *    @param fad WIDGET_FADER widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void toolkit_renderFader( Widget* fad, double bx, double by )
{
   double pos;
   double w,h;
   double tx,ty, tw,th;
   double kx,ky, kw,kh;

   /* Some values. */
   pos = fad->dat.fad.value / (fad->dat.fad.max - fad->dat.fad.min);
   w = fad->w;
   h = fad->h;

   /* Track. */
   tx = bx + fad->x + (h > w ? (w - 5.) / 2 : 0);
   ty = by + fad->y + (h < w ? (h - 5.) / 2 : 0);
   tw = (h < w ? w : 5.);
   th = (h > w ? h : 5.);
   toolkit_drawRect(tx, ty, tw , th, toolkit_colDark, toolkit_colDark);

   /* Knob. */
   kx = bx + fad->x + (h < w ? w * pos - 5. : 0);
   ky = by + fad->y + (h > w ? h * pos - 5. : 0);
   kw = (h < w ? 15. : w);
   kh = (h > w ? 15. : h);

   /* Draw. */
   toolkit_drawRect(kx, ky, kw , kh, toolkit_colDark, toolkit_colLight);
   toolkit_drawOutline(kx, ky, kw , kh, 1., &cBlack, toolkit_colDark);
}


/**
 * @brief Handles input for an input widget.
 *
 *    @param type Event type.
 *    @param inp Input widget to handle event.
 *    @param key Key being handled.
 */
static int toolkit_inputInput( Uint8 type, Widget* inp, SDLKey key )
{
   int n;
   SDLMod mods;

   /* Must be input widget. */
   if (inp->type != WIDGET_INPUT)
      return 0;

   /* We only actually handle keydowns. */
   if (type != SDL_KEYDOWN)
      return 0;

   /*
    * Handle arrow keys.
    * @todo finish implementing, no cursor makes it complicated to see where you are.
    */
#if 0
   if ((type == SDL_KEYDOWN) &&
         ((key == SDLK_LEFT) || (key == SDLK_RETURN))) {
      /* Move pointer. */
      if (key == SDLK_LEFT) {
         if (inp->dat.inp.pos > 0)
            inp->dat.inp.pos -= 1;
      }
      else if (key == SDLK_RIGHT) {
         if ((inp->dat.inp.pos < inp->dat.inp.max-1) &&
               (inp->dat.inp.input[inp->dat.inp.pos+1] != '\0'))
            inp->dat.inp.pos += 1;
      }

      return 1;
   }
#endif

   /* Only catch some keys. */
   if (!nstd_isalnum(key) && (key != SDLK_BACKSPACE) &&
         (key != SDLK_SPACE) && (key != SDLK_RETURN))
      return 0;

   mods = SDL_GetModState();
   if (inp->dat.inp.oneline) {

      /* backspace -> delete text */
      if ((key == SDLK_BACKSPACE) && (inp->dat.inp.pos > 0)) {
         inp->dat.inp.input[ --inp->dat.inp.pos ] = '\0';
         
         if (inp->dat.inp.view > 0) {
            n = gl_printWidth( &gl_smallFont,
                  inp->dat.inp.input + inp->dat.inp.view - 1 );
            if (n+10 < inp->w)
               inp->dat.inp.view--;
         }
      }

      /* in limits. */
      else if ((inp->dat.inp.pos < inp->dat.inp.max-1)) {
         
         if ((key==SDLK_RETURN) && !inp->dat.inp.oneline)
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = '\n';

         /* upper case characters */
         else if (nstd_isalpha(key) && (mods & (KMOD_LSHIFT | KMOD_RSHIFT)))
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = nstd_toupper(key);

         /* rest */
         else if (!nstd_iscntrl(key))
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


static int mouse_down = 0; /**< Records if mouse is down. */
/**
 * @brief Handles the mouse events.
 *
 *    @param event Mouse event to handle.
 */
static void toolkit_mouseEvent( SDL_Event* event )
{
   int i;
   double d;
   double x,y, rel_x,rel_y;
   Window *w;
   Widget *wgt, *wgt_func;

   /* set mouse button status */
   if (event->type==SDL_MOUSEBUTTONDOWN)
      mouse_down = 1;
   else if (event->type==SDL_MOUSEBUTTONUP)
      mouse_down = 0;

   /* absolute positions */
   if (event->type==SDL_MOUSEMOTION) {
      /* Convert to local screen coords. */
      x = (double)event->motion.x;
      y = (double)gl_screen.rh - (double)event->motion.y;
      /* Create relative events. */
      rel_x = x - last_x;
      rel_y = y - last_y;
      last_x = x;
      last_y = y;
   }
   else if ((event->type==SDL_MOUSEBUTTONDOWN) || (event->type==SDL_MOUSEBUTTONUP)) {
      x = (double)event->button.x;
      y = (double)gl_screen.rh - (double)event->button.y;
   }

   /* Handle possible window scaling. */
   x *= gl_screen.mxscale;
   y *= gl_screen.myscale;

   /* Get the window. */
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
            (*wgt->dat.cst.mouse)( w->id, event, x-wgt->x, y-wgt->y );
         else
            switch (event->type) {
               case SDL_MOUSEMOTION:
                  if (!mouse_down)
                     wgt->status = WIDGET_STATUS_MOUSEOVER;
                  else {
                     if (wgt->type == WIDGET_LIST)
                        toolkit_listMove( wgt, y-wgt->y );
                     if (wgt->type == WIDGET_IMAGEARRAY)
                        toolkit_imgarrMove( wgt, rel_y );
                     if (wgt->type == WIDGET_FADER) {
                        d = (wgt->w > wgt->h) ? rel_x / wgt->w : rel_y / wgt->h;
                        toolkit_faderSetValue(wgt, wgt->dat.fad.value + d);
                     }
                  }
                  break;

               case SDL_MOUSEBUTTONDOWN:
                  wgt->status = WIDGET_STATUS_MOUSEDOWN;

                  /* Handle mouse wheel. */
                  if (event->button.button == SDL_BUTTON_WHEELUP) {
                     toolkit_listScroll( wgt, +1 );
                     break;
                  }
                  else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
                     toolkit_listScroll( wgt, -1 );
                     break;
                  }

                  if (toolkit_isFocusable(wgt))
                     w->focus = i;

                  if (wgt->type == WIDGET_LIST)
                     toolkit_listFocus( wgt, x-wgt->x, y-wgt->y );
                  else if (wgt->type == WIDGET_IMAGEARRAY)
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
            (*wgt->dat.cst.mouse)( w->id, event, x-wgt->x, y-wgt->y );
      else if (!mouse_down)
         wgt->status = WIDGET_STATUS_NORMAL;
   }

   /* We trigger this at the end in case it destroys the window that is calling
    * this function.  Otherwise ugly segfaults appear. */
   if (wgt_func)
      (*wgt_func->dat.btn.fptr)(w->id, wgt_func->name);
}


/**
 * @brief Registers a key as down (for key repetition).
 *
 *    @param key Key to register as down.
 */
static void toolkit_regKey( SDLKey key )
{
   if ((input_key==0) && (input_keyTime==0)) {
      input_key = key;
      input_keyTime = SDL_GetTicks();
      input_keyCounter = 0;
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
      input_key = 0;
      input_keyTime = 0;
      input_keyCounter = 0;
   }
}
/**
 * @brief Clears the registered keys.
 */
static void toolkit_clearKey (void)
{
   input_key = 0;
   input_keyTime = 0;
   input_keyCounter = 0;
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

   /* Needs to have at least one window. */
   if (nwindows<=0)
      return 0;

   /* Get event and key. */
   wdw = &windows[nwindows-1];
   wgt = (wdw->focus != -1) ? &wdw->widgets[ wdw->focus ] : NULL;
   key = event->key.keysym.sym;

   /* hack to simulate key repetition */
   if ((key==SDLK_BACKSPACE) || nstd_isalnum(key)) {
      if (event->type == SDL_KEYDOWN)
         toolkit_regKey(key);
      else if (event->type == SDL_KEYUP)
         toolkit_unregKey(key);
   }

   /* handle input widgets */
   if (wgt && (wgt->type==WIDGET_INPUT)) /* grabs all the events it wants */
      if (toolkit_inputInput( event->type, wgt, key))
         return 1;

   /* Key specific. */
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
               (*wdw->cancel_fptr)(wdw->id,wdw->name);
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
      wdw = &windows[nwindows-1];
      wgt = (wdw->focus >= 0) ? &wdw->widgets[ wdw->focus ] : NULL;
      if (wgt && (wgt->type==WIDGET_INPUT))
         toolkit_inputInput( SDL_KEYDOWN, wgt, input_key );
   }

   /* Handle the press. */
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


/**
 * @brief Focus next widget.
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

   /* Type specific. */
   switch (wgt->type) {
      case WIDGET_BUTTON:
         if (wgt->dat.btn.disabled==1)
            return 0;
      case WIDGET_LIST:
      case WIDGET_INPUT:
      case WIDGET_IMAGEARRAY:
         return 1;

      default:
         return 0;
   }
}


/**
 * @brief Trigger the focused widget.
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
         if (wgt->dat.btn.fptr) (*wgt->dat.btn.fptr)(wdw->id,wgt->name);
         else DEBUG("Toolkit: Button '%s' of Window '%s' "
               "doesn't have a function trigger",
               wgt->name, wdw->name );
         break;

      default:
         if (wdw->accept_fptr) (*wdw->accept_fptr)(wdw->id,wgt->name);
         break;
   }
}


/**
 * @brief Tries to scroll a widget up/down by direction.
 *
 *    @param wgt Widget to scroll.
 *    @param direction Direction to scroll.  Positive is up, negative
 *           is down and absolute value is number of elements to scroll.
 */
static void toolkit_listScroll( Widget* wgt, int direction )
{
   double w,h;
   int xelem, yelem;
   double hmax;
   Window *wdw;
   int pos;

   if (wgt == NULL) return;

   wdw = &windows[nwindows-1]; /* get active window */

   switch (wgt->type) {

      case WIDGET_LIST:
         wgt->dat.lst.selected -= direction;

         /* boundry check. */
         wgt->dat.lst.selected = MAX(0,wgt->dat.lst.selected);
         wgt->dat.lst.selected = MIN(wgt->dat.lst.selected, wgt->dat.lst.noptions-1);

         /* see if we have to scroll. */
         pos = (wgt->dat.lst.selected - wgt->dat.lst.pos);
         if (pos < 0) {
            wgt->dat.lst.pos += pos;
            if (wgt->dat.lst.pos < 0)
               wgt->dat.lst.pos = 0;
         }
         else if (2 + (pos+1) * (gl_defFont.h + 2) > wgt->h) {
            wgt->dat.lst.pos += (2 + (pos+1) * (gl_defFont.h + 2) - wgt->h) / (gl_defFont.h + 2);
         }

         if (wgt->dat.lst.fptr)
            (*wgt->dat.lst.fptr)(wdw->id,wgt->name);
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
         if (wgt->dat.iar.fptr)
            (*wgt->dat.iar.fptr)(wdw->id,wgt->name);
         break;

      default:
         break;
   }
}


/** 
 * @brief Changes fader value
 *
 *    @param fad Fader to set value of.
 *    @param value Value to set fader to.
 */
static void toolkit_faderSetValue( Widget *fad, double value )
{
   /* Sanity check and value set. */
   fad->dat.fad.value = MAX(MIN(value, fad->dat.fad.max), fad->dat.fad.min);

   /* Run function if needed. */
   if (fad->dat.fad.fptr != NULL)
      (*fad->dat.fad.fptr)(fad->wdw, fad->name);
}


/**
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


/**
 * @brief Get the position of current item in the list.
 *
 *    @param wid Window identifier where the list is.
 *    @param name Name of the list.
 *    @return The position in the list or -1 on error.
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


/**
 * @brief Handles mouse event focus on a list widget.
 *
 *    @param lst List widget.
 *    @param bx Base X mouse click.
 *    @param by Base Y mouse click.
 */
static void toolkit_listFocus( Widget* lst, double bx, double by )
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
      if (i < lst->dat.lst.noptions) { /* shouldn't be out of boundries */
         lst->dat.lst.selected = i;
         toolkit_listScroll( lst, 0 ); /* checks boundries and triggers callback */
      }
   }
   else {
      /* Get bar position (center). */
      scroll_pos  = (double)(lst->dat.lst.pos * (2 + gl_defFont.h));
      scroll_pos /= (double)lst->dat.lst.height - lst->h;
      y = (lst->h - 30.) * (1.-scroll_pos) + 15.;

      /* Click below the bar. */
      if (by < y-15.)
         toolkit_listScroll( lst, -5 );
      /* Click above the bar. */
      else if (by > y+15.)
         toolkit_listScroll( lst, +5 );
      /* Click on the bar. */
      else
         lst->status = WIDGET_STATUS_SCROLLING;
   }
}

/**
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
   Window *wdw;

   wdw = &windows[nwindows-1]; /* get active window */

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
                  (*iar->dat.iar.fptr)(wdw->id, iar->name);
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
 * @brief Handles List movement.
 *
 *    @param lst List that has mouse movement.
 *    @param ay Absolute Y mouse movement.
 */
static void toolkit_listMove( Widget* lst, double ay )
{
   Window *wdw;
   int psel;
   double p;
   int h;

   if (lst->status == WIDGET_STATUS_SCROLLING) {
      h = lst->h / (2 + gl_defFont.h) - 1;

      /* Save previous position. */
      psel = lst->dat.lst.pos;

      /* Find absolute position. */
      p  = (lst->h - ay) / lst->h * (lst->dat.lst.height - lst->h);
      p /= (2 + gl_defFont.h);
      lst->dat.lst.pos = (int)round(p);

      /* Does boundry checks. */
      lst->dat.lst.selected = MAX(lst->dat.lst.selected, lst->dat.lst.pos);
      lst->dat.lst.selected = MIN(lst->dat.lst.selected, lst->dat.lst.pos+h);

      /* Run change if position changed. */
      if (lst->dat.lst.selected != psel)
         if (lst->dat.lst.fptr) {
            wdw = &windows[nwindows-1]; /* get active window */
            (*lst->dat.lst.fptr)(wdw->id,lst->name);
         }
   }
}


/**
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

      /* Does boundry checks. */
      toolkit_listScroll(iar, 0);
   }
}


/**
 * @brief Gets the focused widget.
 *
 *    @return The focused widget.
 */
static Widget* toolkit_getFocus (void)
{
   Window* wdw;
   wdw = &windows[nwindows-1];
   
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
