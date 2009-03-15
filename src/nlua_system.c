/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_system.c
 *
 * @brief Lua system module.
 */

#include "nlua_system.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_planet.h"
#include "log.h"
#include "naev.h"
#include "rng.h"
#include "land.h"
#include "map.h"


/* System metatable methods */
static int systemL_get( lua_State *L );
static int systemL_eq( lua_State *L );
static int systemL_name( lua_State *L );
static int systemL_faction( lua_State *L );
static int systemL_nebulae( lua_State *L );
static int systemL_jumpdistance( lua_State *L );
static int systemL_adjacent( lua_State *L );
static const luaL_reg system_methods[] = {
   { "get", systemL_get },
   { "__eq", systemL_eq },
   { "__tostring", systemL_name },
   { "name", systemL_name },
   { "faction", systemL_faction },
   { "nebulae", systemL_nebulae },
   { "jumpDist", systemL_jumpdistance },
   { "adjacentSystems", systemL_adjacent },
   {0,0}
}; /**< System metatable methods. */


/**
 * @brief Loads the system library.
 *
 *    @param L State to load system library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int lua_loadSystem( lua_State *L, int readonly )
{
   (void)readonly; /* only read only atm */

   /* Create the metatable */
   luaL_newmetatable(L, SYSTEM_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, system_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, SYSTEM_METATABLE);

   return 0; /* No error */
}


/**
 * @brief Lua system module.
 *
 * This module allows you to use the Star Systems from Lua.
 *
 * Typical example would be something like:
 * @code
 * cur = system.get() -- Gets current system
 * sys = system.get( "Gamma Polaris" )
 * @endcode
 *
 * @luamod system
 */
/**
 * @brief Gets system at index.
 *
 *    @param L Lua state to get system from.
 *    @param ind Index position of system.
 *    @return The LuaSystem at ind.
 */
LuaSystem* lua_tosystem( lua_State *L, int ind )
{     
   if (lua_isuserdata(L,ind)) {
      return (LuaSystem*) lua_touserdata(L,ind);
   }
   luaL_typerror(L, ind, SYSTEM_METATABLE);
   return NULL;
}

/**
 * @brief Pushes a system on the stack.
 *
 *    @param L Lua state to push system onto.
 *    @param sys System to push.
 *    @return System just pushed.
 */
LuaSystem* lua_pushsystem( lua_State *L, LuaSystem sys )
{
   LuaSystem *s;
   s = (LuaSystem*) lua_newuserdata(L, sizeof(LuaSystem));
   *s = sys;
   luaL_getmetatable(L, SYSTEM_METATABLE);
   lua_setmetatable(L, -2);
   return s;
}

/**
 * @brief Checks to see if ind is a system.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if there is a system at index position.
 */
int lua_issystem( lua_State *L, int ind )
{  
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, SYSTEM_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */ 
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Gets a system.
 *
 * Behaves differently depending on what you pass as param:
 *    - nil : Gets the current system.
 *    - string : Gets the system by name.
 *    - planet : Gets the system by planet.
 *
 * @usage sys = system.get() -- Gets the current system.
 * @usage sys = system.get( p ) -- Gets system where planet 'p' is located.
 * @usage sys = system.get( "Gamma Polaris" ) -- Gets the system by name.
 *
 *    @luaparam param Read description for details.
 *    @luareturn System metatable matching param.
 * @luafunc get( param )
 */
static int systemL_get( lua_State *L )
{
   LuaSystem sys;
   LuaPlanet *p;

   /* Get current system with no parameters */
   if (lua_gettop(L) == 0) {
      sys.s = cur_system;
   }
   /* Passing a string (systemname) */
   else if (lua_isstring(L,1)) {
      sys.s = system_get( (char*) lua_tostring(L,1) );
   }
   /* Passing a planet */
   else if (lua_isplanet(L,1)) {
      p = lua_toplanet(L,1);
      sys.s = system_get( planet_getSystem( p->p->name ) );
   }
   else NLUA_INVALID_PARAMETER();

   /* return the system */
   lua_pushsystem(L,sys);
   return 1;
}

/**
 * @brief Check systems for equality.
 *
 * Allows you to use the '=' operator in Lua with systems.
 *
 * @usage if sys == system.get( "Draygar" ) then -- Do something
 *
 *    @luaparam s System comparing.
 *    @luaparam comp System to compare against.
 *    @luareturn true if both systems are the same.
 * @luafunc __eq( s, comp )
 */
static int systemL_eq( lua_State *L )
{
   LuaSystem *a, *b;
   a = lua_tosystem(L,1);
   b = lua_tosystem(L,2);
   if (a->s == b->s)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}

/**
 * @brief Returns the system's name.
 *
 * @usage name = sys:name()
 *
 *    @luaparam s System to get name of.
 *    @luareturn The name of the system.
 * @luafunc name( s )
 */
static int systemL_name( lua_State *L )
{
   LuaSystem *sys;
   sys = lua_tosystem(L,1);
   lua_pushstring(L,sys->s->name);
   return 1;
}

/**
 * @brief Gets system factions.
 *
 * @code
 * sys = system.get() -- Get current system
 * facts = sys:faction() -- Get factions
 * if facts["Empire"] then
 *    -- Do something since there is at least one Empire planet in the system
 * end
 * @endcode
 *
 *    @luaparam s System to get the factions of.
 *    @luareturn A table containing all the factions in the system.
 * @luafunc faction( s )
 */
static int systemL_faction( lua_State *L )
{
   int i;
   LuaSystem *sys;
   sys = lua_tosystem(L,1);

   /* Return result in table */
   lua_newtable(L);
   for (i=0; i<sys->s->nplanets; i++) {
      if (sys->s->planets[i]->faction > 0) { /* Faction must be valid */
         lua_pushboolean(L,1); /* value */
         lua_setfield(L,-2,faction_name(sys->s->planets[i]->faction)); /* key */
         /* allows syntax foo = space.faction("foo"); if foo["bar"] then ... end */
      }
   }
   return 1;

}


/**
 * @brief Gets the system's nebulae parameters.
 *
 * @usage density, volatility = sys:nebulae()
 *
 *    @luaparam s System to get nebulae parameters from.
 *    @luareturn The density and volatility of the system.
 * @luafunc nebulae( s )
 */
static int systemL_nebulae( lua_State *L )
{
   LuaSystem *sys;
   sys = lua_tosystem(L,1);

   /* Push the density and volatility. */
   lua_pushnumber(L, sys->s->nebu_density);
   lua_pushnumber(L, sys->s->nebu_volatility);

   return 2;
}


/**
 * @brief Gets jump distance from current system, or to another.
 *
 * Does different things depending on the parameter type:
 *    - nil : Gets distance from current system.
 *    - string : Gets distance from system matching name.
 *    - system : Gets distance from system
 *
 * @usage d = sys:jumpDist() -- Distance from current system.
 * @usage d = sys:jumpDist( "Draygar" ) -- Distance from system Draygar.
 * @usage d = sys:jumpDist( another_sys ) -- Distance from system another_sys.
 *
 *    @luaparam param See description.
 *    @luareturn Number of jumps to system.
 * @luafunc jumpDist( param )
 */
static int systemL_jumpdistance( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaSystem *sys, *sysp;
   StarSystem **s;
   int jumps;
   char *start, *goal;

   sys = lua_tosystem(L,1);
   start = sys->s->name;

   if (lua_gettop(L) > 1) {
      if (lua_isstring(L,2))
         goal = (char*) lua_tostring(L,2);
      else if (lua_issystem(L,2)) {
         sysp = lua_tosystem(L,2);
         goal = sysp->s->name;
      }
   }
   else
      goal = cur_system->name;

   s = map_getJumpPath( &jumps, start, goal, 1 );
   free(s);

   lua_pushnumber(L,jumps);
   return 1;
}


/**
 * @brief Gets all the ajacent systems to a system.
 *
 * @usage for k,v in pairs( sys:adjacentSystems() ) do -- Iterate over adjacent systems.
 *
 *    @luaparam s System to get adjacent systems of.
 *    @luareturn A table with all the adjacent systems.
 * @luafunc adjacentSystems( s )
 */
static int systemL_adjacent( lua_State *L )
{
   int i;
   LuaSystem *sys, sysp;

   sys = lua_tosystem(L,1);

   /* Push all adjacent systems. */
   lua_newtable(L);
   for (i=0; i<sys->s->njumps; i++) {
      sysp.s = system_getIndex( sys->s->jumps[i] );
      lua_pushnumber(L,i+1); /* key */
      lua_pushsystem(L,sysp); /* value */
      lua_rawset(L,-3);
   }

   return 1;
}

