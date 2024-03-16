/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "glad.h"
/** @endcond */

/**
 * @brief represents a colour via its RGBA values.
 */
typedef struct glColour_ {
   GLfloat r; /**< Red value of the colour (0 to 1). */
   GLfloat g; /**< Green value of the colour (0 to 1). */
   GLfloat b; /**< Blue value of the colour (0 to 1). */
   GLfloat a; /**< Alpha value of the colour (0 to 1). */
} __attribute__( ( packed ) ) glColour;

/*
 * default colours
 */
#include "colours.gen.h"

#define COL_ALPHA( col, alpha )                                                \
   {                                                                           \
      .r = ( col ).r, .g = ( col ).g, .b = ( col ).b, .a = alpha               \
   }

/*
 * Colour space conversion routines.
 */
__attribute__( ( const ) ) double linearToGamma( double x );
__attribute__( ( const ) ) double gammaToLinear( double x );
void                              col_linearToGamma( glColour *c );
void                              col_gammaToLinear( glColour *c );
void col_hsv2rgb( glColour *c, float h, float s, float v );
void col_rgb2hsv( float *h, float *s, float *v, float r, float g, float b );
void col_blend( glColour *blend, const glColour *fg, const glColour *bg,
                float alpha );
