/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_system.c
 *
 * @brief Lua system module.
 */

#include "nlua_system.h"

#include "naev.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_planet.h"
#include "log.h"
#include "rng.h"
#include "land.h"
#include "map.h"


/* System metatable methods */
static int systemL_cur( lua_State *L );
static int systemL_get( lua_State *L );
static int systemL_eq( lua_State *L );
static int systemL_name( lua_State *L );
static int systemL_faction( lua_State *L );
static int systemL_nebula( lua_State *L );
static int systemL_jumpdistance( lua_State *L );
static int systemL_adjacent( lua_State *L );
static int systemL_hasPresence( lua_State *L );
static int systemL_planets( lua_State *L );
static int systemL_presence( lua_State *L );
static int systemL_radius( lua_State *L );
static const luaL_reg system_methods[] = {
   { "cur", systemL_cur },
   { "get", systemL_get },
   { "__eq", systemL_eq },
   { "__tostring", systemL_name },
   { "name", systemL_name },
   { "faction", systemL_faction },
   { "nebula", systemL_nebula },
   { "jumpDist", systemL_jumpdistance },
   { "adjacentSystems", systemL_adjacent },
   { "hasPresence", systemL_hasPresence },
   { "planets", systemL_planets },
   { "presence", systemL_presence },
   { "radius", systemL_radius },
   {0,0}
}; /**< System metatable methods. */


/**
 * @brief Loads the system library.
 *
 *    @param L State to load system library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadSystem( lua_State *L, int readonly )
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
   return (LuaSystem*) lua_touserdata(L,ind);
}
/**
 * @brief Gets system at index raising an error if type doesn't match.
 *
 *    @param L Lua state to get system from.
 *    @param ind Index position of system.
 *    @return The LuaSystem at ind.
 */
LuaSystem* luaL_checksystem( lua_State *L, int ind )
{     
   if (lua_issystem(L,ind))
      return lua_tosystem(L,ind);
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
 * @brief Gets the current system.
 *
 * @usage sys = system.cur() -- Gets the current system
 *
 *    @luareturn Current system.
 * @luafunc cur()
 */
static int systemL_cur( lua_State *L )
{
   LuaSystem sys;
   sys.s = cur_system;
   lua_pushsystem(L,sys);
   return 1;
}


/**
 * @brief Gets a system.
 *
 * Behaves differently depending on what you pass as param:
 *    - nil : -OBSOLETE- Gets the current system. Use system.cur() instead.
 *    - string : Gets the system by name.
 *    - planet : Gets the system by planet.
 *
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
      sys.s = system_get( lua_tostring(L,1) );
   }
   /* Passing a planet */
   else if (lua_isplanet(L,1)) {
      p = lua_toplanet(L,1);
      sys.s = system_get( planet_getSystem( p->p->name ) );
   }
   else NLUA_INVALID_PARAMETER();

   /* Error checking. */
   if(sys.s == NULL) {
      NLUA_ERROR(L, "No matching systems found.");
      return 0;
   }

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
   a = luaL_checksystem(L,1);
   b = luaL_checksystem(L,2);
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
   sys = luaL_checksystem(L,1);
   lua_pushstring(L,sys->s->name);
   return 1;
}

/**
 * @brief Gets system factions.
 *
 * @code
 * sys   = system.get() -- Get current system
 * facts = sys:faction() -- Get factions
 * if facts[ "Empire" ] then
 *    -- Do something since there is at least one Empire planet in the system
 * end
 * value = facts[ "Pirate" ] or 0 -- Get value of pirates in the system
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
   sys = luaL_checksystem(L,1);

   /* Return result in table */
   lua_newtable(L);
   for (i=0; i<sys->s->npresence; i++) {
      lua_pushstring( L, faction_name(sys->s->presence[i].faction) ); /* t, k */
      lua_pushnumber(L,sys->s->presence[i].value); /* t, k, v */
      lua_settable(L,-3);  /* t */
      /* allows syntax foo = space.faction("foo"); if foo["bar"] then ... end */
   }
   return 1;

}


/**
 * @brief Gets the system's nebula parameters.
 *
 * @usage density, volatility = sys:nebula()
 *
 *    @luaparam s System to get nebula parameters from.
 *    @luareturn The density and volatility of the system.
 * @luafunc nebula( s )
 */
static int systemL_nebula( lua_State *L )
{
   LuaSystem *sys;
   sys = luaL_checksystem(L,1);

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
   LuaSystem *sys, *sysp;
   StarSystem **s;
   int jumps;
   const char *start, *goal;

   sys = luaL_checksystem(L,1);
   start = sys->s->name;

   if (lua_gettop(L) > 1) {
      if (lua_isstring(L,2))
         goal = lua_tostring(L,2);
      else if (lua_issystem(L,2)) {
         sysp = lua_tosystem(L,2);
         goal = sysp->s->name;
      }
      else NLUA_INVALID_PARAMETER();
   }
   else
      goal = cur_system->name;

   s = map_getJumpPath( &jumps, start, goal, 1, NULL );
   free(s);

   lua_pushnumber(L,jumps);
   return 1;
}


/**
 * @brief Gets all the adjacent systems to a system and the jump point positions.
 *
 * @usage for k,v in pairs( sys:edjacentSystems() ) do -- Iterate over adjacent systems.
 * @usage v = sys:adjacentSystems()[ system.get("Gamma Polaris") ] -- Get the position of the jump to Gamma Polaris.
 *
 *    @luaparam s System to get adjacent systems of, keys are systems, values are jump position.
 *    @luareturn A table with all the adjacent systems.
 * @luafunc adjacentSystems( s )
 */
static int systemL_adjacent( lua_State *L )
{
   int i;
   LuaSystem *sys, sysp;
   LuaVector lv;

   sys = luaL_checksystem(L,1);

   /* Push all adjacent systems. */
   lua_newtable(L);
   for (i=0; i<sys->s->njumps; i++) {
      sysp.s = sys->s->jumps[i].target;
      lua_pushsystem(L,sysp); /* key. */
      vectcpy( &lv.vec, &sys->s->jumps[i].pos );
      lua_pushvector(L,lv); /* value. */
      lua_rawset(L,-3);
   }

   return 1;
}


/**
 * @brief Checks to see if a faction has presence in a system.
 *
 * This checks to see if the faction has a possibility of having any ships at all
 *  be randomly generated in the system.
 *
 *  @usage if sys:hasPresence( "Empire" ) then -- Checks to see if Empire has ships in the system
 *  @usage if sys:hasPresence( faction.get("Pirate") ) then -- Checks to see if the Pirate has ships in the system
 *
 *    @luaparam s System to check to see if has presence of a certain faction.
 *    @luaparam f Faction or name of faction to check to see if has presence in the system.
 *    @luareturn true If faction has presence in the system, false otherwise.
 * @luafunc hasPresence( s, f )
 */
static int systemL_hasPresence( lua_State *L )
{
   LuaSystem *sys;
   LuaFaction *lf;
   int fct;
   int i, found;

   sys = luaL_checksystem(L,1);

   /* Get the second parameter. */
   if (lua_isstring(L,2)) {
      fct = faction_get( lua_tostring(L,2) );
   }
   else if (lua_isfaction(L,2)) {
      lf = lua_tofaction(L,2);
      fct = lf->f;
   }
   else NLUA_INVALID_PARAMETER();

   /* Try to find a fleet of the faction. */
   found = 0;
   for (i=0; i<sys->s->npresence; i++) {
      if (sys->s->presence[i].faction == fct) {
         found = 1;
         break;
      }
   }

   lua_pushboolean(L, found);
   return 1;
}


/**
 * @brief Gets the planets in a system.
 *
 * @usage for k,v in pairs( sys:planets() ) do -- Iterate over planets in system
 * @usage if #sys:planets() > 0then -- System has planets
 *
 *    @luaparam s System to get planets of
 *    @luareturn A table with all the planets
 * @luafunc planets( s )
 */
static int systemL_planets( lua_State *L )
{
   int i, key;
   LuaSystem *sys;
   LuaPlanet p;

   sys = luaL_checksystem(L,1);

   /* Push all planets. */
   lua_newtable(L);
   key = 0;
   for (i=0; i<sys->s->nplanets; i++) {
      p.p = sys->s->planets[i];
      if(p.p->real == ASSET_REAL) {
         key++;
         lua_pushnumber(L,key); /* key */
         lua_pushplanet(L,p); /* value */
         lua_rawset(L,-3);
      }
   }

   return 1;
}


/**
 * @brief Gets the presence in the system.
 *
 * Possible parameters are besides a faction:<br/>
 *  - "all": Gets the sum of all the presences.<br />
 *  - "friendly": Gets the sum of all the friendly presences.<br />
 *  - "hostile": Gets the sum of all the hostile presences.<br />
 *  - "neutral": Gets the sum of all the neutral presences.<br />
 *
 * @usage p = sys:presence( f ) -- Gets the presence of a faction f
 * @usage p = sys:presence( "all" ) -- Gets the sum of all the presences
 * @usage if sys:presence("friendly") > sys:presence("hostile") then -- Checks to see if the system is dominantly friendly
 *
 *    @luaparam s System to get presence level of.
 *    @luareturn The presence level in sys (absolute value).
 * @luafunc presence( s )
 */
static int systemL_presence( lua_State *L )
{
   LuaSystem *sys;
   LuaFaction *lf;
   int *fct;
   int nfct;
   double presence;
   int i;
   const char *cmd;

   /* Get parameters. */
   sys = luaL_checksystem(L, 1);

   /* Get the second parameter. */
   if (lua_isstring(L, 2)) {
      /* A string command has been given. */
      cmd  = lua_tostring(L, 2);
      nfct = 0;

      /* Check the command string and get the appropriate faction group.*/
      if(strcmp(cmd, "all") == 0)
         fct = faction_getGroup(&nfct, 0);
      else if(strcmp(cmd, "friendly") == 0)
         fct = faction_getGroup(&nfct, 1);
      else if(strcmp(cmd, "hostile") == 0)
         fct = faction_getGroup(&nfct, 3);
      else if(strcmp(cmd, "neutral") == 0)
         fct = faction_getGroup(&nfct, 2);
      else /* Invalid command string. */
         NLUA_INVALID_PARAMETER();
   }
   else if (lua_isfaction(L, 2)) {
      /* A faction id was given. */
      lf     = lua_tofaction(L, 2);
      nfct   = 1;
      fct    = malloc(sizeof(int) * nfct);
      fct[0] = lf->f;
   }
   else NLUA_INVALID_PARAMETER();

   /* Add up the presence values. */
   presence = 0;
   for(i=0; i<nfct; i++)
      presence += system_getPresence(sys->s, fct[i]);

   /* Clean up after ourselves. */
   free(fct);

   /* Push it back to Lua. */
   lua_pushnumber(L, presence);
   return 1;
}


/**
 * @brief Gets the radius of the system.
 *
 * This is the radius of the circle which all the default jumps will be on.
 *
 * @usage r = s:radius()
 *
 *    @luaparam s System to get the radius of.
 *    @luareturn The radius of the system.
 * @luafunc radius( s )
 */
static int systemL_radius( lua_State *L )
{
   LuaSystem *sys;

   /* Get parameters. */
   sys = luaL_checksystem(L, 1);

   lua_pushnumber( L, sys->s->radius );
   return 1;
}

