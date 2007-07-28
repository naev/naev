

#ifndef XML_H
#  define XML_H


#include "libxml/parser.h"

#define XML_NODE_START  1 
#define XML_NODE_TEXT   3

/* checks to see if node n is of name s */
#define xml_isNode(n,s)		\
	((n)->type==XML_NODE_START) && \
	(strcmp((char*)(n)->name,s)==0)

#define xml_nodeProp(n,s)	\
	(char*)xmlGetProp(n,(xmlChar*)s)


#endif /* XML_H */
