/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file save.c
 *
 * @brief Handles saving/loading games.
 */

#include "save.h"

#include "naev.h"

#include <stdio.h> /* remove() */
#include <errno.h> /* errno */

#include "log.h"
#include "nxml.h"
#include "player.h"
#include "toolkit.h"
#include "dialogue.h"
#include "menu.h"
#include "nfile.h"
#include "hook.h"
#include "ndata.h"
#include "unidiff.h"
#include "nlua_var.h"
#include "event.h"
#include "conf.h"


#define LOAD_WIDTH      400 /**< Load window width. */
#define LOAD_HEIGHT     300 /**< Load window height. */

#define BUTTON_WIDTH    50 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


/*
 * prototypes
 */
/* externs */
/* player.c */
extern int player_save( xmlTextWriterPtr writer ); /**< Saves player related stuff. */
extern int player_load( xmlNodePtr parent ); /**< Loads player related stuff. */
/* mission.c */
extern int missions_saveActive( xmlTextWriterPtr writer ); /**< Saves active missions. */
extern int missions_loadActive( xmlNodePtr parent ); /**< Loads active missions. */
/* nlua_misn.c */
extern int var_save( xmlTextWriterPtr writer ); /**< Saves mission variables. */
extern int var_load( xmlNodePtr parent ); /**< Loads mission variables. */
/* faction.c */
extern int pfaction_save( xmlTextWriterPtr writer ); /**< Saves faction data. */
extern int pfaction_load( xmlNodePtr parent ); /**< Loads faction data. */
/* hook.c */
extern int hook_save( xmlTextWriterPtr writer ); /**< Saves hooks. */
extern int hook_load( xmlNodePtr parent ); /**< Loads hooks. */
/* space.c */
extern int space_sysSave( xmlTextWriterPtr writer ); /**< Saves the space stuff. */
extern int space_sysLoad( xmlNodePtr parent ); /**< Loads the space stuff. */
/* unidiff.c */
extern int diff_save( xmlTextWriterPtr writer ); /**< Saves the universe diffs. */
extern int diff_load( xmlNodePtr parent ); /**< Loads the universe diffs. */
/* menu.c */
extern void menu_main_close (void); /**< Closes the main menu. */
/* static */
static int save_data( xmlTextWriterPtr writer );
static void load_menu_close( unsigned int wdw, char *str );
static void load_menu_load( unsigned int wdw, char *str );
static void load_menu_delete( unsigned int wdw, char *str );
static int load_game( const char* file );


/**
 * @brief Saves all the player's game data.
 *
 *    @param writer XML writer to use.
 *    @return 0 on success.
 */
static int save_data( xmlTextWriterPtr writer )
{
   /* the data itself */
   if (diff_save(writer) < 0) return -1; /* Must save first or can get cleared. */
   if (player_save(writer) < 0) return -1;
   if (missions_saveActive(writer) < 0) return -1;
   if (var_save(writer) < 0) return -1;
   if (pfaction_save(writer) < 0) return -1;
   if (hook_save(writer) < 0) return -1;
   if (space_sysSave(writer) < 0) return -1;

   return 0;
}


/**
 * @brief Saves the current game.
 *
 *    @return 0 on success.
 */
int save_all (void)
{
   char file[PATH_MAX];
   xmlDocPtr doc;
   xmlTextWriterPtr writer;

   /* Create the writer. */
   writer = xmlNewTextWriterDoc(&doc, conf.save_compress);
   if (writer == NULL) {
      ERR("testXmlwriterDoc: Error creating the xml writer");
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start element. */
   xmlw_start(writer);
   xmlw_startElem(writer,"naev_save");

   /* Save the version and such. */
   xmlw_startElem(writer,"version");
   xmlw_elem( writer, "naev", "%d.%d.%d", VMAJOR, VMINOR, VREV );
   xmlw_elem( writer, "data", "%s", ndata_name() );
   xmlw_endElem(writer); /* "version" */

   /* Save the data. */
   if (save_data(writer) < 0) {
      ERR("Trying to save game data");
      goto err_writer;
   }

   /* Finish element. */
   xmlw_endElem(writer); /* "naev_save" */
   xmlw_done(writer);

   /* Write to file. */
   if (nfile_dirMakeExist("%ssaves", nfile_basePath()) < 0) {
      WARN("Aborting save...");
      goto err_writer;
   }
   snprintf(file, PATH_MAX, "%ssaves/%s.ns", nfile_basePath(), player.name);

   /* Back up old savegame. */
   if (nfile_backupIfExists(file) < 0) {
      WARN("Aborting save...");
      goto err_writer;
   }

   /* Critical section, if crashes here player's game gets corrupted.
    * Luckily we have a copy just in case... */
   xmlFreeTextWriter(writer);
   if (xmlSaveFileEnc(file, doc, "UTF-8") < 0) {
      WARN("Failed to write savegame!  You'll most likely have to restore it by copying your backup savegame over your current savegame.");
      goto err;
   }
   xmlFreeDoc(doc);

   return 0;

err_writer:
   xmlFreeTextWriter(writer);
err:
   xmlFreeDoc(doc);
   return -1;
}

/**
 * @brief Reload the current savegame.
 */
void save_reload (void)
{
   char path[PATH_MAX];
   snprintf(path, PATH_MAX, "%ssaves/%s.ns", nfile_basePath(), player.name);
   load_game( path );
}


/**
 * @brief Checks to see if there's a savegame available.
 *
 *    @return 1 if a savegame is available, 0 otherwise.
 */
int save_hasSave (void)
{
   char **files;
   int nfiles, i, len;
   int has_save;

   /* Look for saved games. */
   files = nfile_readDir( &nfiles, "%ssaves", nfile_basePath() );
   has_save = 0;
   for (i=0; i<nfiles; i++) {
      len = strlen(files[i]);

      /* no save extension */
      if ((len >= 5) && (strcmp(&files[i][len-3],".ns")==0)) {
         has_save = 1;
         break;
      }
   }

   /* Clean up. */
   for (i=0; i<nfiles; i++)
      free(files[i]);
   free(files);

   return has_save;
}

/**
 * @brief Opens the load game menu.
 */
void save_loadGameMenu (void)
{
   unsigned int wid;
   char **files;
   int nfiles, i, len;

   /* window */
   wid = window_create( "Load Game", -1, -1, LOAD_WIDTH, LOAD_HEIGHT );
   window_setCancel( wid, load_menu_close );

   /* load the saves */
   files = nfile_readDir( &nfiles, "%ssaves", nfile_basePath() );
   for (i=0; i<nfiles; i++) {
      len = strlen(files[i]);

      /* no save extension */
      if ((len < 5) || strcmp(&files[i][len-3],".ns")) {
         free(files[i]);
         memmove( &files[i], &files[i+1], sizeof(char*) * (nfiles-i-1) );
         nfiles--;
         i--;
      }
      else /* remove the extension */
         files[i][len-3] = '\0';
   }
   /* case there are no files */
   if (files == NULL) {
      files = malloc(sizeof(char*));
      files[0] = strdup("None");
      nfiles = 1;
   }
   window_addList( wid, 20, -50,
         LOAD_WIDTH-BUTTON_WIDTH-50, LOAD_HEIGHT-110,
         "lstSaves", files, nfiles, 0, NULL );

   /* buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", "Back", load_menu_close );
   window_addButton( wid, -20, 30 + BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", "Load", load_menu_load );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDelete", "Del", load_menu_delete );

   /* default action */
   window_setAccept( wid, load_menu_load );
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
 * @brief Loads a new game.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void load_menu_load( unsigned int wdw, char *str )
{
   (void)str;
   char *save, path[PATH_MAX];
   int wid;

   wid = window_get( "Load Game" );
   save = toolkit_getList( wid, "lstSaves" );

   if (strcmp(save,"None") == 0)
      return;

   snprintf( path, PATH_MAX, "%ssaves/%s.ns", nfile_basePath(), save );

   /* Close menus before loading for proper rendering. */
   load_menu_close(wdw, NULL);
   menu_main_close();

   if (load_game( path )) {
      menu_main();
      save_loadGameMenu();
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
   char *save, path[PATH_MAX];
   int wid;

   wid = window_get( "Load Game" );
   save = toolkit_getList( wid, "lstSaves" );

   if (strcmp(save,"None") == 0)
      return;

   if (dialogue_YesNo( "Permanently Delete?",
      "Are you sure you want to permanently delete '%s'?", save) == 0)
      return;

   snprintf( path, PATH_MAX, "%ssaves/%s.ns", nfile_basePath(), save );
   remove(path); /* remove is portable and will call unlink on linux. */

   /* need to reload the menu */
   load_menu_close(wdw, NULL);
   save_loadGameMenu();
}


/**
 * @brief Actually loads a new game based on file.
 *
 *    @param file File that contains the new game.
 *    @return 0 on success.
 */
static int load_game( const char* file )
{
   xmlNodePtr node;
   xmlDocPtr doc;

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
   diff_clear();
   var_cleanup();
   missions_cleanup();
   events_cleanup();

   /* Welcome message - must be before space_init. */
   player_message( "\egWelcome to "APPNAME"!" );
   player_message( "\eg v%d.%d.%d", VMAJOR, VMINOR, VREV );

   /* Now begin to load. */
   diff_load(node); /* Must load first to work properly. */
   pfaction_load(node); /* Must be loaded before player so the messages show up properly. */
   player_load(node);
   var_load(node);
   missions_loadActive(node);
   hook_load(node);
   space_sysLoad(node);

   /* Initialize the economy. */
   economy_init();

   /* Need to run takeoff hooks since player just "took off" */
   hooks_run("takeoff");
   player_addEscorts();
   hooks_run("enter");
   events_trigger( EVENT_TRIGGER_ENTER );

   xmlFreeDoc(doc);
   xmlCleanupParser();
   
   return 0;

err_doc:
   xmlFreeDoc(doc);
   xmlCleanupParser();
err:
   WARN("Savegame '%s' invalid!", file);
   return -1;
}


