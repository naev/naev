/*
 * See Licensing and Copyright notice in naev.h
 */


/** @cond */
#include <assert.h>

#include "naev.h"
/** @endcond */

#include "map_find.h"

#include "array.h"
#include "dialogue.h"
#include "log.h"
#include "map.h"
#include "nstring.h"
#include "player.h"
#include "space.h"
#include "tech.h"
#include "toolkit.h"


#define BUTTON_WIDTH    120 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


/* Stored checkbox values. */
static int map_find_systems = 1; /**< Systems checkbox value. */
static int map_find_planets = 0; /**< Planets checkbox value. */
static int map_find_outfits = 0; /**< Outfits checkbox value. */
static int map_find_ships   = 0; /**< Ships checkbox value. */

/* Misc ugly globals. */
/* Current found stuff. */
static map_find_t *map_found_cur    = NULL;  /**< Pointer to found stuff. */
static int map_found_ncur           = 0;     /**< Number of found stuff. */
static char **map_foundOutfitNames  = NULL; /**< Array (array.h): Internal names of outfits in the search results. */
/* Tech hack. */
static tech_group_t **map_known_techs = NULL; /**< Array (array.h) of known techs. */
static Planet **map_known_planets   = NULL;  /**< Array (array.h) of known planets with techs. */


/*
 * Prototypes.
 */
/* Init/cleanup. */
static int map_knownInit (void);
static void map_knownClean (void);
/* Toolkit-related. */
static void map_find_check_update( unsigned int wid, char *str );
static void map_findClose( unsigned int wid, char* str );
static int map_findSearchSystems( unsigned int parent, const char *name );
static int map_findSearchPlanets( unsigned int parent, const char *name );
static int map_findSearchOutfits( unsigned int parent, const char *name );
static int map_findSearchShips( unsigned int parent, const char *name );
static void map_findSearch( unsigned int wid, char* str );
/* Misc. */
static void map_findAccumulateResult( map_find_t *found, int n,  StarSystem *sys, Planet *pnt );
static int map_sortCompare( const void *p1, const void *p2 );
static void map_sortFound( map_find_t *found, int n );
static char map_getPlanetColourChar( Planet *p );
/* Fuzzy outfit/ship stuff. */
static char **map_fuzzyOutfits( Outfit **o, const char *name );
static char **map_outfitsMatch( const char *name );
static char **map_fuzzyShips( Ship **o, const char *name );
static char **map_shipsMatch( const char *name );


/**
 * @brief Initializes stuff the pilot knows.
 */
static int map_knownInit (void)
{
   int i, j;
   Planet *pnt, **planets;
   StarSystem *sys;
   tech_group_t **t;

   /* Allocate techs. */
   t        = array_create( tech_group_t* );
   planets  = array_create( Planet* );
   sys      = system_getAll();

   /* Get techs. */
   for (i=0; i<array_size(sys); i++) {
      if (!sys_isKnown( &sys[i] ))
         continue;

      for (j=0; j<array_size(sys[i].planets); j++) {
         pnt = sys[i].planets[j];

         /* Must be real. */
         if (pnt->real != ASSET_REAL)
            continue;

         /* Must be known. */
         if (!planet_isKnown( pnt ))
            continue;

         /* Must have techs. */
         if (pnt->tech != NULL) {
            array_push_back( &planets, pnt );
            array_push_back( &t, pnt->tech );
         }
      }
   }

   /* Pointer voodoo. */
   map_known_techs   = t;
   map_known_planets = planets;

   return 0;
}


/**
 * @brief Cleans up stuff the pilot knows.
 */
static void map_knownClean (void)
{
   array_free( map_known_techs );
   map_known_techs = NULL;
   array_free( map_known_planets );
   map_known_planets = NULL;
}


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
 * @brief Starts the map search with a specific default type.
 *
 *    @param Parent window's ID.
 *    @param Default type to search for.
 */
void map_inputFindType( unsigned int parent, char *type )
{
   map_find_systems = 0;
   map_find_planets = 0;
   map_find_outfits = 0;
   map_find_ships   = 0;

   if (strcmp(type,"system")==0)
      map_find_systems = 1;
   else if (strcmp(type,"planet")==0)
      map_find_planets = 1;
   else if (strcmp(type,"outfit")==0)
      map_find_outfits = 1;
   else if (strcmp(type,"ship")==0)
      map_find_ships   = 1;

   map_inputFind(parent, NULL);
}


/**
 * @brief Closes the find window.
 */
static void map_findClose( unsigned int wid, char* str )
{
   window_close( wid, str );

   /* Clean up if necessary. */
   free( map_found_cur );
   map_found_cur = NULL;

   /* Clean up. */
   map_knownClean();
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

   /* Sort the found by distance. */
   map_sortFound( found, n );

   /* Create window. */
   wid = window_create( "wswFindResult", _("Search Results"), -1, -1, 500, 452 );
   window_setParent( wid, parent );
   window_setAccept( wid, map_findDisplayMark );
   window_setCancel( wid, window_close );

   /* The list. */
   ll = malloc( sizeof(char*) * n );
   for (i=0; i<n; i++)
      ll[i] = strdup( found[i].display );
   window_addList( wid, 20, -40, 460, 300,
         "lstResult", ll, n, 0, NULL, map_findDisplayMark );

   /* Buttons. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSelect", _("Select"), map_findDisplayMark );
   window_addButton( wid, -40 - BUTTON_WIDTH, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("Cancel"), window_close );
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

   /* If they're the same it doesn't matter, so we'll sort by name. */
   return strcasecmp( f1->sys->name, f2->sys->name );
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
static int map_findDistance( StarSystem *sys, Planet *pnt, int *jumps, double *distance )
{
   int i, j;
   StarSystem **slist, *ss;
   double d;
   Vector2d *vs, *ve;

   /* Defaults. */
   ve = NULL;

   /* Special case it's the current system. */
   if (sys == cur_system) {
      *jumps = 0;
      if (pnt != NULL)
         *distance = vect_dist( &player.p->solid->pos, &pnt->pos );
      else
         *distance = 0.;

      return 0;
   }

   /* Calculate jump path. */
   slist = map_getJumpPath( cur_system->name, sys->name, 0, 1, NULL );
   *jumps = array_size( slist );
   if (slist==NULL)
      /* Unknown. */
      return -1;

   /* Distance to first jump point. */
   vs = &player.p->solid->pos;
   for (j=0; j < array_size(cur_system->jumps); j++) {
      if (cur_system->jumps[j].target == slist[0]) {
         ve = &cur_system->jumps[j].pos;
         break;
      }
   }
   if (ve == NULL) {
      WARN(_("Jump to first system not found!"));
      d = 0.;
   }
   else
      d = vect_dist( vs, ve );

   /* Calculate distance. */
   for (i=0; i<(*jumps-1); i++) {
      ss = slist[i];

      /* Search jump points. */
      for (j=0; j < array_size(ss->jumps); j++) {

         /* Get previous jump. */
         if (i == 0) {
            if (ss->jumps[j].target == cur_system)
               vs = &ss->jumps[j].pos;
         }
         else {
            if (ss->jumps[j].target == slist[i-1])
               vs = &ss->jumps[j].pos;
         }

         /* Get next jump. */
         if (ss->jumps[j].target == slist[i+1]) {
            ve = &ss->jumps[j].pos;
            break;
         }
      }

      /* Use current position. */
      if (i==0)
         vs = &player.p->solid->pos;

#ifdef DEBUGGING
      if ((vs==NULL) || (ve==NULL)) {
         WARN( _("Matching jumps not found, something is up...") );
         continue;
      }
#endif /* DEBUGGING */

      /* Calculate. */
      d += vect_dist( vs, ve );
   }

   /* Account final travel to planet for planet targets. */
   if (pnt != NULL) {
      if (i > 0) {
         ss = slist[ i ];
         for (j=0; j < array_size(ss->jumps); j++) {
            if (ss->jumps[j].target == slist[i-1]) {
               vs = &ss->jumps[j].pos;
               break;
            }
         }
      }

      ve = &pnt->pos;

      assert( vs != NULL );
      assert( ve != NULL );

      d += vect_dist( vs, ve );
   }

   /* Cleanup. */
   array_free(slist);

   *distance = d;
   return 0;
}


/**
 * @brief Loads a search result into a table.
 *
 *    @param found Results array to save into.
 *    @param n Position to use.
 *    @param sys Result's star system.
 *    @param pnt Result's planet, or NULL to leave unspecified.
 *
 */
static void map_findAccumulateResult( map_find_t *found, int n,  StarSystem *sys, Planet *pnt )
{
   int ret;
   char route_info[256];

   /* Set some values. */
   found[n].pnt      = pnt;
   found[n].sys      = sys;

   /* Set more values. */
   ret = map_findDistance( sys, pnt, &found[n].jumps, &found[n].distance );
   if (ret) {
      found[n].jumps    = 10000;
      found[n].distance = 1e6;
      snprintf( route_info, sizeof(route_info), "%s", _("unknown route") );
   }
   else
      snprintf( route_info, sizeof(route_info),
            n_( "%d jump, %.0fk distance", "%d jumps, %.0fk distance", found[n].jumps ),
            found[n].jumps, found[n].distance/1000. );

   /* Set fancy name. */
   if (pnt == NULL)
      snprintf( found[n].display, sizeof(found[n].display),
            _("%s (%s)"), _(sys->name), route_info );
   else
      snprintf( found[n].display, sizeof(found[n].display),
            _("#%c%s%s (%s, %s)"), map_getPlanetColourChar(pnt),
            planet_getSymbol(pnt),
            _(pnt->name), _(sys->name), route_info );
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
   int len, n;
   map_find_t *found;

   /* Search for names. */
   sysname = system_existsCase( name );
   names   = system_searchFuzzyCase( name, &len );
   if (names == NULL)
      return -1;

   /* Exact match. */
   if ((sysname != NULL) && (len == 1)) {
      /* Select and show. */
      sys = system_get(sysname);
      if (sys_isKnown(sys)) {
         map_select( sys, 0 );
         map_center( sysname );
         free(names);
         return 1;
      }
   }

   /* Construct found table. */
   found = NULL;
   n = 0;
   for (i=0; i<len; i++) {

      /* System must be known. */
      sys = system_get( names[i] );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, NULL );
      n++;
   }
   free(names);

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( parent, found, n );
   return 0;
}


/**
 * @brief Searches for a planet.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchPlanets( unsigned int parent, const char *name )
{
   int i;
   char **names;
   int len, n;
   map_find_t *found;
   const char *sysname, *pntname;
   StarSystem *sys;
   Planet *pnt;

   /* Match planet first. */
   pntname = planet_existsCase( name );
   names   = planet_searchFuzzyCase( name, &len );
   if (names == NULL)
      return -1;

   /* Exact match. */
   if ((pntname != NULL) && (len == 1)) {

      /* Check exact match. */
      sysname = planet_getSystem( pntname );
      if (sysname != NULL) {
         /* Make sure it's known. */
         pnt = planet_get( pntname );
         if ((pnt != NULL) && planet_isKnown(pnt)) {

            /* Select and show. */
            sys = system_get(sysname);
            if (sys_isKnown(sys)) {
               map_select( sys, 0 );
               map_center( sysname );
               free(names);
               return 1;
            }
         }
      }
   }

   /* Construct found table. */
   found = NULL;
   n = 0;
   for (i=0; i<len; i++) {

      /* Planet must be real. */
      pnt = planet_get( names[i] );
      if (pnt == NULL)
         continue;
      if (pnt->real != ASSET_REAL)
         continue;
      if (!planet_isKnown(pnt))
         continue;

      /* System must be known. */
      sysname = planet_getSystem( names[i] );
      if (sysname == NULL)
         continue;
      sys = system_get( sysname );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, pnt );
      n++;
   }
   free(names);

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( parent, found, n );
   return 0;
}


/**
 * @brief Gets a colour char for a planet, simplified for map use.
 */
static char map_getPlanetColourChar( Planet *p )
{
   char colcode;

   planet_updateLand(p);
   colcode = planet_getColourChar(p);

   return colcode;
}


/**
 * @brief Does fuzzy name matching for outfits in an Array. Searches translated names but returns internal names.
 */
static char **map_fuzzyOutfits( Outfit **o, const char *name )
{
   int i;
   char **names;

   names = array_create( char* );

   /* Do fuzzy search. */
   for (i=0; i<array_size(o); i++)
      if (strcasestr( _(o[i]->name), name ) != NULL)
         array_push_back( &names, o[i]->name );

   return names;
}

/**
 * @brief Gets the possible names the outfit name matches.
 */
static char **map_outfitsMatch( const char *name )
{
   Outfit **o;
   char **names;

   /* Get outfits and names. */
   o     = tech_getOutfitArray( map_known_techs, array_size( map_known_techs ) );
   names = map_fuzzyOutfits( o, name );
   array_free(o);

   return names;
}
/**
 * @brief Add widgets to the extended area on the outfit search
 *    listpanel.
 *
 *    @param wid The windowid we're adding widgets to
 *    @param x The x offset where we can start drawing
 *    @param y the y offset where we can start drawing
 *    @param w The width of the area where we can draw
 *    @param h The height of the area where we can draw
 */
static void map_addOutfitDetailFields(unsigned int wid, int x, int y, int w, int h)
{
   (void) h;
   (void) y;
   int iw;

   iw = x;

   window_addRect( wid, -1 + iw, -50, 128, 129, "rctImage", &cBlack, 0 );
   window_addImage( wid, iw, -50-128, 0, 0, "imgOutfit", NULL, 1 );

   window_addText( wid, iw + 128 + 20, -60,
         280, 160, 0, "txtOutfitName", &gl_defFont, NULL, NULL );
   window_addText( wid, iw + 128 + 20, -60 - gl_defFont.h - 20,
         280, 160, 0, "txtDescShort", &gl_smallFont, NULL, NULL );
   window_addText( wid, iw+20, -60-128-10,
         90, 160, 0, "txtSDesc", &gl_smallFont, NULL,
         _("#nOwned:#0\n"
         "\n"
         "#nSlot:#0\n"
         "#nSize:#0\n"
         "#nMass:#0\n"
         "\n"
         "#nPrice:#0\n"
         "#nMoney:#0\n"
         "#nLicense:#0\n") );
   window_addText( wid, iw+20, -60-128-10,
         w - (20 + iw + 20 + 90), 160, 0, "txtDDesc", &gl_smallFont, NULL, NULL );
   window_addText( wid, iw+20, -60-128-10-160,
         w-(iw+80), 180, 0, "txtDescription",
         &gl_smallFont, NULL, NULL );
}
/**
 * @brief Update the listPanel outfit details to the outfit selected.
 *
 *    @param wid The windowid of the window we're updating.
 *    @param wgtname The name of the list that was selected.
 *    @param x The x offset where we can start drawing
 *    @param y the y offset where we can start drawing
 *    @param w The width of the area where we can draw
 *    @param h The height of the area where we can draw
 */
static void map_showOutfitDetail(unsigned int wid, char* wgtname, int x, int y, int w, int h)
{
   (void) x;
   (void) y;
   (void) h;
   Outfit *outfit;
   char buf[PATH_MAX], buf2[ECON_CRED_STRLEN], buf3[ECON_CRED_STRLEN];
   double th;
   int iw;
   double mass;

   /* 452 px is the sum of the 128 px outfit image width, its 4 px border,
    * a 20 px gap, 280 px for the outfit's name and a final 20 px gap. */
   iw = w - 452;

   outfit = outfit_get( map_foundOutfitNames[toolkit_getListPos(wid, wgtname)] );
   window_modifyText( wid, "txtOutfitName", _(outfit->name) );
   window_modifyImage( wid, "imgOutfit", outfit->gfx_store, 0, 0 );

   mass = outfit->mass;
   if ((outfit_isLauncher(outfit) || outfit_isFighterBay(outfit)) &&
         (outfit_ammo(outfit) != NULL)) {
      mass += outfit_amount(outfit) * outfit_ammo(outfit)->mass;
   }

   window_modifyText( wid, "txtDescription", _(outfit->description) );
   credits2str( buf2, outfit->price, 2 );
   credits2str( buf3, player.p->credits, 2 );
   snprintf( buf, sizeof(buf),
         _("%d\n"
         "\n"
         "%s\n"
         "%s\n"
         "%.0f tonnes\n"
         "\n"
         "%s\n"
         "%s\n"
         "%s\n"),
         player_outfitOwned(outfit),
         _(outfit_slotName(outfit)),
         _(outfit_slotSize(outfit)),
         mass,
         buf2,
         buf3,
         (outfit->license != NULL) ? _(outfit->license) : _("None") );
   window_modifyText( wid, "txtDDesc", buf );
   window_modifyText( wid, "txtOutfitName", _(outfit->name) );
   window_modifyText( wid, "txtDescShort", outfit->desc_short );
   th = MAX( 128, gl_printHeightRaw( &gl_smallFont, 280, outfit->desc_short ) );
   window_moveWidget( wid, "txtSDesc", iw+20, -60-th-20 );
   window_moveWidget( wid, "txtDDesc", iw+20+90, -60-th-20 );
   th += gl_printHeightRaw( &gl_smallFont, 280, buf );
   window_moveWidget( wid, "txtDescription", iw+20, -60-th-40 );
}

/**
 * @brief Searches for a outfit.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchOutfits( unsigned int parent, const char *name )
{
   int i, j;
   int len, n;
   map_find_t *found;
   Planet *pnt;
   StarSystem *sys;
   const char *oname, *sysname;
   char **list;
   Outfit *o, **olist;

   assert( "Outfit search is not reentrant!" && map_foundOutfitNames == NULL );

   /* Match planet first. */
   o     = NULL;
   oname = outfit_existsCase( name );
   map_foundOutfitNames = map_outfitsMatch( name );
   len = array_size( map_foundOutfitNames );
   if ((oname != NULL) && (len == 1))
      o = outfit_get( oname );
   /* Do fuzzy match. */
   else if (len > 0) {
      /* Ask which one player wants. */
      list  = malloc( len*sizeof(char*) );
      for (i=0; i<len; i++)
         list[i] = strdup( _(map_foundOutfitNames[i]) );
      i = dialogue_listPanel( _("Search Results"), list, len, 452, 650,
            map_addOutfitDetailFields, map_showOutfitDetail,
            _("Search results for outfits matching '%s':"), name );
      if (i < 0) {
         array_free( map_foundOutfitNames );
         map_foundOutfitNames = NULL;
         return 0;
      }
      o = outfit_get( map_foundOutfitNames[i] );
   }
   array_free( map_foundOutfitNames );
   map_foundOutfitNames = NULL;
   if (o == NULL)
      return -1;

   /* Construct found table. */
   found = NULL;
   n = 0;
   len = array_size(map_known_techs);
   for (i=0; i<len; i++) {
      /* Try to find the outfit in the planet. */
      olist = tech_getOutfit( map_known_techs[i] );
      for (j=array_size(olist)-1; j>=0; j--)
         if (olist[j] == o)
            break;
      array_free( olist );
      olist = NULL;
      if (j < 0)
         continue;
      pnt = map_known_planets[i];

      /* System must be known. */
      sysname = planet_getSystem( pnt->name );
      if (sysname == NULL)
         continue;
      sys = system_get( sysname );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, pnt );
      n++;
   }

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( parent, found, n );
   return 0;
}


/**
 * @brief Does fuzzy name matching for ships in an Array. Searches translated names but returns internal names.
 */
static char **map_fuzzyShips( Ship **s, const char *name )
{
   int i;
   char **names;

   names = array_create( char* );

   /* Do fuzzy search. */
   for (i=0; i<array_size(s); i++)
      if (strcasestr( _(s[i]->name), name ) != NULL)
         array_push_back( &names, s[i]->name );

   return names;
}
/**
 * @brief Gets the possible names the ship name matches.
 */
static char **map_shipsMatch( const char *name )
{
   Ship **s;
   char **names;

   /* Get ships and names. */
   s     = tech_getShipArray( map_known_techs, array_size( map_known_techs ) );
   names = map_fuzzyShips( s, name );
   array_free(s);

   return names;
}

/**
 * @brief Searches for a ship.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchShips( unsigned int parent, const char *name )
{
   int i, j;
   char **names;
   int len, n;
   map_find_t *found;
   Planet *pnt;
   StarSystem *sys;
   const char *sname, *sysname;
   char **list;
   Ship *s, **slist;

   /* Match planet first. */
   s     = NULL;
   sname = ship_existsCase( name );
   names = map_shipsMatch( name );
   len = array_size( names );
   if ((sname != NULL) && (len == 1))
      s = ship_get( sname );
   /* Handle fuzzy matching. */
   else if (len > 0) {
      /* Ask which one player wants. */
      list  = malloc( len*sizeof(char*) );
      for (i=0; i<len; i++)
         list[i] = strdup( _(names[i]) );
      i = dialogue_list( _("Search Results"), list, len,
            _("Search results for ships matching '%s':"), name );
      if (i < 0) {
         array_free(names);
         return 0;
      }
      s = ship_get( names[i] );
   }
   array_free(names);
   names = NULL;
   if (s == NULL)
      return -1;

   /* Construct found table. */
   found = NULL;
   n = 0;
   len = array_size(map_known_techs);
   for (i=0; i<len; i++) {
      /* Try to find the ship in the planet. */
      slist = tech_getShip( map_known_techs[i] );
      for (j=array_size(slist)-1; j>=0; j--)
         if (slist[j] == s)
            break;
      array_free(slist);
      slist = NULL;
      if (j < 0)
         continue;
      pnt = map_known_planets[i];

      /* System must be known. */
      sysname = planet_getSystem( pnt->name );
      if (sysname == NULL)
         continue;
      sys = system_get( sysname );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, pnt );
      n++;
   }

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( parent, found, n );
   return 0;
}


/**
 * @brief Does a search.
 */
static void map_findSearch( unsigned int wid, char* str )
{
   int ret;
   const char *name, *searchname;

   /* Get the name. */
   name = window_getInput( wid, "inpSearch" );
   if ( (name == NULL) || ( strcmp("", name) == 0 ) )
      return;

   /* Prevent reentrancy, e.g. the toolkit spontaneously deciding a future mouseup event was the
    * user releasing the clicked "Find" button and should reactivate it, never mind that they were
    * actually clicking on the dialogue_listPanel we opened to present the results.
    * FIXME: That behavior doesn't seem right, but I'm not sure if it's an actual bug or not. */
   window_disableButton( wid, "btnSearch" );

   /* Clean up if necessary. */
   free( map_found_cur );
   map_found_cur = NULL;

   /* Handle different search cases. */
   if (map_find_systems) {
      ret = map_findSearchSystems( wid, name );
      searchname = _("System");
   }
   else if (map_find_planets) {
      ret = map_findSearchPlanets( wid, name );
      searchname = _("Planet");
   }
   else if (map_find_outfits) {
      ret = map_findSearchOutfits( wid, name );
      searchname = _("Outfit");
   }
   else if (map_find_ships) {
      ret = map_findSearchShips( wid, name );
      searchname = _("Ship");
   }
   else
      ret = 1;

   if (ret < 0)
      dialogue_alert( _("%s matching '%s' not found!"), searchname, name );

   /* Safe at last. */
   window_enableButton( wid, "btnSearch" );

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
   int x, y, w, h;

   /* initialize known. */
   map_knownInit();

   /* Create the window. */
   w = 400;
   h = 220;
   wid = window_create( "wdwFind", _("Find..."), -1, -1, w, h );
   window_setAccept( wid, map_findSearch );
   window_setCancel( wid, map_findClose );
   window_setParent( wid, parent );

   /* Text. */
   y = -40;
   window_addText( wid, 20, y, w - 50, gl_defFont.h+4, 0,
         "txtDescription", &gl_defFont, NULL,
         _("Enter keyword to search for:") );
   y -= 30;

   /* Create input. */
   window_addInput( wid, 30, y, w - 60, 20,
         "inpSearch", 32, 1, &gl_smallFont );
   y -= 40;

   /* Create buttons. */
   window_addButton( wid, -30, 20+BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSearch", _("Find"), map_findSearch );
   window_addButton( wid, -30, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("Close"), map_findClose );

   /* Create check boxes. */
   x = 40;
   window_addCheckbox( wid, x, y, 160, 20,
         "chkSystem", _("Systems"), map_find_check_update, map_find_systems );
   y -= 20;
   window_addCheckbox( wid, x, y, 160, 20,
         "chkPlanet", _("Planets"), map_find_check_update, map_find_planets );
   y -= 20;
   window_addCheckbox( wid, x, y, 160, 20,
         "chkOutfit", _("Outfits"), map_find_check_update, map_find_outfits );
   y -= 20;
   window_addCheckbox( wid, x, y, 160, 20,
         "chkShip", _("Ships"), map_find_check_update, map_find_ships );
}

