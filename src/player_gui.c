/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file player_gui.c
 *
 * @brief Handles the GUIs the player owns.
 */


#include "player_gui.h"

#include "naev.h"

#include "log.h"
#include "array.h"
#ifdef DEBUGGING
#include "ndata.h"
#endif /* DEBUGGING */
#include "nstring.h"


static char** gui_list = NULL; /**< List of GUIs the player has. */



/**
 * @brief Cleans up the player's GUI list.
 */
void player_guiCleanup (void)
{
   int i;
   if (gui_list != NULL) {
      for (i=0; i<array_size(gui_list); i++)
         free( gui_list[i] );
      array_free( gui_list );
   }
   gui_list = NULL;
}


/**
 * @brief Adds a gui to the player.
 */
int player_guiAdd( char* name )
{
   char **new;

   /* Name must not be NULL. */
   if (name == NULL)
      return -1;

   /* Create new array. */
   if (gui_list == NULL)
      gui_list = array_create( char* );

   /* Check if already exists. */
   if (player_guiCheck(name))
      return 1;

#ifdef DEBUGGING
   /* Make sure the GUI is vaild. */
   SDL_RWops *rw;
   char buf[PATH_MAX];
   nsnprintf( buf, sizeof(buf), "dat/gui/%s.lua", name );
   rw = ndata_rwops( buf );
   if (rw == NULL) {
      WARN("GUI '%s' does not exist as a file: '%s' not found.", name, buf );
      return -1;
   }
   SDL_RWclose(rw);
#endif /* DEBUGGING */

   /* Add. */
   new      = &array_grow( &gui_list );
   new[0]   = strdup(name);
   return 0;
}


/**
 * @brief Removes a player GUI.
 */
void player_guiRm( char* name )
{
   (void) name;
   if (gui_list == NULL)
      return;
}


/**
 * @brief Check if player has a GUI.
 */
int player_guiCheck( char* name )
{
   int i;

   if (gui_list == NULL)
      return 0;

   if (name == NULL)
      return 0;

   for (i=0; i<array_size(gui_list); i++)
      if (strcmp(gui_list[i], name)==0)
         return 1;

   return 0;
}


/**
 * @brief Gets the list of GUIs.
 */
char** player_guiList( int *n )
{
   if (gui_list == NULL) {
      *n = 0;
      return NULL;
   }

   *n = array_size(gui_list);
   return gui_list;
}

