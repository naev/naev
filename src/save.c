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
extern int missions_save( xmlTextWriterPtr writer );
extern int var_save( xmlTextWriterPtr writer ); /* misn var */
extern int player_load( xmlNodePtr parent );
extern void menu_main_close (void);
/* static */
static int save_data( xmlTextWriterPtr writer );
static void load_menu_close( char *str );
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

   wid = window_create( "Load Game", -1, -1, LOAD_WIDTH, LOAD_HEIGHT );
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", "Back", load_menu_close );
}
static void load_menu_close( char *str )
{
   (void)str;

   window_destroy( window_get("Load Game") );
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

   player_load(node);

   xmlFreeDoc(doc);
   
   return 0;
}


