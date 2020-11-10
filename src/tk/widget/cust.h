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
   void (*render) (double bx, double by, double bw, double bh, void* data); /**< Function to run when rendering. */
   void (*renderOverlay) (double bx, double by, double bw, double bh, void* data); /**< Function to run when rendering overlay. */
   int (*mouse) (unsigned int wid, SDL_Event* event, double bx, double by, double bw, double bh, double rx, double ry, void* data); /**< Function to run when receiving mouse events. */
   int clip; /**< 1 if should clip with glScissors or the like, 0 otherwise. */
   void *userdata;
} WidgetCustData;


/* Required functions. */
void window_addCust( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int border,
      void (*render) (double x, double y, double w, double h, void* data),
      int (*mouse) (unsigned int wid, SDL_Event* event, double x, double y, double w, double h, double rx, double ry, void* data),
      void *data );


void window_custSetClipping( const unsigned int wid, const char *name, int clip );
void window_custSetOverlay( const unsigned int wid, const char *name,
      void (*renderOverlay) (double bx, double by, double bw, double bh, void* data) );
void *window_custGetData( const unsigned int wid, const char *name );


#endif /* WGT_CUST_H */

