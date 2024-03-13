/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL.h"
/** @endcond */

/**
 * @brief The custom widget data.
 */
typedef struct WidgetCustData_ {
   int border; /**< 1 if widget should have border, 0 if it shouldn't. */
   void ( *render )( double bx, double by, double bw, double bh,
                     void *data ); /**< Function to run when rendering. */
   void ( *renderOverlay )(
      double bx, double by, double bw, double bh,
      void *data ); /**< Function to run when rendering overlay. */
   int ( *mouse )(
      unsigned int wid, const SDL_Event *event, double bx, double by, double bw,
      double bh, double rx, double ry,
      void *data ); /**< Function to run when receiving mouse events. */
   void ( *focusGain )( unsigned int wid,
                        const char  *wgtname ); /**< Get focus. */
   void ( *focusLose )( unsigned int wid,
                        const char  *wgtname ); /**< Lose focus. */
   int clip; /**< 1 if should clip with glScissors or the like, 0 otherwise. */
   void *userdata;
   int   autofree; /**< 1 if widget should free userdata upon cleanup, 0 if it
                      shouldn't. */
} WidgetCustData;

/* Required functions. */
void window_addCust(
   unsigned int wid, const int x, const int y, /* position */
   const int w, const int h,                   /* size */
   char *name, const int border,
   void ( *render )( double x, double y, double w, double h, void *data ),
   int ( *mouse )( unsigned int wid, const SDL_Event *event, double x, double y,
                   double w, double h, double rx, double ry, void *data ),
   void ( *focusGain )( unsigned int wid, const char *wgtname ),
   void ( *focusLose )( unsigned int wid, const char *wgtname ), void *data );

void  window_custSetClipping( unsigned int wid, const char *name, int clip );
void  window_custSetOverlay( unsigned int wid, const char *name,
                             void ( *renderOverlay )( double bx, double by,
                                                     double bw, double bh,
                                                     void *data ) );
void *window_custGetData( unsigned int wid, const char *name );
void  window_custAutoFreeData( unsigned int wid, const char *name );
void  window_custSetDynamic( unsigned int wid, const char *name, int dynamic );
