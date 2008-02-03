/*
 * See Licensing and Copyright notice in naev.h
 */


#include "mission.h"

#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "naev.h"
#include "log.h"
#include "hook.h"
#include "pack.h"
#include "xml.h"


#define XML_MISSION_ID			"Missions"   /* XML section identifier */
#define XML_MISSION_TAG			"mission"

#define MISSION_DATA				"dat/mission.xml"
#define MISSION_LUA_PATH		"dat/missions/"


/*
 * current player missions
 */
static unsigned int mission_id = 0;
Mission player_missions[MISSION_MAX];


/*
 * mission stack
 */
static MissionData *mission_stack = NULL; /* unmuteable after creation */
static int mission_nstack = 0;


/*
 * prototypes
 */
/* extern */
extern int misn_run( Mission *misn, char *func );
/* static */
static void mission_cleanup( Mission* misn );
static void mission_free( MissionData* mission );
static int mission_location( char* loc );
static MissionData* mission_parse( const xmlNodePtr parent );


/*
 * creates a mission
 */
int mission_create( MissionData* misn )
{
	int i;

	/* find last mission */
	for (i=0; i<MISSION_MAX; i++)
		if (player_missions[i].data == NULL) break;
	
	/* no missions left */
	if (i>=MISSION_MAX) return -1;


	player_missions[i].id = ++mission_id;
	player_missions[i].data = misn;

	/* init lua */
	player_missions[i].L = luaL_newstate();
	luaopen_string( player_missions[i].L ); /* string.format can be very useful */
	misn_loadLibs( player_missions[i].L ); /* load our custom libraries */

	/* run create function */
	misn_run( &player_missions[i], "create");

	return 0;
}

/*
 * cleans up a mission
 */
static void mission_cleanup( Mission* misn )
{
	hook_rmParent( misn->id ); /* remove existing hooks */
	misn->data = NULL;
	if (misn->title) free(misn->title);
	if (misn->desc) free(misn->desc);
	if (misn->reward) free(misn->reward);
	lua_close(misn->L);
}


/*
 * frees a mission
 */
static void mission_free( MissionData* mission )
{
	if (mission->name) {
		free(mission->name);
		mission->name = NULL;
	}
	if (mission->avail.planet) {
		free(mission->avail.planet);
		mission->avail.planet = NULL;
	}
	if (mission->avail.system) {
		free(mission->avail.system);
		mission->avail.system = NULL;
	}
	if (mission->avail.factions) {
		free(mission->avail.factions);
		mission->avail.factions = NULL;
		mission->avail.nfactions = 0;
	}
}


/*
 * returns location based on string
 */
static int mission_location( char* loc )
{
	if (strcmp(loc,"None")==0) return MIS_AVAIL_NONE;
	else if (strcmp(loc,"Computer")==0) return MIS_AVAIL_COMPUTER;
	else if (strcmp(loc,"Bar")==0) return MIS_AVAIL_BAR;
	else if (strcmp(loc,"Outfit")==0) return MIS_AVAIL_OUTFIT;
	else if (strcmp(loc,"Shipyard")==0) return MIS_AVAIL_SHIPYARD;
	else if (strcmp(loc,"Land")==0) return MIS_AVAIL_LAND;
	return -1;
}


/*
 * parses a node of a mission
 */
static MissionData* mission_parse( const xmlNodePtr parent )
{
	MissionData *temp;
	xmlNodePtr cur, node;

	temp = malloc(sizeof(MissionData));
	memset( temp, 0, sizeof(MissionData) );

	/* get the name */
	temp->name = xml_nodeProp(parent,"name");
	if (temp->name == NULL) WARN("Mission in "MISSION_DATA" has invalid or no name");

	node = parent->xmlChildrenNode;

	char str[PATH_MAX] = "\0";

	do { /* load all the data */
		if (xml_isNode(node,"lua")) {
			snprintf( str, PATH_MAX, MISSION_LUA_PATH"%s.lua", xml_get(node) );
			temp->lua = strdup( str );
			str[0] = '\0';
		}
		else if (xml_isNode(node,"avail")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"location"))
					temp->avail.loc = mission_location( xml_get(cur) );
				/*else if (xml_isNode(cur,""))  need to parse other thingies
					temp->u.amm.damage_shield = xml_getFloat(cur);*/
			} while ((cur = cur->next));
		}
	} while ((node = node->next));

#define MELEMENT(o,s) \
	if (o) WARN("Mission '%s' missing/invalid '"s"' element", temp->name)
	MELEMENT(temp->lua==NULL,"lua");
	MELEMENT(temp->avail.loc==-1,"location");
#undef MELEMENT

	return temp;
}


/*
 * load/free
 */
int missions_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, MISSION_DATA, &bufsize );

	MissionData *temp;

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (!xml_isNode(node,XML_MISSION_ID)) {
		ERR("Malformed '"MISSION_DATA"' file: missing root element '"XML_MISSION_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first mission node */
	if (node == NULL) {
		ERR("Malformed '"MISSION_DATA"' file: does not contain elements");
		return -1;
	}

	do {
		if (xml_isNode(node,XML_MISSION_TAG)) {

			temp = mission_parse(node);
			mission_stack = realloc(mission_stack, sizeof(MissionData)*(++mission_nstack));
			memcpy(mission_stack+mission_nstack-1, temp, sizeof(MissionData));
			free(temp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	DEBUG("Loaded %d Mission%s", mission_nstack, (mission_nstack==1) ? "" : "s" );

	return 0;
}
void missions_free (void)
{
	int i;

	/* free the mission data */
	for (i=0; i<mission_nstack; i++)
		mission_free( &mission_stack[i] );
	free( mission_stack );
	mission_stack = NULL;
	mission_nstack = 0;
}
void missions_cleanup (void)
{
	int i;

	for (i=0; i<MISSION_MAX; i++)
		mission_cleanup( &player_missions[i] );
}




