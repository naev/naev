

#include "faction.h"

#include <malloc.h>
#include <string.h>

#include "libxml/parser.h"

#include "main.h"
#include "log.h"
#include "pack.h"

#define XML_NODE_START	1
#define XML_NODE_TEXT	3

#define XML_FACTION_ID		"Factions"   /* XML section identifier */
#define XML_FACTION_TAG		"faction"
#define XML_ALLIANCE_ID		"Alliances"
#define XML_ALLIANCE_TAG	"alliance"
#define XML_ENEMIES_ID		"Enemies"
#define XML_ENEMIES_TAG		"enemies"

#define FACTION_DATA  "dat/faction.xml"


Faction* faction_stack = NULL;
int nfactions = 0;


/*
 * Prototypes
 */
static Faction* faction_parse( xmlNodePtr parent );
static void alliance_parse( xmlNodePtr parent );
static void enemies_parse( xmlNodePtr parent );


/*
 * returns the faction of name name
 */
Faction* faction_get( const char* name )
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
	int i;

	if (a==b) return 0;

	for (i=0;i<a->nenemies;i++)
		if (a->enemies[i] == b)
			return 1;
	for (i=0;i<b->nenemies;i++)
		if(b->enemies[i] == a)
			return 1;
	return 0;
}


/*
 * returns 1 if Faction a and b are allies
 */
int areAllies( Faction* a, Faction* b )
{
	int i;

	if (a==b) return 1;

	for (i=0;i<a->nallies;i++)
		if (a->allies[i] == b)                               
			return 1;
	for (i=0;i<b->nallies;i++)
		if(b->allies[i] == a)
			return 1;
	return 0;
}


/*
 * parses a single faction, but doesn't set the allies/enemies bit
 */
static Faction* faction_parse( xmlNodePtr parent )
{
	Faction* temp = CALLOC_ONE(Faction);

	temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name");
	if (temp->name == NULL) WARN("Faction from "FACTION_DATA" has invalid or no name");

	return temp;
}


/*
 * sets the allies bit in the faction_stack
 */
static void alliance_parse( xmlNodePtr parent )
{
	Faction** f = NULL;
	int i,j,n,m;
	i = 0;
	char* name = NULL;
	xmlNodePtr node, cur;

	node = parent->xmlChildrenNode;

	do {                                
		if (node->type==XML_NODE_START) {
			if (strcmp((char*)node->name,XML_ALLIANCE_TAG)==0) {
				name = (char*)xmlGetProp(node,(xmlChar*)"name");


				/* parse the current alliance's allies */
				cur = node->xmlChildrenNode;
				do {
					if (strcmp((char*)cur->name,"ally")==0) {
						f = realloc(f,(++i)*sizeof(Faction*));
						f[i-1] = faction_get((char*)cur->children->content);
						if (f[i-1]==NULL)
							WARN("Faction %s in alliance %s doesn't exist in "FACTION_DATA,
									(char*)cur->children->content, name );
					}
				} while ((cur = cur->next));


				/* set the stuff needed in faction_stack */
				for (j=0; j<i; j++) {
					f[j]->nallies += i-1;
					f[j]->allies = realloc( f[j]->allies, f[j]->nallies*sizeof(Faction*));
					for (n=0,m=0; n<i; n++,m++) { /* add as ally all factions except self */
						if (n==j) m--;
						else if (n!=j) f[j]->allies[ f[j]->nallies-i+1+m ] = f[n];
					}
				}


				/* free some memory */
				if (f) {
					free(f);
					f = NULL;
					i = 0;
				}
				if (name) free(name);
			}
		}

	} while ((node = node->next));
}


/*
 * sets the enemies bit in the faction_stack
 */
static void enemies_parse( xmlNodePtr parent )
{
	xmlNodePtr node;

	node = parent->xmlChildrenNode;

	do {
		if (node->type==XML_NODE_START) {
			if (strcmp((char*)node->name,XML_ENEMIES_TAG)==0) {
			}
		}
	} while ((node = node->next));

}


/*
 * loads up all the factions
 */
int factions_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile(DATA, FACTION_DATA, &bufsize);

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	Faction* temp = NULL;

	node = doc->xmlChildrenNode; /* Factions node */
	if (strcmp((char*)node->name,XML_FACTION_ID)) {
		ERR("Malformed "FACTION_DATA" file: missing root element '"XML_FACTION_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first faction node */
	if (node == NULL) {
		ERR("Malformed "FACTION_DATA" file: does not contain elements");
		return -1;
	}

	do {
		if (node->type==XML_NODE_START) {
			if (strcmp((char*)node->name,XML_FACTION_TAG)==0) {
				temp = faction_parse(node);
				faction_stack = realloc(faction_stack, sizeof(Faction)*(++nfactions));        
				memcpy(faction_stack+nfactions-1, temp, sizeof(Faction));                  
				free(temp);
			}
			else if (strcmp((char*)node->name,XML_ALLIANCE_ID)==0)
				alliance_parse(node);
			else if (strcmp((char*)node->name,XML_ENEMIES_ID)==0)
				enemies_parse(node);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	DEBUG("Loaded %d faction%c", nfactions, (nfactions==1)?' ':'s');

	return 0;
}

void factions_free (void)
{
	int i;
	for (i=0; i<nfactions; i++) {
		free(faction_stack[i].name);
		if (faction_stack[i].nallies > 0) free(faction_stack[i].allies);
		if (faction_stack[i].nenemies > 0) free(faction_stack[i].enemies);
	}
	free(faction_stack);
	nfactions = 0;
}

