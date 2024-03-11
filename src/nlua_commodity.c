/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_commodity.c
 *
 * @brief Handles the Lua commodity bindings.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_commodity.h"

#include "array.h"
#include "log.h"
#include "ndata.h"
#include "nlua_faction.h"
#include "nlua_spob.h"
#include "nlua_system.h"
#include "nlua_time.h"
#include "nlua_tex.h"
#include "nluadef.h"
#include "rng.h"

/* Commodity metatable methods. */
static int commodityL_eq( lua_State *L );
static int commodityL_get( lua_State *L );
static int commodityL_getStandard( lua_State *L );
static int commodityL_flags( lua_State *L );
static int commodityL_name( lua_State *L );
static int commodityL_nameRaw( lua_State *L );
static int commodityL_price( lua_State *L );
static int commodityL_priceAt( lua_State *L );
static int commodityL_priceAtTime( lua_State *L );
static int commodityL_canSell( lua_State *L );
static int commodityL_canBuy( lua_State *L );
static int commodityL_icon( lua_State *L );
static int commodityL_description( lua_State *L );
static int commodityL_new( lua_State *L );
static int commodityL_illegalto( lua_State *L );
static int commodityL_illegality( lua_State *L );
static const luaL_Reg commodityL_methods[] = {
   { "__tostring", commodityL_name },
   { "__eq", commodityL_eq },
   { "get", commodityL_get },
   { "getStandard", commodityL_getStandard },
   { "flags", commodityL_flags },
   { "name", commodityL_name },
   { "nameRaw", commodityL_nameRaw },
   { "price", commodityL_price },
   { "priceAt", commodityL_priceAt },
   { "priceAtTime", commodityL_priceAtTime },
   { "canSell", commodityL_canSell },
   { "canBuy", commodityL_canBuy },
   { "icon", commodityL_icon },
   { "description", commodityL_description },
   { "new", commodityL_new },
   { "illegalto", commodityL_illegalto },
   { "illegality", commodityL_illegality },
   {0,0}
}; /**< Commodity metatable methods. */

/**
 * @brief Loads the commodity library.
 *
 *    @param env Environment to load commodity library into.
 *    @return 0 on success.
 */
int nlua_loadCommodity( nlua_env env )
{
   nlua_register(env, COMMODITY_METATABLE, commodityL_methods, 1);
   return 0;
}

/**
 * @brief Lua bindings to interact with commodities.
 *
 * This will allow you to create and manipulate commodities in-game.
 *
 * An example would be:
 * @code
 * c = commodity.get( "Food" ) -- Gets the commodity by name
 * if c:price() > 500 then
 *    -- Do something with high price
 * end
 * @endcode
 *
 * @luamod commodity
 */
/**
 * @brief Gets commodity at index.
 *
 *    @param L Lua state to get commodity from.
 *    @param ind Index position to find the commodity.
 *    @return Commodity found at the index in the state.
 */
Commodity* lua_tocommodity( lua_State *L, int ind )
{
   return *((Commodity**) lua_touserdata(L,ind));
}
/**
 * @brief Gets commodity at index or raises error if there is no commodity at index.
 *
 *    @param L Lua state to get commodity from.
 *    @param ind Index position to find commodity.
 *    @return Commodity found at the index in the state.
 */
Commodity* luaL_checkcommodity( lua_State *L, int ind )
{
   if (lua_iscommodity(L,ind))
      return lua_tocommodity(L,ind);
   luaL_typerror(L, ind, COMMODITY_METATABLE);
   return NULL;
}
/**
 * @brief Makes sure the commodity is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the commodity to validate.
 *    @return The commodity (doesn't return if fails - raises Lua error ).
 */
Commodity* luaL_validcommodity( lua_State *L, int ind )
{
   Commodity *o;

   if (lua_iscommodity(L, ind))
      o = luaL_checkcommodity(L, ind);
   else if (lua_isstring(L, ind))
      o = commodity_get( lua_tostring(L, ind) );
   else {
      luaL_typerror(L, ind, COMMODITY_METATABLE);
      return NULL;
   }

   if (o == NULL)
      NLUA_ERROR(L, _("Commodity is invalid."));

   return o;
}
/**
 * @brief Pushes a commodity on the stack.
 *
 *    @param L Lua state to push commodity into.
 *    @param commodity Commodity to push.
 *    @return Newly pushed commodity.
 */
Commodity** lua_pushcommodity( lua_State *L, Commodity* commodity )
{
   Commodity **o = (Commodity**) lua_newuserdata(L, sizeof(Commodity*));
   *o = commodity;
   luaL_getmetatable(L, COMMODITY_METATABLE);
   lua_setmetatable(L, -2);
   return o;
}
/**
 * @brief Checks to see if ind is a commodity.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a commodity.
 */
int lua_iscommodity( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, COMMODITY_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Checks to see if two commodities are the same.
 *
 * @usage if o1 == o2 then -- Checks to see if commodity o1 and o2 are the same
 *
 *    @luatparam Commodity o1 First commodity to compare.
 *    @luatparam Commodity o2 Second commodity to compare.
 *    @luatreturn boolean true if both commodities are the same.
 * @luafunc __eq
 */
static int commodityL_eq( lua_State *L )
{
   const Commodity *a, *b;
   a = luaL_checkcommodity(L,1);
   b = luaL_checkcommodity(L,2);
   if (a == b)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}

/**
 * @brief Gets a commodity.
 *
 * @usage s = commodity.get( "Food" ) -- Gets the food commodity
 *
 *    @luatparam string s Raw (untranslated) name of the commodity to get.
 *    @luatreturn Commodity|nil The commodity matching name or nil if error.
 * @luafunc get
 */
static int commodityL_get( lua_State *L )
{
   /* Handle parameters. */
   const char *name = luaL_checkstring(L,1);

   /* Get commodity. */
   Commodity *commodity = commodity_get( name );
   if (commodity == NULL) {
      return NLUA_ERROR(L,_("Commodity '%s' not found!"), name);
   }

   /* Push. */
   lua_pushcommodity(L, commodity);
   return 1;
}

/**
 * @brief Gets the list of standard commodities.
 *
 *    @luatreturn table A table containing commodity objects, namely those which are standard (buyable/sellable anywhere).
 * @luafunc getStandard
 */
static int commodityL_getStandard( lua_State *L )
{
   /* Get commodity. */
   Commodity **standard = standard_commodities();
   /* Push. */
   lua_newtable( L );
   for (int i=0; i<array_size(standard); i++) {
      lua_pushcommodity( L, standard[i] );
      lua_rawseti( L, -2, i+1 );
   }
   array_free( standard );
   return 1;
}

/**
 * @brief Gets the flags that are set for a commodity.
 *
 *    @luatreturn table A table containing the flags as key and value as boolean.
 * @luafunc flags
 */
static int commodityL_flags( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   lua_newtable(L);

   lua_pushboolean(L, commodity_isFlag(c,COMMODITY_FLAG_STANDARD));
   lua_setfield(L, -2, "standard");

   lua_pushboolean(L, commodity_isFlag(c,COMMODITY_FLAG_ALWAYS_CAN_SELL));
   lua_setfield(L, -2, "always_can_sell");

   lua_pushboolean(L, commodity_isFlag(c,COMMODITY_FLAG_PRICE_CONSTANT));
   lua_setfield(L, -2, "price_constant");

   return 1;
}

/**
 * @brief Gets the translated name of the commodity.
 *
 * This translated name should be used for display purposes (e.g.
 * messages). It cannot be used as an identifier for the commodity; for
 * that, use commodity.nameRaw() instead.
 *
 * @usage commodityname = c:name() -- Equivalent to `_(c:nameRaw())`
 *
 *    @luatparam Commodity c Commodity to get the translated name of.
 *    @luatreturn string The translated name of the commodity.
 * @luafunc name
 */
static int commodityL_name( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   lua_pushstring(L, _(c->name));
   return 1;
}

/**
 * @brief Gets the raw (untranslated) name of the commodity.
 *
 * This untranslated name should be used for identification purposes
 * (e.g. can be passed to commodity.get()). It should not be used
 * directly for display purposes without manually translating it with
 * _().
 *
 * @usage commodityrawname = c:nameRaw()
 *
 *    @luatparam Commodity c Commodity to get the raw name of.
 *    @luatreturn string The raw name of the commodity.
 * @luafunc nameRaw
 */
static int commodityL_nameRaw( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   lua_pushstring(L, c->name);
   return 1;
}

/**
 * @brief Gets the base price of an commodity.
 *
 * @usage print( c:price() ) -- Prints the base price of the commodity
 *
 *    @luatparam Commodity c Commodity to get information of.
 *    @luatreturn number The base price of the commodity.
 * @luafunc price
 */
static int commodityL_price( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   lua_pushnumber(L, c->price);
   return 1;
}

/**
 * @brief Gets the base price of an commodity on a certain spob.
 *
 * @usage if c:priceAt( spob.get("Polaris Prime") ) > 100 then -- Checks price of a commodity at polaris prime
 *
 *    @luatparam Commodity c Commodity to get information of.
 *    @luatparam Spob p Spob to get price at.
 *    @luatreturn number The price of the commodity at the spob.
 * @luafunc priceAt
 */
static int commodityL_priceAt( lua_State *L )
{
   const Commodity *c;
   const Spob *p;
   const StarSystem *sys;
   const char *sysname;

   c = luaL_validcommodity(L,1);
   p = luaL_validspob(L,2);
   sysname = spob_getSystem( p->name );
   if (sysname == NULL)
      return NLUA_ERROR( L, _("Spob '%s' does not belong to a system."), p->name );
   sys = system_get( sysname );
   if (sys == NULL)
      return NLUA_ERROR( L, _("Spob '%s' can not find its system '%s'."), p->name, sysname );

   lua_pushnumber( L, spob_commodityPrice( p, c ) );
   return 1;
}

/**
 * @brief Gets the price of an commodity on a certain spob at a certain time.
 *
 * @usage if c:priceAtTime( spob.get("Polaris Prime"), time ) > 100 then -- Checks price of a commodity at polaris prime
 *
 *    @luatparam Commodity c Commodity to get information of.
 *    @luatparam Spob p Spob to get price at.
 *    @luatparam Time t Time to get the price at.
 *    @luatreturn number The price of the commodity at the spob.
 * @luafunc priceAtTime
 */
static int commodityL_priceAtTime( lua_State *L )
{
   const Commodity *c;
   const Spob *p;
   const StarSystem *sys;
   const char *sysname;
   ntime_t t;
   c = luaL_validcommodity(L,1);
   p = luaL_validspob(L,2);
   t = luaL_validtime(L, 3);
   sysname = spob_getSystem( p->name );
   if (sysname == NULL)
      return NLUA_ERROR( L, _("Spob '%s' does not belong to a system."), p->name );
   sys = system_get( sysname );
   if (sys == NULL)
      return NLUA_ERROR( L, _("Spob '%s' can not find its system '%s'."), p->name, sysname );

   lua_pushnumber( L, spob_commodityPriceAtTime( p, c, t ) );
   return 1;
}

static int spob_hasCommodity( const Commodity *c, const Spob *s )
{
   for (int i=0; i<array_size(s->commodities); i++) {
      const Commodity *sc = s->commodities[i];
      if (sc==c)
         return 1;
   }

   return 0;
}

/**
 * @brief Sees if a commodity can be sold at either a spob or system.
 *
 * It does not check faction standings, only if it is possible to sell at a spob or a system (checking all spobs in the system).
 *
 *    @luatparam Commodity c Commodity to check.
 *    @luatparam Spob|System where Either a spob or a system to see if the commodity can be sold there.
 * @luafunc canSell
 */
static int commodityL_canSell( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);

   if (commodity_isFlag( c, COMMODITY_FLAG_ALWAYS_CAN_SELL )) {
      lua_pushboolean(L,1);
      return 1;
   }

   if (lua_issystem(L,2)) {
      const StarSystem *s = luaL_validsystem(L,2);
      for (int i=0; i<array_size(s->spobs); i++) {
         if (spob_hasCommodity( c, s->spobs[i] )) {
            lua_pushboolean(L,1);
            return 1;
         }
      }
   }
   else {
      const Spob *s = luaL_validspob(L,2);
      lua_pushboolean(L, spob_hasCommodity( c, s ) );
      return 1;
   }

   lua_pushboolean(L,0);
   return 1;
}

/**
 * @brief Sees if a commodity can be bought at either a spob or system.
 *
 * It does not check faction standings, only if it is possible to buy at a spob or a system (checking all spobs in the system).
 *
 *    @luatparam Commodity c Commodity to check.
 *    @luatparam Spob|System where Either a spob or a system to see if the commodity is sold there.
 * @luafunc canBuy
 */
static int commodityL_canBuy( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);

   if (lua_issystem(L,2)) {
      const StarSystem *s = luaL_validsystem(L,2);
      for (int i=0; i<array_size(s->spobs); i++) {
         if (spob_hasCommodity( c, s->spobs[i] )) {
            lua_pushboolean(L,1);
            return 1;
         }
      }
   }
   else {
      const Spob *s = luaL_validspob(L,2);
      lua_pushboolean(L, spob_hasCommodity( c, s ) );
      return 1;
   }

   lua_pushboolean(L,0);
   return 1;
}

/**
 * @brief Gets the store icon of a commodity if it exists.
 *
 *    @luatparam Commodity c Commodity to get icon of.
 *    @luatreturn Texture|nil Texture of the store icon if exists, otherwise nil.
 * @luafunc icon
 */
static int commodityL_icon( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   if (c->gfx_store==NULL)
      return 0;
   lua_pushtex(L,gl_dupTexture(c->gfx_store));
   return 1;
}

/**
 * @brief Gets the description of a commodity if it exists.
 *
 *    @luatparam Commodity c Commodity to get decription of
 *    @luatreturn string|nil Description of the commodity if exists, otherwise nil.
 * @luafunc description
 */
static int commodityL_description( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   if (c->description==NULL)
      return 0;
   lua_pushstring(L,c->description);
   return 1;
}

/**
 * @brief Creates a new temporary commodity. If a temporary commodity with the same name exists, that gets returned instead.
 *        "Temporary" is a relative term. The commodity will be saved with the player while it is in the inventory of their
 *        fleet. However, when all instances are gone, it will no longer be saved and disappear.
 *
 * @usage commodity.new( N_("Cheesburgers"), N_("I can has cheezburger?") )
 *
 *    @luatparam string cargo Name of the cargo to add. This must not match a cargo name defined in commodity.xml.
 *    @luatparam string decription Description of the cargo to add.
 *    @luatparam[opt=nil] table params Table of named parameters. Currently supported is "gfx_space".
 *    @luatreturn Commodity The newly created commodity or an existing temporary commodity with the same name.
 * @luafunc new
 */
static int commodityL_new( lua_State *L )
{
   const char *cname, *cdesc;
   char str[STRMAX_SHORT];
   Commodity *cargo;

   /* Parameters. */
   cname    = luaL_checkstring(L,1);
   cdesc    = luaL_checkstring(L,2);

   cargo    = commodity_getW(cname);
   if ((cargo != NULL) && !cargo->istemp)
      return NLUA_ERROR(L,_("Trying to create new cargo '%s' that would shadow existing non-temporary cargo!"), cname);

   if (cargo==NULL)
      cargo = commodity_newTemp( cname, cdesc );

   if (!lua_isnoneornil(L,3)) {
      const char *buf;
      lua_getfield(L,3,"gfx_space");
      buf = luaL_optstring(L,-1,NULL);
      if (buf) {
         gl_freeTexture(cargo->gfx_space);
         snprintf( str, sizeof(str), COMMODITY_GFX_PATH"space/%s", buf );
         cargo->gfx_space = gl_newImage( str, 0 );
      }
   }

   lua_pushcommodity(L, cargo);
   return 1;
}

/**
 * @brief Makes a temporary commodity illegal to a faction.
 *
 *    @luatparam Commodity c Temporary commodity to make illegal to factions.
 *    @luatparam Faction|table f Faction or table of factions to make illegal to.
 * @luafunc illegalto
 */
static int commodityL_illegalto( lua_State *L )
{
   Commodity *c = luaL_validcommodity(L,1);
   if (lua_istable(L,2)) {
      lua_pushnil(L); /* nil */
      while (lua_next(L,-2) != 0) { /* k, v */
         int f = luaL_validfaction(L,-1);
         commodity_tempIllegalto( c, f );
         lua_pop(L,1); /* k */
      }
   }
   else {
      int f = luaL_validfaction(L,2);
      commodity_tempIllegalto( c, f );
   }
   return 0;
}

/**
 * @brief Gets the factions to which the commodity is illegal to.
 *
 *    @luatparam c Commodity to get illegality status of.
 *    @luatreturn table Table of all the factions the commodity is illegal to.
 * @luafunc illegality
 */
static int commodityL_illegality( lua_State *L )
{
   const Commodity *c = luaL_validcommodity(L,1);
   lua_newtable(L);
   for (int i=0; i<array_size(c->illegalto); i++) {
      lua_pushfaction( L, c->illegalto[i] );
      lua_rawseti( L, -2, i+1 );
   }
   return 1;
}
