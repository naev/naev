/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file load.c
 *
 * @brief Contains stuff to load a pilot or look up information about it.
 */

/** @cond */
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "load.h"

#include "array.h"
#include "dialogue.h"
#include "economy.h"
#include "event.h"
#include "faction.h"
#include "difficulty.h"
#include "gui.h"
#include "hook.h"
#include "land.h"
#include "log.h"
#include "menu.h"
#include "mission.h"
#include "news.h"
#include "ndata.h"
#include "nlua_var.h"
#include "nstring.h"
#include "nxml.h"
#include "outfit.h"
#include "player.h"
#include "plugin.h"
#include "render.h"
#include "save.h"
#include "shiplog.h"
#include "start.h"
#include "space.h"
#include "threadpool.h"
#include "toolkit.h"
#include "unidiff.h"
#include "ntracing.h"

#define LOAD_WIDTH      600 /**< Load window width. */
#define LOAD_HEIGHT     530 /**< Load window height. */

#define BUTTON_WIDTH    120 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */

typedef struct player_saves_s {
   char *name;
   nsave_t *saves;
} player_saves_t;

static player_saves_t *load_saves = NULL; /**< Array of saves */
static player_saves_t *load_player = NULL; /**< Points to current element in load_saves. */
static int old_saves_detected = 0, player_warned = 0;
static char *selected_player = NULL;
extern int save_loaded; /**< From save.c */

/*
 * Prototypes.
 */
/* externs */
/* player.c */
extern Spob* player_load( xmlNodePtr parent ); /**< Loads player related stuff. */
/* event.c */
extern int events_loadActive( xmlNodePtr parent );
/* news.c */
extern int news_loadArticles( xmlNodePtr parent );
/* nlua_var.c */
extern int var_load( xmlNodePtr parent ); /**< Loads mission variables. */
/* faction.c */
extern int pfaction_load( xmlNodePtr parent ); /**< Loads faction data. */
/* hook.c */
extern int hook_load( xmlNodePtr parent ); /**< Loads hooks. */
/* space.c */
extern int space_sysLoad( xmlNodePtr parent ); /**< Loads the space stuff. */
/* economy.c */
extern int economy_sysLoad( xmlNodePtr parent ); /**< Loads the economy stuff. */
/* unidiff.c */
extern int diff_load( xmlNodePtr parent ); /**< Loads the universe diffs. */
/* static */
static void load_menu_update( unsigned int wid, const char *str );
static void load_menu_close( unsigned int wdw, const char *str );
static void load_menu_load( unsigned int wdw, const char *str );
static void load_menu_delete( unsigned int wdw, const char *str );
static void load_menu_snapshots( unsigned int wdw, const char *str );
static void load_snapshot_menu_update( unsigned int wid, const char *str );
static void load_snapshot_menu_close( unsigned int wdw, const char *str );
static void load_snapshot_menu_onClose( unsigned int wid, const char *str );
static void load_snapshot_menu_load( unsigned int wdw, const char *str );
static void load_snapshot_menu_delete( unsigned int wdw, const char *str );
static void load_snapshot_menu_save( unsigned int wdw, const char *str );
static void display_save_info( unsigned int wid, const nsave_t *ns );
static void move_old_save( const char *path, const char *fname, const char *ext, const char *new_name );
static int load_load( nsave_t *save );
static int load_game( const nsave_t *ns );
static int load_gameInternal( const char* file, const char* version );
static int load_gameInternalHook( void *data );
static int load_enumerateCallback( void* data, const char* origdir, const char* fname );
static int load_enumerateCallbackPlayer( void* data, const char* origdir, const char* fname );
static int load_compatibilityTest( const nsave_t *ns );
static const char* load_compatibilityString( const nsave_t *ns );
static int has_plugin( const char *plugin );
static SaveCompatibility load_compatibility( const nsave_t *ns );
static int load_sortComparePlayersName( const void *p1, const void *p2 );
static int load_sortComparePlayers( const void *p1, const void *p2 );
static int load_sortCompareName( const void *p1, const void *p2 );
static int load_sortCompare( const void *p1, const void *p2 );
static xmlDocPtr load_xml_parsePhysFS( const char* filename );
static void load_freeSave( nsave_t *ns );

/**
 * @brief Loads an individual save.
 * @param[out] save Structure to populate.
 * @return 0 on success.
 */
static int load_load( nsave_t *save )
{
   xmlDocPtr doc;
   xmlNodePtr root, parent;

   /* Load the XML. */
   doc = load_xml_parsePhysFS( save->path );
   if (doc == NULL) {
      WARN( _("Unable to parse save path '%s'."), save->path);
      return -1;
   }
   root = doc->xmlChildrenNode; /* base node */
   if (root == NULL) {
      WARN( _("Unable to get child node of save '%s'."), save->path);
      xmlFreeDoc(doc);
      return -1;
   }

   /* Iterate inside the naev_save. */
   parent = root->xmlChildrenNode;
   do {
      xml_onlyNodes(parent);

      /* Info. */
      if (xml_isNode(parent, "version")) {
         xmlNodePtr node = parent->xmlChildrenNode;
         do {
            xmlr_strd(node, "naev", save->version);
            xmlr_strd(node, "data", save->data);
         } while (xml_nextNode(node));
         continue;
      }

      else if (xml_isNode(parent, "player")) {
         /* Get name. */
         xmlr_attr_strd(parent, "name", save->player_name);
         /* Parse rest. */
         xmlNodePtr node = parent->xmlChildrenNode;
         do {
            xml_onlyNodes(node);

            /* Player info. */
            xmlr_strd(node, "location", save->spob);
            xmlr_ulong(node, "credits", save->credits);
            xmlr_strd(node, "chapter", save->chapter);
            xmlr_strd(node, "difficulty", save->difficulty);

            /* Time. */
            if (xml_isNode(node, "time")) {
               int cycles, periods, seconds;
               xmlNodePtr cur = node->xmlChildrenNode;
               cycles = periods = seconds = 0;
               do {
                  xmlr_int(cur, "SCU", cycles);
                  xmlr_int(cur, "STP", periods);
                  xmlr_int(cur, "STU", seconds);
               } while (xml_nextNode(cur));
               save->date = ntime_create( cycles, periods, seconds );
               continue;
            }

            /* Ship info. */
            if (xml_isNode(node, "ship")) {
               xmlr_attr_strd(node, "name", save->shipname);
               xmlr_attr_strd(node, "model", save->shipmodel);
               continue;
            }
         } while (xml_nextNode(node));
         continue;
      }
      else if (xml_isNode(parent, "plugins")) {
         save->plugins = array_create( char* );
         /* Parse rest. */
         xmlNodePtr node = parent->xmlChildrenNode;
         do {
            xml_onlyNodes(node);

            if (xml_isNode(node, "plugin")) {
               const char *name = xml_get(node);
               if (name != NULL)
                  array_push_back( &save->plugins, strdup(name) );
               else
                  WARN(_("Save '%s' has unnamed plugin node!"), save->path);
            }
         } while (xml_nextNode(node));
         continue;
      }
   } while (xml_nextNode(parent));

   /* Defaults. */
   if (save->chapter==NULL)
      save->chapter = strdup( start_chapter() );

   save->compatible = load_compatibility( save );

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}

static int load_loadThread( void *ptr )
{
   nsave_t *ns = ptr;
   ns->ret = load_load( ns );
   return ns->ret;
}

/**
 * @brief Loads or refreshes saved games for the player.
 */
int load_refresh (void)
{
   ThreadQueue *tq = vpool_create();

   if (load_saves != NULL)
      load_free();

   /* Load the saves candidates. */
   load_saves = array_create( player_saves_t );
   PHYSFS_enumerate( "saves", load_enumerateCallback, NULL );

   /* Set up threads and load. */
   for (int i=0; i<array_size(load_saves); i++) {
      player_saves_t *ps = &load_saves[i];
      for (int j=0; j<array_size(ps->saves); j++) {
         nsave_t *ns = &ps->saves[j];
         vpool_enqueue( tq, load_loadThread, ns );
      }
   }
   vpool_wait( tq );
   vpool_cleanup( tq );

   /* Load the saves. */
   for (int i=array_size(load_saves)-1; i>=0; i--) {
      player_saves_t *ps = &load_saves[i];
      for (int j=array_size(ps->saves)-1; j>=0; j--) {
         const nsave_t *ns = &ps->saves[j];
         if (ns->ret!=0) {
            free( ns->path );
            free( ns->save_name );
            array_erase( &ps->saves, &ps->saves[j], &ps->saves[j+1] );
            continue;
         }
         if (ps->name == NULL)
            ps->name = strdup( ns->player_name );
      }
      if (ps->name==NULL)
         array_erase( &load_saves, &load_saves[i], &load_saves[i+1] );
   }

   /* Sort and done. */
   for (int i=0; i<array_size(load_saves); i++) {
      player_saves_t *ps = &load_saves[i];
      qsort( ps->saves, array_size(ps->saves), sizeof(nsave_t), load_sortCompare );
   }

   /* Dedup as necessary, this can be caused by some OS secretly renaming files
    * when creating. In particular, windows seems to dislike directory names
    * ending with a '.' which can cause the player save directory to mismatch
    * the player save name.. */
   qsort( load_saves, array_size(load_saves), sizeof(player_saves_t), load_sortComparePlayersName );
   for (int i=array_size(load_saves)-1; i>0; i--) {
      player_saves_t *ps = &load_saves[i];
      if (strcmp(ps->name, ps[-1].name)!=0)
         continue;

      /* Copy saves over. */
      for (int j=0; j<array_size(ps->saves); j++) {
         const nsave_t *ns = &ps->saves[j];
         array_push_back( &ps[-1].saves, *ns );
      }

      /* Now have to dedup saves. */
      qsort( ps[-1].saves, array_size(ps[-1].saves), sizeof(nsave_t), load_sortCompareName );
      for (int j=array_size(ps[-1].saves)-1; j>0; j--) {
         nsave_t *ns = &ps[-1].saves[j];
         if (strcmp(ns->save_name, ns[-1].save_name)!=0)
            continue;

         load_freeSave( ns );
         array_erase( &ps[-1].saves, ns, &ns[1] );
      }

      /* Properly resort saves. */
      qsort( ps[-1].saves, array_size(ps[-1].saves), sizeof(nsave_t), load_sortCompare );

      /* Erase current iterator. */
      array_free( ps->saves );
      free( ps->name );
      array_erase( &load_saves, ps, &ps[1] );
   }

   /* Resort based on time. */
   qsort( load_saves, array_size(load_saves), sizeof(player_saves_t), load_sortComparePlayers );

   return 0;
}

static int load_enumerateCallbackPlayer( void* data, const char* origdir, const char* fname )
{
   char *path;
   const char *fmt;
   size_t dir_len;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );

   fmt = dir_len && origdir[dir_len-1]=='/' ? "%s%s" : "%s/%s";
   SDL_asprintf( &path, fmt, origdir, fname );
   if (!PHYSFS_stat( path, &stat )) {
      WARN( _("PhysicsFS: Cannot stat %s: %s"), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      free( path );
   }
   else if (stat.filetype == PHYSFS_FILETYPE_REGULAR) {
      player_saves_t *ps = (player_saves_t*) data;
      nsave_t ns;
      memset( &ns, 0, sizeof(ns) );
      ns.path = path;
      ns.save_name = strdup( fname );
      ns.save_name[ strlen(ns.save_name)-3 ] = '\0';
      ns.modtime = stat.modtime;
      array_push_back( &ps->saves, ns );
   }
   else
      free( path );

   return PHYSFS_ENUM_OK;
}

/**
 * @brief The PHYSFS_EnumerateCallback for load_refresh
 */
static int load_enumerateCallback( void* data, const char* origdir, const char* fname )
{
   (void) data;
   char *path, *backup_path;
   const char *fmt;
   size_t dir_len, name_len;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );
   name_len = strlen( fname );

   fmt = dir_len && origdir[dir_len-1]=='/' ? "%s%s" : "%s/%s";
   SDL_asprintf( &path, fmt, origdir, fname );
   if (!PHYSFS_stat( path, &stat ))
      WARN( _("PhysicsFS: Cannot stat %s: %s"), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
   /* TODO remove this sometime in the future. Maybe 0.12.0 or 0.13.0? */
   else if (stat.filetype == PHYSFS_FILETYPE_REGULAR) {
      if ((name_len < 4 || strcmp( &fname[name_len-3], ".ns" )) && (name_len < 11 || strcmp( &fname[name_len-10], ".ns.backup" ))) {
         free( path );
         return PHYSFS_ENUM_OK;
      }
      if (!PHYSFS_exists( "saves-pre-0.10.0" ))
         PHYSFS_mkdir( "saves-pre-0.10.0" );
      SDL_asprintf( &backup_path, "saves-pre-0.10.0/%s", fname );
      if (!ndata_copyIfExists( path, backup_path ))
         old_saves_detected = 1;
      free( backup_path );
      move_old_save( path, fname, ".ns", "autosave.ns" );
      move_old_save( path, fname, ".ns.backup", "backup.ns" );
   }
   else if (stat.filetype == PHYSFS_FILETYPE_DIRECTORY) {
      player_saves_t psave;
      psave.name = NULL;
      psave.saves = array_create( nsave_t );
      PHYSFS_enumerate( path, load_enumerateCallbackPlayer, &psave );
      array_push_back( &load_saves, psave );
   }

   free( path );
   return PHYSFS_ENUM_OK;
}

static int load_compatibilityTest( const nsave_t *ns )
{
   char buf[STRMAX], buf2[STRMAX];
   const plugin_t *plugins = plugin_list();
   int l;

   switch (ns->compatible) {
      case SAVE_COMPATIBILITY_NAEV_VERSION:
         if (!dialogue_YesNo( _("Save game version mismatch"),
               _("Save game '%s' version does not match Naev version:\n"
               "   Save version: #r%s#0\n"
               "   Naev version: %s\n"
               "Are you sure you want to load this game? It may lose data."),
               ns->player_name, ns->version, naev_version( 0 ) ))
            return -1;
         break;

      case SAVE_COMPATIBILITY_PLUGINS:
         buf[0] = '\0';
         l = 0;
         for (int i=0; i<array_size(ns->plugins); i++)
            l += scnprintf( &buf[l], sizeof(buf)-l, "%s%s", (l>0)?p_("plugins", ", "):"#r", ns->plugins[i] );
         l += scnprintf( &buf[l], sizeof(buf)-l, "#0" );
         buf2[0] = '\0';
         l = 0;
         for (int i=0; i<array_size(plugins); i++)
            l += scnprintf( &buf2[l], sizeof(buf2)-l, "%s%s", (l>0)?p_("plugins", ", "):"", plugin_name(&plugins[i]) );
         if (!dialogue_YesNo( _("Save game plugin mismatch"),
               _("Save game '%s' plugins do not match loaded plugins:\n"
               "   Save plugins: %s\n"
               "   Naev plugins: %s\n"
               "Are you sure you want to load this game? It may lose data."),
               ns->player_name, buf, buf2 ) )
            return -1;
         break;

      case SAVE_COMPATIBILITY_OK:
         break;
   }

   return 0;
}

static const char* load_compatibilityString( const nsave_t *ns )
{
   switch (ns->compatible) {
      case SAVE_COMPATIBILITY_NAEV_VERSION:
         return _("version mismatch");

      case SAVE_COMPATIBILITY_PLUGINS:
         return _("plugins mismatch");

      case SAVE_COMPATIBILITY_OK:
         return _("compatible");
   }
   return NULL;
}

/**
 * @brief Checks to see if has a plugin.
 */
static int has_plugin( const char *plugin )
{
   const plugin_t *plugins = plugin_list();
   for (int j=0; j<array_size(plugins); j++)
      if (strcmp( plugin, plugin_name(&plugins[j]) )==0)
         return 1;
   return 0;
}

/**
 * @brief Checks to see if a save is compatible with current Naev.
 */
static SaveCompatibility load_compatibility( const nsave_t *ns )
{
   int diff = naev_versionCompare( ns->version );

   if (ABS(diff) >= 2)
      return SAVE_COMPATIBILITY_NAEV_VERSION;

   for (int i=0; i<array_size(ns->plugins); i++) {
      if (!has_plugin( ns->plugins[i] ))
         return SAVE_COMPATIBILITY_PLUGINS;
   }

   return SAVE_COMPATIBILITY_OK;
}

/**
 * @brief qsort compare function for files.
 */
static int load_sortComparePlayersName( const void *p1, const void *p2 )
{
   const player_saves_t *ps1, *ps2;
   ps1 = (const player_saves_t*) p1;
   ps2 = (const player_saves_t*) p2;
   return strcmp( ps1->name, ps2->name );
}

/**
 * @brief qsort compare function for files.
 */
static int load_sortComparePlayers( const void *p1, const void *p2 )
{
   const player_saves_t *ps1, *ps2;
   ps1 = (const player_saves_t*) p1;
   ps2 = (const player_saves_t*) p2;
   return load_sortCompare( &ps1->saves[0], &ps2->saves[0] );
}

static int load_sortCompareName( const void *p1, const void *p2 )
{
   int ret;
   const nsave_t *ns1 = (const nsave_t*) p1;
   const nsave_t *ns2 = (const nsave_t*) p2;
   ret = strcmp( ns1->save_name, ns2->save_name );
   if (ret)
      return ret;

   /* Sort by compatibility first. */
   if (!ns1->compatible && ns2->compatible)
      return -1;
   else if (ns1->compatible && !ns2->compatible)
      return +1;

   /* Search by file modification date. */
   if (ns1->modtime > ns2->modtime)
      return -1;
   else if (ns1->modtime < ns2->modtime)
      return +1;

   return 0;
}

/**
 * @brief qsort compare function for files.
 */
static int load_sortCompare( const void *p1, const void *p2 )
{
   const nsave_t *ns1, *ns2;
   ns1 = (const nsave_t*) p1;
   ns2 = (const nsave_t*) p2;

   /* Sort by compatibility first. */
   if (!ns1->compatible && ns2->compatible)
      return -1;
   else if (ns1->compatible && !ns2->compatible)
      return +1;

   /* Search by file modification date. */
   if (ns1->modtime > ns2->modtime)
      return -1;
   else if (ns1->modtime < ns2->modtime)
      return +1;

   /* Finally sort by name. */
   return strcmp( ns1->save_name, ns2->save_name );
}

static void load_freeSave( nsave_t *ns )
{
   for (int k=0; k<array_size(ns->plugins); k++)
      free( ns->plugins[k] );
   array_free( ns->plugins );
   free(ns->save_name);
   free(ns->player_name);
   free(ns->path);
   free(ns->version);
   free(ns->data);
   free(ns->spob);
   free(ns->chapter);
   free(ns->difficulty);
   free(ns->shipname);
   free(ns->shipmodel);
}

/**
 * @brief Frees loaded save stuff.
 */
void load_free (void)
{
   for (int i=0; i<array_size(load_saves); i++) {
      player_saves_t *ps = &load_saves[i];
      free( ps->name );
      for (int j=0; j<array_size(ps->saves); j++) {
         load_freeSave( &ps->saves[j] );
      }
      array_free( ps->saves );
   }
   array_free( load_saves );
   load_saves = NULL;
}

/**
 * @brief Gets the array (array.h) of loaded saves.
 */
const nsave_t *load_getList( const char *name )
{
   if (!array_size(load_saves))
      return NULL;
   if (name==NULL)
      return load_saves[0].saves;
   for (int i=0; i<array_size(load_saves); i++)
      if (strcmp(load_saves[i].name,name)==0)
         return load_saves[i].saves;
   return NULL;
}

/**
 * @brief Opens the load game menu.
 */
void load_loadGameMenu (void)
{
   unsigned int wid;
   char **names;
   int n;
   int pos = 0;

   /* window */
   wid = window_create( "wdwLoadGameMenu", _("Load Pilot"), -1, -1, LOAD_WIDTH, LOAD_HEIGHT );
   window_setAccept( wid, load_menu_load );
   window_setCancel( wid, load_menu_close );

   load_refresh();

   n = array_size( load_saves );
   if (n > 0) {
      names = malloc( sizeof(char*)*n );
      for (int i=0; i<n; i++) {
         nsave_t *ns = &load_saves[i].saves[0];
         if (ns->compatible) {
            char buf[STRMAX_SHORT];
            scnprintf( buf, sizeof(buf), _("%s (#r%s#0)"),
               ns->player_name, load_compatibilityString( ns ) );
            names[i] = strdup( buf );
         }
         else
            names[i] = strdup( ns->player_name );
         if (selected_player != NULL && !strcmp( names[i], selected_player ))
            pos = i;
      }
   }
   /* case there are no players */
   else {
      names = malloc(sizeof(char*));
      names[0] = strdup(_("None"));
      n = 1;
   }

   /* Player text. */
   window_addText( wid, -20, -40, BUTTON_WIDTH*2+20, LOAD_HEIGHT-40-20-2*(BUTTON_HEIGHT+20),
         0, "txtPilot", &gl_smallFont, NULL, NULL );

   window_addList( wid, 20, -40,
         LOAD_WIDTH-BUTTON_WIDTH*2-80, LOAD_HEIGHT-110,
         "lstNames", names, n, pos, load_menu_update, load_menu_load );

   /* Buttons */
   window_addButtonKey( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", _("Back"), load_menu_close, SDLK_b );
   window_addButtonKey( wid, -20-BUTTON_WIDTH-20, 20 + BUTTON_HEIGHT+15, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSnapshots", _("Snapshots"), load_menu_snapshots, SDLK_s );
   window_addButtonKey( wid, -20, 20 + BUTTON_HEIGHT+15, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", _("Load"), load_menu_load, SDLK_l );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDelete", _("Delete"), load_menu_delete );

   if (old_saves_detected && !player_warned) {
      char buf[STRMAX_SHORT];
      snprintf( buf, sizeof(buf), "%s%s", PHYSFS_getRealDir("saves-pre-0.10.0"), "saves-pre-0.10.0" );
      dialogue_alert( _("Naev has detected saves in pre-0.10.0 format, and has automatically migrated them to the new format. Old saves have been backed up at '%s'."), buf );
      player_warned = 1;
   }
}

static void load_snapshot_menu_onClose( unsigned int wid, const char *str )
{
   (void) str;
   free( window_getData( wid ) );
}

/**
 * @brief Opens the load snapshot menu.
 *    @param name Player's name.
 *    @param disablesave Forcibly disable saving.
 */
void load_loadSnapshotMenu( const char *name, int disablesave )
{
   unsigned int wid;
   char **names;
   player_saves_t *ps;
   int n;
   char *t;
   int *data;

   ps = NULL;
   for (int i=0; i<array_size(load_saves); i++) {
      if (strcmp(load_saves[i].name, name)==0) {
         ps = &load_saves[i];
         break;
      }
   }
   if (ps==NULL) {
      WARN(_("Player '%s' not found in list of saves!"),name);
      return;
   }
   load_player = ps;

   t = strdup( name );
   free( selected_player );
   selected_player = t;

   /* window */
   wid = window_create( "wdwLoadSnapshotMenu", _("Load Snapshot"), -1, -1, LOAD_WIDTH, LOAD_HEIGHT );
   window_setAccept( wid, load_snapshot_menu_load );
   window_setCancel( wid, load_snapshot_menu_close );

   data = malloc( sizeof( int ) );
   *data = disablesave;
   window_setData( wid, data );
   window_onClose( wid, load_snapshot_menu_onClose );

   /* load the saves */
   n = array_size( ps->saves );
   if (n > 0) {
      names = malloc( sizeof(char*)*n );
      for (int i=0; i<n; i++) {
         nsave_t *ns = &ps->saves[i];
         if (ns->compatible) {
            char buf[STRMAX_SHORT];
            scnprintf( buf, sizeof(buf), _("%s (#r%s#0)"),
               ns->save_name, load_compatibilityString( ns ) );
            names[i] = strdup( buf );
         }
         else
            names[i] = strdup( ns->save_name );
      }
   }
   /* case there are no files */
   else {
      names = malloc(sizeof(char*));
      names[0] = strdup(_("None"));
      n     = 1;
   }

   /* Player text. */
   window_addText( wid, -20, -40, BUTTON_WIDTH*2+20, LOAD_HEIGHT-40-20-2*(BUTTON_HEIGHT+20),
         0, "txtPilot", &gl_smallFont, NULL, NULL );

   window_addList( wid, 20, -40,
         LOAD_WIDTH-BUTTON_WIDTH*2-80, LOAD_HEIGHT-110,
         "lstSaves", names, n, 0, load_snapshot_menu_update, load_snapshot_menu_load );

   /* Buttons */
   window_addButtonKey( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", _("Back"), load_snapshot_menu_close, SDLK_b );
   window_addButtonKey( wid, -20-BUTTON_WIDTH-20, 20 + BUTTON_HEIGHT+15, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSave", _("Save As"), load_snapshot_menu_save, SDLK_s );
   window_addButtonKey( wid, -20, 20 + BUTTON_HEIGHT+15, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", _("Load"), load_snapshot_menu_load, SDLK_l );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDelete", _("Delete"), load_snapshot_menu_delete );

   if (disablesave || window_exists( "wdwLoadGameMenu" ))
      window_disableButton( wid, "btnSave" );
   else {
      int can_save = landed && !player_isFlag(PLAYER_NOSAVE);
      if (!can_save)
         window_disableButton( wid, "btnSave" );
   }
}

/**
 * @brief Opens the load snapshot menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_snapshots( unsigned int wdw, const char *str )
{
   (void) str;
   int pos = toolkit_getListPos( wdw, "lstNames" );
   if (array_size(load_saves) <= 0)
      return;
   load_loadSnapshotMenu( load_saves[pos].name, 1 );
}

/**
 * @brief Creates new custom snapshot.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_snapshot_menu_save( unsigned int wdw, const char *str )
{
   char *save_name = dialogue_input( _("Save game"), 1, 60, _("Please give the new snapshot a name:") );
   if (save_name == NULL)
      return;
   char path[PATH_MAX];
   snprintf(path, sizeof(path), "saves/%s/%s.ns", player.name, save_name);
   if (PHYSFS_exists( path )) {
      int r = dialogue_YesNo(_("Overwrite"),
         _("You already have a snapshot named '%s'. Overwrite?"), save_name);
      if (r==0) {
         load_snapshot_menu_save( wdw, str );
         free( save_name );
         return;
      }
   }
   if (save_all_with_name(save_name) < 0)
      dialogue_alert( _("Failed to save the game! You should exit and check the log to see what happened and then file a bug report!") );
   else {
      load_refresh();
      int disablesave = * (int *) window_getData( wdw );
      load_snapshot_menu_close( wdw, str );
      load_loadSnapshotMenu( player.name, disablesave );
   }
   free( save_name );
}

/**
 * @brief Closes the load game menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_close( unsigned int wdw, const char *str )
{
   (void)str;
   window_destroy( wdw );
}

/**
 * @brief Closes the load snapshot menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_snapshot_menu_close( unsigned int wdw, const char *str )
{
   (void)str;
   window_destroy( wdw );
}

/**
* @brief Displays Naev save info.
*    @param wid Widget for displaying save info.
*    @param ns Naev save.
*/
static void display_save_info( unsigned int wid, const nsave_t *ns )
{
   char buf[STRMAX_SHORT], credits[ECON_CRED_STRLEN], date[64], difficulty[STRMAX_SHORT];
   size_t l = 0;

   if (ns->difficulty == NULL) {
      const Difficulty *d = difficulty_cur();
      snprintf( difficulty, sizeof(difficulty), _("%s (options)"), _(d->name) );
   }
   else
      snprintf( difficulty, sizeof(difficulty), _("%s (this save)"), _(ns->difficulty) );

   credits2str( credits, ns->credits, 2 );
   ntime_prettyBuf( date, sizeof(date), ns->date, 2 );
   l += scnprintf( &buf[l], sizeof(buf)-l, "#n%s", _("Name:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", ns->player_name );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Version:") );
   if (ns->compatible == SAVE_COMPATIBILITY_NAEV_VERSION)
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n   #r%s#0", ns->version );
   else
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", ns->version );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Difficulty:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", difficulty );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Date:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", date );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Chapter:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", ns->chapter );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Space Object:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", _(ns->spob) );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Credits:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", credits );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Ship Name:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", ns->shipname );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Ship Model:") );
   l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   %s", _(ns->shipmodel) );
   if (array_size(ns->plugins)>0) {
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n#n%s", _("Plugins:") );
      l += scnprintf( &buf[l], sizeof(buf)-l, "\n#0   #%c%s#0", has_plugin(ns->plugins[0]) ? '0' : 'r', ns->plugins[0] );
      for (int i=1; i<array_size(ns->plugins); i++) {
         l += scnprintf( &buf[l], sizeof(buf)-l, p_("plugins list",", #%c%s#0"),
               has_plugin(ns->plugins[i]) ? '0' : 'r', ns->plugins[i] );
      }
   }
   window_modifyText( wid, "txtPilot", buf );
}

/**
* @brief Moves old Naev saves to subdirectories.
*    @param path Path to old file.
*    @param fname Old filename.
*    @param ext Extension of file to move.
*    @param new_name Name for file in subdirectory.
*/
static void move_old_save( const char *path, const char *fname, const char *ext, const char *new_name )
{
   size_t name_len = strlen(fname);
   size_t ext_len = strlen(ext);
   if (name_len >= ext_len + 1 && !strcmp( &fname[name_len - ext_len], ext )) {
      char *new_path;
      char *dirname = strdup( fname );
      dirname[name_len - ext_len] = '\0';
      SDL_asprintf( &new_path, "saves/%s", dirname );
      if (!PHYSFS_exists( new_path ))
         PHYSFS_mkdir( new_path );
      free( new_path );
      SDL_asprintf( &new_path, "saves/%s/%s", dirname, new_name );
      /* If it's going to overwrite a file, try to back it up. */
      if (PHYSFS_exists( new_path )) {
         int tries = 0;
         char *bkp_path;
         SDL_asprintf( &bkp_path, "%s.bkp", new_path );
         while (PHYSFS_exists(bkp_path) && (tries++ < 10)) {
            char *bkp_bkp_path;
            SDL_asprintf( &bkp_bkp_path, "%s.bkp", bkp_path );
            free( bkp_path );
            bkp_path = bkp_bkp_path;
         }
         ndata_copyIfExists( new_path, bkp_path );
         free( bkp_path );
      }
      /* Copy over the old file. */
      if (!ndata_copyIfExists( path, new_path ))
         if (!PHYSFS_delete( path ))
            dialogue_alert( _("Unable to delete %s"), path );
      free( new_path );
      free( dirname );
   }
}

/**
 * @brief Updates the load menu.
 *    @param wid Widget triggering function.
 *    @param str Unused.
 */
static void load_menu_update( unsigned int wid, const char *str )
{
   (void) str;
   int pos;
   const nsave_t *ns;

   /* Make sure list is ok. */
   if (array_size( load_saves ) <= 0)
      return;

   /* Get position. */
   pos = toolkit_getListPos( wid, "lstNames" );
   ns = &load_saves[pos].saves[0];
   if (selected_player != NULL)
      free( selected_player );
   selected_player = strdup( load_saves[pos].name );

   display_save_info( wid, ns );
}

/**
 * @brief Updates the load snapshot menu.
 *    @param wid Widget triggering function.
 *    @param str Unused.
 */
static void load_snapshot_menu_update( unsigned int wid, const char *str )
{
   (void) str;
   int pos;
   nsave_t *ns;

   /* Make sure list is ok. */
   if (array_size( load_player->saves ) <= 0)
      return;

   /* Get position. */
   pos = toolkit_getListPos( wid, "lstSaves" );
   ns  = &load_player->saves[pos];

   display_save_info( wid, ns );
}

/**
 * @brief Loads a new game.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_load( unsigned int wdw, const char *str )
{
   (void) str;
   int pos;
   unsigned int wid;
   nsave_t *ns;

   wid = window_get( "wdwLoadGameMenu" );
   pos = toolkit_getListPos( wid, "lstNames" );

   if (array_size(load_saves) <= 0)
      return;

   ns = &load_saves[pos].saves[0];

   /* Check version. */
   if (load_compatibilityTest( ns ))
      return;

   /* Close menus before loading for proper rendering. */
   load_menu_close(wdw, NULL);

   /* Close the main menu. */
   menu_main_close();

   /* Try to load the game. */
   if (load_game( ns )) {
      /* Failed so reopen both. */
      menu_main();
      load_loadGameMenu();
   }
}

/**
 * @brief Loads a new game.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_snapshot_menu_load( unsigned int wdw, const char *str )
{
   (void) str;
   int wid, pos;

   wid = window_get( "wdwLoadSnapshotMenu" );

   if (array_size( load_player->saves ) <= 0)
      return;

   pos = toolkit_getListPos( wid, "lstSaves" );

   /* Check version. */
   if (load_compatibilityTest( &load_player->saves[pos]  ))
      return;

   /* Close menus before loading for proper rendering. */
   load_snapshot_menu_close(wdw, NULL);

   if (window_exists( "wdwLoadGameMenu" )) {
      /* Close the main and the load menu. */
      load_menu_close( window_get( "wdwLoadGameMenu" ), NULL );
      menu_main_close();
   }
   else
      menu_small_close();

   /* Try to load the game. */
   if (load_game( &load_player->saves[pos] )) {
      /* Failed so reopen both. */
      /* TODO how to handle failure here? It can happen at many different points now. */
      /*menu_main();
      load_loadGameMenu();*/
   }
}

static void load_menu_delete( unsigned int wdw, const char *str )
{
   unsigned int wid;
   int n, pos;
   char path[PATH_MAX];

   wid = window_get( "wdwLoadGameMenu" );
   pos = toolkit_getListPos( wid, "lstNames" );

   if (array_size(load_saves) <= 0)
      return;

   if (dialogue_YesNo( _("Permanently Delete?"),
      _("Are you sure you want to permanently delete the character '%s'?\n#rThis is an undoable action!#0"), load_saves[pos].name) == 0)
      return;

   /* Remove it. */
   n = array_size( load_saves[pos].saves );
   for (int i = 0; i < n; i++)
      if (!PHYSFS_delete( load_saves[pos].saves[i].path ))
         dialogue_alert( _("Unable to delete %s"), load_saves[pos].saves[i].path );
   snprintf(path, sizeof(path), "saves/%s", load_saves[pos].name);
   if (!PHYSFS_delete( path ))
      dialogue_alert( _("Unable to delete '%s' directory"), load_saves[pos].name );

   load_refresh();

   /* need to reload the menu */
   load_menu_close( wdw, str );
   load_loadGameMenu();
}

/**
 * @brief Deletes an old game.
 *    @param wdw Window to delete.
 *    @param str Unused.
 */
static void load_snapshot_menu_delete( unsigned int wdw, const char *str )
{
   int pos, last_save;
   unsigned int wid = window_get( "wdwLoadSnapshotMenu" );

   if (array_size(load_player->saves) <= 0)
      return;

   pos = toolkit_getListPos( wid, "lstSaves" );

   if (dialogue_YesNo( _("Permanently Delete?"),
      _("Are you sure you want to permanently delete the snapshot '%s'?\n#rThis is an undoable action!#0"), load_player->saves[pos].save_name) == 0)
      return;

   /* Remove it. */
   if (!PHYSFS_delete( load_player->saves[pos].path ))
      dialogue_alert( _("Unable to delete %s"), load_player->saves[pos].path );
   last_save = (array_size(load_player->saves) <= 1);

   /* Delete directory if all are gone. */
   if (last_save) {
      char path[PATH_MAX];
      snprintf(path, sizeof(path), "saves/%s", load_player->name);
      if (!PHYSFS_delete( path ))
         dialogue_alert( _("Unable to delete '%s' directory"), load_player->name );
   }

   load_refresh();

   /* need to reload the menu */
   int disablesave = * (int *) window_getData( wdw );
   load_snapshot_menu_close( wdw, str );
   if (window_exists( "wdwLoadGameMenu" )) {
      wid = window_get( "wdwLoadGameMenu" );
      load_menu_close( wid, str );
      load_loadGameMenu();
   }
   if (!last_save)
      load_loadSnapshotMenu( selected_player, disablesave );
}

static void load_compatSlots (void)
{
   /* Vars for loading old saves. */
   char **sships;
   glTexture **tships;
   int nships;
   Pilot *ship;

   nships = player_nships();
   sships = malloc(nships * sizeof(char*));
   tships = malloc(nships * sizeof(glTexture*));
   nships = player_ships( sships, tships );
   ship   = player.p;
   for (int i=-1; i<nships; i++) {
      if (i >= 0)
         ship = player_getShip( sships[i] );
      /* Remove all outfits. */
      for (int j=0; j<array_size(ship->outfits); j++) {
         ShipOutfitSlot *sslot;

         if (ship->outfits[j]->outfit != NULL) {
            player_addOutfit( ship->outfits[j]->outfit, 1 );
            pilot_rmOutfitRaw( ship, ship->outfits[j] );
         }

         /* Add default outfit. */
         sslot = ship->outfits[j]->sslot;
         if (sslot->data != NULL)
            pilot_addOutfitRaw( ship, sslot->data, ship->outfits[j] );
      }

      pilot_calcStats( ship );
   }

   /* Clean up. */
   for (int i=0; i<nships; i++)
      free(sships[i]);
   free(sships);
   free(tships);
}

/**
 * @brief Loads the diffs from game file.
 *
 *    @param file PhysicsFS path (i.e., relative path starting with "saves/").
 *    @return 0 on success.
 */
int load_gameDiff( const char* file )
{
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Make sure it exists. */
   if (!PHYSFS_exists( file )) {
      dialogue_alert( _("Saved game file seems to have been deleted.") );
      return -1;
   }

   /* Load the XML. */
   doc = load_xml_parsePhysFS( file );
   if (doc == NULL)
      goto err;
   node  = doc->xmlChildrenNode; /* base node */
   if (node == NULL)
      goto err_doc;

   /* Diffs should be cleared automatically first. */
   diff_load(node);

   /* Free. */
   xmlFreeDoc(doc);

   return 0;

err_doc:
   xmlFreeDoc(doc);
err:
   WARN( _("Saved game '%s' invalid!"), file);
   return -1;
}

/**
 * @brief Loads the game from a file.
 *
 *    @param file PhysicsFS path (i.e., relative path starting with "saves/").
 *    @return 0 on success
 */
int load_gameFile( const char *file )
{
   return load_gameInternal( file, naev_version(0) );
}

/**
 * @brief Actually loads a new game based on save structure.
 *
 *    @param ns Save game to load.
 *    @return 0 on success.
 */
static int load_game( const nsave_t *ns )
{
   return load_gameInternal( ns->path, ns->version );
}

/**
 * @brief Actually loads a new game.
 *
 *    @param file PhysicsFS path (i.e., relative path starting with "saves/").
 *    @param version Version string of game to load.
 *    @return 0 on success.
 */
static int load_gameInternal( const char* file, const char* version )
{
   const char** data;

   /* Make sure it exists. */
   if (!PHYSFS_exists( file )) {
      dialogue_alert( _("Saved game file seems to have been deleted.") );
      return -1;
   }

#if HAVE_TRACY
   char buf[STRMAX_SHORT];
   size_t l = snprintf( buf, sizeof(buf), "Loading save '%s'", file );
   NTracingMessage( buf, l );
#endif /* TRACY */

   /* Some global cleaning has to be done here. */
   toolkit_closeAll();
   hook_clear();

   data = malloc( sizeof(const char*) * 2 );
   data[0] = file;
   data[1] = version;
   /* If the player isn't loaded, hooks aren't run so we can just go right away. */
   if ((player.p == NULL) || player_isFlag(PLAYER_DESTROYED)) /* same condition in hook.c */
      return load_gameInternalHook( data );
   else {
      /* Break out of potential inner loops. */
      SDL_Event event;
      SDL_memset( &event, 0, sizeof(event) );
      event.type = SDL_LOOPDONE;
      SDL_PushEvent( &event );

      /* Delay one frame. */
      hook_addFunc( load_gameInternalHook, data, "safe" );
   }
   return 0;
}

/**
 * @brief Loads a game. Meant to be run in a function hook.
 */
static int load_gameInternalHook( void *data )
{
   xmlNodePtr node;
   xmlDocPtr doc;
   Spob *pnt;
   int misn_failed = 0, evt_failed = 0;
   const char **sdata = data;
   const char *file = sdata[0];
   const char *version = sdata[1];
   int version_diff = (version!=NULL) ? naev_versionCompare(version) : 0;
   free(data);

   /* Load the XML. */
   doc = load_xml_parsePhysFS( file );
   if (doc == NULL)
      goto err;
   node  = doc->xmlChildrenNode; /* base node */
   if (node == NULL)
      goto err_doc;

   /* Clean up possible stuff that should be cleaned. */
   unidiff_universeDefer( 1 );
   player_cleanup();
   render_postprocessCleanup();

   /* Welcome message - must be before space_init. */
   player_message( _("#gWelcome to %s!"), APPNAME );
   player_message( "#g v%s", naev_version(0) );

   /* Now begin to load. */
   diff_load(node); /* Must load first to work properly. */
   unidiff_universeDefer( 0 );
   missions_loadCommodity(node); /* Must be loaded before player. */
   pfaction_load(node); /* Must be loaded before player so the messages show up properly. */
   pnt = player_load(node);
   player.loaded_version = strdup( (version!=NULL) ? version : naev_version(0) );

   /* Sanitize for new version. */
   if (version_diff <= -2) {
      WARN( _("Old version detected. Sanitizing ships for slots") );
      load_compatSlots();
   }

   /* Load more stuff. */
   space_sysLoad(node);
   var_load(node);
   misn_failed = missions_loadActive(node);
   evt_failed = events_loadActive(node);
   news_loadArticles( node );
   hook_load(node);

   /* Initialize the economy. */
   economy_init();
   economy_sysLoad(node);

   /* Initialise the ship log */
   shiplog_new();
   shiplog_load(node);

   /* Check validity. */
   event_checkValidity();

   /* Run the load event trigger. */
   events_trigger( EVENT_TRIGGER_LOAD );

   /* Create escorts in space. */
   player_addEscorts();

   /* Load the GUI. */
   if (gui_load( gui_pick() ))
      gui_load( start_gui() ); /* Failed so use fallback... */

   /* Land the player. */
   land( pnt, 1 );

   /* Sanitize the GUI. */
   gui_setCargo();
   gui_setShip();

   xmlFreeDoc(doc);

   if (misn_failed || evt_failed) {
      char buf[STRMAX];
      unsigned int l = 0;
      const char **misn_failed_str = mission_loadFailed();
      l += scnprintf( &buf[l], sizeof(buf)-l, _("Saved game '%s' failed to load some missions/events properly!"), file);
      if (misn_failed) {
         l += scnprintf( &buf[l], sizeof(buf)-l, _("\nIn particular, the following missions have failed to load and been removed:"));
         for (int i=0; i<array_size(misn_failed_str); i++)
            l += scnprintf( &buf[l], sizeof(buf)-l, _("\n   #r%s#0"), misn_failed_str[i]);
      }
      /*l +=*/ scnprintf( &buf[l], sizeof(buf)-l, _("\nNote that, in general, you should be able to find the missions/events again and start them without penalty."));
      dialogue_alertRaw( buf );
   }

   /* Set loaded. */
   save_loaded = 1;

   return 0;

err_doc:
   xmlFreeDoc(doc);
err:
   dialogue_alert( _("Saved game '%s' invalid!"), file);
   menu_main();
   return -1;
}

/**
 * @brief Temporary (hopefully) wrapper around xml_parsePhysFS in support of gzipped XML (like .ns files).
 */
static xmlDocPtr load_xml_parsePhysFS( const char* filename )
{
   char buf[PATH_MAX];
   snprintf( buf, sizeof(buf), "%s/%s", PHYSFS_getWriteDir(), filename );
   return xmlParseFile( buf );
}
