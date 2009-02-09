/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef RECT_H
#  define RECT_H


#include "colour.h"


/**
 * @brief The rectangle widget data.
 */
typedef struct WidgetRectData_{ /* WIDGET_RECT */
   glColour* colour; /**< Background colour. */
   int border; /**< 1 if widget should have border, 0 if it shouldn't. */
} WidgetRectData;


/* Required functions. */
void window_addRect( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, glColour* colour, int border ); /* properties */


#endif /* RECT_H */

