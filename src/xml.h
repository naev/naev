

#ifndef XML_H
#  define XML_H


#include "libxml/parser.h"

#define XML_NODE_START  1 
#define XML_NODE_TEXT   3

/* checks to see if node n is of name s */
#define xml_isNode(n,s)		\
	(((n)->type==XML_NODE_START) && \
	(strcmp((char*)(n)->name,s)==0))

/* gets the property s of node n. WARNING: MALLOCS! */
#define xml_nodeProp(n,s)		(char*)xmlGetProp(n,(xmlChar*)s)

/* get data different ways */
#define xml_get(n)				((char*)(n)->children->content)
#define xml_getInt(n)			(atoi((char*)(n)->children->content))
#define xml_getFloat(n)			(atof((char*)(n)->children->content))


#endif /* XML_H */
