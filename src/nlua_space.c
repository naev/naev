/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_space.c
 *
 * @brief Handles the Lua space bindings.
 *
 * These bindings control the planets and systems.
 */

#include "nlua_space.h"

#include "lauxlib.h"

#include "log.h"
#include "naev.h"
#include "rng.h"
#include "land.h"
#include "nluadef.h"
#include "map.h"


/*
 * Prototypes
 */
static int planetL_createmetatable( lua_State *L );
static int systemL_createmetatable( lua_State *L );

/* space */
static int planetL_get( lua_State *L );
static int systemL_get( lua_State *L );
static const luaL_reg space_methods[] = {
   { "getPlanet", planetL_get },
   { "getSystem", systemL_get },
   {0,0}
}; /**< Space Lua methods. */

/* Planet metatable methods */
static int planetL_eq( lua_State *L );
static int planetL_name( lua_State *L );
static int planetL_faction( lua_State *L );
static int planetL_class( lua_State *L );
static int planetL_services( lua_State *L );
static const luaL_reg planet_methods[] = {
   { "__eq", planetL_eq },
   { "name", planetL_name },
   { "faction", planetL_faction },
   { "class", planetL_class },
   { "services", planetL_services },
   {0,0}
}; /**< Planet metatable methods. */

/* System metatable methods */
static int systemL_eq( lua_State *L );
static int systemL_name( lua_State *L );
static int systemL_faction( lua_State *L );
static int systemL_jumpdistance( lua_State *L );
static const luaL_reg system_methods[] = {
   { "__eq", systemL_eq },
   { "name", systemL_name },
   { "faction", systemL_faction },
   { "jumpDist", systemL_jumpdistance },
   {0,0}
}; /**< System metatable methods. */



/**
 * @fn int lua_loadSpace( lua_State *L, int readonly )
 *
 * @brief Loads the space library.
 *
 *    @param L State to load space library into.
 *    @return 0 on success.
 */
int lua_loadSpace( lua_State *L, int readonly )
{
   (void)readonly;

   /* Register the functions. */
   luaL_register(L, "space", space_methods);

   /* Register the metatables. */
   planetL_createmetatable( L );
   systemL_createmetatable( L );

   return 0;
}


/**
 * @fn static int planetL_createmetatable( lua_State *L )
 *
 * @brief Registers the planet metatable.
 *
 *    @param L Lua state to register metatable in.
 *    @return 0 on success.
 */
static int planetL_createmetatable( lua_State *L )
{
   /* Create the metatable */
   luaL_newmetatable(L, PLANET_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, planet_methods);

   return 0; /* No error */
}
/**
 * @fn static int systemL_createmetatable( lua_State *L )
 *
 * @brief Registers the system metatable.
 *
 *    @param L Lua state to register metatable in.
 *    @return 0 on success.
 */
static int systemL_createmetatable( lua_State *L )
{
   /* Create the metatable */
   luaL_newmetatable(L, SYSTEM_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, system_methods);

   return 0; /* No error */
}



/**
 * @defgroup SPACE Space Lua Bindings
 *
 * @brief Contains Lua bindings for manipulating the space itself.
 */
/**
 * @defgroup META_PLANET Planet Metatable
 *
 * @brief The planet metatable is a way to represent a planet in Lua.
 *
 * It allows all sorts of operators making it much more natural to use.
 *
 * To call members of the metatable always use:
 * @code 
 * planet:function( param )
 * @endcode
 */
/**
 * @fn LuaPlanet* lua_toplanet( lua_State *L, int ind )
 *
 * @brief Gets planet at index.
 *
 *    @param L Lua state to get planet from.
 *    @param ind Index position to find the planet.
 *    @return Planet found at the index in the state.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind )
{
   if (lua_isuserdata(L,ind)) {
      return (LuaPlanet*) lua_touserdata(L,ind);
   }
   luaL_typerror(L, ind, PLANET_METATABLE);
   return NULL;
}
/**
 * @fn LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet )
 *
 * @brief Pushes a planet on the stack.
 *
 *    @param L Lua state to push planet into.
 *    @param planet Planet to push.
 *    @return Newly pushed planet.
 */
LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet )
{
   LuaPlanet *p;
   p = (LuaPlanet*) lua_newuserdata(L, sizeof(LuaPlanet));
   *p = planet;
   luaL_getmetatable(L, PLANET_METATABLE);
   lua_setmetatable(L, -2);
   return p;
}
/**
 * @fn int lua_isplanet( lua_State *L, int ind )
 *
 * @brief Checks to see if ind is a planet.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a planet.
 */
int lua_isplanet( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, PLANET_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */ 
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */ 
   return ret;
}

/**
 * @fn static int planetL_get( lua_State *L )
 * @ingroup SPACE
 *
 * @brief planet, system getPlanet( [param] )
 *
 * Gets a planet.
 *
 * Possible values of param:
 *    - nil : Gets the current landed planet or nil if there is none.
 *    - number : Gets random planet belonging to faction matching the number.
 *    - string : Gets the planet by name.
 *    - table : Gets random planet belonging to any of the factions in the
 *               table.
 *
 *    @param param See description.
 *    @return Returns the planet and the system it belongs to.
 */
static int planetL_get( lua_State *L )
{
   int i;
   int *factions;
   int nfactions;
   char **planets;
   int nplanets;
   char *rndplanet;
   LuaPlanet planet;
   LuaSystem sys;

   rndplanet = NULL;
   nplanets = 0;
  
   /* Get the landed planet */
   if (lua_gettop(L) == 0) {
      if (land_planet != NULL) {
         planet.p = land_planet;
         lua_pushplanet(L,planet);
         sys.s = system_get( planet_getSystem(land_planet->name) );
         lua_pushsystem(L,sys);
         return 2;
      }
      return 0; /* Not landed. */
   }

   /* Get a planet by faction */
   else if (lua_isnumber(L,1)) {
      i = lua_tonumber(L,1);
      planets = space_getFactionPlanet( &nplanets, &i, 1 );
   }

   /* Get a planet by name */
   else if (lua_isstring(L,1)) {
      rndplanet = (char*) lua_tostring(L,1);
   }

   /* Get a planet from faction list */
   else if (lua_istable(L,1)) {
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
   }
   else NLUA_INVALID_PARAMETER(); /* Bad Parameter */

   /* No suitable planet found */
   if ((rndplanet == NULL) && (nplanets == 0)) {
      free(planets);
      return 0;
   }
   /* Pick random planet */
   else if (rndplanet == NULL) {
      rndplanet = planets[RNG(0,nplanets-1)];
      free(planets);
   }

   /* Push the planet */
   planet.p = planet_get(rndplanet); /* The real planet */
   lua_pushplanet(L,planet);
   sys.s = system_get( planet_getSystem(rndplanet) );
   lua_pushsystem(L,sys);
   return 2;
}

/**
 * @fn static int planetL_eq( lua_State *L )
 * @ingroup META_PLANET
 *
 * @brief bool __eq( planet comp )
 *
 * __eq (equality) metamethod for planets.
 *
 * You can use the '=' operator within Lua to compare planets with this.
 *
 *    @param comp planet to compare against.
 *    @return true if both planets are the same.
 */
static int planetL_eq( lua_State *L )
{
   LuaPlanet *a, *b;
   a = lua_toplanet(L,1);
   b = lua_toplanet(L,2);
   if (a->p == b->p)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}

/**
 * @fn static int planetL_name( lua_State *L )
 * @ingroup META_PLANET
 *
 * @brief string name( nil )
 *
 * Gets the planet's name.
 *
 *    @return The name of the planet.
 */
static int planetL_name( lua_State *L )
{
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   lua_pushstring(L,p->p->name);
   return 1;
}

/**
 * @fn static int planetL_faction( lua_State *L )
 * @ingroup META_PLANET
 *
 * @brief number faction( nil )
 *
 * Gets the planet's faction.
 *
 *    @return The planet's faction.
 */
static int planetL_faction( lua_State *L )
{
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   lua_pushstring(L,faction_name(p->p->faction));
   return 1;
}

/**
 * @fn static int planetL_class(lua_State *L )
 * @ingroup META_PLANET
 *
 * @brief string class( nil )
 *
 * Gets the planet's class.
 *
 *    @return The class of the planet in a one char identifier.
 */
static int planetL_class(lua_State *L )
{
   char buf[2];
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   buf[0] = planet_getClass(p->p);
   buf[1] = '\0';
   lua_pushstring(L,buf);
   return 1;
}

/**
 * @fn static int planetL_services( lua_State *L )
 * @ingroup META_PLANET
 *
 * @brief number services( nil )
 *
 * Gets planet services.
 *
 *    @return The services the planet has it stored bitwise.
 */
static int planetL_services( lua_State *L )
{
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   lua_pushnumber(L,p->p->services);
   return 1;
}



/**
 * @defgroup META_SYSTEM System Metatable
 *
 * @brief Represents a system in Lua.
 *
 * To call members of the metatable always use:
 * @code 
 * system:function( param )
 * @endcode
 */
/**
 * @fn LuaSystem* lua_tosystem( lua_State *L, int ind )
 *
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
 * @fn LuaSystem* lua_pushsystem( lua_State *L, LuaSystem sys )
 *
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
 * @fn int lua_issystem( lua_State *L, int ind )
 *
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
 * @fn static int systemL_get( lua_State *L )
 * @ingroup SPACE
 *
 * @brief system getSystem( [param] )
 *
 * Gets a system.
 *
 * Behaves differently depending on what you pass as param:
 *    - nil : Gets the current system.
 *    - string : Gets the system by name.
 *    - planet : Gets the system by planet.
 *
 *    @param param Read description for details.
 *    @return System metatable matching param.
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
 * @fn static int systemL_eq( lua_State *L )
 * @ingroup META_SYSTEM
 *
 * @brief bool __eq( system comp )
 *
 * Check systems for equality.
 *
 * Allows you to use the '=' operator in Lua with systems.
 *
 *    @param comp System to compare against.
 *    @return true if both systems are the same.
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
 * @fn static int systemL_name( lua_State *L )
 * @ingroup META_SYSTEM
 *
 * @brief string name( nil )
 *
 * Returns the system's name.
 *
 *    @return The name of the system.
 */
static int systemL_name( lua_State *L )
{
   LuaSystem *sys;
   sys = lua_tosystem(L,1);
   lua_pushstring(L,sys->s->name);
   return 1;
}

/**
 * @fn static int systemL_faction( lua_State *L )
 * @ingroup META_SYSTEM
 *
 * @brief table faction( nil )
 *
 * Gets system factions.
 *
 * @code
 * foo = space.faction("foo")
 * if foo["bar"] then
 *    print( "faction 'bar' found" )
 * end
 * @endcode
 *
 *    @return A table containing all the factions in the system.
 */
static int systemL_faction( lua_State *L )
{
   int i;
   LuaSystem *sys;
   sys = lua_tosystem(L,1);

   /* Return result in table */
   lua_newtable(L);
   for (i=0; i<sys->s->nplanets; i++) {
      if (sys->s->planets[i].faction > 0) { /* Faction must be valid */
         lua_pushboolean(L,1); /* value */
         lua_setfield(L,-2,faction_name(sys->s->planets[i].faction)); /* key */
         /* allows syntax foo = space.faction("foo"); if foo["bar"] then ... end */
      }
   }
   return 1;

}

/**
 * @fn static int systemL_jumpdistance( lua_State *L )
 * @ingroup META_SYSTEM
 *
 * @brief number jumpDist( [param] )
 *
 * Gets jump distance from current system, or to another.
 *
 * Does different things depending on the parameter type:
 *    - nil : Gets distance from current system.
 *    - string : Gets distance from system matching name.
 *    - system : Gets distance from system
 *
 *    @param param See description.
 *    @return Number of jumps to system.
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

