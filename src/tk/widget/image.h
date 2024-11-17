/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"
#include "font.h"

/**
 * @brief The image widget data
 */
typedef struct WidgetImageData_ {
   glTexture *image;  /**< Image to display. */
   glColour   colour; /**< Colour to warp to. */
   int        border; /**< 1 if widget should have border. */
   /* Additional layers can be set if needed. */
   glTexture **layers;  /**< Layers to be added. */
   int         nlayers; /**< Total number of layers. */
} WidgetImageData;

/* Required functions. */
void window_addImage( unsigned int wid, const int x, const int y, /* position */
                      const int w, const int h, /* dimensions */
                      char *name, const glTexture *image,
                      int border ); /* label and image itself */

/* Misc functions. */
void window_modifyImage( unsigned int wid, char *name, const glTexture *image,
                         int w, int h );
void window_imgColour( unsigned int wid, char *name, const glColour *colour );
glTexture *window_getImage( unsigned int wid, char *name );
void window_modifyImageLayers( unsigned int wid, char *name, glTexture **layers,
                               int n );
