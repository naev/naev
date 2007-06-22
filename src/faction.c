

#include "faction.h"

#include <malloc.h>
#include <string.h>

#include "libxml/parser.h"

#include "main.h"
#include "log.h"
#include "pack.h"

#define XML_NODE_START  1
#define XML_NODE_TEXT   3

#define XML_FACTION_ID      "Factions"   /* XML section identifier */
#define XML_FACTION_TAG     "faction"

#define FACTION_DATA  "dat/faction.xml"


Faction* faction_stack = NULL;
int nfactions = 0;


/*
 * Prototypes
 */
static Faction* faction_parse( xmlNodePtr parent );


/*
 * returns the faction of name name
 */
Faction* get_faction( const char* name )
{
	int i;
	for (i=0; i<nfactions; i++) 
		if (strcmp(faction_stack[i].name, name)==0)
			break;

	if (i != nfactions)
		return faction_stack+i;
	return NULL;
}


/*
 * returns 1 if Faction a and b are enemies
 */
int areEnemies( Faction* a, Faction* b)
{
	int i = 0;
	while ( a->enemies[i] != NULL && b != a->enemies[i] ) i++;

	if (a->enemies[i] == NULL) return 0;
	return 1;
}


/*
 * returns 1 if Faction a and b are allies
 */
int areAllies( Faction* a, Faction* b )
{
	int i = 0;
	while ( a->allies[i] != NULL && b != a->allies[i] ) i++;

	if (a->allies[i] == NULL) return 0;
	return 1;
}


static Faction* faction_parse( xmlNodePtr parent )
{
	Faction* temp = CALLOC_ONE(Faction);

	temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name");

	return temp;
}


int factions_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile(DATA, FACTION_DATA, &bufsize);

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	Faction* temp = NULL;

	node = doc->xmlChildrenNode; /* Ships node */
	if (strcmp((char*)node->name,XML_FACTION_ID)) {
		ERR("Malformed "FACTION_DATA" file: missing root element '"XML_FACTION_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first ship node */
	if (node == NULL) {
		ERR("Malformed "FACTION_DATA" file: does not contain elements");
		return -1;
	}

	do {
		if (node->type ==XML_NODE_START &&
				strcmp((char*)node->name,XML_FACTION_TAG)==0) {
			temp = faction_parse(node);
			faction_stack = realloc(faction_stack, sizeof(Faction)*(++nfactions));        
			memcpy(faction_stack+nfactions-1, temp, sizeof(Faction));                  
			free(temp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	DEBUG("Loaded %d factions",nfactions);

	return 0;
}

void factions_free (void)
{
	free(faction_stack);
	nfactions = 0;
}

