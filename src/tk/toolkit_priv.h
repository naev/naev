/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef TOOLKIT_PRIV_H
#  define TOOLKIT_PRIV_H


#include "naev.h"
#include "log.h"

#include "tk/widget.h"


/*
 * Colours to use.
 */
extern glColour* toolkit_colLight;
extern glColour* toolkit_col;
extern glColour* toolkit_colDark;


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
   WIDGET_FADER,
   WIDGET_TABBEDWINDOW
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


#define WGT_FLAG_CANFOCUS     (1<<0)   /**< Widget can get focus. */
#define WGT_FLAG_RAWINPUT     (1<<1)   /**< Widget should always get raw input. */
#define wgt_setFlag(w,f)      ((w)->flags |= (f)) /**< Sets a widget flag. */
#define wgt_rmFlag(w,f)       ((w)->flags &= ~(f)) /**< Removes a widget flag. */
#define wgt_isFlag(w,f)       ((w)->flags & (f)) /**< Checks if a widget has a fla.g */


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

   unsigned int flags; /**< Widget flags. */

   /* Event abstraction. */
   int (*keyevent) ( struct Widget_ *wgt, SDLKey k, SDLMod m ); /**< Key event handler function for the widget. */
   int (*textevent) ( struct Widget_ *wgt, const char *text ); /**< Text event function handler for the widget. */
   int (*mmoveevent) ( struct Widget_ *wgt, int x, int y, int rx, int ry); /**< Mouse movement handler function for the widget. */
   int (*mclickevent) ( struct Widget_ *wgt, int button, int x, int y ); /**< Mouse click event handler function for the widget. */
   int (*rawevent) ( struct Widget_ *wgt, SDL_Event *event ); /**< Raw event handler function for widget. */

   /* Misc. routines. */
   void (*render) ( struct Widget_ *wgt, double x, double y ); /**< Render function for the widget. */
   void (*renderOverlay) ( struct Widget_ *wgt, double x, double y ); /**< Overlay render fuction for the widget. */
   void (*cleanup) ( struct Widget_ *wgt ); /**< Clean up function for the widget. */

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
      WidgetTabbedWindowData tab; /**< WIDGET_TABBEDWINDOW */
   } dat; /**< Stores the widget specific data. */
} Widget;


#define WINDOW_NOFOCUS     (1<<0) /**< Window can not be active window. */
#define WINDOW_NOINPUT     (1<<1) /**< Window recieves no input. */
#define WINDOW_NORENDER    (1<<2) /**< Window does not render even if it should. */
#define WINDOW_NOBORDER    (1<<3) /**< Window does not need border. */
#define window_isFlag(w,f) ((w)->flags & (f)) /**< Checks a window flag. */
#define window_setFlag(w,f) ((w)->flags |= (f)) /**< Sets a window flag. */
#define window_rmFlag(w,f) ((w)->flags &= ~(f)) /**< Removes a window flag. */


/**
 * @struct Window                                                   
 *                                                                  
 * @brief Represents a graphical window.
 */
typedef struct Window_ {
   unsigned int id; /**< Unique ID. */
   char *name; /**< Window name - should be unique. */
   unsigned int flags; /**< Window flags. */

   unsigned int parent; /**< Parent window, will close if this one closes. */
   void (*close_fptr)(unsigned int,char*); /**< How to close the window. */

   void (*accept_fptr)(unsigned int,char*); /**< Triggered by hitting 'enter' with no widget that catches the keypress. */
   void (*cancel_fptr)(unsigned int,char*); /**< Triggered by hitting 'escape' with no widget that catches the keypress. */
   int (*keyevent)(unsigned int,SDLKey,SDLMod); /**< User defined custom key event handler. */

   /* Position and dimensions. */
   double x; /**< X position of the window. */
   double y; /**< Y position of the window. */
   double w; /**< Window width. */
   double h; /**< Window height. */

   int focus; /**< Current focused widget. */
   Widget *widgets; /**< Widget storage. */
   int nwidgets; /**< Total number of widgets. */
} Window;


/* Window stuff. */
Window* toolkit_getActiveWindow (void);
Window* window_wget( const unsigned int wid );
int toolkit_inputWindow( Window *wdw, SDL_Event *event );
void window_render( Window* w );
void window_renderOverlay( Window* w );


/* Widget stuff. */
Widget* window_newWidget( Window* w );
void widget_cleanup( Widget *widget );
Widget* window_getwgt( const unsigned int wid, const char* name );
void toolkit_setPos( Window *wdw, Widget *wgt, int x, int y );
void toolkit_nextFocus (void);


/* Render stuff. */
void toolkit_drawOutline( double x, double y,
      double w, double h, double b,
      glColour* c, glColour* lc );
void toolkit_drawScrollbar( double x, double y, double w, double h, double pos );
void toolkit_clip( double x, double y, double w, double h );
void toolkit_unclip (void);
void toolkit_drawRect( double x, double y,
      double w, double h, glColour* c, glColour* lc );


/* Input stuff. */
Uint8 toolkit_inputTranslateCoords( Window *w, SDL_Event *event,
      int *x, int *y, int *rx, int *ry, int convert );


#endif /* TOOLKIT_PRIV_H */

