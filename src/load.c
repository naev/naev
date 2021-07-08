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
#include "shiplog.h"
#include "space.h"
#include "toolkit.h"
#include "unidiff.h"

#define LOAD_WIDTH      600 /**< Load window width. */
#define LOAD_HEIGHT     500 /**< Load window height. */

#define BUTTON_WIDTH    200 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


/**
 * @brief Struct containing a file's name and stat structure.
 */
typedef struct filedata {
   char *name;
   PHYSFS_Stat stat;
} filedata_t;


static nsave_t *load_saves = NULL; /**< Array of save.s */
extern int save_loaded; /**< From save.c */


/*
 * Prototypes.
 */
/* externs */
/* player.c */
extern Planet* player_load( xmlNodePtr parent ); /**< Loads player related stuff. */
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
static void load_menu_update( unsigned int wid, char *str );
static void load_menu_close( unsigned int wdw, char *str );
static void load_menu_load( unsigned int wdw, char *str );
static void load_menu_delete( unsigned int wdw, char *str );
static int load_load( nsave_t *save, const char *path );
static int load_gameInternal( const char* file, const char* version );
static int load_enumerateCallback( void* data, const char* origdir, const char* fname );
static int load_sortCompare( const void *p1, const void *p2 );
static xmlDocPtr load_xml_parsePhysFS( const char* filename );


/**
 * @brief Loads an individual save.
 * @param[out] save Structure to populate.
 * @param path PhysicsFS path (i.e., relative path starting with "saves/").
 */
static int load_load( nsave_t *save, const char *path )
{
   xmlDocPtr doc;
   xmlNodePtr root, parent, node, cur;
   int cycles, periods, seconds;

   memset( save, 0, sizeof(nsave_t) );

   /* Load the XML. */
   doc = load_xml_parsePhysFS( path );
   if (doc == NULL) {
      WARN( _("Unable to parse save path '%s'."), path);
      return -1;
   }
   root = doc->xmlChildrenNode; /* base node */
   if (root == NULL) {
      WARN( _("Unable to get child node of save '%s'."), path);
      xmlFreeDoc(doc);
      return -1;
   }

   /* Save path. */
   save->path = strdup(path);

   /* Iterate inside the naev_save. */
   parent = root->xmlChildrenNode;
   do {
      xml_onlyNodes(parent);

      /* Info. */
      if (xml_isNode(parent, "version")) {
         node = parent->xmlChildrenNode;
         do {
            xmlr_strd(node, "naev", save->version);
            xmlr_strd(node, "data", save->data);
         } while (xml_nextNode(node));
         continue;
      }

      else if (xml_isNode(parent, "player")) {
         /* Get name. */
         xmlr_attr_strd(parent, "name", save->name);
         /* Parse rest. */
         node = parent->xmlChildrenNode;
         do {
            xml_onlyNodes(node);

            /* Player info. */
            xmlr_strd(node, "location", save->planet);
            xmlr_ulong(node, "credits", save->credits);

            /* Time. */
            if (xml_isNode(node, "time")) {
               cur = node->xmlChildrenNode;
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
   } while (xml_nextNode(parent));

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}


/**
 * @brief Loads or refreshes saved games.
 */
int load_refresh (void)
{
   char buf[PATH_MAX];
   filedata_t *files, tmp;
   size_t len;
   int i, ok;
   nsave_t *ns;

   if (load_saves != NULL)
      load_free();

   /* load the saves */
   files = array_create( filedata_t );
   PHYSFS_enumerate( "saves", load_enumerateCallback, &files );
   qsort(files, array_size(files), sizeof(filedata_t), load_sortCompare);

   if (array_size(files) == 0) {
      array_free( files );
      return 0;
   }

   /* Make sure backups are after saves. */
   for (i=0; i<array_size(files)-1; i++) {
      len = strlen( files[i].name );

      /* Only interested in swapping backup with file after it if it's not backup. */
      if ((len < 11) || strcmp( &files[i].name[len-10],".ns.backup" ))
         continue;

      /* Don't match. */
      if (strncmp( files[i].name, files[i+1].name, len-10 ))
         continue;

      /* Swap around. */
      tmp         = files[i];
      files[i]    = files[i+1];
      files[i+1]  = tmp;
   }

   /* Allocate and parse. */
   ok = 0;
   ns = NULL;
   load_saves = array_create_size( nsave_t, array_size(files) );
   for (i=0; i<array_size(files); i++) {
      if (!ok)
         ns = &array_grow( &load_saves );
      snprintf( buf, sizeof(buf), "saves/%s", files[i].name );
      ok = load_load( ns, buf );
   }

   /* If the save was invalid, array is 1 member too large. */
   if (ok)
      array_resize( &load_saves, array_size(load_saves)-1 );

   /* Clean up memory. */
   for (i=0; i<array_size(files); i++)
      free( files[i].name );
   array_free( files );

   return 0;
}


/**
 * @brief The PHYSFS_EnumerateCallback for load_refresh
 */
static int load_enumerateCallback( void* data, const char* origdir, const char* fname )
{
   char *path;
   const char *fmt;
   size_t dir_len, name_len;
   filedata_t *tmp;
   PHYSFS_Stat stat;

   dir_len = strlen( origdir );
   name_len = strlen( fname );

   /* no save or backup save extension? */
   if ((name_len < 4 || strcmp( &fname[name_len-3], ".ns" )) && (name_len < 11 || strcmp( &fname[name_len-10], ".ns.backup" )))
      return PHYSFS_ENUM_OK;

   fmt = dir_len && origdir[dir_len-1]=='/' ? "%s%s" : "%s/%s";
   asprintf( &path, fmt, origdir, fname );
   if (!PHYSFS_stat( path, &stat ))
      WARN( _("PhysicsFS: Cannot stat %s: %s"), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
   else if (stat.filetype == PHYSFS_FILETYPE_REGULAR) {
      tmp = &array_grow( (filedata_t**)data );
      tmp->name = strdup( fname );
      tmp->stat = stat;
   }

   free( path );
   return PHYSFS_ENUM_OK;
}


/**
 * @brief qsort compare function for files.
 */
static int load_sortCompare( const void *p1, const void *p2 )
{
   filedata_t *f1, *f2;

   f1 = (filedata_t*) p1;
   f2 = (filedata_t*) p2;

   if (f1->stat.modtime > f2->stat.modtime)
      return -1;
   else if (f1->stat.modtime < f2->stat.modtime)
      return +1;

   return strcmp( f1->name, f2->name );
}


/**
 * @brief Frees loaded save stuff.
 */
void load_free (void)
{
   int i;
   nsave_t *ns;

   for (i=0; i<array_size(load_saves); i++) {
      ns = &load_saves[i];
      free(ns->path);
      free(ns->name);
      free(ns->version);
      free(ns->data);
      free(ns->planet);
      free(ns->shipname);
      free(ns->shipmodel);
   }
   array_free( load_saves );
   load_saves = NULL;
}


/**
 * @brief Gets the array (array.h) of loaded saves.
 */
const nsave_t *load_getList (void)
{
   return load_saves;
}

/**
 * @brief Opens the load game menu.
 */
void load_loadGameMenu (void)
{
   unsigned int wid;
   char **names, buf[PATH_MAX];
   nsave_t *ns;
   int i, n, len;

   /* window */
   wid = window_create( "wdwLoadGameMenu", _("Load Game"), -1, -1, LOAD_WIDTH, LOAD_HEIGHT );
   window_setAccept( wid, load_menu_load );
   window_setCancel( wid, load_menu_close );

   /* Load loads. */
   load_refresh();

   /* load the saves */
   n = array_size( load_saves );
   if (n > 0) {
      names = malloc( sizeof(char*)*n );
      for (i=0; i<n; i++) {
         ns       = &load_saves[i];
         len      = strlen(ns->path);
         if (strcmp(&ns->path[len-10],".ns.backup")==0) {
            snprintf( buf, sizeof(buf), _("%s #r(Backup)#0"), ns->name );
            names[i] = strdup(buf);
         }
         else
            names[i] = strdup( ns->name );
      }
   }
   /* case there are no files */
   else {
      names = malloc(sizeof(char*));
      names[0] = strdup(_("None"));
      n     = 1;
   }

   /* Player text. */
   window_addText( wid, -20, -40, 200, LOAD_HEIGHT-40-20-2*(BUTTON_HEIGHT+20),
         0, "txtPilot", &gl_smallFont, NULL, NULL );

   window_addList( wid, 20, -50,
         LOAD_WIDTH-200-60, LOAD_HEIGHT-110,
         "lstSaves", names, n, 0, load_menu_update, load_menu_load );

   /* Buttons */
   window_addButtonKey( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", _("Back"), load_menu_close, SDLK_b );
   window_addButtonKey( wid, -20, 20 + BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", _("Load"), load_menu_load, SDLK_l );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDelete", _("Delete"), load_menu_delete );
}
/**
 * @brief Closes the load game menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_close( unsigned int wdw, char *str )
{
   (void)str;
   window_destroy( wdw );
}
/**
 * @brief Updates the load menu.
 *    @param wid Widget triggering function.
 *    @param str Unused.
 */
static void load_menu_update( unsigned int wid, char *str )
{
   (void) str;
   int pos;
   nsave_t *ns;
   char *save;
   char buf[STRMAX_SHORT], credits[ECON_CRED_STRLEN], date[64];

   /* Make sure list is ok. */
   save = toolkit_getList( wid, "lstSaves" );
   if (strcmp(save,_("None")) == 0)
      return;

   /* Get position. */
   pos = toolkit_getListPos( wid, "lstSaves" );
   ns  = &load_saves[pos];

   /* Display text. */
   credits2str( credits, ns->credits, 2 );
   ntime_prettyBuf( date, sizeof(date), ns->date, 2 );
   snprintf( buf, sizeof(buf),
         _("#nName:\n"
         "#0   %s\n\n"
         "#nVersion:\n"
         "#0   %s\n\n"
         "#nDate:\n"
         "#0   %s\n\n"
         "#nPlanet:\n"
         "#0   %s\n\n"
         "#nCredits:\n"
         "#0   %s\n\n"
         "#nShip Name:\n"
         "#0   %s\n\n"
         "#nShip Model:\n"
         "#0   %s"),
         ns->name, ns->version, date, ns->planet,
         credits, ns->shipname, ns->shipmodel );
   window_modifyText( wid, "txtPilot", buf );
}
/**
 * @brief Loads a new game.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_load( unsigned int wdw, char *str )
{
   (void)str;
   char *save;
   int wid, pos;
   int diff;

   wid = window_get( "wdwLoadGameMenu" );
   save = toolkit_getList( wid, "lstSaves" );

   if (strcmp(save,_("None")) == 0)
      return;

   pos = toolkit_getListPos( wid, "lstSaves" );

   /* Check version. */
   diff = naev_versionCompare( load_saves[pos].version );
   if (ABS(diff) >= 2) {
      if (!dialogue_YesNo( _("Save game version mismatch"),
            _("Save game '%s' version does not match Naev version:\n"
            "   Save version: #r%s#0\n"
            "   Naev version: %s\n"
            "Are you sure you want to load this game? It may lose data."),
            save, load_saves[pos].version, VERSION ))
         return;
   }

   /* Close menus before loading for proper rendering. */
   load_menu_close(wdw, NULL);

   /* Close the main menu. */
   menu_main_close();

   /* Try to load the game. */
   if (load_game( &load_saves[pos] )) {
      /* Failed so reopen both. */
      menu_main();
      load_loadGameMenu();
   }
}
/**
 * @brief Deletes an old game.
 *    @param wdw Window to delete.
 *    @param str Unused.
 */
static void load_menu_delete( unsigned int wdw, char *str )
{
   (void)str;
   char *save;
   int wid, pos;

   wid = window_get( "wdwLoadGameMenu" );
   save = toolkit_getList( wid, "lstSaves" );

   if (strcmp(save,"None") == 0)
      return;

   if (dialogue_YesNo( _("Permanently Delete?"),
      _("Are you sure you want to permanently delete '%s'?"), save) == 0)
      return;

   /* Remove it. */
   pos = toolkit_getListPos( wid, "lstSaves" );
   PHYSFS_delete( load_saves[pos].path );

   /* need to reload the menu */
   load_menu_close(wdw, NULL);
   load_loadGameMenu();
}


static void load_compatSlots (void)
{
   /* Vars for loading old saves. */
   int i,j;
   char **sships;
   glTexture **tships;
   int nships;
   Pilot *ship;
   ShipOutfitSlot *sslot;

   nships = player_nships();
   sships = malloc(nships * sizeof(char*));
   tships = malloc(nships * sizeof(glTexture*));
   nships = player_ships( sships, tships );
   ship   = player.p;
   for (i=-1; i<nships; i++) {
      if (i >= 0)
         ship = player_getShip( sships[i] );
      /* Remove all outfits. */
      for (j=0; j<array_size(ship->outfits); j++) {
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
   for (i=0; i<nships; i++)
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
int load_game( nsave_t *ns )
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
   xmlNodePtr node;
   xmlDocPtr doc;
   Planet *pnt;
   int version_diff = (version!=NULL) ? naev_versionCompare(version) : 0;

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

   /* Clean up possible stuff that should be cleaned. */
   player_cleanup();

   /* Welcome message - must be before space_init. */
   player_message( _("#gWelcome to %s!"), APPNAME );
   player_message( "#g v%s", naev_version(0) );

   /* Now begin to load. */
   diff_load(node); /* Must load first to work properly. */
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
   var_load(node);
   missions_loadActive(node);
   events_loadActive(node);
   news_loadArticles( node );
   hook_load(node);
   space_sysLoad(node);

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

   /* Land the player. */
   land( pnt, 1 );

   /* Load the GUI. */
   if (gui_load( gui_pick() )) {
      if (player.p->ship->gui != NULL)
         gui_load( player.p->ship->gui );
   }

   /* Sanitize the GUI. */
   gui_setCargo();
   gui_setShip();

   xmlFreeDoc(doc);

   /* Set loaded. */
   save_loaded = 1;

   return 0;

err_doc:
   xmlFreeDoc(doc);
err:
   WARN( _("Saved game '%s' invalid!"), file);
   return -1;
}


/**
 * @brief Temporary (hopefully) wrapper around xml_parsePhysFS in support of gzipped XML (like .ns files).
 */
static xmlDocPtr load_xml_parsePhysFS( const char* filename )
{
   char buf[PATH_MAX];

   snprintf( buf, sizeof(buf), "%s/%s", PHYSFS_getWriteDir(), filename);
   return xmlParseFile( buf );
}
