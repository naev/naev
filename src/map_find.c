/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map_find.h"

#include "naev.h"

#include "log.h"
#include "toolkit.h"
#include "map.h"
#include "dialogue.h"


#define MAP_WDWNAME     "Star Map" /**< Map window name. */

#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


/**
 * @brief Opens a search input box to find a system or planet.
 */
void map_inputFind( unsigned int wid, char* str )
{
   (void) wid;
   (void) str;
   char *name;
   const char *sysname;
   const char *realname;
   StarSystem *sys;

   name = dialogue_inputRaw( "Find...", 0, 32, "What do you want to find? (systems, planets)" );
   if (name == NULL || !strcmp("",name))
      return;

   /* Match system first. */
   sysname  = NULL;
   realname = system_existsCase( name );
   if (realname != NULL)
      sysname = realname;
   else {
      realname = planet_existsCase( name );
      if (realname != NULL)
         sysname = planet_getSystem( realname );
   }
   if (sysname != NULL) {
      sys = system_get(sysname);
      if (sys_isKnown(sys)) {
         map_select( sys, 0 );
         map_center( sysname );
         free(name);
         return;
      }
   }

   dialogue_alert( "System/Planet matching '%s' not found!", name );
   free(name);
   return;
}

