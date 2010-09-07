/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_time.c
 *
 * @brief Time manipulation Lua bindings.
 */

#include "nlua_time.h"

#include "naev.h"

#include <stdlib.h>

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "ntime.h"


/* Time methods. */
static int time_get( lua_State *L );
static int time_str( lua_State *L );
static int time_units( lua_State *L );
static int time_inc( lua_State *L );
static const luaL_reg time_methods[] = {
   { "get", time_get },
   { "str", time_str },
   { "units", time_units },
   { "inc", time_inc },
   {0,0}
}; /**< Time Lua methods. */


/**
 * @brief Loads the Time Lua library.
 *
 *    @param L Lua state.
 *    @param readonly Whether to open it as read only.
 *    @return 0 on success.
 */
int nlua_loadTime( lua_State *L, int readonly )
{
	(void)readonly;
	luaL_register(L, "time", time_methods);
	return 0;
}


/**
 * @brief Bindings for interacting with the time.
 *
 * Usage is generally something as follows:
 * @code
 * time_limit = time.get() + time.units(15)
 * player.msg( string.format("You only have %s left!", time.str(time.get() - time_limit)) )
 *
 * -- Do stuff here
 *
 * if time.get() > time_limit then
 *    -- Limit is up
 * end
 * @endcode
 *
 * @luamod time
 */
/**
 * @brief Gets the current time in internal representation time.
 *
 * @usage t = time.get()
 *
 *    @luareturn Time in internal representation time.
 * @luafunc get()
 */
static int time_get( lua_State *L )
{
	lua_pushnumber( L, ntime_get() );
	return 1;
}
/**
 * @brief Converts the time to a pretty human readable format.
 *
 * @usage strt = time.str()
 * @usage strt = time.str( time.get() + time.units(5) )
 *
 *    @luaparam t Time to convert to pretty format.  If ommitted, current time is
 *              used.
 *    @luareturn The time in human readable format.
 * @luafunc str( t )
 */
static int time_str( lua_State *L )
{
	char *nt;
	if ((lua_gettop(L) > 0) && (lua_isnumber(L,1)))
		nt = ntime_pretty( (unsigned int) lua_tonumber(L,1) );
	else
		nt = ntime_pretty( ntime_get() );
	lua_pushstring(L, nt);
	free(nt);
	return 1;
}

/**
 * @brief Converts stu to internal representation time.
 *
 * @usage time_limit = time.get() + time.units(5)
 *
 *    @luaparam stu Time in stu to convert to internal representation time.  If
 *                ommitted, 1 stu is used.
 *    @luareturn The value of stu in internal representation time.
 * @luafunc units( stu )
 */
static int time_units( lua_State *L )
{
	if ((lua_gettop(L) > 0) && (lua_isnumber(L,1)))
		lua_pushnumber( L, (unsigned int)lua_tonumber(L,1) * NTIME_UNIT_LENGTH );
	else
		lua_pushnumber( L, NTIME_UNIT_LENGTH );
	return 1;
}


/**
 * @brief Increases or decreases the time.
 *
 * @usage time.inc( time.units(100) ) -- Increments the time by 100 STU.
 *
 *    @luaparam t Amount to increment or decrement the time by.
 * @luafunc inc( t )
 */
static int time_inc( lua_State *L )
{
   if (lua_isnumber(L,1))
      ntime_inc( lua_tonumber(L, 1));
   return 0;
}
