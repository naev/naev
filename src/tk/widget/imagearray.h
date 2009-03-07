/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_IMAGEARRAY_H
#  define WGT_IMAGEARRAY_H


#include "opengl.h"
#include "font.h"
#include "colour.h"


/**
 * @brief The image array widget data.
 */
typedef struct WidgetImageArrayData_ {
   glTexture **images; /**< Image array. */
   char **captions; /**< Corresponding caption array. */
   int nelements; /**< Number of elements. */
   int xelem; /**< Number of horizontal elements. */
   int yelem; /**< Number of vertical elements. */
   int selected; /**< Currently selected element. */
   double pos; /**< Current y position. */
   int iw; /**< Image width to use. */
   int ih; /**< Image height to use. */
   void (*fptr) (unsigned int,char*); /**< Modify callback - triggered on selection. */
} WidgetImageArrayData;


/* Required functions. */
void window_addImageArray( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int iw, const int ih, /* name and image sizes */
      glTexture** tex, char** caption, int nelem, /* elements */    
      void (*call) (unsigned int,char*) );

/* Misc functions. */
char* toolkit_getImageArray( const unsigned int wid, char* name );


#endif /* WGT_IMAGEARRAY_H */

