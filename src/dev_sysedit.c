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

#include "space.h"


/**
 * @brief Opens the system editor interface.
 */
int sysedit_open( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   unsigned int wid;

   /* Create the window. */
   wid = window_create( "System Editor", -1, -1, -1, -1 );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   
}
