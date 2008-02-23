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

   /* the data itself */
   player_save(writer);
   missions_save(writer);

   xmlw_done(writer);

   file = "test.xml";

   xmlFreeTextWriter(writer);
   xmlSaveFileEnc(file, doc, "UTF-8");
   xmlFreeDoc(doc);

   return 0;
}
