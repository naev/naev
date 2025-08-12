/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#ifdef __MINGW64_VERSION_MAJOR
/* HACK: libxml2 assumes in its function declarations that its format
 * strings are handled by the native (legacy Microsoft) printf-family
 * functions. Their source even #defines vsnprintf to _vsnprintf for maximum
 * breakage. However, testing a shows, e.g., xmlw_attr with PRIu64 formats
 * will still work on a MinGW64 build.
 * Therefore, we vandalize their (unfixable) diagnostics Dvaered-style.
 * */
#define LIBXML_ATTR_FORMAT( fmt, args )
#endif

#include "libxml/parser.h"    // IWYU pragma: export
#include "libxml/xmlwriter.h" // IWYU pragma: export
#include <stdlib.h>
/** @endcond */

#include "attributes.h"
#include "log.h"
#include "ntime.h"
#include "opengl_tex.h"

#define XML_NODE_START 1
#define XML_NODE_TEXT 3

/**
 * @brief Only handle nodes.
 */
#define xml_onlyNodes( n )                                                     \
   if ( ( ( n ) == NULL ) || ( ( n )->type != XML_NODE_START ) )               \
      continue;

/* checks to see if node n is of name s */
#define xml_isNode( n, s )                                                     \
   ( ( n != NULL ) && ( ( n )->type == XML_NODE_START ) &&                     \
     ( strcmp( (char *)( n )->name, s ) == 0 ) )

/* gets the next node */
#define xml_nextNode( n ) ( ( n != NULL ) && ( ( n = n->next ) != NULL ) )

/* get data different ways */
#define xml_raw( n ) ( (char *)( n )->children->content )
#define xml_get( n )                                                           \
   ( ( ( n )->children == NULL ) ? NULL : (char *)( n )->children->content )
#define xml_getInt( n )                                                        \
   ( ( xml_get( n ) == NULL ) ? 0 : strtol( xml_raw( n ), NULL, 10 ) )
#define xml_getUInt( n )                                                       \
   ( ( xml_get( n ) == NULL ) ? 0 : strtoul( xml_raw( n ), NULL, 10 ) )
#define xml_getLong( n )                                                       \
   ( ( xml_get( n ) == NULL ) ? 0 : strtoll( xml_raw( n ), NULL, 10 ) )
#define xml_getULong( n )                                                      \
   ( ( xml_get( n ) == NULL ) ? 0 : strtoull( xml_raw( n ), NULL, 10 ) )
#define xml_getFloat( n )                                                      \
   ( ( xml_get( n ) == NULL ) ? 0. : strtod( xml_raw( n ), NULL ) )
#define xml_getStrd( n )                                                       \
   ( ( xml_get( n ) == NULL ) ? NULL : strdup( xml_raw( n ) ) )

/*
 * reader crap
 */
#define xmlr_int( n, s, i )                                                    \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         i = xml_getInt( n );                                                  \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_uint( n, s, i )                                                   \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         i = xml_getUInt( n );                                                 \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_long( n, s, l )                                                   \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         l = xml_getLong( n );                                                 \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_ulong( n, s, l )                                                  \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         l = xml_getULong( n );                                                \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_float( n, s, f )                                                  \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         f = xml_getFloat( n );                                                \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_str( n, s, str )                                                  \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         str = xml_get( n );                                                   \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_strd( n, s, str )                                                 \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         if ( str != NULL ) {                                                  \
            WARN( "Node '%s' already loaded and being replaced from '%s' to "  \
                  "'%s'",                                                      \
                  s, str, xml_raw( n ) );                                      \
            free( str );                                                       \
         }                                                                     \
         str = ( ( xml_get( n ) == NULL ) ? NULL : strdup( xml_raw( n ) ) );   \
         continue;                                                             \
      }                                                                        \
   }
#define xmlr_strd_free( n, s, str )                                            \
   {                                                                           \
      if ( xml_isNode( n, s ) ) {                                              \
         if ( str != NULL )                                                    \
            free( str );                                                       \
         str = ( ( xml_get( n ) == NULL ) ? NULL : strdup( xml_raw( n ) ) );   \
         continue;                                                             \
      }                                                                        \
   }

/* Hack for better leak tracing: tools like LeakSanitizer can't trace past
 * xmlGetProp(), but there's no issue if we duplicate the string ourselves. */

#if DEBUGGING
static inline char *nxml_trace_strdup( void *ptr )
{
   void *pointer_from_libxml2 = ptr;
   char *ret                  = ( ptr == NULL ) ? NULL : strdup( ptr );
   free( pointer_from_libxml2 );
   return ret;
}
#else
#define nxml_trace_strdup( ptr ) ( (char *)( ptr ) )
#endif /* DEBUGGING */

/* Attribute reader (allocates memory). */
#define xmlr_attr_strd( n, s, a )                                              \
   a = nxml_trace_strdup( xmlGetProp( n, (xmlChar *)s ) )
#define xmlr_attr_strd_free( n, s, a )                                         \
   do {                                                                        \
      if ( a != NULL )                                                         \
         free( a );                                                            \
      a = nxml_trace_strdup( xmlGetProp( n, (xmlChar *)s ) );                  \
   } while ( 0 )

/* Attribute readers with defaults. */
#define xmlr_attr_int_def( n, s, a, def )                                      \
   do {                                                                        \
      xmlr_attr_strd( n, s, char *T );                                         \
      a = T == NULL ? def : strtol( T, NULL, 10 );                             \
      free( T );                                                               \
   } while ( 0 )
#define xmlr_attr_uint_def( n, s, a, def )                                     \
   do {                                                                        \
      xmlr_attr_strd( n, s, char *T );                                         \
      a = T == NULL ? def : strtoul( T, NULL, 10 );                            \
      free( T );                                                               \
   } while ( 0 )
#define xmlr_attr_long_def( n, s, a, def )                                     \
   do {                                                                        \
      xmlr_attr_strd( n, s, char *T );                                         \
      a = T == NULL ? def : strtoll( T, NULL, 10 );                            \
      free( T );                                                               \
   } while ( 0 )
#define xmlr_attr_ulong_def( n, s, a, def )                                    \
   do {                                                                        \
      xmlr_attr_strd( n, s, char *T );                                         \
      a = T == NULL ? def : strtoull( T, NULL, 10 );                           \
      free( T );                                                               \
   } while ( 0 )
#define xmlr_attr_float_def( n, s, a, def )                                    \
   do {                                                                        \
      xmlr_attr_strd( n, s, char *T );                                         \
      a = T == NULL ? def : strtod( T, NULL );                                 \
      free( T );                                                               \
   } while ( 0 )
/* Attribute readers defaulting to zero. */
#define xmlr_attr_int( n, s, a ) xmlr_attr_int_def( n, s, a, 0 )
#define xmlr_attr_uint( n, s, a ) xmlr_attr_uint_def( n, s, a, 0 )
#define xmlr_attr_long( n, s, a ) xmlr_attr_long_def( n, s, a, 0 )
#define xmlr_attr_ulong( n, s, a ) xmlr_attr_ulong_def( n, s, a, 0 )
#define xmlr_attr_float( n, s, a ) xmlr_attr_float_def( n, s, a, 0. )
/* Attribute readers for optional values. */
#define xmlr_attr_int_opt( n, s, a ) xmlr_attr_int_def( n, s, a, a )
#define xmlr_attr_uint_opt( n, s, a ) xmlr_attr_uint_def( n, s, a, a )
#define xmlr_attr_long_opt( n, s, a ) xmlr_attr_long_def( n, s, a, a )
#define xmlr_attr_ulong_opt( n, s, a ) xmlr_attr_ulong_def( n, s, a, a )
#define xmlr_attr_float_opt( n, s, a ) xmlr_attr_float_def( n, s, a, a )

/*
 * writer crap
 */
/* encompassing element */
#define xmlw_startElem( w, str )                                               \
   do {                                                                        \
      if ( xmlTextWriterStartElement( w, (xmlChar *)str ) < 0 ) {              \
         WARN( "xmlw: unable to create start element" );                       \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
#define xmlw_endElem( w )                                                      \
   do {                                                                        \
      if ( xmlTextWriterEndElement( w ) < 0 ) {                                \
         WARN( "xmlw: unable to create end element" );                         \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
/* other stuff */
#define xmlw_elemEmpty( w, n )                                                 \
   do {                                                                        \
      xmlw_startElem( w, n );                                                  \
      xmlw_endElem( w );                                                       \
   } while ( 0 )
#define xmlw_elem( w, n, str, ... )                                            \
   do {                                                                        \
      if ( xmlTextWriterWriteFormatElement( w, (xmlChar *)n, str,              \
                                            ##__VA_ARGS__ ) < 0 ) {            \
         WARN( "xmlw: unable to write format element" );                       \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
#define xmlw_raw( w, b, l )                                                    \
   do {                                                                        \
      if ( xmlTextWriterWriteRawLen( w, (xmlChar *)b, l ) < 0 ) {              \
         WARN( "xmlw: unable to write raw element" );                          \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
#define xmlw_attr_raw( w, str, val )                                           \
   do {                                                                        \
      if ( xmlTextWriterWriteAttribute( w, (xmlChar *)str, (xmlChar *)val ) <  \
           0 ) {                                                               \
         WARN( "xmlw: unable to write element attribute" );                    \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
#define xmlw_attr( w, str, ... )                                               \
   do {                                                                        \
      if ( xmlTextWriterWriteFormatAttribute( w, (xmlChar *)str,               \
                                              ##__VA_ARGS__ ) < 0 ) {          \
         WARN( "xmlw: unable to write element attribute" );                    \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
#define xmlw_str( w, str, ... )                                                \
   do {                                                                        \
      if ( xmlTextWriterWriteFormatString( w, str, ##__VA_ARGS__ ) < 0 ) {     \
         WARN( "xmlw: unable to write element data" );                         \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
/* document level */
#define xmlw_start( w )                                                        \
   do {                                                                        \
      if ( xmlTextWriterStartDocument( writer, NULL, "UTF-8", NULL ) < 0 ) {   \
         WARN( "xmlw: unable to start document" );                             \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )
#define xmlw_done( w )                                                         \
   do {                                                                        \
      if ( xmlTextWriterEndDocument( w ) < 0 ) {                               \
         WARN( "xmlw: unable to end document" );                               \
         return -1;                                                            \
      }                                                                        \
   } while ( 0 )

/*
 * Functions for generic complex reading.
 */
xmlDocPtr             xml_parsePhysFS( const char *filename );
USE_RESULT glTexture *xml_parseTexture( xmlNodePtr node, const char *path,
                                        int defsx, int defsy,
                                        const unsigned int flags );
int                   xml_parseTime( xmlNodePtr node, time_t *t );
int                   xml_parseNTime( xmlNodePtr node, ntime_t *t );

/*
 * Functions for generic complex writing.
 */
void xmlw_setParams( xmlTextWriterPtr writer );
int  xmlw_saveTime( xmlTextWriterPtr writer, const char *name, time_t t );
int  xmlw_saveNTime( xmlTextWriterPtr writer, const char *name, ntime_t t );

int nxml_init( void );
