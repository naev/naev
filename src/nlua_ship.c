/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_ship.c
 *
 * @brief Handles the Lua ship bindings.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_ship.h"

#include "array.h"
#include "nlua_canvas.h"
#include "nlua_faction.h"
#include "nlua_outfit.h"
#include "nlua_tex.h"
#include "nluadef.h"
#include "player.h"
#include "slots.h"

/*
 * Prototypes.
 */
static const ShipOutfitSlot *ship_outfitSlotFromID( const Ship *s, int id );

/* Ship metatable methods. */
static int shipL_eq( lua_State *L );
static int shipL_get( lua_State *L );
static int shipL_exists( lua_State *L );
static int shipL_getAll( lua_State *L );
static int shipL_name( lua_State *L );
static int shipL_nameRaw( lua_State *L );
static int shipL_baseType( lua_State *L );
static int shipL_inherits( lua_State *L );
static int shipL_class( lua_State *L );
static int shipL_classDisplay( lua_State *L );
static int shipL_faction( lua_State *L );
static int shipL_fabricator( lua_State *L );
static int shipL_crew( lua_State *L );
static int shipL_mass( lua_State *L );
static int shipL_armour( lua_State *L );
static int shipL_cargo( lua_State *L );
static int shipL_fuelConsumption( lua_State *L );
static int shipL_license( lua_State *L );
static int shipL_points( lua_State *L );
static int shipL_slots( lua_State *L );
static int shipL_getSlots( lua_State *L );
static int shipL_fitsSlot( lua_State *L );
static int shipL_CPU( lua_State *L );
static int shipL_gfxPath( lua_State *L );
static int shipL_gfxComm( lua_State *L );
static int shipL_gfxStore( lua_State *L );
static int shipL_gfx( lua_State *L );
static int shipL_dims( lua_State *L );
static int shipL_screenSize( lua_State *L );
static int shipL_price( lua_State *L );
static int shipL_time_mod( lua_State *L );
static int shipL_getSize( lua_State *L );
static int shipL_description( lua_State *L );
static int shipL_getShipStat( lua_State *L );
static int shipL_getShipStatDesc( lua_State *L );
static int shipL_known( lua_State *L );
static int shipL_tags( lua_State *L );
static int shipL_render( lua_State *L );

static const luaL_Reg shipL_methods[] = {
   { "__tostring", shipL_name },
   { "__eq", shipL_eq },
   { "get", shipL_get },
   { "exists", shipL_exists },
   { "getAll", shipL_getAll },
   { "name", shipL_name },
   { "nameRaw", shipL_nameRaw },
   { "baseType", shipL_baseType },
   { "inherits", shipL_inherits },
   { "class", shipL_class },
   { "classDisplay", shipL_classDisplay },
   { "faction", shipL_faction },
   { "fabricator", shipL_fabricator },
   { "crew", shipL_crew },
   { "mass", shipL_mass },
   { "armour", shipL_armour },
   { "cargo", shipL_cargo },
   { "fuelConsumption", shipL_fuelConsumption },
   { "license", shipL_license },
   { "points", shipL_points },
   { "slots", shipL_slots },
   { "getSlots", shipL_getSlots },
   { "fitsSlot", shipL_fitsSlot },
   { "cpu", shipL_CPU },
   { "price", shipL_price },
   { "time_mod", shipL_time_mod },
   { "size", shipL_getSize },
   { "gfxPath", shipL_gfxPath },
   { "gfxComm", shipL_gfxComm },
   { "gfxStore", shipL_gfxStore },
   { "gfx", shipL_gfx },
   { "dims", shipL_dims },
   { "screenSize", shipL_screenSize },
   { "description", shipL_description },
   { "shipstat", shipL_getShipStat },
   { "shipstatDesc", shipL_getShipStatDesc },
   { "known", shipL_known },
   { "tags", shipL_tags },
   { "render", shipL_render },
   { 0, 0 } }; /**< Ship metatable methods. */

/**
 * @brief Loads the ship library.
 *
 *    @param env Environment to load ship library into.
 *    @return 0 on success.
 */
int nlua_loadShip( nlua_env *env )
{
   nlua_register( env, SHIP_METATABLE, shipL_methods, 1 );
   return 0;
}

/**
 * @brief Lua bindings to interact with ships.
 *
 * This will allow you to create and manipulate ships in-game.
 *
 * An example would be:
 * @code
 * s = ship.get( "Empire Lancelot" ) -- Gets the ship
 * cpu_free = s:cpu() -- Gets the CPU
 * @endcode
 *
 * @luamod ship
 */
/**
 * @brief Gets ship at index.
 *
 *    @param L Lua state to get ship from.
 *    @param ind Index position to find the ship.
 *    @return Ship found at the index in the state.
 */
const Ship *lua_toship( lua_State *L, int ind )
{
   return *( (Ship **)lua_touserdata( L, ind ) );
}
/**
 * @brief Gets ship at index or raises error if there is no ship at index.
 *
 *    @param L Lua state to get ship from.
 *    @param ind Index position to find ship.
 *    @return Ship found at the index in the state.
 */
const Ship *luaL_checkship( lua_State *L, int ind )
{
   if ( lua_isship( L, ind ) )
      return lua_toship( L, ind );
   luaL_typerror( L, ind, SHIP_METATABLE );
   return NULL;
}
/**
 * @brief Makes sure the ship is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the ship to validate.
 *    @return The ship (doesn't return if fails - raises Lua error ).
 */
const Ship *luaL_validship( lua_State *L, int ind )
{
   const Ship *s;

   if ( lua_isship( L, ind ) )
      s = luaL_checkship( L, ind );
   else if ( lua_isstring( L, ind ) )
      s = ship_get( lua_tostring( L, ind ) );
   else {
      luaL_typerror( L, ind, SHIP_METATABLE );
      return NULL;
   }

   if ( s == NULL )
      NLUA_ERROR( L, _( "Ship is invalid." ) );

   return s;
}
/**
 * @brief Pushes a ship on the stack.
 *
 *    @param L Lua state to push ship into.
 *    @param ship Ship to push.
 *    @return Newly pushed ship.
 */
const Ship **lua_pushship( lua_State *L, const Ship *ship )
{
   const Ship **p;
   p  = (const Ship **)lua_newuserdata( L, sizeof( Ship * ) );
   *p = ship;
   luaL_getmetatable( L, SHIP_METATABLE );
   lua_setmetatable( L, -2 );
   return p;
}
/**
 * @brief Checks to see if ind is a ship.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a ship.
 */
int lua_isship( lua_State *L, int ind )
{
   int ret;

   if ( lua_getmetatable( L, ind ) == 0 )
      return 0;
   lua_getfield( L, LUA_REGISTRYINDEX, SHIP_METATABLE );

   ret = 0;
   if ( lua_rawequal( L, -1, -2 ) ) /* does it have the correct mt? */
      ret = 1;

   lua_pop( L, 2 ); /* remove both metatables */
   return ret;
}

/**
 * @brief Checks to see if two ships are the same.
 *
 * @usage if s1 == s2 then -- Checks to see if ship s1 and s2 are the same
 *
 *    @luatparam Ship s1 First ship to compare.
 *    @luatparam Ship s2 Second ship to compare.
 *    @luatreturn boolean true if both ships are the same.
 * @luafunc __eq
 */
static int shipL_eq( lua_State *L )
{
   const Ship *a, *b;
   a = luaL_checkship( L, 1 );
   b = luaL_checkship( L, 2 );
   if ( a == b )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Gets a ship.
 *
 * @usage s = ship.get( "Hyena" ) -- Gets the hyena
 *
 *    @luatparam string s Raw (untranslated) name of the ship to get.
 *    @luatreturn Ship The ship matching name or nil if error.
 * @luafunc get
 */
static int shipL_get( lua_State *L )
{
   const Ship *ship = luaL_validship( L, 1 );
   lua_pushship( L, ship );
   return 1;
}

/**
 * @brief Gets a ship if it exists, nil otherwise.
 *
 * Does not raise any warnings or errors if fails.
 *
 * @usage s = ship.exists( "Llama" ) -- Gets the Llama if it exsits
 *
 *    @luatparam string s Raw (untranslated) name of the ship to get.
 *    @luatreturn Ship|nil The ship matching name or nil if not found
 * @luafunc exists
 */
static int shipL_exists( lua_State *L )
{
   const Ship *o = NULL;
   if ( lua_isship( L, 1 ) )
      o = luaL_checkship( L, 1 );
   else if ( lua_isstring( L, 1 ) ) {
      const char *str = lua_tostring( L, 1 );
      o               = ship_getW( str );
   } else {
      luaL_typerror( L, 1, SHIP_METATABLE );
      return 0;
   }
   if ( o != NULL ) {
      lua_pushship( L, o );
      return 1;
   }
   return 0;
}

/**
 * @brief Gets a table containing all the ships.
 *
 *    @luatreturn table A table containing all the ships in the game.
 * @luafunc getAll
 */
static int shipL_getAll( lua_State *L )
{
   const Ship *ships = ship_getAll();
   lua_newtable( L ); /* t */
   for ( int i = 0; i < array_size( ships ); i++ ) {
      lua_pushship( L, &ships[i] );
      lua_rawseti( L, -2, i + 1 );
   }
   return 1;
}

/**
 * @brief Gets the translated name of the ship.
 *
 * This translated name should be used for display purposes (e.g.
 * messages). It cannot be used as an identifier for the ship; for
 * that, use ship.nameRaw() instead.
 *
 * @usage shipname = s:name() -- Equivalent to `_(s:nameRaw())`
 *
 *    @luatparam Ship s Ship to get the translated name of.
 *    @luatreturn string The translated name of the ship.
 * @luafunc name
 */
static int shipL_name( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, ship_name( s ) );
   return 1;
}

/**
 * @brief Gets the raw (untranslated) name of the ship.
 *
 * This untranslated name should be used for identification purposes
 * (e.g. can be passed to ship.get()). It should not be used directly
 * for display purposes without manually translating it with _().
 *
 * @usage shipname = s:nameRaw()
 *
 *    @luatparam Ship s Ship to get the raw name of.
 *    @luatreturn string The raw name of the ship.
 * @luafunc nameRaw
 */
static int shipL_nameRaw( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, s->name );
   return 1;
}

/**
 * @brief Gets the raw (untranslated) name of the ship's base type.
 *
 * For example "Empire Lancelot" and "Lancelot" are both of the base type
 * "Lancelot".
 *
 * @usage type = s:baseType()
 *
 *    @luatparam Ship s Ship to get the ship base type of.
 *    @luatreturn string The raw name of the ship base type.
 * @luafunc baseType
 */
static int shipL_baseType( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, s->base_type );
   return 1;
}

/**
 * @brief Gets the ship it inherits stats from if applicable.
 *
 *    @luatparam Ship s Ship to get the ship it is inheriting from.
 *    @luatreturn Ship|nil The ship it is inheriting from or nil if not
 * applicable.
 * @luafunc inherits
 */
static int shipL_inherits( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   if ( s->inherits == NULL )
      return 0;
   lua_pushship( L, ship_get( s->inherits ) );
   return 1;
}

/**
 * @brief Gets the raw (untranslated) name of the ship's class.
 *
 * @usage shipclass = s:class()
 *
 *    @luatparam Ship s Ship to get ship class name of.
 *    @luatreturn string The raw name of the ship's class.
 * @luafunc class
 */
static int shipL_class( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, ship_class( s ) );
   return 1;
}

/**
 * @brief Gets the raw (untranslated) display name of the ship's class (not
 * ship's base class).
 *
 * @usage shipclass = s:classDisplay()
 *
 *    @luatparam Ship s Ship to get ship display class name of.
 *    @luatreturn string The raw name of the ship's display class.
 * @luafunc classDisplay
 */
static int shipL_classDisplay( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, ship_classDisplay( s ) );
   return 1;
}

/**
 * @brief Gets the faction of a ship.
 *
 * @usage shipclass = s:faction()
 *
 *    @luatparam Ship s Ship to get faction of.
 *    @luatreturn Faction The faction of the ship, or nil if not applicable.
 * @luafunc faction
 */
static int shipL_faction( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   if ( faction_isFaction( s->faction ) ) {
      lua_pushfaction( L, s->faction );
      return 1;
   }
   return 0;
}

/**
 * @brief Gets the raw (untranslated) fabricator of the ship.
 *
 *    @luatparam Ship s Ship to get fabricator of.
 *    @luatreturn string The raw name of the ship's fabricator.
 * @luafunc fabricator
 */
static int shipL_fabricator( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, s->fabricator );
   return 1;
}

/**
 * @brief Gets the number of crew of the ship.
 *
 *    @luatparam Ship s Ship to get crew of.
 *    @luatreturn integer The number of crew the ship has.
 * @luafunc crew
 */
static int shipL_crew( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushinteger( L, s->crew );
   return 1;
}

/**
 * @brief Gets the number of mass of the ship.
 *
 *    @luatparam Ship s Ship to get mass of.
 *    @luatreturn integer The number of mass the ship has.
 * @luafunc mass
 */
static int shipL_mass( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->mass );
   return 1;
}

/**
 * @brief Gets the number of armour of the ship.
 *
 *    @luatparam Ship s Ship to get armour of.
 *    @luatreturn integer The number of armour the ship has.
 * @luafunc armour
 */
static int shipL_armour( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->armour );
   return 1;
}

/**
 * @brief Gets the number of cargo of the ship.
 *
 *    @luatparam Ship s Ship to get cargo of.
 *    @luatreturn integer The number of cargo the ship has.
 * @luafunc cargo
 */
static int shipL_cargo( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->cap_cargo );
   return 1;
}

/**
 * @brief Gets the number of fuel consumption of the ship.
 *
 *    @luatparam Ship s Ship to get fuel consumption of.
 *    @luatreturn integer The number of fuel consumption of the ship.
 * @luafunc fuelConsumption
 */
static int shipL_fuelConsumption( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->fuel_consumption );
   return 1;
}

/**
 * @brief Gets licence required for the ship.
 *
 * @usage licence = s:licence()
 *
 *    @luatparam Ship s Ship to get licence of.
 *    @luatreturn string Required licence or nil if not necessary.
 * @luafunc licence
 */
static int shipL_license( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   if ( s->license == NULL )
      return 0;
   lua_pushstring( L, s->license );
   return 1;
}

/**
 * @brief Gets the point value of a ship. Used for comparing relative ship
 * strengths (minus outfits).
 *
 * @usage points = s:points()
 *
 *    @luatparam Ship s Ship to get points of.
 *    @luatreturn number Point value of the ship.
 * @luafunc points
 */
static int shipL_points( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushinteger( L, s->points );
   return 1;
}

/**
 * @brief Gets the amount of the ship's slots.
 *
 * @usage slots_weapon, slots_utility, slots_structure = p:slots()
 *
 *    @luatparam Ship s Ship to get ship slots of.
 *    @luatreturn number Number of weapon slots.
 *    @luatreturn number Number of utility slots.
 *    @luatreturn number Number of structure slots.
 * @luafunc slots
 */
static int shipL_slots( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   /* Push slot numbers. */
   lua_pushnumber( L, array_size( s->outfit_weapon ) );
   lua_pushnumber( L, array_size( s->outfit_utility ) );
   lua_pushnumber( L, array_size( s->outfit_structure ) );
   return 3;
}

/**
 * @brief Get a table of slots of a ship, where a slot is a table with a string
 * size, type, and property
 *
 * @usage for i, v in ipairs( ship.getSlots( ship.get("Llama") ) ) do
 * print(v["type"]) end
 *
 *    @luatparam Ship s Ship to get slots of
 *    @luatparam[opt=false] boolean ignore_locked Whether or not to ignore
 * locked slots.
 *    @luareturn A table of tables with slot properties string `size`, string
 * `type`, string `property`, boolean `required`, boolean `exclusive`, boolean
 * `locked`, and (if applicable) outfit `outfit` (Strings are English.)
 * @luafunc getSlots
 */
static int shipL_getSlots( lua_State *L )
{
   int                   k;
   const Ship           *s               = luaL_validship( L, 1 );
   int                   ignore_locked   = lua_toboolean( L, 2 );
   const ShipOutfitSlot *outfit_arrays[] = {
      s->outfit_structure, s->outfit_utility, s->outfit_weapon };

   lua_newtable( L );
   k = 1;
   for ( int i = 0; i < 3; i++ ) {
      for ( int j = 0; j < array_size( outfit_arrays[i] ); j++ ) {
         const OutfitSlot     *slot  = &outfit_arrays[i][j].slot;
         const ShipOutfitSlot *sslot = &outfit_arrays[i][j];

         /* Skip locked if necessary. */
         if ( ignore_locked && sslot->locked )
            continue;

         /* make the slot table and put it in */
         lua_newtable( L );

         /* Index can be used as an ID (at least for now...) */
#if 0
         lua_pushstring(L, "id" ); /* key */
         lua_pushinteger(L, k); /* value */
         lua_rawset(L, -3); /* table[key] = value*/
#endif

         lua_pushstring( L, slotName( slot->type ) ); /* value */
         lua_setfield( L, -2, "type" );               /* table[key] = value*/

         lua_pushstring( L, slotSize( slot->size ) );
         lua_setfield( L, -2, "size" ); /* table[key] = value */

         lua_pushstring( L, sp_display( slot->spid ) ); /* value */
         lua_setfield( L, -2, "property" );             /* table[key] = value */

         lua_pushboolean( L, sslot->required ); /* value */
         lua_setfield( L, -2, "required" );     /* table[key] = value */

         lua_pushboolean( L, sslot->exclusive ); /* value */
         lua_setfield( L, -2, "exclusive" );     /* table[key] = value */

         lua_pushboolean( L, sslot->locked ); /* value */
         lua_setfield( L, -2, "locked" );     /* table[key] = value */

         lua_pushboolean( L, sslot->visible ); /* value */
         lua_setfield( L, -2, "visible" );     /* table[key] = value */

         lua_newtable( L );
         const char **tags = sp_tags( slot->spid );
         for ( int t = 0; t < array_size( tags ); t++ ) {
            lua_pushboolean( L, 1 );
            lua_setfield( L, -2, tags[t] );
         }
         lua_setfield( L, -2, "tags" );

         if ( sslot->data != NULL ) {
            lua_pushoutfit( L, sslot->data ); /* value*/
            lua_setfield( L, -2, "outfit" );  /* table[key] = value */
         }

         lua_rawseti( L, -2, k++ ); /* put the slot table in */
      }
   }

   return 1;
}

/**
 * @brief Gets an outfit slot from ID.
 *
 *    @param s Ship to get slot of.
 *    @param id ID to get slot of.
 */
static const ShipOutfitSlot *ship_outfitSlotFromID( const Ship *s, int id )
{
   const ShipOutfitSlot *outfit_arrays[] = {
      s->outfit_structure, s->outfit_utility, s->outfit_weapon };

   for ( int i = 0; i < 3; i++ ) {
      int n = array_size( outfit_arrays[i] );
      if ( id <= n )
         return &outfit_arrays[i][id - 1];
      id -= n;
   }
   return NULL;
}

/**
 * @brief Checks to see if an outfit fits a ship slot.
 *
 *    @luatparam Ship s Ship to check.
 *    @luatparam number id ID of the slot to check (index in `getSlots()`
 * table).
 *    @luatparam Outfit o Outfit to check to see if it fits in the slot.
 *    @luatreturn boolean Whether or not the outfit fits the slot.
 * @luafunc fitsSlot
 */
static int shipL_fitsSlot( lua_State *L )
{
   const Ship           *s  = luaL_validship( L, 1 );
   int                   id = luaL_checkinteger( L, 2 );
   const Outfit         *o  = luaL_validoutfit( L, 3 );
   const ShipOutfitSlot *ss = ship_outfitSlotFromID( s, id );
   if ( ss->locked ) {
      lua_pushboolean( L, 0 );
      return 1;
   }
   lua_pushboolean( L, outfit_fitsSlot( o, &ss->slot ) );
   return 1;
}

/**
 * @brief Gets the ship available CPU.
 *
 * @usage cpu_left = s:cpu()
 *
 *    @luatparam Ship s Ship to get available CPU of.
 *    @luatreturn number The CPU available on the ship.
 * @luafunc cpu
 */
static int shipL_CPU( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->cpu );
   return 1;
}

/**
 * @brief Gets the ship's price, with and without default outfits.
 *
 * @usage price, base = s:price()
 *
 *    @luatparam Ship s Ship to get the price of.
 *    @luatreturn number The ship's final purchase price.
 *    @luatreturn number The ship's base price.
 * @luafunc price
 */
static int shipL_price( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, ship_buyPrice( s ) );
   lua_pushnumber( L, ship_basePrice( s ) );
   return 2;
}

/**
 * @brief Gets the ship's time_mod.
 *
 *    @luatparam Ship s Ship to get the time_mod of.
 *    @luatreturn number The ship's time_mod.
 * @luafunc time_mod
 */
static int shipL_time_mod( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->dt_default );
   return 1;
}

/**
 * @brief Gets the ship's size. Ultra-light is 1, light is 2, medium is 3,
 * heavy-medium is 4, heavy is 5, and super-heavy is 6.
 *
 *    @luatparam Ship s Ship to get the size of.
 *    @luatreturn number The ship's size.
 * @luafunc size
 */
static int shipL_getSize( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushinteger( L, ship_size( s ) );
   return 1;
}

/**
 * @brief Gets the path where the ship's graphics are located. Useful for seeing
 * if two ships share the same graphics.
 *
 *    @luatparam Ship s Ship to get the path of the graphics of.
 *    @luatreturn string The path to the ship graphics.
 * @luafunc gfxPath
 */
static int shipL_gfxPath( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, s->gfx_path );
   return 1;
}

/**
 * @brief Gets the ship's comm graphics.
 *
 * Will not work without access to the Tex module.
 *
 * @usage gfx = s:gfxComm()
 *
 *    @luatparam Ship s Ship to get comm graphics of.
 *    @luatparam[opt=512] number resolution Resolution to render the image at.
 *    @luatreturn Tex The comm graphics of the ship.
 * @luafunc gfxComm
 */
static int shipL_gfxComm( lua_State *L )
{
   const Ship *s    = luaL_validship( L, 1 );
   int         size = luaL_optinteger( L, 2, 512 );
   glTexture  *tex  = ship_gfxComm( s, size, 0., 0., &L_store_const );
   if ( tex == NULL ) {
      NLUA_WARN( L, _( "Unable to get ship comm graphic for '%s'." ), s->name );
      return 0;
   }
   lua_pushtex( L, tex );
   return 1;
}

/**
 * @brief Gets the ship's store graphics.
 *
 * Will not work without access to the Tex module.
 *
 * @usage gfx = s:gfxStore()
 *
 *    @luatparam Ship s Ship to get store graphics of.
 *    @luatreturn Tex The store graphics of the ship.
 * @luafunc gfxStore
 */
static int shipL_gfxStore( lua_State *L )
{
   const Ship *s   = luaL_validship( L, 1 );
   glTexture  *tex = ship_gfxStore( s, 256, 0., 0., 0. );
   if ( tex == NULL ) {
      NLUA_WARN( L, _( "Unable to get ship store graphic for '%s'." ),
                 s->name );
      return 0;
   }
   lua_pushtex( L, tex );
   return 1;
}

/**
 * @brief Gets the ship's graphics.
 *
 * Will not work without access to the Tex module. These are nearly always a
 * sprite sheet.
 *
 * @usage gfx = s:gfx()
 *
 *    @luatparam Ship s Ship to get graphics of.
 *    @luatreturn Tex The graphics of the ship.
 * @luafunc gfx
 */
static int shipL_gfx( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   ship_gfxLoad( (Ship *)s );
   glTexture *tex = gl_dupTexture( s->gfx_space );
   if ( tex == NULL ) {
      NLUA_WARN( L, _( "Unable to get ship graphic for '%s'." ), s->name );
      return 0;
   }
   lua_pushtex( L, tex );
   return 1;
}

/**
 * @brief Gets the onscreen dimensions of the ship.
 *
 *    @luatreturn number Width of the ship.
 *    @luatreturn number Height of the ship.
 * @luafunc dims
 */
static int shipL_dims( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->size );
   lua_pushnumber( L, s->size );
   return 2;
}

/**
 * @brief Gets the onscreen size of the ship.
 *
 *    @luatreturn number Size of the ship.
 * @luafunc screenSize
 */
static int shipL_screenSize( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushnumber( L, s->size );
   return 1;
}

/**
 * @brief Gets the description of the ship (translated).
 *
 * @usage description = s:description()
 *
 *    @luatparam Ship s Ship to get the description of.
 *    @luatreturn string The description of the ship.
 * @luafunc description
 */
static int shipL_description( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   lua_pushstring( L, _( s->description ) );
   return 1;
}

/**
 * @brief Gets a shipstat from an Ship by name, or a table containing all the
 * ship stats if not specified.
 *
 *    @luatparam Ship s Ship to get ship stat of.
 *    @luatparam[opt=nil] string name Name of the ship stat to get.
 *    @luatparam[opt=false] boolean internal Whether or not to use the internal
 * representation.
 *    @luatreturn number|table Value of the ship stat or a tale containing all
 * the ship stats if name is not specified.
 * @luafunc shipstat
 */
static int shipL_getShipStat( lua_State *L )
{
   const Ship *s        = luaL_validship( L, 1 );
   const char *str      = luaL_optstring( L, 2, NULL );
   int         internal = lua_toboolean( L, 3 );
   ss_statsGetLua( L, &s->stats_array, str, internal );
   return 1;
}

/**
 * @brief Gets the ship stats description for a ship.
 *
 *    @luatparam Ship s Ship to get ship stat description of.
 *    @luatreturn string Description of the ship's stats.
 * @luafunc shipstatDesc
 */
static int shipL_getShipStatDesc( lua_State *L )
{
   char        buf[STRMAX];
   const Ship *s = luaL_validship( L, 1 );
   ss_statsDesc( &s->stats_array, buf, sizeof( buf ), 0 );
   lua_pushstring( L, buf );
   return 1;
}

/**
 * @brief Gets whether or not the ship is known to the player, as in they know a
 * spob that sells it or own it.
 */
static int shipL_known( lua_State *L )
{
   /* TODO cache this and mark as dirty instead of recomputing for each outfit.
    */
   const Ship         *s  = luaL_validship( L, 1 );
   const PlayerShip_t *ps = player_getShipStack();
   for ( int i = 0; i < array_size( ps ); i++ ) {
      if ( ps[i].p->ship == s ) {
         lua_pushboolean( L, 1 );
         return 1;
      }
   }
   const Spob *ss = spob_getAll();
   for ( int i = 0; i < array_size( ss ); i++ ) {
      const Spob *spb = &ss[i];
      if ( !spob_hasService( spb, SPOB_SERVICE_SHIPYARD ) )
         continue;
      if ( !spob_isKnown( spb ) )
         continue;
      if ( tech_hasShip( spb->tech, s, 1 ) ) {
         lua_pushboolean( L, 1 );
         return 1;
      }
   }
   lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Gets the ship tags.
 *
 * @usage if s:tags()["fancy"] then -- Has the "fancy" tag
 * @usage if s:tags("fancy") then -- Has the "fancy" tag
 *
 *    @luatparam Ship s Ship to get tags of.
 *    @luatparam[opt=nil] string tag Tag to check if exists.
 *    @luatreturn table|boolean Table of tags where the name is the key and true
 * is the value or a boolean value if a string is passed as the second parameter
 * indicating whether or not the specified tag exists.
 * @luafunc tags
 */
static int shipL_tags( lua_State *L )
{
   const Ship *s = luaL_validship( L, 1 );
   return nlua_helperTags( L, 2, s->tags );
}

/**
 * @brief Renders the pilot to a canvas
 *
 *    @luatparam Ship s Ship to render on the screen.
 *    @luatparam number dir Direction the ship should be facing (in radians).
 *    @luatparam number engineglow How much engine glow to render with.
 *    @luatparam number tilt How much to tilt the ship (in radians).
 *    @luatreturn Canvas The canvas with the pilot drawn on it.
 * @luafunc render
 */
static int shipL_render( lua_State *L )
{
   LuaCanvas_t lc;
   int         w, h, sx, sy;
   const Ship *s    = luaL_validship( L, 1 );
   double      dir  = luaL_checknumber( L, 2 );
   double      eg   = luaL_optnumber( L, 3, 0. );
   double      tilt = luaL_optnumber( L, 4, 0. );

   ship_gfxLoad( (Ship *)s );

   gl_getSpriteFromDir( &sx, &sy, s->sx, s->sy, dir );

   w = s->size;
   h = s->size;
   if ( canvas_new( &lc, w, h ) )
      return NLUA_ERROR( L, _( "Error setting up framebuffer!" ) );

   /* The code path below is really buggy.
    * 1. engine_glow seems to scale 3D models improperly when interpolating, so
    * it's disabled.
    * 2. for some reason, have to pass real dimensions and not fbo dimensions.
    * TODO fix this shit. */
   ship_renderFramebuffer( s, lc.fbo, gl_screen.rw, gl_screen.rh, dir, eg, tilt,
                           0., sx, sy, NULL, NULL );

   lua_pushcanvas( L, lc );
   return 1;
}
