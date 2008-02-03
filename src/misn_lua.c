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


#define MIN_ARGS(n)		if (lua_gettop(L) < n) return 0


/*
 * current mission
 */
static Mission *cur_mission = NULL;

/*
 * libraries
 */
/* misn */
static int misn_setTitle( lua_State *L );
static int misn_setDesc( lua_State *L );
static int misn_setReward( lua_State *L );
static const luaL_reg misn_methods[] = {
	{ "setTitle", misn_setTitle },
	{ "setDesc", misn_setDesc },
	{ "setReward", misn_setReward },
	{0,0}
};
/* space */
static int space_getPlanet( lua_State *L );
static const luaL_reg space_methods[] = {
	{ "getPlanet", space_getPlanet },
	{0,0}
};
/* rnd */
static int rnd_int( lua_State *L );
static const luaL_reg rnd_methods[] = {
	{ "int", rnd_int },
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
	luaL_register(L, "rnd", rnd_methods);
	luaL_register(L, "hook", hook_methods);
	return 0;
}


/*
 * runs a mission function
 */
int misn_run( Mission *misn, char *func )
{
	int ret;

	cur_mission = misn;

	lua_getglobal( misn->L, func );
	if ((ret = lua_pcall(misn->L, 0, 0, 0))) /* error has occured */
		WARN("Mission '%s' -> '%s': %s",
				cur_mission->data->name, func, lua_tostring(misn->L,-1));

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
		cur_mission->title = strdup(lua_tostring(L, -1));
	}
	return 0;
}
static int misn_setDesc( lua_State *L )
{
	MIN_ARGS(1);
	if (lua_isstring(L, -1)) {    
		if (cur_mission->title) /* cleanup old title */
			free(cur_mission->title);
		cur_mission->desc = strdup(lua_tostring(L, -1));
	}
	return 0;
}
static int misn_setReward( lua_State *L )
{
	MIN_ARGS(1);
	if (lua_isstring(L, -1)) {    
		if (cur_mission->title) /* cleanup old title */
			free(cur_mission->title);
		cur_mission->reward = strdup(lua_tostring(L, -1));
	}
	return 0;
}



/*
 *   S P A C E
 */
static int space_getPlanet( lua_State *L )
{
	(void)L;
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
 * H O O K
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
	hook_add( L, cur_mission->id, func, "land" );
	return 0;
}
