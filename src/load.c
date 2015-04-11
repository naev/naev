/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file load.c
 *
 * @brief Contains stuff to load a pilot or look up information about it.
 */


#include "load.h"

#include "naev.h"

#include "nxml.h"
#include "log.h"
#include "player.h"
#include "nfile.h"
#include "array.h"
#include "space.h"
#include "toolkit.h"
#include "menu.h"
#include "dialogue.h"
#include "event.h"
#include "news.h"
#include "mission.h"
#include "faction.h"
#include "gui.h"
#include "unidiff.h"
#include "nlua_var.h"
#include "land.h"
#include "hook.h"
#include "nstring.h"
#include "outfit.h"


#define LOAD_WIDTH      600 /**< Load window width. */
#define LOAD_HEIGHT     500 /**< Load window height. */

#define BUTTON_WIDTH    80 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


static nsave_t *load_saves = NULL; /**< Array of save.s */


extern int save_loaded; /**< From save.c */


/*
 * Prototypes.
 */
/* externs */
/* player.c */
extern Planet* player_load( xmlNodePtr parent ); /**< Loads player related stuff. */
/* mission.c */
extern int missions_loadActive( xmlNodePtr parent ); /**< Loads active missions. */
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
/* unidiff.c */
extern int diff_load( xmlNodePtr parent ); /**< Loads the universe diffs. */
/* static */
static void load_menu_update( unsigned int wid, char *str );
static void load_menu_close( unsigned int wdw, char *str );
static void load_menu_load( unsigned int wdw, char *str );
static void load_menu_delete( unsigned int wdw, char *str );
static int load_load( nsave_t *save, const char *path );


/**
 * @brief Loads an individual save.
 */
static int load_load( nsave_t *save, const char *path )
{
   xmlDocPtr doc;
   xmlNodePtr root, parent, node, cur;
   int scu, stp, stu;
   char *version = NULL;

   memset( save, 0, sizeof(nsave_t) );

   /* Load the XML. */
   doc   = xmlParseFile(path);
   if (doc == NULL) {
      WARN("Unable to parse save path '%s'.", path);
      return -1;
   }
   root = doc->xmlChildrenNode; /* base node */
   if (root == NULL) {
      WARN("Unable to get child node of save '%s'.",path);
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
      if (xml_isNode(parent,"version")) {
         node = parent->xmlChildrenNode;
         do {
            xmlr_strd(node,"naev",version);
            xmlr_strd(node,"data",save->data);
         } while (xml_nextNode(node));
         continue;
      }

      if (xml_isNode(parent,"player")) {
         /* Get name. */
         xmlr_attr(parent,"name",save->name);
         /* Parse rest. */
         node = parent->xmlChildrenNode;
         do {
            xml_onlyNodes(node);

            /* Player info. */
            xmlr_strd(node,"location",save->planet);
            xmlr_ulong(node,"credits",save->credits);

            /* Time. */
            if (xml_isNode(node,"time")) {
               cur = node->xmlChildrenNode;
               scu = stp = stu = 0;
               do {
                  xmlr_int(cur,"SCU",scu);
                  xmlr_int(cur,"STP",stp);
                  xmlr_int(cur,"STU",stu);
               } while (xml_nextNode(cur));
               save->date = ntime_create( scu, stp, stu );
               continue;
            }

            /* Ship info. */
            if (xml_isNode(node,"ship")) {
               xmlr_attr(node,"name",save->shipname);
               xmlr_attr(node,"model",save->shipmodel);
               continue;
            }
         } while (xml_nextNode(node));
         continue;
      }
   } while (xml_nextNode(parent));

   /* Handle version. */
   if (version != NULL) {
      naev_versionParse( save->version, version, strlen(version) );
      free(version);
   }

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}


/**
 * @brief Loads or refreshes saved games.
 */
int load_refresh (void)
{
   char **files, buf[PATH_MAX], *tmp;
   int nfiles, i, len;
   int ok;
   nsave_t *ns;

   if (load_saves != NULL)
      load_free();
   load_saves = array_create( nsave_t );

   /* load the saves */
   files = nfile_readDir( &nfiles, "%ssaves", nfile_dataPath() );
   for (i=0; i<nfiles; i++) {
      len = strlen(files[i]);

      /* no save or backup save extension */
      if (((len < 5) || strcmp(&files[i][len-3],".ns")) &&
            ((len < 12) || strcmp(&files[i][len-10],".ns.backup"))) {
         free(files[i]);
         memmove( &files[i], &files[i+1], sizeof(char*) * (nfiles-i-1) );
         nfiles--;
         i--;
      }
   }

   /* Make sure files are none. */
   if (files == NULL)
      return 0;

   /* Make sure backups are after saves. */
   for (i=0; i<nfiles-1; i++) {
      len = strlen( files[i] );

      /* Only interested in swapping backup with file after it if it's not backup. */
      if ((len < 12) || strcmp( &files[i][len-10],".ns.backup" ))
         continue;

      /* Don't match. */
      if (strncmp( files[i], files[i+1], (len-10) ))
         continue;
  
      /* Swap around. */
      tmp         = files[i];
      files[i]    = files[i+1];
      files[i+1]  = tmp;
   }

   /* Allocate and parse. */
   ok = 0;
   ns = NULL;
   for (i=0; i<nfiles; i++) {
      if (!ok)
         ns = &array_grow( &load_saves );
      nsnprintf( buf, sizeof(buf), "%ssaves/%s", nfile_dataPath(), files[i] );
      ok = load_load( ns, buf );
   }

   /* If the save was invalid, array is 1 member too large. */
   if (ok)
      array_resize( &load_saves, array_size(load_saves)-1 );

   /* Clean up memory. */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   return 0;
}


/**
 * @brief Frees loaded save stuff.
 */
void load_free (void)
{
   int i;
   nsave_t *ns;

   if (load_saves != NULL) {
      for (i=0; i<array_size(load_saves); i++) {
         ns = &load_saves[i];
         free(ns->path);
         if (ns->name != NULL)
            free(ns->name);

         if (ns->data != NULL)
            free(ns->data);

         if (ns->planet != NULL)
            free(ns->planet);

         if (ns->shipname != NULL)
            free(ns->shipname);
         if (ns->shipmodel != NULL)
            free(ns->shipmodel);
      }
      array_free( load_saves );
   }
   load_saves = NULL;
}


/**
 * @brief Gets the list of loaded saves.
 */
nsave_t *load_getList( int *n )
{
   if (load_saves == NULL) {
      *n = 0;
      return NULL;
   }

   *n = array_size( load_saves );
   return load_saves;
}

/**
 * @brief Opens the load game menu.
 */
void load_loadGameMenu (void)
{
   unsigned int wid;
   char **names, buf[PATH_MAX];
   nsave_t *nslist, *ns;
   int i, n, len;

   /* window */
   wid = window_create( "Load Game", -1, -1, LOAD_WIDTH, LOAD_HEIGHT );
   window_setAccept( wid, load_menu_load );
   window_setCancel( wid, load_menu_close );

   /* Load loads. */
   load_refresh();

   /* load the saves */
   nslist = load_getList( &n );
   if (n > 0) {
      names = malloc( sizeof(char*)*n );
      for (i=0; i<n; i++) {
         ns       = &nslist[i];
         len      = strlen(ns->path);
         if (strcmp(&ns->path[len-10],".ns.backup")==0) {
            nsnprintf( buf, sizeof(buf), "%s \er(Backup)\e0", ns->name );
            names[i] = strdup(buf);
         }
         else
            names[i] = strdup( ns->name );
      }
   }
   /* case there are no files */
   else {
      names = malloc(sizeof(char*));
      names[0] = strdup("None");
      n     = 1;
   }

   /* Player text. */
   window_addText( wid, -20, -40, 200, LOAD_HEIGHT-40-20-2*(BUTTON_HEIGHT+20),
         0, "txtPilot", NULL, &cBlack, NULL );

   window_addList( wid, 20, -50,
         LOAD_WIDTH-200-60, LOAD_HEIGHT-110,
         "lstSaves", names, n, 0, load_menu_update );

   /* Buttons */
   window_addButtonKey( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", "Back", load_menu_close, SDLK_b );
   window_addButtonKey( wid, -20, 20 + BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", "Load", load_menu_load, SDLK_l );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDelete", "Del", load_menu_delete );
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
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_update( unsigned int wid, char *str )
{
   (void) str;
   int pos;
   nsave_t *ns;
   int n;
   char *save;
   char buf[256], credits[ECON_CRED_STRLEN], date[64], version[256];

   /* Make sure list is ok. */
   save = toolkit_getList( wid, "lstSaves" );
   if (strcmp(save,"None") == 0)
      return;

   /* Get position. */
   pos = toolkit_getListPos( wid, "lstSaves" );
   ns  = load_getList( &n );
   ns  = &ns[pos];

   /* Display text. */
   credits2str( credits, ns->credits, 2 );
   ntime_prettyBuf( date, sizeof(date), ns->date, 2 );
   naev_versionString( version, sizeof(version), ns->version[0], ns->version[1], ns->version[2] );
   nsnprintf( buf, sizeof(buf),
         "\eDName:\n"
         "\e0   %s\n"
         "\eDVersion:\n"
         "\e0   %s\n"
         "\eDDate:\n"
         "\e0   %s\n"
         "\eDPlanet:\n"
         "\e0   %s\n"
         "\eDCredits:\n"
         "\e0   %s\n"
         "\eDShip Name:\n"
         "\e0   %s\n"
         "\eDShip Model:\n"
         "\e0   %s",
         ns->name, version, date, ns->planet,
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
   nsave_t *ns;
   int n;
   char version[64];
   int diff;

   wid = window_get( "Load Game" );
   save = toolkit_getList( wid, "lstSaves" );

   if (strcmp(save,"None") == 0)
      return;

   pos = toolkit_getListPos( wid, "lstSaves" );
   ns  = load_getList( &n );

   /* Check version. */
   diff = naev_versionCompare( ns[pos].version );
   if (ABS(diff) >= 2) {
      naev_versionString( version, sizeof(version), ns[pos].version[0],
            ns[pos].version[1], ns[pos].version[2] );
      if (!dialogue_YesNo( "Save game version mismatch",
            "Save game '%s' version does not match Naev version:\n"
            "   Save version: \er%s\e0\n"
            "   Naev version: \eD%s\e0\n"
            "Are you sure you want to load this game? It may lose data.",
            save, version, naev_version(0) ))
         return;
   }

   /* Close menus before loading for proper rendering. */
   load_menu_close(wdw, NULL);

   /* Close the main menu. */
   menu_main_close();

   /* Try to load the game. */
   if (load_game( ns[pos].path, diff )) {
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
   nsave_t *ns;
   int n;

   wid = window_get( "Load Game" );
   save = toolkit_getList( wid, "lstSaves" );

   if (strcmp(save,"None") == 0)
      return;

   if (dialogue_YesNo( "Permanently Delete?",
      "Are you sure you want to permanently delete '%s'?", save) == 0)
      return;

   /* Remove it. */
   pos = toolkit_getListPos( wid, "lstSaves" );
   ns  = load_getList( &n );
   remove( ns[pos].path ); /* remove is portable and will call unlink on linux. */

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
      for (j=0; j<ship->noutfits; j++) {
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
 * @brief Actually loads a new game based on file.
 *
 *    @param file File that contains the new game.
 *    @return 0 on success.
 */
int load_game( const char* file, int version_diff )
{
   xmlNodePtr node;
   xmlDocPtr doc;
   Planet *pnt;

   /* Make sure it exists. */
   if (!nfile_fileExists(file)) {
      dialogue_alert("Savegame file seems to have been deleted.");
      return -1;
   }

   /* Load the XML. */
   doc   = xmlParseFile(file);
   if (doc == NULL)
      goto err;
   node  = doc->xmlChildrenNode; /* base node */
   if (node == NULL)
      goto err_doc;

   /* Clean up possible stuff that should be cleaned. */
   player_cleanup();

   /* Welcome message - must be before space_init. */
   player_message( "\egWelcome to "APPNAME"!" );
   player_message( "\eg v%s", naev_version(0) );

   /* Now begin to load. */
   diff_load(node); /* Must load first to work properly. */
   pfaction_load(node); /* Must be loaded before player so the messages show up properly. */
   pnt = player_load(node);

   /* Sanitize for new version. */
   if (version_diff <= -2) {
      WARN("Old version detected. Sanitizing ships for slots");
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

   /* Check sanity. */
   event_checkSanity();

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
   WARN("Savegame '%s' invalid!", file);
   return -1;
}


