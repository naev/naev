/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_sysedit.c
 *
 * @brief Handles the star system editor.
 */

#include "dev_sysedit.h"

#include "naev.h"

#include "SDL.h"

#include "space.h"
#include "toolkit.h"
#include "opengl.h"
#include "map.h"


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


extern int systems_nstack;


static double sysedit_xpos = 0.;
static double sysedit_ypos = 0.;
static double sysedit_zoom = 1.;
static int sysedit_drag    = 0;


/*
 * Prototypes.
 */
/* Custom system editor widget. */
static void sysedit_buttonZoom( unsigned int wid, char* str );
static void sysedit_render( double bx, double by, double w, double h, void *data );
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );


/**
 * @brief Opens the system editor interface.
 */
void sysedit_open( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   unsigned int wid;

   /* Needed to generate faction disk. */
   map_setZoom( 1. );

   /* Create the window. */
   wid = window_create( "System Editor", -1, -1, -1, -1 );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", sysedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", sysedit_buttonZoom );


   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 60 - BUTTON_WIDTH, SCREEN_H - 80 - BUTTON_HEIGHT,
         "cstSysEdit", 1, sysedit_render, sysedit_mouse, NULL );
}


/**
 * @brief System editor custom widget rendering.
 */
static void sysedit_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;

   /* Parameters. */
   map_renderParams( bx, by, sysedit_xpos, sysedit_ypos, w, h, sysedit_zoom, &x, &y, &r );

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, w, h, r, 1 );

   /* Render system names. */
   map_renderNames( x, y, 1 );
}

/**
 * @brief System editor custom widget mouse handling.
 */
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i;
   double x,y, t;
   StarSystem *sys;

   t = 15.*15.; /* threshold */

   switch (event->type) {
      
      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return;

         /* Zooming */
         if (event->button.button == SDL_BUTTON_WHEELUP)
            sysedit_buttonZoom( 0, "btnZoomIn" );
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            sysedit_buttonZoom( 0, "btnZoomOut" );

         /* selecting star system */
         else {
            mx -= w/2 - sysedit_xpos;
            my -= h/2 - sysedit_ypos;

            for (i=0; i<systems_nstack; i++) {
               sys = system_getIndex( i );

               /* must be reachable */
               if (!sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
                     && !space_sysReachable(sys))
                  continue;

               /* get position */
               x = sys->pos.x * sysedit_zoom;
               y = sys->pos.y * sysedit_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /*sysedit_select( sys );*/
                  break;
               }
            }
            sysedit_drag = 1;
         }
         break;

      case SDL_MOUSEBUTTONUP:
         if (sysedit_drag)
            sysedit_drag = 0;
         break;

      case SDL_MOUSEMOTION:
         if (sysedit_drag) {
            /* axis is inverted */
            sysedit_xpos -= event->motion.xrel;
            sysedit_ypos += event->motion.yrel;
         }
         break;
   }
}


/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void sysedit_buttonZoom( unsigned int wid, char* str )
{
   (void) wid;

   /* Transform coords to normal. */
   sysedit_xpos /= sysedit_zoom;
   sysedit_ypos /= sysedit_zoom;

   /* Apply zoom. */
   if (strcmp(str,"btnZoomIn")==0) {
      sysedit_zoom += (sysedit_zoom >= 1.) ? 0.5 : 0.25;
      sysedit_zoom = MIN(2.5, sysedit_zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      sysedit_zoom -= (sysedit_zoom > 1.) ? 0.5 : 0.25;
      sysedit_zoom = MAX(0.5, sysedit_zoom);
   }

   /* Hack for the circles to work. */
   map_setZoom(sysedit_zoom);

   /* Transform coords back. */
   sysedit_xpos *= sysedit_zoom;
   sysedit_ypos *= sysedit_zoom;
}

