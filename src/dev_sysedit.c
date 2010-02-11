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


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


/*
 * Prototypes.
 */
/* Custom system editor widget. */
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

   /* Create the window. */
   wid = window_create( "System Editor", -1, -1, -1, -1 );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 60 - BUTTON_WIDTH, SCREEN_H - 60 - BUTTON_HEIGHT,
         "cstSysEdit", 1, sysedit_render, sysedit_mouse, NULL );
}


/**
 * @brief System editor custom widget rendering.
 */
static void sysedit_render( double bx, double by, double w, double h, void *data )
{
}

/**
 * @brief System editor custom widget mouse handling.
 */
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
}



