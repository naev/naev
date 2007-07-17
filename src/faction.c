

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
 * used to save alliances
 */
typedef struct {
	char* name;
	Faction** factions;
	int nfactions;
} Alliance;


/*
 * alliance stack
 */
static Alliance* alliances = NULL;
static int nalliances = 0;


/*
 * Prototypes
 */
static Faction* faction_parse( xmlNodePtr parent );
static void alliance_parse( xmlNodePtr parent );
static void enemies_parse( xmlNodePtr parent );
static Alliance* alliance_get( char* name );


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
 * returns the alliance of name name
 */
static Alliance* alliance_get( char* name )
{
	int i;
	for (i=0; i<nalliances; i++)
		if (strcmp(alliances[i].name, name)==0)
			break;
	
	if (i != nalliances)
		return alliances+i;
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
	Alliance* a;
	int *i,j,n,m;
	xmlNodePtr node, cur;

	node = parent->xmlChildrenNode;

	do {                                
		if ((node->type==XML_NODE_START) &&
				(strcmp((char*)node->name,XML_ALLIANCE_TAG)==0)) {

			/* alloc a new alliance */
			alliances = realloc(alliances, sizeof(Alliance)*(++nalliances));
			alliances[nalliances-1].name = (char*)xmlGetProp(node,(xmlChar*)"name");
			alliances[nalliances-1].factions = NULL;
			alliances[nalliances-1].nfactions = 0;

			/* parse the current alliance's allies */
			cur = node->xmlChildrenNode;
			do {
				if (strcmp((char*)cur->name,"ally")==0) {

					/* add the faction (and pointers to make life easier */
					a = alliances + nalliances-1;
					i = &a->nfactions;
					(*i)++;

					/* load the faction */
					a->factions = realloc( a->factions, (*i)*sizeof(Faction*) );
					a->factions[(*i)-1] = faction_get((char*)cur->children->content);
							
					if (a->factions[(*i)-1]==NULL)
						WARN("Faction %s in alliance %s doesn't exist in "FACTION_DATA,
								(char*)cur->children->content, a->name );
				}
			} while ((cur = cur->next));


			/* set the stuff needed in faction_stack */
			for (j=0; j<(*i); j++) {
				a->factions[j]->nallies += (*i)-1;
				a->factions[j]->allies = realloc( a->factions[j]->allies, 
						(a->factions[j]->nallies)*sizeof(Faction*));
				for (n=0,m=0; n<(*i); n++,m++) { /* add as ally all factions except self */
					if (n==j) m--;
					else if (n!=j)
						a->factions[j]->allies[ a->factions[j]->nallies-(*i)+1+m ] =
								a->factions[n];
				}
			}
		}
	} while ((node = node->next));
}


/*
 * sets the enemies bit in the faction_stack
 */
static void enemies_parse( xmlNodePtr parent )
{
	xmlNodePtr node, cur;
	Faction*** f;
	Alliance* a;
	int i, *j, n, m, x, y, z, e;
	char* type;

	i = 0;
	f = NULL;
	j = NULL;

	node = parent->xmlChildrenNode;

	do {
		if ((node->type==XML_NODE_START) &&
				(strcmp((char*)node->name,XML_ENEMIES_TAG)==0)) {

			cur = node->xmlChildrenNode;
			do {
				if (strcmp((char*)cur->name,"enemy")==0) {

					type = (char*)xmlGetProp(cur,(xmlChar*)"type");
					
					i++;
					j = realloc(j, sizeof(int)*i);
					f = realloc(f, sizeof(Faction**)*i);

					/* enemy thing is an alliance */
					if (strcmp(type,"alliance")==0) {
						a = alliance_get( (char*)cur->children->content );
						if (a==NULL)
							WARN("Alliance %s not found in stack", (char*)cur->children->content);
						j[i-1] = a->nfactions;
						f[i-1] = a->factions;
					}
					/* enemy thing is only a faction */
					else if (strcmp(type,"faction")==0) {
						j[i-1] = 1;
						f[i-1] = malloc(sizeof(Faction*));
						f[i-1][0] = faction_get( (char*)cur->children->content );
						if (f[i-1][0]==NULL)
							WARN("Faction %s not found in stack", (char*)cur->children->content);
					}

					free(type);
				}
			} while ((cur = cur->next));

			/* now actually parse and load up the enemies */
			for (n=0; n<i; n++) { /* Faction** */
				for (m=0; m<j[n]; m++) { /* Faction* */

					/* add all the faction enemies to nenemies and alloc */
					for (e=0,x=0; x<i; x++)
						if (x!=n) e += j[x]; /* stores the total enemies */
					/* now alloc the memory */
					f[n][m]->nenemies += e;
					f[n][m]->enemies = realloc( f[n][m]->enemies,
							sizeof(Faction*)*f[n][m]->nenemies );

					/* now add the actual enemies */
					for (x=0,z=0; x<i; x++)
						if (x!=n) /* make sure isn't from the same group */
							for (y=0; y<j[x]; y++,z++)
								f[n][m]->enemies[ f[n][m]->nenemies-e + z ] = f[x][y];
				}
			}
			/* free all the temporary memory */
			for (x=0; x<i; x++)
				if (j[x]==1) free(f[x]); /* free the single malloced factions */
			free(f); /* free the rest */
			free(j);
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

	/* free aliances */
	for (i=0; i<nalliances; i++) {
		free(alliances[i].name);
		free(alliances[i].factions);
	}
	free(alliances);
	alliances = NULL;
	nalliances = 0;

	/* free factions */
	for (i=0; i<nfactions; i++) {
		free(faction_stack[i].name);
		if (faction_stack[i].nallies > 0) free(faction_stack[i].allies);
		if (faction_stack[i].nenemies > 0) free(faction_stack[i].enemies);
	}
	free(faction_stack);
	faction_stack = NULL;
	nfactions = 0;
}

