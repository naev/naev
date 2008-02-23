/*
 * See Licensing and Copyright notice in naev.h
 */


#include "save.h"

#include "naev.h"
#include "log.h"
#include "xml.h"


/*
 * prototypes
 */
/* externs */
extern int player_save( xmlTextWriterPtr writer );
extern int missions_save( xmlTextWriterPtr writer );
/* static */
static int save_data( xmlTextWriterPtr writer );


static int save_data( xmlTextWriterPtr writer )
{
   /* the data itself */
   if (player_save(writer) < 0) return -1;
   if (missions_save(writer) < 0) return -1;

   return 0;
}



int save_all (void)
{
   char *file;
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

   file = "test.xml";

   xmlFreeTextWriter(writer);
   //xmlSaveFileEnc(file, doc, "UTF-8");
   xmlFreeDoc(doc);

   return 0;
}


