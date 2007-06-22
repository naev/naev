

#include "ship.h"

#include <string.h>

#include "libxml/parser.h"

#include "main.h"
#include "log.h"
#include "pack.h"


#define XML_NODE_START	1
#define XML_NODE_TEXT	3

#define XML_ID		"Ships"	/* XML section identifier */
#define XML_SHIP	"ship"

#define SHIP_DATA		"dat/ship.xml"
#define SHIP_GFX		"gfx/ship/"
#define SHIP_EXT		".png"
#define SHIP_TARGET	"_target"

static Ship* ship_stack = NULL;
static int ships = 0;


/*
 * Prototypes
 */
static Ship* ship_parse( xmlNodePtr parent );


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


static Ship* ship_parse( xmlNodePtr parent )
{
	xmlNodePtr cur, node;
	Ship* temp = CALLOC_ONE(Ship);
	ShipOutfit *otemp, *ocur;

	char str[PATH_MAX] = "\0";
	xmlChar* xstr;

	temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name");

	node = parent->xmlChildrenNode;

	while ((node = node->next)) { /* load all the data */
		if (strcmp((char*)node->name, "GFX")==0) {
			snprintf( str, strlen((char*)node->children->content)+
					sizeof(SHIP_GFX)+sizeof(SHIP_EXT),
					SHIP_GFX"%s"SHIP_EXT, (char*)node->children->content);
			temp->gfx_space = gl_newSprite(str, 6, 6);
			snprintf( str, strlen((char*)node->children->content)+
					sizeof(SHIP_GFX)+sizeof(SHIP_TARGET)+sizeof(SHIP_EXT),
					SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, (char*)node->children->content);
			temp->gfx_target = gl_newImage(str);
		}
		else if (strcmp((char*)node->name, "class")==0)
			temp->class = atoi((char*)node->children->content);
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
			while ((cur = cur->next)) {
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
		else if (strcmp((char*)node->name,"outfits")==0) {
			cur = node->children;
			while ((cur = cur->next)) {
				if (strcmp((char*)cur->name,"outfit")==0) {
					otemp = MALLOC_ONE(ShipOutfit);
					otemp->data = outfit_get((char*)cur->children->content);
					xstr = xmlGetProp(cur,(xmlChar*)"quantity");
					if (!xstr)
						WARN("Ship '%s' is missing tag 'quantity' for outfit '%s'",
								temp->name, otemp->data->name);
					otemp->quantity = atoi((char*)xstr);
					free(xstr);
					otemp->next = NULL;
					
					if ((ocur=temp->outfit) == NULL) temp->outfit = otemp;
					else {
						while (ocur->next);
						ocur->next = otemp;
					}
				}
			}
		}

	}
	temp->thrust *= temp->mass; /* helps keep numbers sane */

	/* ship validator */
#define MELEMENT(o,s)		if (o == 0) WARN("Ship '%s' missing '"s"' element", temp->name)
	if (temp->name == NULL) WARN("Ship '%s' missing 'name' tag", temp->name);
	if (temp->gfx_space == NULL) WARN("Ship '%s' missing 'GFX' element", temp->name);
	MELEMENT(temp->thrust,"thrust");
	MELEMENT(temp->turn,"turn");
	MELEMENT(temp->speed,"speed");
	MELEMENT(temp->crew,"crew");
	MELEMENT(temp->mass,"mass");
	MELEMENT(temp->armor,"armor");
	MELEMENT(temp->armor_regen,"armor_regen");
	MELEMENT(temp->shield,"shield");
	MELEMENT(temp->shield_regen,"shield_regen");
	MELEMENT(temp->energy,"energy");
	MELEMENT(temp->energy_regen,"energy_regen");
	MELEMENT(temp->cap_cargo,"cap_cargo");
	MELEMENT(temp->cap_weapon,"cap_weapon");
#undef MELEMENT

	DEBUG("Loaded Ship '%s'", temp->name);
	return temp;
}


int ships_load(void)
{
	uint32_t bufsize;
	char *buf = pack_readfile(DATA, SHIP_DATA, &bufsize);

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	Ship* temp = NULL;

	node = doc->xmlChildrenNode; /* Ships node */
	if (strcmp((char*)node->name,XML_ID)) {
		ERR("Malformed "SHIP_DATA" file: missing root element '"XML_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first ship node */
	if (node == NULL) {
		ERR("Malformed "SHIP_DATA" file: does not contain elements");
		return -1;
	}

	do {
		if (node->type ==XML_NODE_START &&
				strcmp((char*)node->name,XML_SHIP)==0) {
			temp = ship_parse(node);
			ship_stack = realloc(ship_stack, sizeof(Ship)*(++ships));
			memcpy(ship_stack+ships-1, temp, sizeof(Ship));
			free(temp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	return 0;
}

void ships_free()
{
	ShipOutfit *so, *sot;
	int i;
	for (i = 0; i < ships; i++) {
		if ((ship_stack+i)->name) /* free the name */
			free((ship_stack+i)->name);
		so=(ship_stack+i)->outfit;
		while (so) { /* free the outfits */
			sot = so;
			so = so->next;
			free(sot);
		}

		gl_freeTexture((ship_stack+i)->gfx_space);
		gl_freeTexture((ship_stack+i)->gfx_target);
	}
	free(ship_stack);
	ship_stack = NULL;
}
