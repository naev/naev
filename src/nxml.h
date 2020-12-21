/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef XML_H
#  define XML_H

/** @cond */
#include <errno.h>

#ifdef __MINGW64_VERSION_MAJOR
   /* HACK: libxml2 assumes in its function declarations that its format
    * strings are handled by the native (legacy Microsoft) printf-family
    * functions. Their source even #defines vsnprintf to _vsnprintf for maximum
    * breakage. However, testing a shows, e.g., xmlw_attr with PRIu64 formats
    * will still work on a MinGW64 build.
    * Therefore, we vandalize their (unfixable) diagnostics Dvaered-style.
    * */
#  define LIBXML_ATTR_FORMAT( fmt, args )
#endif

#include "libxml/parser.h"
#include "libxml/xmlwriter.h"
/** @endcond */

#include "log.h"
#include "opengl.h"


#define XML_NODE_START  1
#define XML_NODE_TEXT   3

/**
 * @brief Only handle nodes.
 */
#define xml_onlyNodes(n)    \
   if (((n)==NULL) || ((n)->type!=XML_NODE_START)) \
      continue;

/* checks to see if node n is of name s */
#define xml_isNode(n,s)    \
   ((n!=NULL) && ((n)->type==XML_NODE_START) && \
   (strcmp((char*)(n)->name,s)==0))

/* gets the next node */
#define xml_nextNode(n)     \
   ((n!=NULL) && ((n = n->next) != NULL))

/* get data different ways */
#define xml_raw(n)            ((char*)(n)->children->content)
#define xml_get(n)            (((n)->children == NULL) ? NULL : (char*)(n)->children->content)
#define xml_getInt(n)         ((xml_get(n) == NULL) ? 0  : strtol(  xml_raw(n), (char**)NULL, 10))
#define xml_getUInt(n)        ((xml_get(n) == NULL) ? 0  : strtoul( xml_raw(n), (char**)NULL, 10))
#define xml_getLong(n)        ((xml_get(n) == NULL) ? 0  : strtoll( xml_raw(n), (char**)NULL, 10))
#define xml_getULong(n)       ((xml_get(n) == NULL) ? 0  : strtoull(xml_raw(n), (char**)NULL, 10))
#define xml_getFloat(n)       ((xml_get(n) == NULL) ? 0. : atof(xml_raw(n)))
#define xml_getStrd(n)        ((xml_get(n) == NULL) ? NULL : strdup(xml_raw(n)))


/*
 * reader crap
 */
#define xmlr_int(n,s,i) \
   {if (xml_isNode(n,s)) { \
      i = xml_getInt(n); continue; }}
#define xmlr_uint(n,s,i) \
   {if (xml_isNode(n,s)) { \
      i = xml_getUInt(n); continue; }}
#define xmlr_long(n,s,l) \
   {if (xml_isNode(n,s)) { \
      l = xml_getLong(n); continue; }}
#define xmlr_ulong(n,s,l) \
   {if (xml_isNode(n,s)) { \
      l = xml_getULong(n); continue; }}
#define xmlr_float(n,s,f) \
   {if (xml_isNode(n,s)) { \
      f = xml_getFloat(n); continue; }}
#define xmlr_floatR(n,s,f) \
   {if (xml_isNode(n,s)) { \
      f = xml_getFloat(n); return 0; }}
#define xmlr_str(n,s,str) \
   {if (xml_isNode(n,s)) { \
      str = xml_get(n); continue; }}
#define xmlr_strd(n,s,str) \
   {if (xml_isNode(n,s)) { \
      if (str != NULL) { \
         WARN("Node '%s' already loaded and being replaced from '%s' to '%s'", \
               s, str, xml_raw(n) ); } \
      str = ((xml_get(n) == NULL) ? NULL : strdup(xml_raw(n))); continue; }}
/** DEPRECATED synonym for xmlr_attr_strd. (This one just isn't explicit about being a malloc.)
 * It's still used by old code that needs to be reviewed or deleted for other reasons. */
#define xmlr_attr(n,s,a) \
   a = (char*)xmlGetProp(n,(xmlChar*)s)
#define xmlr_attr_int(n,s,a)     do {xmlr_attr(n,s,char*T); a = T==NULL ? 0 : strtol(  T, NULL, 10); free(T);} while(0)
#define xmlr_attr_int(n,s,a)     do {xmlr_attr(n,s,char*T); a = T==NULL ? 0 : strtol(  T, NULL, 10); free(T);} while(0)
#define xmlr_attr_uint(n,s,a)    do {xmlr_attr(n,s,char*T); a = T==NULL ? 0 : strtoul( T, NULL, 10); free(T);} while(0)
#define xmlr_attr_long(n,s,a)    do {xmlr_attr(n,s,char*T); a = T==NULL ? 0 : strtoll( T, NULL, 10); free(T);} while(0)
#define xmlr_attr_ulong(n,s,a)   do {xmlr_attr(n,s,char*T); a = T==NULL ? 0 : strtoull(T, NULL, 10); free(T);} while(0)
#define xmlr_attr_float(n,s,a)   do {xmlr_attr(n,s,char*T); a = T==NULL ? 0 :     atof(T          ); free(T);} while(0)
#define xmlr_attr_strd(n,s,a)    xmlr_attr(n,s,a)
/** DEPRECATED common pattern for optional ints (actually of type int) */
#define xmlr_attr_atoi_neg1(n,s,a) do {xmlr_attr(n,s,char*T); a = T==NULL ? -1 : atoi(T); free(T);} while(0)

/*
 * writer crap
 */
/* encompassing element */
#define xmlw_startElem(w,str)   \
do {if (xmlTextWriterStartElement(w,(xmlChar*)str) < 0) { \
   ERR("xmlw: unable to create start element"); return -1; } } while (0)
#define xmlw_endElem(w) \
do {if (xmlTextWriterEndElement(w) < 0) { \
   ERR("xmlw: unable to create end element"); return -1; } } while (0)
/* other stuff */
#define xmlw_elemEmpty(w,n)   \
do { xmlw_startElem(w,n); xmlw_endElem(w); } while (0)
#define xmlw_elem(w,n,str,args...) \
do { if (xmlTextWriterWriteFormatElement(w,(xmlChar*)n, \
      str, ## args) < 0) { \
   ERR("xmlw: unable to write format element"); return -1; } } while (0)
#define xmlw_raw(w,b,l) \
do {if (xmlTextWriterWriteRawLen(w,(xmlChar*)b,l) < 0) { \
   ERR("xmlw: unable to write raw element"); return -1; } } while (0)
#define xmlw_attr(w,str,val...)  \
do {if (xmlTextWriterWriteFormatAttribute(w,(xmlChar*)str, \
      ## val) < 0) { \
   ERR("xmlw: unable to write element attribute"); return -1; } } while (0)
#define xmlw_str(w,str,val...) \
do {if (xmlTextWriterWriteFormatString(w,str, ## val) < 0) { \
   ERR("xmlw: unable to write element data"); return -1; } } while (0)
/* document level */
#define xmlw_start(w) \
do {if (xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0) { \
   ERR("xmlw: unable to start document"); return -1; } } while (0)
#define xmlw_done(w) \
do {if (xmlTextWriterEndDocument(w) < 0) { \
   ERR("xmlw: unable to end document"); return -1; } } while (0)


/*
 * Functions for generic complex reading.
 */
xmlDocPtr xml_parsePhysFS( const char* filename );
glTexture* xml_parseTexture( xmlNodePtr node,
      const char *path, int defsx, int defsy,
      const unsigned int flags );


/*
 * Functions for generic complex writing.
 */
void xmlw_setParams( xmlTextWriterPtr writer );


#endif /* XML_H */
