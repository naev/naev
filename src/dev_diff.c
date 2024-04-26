/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file dev_diff.c
 *
 * @brief Handles saving unidiffs.
 */

#include "dev_diff.h"

#include "array.h"
#include "nxml.h"

int ddiff_save( UniHunk_t *diffs, const char *filename )
{
   xmlDocPtr        doc;
   xmlTextWriterPtr writer;
   int              ret = 0;

   /* Create the writer. */
   writer = xmlNewTextWriterDoc( &doc, 0 );
   if ( writer == NULL ) {
      WARN( _( "testXmlwriterDoc: Error creating the xml writer" ) );
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start writer. */
   xmlw_start( writer );
   xmlw_startElem( writer, "unidiff" );

   /* Write the bulk of the diff. */
   for ( int i = 0; i < array_size( diffs ); i++ ) {
   }

   xmlw_endElem( writer ); /* "unidiff" */
   xmlw_done( writer );

   /* No need for writer anymore. */
   xmlFreeTextWriter( writer );

   /* Write data. */
   if ( xmlSaveFileEnc( filename, doc, "UTF-8" ) < 0 ) {
      WARN( "Failed to write '%s'!", filename );
      ret = -1;
   }

   /* Clean up. */
   xmlFreeDoc( doc );

   return ret;
}
