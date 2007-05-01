

#include "outfit.h"

#include <math.h>
#include <string.h>

#include "libxml/parser.h"

#include "main.h"
#include "log.h"
#include "pack.h"

#define XML_NODE_START  1
#define XML_NODE_TEXT   3

#define XML_OUTFIT_ID		"Outfits"	/* XML section identifier */
#define XML_OUTFIT_TAG		"outfit"

#define OUTFIT_DATA	"dat/outfit.xml"
#define OUTFIT_GFX	"gfx/outfit/"


static Outfit* outfit_stack = NULL;
static int outfits = 0;


/*
 * Prototypes
 */
static Outfit* outfit_parse( const xmlNodePtr parent );
static void outfit_parseSWeapon( Outfit* temp, const xmlNodePtr parent );


/*
 * returns the outfit
 */
Outfit* outfit_get( const char* name )
{
	int i;
	for (i=0; i<outfits; i++)
		if (strcmp(name,outfit_stack[i].name)==0)
			return &outfit_stack[i];
	return NULL;
}


/*
 * return 1 if o is a weapon
 */
int outfit_isWeapon( const Outfit* o )
{
	return ( (o->type==OUTFIT_TYPE_BOLT)		||
			(o->type==OUTFIT_TYPE_BEAM) );
}
/*
 * return 1 if o is a launcher
 */
int outfit_isLauncher( const Outfit* o )
{
	return ( (o->type==OUTFIT_TYPE_MISSILE_DUMB) ||
			(o->type==OUTFIT_TYPE_MISSILE_SEEK) ||
			(o->type==OUTFIT_TYPE_MISSILE_SEEK_SMART) ||
			(o->type==OUTFIT_TYPE_MISSILE_SWARM) ||
			(o->type==OUTFIT_TYPE_MISSILE_SWARM_SMART) );
}

/*
 * return 1 if o is weapon ammunition
 */
int outfit_isAmmo( const Outfit* o )
{
	return ( (o->type==OUTFIT_TYPE_MISSILE_DUMB_AMMO)	||
			(o->type==OUTFIT_TYPE_MISSILE_SEEK_AMMO)		||
			(o->type==OUTFIT_TYPE_MISSILE_SEEK_SMART_AMMO) ||
			(o->type==OUTFIT_TYPE_MISSILE_SWARM_AMMO)		||
			(o->type==OUTFIT_TYPE_MISSILE_SWARM_SMART_AMMO) );
}

/*
 * returns the associated name
 */
const char* outfit_typename[] = { "NULL",
		"Bolt Cannon",
		"Beam Cannon",
		"Dumb Missile",
		"Dumb Missile Ammunition",
		"Seeker Missile",
		"Seeker Missile Ammunition",
		"Smart Seeker Missile",
		"Smart Seeker Missile Ammunition",
		"Swarm Missile",
		"Swarm Missile Ammunition Pack",
		"Smart Swarm Missile",
		"Smart Swarm Missile Ammunition Pack"
};
const char* outfit_getType( const Outfit* o )
{
	return outfit_typename[o->type];
}


/*
 * parses the specific area for a weapon and loads it into Outfit
 */
static void outfit_parseSWeapon( Outfit* temp, const xmlNodePtr parent )
{
	xmlNodePtr cur, node;
	node  = parent->xmlChildrenNode;

	char str[PATH_MAX] = "\0";

	while ((node = node->next)) { /* load all the data */
		if (strcmp((char*)node->name,"speed")==0)
			temp->speed = (double)atoi((char*)node->children->content);
		else if (strcmp((char*)node->name,"delay")==0)
			temp->delay = atoi((char*)node->children->content);
		else if (strcmp((char*)node->name,"accuracy")==0)
			temp->accuracy = atof((char*)node->children->content)*M_PI/180.; /* to rad */
		else if (strcmp((char*)node->name, "gfx")==0) {
			snprintf( str, strlen((char*)node->children->content)+sizeof(OUTFIT_GFX),
					OUTFIT_GFX"%s", (char*)node->children->content);
			temp->gfx_space = gl_newSprite(str, 6, 6);
		}
		if (strcmp((char*)node->name,"damage")==0) {
			cur = node->children;
			while ((cur = cur->next)) {
				if (strcmp((char*)cur->name,"armor")==0)
					temp->damage_armor = atof((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"shield")==0)
					temp->damage_shield = atof((char*)cur->children->content);
			}
		}
	}

#define MELEMENT(o,s)      if ((o) == 0) WARN("Outfit '%s' missing '"s"' element", temp->name)
	MELEMENT(temp->speed,"speed");
	MELEMENT(temp->accuracy,"tech");
	MELEMENT(temp->delay,"delay");
	MELEMENT(temp->damage_armor,"armor' from element 'damage");
	MELEMENT(temp->damage_shield,"shield' from element 'damage");
#undef MELEMENT
}


/*
 * parses and returns Outfit from parent node
 */
static Outfit* outfit_parse( const xmlNodePtr parent )
{
	Outfit* temp = CALLOC_ONE(Outfit);
	xmlNodePtr cur, node;
	xmlChar* prop;

	temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name"); /* already mallocs */

	node  = parent->xmlChildrenNode;

	while ((node = node->next)) { /* load all the data */
		if (strcmp((char*)node->name,"general")==0) {
			cur = node->children;
			while ((cur = cur->next)) {
				if (strcmp((char*)cur->name,"max")==0)
					temp->max = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"tech")==0)
					temp->tech = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"mass")==0)
					temp->mass = atoi((char*)cur->children->content);
			}
		}
		else if (strcmp((char*)node->name,"specific")==0) { /* has to be processed seperately */
			prop = xmlGetProp(node,(xmlChar*)"type");
			if (prop == NULL)
				ERR("Outfit '%s' element 'specific' missing property 'type'",temp->name);
			temp->type = atoi((char*)prop);
			free(prop);
			switch (temp->type) {
				case OUTFIT_TYPE_NULL:
					WARN("Outfit '%s' is of type NONE", temp->name);
					break;
				case OUTFIT_TYPE_BOLT:
					outfit_parseSWeapon( temp, node );
					break;

				default:
					break;
			}
		}
	}

#define MELEMENT(o,s)      if ((o) == 0) WARN("Outfit '%s' missing '"s"' element", temp->name)
	if (temp->name == NULL) WARN("Outfit '%s' missing 'name' tag", temp->name);
	MELEMENT(temp->max,"max");
	MELEMENT(temp->tech,"tech");
	MELEMENT(temp->mass,"mass");
	MELEMENT(temp->type,"type");
#undef MELEMENT

	DEBUG("Loaded Outfit '%s' of type '%s'", temp->name, outfit_getType(temp));

	return temp;
}


/*
 * loads all the outfits into the outfit_stack
 */
int outfit_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, OUTFIT_DATA, &bufsize );

	Outfit *temp;

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (strcmp((char*)node->name,XML_OUTFIT_ID)) {
		ERR("Malformed "OUTFIT_DATA"file: missing root element '"XML_OUTFIT_ID"'");
		return -1;
	}        

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed "OUTFIT_DATA" file: does not contain elements");
		return -1;
	}        

	do {
		if (node->type == XML_NODE_START &&
				strcmp((char*)node->name,XML_OUTFIT_TAG)==0) {

			temp = outfit_parse(node);               
			outfit_stack = realloc(outfit_stack, sizeof(Outfit)*(++outfits));
			memcpy(outfit_stack+outfits-1, temp, sizeof(Outfit));
			free(temp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	return 0;
}


/*
 * frees the outfit stack
 */
void outfit_free (void)
{
	int i;
	for (i=0; i < outfits; i++) {
		if (outfit_isWeapon(&outfit_stack[i]) && outfit_stack[i].gfx_space)
			gl_freeTexture(outfit_stack[i].gfx_space);
		free(outfit_stack[i].name);
	}
	free(outfit_stack);
}
