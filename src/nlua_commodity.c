/*
 * See Licensing and Copyright notice in naev.h
 */

/*
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

static int spob_hasCommodity( CommodityRef c, const Spob *s )
{
   for ( int i = 0; i < array_size( s->commodities ); i++ ) {
      CommodityRef sc = s->commodities[i];
      if ( sc == c )
         return 1;
   }

   return 0;
}

/*
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

/*
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
