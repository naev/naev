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
   int              ret    = 0;
   UniHunkTarget_t  target = {
       .type = HUNK_TARGET_NONE,
   };

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

   /* Attributes. */
   xmlw_attr( writer, "name", "test diff" );

   /* Write the bulk of the diff, we assume they are sorted by target. */
   for ( int i = 0; i < array_size( diffs ); i++ ) {
      const UniHunk_t *h   = &diffs[i];
      const char      *tag = diff_hunkTag( h->type );

      /* Assuming sorted, so we try to group all diffs with same target. */
      if ( ( target.type != h->target.type ) ||
           strcmp( target.u.name, h->target.u.name ) != 0 ) {
         if ( target.type != HUNK_TARGET_NONE )
            xmlw_endElem( writer ); /* current target */
         target = h->target;
         switch ( target.type ) {
         case HUNK_TARGET_SYSTEM:
            xmlw_startElem( writer, "system" );
            break;
         case HUNK_TARGET_SPOB:
            xmlw_startElem( writer, "spob" );
            break;
         case HUNK_TARGET_TECH:
            xmlw_startElem( writer, "tech" );
            break;
         case HUNK_TARGET_FACTION:
            xmlw_startElem( writer, "faction" );
            break;

         default:
            WARN( _( "Trying to save unknown target type '%d'!" ),
                  target.type );
            xmlw_startElem( writer, "unknown" );
            break;
         }
         xmlw_attr( writer, "name", "%s", h->target.u.name );
      }

      /* Write the diff contents. */
      switch ( h->dtype ) {
      case HUNK_DATA_NONE:
         xmlw_elemEmpty( writer, tag );
         break;
      case HUNK_DATA_STRING:
         xmlw_elem( writer, tag, "%s", h->u.name );
         break;
      case HUNK_DATA_INT:
         xmlw_elem( writer, tag, "%d", h->u.data );
         break;
      case HUNK_DATA_FLOAT:
         xmlw_elem( writer, tag, "%f", h->u.fdata );
         break;
      case HUNK_DATA_POINTER:
         WARN( _( "Hunk '%s' is trying to save pointer data!" ),
               diff_hunkName( h->type ) );
         break;
      }
      if ( target.type != HUNK_TARGET_NONE ) {
         for ( int j = 0; i < array_size( h->attr ); i++ ) {
            const UniAttribute_t *attr = &h->attr[j];
            xmlw_attr_raw( writer, attr->name, (const xmlChar *)attr->value );
         }
      }
   }
   if ( target.type != HUNK_TARGET_NONE )
      xmlw_endElem( writer ); /* current target */

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
