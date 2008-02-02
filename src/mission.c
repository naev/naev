/*
 * See Licensing and Copyright notice in naev.h
 */


#include "mission.h"

#include <string.h>
#include <malloc.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "naev.h"
#include "log.h"
#include "hook.h"



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
	lua_getglobal(player_missions[i].L, "create");
	if (lua_pcall(player_missions[i].L, 0, 0, 0)) /* error has occured */
		WARN("Mission '%s' -> 'create': %s",
				misn->name, lua_tostring(player_missions[i].L,-1));


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
 * load/free
 */
int missions_load (void)
{
#if 0
	yaml_parser_t parser;
	yaml_event_t input_event;

	memset(&parser, 0, sizeof(parser));
	memset(&input_event, 0, sizeof(input_event));

	/* set up parser */
	if (!yaml_parser_initialize(&parser)) {
		ERR("Could not initialize the parser object!");
		return -1;
	}
	yaml_parser_set_input_file(&parser, stdin);

	/* cleanup */
	yaml_event_delete(&input_event);
	yaml_parser_delete(&parser);
#endif

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




