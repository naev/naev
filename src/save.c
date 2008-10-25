/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file save.c
 *
 * @brief Handles saving/loading games.
 */

#include "save.h"

#include <stdio.h> /* remove() */

#include "naev.h"
#include "log.h"
#include "xml.h"
#include "player.h"
#include "toolkit.h"
#include "dialogue.h"
#include "menu.h"
#include "nfile.h"
#include "hook.h"


#define LOAD_WIDTH      400 /**< Load window width. */
#define LOAD_HEIGHT     300 /**< Load window height. */

#define BUTTON_WIDTH    50 /**< Button width. */
#define BUTTON_HEIGHT   30 /**< Button height. */


/*
 * prototypes
 */
/* externs */
extern int player_save( xmlTextWriterPtr writer ); /* alot of stuff :P */
extern int player_load( xmlNodePtr parent );
extern int missions_saveActive( xmlTextWriterPtr writer ); /* active missions */
extern int missions_loadActive( xmlNodePtr parent );
extern int var_save( xmlTextWriterPtr writer ); /* misn var */
extern int var_load( xmlNodePtr parent );
extern int pfaction_save( xmlTextWriterPtr writer ); /* faction data */
extern int pfaction_load( xmlNodePtr parent );
extern int hook_save( xmlTextWriterPtr writer ); /* hooks */
extern int hook_load( xmlNodePtr parent );
extern int space_sysSave( xmlTextWriterPtr writer ); /* space stuff */
extern int space_sysLoad( xmlNodePtr parent );
extern int diff_save( xmlTextWriterPtr writer );
extern int diff_load( xmlNodePtr parent );
extern void menu_main_close (void);
/* static */
static int save_data( xmlTextWriterPtr writer );
static void load_menu_close( unsigned int wdw, char *str );
static void load_menu_load( unsigned int wdw, char *str );
static void load_menu_delete( unsigned int wdw, char *str );
static int load_game( char* file );


/**
 * @fn static int save_data( xmlTextWriterPtr writer )
 *
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
 * @fn int save_all (void)
 *
 * @brief Saves the current game.
 *
 *    @return 0 on success.
 */
int save_all (void)
{
   char file[PATH_MAX];
   xmlDocPtr doc;
   xmlTextWriterPtr writer;

   writer = xmlNewTextWriterDoc(&doc, 0);
   if (writer == NULL) {
      ERR("testXmlwriterDoc: Error creating the xml writer");
      return -1;
   }

   xmlw_start(writer);
   xmlw_startElem(writer,"naev_save");

   /* save the version and such */
   xmlw_startElem(writer,"version");
   xmlw_elem( writer, "naev", "%d.%d.%d", VMAJOR, VMINOR, VREV );
   xmlw_elem( writer, "data", dataname );
   xmlw_endElem(writer); /* "version" */

   if (save_data(writer) < 0) {
      ERR("Trying to save game data");
      xmlFreeTextWriter(writer);
      xmlFreeDoc(doc);
      return -1;
   }

   xmlw_endElem(writer); /* "naev_save" */
   xmlw_done(writer);

   if (nfile_dirMakeExist("saves") < 0) {
      WARN("aborting save...");
      xmlFreeTextWriter(writer);
      xmlFreeDoc(doc);
      return -1;
   }
   snprintf(file, PATH_MAX,"%ssaves/%s.ns", nfile_basePath(), player_name);

   xmlFreeTextWriter(writer);
   xmlSaveFileEnc(file, doc, "UTF-8");
   xmlFreeDoc(doc);

   return 0;
}


/**
 * @fn void load_game_menu (void)
 *
 * @brief Opens the load game menu.
 */
void load_game_menu (void)
{
   unsigned int wid;
   char **files;
   int nfiles, i, len;

   /* window */
   wid = window_create( "Load Game", -1, -1, LOAD_WIDTH, LOAD_HEIGHT );

   /* load the saves */
   files = nfile_readDir( &nfiles, "saves" );
   for (i=0; i<nfiles; i++) {
      len = strlen(files[i]);

      /* no save extension */
      if ((len < 6) || strcmp(&files[i][len-3],".ns")) {
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
         LOAD_WIDTH-BUTTON_WIDTH-50, LOAD_HEIGHT-90,
         "lstSaves", files, nfiles, 0, NULL );

   /* buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", "Back", load_menu_close );
   window_addButton( wid, -20, 30 + BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", "Load", load_menu_load );
   window_addButton( wid, -20, 20 + 2*(10 + BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT,
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
   load_game( path );
   load_menu_close(wdw, NULL);
   menu_main_close();
}
/**
 * @brief Deletes an old game.
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
   load_game_menu();
}


/**
 * @fn static int load_game( char* file )
 * 
 * @brief Actually loads a new game based on file.
 *
 *    @param file File that contains the new game.
 *    @return 0 on success.
 */
static int load_game( char* file )
{
   xmlNodePtr node;
   xmlDocPtr doc;

   doc = xmlParseFile(file);
   node = doc->xmlChildrenNode; /* base node */
   if (node == NULL) {
      WARN("Savegame '%s' invalid!", file);
      return -1;
   }

   diff_load(node); /* Must load first to work properly. */
   player_load(node);
   var_load(node);
   missions_loadActive(node);
   pfaction_load(node);
   hook_load(node);
   space_sysLoad(node);

   /* Need to run takeoff hooks since player just "took off" */
   hooks_run("takeoff");
   hooks_run("enter");

   xmlFreeDoc(doc);
   xmlCleanupParser();
   
   return 0;
}


