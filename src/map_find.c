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
 * @brief Represents a found target.
 */
typedef struct map_find_s {
   Planet *pnt;         /**< Planet available at. */
   StarSystem *sys;     /**< System available at. */
   char display[128];   /**< Name to display. */
   int jumps;           /**< Jumps to system. */
   double distance;     /**< Distance to system. */
} map_find_t;


/* Stored checkbox values. */
static int map_find_systems = 1; /**< Systems checkbox value. */
static int map_find_planets = 0; /**< Planets checkbox value. */
static int map_find_outfits = 0; /**< Outfits checkbox value. */
static int map_find_ships   = 0; /**< Ships checkbox value. */

/* Misc ugly globals. */
static map_find_t *map_found_cur    = NULL;  /**< Pointer to found stuff. */
static int map_found_ncur           = 0;     /**< Number of found stuff. */


/*
 * Prototypes.
 */
static void map_find_check_update( unsigned int wid, char *str );
static void map_findClose( unsigned int wid, char* str );
static int map_sortCompare( const void *p1, const void *p2 );
static void map_sortFound( map_find_t *found, int n );
static int map_findSearchSystems( unsigned int parent, const char *name );
static int map_findSearchPlanets( unsigned int parent, const char *name );
static int map_findSearchOutfits( unsigned int parent, const char *name );
static int map_findSearchShips( unsigned int parent, const char *name );
static void map_findSearch( unsigned int wid, char* str );


/**
 * @brief Updates the checkboxes.
 */
static void map_find_check_update( unsigned int wid, char* str )
{
   (void) str;
   map_find_systems ^= window_checkboxState( wid, "chkSystem" );
   map_find_planets ^= window_checkboxState( wid, "chkPlanet" );
   map_find_outfits ^= window_checkboxState( wid, "chkOutfit" );
   map_find_ships   ^= window_checkboxState( wid, "chkShip" );
   window_checkboxSet( wid, "chkSystem", map_find_systems );
   window_checkboxSet( wid, "chkPlanet", map_find_planets );
   window_checkboxSet( wid, "chkOutfit", map_find_outfits );
   window_checkboxSet( wid, "chkShip",   map_find_ships );
}


/**
 * @brief Clsoses the find window.
 */
static void map_findClose( unsigned int wid, char* str )
{
   window_close( wid, str );

   /* Clean up if necessary. */
   if (map_found_cur != NULL)
      free( map_found_cur );
   map_found_cur = NULL;
}


/**
 * @brief Goes to a found system to display it.
 */
static void map_findDisplayMark( unsigned int wid, char* str )
{
   int pos;
   StarSystem *sys;

   /* Get system. */
   pos = toolkit_getListPos( wid, "lstResult" );
   sys = map_found_cur[ pos ].sys;

   /* Select. */
   map_select( sys, 0 );
   map_center( sys->name );

   /* Close parent. */
   window_close( window_getParent(wid), str );
}


/**
 * @brief Displays the results of the find.
 */
static void map_findDisplayResult( unsigned int parent, map_find_t *found, int n )
{
   int i;
   unsigned int wid;
   char **ll;

   /* Globals. */
   map_found_cur  = found;
   map_found_ncur = n;

   /* Create window. */
   wid = window_create( "Search Results", -1, -1, 300, 220 );
   window_setParent( wid, parent );
   window_setAccept( wid, map_findDisplayMark );
   window_setCancel( wid, window_close );

   /* The list. */
   ll = malloc( sizeof(char*) * n );
   for (i=0; i<n; i++)
      ll[i] = strdup( found[i].display );
   window_addList( wid, 20, -40, 260, 110,
         "lstResult", ll, n, 0, NULL );

   /* Buttons. */
   window_addButton( wid, 20, 20, 110, BUTTON_HEIGHT,
         "btnSelect", "Select", map_findDisplayMark );
   window_addButton( wid, -20, 20, 110, BUTTON_HEIGHT,
         "btnClose", "Cancel", window_close );
}


/**
 * @brief qsort compare function for map finds.
 */
static int map_sortCompare( const void *p1, const void *p2 )
{
   map_find_t *f1, *f2;

   /* Convert pointer. */
   f1 = (map_find_t*) p1;
   f2 = (map_find_t*) p2;

   /* Compare jumps. */
   if (f1->jumps > f2->jumps)
      return +1;
   else if (f1->jumps < f2->jumps)
      return -1;

   /* Compare distance. */
   if (f1->distance > f2->distance)
      return +1;
   else if (f1->distance < f2->distance)
      return -1;

   /* If they're the same it doesn't matter. */
   return 0;
}


/**
 * @brief Sorts the map findings.
 */
static void map_sortFound( map_find_t *found, int n )
{
   qsort( found, n, sizeof(map_find_t), map_sortCompare );
}


/**
 * @brief Gets the distance.
 */
static int map_findDistance( StarSystem *sys, int *jumps, double *distance )
{
   int i;
   StarSystem **s;
   double d;

   /* Calculate jump path. */
   s = map_getJumpPath( jumps, cur_system->name, sys->name, 1, NULL );
   if (s==NULL)
      return -1;

   /* Calculate distance. */
   d = 0.;
   for (i=0; i<*jumps; i++) {
   }

   /* Cleanup. */
   free(s);

   *distance = d;
   return 0;
}


/**
 * @brief Searches for a system.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchSystems( unsigned int parent, const char *name )
{
   int i;
   const char *sysname;
   StarSystem *sys;
   char **names;
   int len, n, ret;
   map_find_t *found;

   /* Check exact match. */
   sysname = system_existsCase( name );
   if (sysname != NULL) {
      /* Select and show. */
      sys = system_get(sysname);
      if (sys_isKnown(sys)) {
         map_select( sys, 0 );
         map_center( sysname );
         return 1;
      }
   }

   /* Do fuzzy match. */
   names = system_searchFuzzyCase( name, &len );
   if (len > 0) {

      /* Construct found table. */
      found = malloc( sizeof(map_find_t) * len );
      n = 0;
      for (i=0; i<len; i++) {

         /* System must be known. */
         sys = system_get( names[i] );
         if (!sys_isKnown(sys))
            continue;

         /* Set more values. */
         ret = map_findDistance( sys, &found[n].jumps, &found[n].distance );
         if (ret)
            continue;

         /* Set some values. */
         found[n].pnt      = NULL;
         found[n].sys      = sys;

         /* Set fancy name. */
         snprintf( found[n].display, sizeof(found[n].display),
               "%s (%d jumps, %.3f distance)",
               sys->name, found[n].jumps, found[n].distance );
         n++;
      }
      free(names);

      /* No visible match. */
      if (n==0)
         return -1;

      /* Sort the found by distance. */
      map_sortFound( found, n );

      /* Display results. */
      map_findDisplayResult( parent, found, n );
      return 0;
   }

   /* No match. */
   return -1;
}


/**
 * @brief Searches for a planet.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchPlanets( unsigned int parent, const char *name )
{
   const char *sysname;
   const char *pntname;
   StarSystem *sys;

   /* Match planet first. */
   pntname = planet_existsCase( name );
   if (pntname == NULL)
      return -1;

   /* Check exact match. */
   sysname = planet_getSystem( pntname );
   if (sysname != NULL) {
      /* Select and show. */
      sys = system_get(sysname);
      if (sys_isKnown(sys)) {
         map_select( sys, 0 );
         map_center( sysname );
         return 1;
      }
   }

   /* Do fuzzy match. */
   return 0;

   /* No match. */
   return -1;
}


/**
 * @brief Searches for a outfit.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchOutfits( unsigned int parent, const char *name )
{
   return 0;
}


/**
 * @brief Searches for a ship.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchShips( unsigned int parent, const char *name )
{
   return 0;
}


/**
 * @brief Does a search.
 */
static void map_findSearch( unsigned int wid, char* str )
{
   int ret;
   char *name;
   char *searchname;

   /* Get the name. */
   name = window_getInput( wid, "inpSearch" );
   if (name == NULL || !strcmp("",name))
      return;

   /* Clean up if necessary. */
   if (map_found_cur != NULL)
      free( map_found_cur );
   map_found_cur = NULL;

   /* Handle different search cases. */
   if (map_find_systems) {
      ret = map_findSearchSystems( wid, name );
      searchname = "System";
   }
   else if (map_find_planets) {
      ret = map_findSearchPlanets( wid, name );
      searchname = "Planet";
   }
   else if (map_find_outfits) {
      ret = map_findSearchOutfits( wid, name );
      searchname = "Outfit";
   }
   else if (map_find_ships) {
      ret = map_findSearchShips( wid, name );
      searchname = "Ship";
   }
   else {
      ret = 1;
   }

   if (ret < 0)
      dialogue_alert( "%s matching '%s' not found!", searchname, name );

   if (ret > 0)
      map_findClose( wid, str );
}


/**
 * @brief Opens a search input box to find a system or planet.
 */
void map_inputFind( unsigned int parent, char* str )
{
   (void) str;
   unsigned int wid;
   int x, y;

   /* Create the window. */
   wid = window_create( "Find...", -1, -1, 300, 220 );
   window_setAccept( wid, map_findSearch );
   window_setCancel( wid, map_findClose );
   window_setParent( wid, parent );

   /* Text. */
   y = -40;
   window_addText( wid, 20, y, 300, gl_defFont.h+4, 0,
         "txtDescription", &gl_defFont, &cDConsole,
         "Enter keyword to search for:" );
   y -= 30;

   /* Create input. */
   window_addInput( wid, 30, y, 240, 20,
         "inpSearch", 32, 1, &gl_defFont );
   y -= 40;

   /* Create buttons. */
   window_addButton( wid,300-BUTTON_WIDTH-30, 20+BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSearch", "Find", map_findSearch );
   window_addButton( wid,300-BUTTON_WIDTH-30, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", map_findClose );

   /* Create check boxes. */
   x = 40;
   window_addCheckbox( wid, x, y, 100, 20,
         "chkSystem", "Systems", map_find_check_update, map_find_systems );
   y -= 20;
   window_addCheckbox( wid, x, y, 100, 20,
         "chkPlanet", "Planets", map_find_check_update, map_find_planets );
   y -= 20;
   window_addCheckbox( wid, x, y, 100, 20,
         "chkOutfit", "Outfits", map_find_check_update, map_find_outfits );
   y -= 20;
   window_addCheckbox( wid, x, y, 100, 20,
         "chkShip", "Ships", map_find_check_update, map_find_ships );
   y -= 20;
}

