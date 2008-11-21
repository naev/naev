/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file menu.h
 *
 * @brief Handles the important game menus.
 */


#include "options.h"

#include <string.h>

#include "SDL.h"

#include "log.h"
#include "naev.h"
#include "input.h"
#include "toolkit.h"


#define KEYBINDS_WIDTH  400 /**< Options menu width. */
#define KEYBINDS_HEIGHT 300 /**< Options menu height. */

#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */


/*
 * External stuff.
 */
extern const char *keybindNames[];


/*
 * prototypes
 */
static void menuKeybinds_update( unsigned int wid, char *name );


/**
 * @brief Opens the keybindings menu.
 */
void opt_menuKeybinds (void)
{
   int i, j;
   unsigned int wid;
   char **str;

   /* Create the window. */
   wid = window_create( "Keybindings", -1, -1, KEYBINDS_WIDTH, KEYBINDS_HEIGHT );
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Text stuff. */
   window_addText( wid, 200, -40, KEYBINDS_WIDTH-220, 30, 1, "txtName",
         NULL, &cDConsole, NULL );
   window_addText( wid, 200, -90,
         KEYBINDS_WIDTH-220, KEYBINDS_HEIGHT-70-60-BUTTON_HEIGHT,
         0, "txtDesc", &gl_smallFont, NULL, NULL );

   /* Create the list. */
   for (i=0; strcmp(keybindNames[i],"end"); i++);
   str = malloc(sizeof(char*) * (i-1));
   for (j=0; j < i; j++)
      str[j] = strdup(keybindNames[j]);
   window_addList( wid, 20, -40, 160, KEYBINDS_HEIGHT-60, "lstKeybinds",
         str, i-1, 0, menuKeybinds_update );

   /* Update the list. */
   menuKeybinds_update( wid, NULL );
}


/**
 * @brief Updates the keybindings menu.
 */
static void menuKeybinds_update( unsigned int wid, char *name )
{
   (void) name;
   char *keybind;
   const char *desc;

   keybind = toolkit_getList( wid, "lstKeybinds" );
   window_modifyText( wid, "txtName", keybind );
   desc = input_getKeybindDescription( keybind );
   window_modifyText( wid, "txtDesc", (char*)desc );
}

