/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_CUST_H
#  define WGT_CUST_H


#include "SDL.h"


/**
 * @brief The custom widget data.
 */
typedef struct WidgetCustData_ {
   int border; /**< 1 if widget should have border, 0 if it shouldn't. */
   void (*render) (double bx, double by, double bw, double bh); /**< Function to run when rendering. */
   void (*mouse) (unsigned int wid, SDL_Event* event, double bx, double by, double bw, double bh); /**< Function to run when recieving mouse events. */
} WidgetCustData;


/* Required functions. */
void window_addCust( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int border,
      void (*render) (double x, double y, double w, double h),
      void (*mouse) (unsigned int wid, SDL_Event* event, double x, double y, double w, double h) );


#endif /* WGT_CUST_H */

