/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"
#include "font.h"
#include "opengl.h"

typedef struct ImageArrayCell_ {
   glTexture       *image;    /**< Image to display. */
   char            *caption;  /**< Corresponding caption. */
   char            *alt;      /**< Corresponding alt text. */
   int              quantity; /**< Corresponding quantity. */
   glColour         bg;       /**< Background colour. */
   char            *slottype; /**< Type of slot. */
   const glTexture *sloticon; /**< Icon for type of slot. */
   /* Additional layers can be set if needed. */
   glTexture **layers; /**< Layers to be added. */
} ImageArrayCell;

/**
 * @brief The image array widget data.
 */
typedef struct WidgetImageArrayData_ {
   ImageArrayCell *images;    /**< Image array. */
   int             nelements; /**< Number of elements. */
   int             xelem;     /**< Number of horizontal elements. */
   int             yelem;     /**< Number of vertical elements. */
   int             selected;  /**< Currently selected element. */
   int             alt;       /**< Currently displaying alt text. */
   int             altx;      /**< Alt x position. */
   int             alty;      /**< Alt y position. */
   double          pos;       /**< Current y position. */
   int             iwref;     /**< Reference image width to use. */
   int             ihref;     /**< Reference image height to use. */
   double          zoom;      /**< How zoomed in it is. */
   int             iw;        /**< Image width to use. */
   int             ih;        /**< Image height to use. */
   int             mx;        /**< Last mouse x position. */
   int             my;        /**< Last mouse y position. */
   void ( *fptr )(
      unsigned int,
      const char * ); /**< Modify callback - triggered on selection. */
   void ( *rmptr )( unsigned int, const char * ); /**< Right click callback. */
   void ( *dblptr )(
      unsigned int,
      const char * ); /**< Double click callback (for one selection). */
   void ( *accept )(
      unsigned int,
      const char * ); /**< Accept function pointer (when hitting enter). */
} WidgetImageArrayData;

/**
 * @brief Stores position and offset data for an image array.
 */
typedef struct iar_data_s {
   int    pos;    /**< Position (index) of the selected item. */
   double offset; /**< Scroll position of the image array. */
   double zoom;   /**< Current amount of zoom. */
} iar_data_t;

/* Required functions. */
void window_addImageArray(
   unsigned int wid, const int x, const int y,     /* position */
   const int w, const int h,                       /* size */
   const char *name, const int iw, const int ih,   /* name and image sizes */
   ImageArrayCell *img, int nelem,                 /* elements */
   void ( *call )( unsigned int, const char * ),   /* update callback */
   void ( *rmcall )( unsigned int, const char * ), /* right click callback */
   void ( *dblcall )( unsigned int,
                      const char * ) ); /* double click callback */

/* Misc functions. */
const char *toolkit_getImageArray( unsigned int wid, const char *name );
int         toolkit_setImageArray( unsigned int wid, const char *name,
                                   const char *elem );
int         toolkit_getImageArrayPos( unsigned int wid, const char *name );
int    toolkit_setImageArrayPos( unsigned int wid, const char *name, int pos );
double toolkit_getImageArrayOffset( unsigned int wid, const char *name );
int    toolkit_setImageArrayOffset( unsigned int wid, const char *name,
                                    double off );
double toolkit_getImageArrayZoom( unsigned int wid, const char *name );
int    toolkit_setImageArrayZoom( unsigned int wid, const char *name,
                                  double zoom );
int    toolkit_saveImageArrayData( unsigned int wid, const char *name,
                                   iar_data_t *iar_data );
int    toolkit_loadImageArrayData( unsigned int wid, const char *name,
                                   const iar_data_t *iar_data );
void   toolkit_initImageArrayData( iar_data_t *iar_data );
int    toolkit_unsetSelection( unsigned int wid, const char *name );
void   toolkit_setImageArrayAccept( unsigned int wid, const char *name,
                                    void ( *fptr )( unsigned int,
                                                  const char   *) );
int toolkit_getImageArrayVisibleElements( unsigned int wid, const char *name );
int toolkit_simImageArrayVisibleElements( int w, int h, int iw, int ih );
