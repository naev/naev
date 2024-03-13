/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "font.h"

/**
 * @brief The text widget data.
 */
typedef struct WidgetTextData_ {
   char *
      text; /**< Text to display, using printMid if centered, else printText. */
   glFont  *font;     /**< Text font. */
   glColour colour;   /**< Text colour. */
   int      centered; /**< 1 if text is centered, 0 if it isn't. */
} WidgetTextData;

/* Required functions. */
void window_addText( unsigned int wid, const int x, const int y, /* position */
                     const int w, const int h,                   /* size */
                     const int   centered,
                     const char *name, /* text is centered? label name */
                     glFont *font, const glColour *colour,
                     const char *string ); /* font, colour and actual text */

/* Misc functions. */
void window_modifyText( unsigned int wid, const char *name,
                        const char *newstring );
int  window_getTextHeight( unsigned int wid, const char *name );
