/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"

/**
 * @brief The rectangle widget data.
 */
typedef struct WidgetRectData_ {
   glColour colour; /**< Background colour. */
   int      fill;   /**< Whether or not rectangle is filled. */
   int      border; /**< 1 if widget should have border, 0 if it shouldn't. */
} WidgetRectData;

/* Required functions. */
void window_addRect( unsigned int wid, const int x, const int y, /* position */
                     const int w, const int h,                   /* size */
                     char *name, const glColour *colour,
                     int border ); /* properties */
