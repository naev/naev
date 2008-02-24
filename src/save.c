/*
 * See Licensing and Copyright notice in naev.h
 */


#include "save.h"

#include <stdlib.h>
#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif /* LINUX */

#include "naev.h"
#include "log.h"
#include "xml.h"
#include "player.h"


/*
 * prototypes
 */
/* externs */
extern int player_save( xmlTextWriterPtr writer );
extern int missions_save( xmlTextWriterPtr writer );
extern int var_save( xmlTextWriterPtr writer ); /* misn var */
extern int player_load( xmlNodePtr parent );
/* static */
static int save_data( xmlTextWriterPtr writer );


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
   char file[PATH_MAX], *home;
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

#ifdef LINUX
   struct stat buf;
   home = getenv("HOME");
   snprintf(file, PATH_MAX,"%s/.naev/saves",home);
   stat(file,&buf);
   if (!S_ISDIR(buf.st_mode))
      if (mkdir(file,S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
         WARN("Unable to create '%s' for saving: %s",file,strerror(errno));
         WARN("aborting save...");
         xmlFreeTextWriter(writer);
         xmlFreeDoc(doc);
         return -1;
      }
   snprintf(file, PATH_MAX,"%s/.naev/saves/%s.ns",home,player_name);
#endif /* LINUX */

   xmlFreeTextWriter(writer);
   xmlSaveFileEnc(file, doc, "UTF-8");
   xmlFreeDoc(doc);

   return 0;
}


/*
 * loads a new game
 */
int load_game( char* file )
{
   xmlNodePtr node;
   xmlDocPtr doc;

   doc = xmlParseFile(file);
   node = doc->xmlChildrenNode; /* base node */

   player_load(node);

   xmlFreeDoc(doc);
   
   return 0;
}


