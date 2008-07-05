/*
 * See Licensing and Copyright notice in naev.h
 */


#include "nlua_space.h"

#include "lauxlib.h"

#include "log.h"
#include "naev.h"
#include "rng.h"
#include "space.h"
#include "land.h"
#include "nluadef.h"
#include "map.h"


#define PLANET_METATABLE   "Planet"
#define SYSTEM_METATABLE   "System"


/*
 * Lua wrappers.
 */
typedef struct LuaPlanet_ {
   Planet *p;
} LuaPlanet;
typedef struct LuaSystem_ {
   StarSystem *s;
} LuaSystem;


/*
 * Prototypes
 */
static int planetL_createmetatable( lua_State *L );

/* space */
static int planetL_get( lua_State *L );
static int space_getSystem( lua_State *L );
static int spaceL_getlanded( lua_State *L );
static int space_systemName( lua_State *L );
static int space_jumpDist( lua_State *L );
static int space_faction( lua_State *L );
static const luaL_reg space_methods[] = {
   { "getPlanet", planetL_get },
   { "getSystem", space_getSystem },
   { "getLanded", spaceL_getlanded },
   { "system", space_systemName },
   { "jumpDist", space_jumpDist },
   { "faction", space_faction },
   {0,0}
};

/* Planet metatable methods */
static int planetL_eq( lua_State *L );
static int planetL_name( lua_State *L );
static int planetL_faction( lua_State *L );
static int planetL_class( lua_State *L );
static int planetL_services( lua_State *L );
static const luaL_reg planet_methods[] = {
   { "__eq", planetL_eq },
   { "__tostring", planetL_name },
   { "name", planetL_name },
   { "faction", planetL_faction },
   { "class", planetL_class },
   { "services", planetL_services },
   {0,0}
};



/*
 * Loads the space library.
 */
int lua_loadSpace( lua_State *L, int readonly )
{
   (void)readonly;

   /*
    * Register the functions.
    */
   luaL_register(L, "space", space_methods);


   /*
    * Register the metatables
    */
   planetL_createmetatable( L );

   return 0;
}


/*
 *   S P A C E
 */

/*
 * Creates a Planet metatable from a planet and puts it on top
 * of the stack.
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

/*
 *   P L A N E T
 */
/*
 * Gets planet at index.
 */
static LuaPlanet* lua_toplanet( lua_State *L, int ind )
{
   if (lua_isuserdata(L,ind)) {
      return (LuaPlanet*) lua_touserdata(L,ind);
   }
   luaL_typerror(L, ind, PLANET_METATABLE);
   return NULL;
}

/*
 * Pushes a planet on the stack.
 */
static LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet )
{
   LuaPlanet *p;
   p = (LuaPlanet*) lua_newuserdata(L, sizeof(LuaPlanet));
   *p = planet;
   luaL_getmetatable(L, PLANET_METATABLE);
   lua_setmetatable(L, -2);
   return p;
}

/*
 * Gets a planet.
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

   rndplanet = NULL;
   nplanets = 0;
  
   /* Get a random planet */
   if (lua_gettop(L) == 0) {
      rndplanet = space_getRndPlanet();
   }

   /* Get a planet by faction */
   else if (lua_isnumber(L,1)) {
      i = lua_tonumber(L,1);
      planets = space_getFactionPlanet( &nplanets, &i, 1 );
   }

   /* Get a planet by name */
   else if (lua_isstring(L,1)) {
      i = faction_get((char*) lua_tostring(L,1));
      planets = space_getFactionPlanet( &nplanets, &i, 1 );
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
   lua_pushstring(L,planet_getSystem(rndplanet));
   return 2;
}

/*
 * __eq (equality) metamethod for planets.
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

/*
 * Gets the planet's name.
 */
static int planetL_name( lua_State *L )
{
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   lua_pushstring(L,p->p->name);
   return 1;
}

/*
 * Gets the planet's faction.
 */
static int planetL_faction( lua_State *L )
{
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   lua_pushnumber(L,p->p->faction);
   return 1;
}

/*
 * Gets the planet's class.
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
static int planetL_services( lua_State *L )
{
   LuaPlanet *p;
   p = lua_toplanet(L,1);
   lua_pushnumber(L,p->p->services);
   return 1;
}



/*
 * Gets a system.
 */
static int space_getSystem( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   char *planetname, *sysname;

   if (lua_isstring(L,1)) planetname = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   sysname = planet_getSystem( planetname );
   lua_pushstring(L,sysname);
   return 1;
}
static int spaceL_getlanded( lua_State *L )
{
   LuaPlanet planet;
   if (landed) {
      planet.p = land_planet;
      lua_pushplanet(L,planet);
      return 1;
   }
   return 0;
}
static int space_systemName( lua_State *L )
{
   lua_pushstring(L, cur_system->name);
   return 1;
}
static int space_jumpDist( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   StarSystem **s;
   int jumps;
   char *start, *goal;

   if (lua_isstring(L,1))
      start = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   if ((lua_gettop(L) > 1) && lua_isstring(L,2))
      goal = (char*) lua_tostring(L,2);
   else
      goal = cur_system->name;

   s = map_getJumpPath( &jumps, start, goal, 1 );
   free(s);

   lua_pushnumber(L,jumps);
   return 1;
}
static int space_faction( lua_State *L )
{
   int i;
   StarSystem *s;

   /* Get system */
   if (lua_isstring(L,1))
      s = system_get( lua_tostring(L,1) );
   else s = cur_system;

   /* Check if valid */
   if (s == NULL) {
      NLUA_DEBUG("Invalid system!");
      return 0;
   }

   /* Return result in table */
   lua_newtable(L);
   for (i=0; i<s->nplanets; i++) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,faction_name(s->planets[i].faction)); /* key */
      /* allows syntax foo = space.faction("foo"); if foo["bar"] then ... end */
   }
   return 1;
}

