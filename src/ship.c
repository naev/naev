

#include "ship.h"

#include <string.h>
/* libxml2 needed later then 2.6 */
#include "libxml/xmlreader.h"

#include "log.h"


#define MAX_PATH_NAME	20	/* maximum size of the path */


#define XML_NODE_START	1
#define XML_NODE_TEXT	3
#define XML_NODE_CLOSE	15
#define XML_NODE_CDATA	4

#define XML_ID		"Ships"	/* XML section identifier */
#define XML_SHIP	"ship"

#define SHIP_DATA		"dat/ship.xml"
#define SHIP_GFX		"gfx/ship/"

static Ship* ship_stack = NULL;
static int ships = 0;



/*
 * Gets a ship based on its name
 */
Ship* get_ship( const char* name )
{
	Ship* temp = ship_stack;
	int i;

	for (i=0; i < ships; i++)
		if (strcmp((temp+i)->name, name)==0) break;

	if (i == ships) /* ship does not exist, game will probably crash now */
		WARN("Ship %s does not exist", name);

	return temp+i;
}


Ship* ship_parse( xmlNodePtr node )
{
	xmlNodePtr cur;
	Ship* temp = CALLOC_ONE(Ship);

	char str[MAX_PATH_NAME] = "\0";

	temp->name = (char*)xmlGetProp(node,(xmlChar*)"name");

	node = node->xmlChildrenNode;

	while ((node = node->next)) {
		if (strcmp((char*)node->name, "GFX")==0) {
			cur = node->children;
			if (strcmp((char*)cur->name,"text")==0) {
				snprintf( str, strlen((char*)cur->content)+sizeof(SHIP_GFX),
						SHIP_GFX"%s", (char*)cur->content);
				temp->gfx_ship = gl_newSprite(str, 6, 6);
			}
		}
		else if (strcmp((char*)node->name, "class")==0) {
			cur = node->children;
			if (strcmp((char*)cur->name,"text")==0)
				temp->class = atoi((char*)cur->content);
		}
		else if (strcmp((char*)node->name, "movement")==0) {
			cur = node->children;
			while ((cur = cur->next)) {
				if (strcmp((char*)cur->name,"thrust")==0)
					temp->thrust = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"turn")==0)
					temp->turn = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"speed")==0)
					temp->speed = atoi((char*)cur->children->content);
			}
		}
		else if (strcmp((char*)node->name,"health")==0) {
			cur = node->children;
			while ((cur = cur->next)) {
				if (strcmp((char*)cur->name,"armor")==0)
					temp->armor = (double)atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"shield")==0)
					temp->shield = (double)atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"energy")==0)
					temp->energy = (double)atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"armor_regen")==0)
					temp->armor_regen = (double)(atoi((char*)cur->children->content))/60.0;
				else if (strcmp((char*)cur->name,"shield_regen")==0)
					temp->shield_regen = (double)(atoi((char*)cur->children->content))/60.0;
				else if (strcmp((char*)cur->name,"energy_regen")==0)
					temp->energy_regen = (double)(atoi((char*)cur->children->content))/60.0;
			}
		}
		else if (strcmp((char*)node->name,"caracteristics")==0) {
			cur = node->children;
			while((cur = cur->next)) {
				if (strcmp((char*)cur->name,"crew")==0)
					temp->crew = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"mass")==0)
					temp->mass = (double)atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"cap_weapon")==0)
					temp->cap_weapon = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"cap_cargo")==0)
					temp->cap_cargo = atoi((char*)cur->children->content);
			}
		}
	}
	temp->thrust *= temp->mass; /* helps keep numbers sane */

	DEBUG("Loaded ship '%s'", temp->name);
	return temp;
}


int ships_load(void)
{
	xmlTextReaderPtr reader;
	xmlNodePtr node;
	Ship* temp = NULL;

	if ((reader=xmlNewTextReaderFilename(SHIP_DATA))==NULL) {
		WARN("XML error reading "SHIP_DATA);
		return -1;
	}

	/* get to the start of the "Ships" section */
	while (xmlTextReaderRead(reader)==1) {
		if (xmlTextReaderNodeType(reader)==XML_NODE_START &&
				strcmp((char*)xmlTextReaderConstName(reader),XML_ID)==0) break;
	}
	xmlTextReaderRead(reader); /* at Ships node */

	while (xmlTextReaderRead(reader)==1) {
		if (xmlTextReaderNodeType(reader)==XML_NODE_START &&
				strcmp((char*)xmlTextReaderConstName(reader),XML_SHIP)==0) {

			node = xmlTextReaderCurrentNode(reader); /* node to process */
			if (node == NULL) break; /* no node */
			if (ship_stack==NULL) {
				ship_stack = temp = ship_parse(node);
				ships = 1;
			}
			else {
				temp = ship_parse(node);
				ship_stack = realloc(ship_stack, sizeof(Ship)*(++ships));
				memcpy(ship_stack+ships-1, temp, sizeof(Ship));
				free(temp);
			}
		}
	}

	xmlFreeTextReader(reader);

	return 0;
}

void ships_free()
{
	int i;
	for (i = 0; i < ships; i++) {
		if ((ship_stack+i)->name)
			free((ship_stack+i)->name);
		gl_freeTexture((ship_stack+i)->gfx_ship);
	}
	free(ship_stack);
	ship_stack = NULL;
}
