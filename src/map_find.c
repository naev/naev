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
static int map_find_spobs   = 0; /**< Spobs checkbox value. */
static int map_find_outfits = 0; /**< Outfits checkbox value. */
static int map_find_ships   = 0; /**< Ships checkbox value. */

/* Misc ugly globals. */
/* Current found stuff. */
static map_find_t *map_found_cur    = NULL;  /**< Pointer to found stuff. */
static int map_found_ncur           = 0;     /**< Number of found stuff. */
static char **map_foundOutfitNames  = NULL; /**< Array (array.h): Internal names of outfits in the search results. */
/* Tech hack. */
static tech_group_t **map_known_techs = NULL; /**< Array (array.h) of known techs. */
static Spob **map_known_spobs   = NULL;  /**< Array (array.h) of known spobs with techs. */

/*
 * Prototypes.
 */
/* Init/cleanup. */
static int map_knownInit (void);
static void map_knownClean (void);
/* Toolkit-related. */
static void map_addOutfitDetailFields(unsigned int wid_results, int x, int y, int w, int h);
static void map_findCheckUpdate( unsigned int wid_map_find, const char *str );
static void map_findOnClose( unsigned int wid_map_find, const char* str );
static void map_findDisplayMark( unsigned int wid_results, const char* str );
static void map_findDisplayResult( unsigned int wid_map_find, map_find_t *found, int n );
static int map_findSearchSystems( unsigned int wid_map_find, const char *name );
static int map_findSearchSpobs( unsigned int wid_map_find, const char *name );
static int map_findSearchOutfits( unsigned int wid_map_find, const char *name );
static int map_findSearchShips( unsigned int wid_map_find, const char *name );
static void map_findSearch( unsigned int wid_map_find, const char* str );
static void map_showOutfitDetail(unsigned int wid, const char* wgtname, int x, int y, int w, int h);
/* Misc. */
static void map_findAccumulateResult( map_find_t *found, int n, StarSystem *sys, Spob *spob );
static void map_findSelect( const StarSystem *sys );
static int map_sortCompare( const void *p1, const void *p2 );
static void map_sortFound( map_find_t *found, int n );
static char map_getSpobColourChar( Spob *p );
static const char *map_getSpobSymbol( Spob *p );
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
   const StarSystem *sys = system_getAll();

   map_knownClean();
   map_known_techs = array_create( tech_group_t* );
   map_known_spobs = array_create( Spob* );

   /* Get techs. */
   for (int i=0; i<array_size(sys); i++) {
      if (!sys_isKnown( &sys[i] ))
         continue;

      for (int j=0; j<array_size(sys[i].spobs); j++) {
         Spob *spob = sys[i].spobs[j];

         if (spob_isKnown( spob ) && spob->tech != NULL) {
            array_push_back( &map_known_spobs, spob );
            array_push_back( &map_known_techs, spob->tech );
         }
      }
   }

   return 0;
}

/**
 * @brief Cleans up stuff the pilot knows.
 */
static void map_knownClean (void)
{
   array_free( map_known_techs );
   map_known_techs = NULL;
   array_free( map_known_spobs );
   map_known_spobs = NULL;
}

/**
 * @brief Updates the checkboxes.
 */
static void map_findCheckUpdate( unsigned int wid_map_find, const char* str )
{
   (void) str;
   map_find_systems ^= window_checkboxState( wid_map_find, "chkSystem" );
   map_find_spobs   ^= window_checkboxState( wid_map_find, "chkSpob" );
   map_find_outfits ^= window_checkboxState( wid_map_find, "chkOutfit" );
   map_find_ships   ^= window_checkboxState( wid_map_find, "chkShip" );
   window_checkboxSet( wid_map_find, "chkSystem", map_find_systems );
   window_checkboxSet( wid_map_find, "chkSpob",   map_find_spobs );
   window_checkboxSet( wid_map_find, "chkOutfit", map_find_outfits );
   window_checkboxSet( wid_map_find, "chkShip",   map_find_ships );
}

/**
 * @brief Starts the map search with a specific default type.
 *
 *    @param Parent window's ID.
 *    @param Default type to search for.
 */
void map_inputFindType( unsigned int parent, const char *type )
{
   map_find_systems = 0;
   map_find_spobs   = 0;
   map_find_outfits = 0;
   map_find_ships   = 0;

   if (strcmp(type,"system")==0)
      map_find_systems = 1;
   else if (strcmp(type,"spob")==0)
      map_find_spobs = 1;
   else if (strcmp(type,"outfit")==0)
      map_find_outfits = 1;
   else if (strcmp(type,"ship")==0)
      map_find_ships   = 1;

   map_inputFind(parent, NULL);
}

/**
 * @brief Cleans up resources used by the find window.
 */
static void map_findOnClose( unsigned int wid, const char* str )
{
   (void) wid;
   (void) str;

   free( map_found_cur );
   map_found_cur = NULL;
   map_knownClean();
}

/**
 * @brief Goes to a found system to display it.
 */
static void map_findDisplayMark( unsigned int wid_results, const char* str )
{
   /* Get system. */
   int pos = toolkit_getListPos( wid_results, "lstResult" );
   StarSystem *sys = map_found_cur[ pos ].sys;
   int wid_map_find = window_getParent( wid_results );

   map_findSelect( sys );

   /* Close parent. */
   window_close( wid_map_find, str );
}

/**
 * @brief Displays the results of the find.
 */
static void map_findDisplayResult( unsigned int wid_map_find, map_find_t *found, int n )
{
   unsigned int wid_results;
   char **ll;

   /* Globals. */
   map_found_cur  = found;
   map_found_ncur = n;

   /* Sort the found by distance. */
   map_sortFound( found, n );

   /* Create window. */
   wid_results = window_create( "wdwFindResult", _("Search Results"), -1, -1, 500, 452 );
   window_setParent( wid_results, wid_map_find );
   window_setAccept( wid_results, map_findDisplayMark );
   window_setCancel( wid_results, window_close );

   /* The list. */
   ll = malloc( sizeof(char*) * n );
   for (int i=0; i<n; i++)
      ll[i] = strdup( found[i].display );
   window_addList( wid_results, 20, -40, 460, 300,
         "lstResult", ll, n, 0, NULL, map_findDisplayMark );

   /* Buttons. */
   window_addButton( wid_results, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSelect", _("Select"), map_findDisplayMark );
   window_addButton( wid_results, -40 - BUTTON_WIDTH, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
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
static int map_findDistance( StarSystem *sys, Spob *spob, int *jumps, double *distance )
{
   StarSystem **slist;
   double d;
   int i;
   vec2 *vs, *ve;

   /* Defaults. */
   ve = NULL;

   /* Special case it's the current system. */
   if (sys == cur_system) {
      *jumps = 0;
      if (spob != NULL)
         *distance = vec2_dist( &player.p->solid->pos, &spob->pos );
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
   for (int j=0; j < array_size(cur_system->jumps); j++) {
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
      d = vec2_dist( vs, ve );

   /* Calculate distance. */
   for (i=0; i<(*jumps-1); i++) {
      StarSystem *ss = slist[i];

      /* Search jump points. */
      for (int j=0; j < array_size(ss->jumps); j++) {

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
      d += vec2_dist( vs, ve );
   }

   /* Account final travel to spob for spob targets. */
   if (spob != NULL) {
      if (i > 0) {
         StarSystem *ss = slist[ i ];
         for (int j=0; j < array_size(ss->jumps); j++) {
            if (ss->jumps[j].target == slist[i-1]) {
               vs = &ss->jumps[j].pos;
               break;
            }
         }
      }

      ve = &spob->pos;

      assert( vs != NULL );
      assert( ve != NULL );

      d += vec2_dist( vs, ve );
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
 *    @param spob Result's spob, or NULL to leave unspecified.
 *
 */
static void map_findAccumulateResult( map_find_t *found, int n, StarSystem *sys, Spob *spob )
{
   int ret;
   char route_info[STRMAX_SHORT];

   /* Set some values. */
   found[n].spob  = spob;
   found[n].sys   = sys;

   /* Set more values. */
   ret = map_findDistance( sys, spob, &found[n].jumps, &found[n].distance );
   if (ret) {
      found[n].jumps    = 10e3;
      found[n].distance = 1e6;
      snprintf( route_info, sizeof(route_info), "%s", _("unknown route") );
   }
   else
      snprintf( route_info, sizeof(route_info),
            n_( "%d jump, %.0fk distance", "%d jumps, %.0fk distance", found[n].jumps ),
            found[n].jumps, found[n].distance/1000. );

   /* Set fancy name. */
   if (spob == NULL)
      snprintf( found[n].display, sizeof(found[n].display),
            _("%s (%s)"), _(sys->name), route_info );
   else
      snprintf( found[n].display, sizeof(found[n].display),
            _("#%c%s%s (%s, %s)"), map_getSpobColourChar(spob),
            map_getSpobSymbol(spob),
            spob_name(spob), _(sys->name), route_info );
}

/**
 * @brief Centers the search result in the map, opening if necessary.
 */
static void map_findSelect( const StarSystem *sys )
{
   if (!map_isOpen())
      map_open();
   map_select( sys, 0 );
   map_center( 0, sys->name );
}

/**
 * @brief Searches for a system.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchSystems( unsigned int wid_map_find, const char *name )
{
   const char *sysname;
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
      StarSystem *sys = system_get(sysname);
      if (sys_isKnown(sys)) {
         map_findSelect( sys );
         free(names);
         return 1;
      }
   }

   /* Construct found table. */
   found = NULL;
   n = 0;
   for (int i=0; i<len; i++) {

      /* System must be known. */
      StarSystem *sys = system_get( names[i] );
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
   map_findDisplayResult( wid_map_find, found, n );
   return 0;
}

/**
 * @brief Searches for a spob.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchSpobs( unsigned int wid_map_find, const char *name )
{
   char **names;
   int len, n;
   map_find_t *found;
   const char *spobname;

   /* Match spob first. */
   spobname = spob_existsCase( name );
   names   = spob_searchFuzzyCase( name, &len );
   if (names == NULL)
      return -1;

   /* Exact match. */
   if ((spobname != NULL) && (len == 1)) {
      /* Check exact match. */
      const char *sysname = spob_getSystem( spobname );
      if (sysname != NULL) {
         /* Make sure it's known. */
         Spob *spob = spob_get( spobname );
         if ((spob != NULL) && spob_isKnown(spob)) {

            /* Select and show. */
            StarSystem *sys = system_get(sysname);
            if (sys_isKnown(sys)) {
               map_findSelect( sys );
               free(names);
               return 1;
            }
         }
      }
   }

   /* Construct found table. */
   found = NULL;
   n = 0;
   for (int i=0; i<len; i++) {
      const char *sysname;
      StarSystem *sys;

      /* Spob must be real. */
      Spob *spob = spob_get( names[i] );
      if (spob == NULL)
         continue;
      if (!spob_isKnown(spob))
         continue;

      /* System must be known. */
      sysname = spob_getSystem( names[i] );
      if (sysname == NULL)
         continue;
      sys = system_get( sysname );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, spob );
      n++;
   }
   free(names);

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( wid_map_find, found, n );
   return 0;
}

/**
 * @brief Gets a colour char for a spob, simplified for map use.
 */
static char map_getSpobColourChar( Spob *p )
{
   char colcode;

   spob_updateLand(p);
   colcode = spob_getColourChar(p);

   return colcode;
}

/**
 * @brief Gets a symbol for a spob, simplified for map use.
 */
static const char *map_getSpobSymbol( Spob *p )
{
   spob_updateLand(p);
   return spob_getSymbol(p);
}

/**
 * @brief Does fuzzy name matching for outfits in an Array. Searches translated names but returns internal names.
 */
static char **map_fuzzyOutfits( Outfit **o, const char *name )
{
   char **names = array_create( char* );

   /* Do fuzzy search. */
   for (int i=0; i<array_size(o); i++) {
      if (strcasestr( _(o[i]->name), name ) != NULL)
         array_push_back( &names, o[i]->name );
      else if ((o[i]->typename != NULL) && strcasestr( o[i]->typename, name ) != NULL)
         array_push_back( &names, o[i]->name );
      else if ((o[i]->condstr != NULL) && strcasestr( o[i]->condstr, name ) != NULL)
         array_push_back( &names, o[i]->name );
      else if (strcasestr( outfit_description(o[i]), name ) != NULL)
         array_push_back( &names, o[i]->name );
      else if (strcasestr( outfit_summary(o[i], 0), name ) != NULL)
         array_push_back( &names, o[i]->name );
   }

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
   o     = tech_getOutfitArray( map_known_techs, array_size(map_known_techs) );
   names = map_fuzzyOutfits( o, name );
   array_free(o);

   return names;
}
/**
 * @brief Add widgets to the extended area on the outfit search
 *    listpanel.
 *
 *    @param wid_results The ID of the Search Results window we're populating
 *    @param x The x offset where we can start drawing
 *    @param y the y offset where we can start drawing
 *    @param w The width of the area where we can draw
 *    @param h The height of the area where we can draw
 */
static void map_addOutfitDetailFields(unsigned int wid_results, int x, int y, int w, int h)
{
   (void) h;
   (void) y;
   int iw;
   char buf[STRMAX];
   size_t l = 0;

   iw = x;

   window_addRect( wid_results, -1 + iw, -50, 128, 129, "rctImage", &cBlack, 0 );
   window_addImage( wid_results, iw, -50-128, 0, 0, "imgOutfit", NULL, 1 );

   window_addText( wid_results, iw + 128 + 20, -50,
         280, 160, 0, "txtDescShort", &gl_smallFont, NULL, NULL );
   l += scnprintf( &buf[l], sizeof(buf)-l, "#n%s#0\n", _("Owned:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "#n%s#0\n", _("Mass:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "#n%s#0\n", _("Price:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "#n%s#0\n", _("Money:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "#n%s#0\n", _("License:") );
   window_addText( wid_results, iw+20, -50-128-10,
         90, 160, 0, "txtSDesc", &gl_smallFont, NULL, buf );
   window_addText( wid_results, iw+20, -50-128-10,
         w - (20 + iw + 20 + 90), 160, 0, "txtDDesc", &gl_smallFont, NULL, NULL );
   window_addText( wid_results, iw+20, -50-128-10-160,
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
static void map_showOutfitDetail( unsigned int wid, const char* wgtname, int x, int y, int w, int h )
{
   (void) x;
   (void) y;
   (void) h;
   char buf[STRMAX], buf_price[ECON_CRED_STRLEN], buf_money[ECON_CRED_STRLEN], buf_mass[ECON_MASS_STRLEN];
   const Outfit *outfit = outfit_get( map_foundOutfitNames[toolkit_getListPos(wid, wgtname)] );
   size_t l = 0;
   double th;
   int iw;
   double mass = outfit->mass;

   /* 452 px is the sum of the 128 px outfit image width, its 4 px border,
    * a 20 px gap, 280 px for the outfit's name and a final 20 px gap. */
   iw = w - 452;

   window_modifyImage( wid, "imgOutfit", outfit->gfx_store, 128, 128 );
   l = outfit_getNameWithClass( outfit, buf, sizeof(buf) );
   l += scnprintf( &buf[l], sizeof(buf)-l, " %s", pilot_outfitSummary( player.p, outfit, 0 ) );
   window_modifyText( wid, "txtDescShort", buf );
   th = gl_printHeightRaw( &gl_smallFont, 280, buf );

   if (outfit_isLauncher(outfit))
      mass += outfit_amount(outfit) * outfit->u.lau.ammo_mass;
   else if (outfit_isFighterBay(outfit))
      mass += outfit_amount(outfit) * outfit->u.bay.ship_mass;

   window_modifyText( wid, "txtDescription", pilot_outfitDescription( player.p, outfit ) );
   credits2str( buf_price, outfit->price, 2 );
   credits2str( buf_money, player.p->credits, 2 );
   tonnes2str( buf_mass, (int)round( mass ) );

   l = 0;
   l += scnprintf( &buf[l], sizeof(buf)-l, "%d\n", player_outfitOwned(outfit) );
   l += scnprintf( &buf[l], sizeof(buf)-l, "%s\n", buf_mass );
   l += scnprintf( &buf[l], sizeof(buf)-l, "%s\n", buf_price );
   l += scnprintf( &buf[l], sizeof(buf)-l, "%s\n", buf_money );
   l += scnprintf( &buf[l], sizeof(buf)-l, "%s\n" , (outfit->license != NULL) ? _(outfit->license) : _("None") );

   window_modifyText( wid, "txtDDesc", buf );
   window_resizeWidget( wid, "txtDescShort", 280, th );
   window_moveWidget( wid, "txtDescShort", iw + 128 + 20, -50 );
   th = MAX( 128 + gl_smallFont.h, th );
   window_moveWidget( wid, "txtSDesc", iw+20, -50-th );
   window_moveWidget( wid, "txtDDesc", iw+20+90, -50-th );
   th += gl_printHeightRaw( &gl_smallFont, 280, buf );
   window_moveWidget( wid, "txtDescription", iw+20, -50-th );
}

/**
 * @brief Searches for a outfit.
 *
 *    @param name Name to match.
 *    @return 0 on success.
 */
static int map_findSearchOutfits( unsigned int wid_map_find, const char *name )
{
   int len, n;
   map_find_t *found;
   const char *oname, *sysname;
   char **list;
   const Outfit *o;

   assert( "Outfit search is not reentrant!" && map_foundOutfitNames == NULL );

   /* Match spob first. */
   o     = NULL;
   oname = outfit_existsCase( name );
   map_foundOutfitNames = map_outfitsMatch( name );
   len = array_size( map_foundOutfitNames );
   if ((oname != NULL) && (len == 1))
      o = outfit_get( oname );
   /* Do fuzzy match. */
   else if (len > 0) {
      int i;

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
   for (int i=0; i<len; i++) {
      /* Try to find the outfit in the spob. */
      int j;
      Spob *spob;
      StarSystem *sys;
      Outfit **olist = tech_getOutfit( map_known_techs[i] );
      for (j=array_size(olist)-1; j>=0; j--)
         if (olist[j] == o)
            break;
      array_free( olist );
      olist = NULL;
      if (j < 0)
         continue;
      spob = map_known_spobs[i];

      /* Must have an outfitter. */
      if (!spob_hasService(spob,SPOB_SERVICE_OUTFITS))
         continue;

      /* System must be known. */
      sysname = spob_getSystem( spob->name );
      if (sysname == NULL)
         continue;
      sys = system_get( sysname );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, spob );
      n++;
   }

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( wid_map_find, found, n );
   return 0;
}

/**
 * @brief Does fuzzy name matching for ships in an Array. Searches translated names but returns internal names.
 */
static char **map_fuzzyShips( Ship **s, const char *name )
{
   char **names = array_create( char* );

   /* Do fuzzy search. */
   for (int i=0; i<array_size(s); i++) {
      if (strcasestr( _(s[i]->name), name ) != NULL)
         array_push_back( &names, s[i]->name );
      else if ((s[i]->license != NULL) && strcasestr( _(s[i]->license), name ) != NULL)
         array_push_back( &names, s[i]->name );
      else if (strcasestr( _(ship_classDisplay( s[i] )), name ) != NULL)
         array_push_back( &names, s[i]->name );
      else if (strcasestr( _(s[i]->fabricator), name ) != NULL)
         array_push_back( &names, s[i]->name );
      else if (strcasestr( _(s[i]->description), name ) != NULL)
         array_push_back( &names, s[i]->name );
   }

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
static int map_findSearchShips( unsigned int wid_map_find, const char *name )
{
   char **names;
   int len, n;
   map_find_t *found;
   Spob *spob;
   StarSystem *sys;
   const char *sname, *sysname;
   char **list;
   const Ship *s;
   Ship **slist;

   /* Match spob first. */
   s     = NULL;
   sname = ship_existsCase( name );
   names = map_shipsMatch( name );
   len = array_size( names );
   if ((sname != NULL) && (len == 1))
      s = ship_get( sname );
   /* Handle fuzzy matching. */
   else if (len > 0) {
      int i;
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
   for (int i=0; i<len; i++) {
      int j;

      /* Try to find the ship in the spob. */
      slist = tech_getShip( map_known_techs[i] );
      for (j=array_size(slist)-1; j>=0; j--)
         if (slist[j] == s)
            break;
      array_free(slist);
      slist = NULL;
      if (j < 0)
         continue;
      spob = map_known_spobs[i];

      /* Must have an shipyard. */
      if (!spob_hasService(spob,SPOB_SERVICE_SHIPYARD))
         continue;

      /* System must be known. */
      sysname = spob_getSystem( spob->name );
      if (sysname == NULL)
         continue;
      sys = system_get( sysname );
      if (!sys_isKnown(sys))
         continue;

      if (found == NULL) /* Allocate results array on first match. */
         found = malloc( sizeof(map_find_t) * len );

      map_findAccumulateResult( found, n, sys, spob );
      n++;
   }

   /* No visible match. */
   if (n==0)
      return -1;

   /* Display results. */
   map_findDisplayResult( wid_map_find, found, n );
   return 0;
}

/**
 * @brief Does a search.
 */
static void map_findSearch( unsigned int wid_map_find, const char* str )
{
   int ret;
   const char *name, *searchname;

   /* Get the name. */
   name = window_getInput( wid_map_find, "inpSearch" );
   if (name[0] == '\0')
      return;

   /* Prevent reentrancy, e.g. the toolkit spontaneously deciding a future mouseup event was the
    * user releasing the clicked "Find" button and should reactivate it, never mind that they were
    * actually clicking on the dialogue_listPanel we opened to present the results.
    * FIXME: That behavior doesn't seem right, but I'm not sure if it's an actual bug or not. */
   window_disableButton( wid_map_find, "btnSearch" );

   /* Clean up if necessary. */
   free( map_found_cur );
   map_found_cur = NULL;

   /* Handle different search cases. */
   if (map_find_systems) {
      ret = map_findSearchSystems( wid_map_find, name );
      searchname = _("System");
   }
   else if (map_find_spobs) {
      ret = map_findSearchSpobs( wid_map_find, name );
      searchname = _("Space Objects");
   }
   else if (map_find_outfits) {
      ret = map_findSearchOutfits( wid_map_find, name );
      searchname = _("Outfit");
   }
   else if (map_find_ships) {
      ret = map_findSearchShips( wid_map_find, name );
      searchname = _("Ship");
   }
   else
      ret = 1;

   if (ret < 0)
      dialogue_alert( _("%s matching '%s' not found!"), searchname, name );

   /* Safe at last. */
   window_enableButton( wid_map_find, "btnSearch" );

   if (ret > 0)
      window_close( wid_map_find, str );
}

/**
 * @brief Opens a search input box to find a system or spob.
 */
void map_inputFind( unsigned int parent, const char* str )
{
   (void) str;
   unsigned int wid_map_find;
   int x, y, w, h;

   /* initialize known. */
   map_knownInit();

   /* Create the window. */
   w = 400;
   h = 220;
   wid_map_find = window_create( "wdwFind", _("Find…"), -1, -1, w, h );
   window_setAccept( wid_map_find, map_findSearch );
   window_setCancel( wid_map_find, window_close );
   window_setParent( wid_map_find, parent );
   window_onClose( wid_map_find, map_findOnClose );

   /* Text. */
   y = -40;
   window_addText( wid_map_find, 20, y, w - 50, gl_defFont.h+4, 0,
         "txtDescription", &gl_defFont, NULL,
         _("Enter keyword to search for:") );
   y -= 30;

   /* Create input. */
   window_addInput( wid_map_find, 30, y, w - 60, 20,
         "inpSearch", 32, 1, &gl_defFont );
   y -= 40;

   /* Create buttons. */
   window_addButton( wid_map_find, -30, 20+BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSearch", _("Find"), map_findSearch );
   window_addButton( wid_map_find, -30, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", _("Close"), window_close );

   /* Create check boxes. */
   x = 40;
   window_addCheckbox( wid_map_find, x, y, 160, 20,
         "chkSystem", _("Systems"), map_findCheckUpdate, map_find_systems );
   y -= 20;
   window_addCheckbox( wid_map_find, x, y, 160, 20,
         "chkSpob", _("Space Objects"), map_findCheckUpdate, map_find_spobs );
   y -= 20;
   window_addCheckbox( wid_map_find, x, y, 160, 20,
         "chkOutfit", _("Outfits"), map_findCheckUpdate, map_find_outfits );
   y -= 20;
   window_addCheckbox( wid_map_find, x, y, 160, 20,
         "chkShip", _("Ships"), map_findCheckUpdate, map_find_ships );
}
