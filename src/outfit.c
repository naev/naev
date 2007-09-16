/*
 * See Licensing and Copyright notice in naev.h
 */



#include "outfit.h"

#include <math.h>
#include <string.h>

#include "xml.h"

#include "naev.h"
#include "log.h"
#include "pack.h"


#define outfit_setProp(o,p)		((o)->properties |= p)


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
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent );
static void outfit_parseSAmmo( Outfit* temp, const xmlNodePtr parent );


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
	return ( (o->type==OUTFIT_TYPE_MISSILE_DUMB)	||
			(o->type==OUTFIT_TYPE_MISSILE_SEEK)		||
			(o->type==OUTFIT_TYPE_MISSILE_SEEK_SMART) ||
			(o->type==OUTFIT_TYPE_MISSILE_SWARM)	||
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
 * returns the broad outfit type
 */
const char* outfit_typenamebroad[] = { "NULL",
		"Weapon",
		"Launcher",
		"Ammo"
};
const char* outfit_getTypeBroad( const Outfit* o )
{
	int i = 0;
	if (outfit_isWeapon(o)) i = 1;
	else if (outfit_isLauncher(o)) i = 2;
	else if (outfit_isAmmo(o)) i = 3;

	return outfit_typenamebroad[i];
}


/*
 * parses the specific area for a weapon and loads it into Outfit
 */
static void outfit_parseSWeapon( Outfit* temp, const xmlNodePtr parent )
{
	xmlNodePtr cur, node;
	node  = parent->xmlChildrenNode;

	char str[PATH_MAX] = "\0";

	do { /* load all the data */
		if (xml_isNode(node,"speed")) temp->speed = xml_getFloat(node);
		else if (xml_isNode(node,"delay")) temp->delay = xml_getInt(node);
		else if (xml_isNode(node,"range")) temp->range = xml_getFloat(node);
		else if (xml_isNode(node,"accuracy")) temp->accuracy = xml_getFloat(node);
		else if (xml_isNode(node,"gfx")) {
			snprintf( str, strlen(xml_get(node))+sizeof(OUTFIT_GFX)+4,
					OUTFIT_GFX"%s.png", xml_get(node));
			temp->gfx_space = gl_newSprite(str, 6, 6);
		}
		else if (xml_isNode(node,"sound"))
			temp->sound = sound_get( xml_get(node) );
		else if (xml_isNode(node,"damage")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"armour")) temp->damage_armour = xml_getFloat(cur);
				else if (xml_isNode(cur,"shield")) temp->damage_shield = xml_getFloat(cur);
			} while ((cur = cur->next));
		}
	} while ((node = node->next));

#define MELEMENT(o,s)      if ((o) == 0) WARN("Outfit '%s' missing '"s"' element", temp->name)
	if (temp->gfx_space==NULL)
		WARN("Outfit '%s' missing 'gfx' element", temp->name);
	MELEMENT(temp->sound,"sound");
	MELEMENT(temp->delay,"delay");
	MELEMENT(temp->speed,"speed");
	MELEMENT(temp->range,"range");
	MELEMENT(temp->accuracy,"accuracy");
	MELEMENT(temp->damage_armour,"armour' from element 'damage");
	MELEMENT(temp->damage_shield,"shield' from element 'damage");
#undef MELEMENT
}


/*
 * parses the specific area for a launcher and loads it into Outfit
 */
static void outfit_parseSLauncher( Outfit* temp, const xmlNodePtr parent )
{
	xmlNodePtr node;
	node  = parent->xmlChildrenNode;


	do { /* load all the data */
		if (xml_isNode(node,"delay")) temp->delay = xml_getInt(node);
		else if (xml_isNode(node,"ammo")) temp->ammo = strdup(xml_get(node));
	} while ((node = node->next));

#define MELEMENT(o,s)      if (o) WARN("Outfit '%s' missing '"s"' element", temp->name)
	MELEMENT(temp->ammo==NULL,"ammo");
	MELEMENT(temp->delay==0,"delay");
#undef MELEMENT
}


/*
 * parses the specific area for a weapon and loads it into Outfit
 */
static void outfit_parseSAmmo( Outfit* temp, const xmlNodePtr parent )
{
	xmlNodePtr cur, node;
	node  = parent->xmlChildrenNode;

	char str[PATH_MAX] = "\0";

	do { /* load all the data */
		if (xml_isNode(node,"thrust")) temp->thrust = xml_getFloat(node);
		else if (xml_isNode(node,"turn")) temp->turn = xml_getFloat(node);
		else if (xml_isNode(node,"speed")) temp->speed = xml_getFloat(node);
		else if (xml_isNode(node,"duration"))
			temp->duration = 1000*(unsigned int)xml_getFloat(node);
		else if (xml_isNode(node,"gfx")) {
			snprintf( str, strlen(xml_get(node))+sizeof(OUTFIT_GFX)+4,
					OUTFIT_GFX"%s.png", xml_get(node));
			temp->gfx_space = gl_newSprite(str, 6, 6);
		}
		else if (xml_isNode(node,"sound"))
			temp->sound = sound_get( xml_get(node) );
		else if (xml_isNode(node,"damage")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"armour")) temp->damage_armour = xml_getFloat(cur);
				else if (xml_isNode(cur,"shield")) temp->damage_shield = xml_getFloat(cur);
			} while ((cur = cur->next));
		}
	} while ((node = node->next));

#define MELEMENT(o,s)      if (o) WARN("Outfit '%s' missing '"s"' element", temp->name)
	MELEMENT(temp->gfx_space==NULL,"gfx");
	MELEMENT(temp->sound==0,"sound");
	MELEMENT(temp->thrust==0,"thrust");
	MELEMENT(temp->turn==0,"turn");
	MELEMENT(temp->speed==0,"speed");
	MELEMENT(temp->range==0,"duration");
	MELEMENT(temp->damage_armour==0,"armour' from element 'damage");
	MELEMENT(temp->damage_shield==0,"shield' from element 'damage");
#undef MELEMENT
}


/*
 * parses and returns Outfit from parent node
 */
static Outfit* outfit_parse( const xmlNodePtr parent )
{
	Outfit* temp = CALLOC_ONE(Outfit);
	xmlNodePtr cur, node;
	char *prop;

	temp->name = xml_nodeProp(parent,"name"); /* already mallocs */
	if (temp->name == NULL) WARN("Outfit in "OUTFIT_DATA" has invalid or no name");

	node = parent->xmlChildrenNode;

	do { /* load all the data */
		if (xml_isNode(node,"general")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"max")) temp->max = xml_getInt(cur);
				else if (xml_isNode(cur,"tech")) temp->tech = xml_getInt(cur);
				else if (xml_isNode(cur,"mass")) temp->mass = xml_getInt(cur);
			} while ((cur = cur->next));
		}
		else if (xml_isNode(node,"specific")) { /* has to be processed seperately */

			/* get the type */
			prop = xml_nodeProp(node,"type");
			if (prop == NULL)
				ERR("Outfit '%s' element 'specific' missing property 'type'",temp->name);
			temp->type = atoi(prop);
			free(prop);

			/* is secondary weapon? */
			prop = xml_nodeProp(node,"secondary");
			if (prop != NULL) {
				if ((int)atoi(prop)) outfit_setProp(temp, OUTFIT_PROP_WEAP_SECONDARY);
				free(prop);
			}

			if (temp->type==OUTFIT_TYPE_NULL)
				WARN("Outfit '%s' is of type NONE", temp->name);
			else if (outfit_isWeapon(temp))
				outfit_parseSWeapon( temp, node );
			else if (outfit_isLauncher(temp))
				outfit_parseSLauncher( temp, node );
			else if (outfit_isAmmo(temp))
				outfit_parseSAmmo( temp, node );
		}
	} while ((node = node->next));

#define MELEMENT(o,s)      if (o) WARN("Outfit '%s' missing '"s"' element", temp->name)
	MELEMENT(temp->name==NULL,"name");
	MELEMENT(temp->max==0,"max");
	MELEMENT(temp->tech==0,"tech");
	/*MELEMENT(temp->mass==0,"mass"); Not really needed */
	MELEMENT(temp->type==0,"type");
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
	if (!xml_isNode(node,XML_OUTFIT_ID)) {
		ERR("Malformed '"OUTFIT_DATA"' file: missing root element '"XML_OUTFIT_ID"'");
		return -1;
	}        

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed '"OUTFIT_DATA"' file: does not contain elements");
		return -1;
	}        

	do {
		if (xml_isNode(node,XML_OUTFIT_TAG)) {

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
		/* free graphics */
		if (outfit_stack[i].gfx_space) gl_freeTexture(outfit_stack[i].gfx_space);

		if (outfit_isLauncher(&outfit_stack[i]) && outfit_stack[i].ammo)
			free(outfit_stack[i].ammo);

		free(outfit_stack[i].name);
	}
	free(outfit_stack);
}

