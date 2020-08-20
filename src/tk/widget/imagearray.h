/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_IMAGEARRAY_H
#  define WGT_IMAGEARRAY_H


#include "opengl.h"
#include "font.h"
#include "colour.h"

typedef struct ImageArrayCell_ {
   glTexture* image; /**< Image to display. */
   char *caption; /**< Corresponding caption. */
   char *alt; /**< Corresponding alt text. */
   int quantity; /**< Corresponding quantity. */
   glColour bg; /**< Background colour. */
   char *slottype; /**< Type of slot. */
   /* Additional layers can be set if needed. */
   glTexture** layers; /**< Layers to be added. */
   int nlayers; /**< Total number of layers. */
} ImageArrayCell;


/**
 * @brief The image array widget data.
 */
typedef struct WidgetImageArrayData_ {
   ImageArrayCell *images; /**< Image array. */
   int nelements; /**< Number of elements. */
   int xelem; /**< Number of horizontal elements. */
   int yelem; /**< Number of vertical elements. */
   int selected; /**< Currently selected element. */
   int alt; /**< Currently displaying alt text. */
   int altx; /**< Alt x position. */
   int alty; /**< Alt y position. */
   double pos; /**< Current y position. */
   int iw; /**< Image width to use. */
   int ih; /**< Image height to use. */
   void (*fptr) (unsigned int,char*); /**< Modify callback - triggered on selection. */
   void (*rmptr) (unsigned int,char*); /**< Right click callback. */
} WidgetImageArrayData;


/**
 * @brief Stores position and offset data for an image array.
 */
typedef struct iar_data_s {
   int pos;        /**< Position (index) of the selected item. */
   double offset;  /**< Scroll position of the image array. */
} iar_data_t;


/* Required functions. */
void window_addImageArray( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int iw, const int ih, /* name and image sizes */
      ImageArrayCell *img, int nelem, /* elements */
      void (*call) (unsigned int,char*), /* update callback */
      void (*rmcall) (unsigned int,char*) ); /* right click callback */

/* Misc functions. */
char* toolkit_getImageArray( const unsigned int wid, const char* name );
int toolkit_setImageArray( const unsigned int wid, const char* name, char* elem );
int toolkit_getImageArrayPos( const unsigned int wid, const char* name );
int toolkit_setImageArrayPos( const unsigned int wid, const char* name, int pos );
double toolkit_getImageArrayOffset( const unsigned int wid, const char* name );
int toolkit_setImageArrayOffset( const unsigned int wid, const char* name, double off );
int toolkit_saveImageArrayData( const unsigned int wid, const char *name,
      iar_data_t *iar_data );
int toolkit_unsetSelection( const unsigned int wid, const char *name );


#endif /* WGT_IMAGEARRAY_H */

