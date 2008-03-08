/*
 * See Licensing and Copyright notice in naev.h
 */


#include "save.h"

#include "naev.h"
#include "log.h"
#include "xml.h"
#include "player.h"
#include "toolkit.h"
#include "menu.h"
#include "nfile.h"


#define LOAD_WIDTH      400
#define LOAD_HEIGHT     300

#define BUTTON_WIDTH    50
#define BUTTON_HEIGHT   30


/*
 * prototypes
 */
/* externs */
extern int player_save( xmlTextWriterPtr writer );
extern int player_load( xmlNodePtr parent );
extern int missions_save( xmlTextWriterPtr writer );
extern int var_save( xmlTextWriterPtr writer ); /* misn var */
extern int var_load( xmlNodePtr parent );
extern int pfaction_save( xmlTextWriterPtr writer );
extern int pfaction_load( xmlNodePtr parent );
extern void menu_main_close (void);
/* static */
static int save_data( xmlTextWriterPtr writer );
static void load_menu_close( char *str );
static void load_menu_load( char *str );
static int load_game( char* file );


/*
 * saves all the game data
 */
static int save_data( xmlTextWriterPtr writer )
{
   /* the data itself */
   if (player_save(writer) < 0) return -1;
   if (missions_save(writer) < 0) return -1;
   if (var_save(writer) < 0) return -1;
   if (pfaction_save(writer) < 0) return -1;

   return 0;
}


/*
 * saves the current game
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


/*
 * opens the load game menu
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

   /* default action */
   window_setFptr( wid, load_menu_load );
}
static void load_menu_close( char *str )
{
   (void)str;

   window_destroy( window_get("Load Game") );
}
static void load_menu_load( char *str )
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
   load_menu_close(NULL);
   menu_main_close();
}


/*
 * loads a new game
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

   player_load(node);
   var_load(node);
   pfaction_load(node);

   xmlFreeDoc(doc);
   
   return 0;
}


