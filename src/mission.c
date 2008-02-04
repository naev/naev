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
#include "faction.h"


#define XML_MISSION_ID			"Missions"   /* XML section identifier */
#define XML_MISSION_TAG			"mission"

#define MISSION_DATA				"dat/mission.xml"
#define MISSION_LUA_PATH		"dat/missions/"


/* L state, void* buf, int n size, char* s identifier */
#define luaL_dobuffer(L, b, n, s) \
	(luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))


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
static int mission_init( Mission* mission, MissionData* misn );
static void mission_freeData( MissionData* mission );
static int mission_matchFaction( MissionData* misn, int faction );
static int mission_location( char* loc );
static MissionData* mission_parse( const xmlNodePtr parent );


/*
 * initializes a mission
 */
static int mission_init( Mission* mission, MissionData* misn )
{
	char *buf;
	uint32_t bufsize;

	mission->id = ++mission_id;
	mission->data = misn;

	/* sane defaults */
	mission->title = NULL;
	mission->desc = NULL;
	mission->reward = NULL;

	/* init lua */
	mission->L = luaL_newstate();
	if (mission->L == NULL) {
		ERR("Unable to create a new lua state.");
		return -1;
	}
	luaopen_string( mission->L ); /* string.format can be very useful */
	misn_loadLibs( mission->L ); /* load our custom libraries */

	/* load the file */
	buf = pack_readfile( DATA, misn->lua, &bufsize );
	if (luaL_dobuffer(mission->L, buf, bufsize, misn->lua) != 0) {
		ERR("Error loading AI file: %s",misn->lua);
		ERR("%s",lua_tostring(mission->L,-1));
		WARN("Most likely Lua file has improper syntax, please check");
		return -1;
	}
	free(buf);


	/* run create function */
	misn_run( mission, "create");

	return mission->id;
}


/*
 * adds a mission to the player, you can free the current mission safely
 */
int mission_add( Mission* mission )
{
	int i;

	/* find last mission */
	for (i=0; i<MISSION_MAX; i++)
		if (player_missions[i].data == NULL) break;

	/* no missions left */
	if (i>=MISSION_MAX) return -1;

	/* copy it over */
	memcpy( &player_missions[i], mission, sizeof(Mission) );
	memset( mission, 0, sizeof(Mission) );

	return player_missions[i].id;
}

/*
 * cleans up a mission
 */
void mission_cleanup( Mission* misn )
{
	hook_rmParent( misn->id ); /* remove existing hooks */
	if (misn->title) free(misn->title);
	if (misn->desc) free(misn->desc);
	if (misn->reward) free(misn->reward);
	lua_close(misn->L);
	memset(misn, 0, sizeof(Mission));
}


/*
 * frees a mission
 */
static void mission_freeData( MissionData* mission )
{
	if (mission->name) free(mission->name);
	if (mission->lua) free(mission->lua);
	if (mission->avail.planet) free(mission->avail.planet);
	if (mission->avail.system) free(mission->avail.system);
	if (mission->avail.factions) free(mission->avail.factions);
	memset( mission, 0, sizeof(MissionData) );
}


/*
 * does mission match faction requirement?
 */
static int mission_matchFaction( MissionData* misn, int faction )
{
	int i;

	for (i=0; i<misn->avail.nfactions; i++) {
		if (faction_isFaction(misn->avail.factions[i]) &&
				(faction == misn->avail.factions[i]))
			return 1;
		else if (faction_ofAlliance( faction, misn->avail.factions[i]))
			return 1;
	}

	return 0;
}


/*
 * generates misisons for the computer - special case
 */
Mission* missions_computer( int *n, int faction, char* planet, char* system )
{
	int i, m;
	Mission* tmp;
	MissionData* misn;

	tmp = NULL;
	m = 0;
	for (i=0; i<mission_nstack; i++) {
		misn = &mission_stack[i];
		if ((misn->avail.loc==MIS_AVAIL_COMPUTER) &&
			(((misn->avail.planet && strcmp(misn->avail.planet,planet)==0)) ||
				(misn->avail.system && (strcmp(misn->avail.system,system)==0)) ||
				mission_matchFaction(misn,faction))) {
			tmp = realloc( tmp, sizeof(Mission) * ++m);
			mission_init( &tmp[m-1], misn );
		}
	}

	(*n) = m;
	return tmp;
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
		else if (xml_isNode(node,"flags")) { /* set the various flags */
			cur = node->children;
			do {
				if (xml_isNode(cur,"unique"))
					mis_setFlag(temp,MISSION_UNIQUE);
			} while ((cur = cur->next));
		}
		else if (xml_isNode(node,"avail")) { /* mission availability */
			cur = node->children;
			do {
				if (xml_isNode(cur,"location"))
					temp->avail.loc = mission_location( xml_get(cur) );
				else if (xml_isNode(cur,"planet"))
					temp->avail.planet = strdup( xml_get(cur) );
				else if (xml_isNode(cur,"system"))
					temp->avail.system = strdup( xml_get(cur) );
				else if (xml_isNode(cur,"alliance")) {
					temp->avail.factions = realloc( temp->avail.factions,
							sizeof(int) * ++temp->avail.nfactions );
					temp->avail.factions[temp->avail.nfactions-1] = 
							faction_getAlliance( xml_get(cur) );
				}
				else if (xml_isNode(cur,"faction")) {
					temp->avail.factions = realloc( temp->avail.factions, 
							sizeof(int) * ++temp->avail.nfactions );
					temp->avail.factions[temp->avail.nfactions-1] =
							faction_get( xml_get(cur) );
				}
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
		mission_freeData( &mission_stack[i] );
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




