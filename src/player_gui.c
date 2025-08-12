/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player_gui.c
 *
 * @brief Handles the GUIs the player owns.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "SDL_PhysFS.h"
#include <SDL3/SDL_iostream.h>
#include <stdlib.h>

#include "player_gui.h"

#include "array.h"
#include "log.h"
#ifdef DEBUGGING
#include "ndata.h"
#endif /* DEBUGGING */

static char **gui_list = NULL; /**< List of GUIs the player has. */

/**
 * @brief Cleans up the player's GUI list.
 */
void player_guiCleanup( void )
{
   for ( int i = 0; i < array_size( gui_list ); i++ )
      free( gui_list[i] );
   array_free( gui_list );
   gui_list = NULL;
}

/**
 * @brief Adds a gui to the player.
 */
int player_guiAdd( const char *name )
{
   char **new;

   /* Name must not be NULL. */
   if ( name == NULL )
      return -1;

   /* Create new array. */
   if ( gui_list == NULL )
      gui_list = array_create( char * );

   /* Check if already exists. */
   if ( player_guiCheck( name ) )
      return 1;

#ifdef DEBUGGING
   /* Make sure the GUI is vaild. */
   SDL_IOStream *rw;
   char          buf[PATH_MAX];
   snprintf( buf, sizeof( buf ), GUI_PATH "%s.lua", name );
   rw = SDL_PhysFS_IOFromFile( buf );
   if ( rw == NULL ) {
      WARN( _( "GUI '%s' does not exist as a file: '%s' not found." ), name,
            buf );
      return -1;
   }
   SDL_CloseIO( rw );
#endif /* DEBUGGING */

   /* Add. */
   new    = &array_grow( &gui_list );
   new[0] = strdup( name );
   return 0;
}

/**
 * @brief Removes a player GUI.
 */
void player_guiRm( const char *name )
{
   (void)name;
   if ( gui_list == NULL )
      return;
}

/**
 * @brief Check if player has a GUI.
 */
int player_guiCheck( const char *name )
{
   if ( name == NULL )
      return 0;

   for ( int i = 0; i < array_size( gui_list ); i++ )
      if ( strcmp( gui_list[i], name ) == 0 )
         return 1;

   return 0;
}

/**
 * @brief Gets the list of GUIs.
 */
const char **player_guiList( void )
{
   return (const char **)gui_list;
}
