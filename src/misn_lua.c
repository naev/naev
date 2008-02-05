/*
 * See Licensing and Copyright notice in naev.h
 */

#include "misn_lua.h"

#include "lua.h"
#include "lauxlib.h"

#include "hook.h"
#include "mission.h"
#include "log.h"
#include "naev.h"
#include "rng.h"
#include "space.h"
#include "toolkit.h"
#include "land.h"


#define MIN_ARGS(n)		if (lua_gettop(L) < n) return 0


/*
 * current mission
 */
static Mission *cur_mission = NULL;
static int misn_delete = 0; /* if 1 delete current mission */

/*
 * libraries
 */
/* misn */
static int misn_setTitle( lua_State *L );
static int misn_setDesc( lua_State *L );
static int misn_setReward( lua_State *L );
static int misn_factions( lua_State *L );
static int misn_accept( lua_State *L );
static int misn_finish( lua_State *L );
static const luaL_reg misn_methods[] = {
	{ "setTitle", misn_setTitle },
	{ "setDesc", misn_setDesc },
	{ "setReward", misn_setReward },
	{ "factions", misn_factions },
	{ "accept", misn_accept },
	{ "finish", misn_finish },
	{0,0}
};
/* space */
static int space_getPlanet( lua_State *L );
static int space_landName( lua_State *L );
static const luaL_reg space_methods[] = {
	{ "getPlanet", space_getPlanet },
	{ "landName", space_landName },
	{0,0}
};
/* player */
static int player_freeSpace( lua_State *L );
static int player_addCargo( lua_State *L );
static int player_rmCargo( lua_State *L );
static int player_pay( lua_State *L );
static const luaL_reg player_methods[] = {
	{ "freeCargo", player_freeSpace },
	{ "addCargo", player_addCargo },
	{ "rmCargo", player_rmCargo },
	{ "pay", player_pay },
	{0,0}
};
/* rnd */
static int rnd_int( lua_State *L );
static const luaL_reg rnd_methods[] = {
	{ "int", rnd_int },
	{0,0}
};
/* toolkit */
static int tk_msg( lua_State *L );
static int tk_yesno( lua_State *L );
static int tk_input( lua_State *L );
static const luaL_reg tk_methods[] = {
	{ "msg", tk_msg },
	{ "yesno", tk_yesno },
	{ "input", tk_input },
	{0,0}
};
/* hooks */
static int hook_land( lua_State *L );
static const luaL_reg hook_methods[] = {
	{ "land", hook_land },
	{0,0}
};


/*
 * register all the libraries here
 */
int misn_loadLibs( lua_State *L )
{
	luaL_register(L, "misn", misn_methods);
	luaL_register(L, "space", space_methods);
	luaL_register(L, "player", player_methods);
	luaL_register(L, "rnd", rnd_methods);
	luaL_register(L, "tk", tk_methods);
	luaL_register(L, "hook", hook_methods);
	return 0;
}


/*
 * runs a mission function
 */
int misn_run( Mission *misn, char *func )
{
	int ret;
	char* err;

	cur_mission = misn;
	misn_delete = 0;

	lua_getglobal( misn->L, func );
	if ((ret = lua_pcall(misn->L, 0, 0, 0))) { /* error has occured */
		err = (char*) lua_tostring(misn->L,-1);
		if (strcmp(err,"Mission Finished"))
			WARN("Mission '%s' -> '%s': %s",
					cur_mission->data->name, func, err);
		else ret = 0;
	}

	/* mission is finished */
	if (misn_delete) mission_cleanup( cur_mission );
	cur_mission = NULL;

	return ret;
}



/*
 *   M I S N
 */
static int misn_setTitle( lua_State *L )
{
	MIN_ARGS(1);
	if (lua_isstring(L, -1)) {
		if (cur_mission->title) /* cleanup old title */
			free(cur_mission->title);
		cur_mission->title = strdup((char*)lua_tostring(L, -1));
	}
	return 0;
}
static int misn_setDesc( lua_State *L )
{
	MIN_ARGS(1);
	if (lua_isstring(L, -1)) {    
		if (cur_mission->desc) /* cleanup old description */
			free(cur_mission->desc);
		cur_mission->desc = strdup((char*)lua_tostring(L, -1));
	}
	return 0;
}
static int misn_setReward( lua_State *L )
{
	MIN_ARGS(1);
	if (lua_isstring(L, -1)) {    
		if (cur_mission->reward) /* cleanup old reward */
			free(cur_mission->reward);
		cur_mission->reward = strdup((char*)lua_tostring(L, -1));
	}
	return 0;
}
static int misn_factions( lua_State *L )
{
	int i;
	MissionData *dat;

	dat = cur_mission->data;

	/* we'll push all the factions in table form */
	lua_newtable(L);
	for (i=0; i<dat->avail.nfactions; i++) {
		lua_pushnumber(L,i+1); /* index, starts with 1 */
		lua_pushnumber(L,dat->avail.factions[i]); /* value */
		lua_rawset(L,-3); /* store the value in the table */
	}
	return 1;
}
static int misn_accept( lua_State *L )
{
	int i, ret;

	ret = 0;

	/* find last mission */
	for (i=0; i<MISSION_MAX; i++)
		if (player_missions[i].data == NULL) break;

	/* no missions left */
	if (i>=MISSION_MAX) ret = 1;
	else { /* copy it over */
		memcpy( &player_missions[i], cur_mission, sizeof(Mission) );
		memset( cur_mission, 0, sizeof(Mission) );
		cur_mission = &player_missions[i];
	}

	lua_pushboolean(L,!ret); /* we'll convert C style return to lua */
	return 1;
}
static int misn_finish( lua_State *L )
{
	misn_delete = 1;

	lua_pushstring(L, "Mission Finished");
	lua_error(L); /* shouldn't return */

	return 0;
}



/*
 *   S P A C E
 */
static int space_getPlanet( lua_State *L )
{
	int i;
	int *factions;
	int nfactions;
	char **planets;
	int nplanets;
	char *rndplanet;

	if (lua_gettop(L) == 0) { /* get random planet */
	}
	else if (lua_istable(L,-1)) { /* get planet of faction in table */

		/* load up the table */
		lua_pushnil(L);
		nfactions = (int) lua_gettop(L);
		factions = malloc( sizeof(int) * nfactions );
		i = 0;
		while (lua_next(L, -2) != 0) {
			factions[i++] = (int) lua_tonumber(L,-1);
			lua_pop(L,1);
		}

		/* get the planets */
		planets = space_getFactionPlanet( &nplanets, factions, nfactions );
		free(factions);

		/* choose random planet */
		if (nplanets == 0) { /* no suitable planet */
			free(planets);
			return 0;
		}
		rndplanet = planets[RNG(0,nplanets)];
		free(planets);

		lua_pushstring(L, rndplanet);
		return 1;
	}
	return 0; /* nothing good passed */
}
static int space_landName( lua_State *L )
{
	if (landed) {
		lua_pushstring(L, land_planet->name);
		return 1;
	}
	return 0;
}



/* 
 *   P L A Y E R
 */
static int player_freeSpace( lua_State *L )
{
	lua_pushnumber(L, pilot_freeCargo(player) );
	return 1;
}
static int player_addCargo( lua_State *L )
{
	Commodity *cargo;
	int quantity, ret;

	MIN_ARGS(2);

	if (lua_isstring(L,-2)) cargo = commodity_get( (char*) lua_tostring(L,-2) );
	else return 0;
	if (lua_isnumber(L,-1)) quantity = (int) lua_tonumber(L,-1);
	else return 0;

	ret = pilot_addCargo( player, cargo, quantity );

	lua_pushnumber(L, ret);
	return 1;
}
static int player_rmCargo( lua_State *L )
{
	Commodity *cargo;
	int quantity, ret;

	MIN_ARGS(2);

	if (lua_isstring(L,-2)) cargo = commodity_get( (char*) lua_tostring(L,-2) );
	else return 0;
	if (lua_isnumber(L,-1)) quantity = (int) lua_tonumber(L,-1);
	else return 0;

	ret = pilot_rmCargo( player, cargo, quantity );

	lua_pushnumber(L, ret);
	return 1;
}
static int player_pay( lua_State *L )
{
	int money;

	MIN_ARGS(1);

	if (lua_isnumber(L,-1)) money = (int) lua_tonumber(L,-1);
	else return 0;

	player->credits += money;

	return 0;
}



/*
 *   R N D
 */
static int rnd_int( lua_State *L )
{
	int o;

	o = lua_gettop(L);

	if (o==0) lua_pushnumber(L, RNGF() ); /* random double 0 <= x <= 1 */
	else if (o==1) { /* random int 0 <= x <= parameter */
		if (lua_isnumber(L, -1))
			lua_pushnumber(L, RNG(0, (int)lua_tonumber(L, -1)));
		else return 0;
	}
	else if (o>=2) { /* random int paramater 1 <= x <= parameter 2 */
		if (lua_isnumber(L, -1) && lua_isnumber(L, -2))
			lua_pushnumber(L,
					RNG((int)lua_tonumber(L, -2), (int)lua_tonumber(L, -1)));
		else return 0;
	}
	else return 0;

	return 1; /* unless it's returned 0 already it'll always return a parameter */
}



/*
 *   T O O L K I T
 */
static int tk_msg( lua_State *L )
{
	char *title, *str;
	MIN_ARGS(2);

	if (lua_isstring(L,-2)) title = (char*) lua_tostring(L,-2);
	else return 0;
	if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
	else return 0;

	dialogue_msg( title, str );
	return 0;
}
static int tk_yesno( lua_State *L )
{
	char *title, *str;
	MIN_ARGS(2);

	if (lua_isstring(L,-2)) title = (char*) lua_tostring(L,-2);
	else return 0;
	if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
	else return 0;

	dialogue_YesNo( title, str );
	return 0;
}
static int tk_input( lua_State *L )
{
	char *title, *str;
	int min, max;
	MIN_ARGS(4);

	if (lua_isstring(L,-4)) title = (char*) lua_tostring(L,-4);
	else return 0;
	if (lua_isnumber(L,-3)) min = (int) lua_tonumber(L,-3);
	else return 0;
	if (lua_isnumber(L,-2)) max = (int) lua_tonumber(L,-2);
	else return 0;
	if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
	else return 0;

	dialogue_input( title, min, max, str );
	return 0;
}



/*
 *   H O O K
 */
static int hook_land( lua_State *L )
{
	char *func;

	MIN_ARGS(1);

	if (lua_isstring(L,-1)) func = (char*)lua_tostring(L,-1);
	else {
		WARN("mission '%s': trying to push non-valid function hook",
				cur_mission->data->name);
		return 0;
	}
	hook_add( cur_mission, func, "land" );
	return 0;
}
