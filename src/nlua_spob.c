/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_spob.c
 *
 * @brief Lua spob module.
 */
/** @cond */
#include "naev.h"

#include <lauxlib.h>
/** @endcond */

#include "nlua_spob.h"

#include "array.h"
#include "land.h"
#include "land_outfits.h"
#include "log.h"
#include "map.h"
#include "map_overlay.h"
#include "nlua_colour.h"
#include "nlua_commodity.h"
#include "nlua_faction.h"
#include "nlua_outfit.h"
#include "nlua_ship.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_time.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "nmath.h"
#include "nstring.h"
#include "rng.h"

/* Spob metatable methods */
static int spobL_cur( lua_State *L );
static int spobL_get( lua_State *L );
static int spobL_getS( lua_State *L );
static int spobL_getLandable( lua_State *L );
static int spobL_getAll( lua_State *L );
static int spobL_system( lua_State *L );
static int spobL_eq( lua_State *L );
static int spobL_name( lua_State *L );
static int spobL_nameRaw( lua_State *L );
static int spobL_population( lua_State *L );
static int spobL_radius( lua_State *L );
static int spobL_faction( lua_State *L );
static int spobL_colour( lua_State *L );
static int spobL_class( lua_State *L );
static int spobL_classLong( lua_State *L );
static int spobL_position( lua_State *L );
static int spobL_services( lua_State *L );
static int spobL_flags( lua_State *L );
static int spobL_canland( lua_State *L );
static int spobL_landOverride( lua_State *L );
static int spobL_getLandOverride( lua_State *L );
static int spobL_gfxSpace( lua_State *L );
static int spobL_gfxExterior( lua_State *L );
static int spobL_gfxComm( lua_State *L );
static int spobL_shipsSold( lua_State *L );
static int spobL_outfitsSold( lua_State *L );
static int spobL_commoditiesSold( lua_State *L );
static int spobL_isBlackMarket( lua_State *L );
static int spobL_isKnown( lua_State *L );
static int spobL_setKnown( lua_State *L );
static int spobL_recordCommodityPriceAtTime( lua_State *L );
static int spobL_tags( lua_State *L );
static const luaL_Reg spob_methods[] = {
   { "cur", spobL_cur },
   { "get", spobL_get },
   { "getS", spobL_getS },
   { "getLandable", spobL_getLandable },
   { "getAll", spobL_getAll },
   { "system", spobL_system },
   { "__eq", spobL_eq },
   { "__tostring", spobL_name },
   { "name", spobL_name },
   { "nameRaw", spobL_nameRaw },
   { "population", spobL_population },
   { "radius", spobL_radius },
   { "faction", spobL_faction },
   { "colour", spobL_colour },
   { "class", spobL_class },
   { "classLong", spobL_classLong },
   { "pos", spobL_position },
   { "services", spobL_services },
   { "flags", spobL_flags },
   { "canLand", spobL_canland },
   { "landOverride", spobL_landOverride },
   { "getLandOverride", spobL_getLandOverride },
   { "gfxSpace", spobL_gfxSpace },
   { "gfxExterior", spobL_gfxExterior },
   { "gfxComm", spobL_gfxComm },
   { "shipsSold", spobL_shipsSold },
   { "outfitsSold", spobL_outfitsSold },
   { "commoditiesSold", spobL_commoditiesSold },
   { "blackmarket", spobL_isBlackMarket },
   { "known", spobL_isKnown },
   { "setKnown", spobL_setKnown },
   { "recordCommodityPriceAtTime", spobL_recordCommodityPriceAtTime },
   { "tags", spobL_tags },
   {0,0}
}; /**< Spob metatable methods. */

/**
 * @brief Loads the spob library.
 *
 *    @param env Environment to load spob library into.
 *    @return 0 on success.
 */
int nlua_loadSpob( nlua_env env )
{
   nlua_register(env, SPOB_METATABLE, spob_methods, 1);
   return 0; /* No error */
}

/**
 * @brief This module allows you to handle the spobs from Lua.
 *
 * Generally you do something like:
 *
 * @code
 * p,s = spob.get() -- Get current spob and system
 * if p:services()["inhabited"] > 0 then -- spob is inhabited
 *    v = p:pos() -- Get the position
 *    -- Do other stuff
 * end
 * @endcode
 *
 * @luamod spob
 */
/**
 * @brief Gets spob at index.
 *
 *    @param L Lua state to get spob from.
 *    @param ind Index position to find the spob.
 *    @return Spob found at the index in the state.
 */
LuaSpob lua_tospob( lua_State *L, int ind )
{
   return *((LuaSpob*) lua_touserdata(L,ind));
}
/**
 * @brief Gets spob at index raising an error if isn't a spob.
 *
 *    @param L Lua state to get spob from.
 *    @param ind Index position to find the spob.
 *    @return Spob found at the index in the state.
 */
LuaSpob luaL_checkspob( lua_State *L, int ind )
{
   if (lua_isspob(L,ind))
      return lua_tospob(L,ind);
   luaL_typerror(L, ind, SPOB_METATABLE);
   return 0;
}
/**
 * @brief Gets a spob directly.
 *
 *    @param L Lua state to get spob from.
 *    @param ind Index position to find the spob.
 *    @return Spob found at the index in the state.
 */
Spob* luaL_validspob( lua_State *L, int ind )
{
   LuaSpob lp;
   Spob *p;

   if (lua_isspob(L, ind)) {
      lp = luaL_checkspob(L, ind);
      p  = spob_getIndex(lp);
   }
   else if (lua_isstring(L, ind))
      p = spob_get( lua_tostring(L, ind) );
   else {
      luaL_typerror(L, ind, SPOB_METATABLE);
      return NULL;
   }

   if (p == NULL)
      NLUA_ERROR(L, _("Spob is invalid"));

   return p;
}
/**
 * @brief Pushes a spob on the stack.
 *
 *    @param L Lua state to push spob into.
 *    @param spob Spob to push.
 *    @return Newly pushed spob.
 */
LuaSpob* lua_pushspob( lua_State *L, LuaSpob spob )
{
   LuaSpob *p = (LuaSpob*) lua_newuserdata(L, sizeof(LuaSpob));
   *p = spob;
   luaL_getmetatable(L, SPOB_METATABLE);
   lua_setmetatable(L, -2);
   return p;
}
/**
 * @brief Checks to see if ind is a spob.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a spob.
 */
int lua_isspob( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, SPOB_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Gets the current spob - MUST BE LANDED.
 *
 * @usage p,s = spob.cur() -- Gets current spob (assuming landed)
 *
 *    @luatreturn Spob The spob the player is landed on.
 *    @luatreturn System The system it is in.
 * @luafunc cur
 */
static int spobL_cur( lua_State *L )
{
   LuaSystem sys;
   if (land_spob == NULL) {
      NLUA_ERROR(L,_("Attempting to get landed spob when player not landed."));
      return 0; /* Not landed. */
   }
   lua_pushspob(L,spob_index(land_spob));
   sys = system_index( system_get( spob_getSystem(land_spob->name) ) );
   lua_pushsystem(L,sys);
   return 2;
}

static int spobL_getBackend( lua_State *L, int system, int landable )
{
   int *factions;
   char **spobs;
   const char *rndspob;
   Spob *pnt;

   rndspob = NULL;
   spobs   = NULL;

   /* If boolean return random. */
   if (lua_isboolean(L,1)) {
      pnt = spob_get( space_getRndSpob(landable, 0, NULL) );
      rndspob = pnt->name;
   }

   /* Get a spob by faction */
   else if (lua_isfaction(L,1)) {
      factions = array_create( int );
      array_push_back( &factions, lua_tofaction(L,1) );
      spobs  = space_getFactionSpob( factions, landable );
      array_free( factions );
   }

   /* Get a spob by name */
   else if (lua_isstring(L,1)) {
      rndspob = lua_tostring(L,1);

      if (landable) {
         pnt = spob_get( rndspob );
         if (pnt == NULL) {
            NLUA_ERROR(L, _("Spob '%s' not found in stack"), rndspob);
            return 0;
         }

         /* Check if can land. */
         spob_updateLand( pnt );
         if (!pnt->can_land)
            return 0;
      }
   }

   /* Get a spob from faction list */
   else if (lua_istable(L,1)) {
      /* Get table length and preallocate. */
      factions = array_create_size( int, lua_objlen(L,1) );
      /* Load up the table. */
      lua_pushnil(L);
      while (lua_next(L, -2) != 0) {
         if (lua_isfaction(L, -1))
            array_push_back( &factions, lua_tofaction(L, -1) );
         lua_pop(L,1);
      }

      /* get the spobs */
      spobs = space_getFactionSpob( factions, landable );
      array_free(factions);
   }

   /* Just get a spob. */
   else if (lua_isspob(L,1)) {
      pnt = luaL_validspob( L, 1 );
      if (landable) {
         /* Check if can land. */
         spob_updateLand( pnt );
         if (!pnt->can_land)
            return 0;
      }
      rndspob = pnt->name;
   }

   else
      NLUA_INVALID_PARAMETER(L); /* Bad Parameter */

   /* Pick random spob */
   if (rndspob == NULL) {
      arrayShuffle( (void**)spobs );

      for (int i=0; i<array_size(spobs); i++) {
         if (landable) {
            /* Check landing. */
            pnt = spob_get( spobs[i] );
            if (pnt == NULL)
               continue;

            spob_updateLand( pnt );
            if (!pnt->can_land)
               continue;
         }

         rndspob = spobs[i];
         break;
      }
   }
   array_free(spobs);

   /* No suitable spob found */
   if (rndspob == NULL && array_size( spobs ) == 0)
      return 0;

   /* Push the spob */
   pnt = spob_get(rndspob); /* The real spob */
   if (pnt == NULL) {
      NLUA_ERROR(L, _("Spob '%s' not found in stack"), rndspob);
      return 0;
   }
   lua_pushspob(L,spob_index( pnt ));
   if (system) {
      LuaSystem sys;
      const char *sysname = spob_getSystem( rndspob );
      if (sysname == NULL)
         return 1;
      sys = system_index( system_get( sysname ) );
      lua_pushsystem( L, sys );
      return 2;
   }
   return 1;
}

/**
 * @brief Gets a spob.
 *
 * Possible values of param: <br/>
 *    - bool : Gets a random spob. <br/>
 *    - faction : Gets random spob belonging to faction matching the number. <br/>
 *    - string : Gets the spob by raw (untranslated) name. <br/>
 *    - table : Gets random spob belonging to any of the factions in the
 *               table. <br/>
 *
 * @usage p = spob.get( "Anecu" ) -- Gets spob by name
 * @usage p = spob.get( faction.get( "Empire" ) ) -- Gets random Empire spob
 * @usage p = spob.get(true) -- Gets completely random spob
 * @usage p = spob.get( { faction.get("Empire"), faction.get("Dvaered") } ) -- Random spob belonging to Empire or Dvaered
 *    @luatparam boolean|Faction|string|table param See description.
 *    @luatreturn Spob The matching spob.
 * @luafunc get
 */
static int spobL_get( lua_State *L )
{
   return spobL_getBackend( L, 0, 0 );
}

/**
 * @brief Gets a spob and its corresponding system.
 *
 * Possible values of param: <br/>
 *    - bool : Gets a random spob. <br/>
 *    - faction : Gets random spob belonging to faction matching the number. <br/>
 *    - string : Gets the spob by raw (untranslated) name. <br/>
 *    - table : Gets random spob belonging to any of the factions in the
 *               table. <br/>
 *
 * @usage p,s = spob.get( "Anecu" ) -- Gets spob by name
 * @usage p,s = spob.get( faction.get( "Empire" ) ) -- Gets random Empire spob
 * @usage p,s = spob.get(true) -- Gets completely random spob
 * @usage p,s = spob.get( { faction.get("Empire"), faction.get("Dvaered") } ) -- Random spob belonging to Empire or Dvaered
 *    @luatparam boolean|Faction|string|table param See description.
 *    @luatreturn Spob The matching spob.
 *    @luatreturn System The system it is in.
 * @luafunc getS
 */
static int spobL_getS( lua_State *L )
{
   return spobL_getBackend( L, 1, 0 );
}

/**
 * @brief Gets a spob only if it's landable.
 *
 * It works exactly the same as spob.get(), but it can only return landable
 * spobs. So if the target is not landable it returns nil.
 *
 *    @luatparam boolean|Faction|string|table param See spob.get() description.
 *    @luatreturn Spob The matching spob, if it is landable.
 *    @luatreturn System The system it is in.
 * @luafunc getLandable
 */
static int spobL_getLandable( lua_State *L )
{
   return spobL_getBackend( L, 1, 1 );
}

/**
 * @brief Gets all the spobs.
 *    @luatparam boolean all_spob Whether or not to get all Spob, including those that may not be located in a system at the time.
 *    @luatreturn {Spob,...} An ordered list of all the spobs.
 * @luafunc getAll
 */
static int spobL_getAll( lua_State *L )
{
   Spob *p = spob_getAll();
   int n = 1;
   int all_spob = lua_toboolean(L,1);
   lua_newtable(L);
   for (int i=0; i<array_size(p); i++) {
      if (!all_spob && !spob_hasSystem(&p[i]))
         continue;
      lua_pushspob( L, spob_index( &p[i] ) );
      lua_rawseti( L, -2, n++ );
   }
   return 1;
}

/**
 * @brief Gets the system corresponding to a spob.
 *    @luatparam Spob p Spob to get system of.
 *    @luatreturn System|nil The system to which the spob belongs or nil if it has none.
 * @luafunc system
 */
static int spobL_system( lua_State *L )
{
   LuaSystem sys;
   Spob *p = luaL_validspob(L,1);
   const char *sysname = spob_getSystem( p->name );
   if (sysname == NULL)
      return 0;
   sys = system_index( system_get( sysname ) );
   lua_pushsystem( L, sys );
   return 1;
}

/**
 * @brief You can use the '==' operator within Lua to compare spobs with this.
 *
 * @usage if p.__eq( spob.get( "Anecu" ) ) then -- Do something
 * @usage if p == spob.get( "Anecu" ) then -- Do something
 *    @luatparam Spob p Spob comparing.
 *    @luatparam Spob comp spob to compare against.
 *    @luatreturn boolean true if both spobs are the same.
 * @luafunc __eq
 */
static int spobL_eq( lua_State *L )
{
   LuaSpob a, b;
   a = luaL_checkspob(L,1);
   b = luaL_checkspob(L,2);
   lua_pushboolean(L,(a == b));
   return 1;
}

/**
 * @brief Gets the spob's translated name.
 *
 * This translated name should be used for display purposes (e.g.  messages).
 * It cannot be used as an identifier for the spob; for that, use
 * spob.nameRaw() instead.
 *
 * Note that it can be overwritten by the spob's display name which makes it
 * not equivalent to _(p:nameRaw()) in some cases.
 *
 * @usage name = p:name()
 *    @luatparam Spob p Spob to get the translated name of.
 *    @luatreturn string The translated name of the spob.
 * @luafunc name
 */
static int spobL_name( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushstring(L, spob_name(p));
   return 1;
}

/**
 * @brief Gets the spob's raw (untranslated) name.
 *
 * This untranslated name should be used for identification purposes
 * (e.g. can be passed to spob.get()). It should not be used directly
 * for display purposes without manually translating it with _().
 *
 * @usage name = p:nameRaw()
 *    @luatparam Spob p Spob to get the raw name of.
 *    @luatreturn string The raw name of the spob.
 * @luafunc nameRaw
 */
static int spobL_nameRaw( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushstring(L, p->name);
   return 1;
}

/**
 * @brief Gets the spob's population.
 *
 *    @luatparam Spob p Spob to get the population of.
 *    @luatreturn number The spob's population.
 * @luafunc population
 */
static int spobL_population( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushnumber(L,p->population);
   return 1;
}

/**
 * @brief Gets the spob's radius.
 *
 * @usage radius = p:radius()
 *    @luatparam Spob p Spob to get the radius of.
 *    @luatreturn number The spob's graphics radius.
 * @luafunc radius
 */
static int spobL_radius( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   /* Ensure graphics measurements are available. */
   if (p->radius < 0.)
      spob_gfxLoad(p);
   lua_pushnumber(L,p->radius);
   return 1;
}

/**
 * @brief Gets the spob's faction.
 *
 * @usage f = p:faction()
 *    @luatparam Spob p Spob to get the faction of.
 *    @luatreturn Faction The spob's faction, or nil if it has no faction.
 * @luafunc faction
 */
static int spobL_faction( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   if (p->presence.faction < 0)
      return 0;
   lua_pushfaction(L, p->presence.faction);
   return 1;
}

/**
 * @brief Gets a spob's colour based on its friendliness or hostility to the player.
 *
 * @usage col = p:colour()
 *
 *    @luatparam Pilot p Spob to get the colour of.
 *    @luatreturn Colour The spob's colour.
 * @luafunc colour
 */
static int spobL_colour( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   const glColour *col = spob_getColour( p );
   lua_pushcolour( L, *col );
   return 1;
}

/**
 * @brief Gets the spob's class.
 *
 * Usually classes are characters for spobs and numbers
 * for stations.
 *
 * @usage c = p:class()
 *    @luatparam Spob p Spob to get the class of.
 *    @luatreturn string The class of the spob in a one char identifier.
 * @luafunc class
 */
static int spobL_class(lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushstring(L,p->class);
   return 1;
}

/**
 * @brief Gets the spob's class in long human-readable format already translated.
 *
 * @usage c = p:classLong()
 *    @luatparam Spob p Spob to get the class of.
 *    @luatreturn string The class of the spob in descriptive form such as "Pelagic".
 * @luafunc classLong
 */
static int spobL_classLong(lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushstring(L, spob_getClassName(p->class));
   return 1;
}

/**
 * @brief Checks for spob services.
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
 *  - "blackmarket"<br />
 *
 * @usage if p:services()["refuel"] then -- Spob has refuel service.
 * @usage if p:services()["shipyard"] then -- Spob has shipyard service.
 *    @luatparam Spob p Spob to get the services of.
 *    @luatreturn table Table containing all the services.
 * @luafunc services
 */
static int spobL_services( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   /* Return result in table */
   lua_newtable(L);
   /* allows syntax like foo = spob.get("foo"); if foo["bar"] then ... end */
   for (int i=1; i<SPOB_SERVICES_MAX; i<<=1) {
      if (spob_hasService(p, i)) {
         char lower[STRMAX_SHORT];
         const char *name = spob_getServiceName(i);
         size_t len = strlen(name) + 1;
         snprintf( lower, MIN(len,sizeof(lower)), "%c%s", tolower(name[0]), &name[1] );

         /* GUI depends on this returning the service name. */
         lua_pushstring(L, _(name));
         lua_setfield(L, -2, lower );
      }
   }
   return 1;
}

/**
 * @brief Checks for spob flags.
 *
 * Possible services are:<br />
 *  - "nomissionspawn"<br />
 *
 * @usage if p:flags()["nomissionspawn"] then -- Spob doesn't spawn missions
 *    @luatparam Spob p Spob to get the services of.
 *    @luatreturn table Table containing all the services.
 * @luafunc services
 */
static int spobL_flags( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_newtable(L);
   if (spob_isFlag( p, SPOB_NOMISNSPAWN )) {
      lua_pushstring(L, "nomissionspawn");
      lua_pushboolean(L, 1);
      lua_settable(L,-3);
   }
   return 1;
}

/**
 * @brief Gets whether or not the player can land on the spob (or bribe it).
 *
 * @usage can_land, can_bribe = p:canLand()
 *    @luatparam Spob p Spob to get land and bribe status of.
 *    @luatreturn boolean The land status of the spob.
 * @luafunc canLand
 */
static int spobL_canland( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   spob_updateLand( p );
   lua_pushboolean( L, p->can_land );
   return 1;
}

/**
 * @brief Lets player land on a spob no matter what. The override lasts until the player jumps or lands.
 *
 * @usage p:landOverride( true ) -- Spob can land on p now.
 *    @luatparam Spob p Spob to forcibly allow the player to land on.
 *    @luatparam[opt=false] boolean b Whether or not the player should be allowed to land, true enables, false disables override.
 * @luafunc landOverride
 */
static int spobL_landOverride( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   int old = p->land_override;

   p->land_override = !!lua_toboolean(L,2);

   /* If the value has changed, re-run the landing Lua next frame. */
   if (p->land_override != old)
      space_factionChange();

   return 0;
}

/**
 * @brief Gets the land override status for a spob.
 *
 * @usage if p:getLandOverride() then -- Player can definitely land.
 *    @luatparam Spob p Spob to check.
 *    @luatreturn b Whether or not the player is always allowed to land.
 * @luafunc getLandOverride
 */
static int spobL_getLandOverride( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushboolean(L, p->land_override);
   return 1;
}

/**
 * @brief Gets the position of the spob in the system.
 *
 * @usage v = p:pos()
 *    @luatparam Spob p Spob to get the position of.
 *    @luatreturn Vec2 The position of the spob in the system.
 * @luafunc pos
 */
static int spobL_position( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushvector(L, p->pos);
   return 1;
}

/**
 * @brief Gets the texture of the spob in space.
 *
 * @usage gfx = p:gfxSpace()
 *    @luatparam Spob p Spob to get texture of.
 *    @luatreturn Tex The space texture of the spob.
 * @luafunc gfxSpace
 */
static int spobL_gfxSpace( lua_State *L )
{
   glTexture *tex;
   Spob *p = luaL_validspob(L,1);
   if (p->gfx_space == NULL) { /* Not loaded. */
      /* If the spob has no texture, just return nothing. */
      if (p->gfx_spaceName == NULL)
         return 0;
      tex = gl_newImage( p->gfx_spaceName, OPENGL_TEX_MIPMAPS );
   }
   else
      tex = gl_dupTexture( p->gfx_space );
   lua_pushtex( L, tex );
   return 1;
}

/**
 * @brief Gets the texture of the spob in exterior.
 *
 * @usage gfx = p:gfxExterior()
 *    @luatparam Spob p Spob Spob to get texture of.
 *    @luatreturn Tex The exterior texture of the spob.
 * @luafunc gfxExterior
 */
static int spobL_gfxExterior( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);

   /* If no exterior image just return nothing instead of crashing. */
   if (p->gfx_exterior==NULL)
      return 0;

   lua_pushtex( L, gl_newImage( p->gfx_exterior, 0 ) );
   return 1;
}

/**
 * @brief Gets the texture of the spob for the communication window.
 *
 * @usage gfx = p:gfxComm()
 *    @luatparam Spob p Spob Spob to get texture of.
 *    @luatreturn Tex The communication texture of the spob.
 * @luafunc gfxComm
 */
static int spobL_gfxComm( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   if (p->gfx_comm==NULL)
      return spobL_gfxSpace(L);
   lua_pushtex( L, gl_newImage( p->gfx_comm, 0 ) );
   return 1;
}

/**
 * @brief Gets the ships sold at a spob.
 *
 *    @luatparam Spob p Spob to get ships sold at.
 *    @luatreturn {Ship,...} An ordered table containing all the ships sold (empty if none sold).
 * @luafunc shipsSold
 */
static int spobL_shipsSold( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   Ship **s = tech_getShip( p->tech );

   /* Push results in a table. */
   lua_newtable(L);
   for (int i=0; i<array_size(s); i++) {
      lua_pushship(L,s[i]); /* value = LuaShip */
      lua_rawseti(L,-2,i+1); /* store the value in the table */
   }

   array_free(s);
   return 1;
}

/**
 * @brief Gets the outfits sold at a spob.
 *
 *    @luatparam Spob p Spob to get outfits sold at.
 *    @luatreturn {Outfit,...} An ordered table containing all the outfits sold (empty if none sold).
 * @luafunc outfitsSold
 */
static int spobL_outfitsSold( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   Outfit **o = tech_getOutfit( p->tech );

   /* Push results in a table. */
   lua_newtable(L);
   for (int i=0; i<array_size(o); i++) {
      lua_pushoutfit(L,o[i]); /* value = LuaOutfit */
      lua_rawseti(L,-2,i+1); /* store the value in the table */
   }

   array_free(o);
   return 1;
}

/**
 * @brief Gets the commodities sold at a spob.
 *
 *    @luatparam Pilot p Spob to get commodities sold at.
 *    @luatreturn {Commodity,...} An ordered table containing all the commodities sold (empty if none sold).
 * @luafunc commoditiesSold
 */
static int spobL_commoditiesSold( lua_State *L )
{
   /* Get result and tech. */
   Spob *p = luaL_validspob(L,1);

   /* Push results in a table. */
   lua_newtable(L);
   for (int i=0; i<array_size(p->commodities); i++) {
      lua_pushcommodity(L,p->commodities[i]); /* value = LuaCommodity */
      lua_rawseti(L,-2,i+1); /* store the value in the table */
   }

   return 1;
}

/**
 * @brief Checks to see if a spob is a black market.
 *
 * @usage b = p:blackmarket()
 *
 *    @luatparam Spob p Spob to check if it's a black market.
 *    @luatreturn boolean true if the spob is a black market.
 * @luafunc blackmarket
 */
static int spobL_isBlackMarket( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_pushboolean(L, spob_hasService(p, SPOB_SERVICE_BLACKMARKET));
   return 1;
}

/**
 * @brief Checks to see if a spob is known by the player.
 *
 * @usage b = p:known()
 *
 *    @luatparam Spob p Spob to check if the player knows.
 *    @luatreturn boolean true if the player knows the spob.
 * @luafunc known
 */
static int spobL_isKnown( lua_State *L )
{
   Spob *s = luaL_validspob(L,1);
   lua_pushboolean(L, spob_isKnown(s));
   return 1;
}

/**
 * @brief Sets a spobs's known state.
 *
 * @usage p:setKnown( false ) -- Makes spob unknown.
 *    @luatparam Spob p Spob to set known.
 *    @luatparam[opt=false] boolean b Whether or not to set as known.
 * @luafunc setKnown
 */
static int spobL_setKnown( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   int b = lua_toboolean(L, 2);

   int changed = (b != (int)spob_isKnown(p));

   if (b)
      spob_setKnown( p );
   else
      spob_rmFlag( p, SPOB_KNOWN );

   if (changed) {
      ovr_refresh();
      /* Update outfits image array. */
      outfits_updateEquipmentOutfits();
   }

   return 0;
}

/**
 * @brief Records commodity prices at a given time, adding to players stats.
 *
 * @usage p:recordCommodityPriceAtTime( t )
 *    @luatparam Spob p Spob to record prices at
 *    @luatparam ntime_t t Time at which to record prices.
 * @luafunc recordCommodityPriceAtTime
 */
static int spobL_recordCommodityPriceAtTime( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   ntime_t t = luaL_validtime(L, 2);
   spob_averageSeenPricesAtTime( p, t );
   return 0;
}

/**
 * @brief Gets the spob tags.
 *
 * @usage if spob.cur():tags["fancy"] then -- Has "fancy" tag
 *
 *    @luatparam Spob p Spob to get tags of.
 *    @luatreturn table Table of tags where the name is the key and true is the value.
 * @luafunc tags
 */
static int spobL_tags( lua_State *L )
{
   Spob *p = luaL_validspob(L,1);
   lua_newtable(L);
   for (int i=0; i<array_size(p->tags); i++) {
      lua_pushstring(L,p->tags[i]);
      lua_pushboolean(L,1);
      lua_rawset(L,-3);
   }
   return 1;
}
