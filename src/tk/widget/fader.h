/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"
#include "font.h"

/**
 * @brief The fader widget data.
 */
typedef struct WidgetFaderData_ {
   double value; /**< Current value. */
   double min;   /**< Minimum value. */
   double max;   /**< Maximum value. */
   void ( *fptr )(
      unsigned int,
      const char * ); /**< Modify callback - triggered on value change. */
   void ( *scrolldone )( unsigned int,
                         const char * ); /**< Scroll done callback. */
} WidgetFaderData;

/* Required functions. */
void window_addFader(
   unsigned int wid, const int x, const int y, /* position */
   const int   w,
   const int   h, /* size, if w > h fader is horizontal, else vertical */
   const char *name, const double min,
   const double max, /* name, minimum & maximum values */
   const double def, /* default pos. */
   void ( *call )( unsigned int,
                   const char * ) ); /* function to called on value change */

/* Misc functions. */
void   window_faderValue( unsigned int wid, const char *name, double value );
void   window_faderSetBoundedValue( unsigned int wid, const char *name,
                                    double value );
void   window_faderBounds( unsigned int wid, const char *name, double min,
                           double max );
double window_getFaderValue( unsigned int wid, const char *name );
void   window_faderScrollDone( unsigned int wid, const char *name,
                               void ( *func )( unsigned int, const char   *) );
