

#include "ship.h"

#include <string.h>

#include "xml.h"

#include "main.h"
#include "log.h"
#include "pack.h"


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
Ship* ship_get( const char* name )
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
	char* stmp;

	temp->name = xml_nodeProp(parent,"name");
	if (temp->name == NULL) WARN("Ship in "SHIP_DATA" has invalid or no name");

	node = parent->xmlChildrenNode;

	do { /* load all the data */
		if (xml_isNode(node,"GFX")) {
			snprintf( str, strlen(xml_get(node))+
					sizeof(SHIP_GFX)+sizeof(SHIP_EXT),
					SHIP_GFX"%s"SHIP_EXT, xml_get(node));
			temp->gfx_space = gl_newSprite(str, 6, 6);
			snprintf( str, strlen(xml_get(node))+
					sizeof(SHIP_GFX)+sizeof(SHIP_TARGET)+sizeof(SHIP_EXT),
					SHIP_GFX"%s"SHIP_TARGET SHIP_EXT, xml_get(node));
			temp->gfx_target = gl_newImage(str);
		}
		else if (xml_isNode(node,"GUI"))
			temp->gui = strdup(xml_get(node));
		else if (xml_isNode(node,"sound"))
			temp->sound = sound_get( xml_get(node) );
		else if (xml_isNode(node,"class"))
			temp->class = atoi(xml_get(node));
		else if (xml_isNode(node,"movement")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"thrust"))
					temp->thrust = xml_getInt(cur);
				else if (xml_isNode(cur,"turn"))
					temp->turn = xml_getInt(cur);
				else if (xml_isNode(cur,"speed"))
					temp->speed = xml_getInt(cur);
			} while ((cur = cur->next));
		}
		else if (xml_isNode(node,"health")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"armour"))
					temp->armour = (double)xml_getInt(cur);
				else if (xml_isNode(cur,"shield"))
					temp->shield = (double)xml_getInt(cur);
				else if (xml_isNode(cur,"energy"))
					temp->energy = (double)xml_getInt(cur);
				else if (xml_isNode(cur,"armour_regen"))
					temp->armour_regen = (double)(xml_getInt(cur))/60.0;
				else if (xml_isNode(cur,"shield_regen"))
					temp->shield_regen = (double)(xml_getInt(cur))/60.0;
				else if (xml_isNode(cur,"energy_regen"))
					temp->energy_regen = (double)(xml_getInt(cur))/60.0;
			} while ((cur = cur->next));
		}
		else if (xml_isNode(node,"caracteristics")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"crew"))
					temp->crew = xml_getInt(cur);
				else if (xml_isNode(cur,"mass"))
					temp->mass = (double)xml_getInt(cur);
				else if (xml_isNode(cur,"cap_weapon"))
					temp->cap_weapon = xml_getInt(cur);
				else if (xml_isNode(cur,"cap_cargo"))
					temp->cap_cargo = xml_getInt(cur);
			} while ((cur = cur->next));
		}
		else if (xml_isNode(node,"outfits")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"outfit")) {
					otemp = MALLOC_ONE(ShipOutfit);
					otemp->data = outfit_get(xml_get(cur));
					stmp = xml_nodeProp(cur,"quantity");
					if (!stmp)
						WARN("Ship '%s' is missing tag 'quantity' for outfit '%s'",
								temp->name, otemp->data->name);
					otemp->quantity = atoi(stmp);
					free(stmp);
					otemp->next = NULL;
					
					if ((ocur=temp->outfit) == NULL) temp->outfit = otemp;
					else {
						while (ocur->next) ocur = ocur->next;
						ocur->next = otemp;
					}
				}
			} while ((cur = cur->next));
		}
	} while ((node = node->next));
	temp->thrust *= temp->mass; /* helps keep numbers sane */

	/* ship validator */
#define MELEMENT(o,s)		if (o == 0) WARN("Ship '%s' missing '"s"' element", temp->name)
	if (temp->name == NULL) WARN("Ship '%s' missing 'name' tag", temp->name);
	if (temp->gfx_space == NULL) WARN("Ship '%s' missing 'GFX' element", temp->name);
	if (temp->gui == NULL) WARN("Ship '%s' missing 'GUI' element", temp->name);
	MELEMENT(temp->thrust,"thrust");
	MELEMENT(temp->turn,"turn");
	MELEMENT(temp->speed,"speed");
	MELEMENT(temp->armour,"armour");
	MELEMENT(temp->armour_regen,"armour_regen");
	MELEMENT(temp->shield,"shield");
	MELEMENT(temp->shield_regen,"shield_regen");
	MELEMENT(temp->energy,"energy");
	MELEMENT(temp->energy_regen,"energy_regen");
	MELEMENT(temp->crew,"crew");
	MELEMENT(temp->mass,"mass");
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
		/* free stored strings */
		if ((ship_stack+i)->name) free(ship_stack[i].name);
		if ((ship_stack+i)->gui) free(ship_stack[i].gui);

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
