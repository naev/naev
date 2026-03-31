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
#include "ndata.h"
#include "nlua_faction.h"
#include "nlua_spob.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_time.h"
#include "nluadef.h"

/* Commodity metatable methods. */
static int commodityL_eq( lua_State *L );
static int commodityL_get( lua_State *L );
static int commodityL_getAll( lua_State *L );
static int commodityL_exists( lua_State *L );
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
static int commodityL_tags( lua_State *L );

static const luaL_Reg commodityL_methods[] = {
   { "__tostring", commodityL_name },
   { "__eq", commodityL_eq },
   { "get", commodityL_get },
   { "getAll", commodityL_getAll },
   { "exists", commodityL_exists },
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
   { "tags", commodityL_tags },
   { 0, 0 } }; /**< Commodity metatable methods. */

/**
 * @brief Gets the flags that are set for a commodity.
 *
 *    @luatreturn table A table containing the flags as key and value as
 * boolean.
 * @luafunc flags
 */
static int commodityL_flags( lua_State *L )
{
   CommodityRef c = luaL_validcommodity( L, 1 );
   lua_newtable( L );

   lua_pushboolean( L, commodity_isFlag( c, COMMODITY_FLAG_STANDARD ) );
   lua_setfield( L, -2, "standard" );

   lua_pushboolean( L, commodity_isFlag( c, COMMODITY_FLAG_ALWAYS_CAN_SELL ) );
   lua_setfield( L, -2, "always_can_sell" );

   lua_pushboolean( L, commodity_isFlag( c, COMMODITY_FLAG_PRICE_CONSTANT ) );
   lua_setfield( L, -2, "price_constant" );

   return 1;
}

/**
 * @brief Gets the base price of an commodity on a certain spob.
 *
 * @usage if c:priceAt( spob.get("Polaris Prime") ) > 100 then -- Checks price
 * of a commodity at Polaris Prime
 *
 *    @luatparam Commodity c Commodity to get information of.
 *    @luatparam Spob p Spob to get price at.
 *    @luatreturn number The price of the commodity at the spob.
 * @luafunc priceAt
 */
static int commodityL_priceAt( lua_State *L )
{
   CommodityRef      c;
   const Spob       *p;
   const StarSystem *sys;
   const char       *sysname;

   c       = luaL_validcommodity( L, 1 );
   p       = luaL_validspob( L, 2 );
   sysname = spob_getSystemName( p->name );
   if ( sysname == NULL )
      return NLUA_ERROR( L, _( "Spob '%s' does not belong to a system." ),
                         p->name );
   sys = system_get( sysname );
   if ( sys == NULL )
      return NLUA_ERROR( L, _( "Spob '%s' can not find its system '%s'." ),
                         p->name, sysname );

   lua_pushnumber( L, spob_commodityPrice( p, c ) );
   return 1;
}

/**
 * @brief Gets the price of an commodity on a certain spob at a certain time.
 *
 * @usage if c:priceAtTime( spob.get("Polaris Prime"), time ) > 100 then --
 * Checks price of a commodity at Polaris Prime
 *
 *    @luatparam Commodity c Commodity to get information of.
 *    @luatparam Spob p Spob to get price at.
 *    @luatparam Time t Time to get the price at.
 *    @luatreturn number The price of the commodity at the spob.
 * @luafunc priceAtTime
 */
static int commodityL_priceAtTime( lua_State *L )
{
   CommodityRef      c;
   const Spob       *p;
   const StarSystem *sys;
   const char       *sysname;
   ntime_t           t;
   c       = luaL_validcommodity( L, 1 );
   p       = luaL_validspob( L, 2 );
   t       = luaL_validtime( L, 3 );
   sysname = spob_getSystemName( p->name );
   if ( sysname == NULL )
      return NLUA_ERROR( L, _( "Spob '%s' does not belong to a system." ),
                         p->name );
   sys = system_get( sysname );
   if ( sys == NULL )
      return NLUA_ERROR( L, _( "Spob '%s' can not find its system '%s'." ),
                         p->name, sysname );

   lua_pushnumber( L, spob_commodityPriceAtTime( p, c, t ) );
   return 1;
}

static int spob_hasCommodity( CommodityRef c, const Spob *s )
{
   for ( int i = 0; i < array_size( s->commodities ); i++ ) {
      CommodityRef sc = s->commodities[i];
      if ( sc == c )
         return 1;
   }

   return 0;
}

/**
 * @brief Sees if a commodity can be sold at either a spob or system.
 *
 * It does not check faction standings, only if it is possible to sell at a spob
 * or a system (checking all spobs in the system).
 *
 *    @luatparam Commodity c Commodity to check.
 *    @luatparam Spob|System where Either a spob or a system to see if the
 * commodity can be sold there.
 * @luafunc canSell
 */
static int commodityL_canSell( lua_State *L )
{
   CommodityRef c = luaL_validcommodity( L, 1 );

   if ( commodity_isFlag( c, COMMODITY_FLAG_ALWAYS_CAN_SELL ) ) {
      lua_pushboolean( L, 1 );
      return 1;
   }

   if ( lua_issystem( L, 2 ) ) {
      const StarSystem *s = luaL_validsystem( L, 2 );
      for ( int i = 0; i < array_size( s->spobs ); i++ ) {
         if ( spob_hasCommodity( c, s->spobs[i] ) ) {
            lua_pushboolean( L, 1 );
            return 1;
         }
      }
   } else {
      const Spob *s = luaL_validspob( L, 2 );
      lua_pushboolean( L, spob_hasCommodity( c, s ) );
      return 1;
   }

   lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Sees if a commodity can be bought at either a spob or system.
 *
 * It does not check faction standings, only if it is possible to buy at a spob
 * or a system (checking all spobs in the system).
 *
 *    @luatparam Commodity c Commodity to check.
 *    @luatparam Spob|System where Either a spob or a system to see if the
 * commodity is sold there.
 * @luafunc canBuy
 */
static int commodityL_canBuy( lua_State *L )
{
   CommodityRef c = luaL_validcommodity( L, 1 );

   if ( lua_issystem( L, 2 ) ) {
      const StarSystem *s = luaL_validsystem( L, 2 );
      for ( int i = 0; i < array_size( s->spobs ); i++ ) {
         if ( spob_hasCommodity( c, s->spobs[i] ) ) {
            lua_pushboolean( L, 1 );
            return 1;
         }
      }
   } else {
      const Spob *s = luaL_validspob( L, 2 );
      lua_pushboolean( L, spob_hasCommodity( c, s ) );
      return 1;
   }

   lua_pushboolean( L, 0 );
   return 1;
}
