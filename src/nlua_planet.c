/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_planet.c
 *
 * @brief Lua planet module.
 */

#include "nlua_planet.h"

#include "naev.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_ship.h"
#include "nlua_outfit.h"
#include "nlua_commodity.h"
#include "log.h"
#include "rng.h"
#include "land.h"
#include "map.h"


/* Planet metatable methods */
static int planetL_cur( lua_State *L );
static int planetL_get( lua_State *L );
static int planetL_eq( lua_State *L );
static int planetL_name( lua_State *L );
static int planetL_faction( lua_State *L );
static int planetL_class( lua_State *L );
static int planetL_position( lua_State *L );
static int planetL_services( lua_State *L );
static int planetL_gfxSpace( lua_State *L );
static int planetL_gfxExterior( lua_State *L );
static int planetL_shipsSold( lua_State *L );
static int planetL_outfitsSold( lua_State *L );
static int planetL_commoditiesSold( lua_State *L );
static const luaL_reg planet_methods[] = {
   { "cur", planetL_cur },
   { "get", planetL_get },
   { "__eq", planetL_eq },
   { "__tostring", planetL_name },
   { "name", planetL_name },
   { "faction", planetL_faction },
   { "class", planetL_class },
   { "pos", planetL_position },
   { "services", planetL_services },
   { "gfxSpace", planetL_gfxSpace },
   { "gfxExterior", planetL_gfxExterior },
   { "shipsSold", planetL_shipsSold },
   { "outfitsSold", planetL_outfitsSold },
   { "commoditiesSold", planetL_commoditiesSold },
   {0,0}
}; /**< Planet metatable methods. */


/**
 * @brief Loads the planet library.
 *
 *    @param L State to load planet library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadPlanet( lua_State *L, int readonly )
{
   (void) readonly;
   /* Create the metatable */
   luaL_newmetatable(L, PLANET_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, planet_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, PLANET_METATABLE);

   return 0; /* No error */
}


/**
 * @brief This module allows you to handle the planets from Lua.
 *
 * Generally you do something like:
 *
 * @code
 * p,s = planet.get() -- Get current planet and system
 * if p:services()["inhabited"] > 0 then -- planet is inhabited
 *    v = p:pos() -- Get the position
 *    -- Do other stuff
 * end
 * @endcode
 *
 * @luamod planet
 */
/**
 * @brief Gets planet at index.
 *
 *    @param L Lua state to get planet from.
 *    @param ind Index position to find the planet.
 *    @return Planet found at the index in the state.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind )
{
   return (LuaPlanet*) lua_touserdata(L,ind);
}
/**
 * @brief Gets planet at index raising an error if isn't a planet.
 *
 *    @param L Lua state to get planet from.
 *    @param ind Index position to find the planet.
 *    @return Planet found at the index in the state.
 */
LuaPlanet* luaL_checkplanet( lua_State *L, int ind )
{
   if (lua_isplanet(L,ind))
      return lua_toplanet(L,ind);
   luaL_typerror(L, ind, PLANET_METATABLE);
   return NULL;
}
/**
 * @brief Gets a planet directly.
 *
 *    @param L Lua state to get planet from.
 *    @param ind Index position to find the planet.
 *    @return Planet found at the index in the state.
 */
Planet* luaL_validplanet( lua_State *L, int ind )
{
   LuaPlanet *lp;
   Planet *p;
   lp = luaL_checkplanet( L, ind );
   p  = planet_getIndex( lp->id );
   if (p == NULL) {
      NLUA_ERROR( L, "Planet is invalid" );
      return NULL;
   }
   return p;
}
/**
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
 * @brief Gets the current planet - MUST BE LANDED.
 *
 * @usage p,s = planet.cur() -- Gets current planet (assuming landed)
 *
 *    @luareturn The planet and system in belongs to.
 * @luafunc cur()
 */
static int planetL_cur( lua_State *L )
{
   LuaPlanet planet;
   LuaSystem sys;
   if (land_planet == NULL) {
      NLUA_ERROR(L,"Attempting to get landed planet when player not landed.");
      return 0; /* Not landed. */
   }
   planet.id = planet_index(land_planet);
   lua_pushplanet(L,planet);
   sys.id = system_index( system_get( planet_getSystem(land_planet->name) ) );
   lua_pushsystem(L,sys);
   return 2;
}


/**
 * @brief Gets a planet.
 *
 * Possible values of param:
 *    - nil : -OBSOLETE- Gets the current landed planet or nil if there is none. Use planet.cur() instead.
 *    - bool : Gets a random planet.
 *    - faction : Gets random planet belonging to faction matching the number.
 *    - string : Gets the planet by name.
 *    - table : Gets random planet belonging to any of the factions in the
 *               table.
 *
 * @usage p,s = planet.get( "Anecu" ) -- Gets planet by name
 * @usage p,s = planet.get( faction.get( "Empire" ) ) -- Gets random Empire planet
 * @usage p,s = planet.get(true) -- Gets completely random planet
 * @usage p,s = planet.get( { faction.get("Empire"), faction.get("Dvaered") } ) -- Random planet belonging to Empire or Dvaered
 *    @luaparam param See description.
 *    @luareturn Returns the planet and the system it belongs to.
 * @luafunc get( param )
 */
static int planetL_get( lua_State *L )
{
   int i;
   int *factions;
   int nfactions;
   char **planets;
   int nplanets;
   const char *rndplanet;
   LuaPlanet planet;
   LuaSystem luasys;
   LuaFaction *f;
   Planet *pnt;
   StarSystem *sys;
   char *sysname;

   rndplanet = NULL;
   planets   = NULL;
   nplanets  = 0;

   /* Get the landed planet */
   if (lua_gettop(L) == 0) {
      if (land_planet != NULL) {
         planet.id = planet_index( land_planet );
         lua_pushplanet(L,planet);
         luasys.id = system_index( system_get( planet_getSystem(land_planet->name) ) );
         lua_pushsystem(L,luasys);
         return 2;
      }
      NLUA_ERROR(L,"Attempting to get landed planet when player not landed.");
      return 0; /* Not landed. */
   }

   /* If boolean return random. */
   else if (lua_isboolean(L,1)) {
      pnt = planet_get( space_getRndPlanet() );
      planet.id    = planet_index( pnt );
      lua_pushplanet(L,planet);
      luasys.id      = system_index( system_get( planet_getSystem(pnt->name) ) );
      lua_pushsystem(L,luasys);
      return 2;
   }

   /* Get a planet by faction */
   else if (lua_isfaction(L,1)) {
      f = lua_tofaction(L,1);
      planets = space_getFactionPlanet( &nplanets, &f->f, 1 );
   }

   /* Get a planet by name */
   else if (lua_isstring(L,1)) {
      rndplanet = lua_tostring(L,1);
   }

   /* Get a planet from faction list */
   else if (lua_istable(L,1)) {
      /* Get table length and preallocate. */
      nfactions = (int) lua_objlen(L,1);
      factions = malloc( sizeof(int) * nfactions );
      /* Load up the table. */
      lua_pushnil(L);
      i = 0;
      while (lua_next(L, -2) != 0) {
         f = lua_tofaction(L, -1);
         factions[i++] = f->f;
         lua_pop(L,1);
      }

      /* get the planets */
      planets = space_getFactionPlanet( &nplanets, factions, nfactions );
      free(factions);
   }
   else 
      NLUA_INVALID_PARAMETER(L); /* Bad Parameter */

   /* No suitable planet found */
   if ((rndplanet == NULL) && ((planets == NULL) || nplanets == 0))
      return 0;
   /* Pick random planet */
   else if (rndplanet == NULL) {
      rndplanet = planets[RNG(0,nplanets-1)];
      free(planets);
   }

   /* Push the planet */
   pnt = planet_get(rndplanet); /* The real planet */
   if (pnt == NULL) {
      NLUA_ERROR(L, "Planet '%s' not found in stack", rndplanet);
      return 0;
   }
   sysname = planet_getSystem(rndplanet);
   if (sysname == NULL) {
      NLUA_ERROR(L, "Planet '%s' is not placed in a system", rndplanet);
      return 0;
   }
   sys = system_get( sysname );
   if (sys == NULL) {
      NLUA_ERROR(L, "Planet '%s' can't find system '%s'", rndplanet, sysname); 
      return 0;
   }
   planet.id = planet_index( pnt );
   lua_pushplanet(L,planet);
   luasys.id = system_index( sys );
   lua_pushsystem(L,luasys);
   return 2;
}

/**
 * @brief You can use the '=' operator within Lua to compare planets with this.
 *
 * @usage if p.__eq( planet.get( "Anecu" ) ) then -- Do something
 * @usage if p == planet.get( "Anecu" ) then -- Do something
 *    @luaparam p Planet comparing.
 *    @luaparam comp planet to compare against.
 *    @luareturn true if both planets are the same.
 * @luafunc __eq( p, comp )
 */
static int planetL_eq( lua_State *L )
{
   LuaPlanet *a, *b;
   a = luaL_checkplanet(L,1);
   b = luaL_checkplanet(L,2);
   lua_pushboolean(L,(a->id == b->id));
   return 1;
}

/**
 * @brief Gets the planet's name.
 *
 * @usage name = p:name()
 *    @luaparam p Planet to get the name of.
 *    @luareturn The name of the planet.
 * @luafunc name( p )
 */
static int planetL_name( lua_State *L )
{
   Planet *p;
   p = luaL_validplanet(L,1);
   lua_pushstring(L,p->name);
   return 1;
}

/**
 * @brief Gets the planet's faction.
 *
 * @usage f = p:faction()
 *    @luaparam p Planet to get the faction of.
 *    @luareturn The planet's faction.
 * @luafunc faction( p )
 */
static int planetL_faction( lua_State *L )
{
   Planet *p;
   LuaFaction f;
   p = luaL_validplanet(L,1);
   if (p->faction < 0)
      return 0;
   f.f = p->faction;
   lua_pushfaction(L, f);
   return 1;
}

/**
 * @brief Gets the planet's class.
 *
 * Usually classes are characters for planets (see space.h) and numbers
 * for stations.
 *
 * @usage c = p:class()
 *    @luaparam p Planet to get the class of.
 *    @luareturn The class of the planet in a one char identifier.
 * @luafunc class( p )
 */
static int planetL_class(lua_State *L )
{
   char buf[2];
   Planet *p;
   p = luaL_validplanet(L,1);
   buf[0] = planet_getClass(p);
   buf[1] = '\0';
   lua_pushstring(L,buf);
   return 1;
}


/**
 * @brief Checks for planet services.
 *
 * Possible services are:<br />
 *  - "land"<br />
 *  - "inhabited"<br />
 *  - "refuel"<br />
 *  - "bar"<br />
 *  - "missions"<br />
 *  - "commodity"<br />
 *  - "outfits"<br />
 *  - "shipyard"<br />
 *
 * @usage if p:serivces()["refuel"] then -- PLanet has refuel service.
 * #usage if p:services()["shipyard"] then -- Planet has shipyard service.
 *    @luaparam p Planet to get the services of.
 *    @luareturn Table containing all the services.
 * @luafunc services( p )
 */
static int planetL_services( lua_State *L )
{
   Planet *p;
   p = luaL_validplanet(L,1);

   /* Return result in table */
   lua_newtable(L);
      /* allows syntax foo = space.faction("foo"); if foo["bar"] then ... end */
   if (planet_hasService(p, PLANET_SERVICE_LAND)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"land"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_INHABITED)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"inhabited"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_REFUEL)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"refuel"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_BAR)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"bar"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_MISSIONS)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"missions"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_COMMODITY)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"commodity"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_OUTFITS)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"outfits"); /* key */
   }
   if (planet_hasService(p, PLANET_SERVICE_SHIPYARD)) {
      lua_pushboolean(L,1); /* value */
      lua_setfield(L,-2,"shipyard"); /* key */
   }
   return 1;
}


/**
 * @brief Gets the position of the planet in the system.
 *
 * @usage v = p:pos()
 *    @luaparam p Planet to get the position of.
 *    @luareturn The position of the planet in the system as a vec2.
 * @luafunc pos( p )
 */
static int planetL_position( lua_State *L )
{
   Planet *p;
   LuaVector v;
   p = luaL_validplanet(L,1);
   vectcpy(&v.vec, &p->pos);
   lua_pushvector(L, v);
   return 1;
}


/**
 * @brief Gets the texture of the planet in space.
 *
 * @uasge gfx = p:gfxSpace()
 *    @luaparam p Planet to get texture of.
 *    @luareturn The space texture of the planet.
 * @luafunc gfxSpace( p )
 */
static int planetL_gfxSpace( lua_State *L )
{
   Planet *p;
   LuaTex lt;
   p        = luaL_validplanet(L,1);
   if (p->gfx_space == NULL) /* Not loaded. */
      lt.tex   = gl_newImage( p->gfx_spaceName, OPENGL_TEX_MIPMAPS );
   else
      lt.tex   = gl_dupTexture( p->gfx_space );
   lua_pushtex( L, lt );
   return 1;
}


/**
 * @brief Gets the texture of the planet in exterior.
 *
 * @uasge gfx = p:gfxExterior()
 *    @luaparam p Planet to get texture of.
 *    @luareturn The exterior texture of the planet.
 * @luafunc gfxExterior( p )
 */
static int planetL_gfxExterior( lua_State *L )
{
   Planet *p;
   LuaTex lt;
   p        = luaL_validplanet(L,1);
   lt.tex   = gl_newImage( p->gfx_exterior, 0 );
   lua_pushtex( L, lt );
   return 1;
}


/**
 * @brief Gets the ships sold at a planet.
 *
 *    @luaparam p Planet to get ships sold at.
 *    @luareturn An ordered table containing all the ships sold (empty if none sold).
 * @luafunc shipsSold( p )
 */
static int planetL_shipsSold( lua_State *L )
{
   Planet *p;
   int i, n;
   LuaShip ls;
   Ship **s;

   /* Get result and tech. */
   p = luaL_validplanet(L,1);
   s = tech_getShip( p->tech, &n );

   /* Push results in a table. */
   lua_newtable(L);
   for (i=0; i<n; i++) {
      lua_pushnumber(L,i+1); /* index, starts with 1 */
      ls.ship = s[i];
      lua_pushship(L,ls); /* value = LuaShip */
      lua_rawset(L,-3); /* store the value in the table */
   }

   return 1;
}


/**
 * @brief Gets the outfits sold at a planet.
 *
 *    @luaparam p Planet to get outfits sold at.
 *    @luareturn An ordered table containing all the outfits sold (empty if none sold).
 * @luafunc outfitsSold( p )
 */
static int planetL_outfitsSold( lua_State *L )
{
   Planet *p;
   int i, n;
   LuaOutfit lo;
   Outfit **o;

   /* Get result and tech. */
   p = luaL_validplanet(L,1);
   o = tech_getOutfit( p->tech, &n );

   /* Push results in a table. */
   lua_newtable(L);
   for (i=0; i<n; i++) {
      lua_pushnumber(L,i+1); /* index, starts with 1 */
      lo.outfit = o[i];
      lua_pushoutfit(L,lo); /* value = LuaOutfit */
      lua_rawset(L,-3); /* store the value in the table */
   }

   return 1;
}


/**
 * @brief Gets the commodities sold at a planet.
 *
 *    @luaparam p Planet to get commodities sold at.
 *    @luareturn An ordered table containing all the commodities sold (empty if none sold).
 * @luafunc commoditiesSold( p )
 */
static int planetL_commoditiesSold( lua_State *L )
{
   Planet *p;
   int i, n;
   LuaCommodity lc;
   Commodity **c;

   /* Get result and tech. */
   p = luaL_validplanet(L,1);
   c = tech_getCommodity( p->tech, &n );

   /* Push results in a table. */
   lua_newtable(L);
   for (i=0; i<n; i++) {
      lua_pushnumber(L,i+1); /* index, starts with 1 */
      lc.commodity = c[i];
      lua_pushcommodity(L,lc); /* value = LuaCommodity */
      lua_rawset(L,-3); /* store the value in the table */
   }

   return 1;
}



