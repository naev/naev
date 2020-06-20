/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "log.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"
#include "mission.h"
#include "colour.h"
#include "player.h"
#include "faction.h"
#include "dialogue.h"
#include "gui.h"
#include "map_find.h"
#include "array.h"
#include "mapData.h"
#include "nstring.h"
#include "nmath.h"
#include "nmath.h"
#include "nxml.h"
#include "ndata.h"


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define MAP_LOOP_PROT   1000 /**< Number of iterations max in pathfinding before
                                 aborting. */

/* map decorator stack */
static MapDecorator* decorator_stack = NULL; /**< Contains all the map decorators. */
static int decorator_nstack       = 0; /**< Number of map decorators in the stack. */

static double map_zoom        = 1.; /**< Zoom of the map. */
static double map_xpos        = 0.; /**< Map X position. */
static double map_ypos        = 0.; /**< Map Y position. */
static int map_drag           = 0; /**< Is the user dragging the map? */
static int map_selected       = -1; /**< What system is selected on the map. */
static StarSystem **map_path  = NULL; /**< The path to current selected system. */
glTexture *gl_faction_disk    = NULL; /**< Texture of the disk representing factions. */
static int cur_commod         = -1; /**< Current commodity selected. */
/* VBO. */
//static gl_vbo *map_vbo = NULL; /**< Map VBO. */
//static gl_vbo *marker_vbo = NULL;

#define MAP_SYSTEM_WDWNAME "System map"
/*
 * extern
 */
/* space.c */
extern StarSystem *systems_stack;
extern int systems_nstack;
extern int faction_nstack;

/*land.c*/
extern int landed;
extern Planet* land_planet;

/*
 * prototypes
 */
/* Update. */
static void map_system_update( unsigned int wid );
/* Render. */
static void map_system_render( double bx, double by, double w, double h, void *data );
/* Mouse. */
static int map_system_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Misc. */
static glTexture *gl_genFactionDisk( int radius );
static int map_system_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod );
static void map_system_selectCur (void);


/**
 * @brief Initializes the map subsystem.
 *
 *    @return 0 on success.
 */
int map_system_init (void)
{
   const double beta = M_PI / 9;
   GLfloat vertex[6];

   /* Create the VBO. */
   /*
   map_system_vbo = gl_vboCreateStream( sizeof(GLfloat) * 3*(2+4), NULL );

   vertex[0] = 1;
   vertex[1] = 0;
   vertex[2] = 1 + 3 * cos(beta);
   vertex[3] = 3 * sin(beta);
   vertex[4] = 1 + 3 * cos(beta);
   vertex[5] = -3 * sin(beta);
   marker_vbo = gl_vboCreateStatic( sizeof(GLfloat) * 6, vertex );
   */
   gl_faction_disk = gl_genFactionDisk( 150. );
   
   return 0;
}

/**
 * @brief Loads all the map decorators.
 *
 *    @return 0 on success.
 */
int map_system_load (void)
{
  return 0;
}

/**
 * @brief Destroys the map subsystem.
 */
void map_system_exit (void)
{
   int i;

   /* Destroy the VBO. */
   /*if (map_system_vbo != NULL) {
      gl_vboDestroy(map_system_vbo);
      map_system_vbo = NULL;
      }*/

   if (gl_faction_disk != NULL)
      gl_freeTexture( gl_faction_disk );

   /*if (decorator_stack != NULL) {
      for (i=0; i<decorator_nstack; i++)
         gl_freeTexture( decorator_stack[i].image );
      free( decorator_stack );
      decorator_stack = NULL;
      decorator_nstack = 0;
      }*/
}


/**
 * @brief Handles key input to the map window.
 */
static int map_system_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod )
{
  /*   (void) mod;

   if ((key == SDLK_SLASH) || (key == SDLK_f)) {
      map_system_inputFind( wid, NULL );
      return 1;
      }*/

   return 0;
}


/**
 * @brief Opens the map window.
 */
void map_system_open (int sys_selected)
{
   unsigned int wid;
   StarSystem *cur;
   int w, h, x, y, rw;

   /* Destroy window if exists. */
   wid = window_get(MAP_SYSTEM_WDWNAME);
   if (wid > 0) {
      window_destroy( wid );
      return;
   }

   /* get the selected system. */
   cur = system_getIndex( sys_selected );

   /* Set up window size. */
   w = MAX(600, SCREEN_W - 120);
   h = MAX(540, SCREEN_H - 120);

   /* create the window. */
   wid = window_create( MAP_SYSTEM_WDWNAME, -1, -1, w, h );
   window_setCancel( wid, window_close );
   window_handleKeys( wid, map_system_keyHandler );

   window_addText( wid, -90 + 80, y, 160, 20, 1, "txtSysname",
         &gl_defFont, &cDConsole, cur->name );
   window_addImage( wid, -90 + 32, y - 32, 0, 0, "imgFaction", NULL, 0 );
   /* Close button */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnClose", _("Close"), window_close );
}

/**
 * @brief Checks to see if the map is open.
 *
 *    @return 0 if map is closed, non-zero if it's open.
 */
int map_system_isOpen (void)
{
   return window_exists(MAP_SYSTEM_WDWNAME);
}
/**
 * @brief Shows a solar system map at x, y (relative to wid) with size w,h.
 *
 *    @param wid Window to show map on.
 *    @param x X position to put map at.
 *    @param y Y position to put map at.
 *    @param w Width of map to open.
 *    @param h Height of map to open.
 *    @param zoom Default zoom to use.
 */
void map_system_show( int wid, int x, int y, int w, int h, double zoom )
{
   StarSystem *sys;

   /* Set zoom. */
   //map_system_setZoom(zoom);


   window_addCust( wid, x, y, w, h,
         "cstMapSys", 1, map_system_render, map_system_mouse, NULL );
}


/**
 * @brief Renders the custom solar system map widget.
 *
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void map_system_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;
   glColour col;
   StarSystem *sys;

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

}

/**
 * @brief Map custom widget mouse handling.
 *
 *    @param wid Window sending events.
 *    @param event Event window is sending.
 *    @param mx Mouse X position.
 *    @param my Mouse Y position.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static int map_system_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i;
   double x,y, t;
   StarSystem *sys;
   return 0;
}

/**
 * @brief Generates a texture to represent factions
 *
 * @param radius radius of the disk
 * @return the texture
 */
static glTexture *gl_genFactionDisk( int radius )
{
   int i, j;
   uint8_t *pixels;
   SDL_Surface *sur;
   int dist;
   double alpha;

   /* Calculate parameters. */
   const int w = 2 * radius + 1;
   const int h = 2 * radius + 1;

   /* Create the surface. */
   sur = SDL_CreateRGBSurface( 0, w, h, 32, RGBAMASK );

   pixels = sur->pixels;
   memset(pixels, 0xff, sizeof(uint8_t) * 4 * h * w);

   /* Generate the circle. */
   SDL_LockSurface( sur );

   /* Draw the circle with filter. */
   for (i=0; i<h; i++) {
      for (j=0; j<w; j++) {
         /* Calculate blur. */
         dist = (i - radius) * (i - radius) + (j - radius) * (j - radius);
         alpha = 0.;

         if (dist < radius * radius) {
            /* Computes alpha with an empirically chosen formula.
             * This formula accounts for the fact that the eyes
             * has a logarithmic sensitivity to light */
            alpha = 1. * dist / (radius * radius);
            alpha = (exp(1 / (alpha + 1) - 0.5) - 1) * 0xFF;
         }

         /* Sets the pixel alpha which is the forth byte
          * in the pixel representation. */
         pixels[i*sur->pitch + j*4 + 3] = (uint8_t)alpha;
      }
   }

   SDL_UnlockSurface( sur );

   /* Return texture. */
   return gl_loadImage( sur, OPENGL_TEX_MIPMAPS );
}
