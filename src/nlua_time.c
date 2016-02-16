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

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "ntime.h"


/* Time methods. */
static int time_create( lua_State *L );
static int time_add( lua_State *L );
static int time_add__( lua_State *L );
static int time_sub( lua_State *L );
static int time_sub__( lua_State *L );
static int time_eq( lua_State *L );
static int time_lt( lua_State *L );
static int time_le( lua_State *L );
static int time_get( lua_State *L );
static int time_str( lua_State *L );
static int time_inc( lua_State *L );
static int time_tonumber( lua_State *L );
static int time_fromnumber( lua_State *L );
static const luaL_reg time_methods[] = {
   { "create", time_create },
   { "add", time_add__ },
   { "__add", time_add },
   { "sub", time_sub__ },
   { "__sub", time_sub },
   { "__eq", time_eq },
   { "__lt", time_lt },
   { "__le", time_le },
   { "get", time_get },
   { "str", time_str },
   { "__tostring", time_str },
   { "inc", time_inc },
   { "tonumber", time_tonumber },
   { "fromnumber", time_fromnumber },
   {0,0}
}; /**< Time Lua methods. */
static const luaL_reg time_cond_methods[] = {
   { "create", time_create },
   { "add", time_add__ },
   { "__add", time_add },
   { "sub", time_sub__ },
   { "__sub", time_sub },
   { "__eq", time_eq },
   { "__lt", time_lt },
   { "__le", time_le },
   { "get", time_get },
   { "str", time_str },
   { "__tostring", time_str },
   { "tonumber", time_tonumber },
   { "fromnumber", time_fromnumber },
   {0,0}
}; /**< Time Lua conditional methods. */


/**
 * @brief Loads the Time Lua library.
 *
 *    @param L Lua state.
 *    @param readonly Whether to open it as read only.
 *    @return 0 on success.
 */
int nlua_loadTime( lua_State *L, int readonly )
{
   (void) readonly;
   /* Create the metatable */
   luaL_newmetatable(L, TIME_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, time_cond_methods);
   else
      luaL_register(L, NULL, time_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, TIME_METATABLE);

   return 0; /* No error */
}


/**
 * @brief Bindings for interacting with the time.
 *
 * Usage is generally something as follows:
 * @code
 * time_limit = time.get() + time.create( 0, 5, 0 )
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
 * @brief Gets time at index.
 *
 *    @param L Lua state to get time from.
 *    @param ind Index position to find the time.
 *    @return Time found at the index in the state.
 */
ntime_t* lua_totime( lua_State *L, int ind )
{
   return (ntime_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets time at index raising an error if isn't a time.
 *
 *    @param L Lua state to get time from.
 *    @param ind Index position to find the time.
 *    @return Time found at the index in the state.
 */
ntime_t* luaL_checktime( lua_State *L, int ind )
{
   if (lua_istime(L,ind))
      return lua_totime(L,ind);
   luaL_typerror(L, ind, TIME_METATABLE);
   return NULL;
}
/**
 * @brief Gets a time directly.
 *
 *    @param L Lua state to get time from.
 *    @param ind Index position to find the time.
 *    @return Time found at the index in the state.
 */
ntime_t luaL_validtime( lua_State *L, int ind )
{
   return *luaL_checktime( L, ind );
}
/**
 * @brief Pushes a time on the stack.
 *
 *    @param L Lua state to push time into.
 *    @param time Time to push.
 *    @return Newly pushed time.
 */
ntime_t* lua_pushtime( lua_State *L, ntime_t time )
{
   ntime_t *p;
   p = (ntime_t*) lua_newuserdata(L, sizeof(ntime_t));
   *p = time;
   luaL_getmetatable(L, TIME_METATABLE);
   lua_setmetatable(L, -2);
   return p;
}
/**
 * @brief Checks to see if ind is a time.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a time.
 */
int lua_istime( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, TIME_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Creates a time. This can be absolute or relative.
 *
 * @usage t = time.create( 591, 3271, 12801 ) -- Gets a time near when the incident happened.
 *
 *    @luaparam scu SCU for the new time.
 *    @luaparam stp STP for the new time.
 *    @luaparam stu STU for the new time.
 *    @luareturn A newly created time metatable.
 * @luafunc create( scu, stp, stu )
 */
static int time_create( lua_State *L )
{
   int scu, stp, stu;

   /* Parameters. */
   scu = luaL_checkint(L,1);
   stp = luaL_checkint(L,2);
   stu = luaL_checkint(L,3);

   /* Create the time. */
   lua_pushtime( L, ntime_create( scu, stp, stu ) );
   return 1;
}
/**
 * @brief Adds two time metatables.
 *
 * Overrides the addition operator.
 *
 * @usage new_time = time.get() + time.create( 0, 5, 0 ) -- Adds 5 STP to the current date
 *
 *    @luaparam t1 Time metatable to add to.
 *    @luaparam t2 Time metatable added.
 * @luafunc add( t1, t2)
 */
static int time_add( lua_State *L )
{
   ntime_t t1, t2;

   /* Parameters. */
   t1 = luaL_validtime( L, 1 );
   t2 = luaL_validtime( L, 2 );

   /* Add them. */
   lua_pushtime( L, t1 + t2 );
   return 1;
}


/*
 * Method version of time_add that modifies the first time.
 */
static int time_add__( lua_State *L )
{
   ntime_t *t1, t2;

   /* Parameters. */
   t1 = luaL_checktime( L, 1 );
   t2 = luaL_validtime( L, 2 );

   /* Add them. */
   *t1 += t2;
   lua_pushtime( L, *t1 );
   return 1;
}


/**
 * @brief Subtracts two time metatables.
 *
 * Overrides the subtraction operator.
 *
 * @usage new_time = time.get() - time.create( 0, 3, 0 ) -- Subtracts 3 STP to the current date
 *
 *    @luaparam t1 Time metatable to subtract from.
 *    @luaparam t2 Time metatable subtracted.
 * @luafunc sub( t1, t2)
 */
static int time_sub( lua_State *L )
{
   ntime_t t1, t2;

   /* Parameters. */
   t1 = luaL_validtime( L, 1 );
   t2 = luaL_validtime( L, 2 );

   /* Sub them. */
   lua_pushtime( L, t1 - t2 );
   return 1;
}


/*
 * Method version of time_sub that modifies the first time.
 */
static int time_sub__( lua_State *L )
{
   ntime_t *t1, t2;

   /* Parameters. */
   t1 = luaL_checktime( L, 1 );
   t2 = luaL_validtime( L, 2 );

   /* Sub them. */
   *t1 -= t2;
   lua_pushtime( L, *t1 );
   return 1;
}


/**
 * @brief Checks to see if two time are equal.
 *
 * It is recommended to check with < and <= instead of ==.
 *
 * @usage if time.create( 630, 5, 78) == time.get() then -- do something if they match
 *
 *    @luaparam t1 Time to compare for equality.
 *    @luaparam t2 Time to compare for equality.
 *    @luareturn true if they're equal.
 * @luafunc __eq( t1, t2 )
 */
static int time_eq( lua_State *L )
{
   ntime_t t1, t2;
   t1 = luaL_validtime( L, 1 );
   t2 = luaL_validtime( L, 2 );
   lua_pushboolean( L, t1==t2 );
   return 1;
}
/**
 * @brief Checks to see if a time is strictly larger than another.
 *
 * @usage if time.create( 630, 5, 78) < time.get() then -- do something if time is past UST 630:0005.78
 *
 *    @luaparam t1 Time to see if is is smaller than t2.
 *    @luaparam t2 Time see if is larger than t1.
 *    @luareturn true if t1 < t2
 * @luafunc __lt( t1, t2 )
 */
static int time_lt( lua_State *L )
{
   ntime_t t1, t2;
   t1 = luaL_validtime( L, 1 );
   t2 = luaL_validtime( L, 2 );
   lua_pushboolean( L, t1<t2 );
   return 1;
}
/**
 * @brief Checks to see if a time is larger or equal to another.
 *
 * @usage if time.create( 630, 5, 78) <= time.get() then -- do something if time is past UST 630:0005.78
 *
 *    @luaparam t1 Time to see if is is smaller or equal to than t2.
 *    @luaparam t2 Time see if is larger or equal to than t1.
 *    @luareturn true if t1 <= t2
 * @luafunc __le( t1, t2 )
 */
static int time_le( lua_State *L )
{
   ntime_t t1, t2;
   t1 = luaL_validtime( L, 1 );
   t2 = luaL_validtime( L, 2 );
   lua_pushboolean( L, t1<=t2 );
   return 1;
}
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
   lua_pushtime( L, ntime_get() );
   return 1;
}
/**
 * @brief Converts the time to a pretty human readable format.
 *
 * @usage strt = time.str() -- Gets current time
 * @uasge strt = time.str( nil, 5 ) -- Gets current time with full decimals
 * @usage strt = time.str( time.get() + time.create(0,5,0) ) -- Gets time in 5 STP
 * @usage strt = t:str() -- Gets the string of t
 *
 *    @luaparam t Time to convert to pretty format.  If omitted, current time is used.
 *    @luaparam d Decimals to use for displaying STU (should be between 0 and 5).
 *    @luareturn The time in human readable format.
 * @luafunc str( t, d )
 */
static int time_str( lua_State *L )
{
   int top;
   ntime_t t;
   char nt[64];
   int d;

   /* Defaults. */
   d = 2;

   /* Parse parameters. */
   top = lua_gettop(L);
   if ((top > 0) && !lua_isnil(L,1))
      t = luaL_validtime(L,1);
   else
      t = ntime_get();
   if (top > 1)
      d = luaL_checkint(L,2);

   /* Push string. */
   ntime_prettyBuf( nt, sizeof(nt), t, d );
   lua_pushstring(L, nt);
   return 1;
}


/**
 * @brief Increases or decreases the time.
 *
 * @usage time.inc( time.create(0,0,100) ) -- Increments the time by 100 STU.
 *
 *    @luaparam t Amount to increment or decrement the time by.
 * @luafunc inc( t )
 */
static int time_inc( lua_State *L )
{
   ntime_inc( luaL_validtime(L,1) );
   return 0;
}


/**
 * @brief Gets a number representing this time.
 *
 * The best usage for this currently is mission variables.
 *
 * @usage num = t:tonumber() -- Getting the number from a time t
 *
 *    @luaparam t Time to get number of.
 *    @luareturn Number representing time.
 * @luafunc tonumber( t )
 */
static int time_tonumber( lua_State *L )
{
   ntime_t t = luaL_validtime(L,1);
   lua_pushnumber( L, t );
   return 1;
}


/**
 * @brief Creates a time from a number representing it.
 *
 * The best usage for this currently is mission variables.
 *
 * @usage t = time.fromnumber( t:tonumber() ) -- Should get the time t again
 *
 *    @luaparam num Number to get time from.
 *    @luareturn Time representing number.
 * @luafunc fromnumber( num )
 */
static int time_fromnumber( lua_State *L )
{
   ntime_t t;
   t = (ntime_t) luaL_checknumber(L,1);
   lua_pushtime( L, t );
   return 1;
}



