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


/*
 * Prototypes.
 */
static void map_findClose( unsigned int wid, char* str );
static void map_findSearch( unsigned int wid, char* str );


/**
 * @brief Clsoses the find window.
 */
static void map_findClose( unsigned int wid, char* str )
{
   window_close( wid, str );
}


/**
 * @brief Does a search.
 */
static void map_findSearch( unsigned int wid, char* str )
{
   (void) str;
   char *name;
   const char *sysname;
   const char *realname;
   StarSystem *sys;

   /* Get the name. */
   name = window_getInput( wid, "inpSearch" );
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
         return;
      }
   }

   dialogue_alert( "System/Planet matching '%s' not found!", name );
   return;
}


/**
 * @brief Opens a search input box to find a system or planet.
 */
void map_inputFind( unsigned int parent, char* str )
{
   (void) str;
   unsigned int wid;
   int y;

   /* Create the window. */
   wid = window_create( "Find...", -1, -1, 600, 400 );
   window_setAccept( wid, map_findSearch );
   window_setCancel( wid, map_findClose );
   window_setParent( wid, parent );

   /* Text. */
   y = -20;
   window_addText( wid, 20, y, 100, gl_defFont.h+4, 0,
         "txtDescription", &gl_defFont, &cDConsole,
         "Enter keyword to search for:" );
   y -= 20;

   /* Create input. */
   window_addInput( wid, 40, y, 100, gl_defFont.h+4,
         "inpSearch", 32, 1, &gl_defFont );

   /* Create buttons. */
   window_addButton( wid, 160, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSearch", "Find", map_findSearch );
   y -= 20;

   /* Create check boxes. */
   window_addCheckbox( wid, 20, y, 100, 20,
         "chkSystem", "Systems", NULL, 1 );
   y -= 20;
   window_addCheckbox( wid, 20, y, 100, 20,
         "chkPlanet", "Planets", NULL, 1 );
   y -= 20;
}

