/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef XML_H
#  define XML_H


#include "libxml/parser.h"
#include "libxml/xmlwriter.h"

#include "log.h"


#define XML_NODE_START  1 
#define XML_NODE_TEXT   3

/* checks to see if node n is of name s */
#define xml_isNode(n,s)    \
   ((n!=NULL) && ((n)->type==XML_NODE_START) && \
   (strcmp((char*)(n)->name,s)==0))

/* gets the next node */
#define xml_nextNode(n)     \
   ((n!=NULL) && ((n = n->next) != NULL))

/* gets the property s of node n. WARNING: MALLOCS! */
#define xml_nodeProp(n,s)     (char*)xmlGetProp(n,(xmlChar*)s)

/* get data different ways */
#define xml_get(n)            ((char*)(n)->children->content)
#define xml_getInt(n)         (atoi((char*)(n)->children->content))
#define xml_getFloat(n)       (atof((char*)(n)->children->content))


/*
 * reader crap
 */
#define xmlr_int(n,s,i) \
if (xml_isNode(n,s)) { i = xml_getInt(n); continue; }
#define xmlr_float(n,s,f) \
if (xml_isNode(n,s)) { f = xml_getFloat(n); continue; }

/*
 * writer crap
 */
/* encompassing element */
#define xmlw_startElem(w,str)   \
if (xmlTextWriterStartElement(w,str) < 0) { \
   ERR("xmlw: unable to create start element"); return -1; }
#define xmlw_endElem(w) \
if (xmlTextWriterEndElement(w) < 0) { \
   ERR("xmlw: unable to create end element"); return -1; }
/* other stuff */
#define xmlw_elem(w,n,str, args...) \
if (xmlTextWriterWriteFormatElement(w,n,str, ## args) < 0) { \
   ERR("xmlw: unable to write format element"); return -1; }
#define xmlw_attr(w,str,val)  \
if (xmlTextWriterWriteAttribute(w,str,val) < 0) { \
   ERR("xmlw: unable to write element attribute"); return -1; }
/* document level */
#define xmlw_start(w) \
if (xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0) { \
   ERR("xmlw: unable to start document"); return -1; }
#define xmlw_done(w) \
if (xmlTextWriterEndDocument(w) < 0) { \
   ERR("xmlw: unable to end document"); return -1; }



#endif /* XML_H */
