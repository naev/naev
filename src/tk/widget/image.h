/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_IMAGE_H
#  define WGT_IMAGE_H


#include "font.h"
#include "colour.h"


/**
 * @brief The image widget data
 */
typedef struct WidgetImageData_{
   glTexture* image; /**< Image to display. */
   glColour colour; /**< Colour to warp to. */
   int border; /**< 1 if widget should have border. */
   /* Additional layers can be set if needed. */
   glTexture** layers; /**< Layers to be added. */
   int nlayers; /**< Total number of layers. */
} WidgetImageData;


/* Required functions. */
void window_addImage( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* dimensions */
      char* name, glTexture* image, int border ); /* label and image itself */

/* Misc functions. */
void window_modifyImage( const unsigned int wid,
      char* name, glTexture* image, int w, int h );
void window_imgColour( const unsigned int wid,
      char* name, const glColour* colour );
glTexture* window_getImage( const unsigned int wid, char* name );
void window_modifyImageLayers( const unsigned int wid,
      char* name, glTexture** layers, int n );


#endif /* WGT_IMAGE_H */

