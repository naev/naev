/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_outfit.c
 *
 * @brief Handles the Lua outfit bindings.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_outfit.h"

#include "array.h"
#include "damagetype.h"
#include "nlua_faction.h"
#include "nlua_pilot.h"
#include "nlua_ship.h"
#include "nlua_tex.h"
#include "nluadef.h"
#include "player.h"
#include "slots.h"

/* Outfit metatable methods. */
static int outfitL_eq( lua_State *L );
static int outfitL_get( lua_State *L );
static int outfitL_exists( lua_State *L );
static int outfitL_getAll( lua_State *L );
static int outfitL_name( lua_State *L );
static int outfitL_nameRaw( lua_State *L );
static int outfitL_shortname( lua_State *L );
static int outfitL_type( lua_State *L );
static int outfitL_typeBroad( lua_State *L );
static int outfitL_cpu( lua_State *L );
static int outfitL_mass( lua_State *L );
static int outfitL_slot( lua_State *L );
static int outfitL_slotExtra( lua_State *L );
static int outfitL_limit( lua_State *L );
static int outfitL_icon( lua_State *L );
static int outfitL_license( lua_State *L );
static int outfitL_price( lua_State *L );
static int outfitL_description( lua_State *L );
static int outfitL_summary( lua_State *L );
static int outfitL_unique( lua_State *L );
static int outfitL_friendlyfire( lua_State *L );
static int outfitL_pointdefense( lua_State *L );
static int outfitL_miss_ships( lua_State *L );
static int outfitL_miss_asteroids( lua_State *L );
static int outfitL_toggleable( lua_State *L );
static int outfitL_getShipStat( lua_State *L );
static int outfitL_weapStats( lua_State *L );
static int outfitL_specificStats( lua_State *L );
static int outfitL_illegality( lua_State *L );
static int outfitL_known( lua_State *L );
static int outfitL_tags( lua_State *L );

static const luaL_Reg outfitL_methods[] = {
   { "__tostring", outfitL_name },
   { "__eq", outfitL_eq },
   { "get", outfitL_get },
   { "exists", outfitL_exists },
   { "getAll", outfitL_getAll },
   { "name", outfitL_name },
   { "nameRaw", outfitL_nameRaw },
   { "shortname", outfitL_shortname },
   { "type", outfitL_type },
   { "typeBroad", outfitL_typeBroad },
   { "cpu", outfitL_cpu },
   { "mass", outfitL_mass },
   { "slot", outfitL_slot },
   { "slotExtra", outfitL_slotExtra },
   { "limit", outfitL_limit },
   { "icon", outfitL_icon },
   { "license", outfitL_license },
   { "licence", outfitL_license },
   { "price", outfitL_price },
   { "description", outfitL_description },
   { "summary", outfitL_summary },
   { "unique", outfitL_unique },
   { "friendlyfire", outfitL_friendlyfire },
   { "pointdefence", outfitL_pointdefense },
   { "pointdefense", outfitL_pointdefense },
   { "missShips", outfitL_miss_ships },
   { "missAsteroids", outfitL_miss_asteroids },
   { "toggleable", outfitL_toggleable },
   { "shipstat", outfitL_getShipStat },
   { "weapstats", outfitL_weapStats },
   { "specificstats", outfitL_specificStats },
   { "illegality", outfitL_illegality },
   { "known", outfitL_known },
   { "tags", outfitL_tags },
   { 0, 0 } }; /**< Outfit metatable methods. */

/**
 * @brief Loads the outfit library.
 *
 *    @param env Environment to load outfit library into.
 *    @return 0 on success.
 */
int nlua_loadOutfit( nlua_env *env )
{
   nlua_register( env, OUTFIT_METATABLE, outfitL_methods, 1 );
   return 0;
}

/**
 * @brief Lua bindings to interact with outfits.
 *
 * This will allow you to create and manipulate outfits in-game.
 *
 * An example would be:
 * @code
 * o = outfit.get( "Heavy Laser" ) -- Gets the outfit by name
 * cpu_usage = o:cpu() -- Gets the cpu usage of the outfit
 * slot_name, slot_size = o:slot() -- Gets slot information about the outfit
 * @endcode
 *
 * @luamod outfit
 */
/**
 * @brief Gets outfit at index.
 *
 *    @param L Lua state to get outfit from.
 *    @param ind Index position to find the outfit.
 *    @return Outfit found at the index in the state.
 */
const Outfit *lua_tooutfit( lua_State *L, int ind )
{
   return *( (const Outfit **)lua_touserdata( L, ind ) );
}
/**
 * @brief Gets outfit at index or raises error if there is no outfit at index.
 *
 *    @param L Lua state to get outfit from.
 *    @param ind Index position to find outfit.
 *    @return Outfit found at the index in the state.
 */
const Outfit *luaL_checkoutfit( lua_State *L, int ind )
{
   if ( lua_isoutfit( L, ind ) )
      return lua_tooutfit( L, ind );
   luaL_typerror( L, ind, OUTFIT_METATABLE );
   return NULL;
}
/**
 * @brief Makes sure the outfit is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the outfit to validate.
 *    @return The outfit (doesn't return if fails - raises Lua error ).
 */
const Outfit *luaL_validoutfit( lua_State *L, int ind )
{
   const Outfit *o;

   if ( lua_isoutfit( L, ind ) )
      o = luaL_checkoutfit( L, ind );
   else if ( lua_isstring( L, ind ) )
      o = outfit_get( lua_tostring( L, ind ) );
   else {
      luaL_typerror( L, ind, OUTFIT_METATABLE );
      return NULL;
   }

   if ( o == NULL )
      NLUA_ERROR( L, _( "Outfit is invalid." ) );

   return o;
}
/**
 * @brief Pushes a outfit on the stack.
 *
 *    @param L Lua state to push outfit into.
 *    @param outfit Outfit to push.
 *    @return Newly pushed outfit.
 */
const Outfit **lua_pushoutfit( lua_State *L, const Outfit *outfit )
{
   const Outfit **o = (const Outfit **)lua_newuserdata( L, sizeof( Outfit * ) );
   *o               = outfit;
   luaL_getmetatable( L, OUTFIT_METATABLE );
   lua_setmetatable( L, -2 );
   return o;
}
/**
 * @brief Checks to see if ind is a outfit.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a outfit.
 */
int lua_isoutfit( lua_State *L, int ind )
{
   int ret;

   if ( lua_getmetatable( L, ind ) == 0 )
      return 0;
   lua_getfield( L, LUA_REGISTRYINDEX, OUTFIT_METATABLE );

   ret = 0;
   if ( lua_rawequal( L, -1, -2 ) ) /* does it have the correct mt? */
      ret = 1;

   lua_pop( L, 2 ); /* remove both metatables */
   return ret;
}

/**
 * @brief Checks to see if two outfits are the same.
 *
 * @usage if o1 == o2 then -- Checks to see if outfit o1 and o2 are the same
 *
 *    @luatparam Outfit o1 First outfit to compare.
 *    @luatparam Outfit o2 Second outfit to compare.
 *    @luatreturn boolean true if both outfits are the same.
 * @luafunc __eq
 */
static int outfitL_eq( lua_State *L )
{
   const Outfit *a, *b;
   a = luaL_checkoutfit( L, 1 );
   b = luaL_checkoutfit( L, 2 );
   if ( a == b )
      lua_pushboolean( L, 1 );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Gets a outfit
 *
 * Will raise an error if fails.
 *
 * @usage s = outfit.get( "Heavy Laser" ) -- Gets the heavy laser
 *
 *    @luatparam string s Raw (untranslated) name of the outfit to get.
 *    @luatreturn Outfit|nil The outfit matching name or nil if error.
 * @luafunc get
 */
static int outfitL_get( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushoutfit( L, o );
   return 1;
}

/**
 * @brief Gets a outfit if it exists, nil otherwise.
 *
 * Does not raise any warnings or errors if fails.
 *
 * @usage s = outfit.exists( "Heavy Laser" ) -- Gets the heavy laser if it
 * exists
 *
 *    @luatparam string s Raw (untranslated) name of the outfit to get.
 *    @luatreturn Outfit|nil The outfit matching name or nil if not found
 * @luafunc exists
 */
static int outfitL_exists( lua_State *L )
{
   const Outfit *o = NULL;
   if ( lua_isoutfit( L, 1 ) )
      o = luaL_checkoutfit( L, 1 );
   else if ( lua_isstring( L, 1 ) ) {
      const char *str = lua_tostring( L, 1 );
      o               = outfit_getW( str );
   } else {
      luaL_typerror( L, 1, OUTFIT_METATABLE );
      return 0;
   }
   if ( o != NULL ) {
      lua_pushoutfit( L, o );
      return 1;
   }
   return 0;
}

/**
 * @brief Gets all the outfits.
 *
 *    @luatreturn Table Table containing all the outfits.
 * @luafunc getAll
 */
static int outfitL_getAll( lua_State *L )
{
   const Outfit **outfits = outfit_getAll();
   lua_newtable( L ); /* t */
   for ( int i = 0; i < array_size( outfits ); i++ ) {
      lua_pushoutfit( L, outfits[i] );
      lua_rawseti( L, -2, i + 1 );
   }
   return 1;
}

/**
 * @brief Gets the translated name of the outfit.
 *
 * This translated name should be used for display purposes (e.g.
 * messages). It cannot be used as an identifier for the outfit; for
 * that, use outfit.nameRaw() instead.
 *
 * @usage outfitname = o:name() -- Equivalent to _(o:nameRaw())
 *
 *    @luatparam Outfit s Outfit to get the translated name of.
 *    @luatreturn string The translated name of the outfit.
 * @luafunc name
 */
static int outfitL_name( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushstring( L, outfit_name( o ) );
   return 1;
}

/**
 * @brief Gets the raw (untranslated) name of the outfit.
 *
 * This untranslated name should be used for identification purposes
 * (e.g. can be passed to outfit.get()). It should not be used directly
 * for display purposes without manually translating it with _().
 *
 * @usage outfitrawname = o:nameRaw()
 *
 *    @luatparam Outfit s Outfit to get the raw name of.
 *    @luatreturn string The raw name of the outfit.
 * @luafunc nameRaw
 */
static int outfitL_nameRaw( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushstring( L, outfit_rawname( o ) );
   return 1;
}

/**
 * @brief Gets the translated short name of the outfit.
 *
 * This translated name should be used when you have abbreviate the outfit
 * greatly, e.g., the GUI. In the case the outfit has no specific `shortname`,
 * it's equivalent to `outfit.name()`.
 *
 *    @luatparam Outfit s Outfit to get the translated short name of.
 *    @luatreturn string The translated short name of the outfit.
 * @luafunc shortname
 */
static int outfitL_shortname( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushstring( L, outfit_shortname( o ) );
   return 1;
}

/**
 * @brief Gets the type of an outfit.
 *
 * @usage print( o:type() ) -- Prints the type of the outfit
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string The name of the outfit type (in English).
 * @luafunc type
 */
static int outfitL_type( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushstring( L, outfit_getType( o ) );
   return 1;
}

/**
 * @brief Gets the broad type of an outfit.
 *
 * This name is more generic and vague than type().
 *
 * @usage print( o:typeBroad() ) -- Prints the broad type of the outfit
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string The name of the outfit broad type (in English).
 * @luafunc typeBroad
 */
static int outfitL_typeBroad( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushstring( L, outfit_getTypeBroad( o ) );
   return 1;
}

/**
 * @brief Gets the cpu usage of an outfit.
 *
 * @usage print( o:cpu() ) -- Prints the cpu usage of an outfit
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string The amount of cpu the outfit uses.
 * @luafunc cpu
 */
static int outfitL_cpu( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushnumber( L, outfit_cpu( o ) );
   return 1;
}

/**
 * @brief Gets the mass of an outfit.
 *
 * @usage print( o:mass() ) -- Prints the mass of an outfit
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string The amount of mass the outfit uses.
 * @luafunc mass
 */
static int outfitL_mass( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushnumber( L, outfit_mass( o ) );
   return 1;
}

/**
 * @brief Gets the slot name, size and property of an outfit.
 *
 * @usage slot_name, slot_size, slot_prop = o:slot() -- Gets an outfit's slot
 * info
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string Human readable name (in English).
 *    @luatreturn string Human readable size (in English).
 *    @luatreturn string|nil Human readable property (in English) or nil if
 * none.
 *    @luatreturn boolean Slot is required.
 *    @luatreturn boolean Slot is exclusive.
 * @luafunc slot
 */
static int outfitL_slot( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushstring( L, outfit_slotName( o ) );
   lua_pushstring( L, outfit_slotSizeName( o ) );
   int spid = outfit_slotProperty( o );
   if ( spid == 0 ) {
      lua_pushnil( L );
      lua_pushboolean( L, 0 );
      lua_pushboolean( L, 0 );
   } else {
      lua_pushstring( L, sp_display( spid ) );
      lua_pushboolean( L, sp_required( spid ) );
      lua_pushboolean( L, sp_exclusive( spid ) );
   }
   return 5;
}

/**
 * @brief Gets the extra slot property of an outfit (if applicable).
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string|nil Human readable property (in English) or nil if
 * none.
 * @luafunc slotExtra
 */
static int outfitL_slotExtra( lua_State *L )
{
   const Outfit *o          = luaL_validoutfit( L, 1 );
   int           spid_extra = outfit_slotPropertyExtra( o );
   if ( spid_extra == 0 )
      lua_pushnil( L );
   else
      lua_pushstring( L, sp_display( spid_extra ) );
   return 1;
}

/**
 * @brief Gets the limit string of the outfit. Only one outfit can be equipped
 * at the same time for each limit string.
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string|nil Limit string or nil if not applicable.
 * @luafunc limit
 */
static int outfitL_limit( lua_State *L )
{
   const Outfit *o     = luaL_validoutfit( L, 1 );
   const char   *limit = outfit_limit( o );
   if ( limit ) {
      lua_pushstring( L, limit );
      return 1;
   }
   return 0;
}

/**
 * @brief Gets the store icon for an outfit.
 *
 * @usage ico = o:icon() -- Gets the shop icon for an outfit
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn Tex The texture containing the icon of the outfit.
 * @luafunc icon
 */
static int outfitL_icon( lua_State *L )
{
   const Outfit    *o   = luaL_validoutfit( L, 1 );
   const glTexture *tex = outfit_gfxStore( o );
   lua_pushtex( L, gl_dupTexture( tex ) );
   return 1;
}

/**
 * @brief Gets the license necessary to purchase an outfit (if applicable).
 *
 *    @luatparam Outfit o Outfit to get information of.
 *    @luatreturn string|nil The name of the license or nil if no license is
 * necessary.
 * @luafunc license
 */
static int outfitL_license( lua_State *L )
{
   const Outfit *o       = luaL_validoutfit( L, 1 );
   const char   *license = outfit_license( o );
   if ( license == NULL )
      return 0;
   lua_pushstring( L, license );
   return 1;
}

/**
 * @brief Gets the price of an outfit.
 *
 * @usage price = o:price()
 *
 *    @luatparam outfit|String o Outfit to get the price of.
 *    @luatreturn number The price, in credits.
 * @luafunc price
 */
static int outfitL_price( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushnumber( L, outfit_price( o ) );
   return 1;
}

/**
 * @brief Gets the description of an outfit (translated).
 *
 * @usage description = o:description()
 *
 *    @luatparam outfit|String o Outfit to get the description of.
 *    @luatparam[opt=player.pilot()] Pilot p Pilot to set description to.
 *    @luatreturn string The description (with translating).
 * @luafunc description
 */
static int outfitL_description( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   if ( lua_ispilot( L, 2 ) )
      lua_pushstring(
         L, pilot_outfitDescription( luaL_validpilot( L, 2 ), o, NULL ) );
   else
      lua_pushstring( L, pilot_outfitDescription( player.p, o, NULL ) );
   return 1;
}

/**
 * @brief Gets the summary of an outfit (translated).
 *
 * @usage summary = o:summary()
 *
 *    @luatparam outfit|String o Outfit to get the summary of.
 *    @luatparam[opt=player.pilot()] Pilot p Pilot to set summary to.
 *    @luatparam[opt=false] string noname Whether or not to hide the outfit name
 * at the top.
 *    @luatreturn string The summary (with translating).
 * @luafunc summary
 */
static int outfitL_summary( lua_State *L )
{
   const Outfit *o      = luaL_validoutfit( L, 1 );
   int           noname = lua_toboolean( L, 3 );
   if ( lua_ispilot( L, 2 ) )
      lua_pushstring(
         L, pilot_outfitSummary( luaL_validpilot( L, 2 ), o, !noname, NULL ) );
   else
      lua_pushstring( L, pilot_outfitSummary( player.p, o, !noname, NULL ) );
   return 1;
}

static int getprop( lua_State *L, int prop )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushboolean( L, outfit_isProp( o, prop ) );
   return 1;
}

/**
 * @brief Gets whether or not an outfit is unique
 *
 * @usage isunique = o:unique()
 *
 *    @luatparam outfit|String o Outfit to get the uniqueness of.
 *    @luatreturn boolean The uniqueness of the outfit.
 * @luafunc unique
 */
static int outfitL_unique( lua_State *L )
{
   return getprop( L, OUTFIT_PROP_UNIQUE );
}

/**
 * @brief Gets whether or not a weapon outfit can do friendly fire.
 *
 *    @luatparam outfit|String o Outfit to get the property of of.
 *    @luatreturn boolean Whether or not the outfit can do friendly fire damage.
 * @luafunc friendlyfire
 */
static int outfitL_friendlyfire( lua_State *L )
{
   return getprop( L, OUTFIT_PROP_WEAP_FRIENDLYFIRE );
}

/**
 * @brief Gets whether or not a weapon outfit is point defense.
 *
 *    @luatparam outfit|String o Outfit to get the property of of.
 *    @luatreturn boolean Whether or not the outfit is point defense.
 * @luafunc pointdefense
 */
static int outfitL_pointdefense( lua_State *L )
{
   return getprop( L, OUTFIT_PROP_WEAP_POINTDEFENSE );
}

/**
 * @brief Gets whether or not a weapon outfit misses ships.
 *
 *    @luatparam outfit|String o Outfit to get the property of of.
 *    @luatreturn boolean Whether or not the outfit misses ships.
 * @luafunc missShips
 */
static int outfitL_miss_ships( lua_State *L )
{
   return getprop( L, OUTFIT_PROP_WEAP_MISS_SHIPS );
}

/**
 * @brief Gets whether or not a weapon outfit misses asteroids.
 *
 *    @luatparam outfit|String o Outfit to get the property of of.
 *    @luatreturn boolean Whether or not the outfit misses asteroids.
 * @luafunc missAsteroids
 */
static int outfitL_miss_asteroids( lua_State *L )
{
   return getprop( L, OUTFIT_PROP_WEAP_MISS_ASTEROIDS );
}

/**
 * @brief Gets whether or not an outfit is toggleable.
 *
 *    @luatparam outfit|String o Outfit to get the property of of.
 *    @luatreturn boolean Whether or not the outfit is toggleable.
 * @luafunc toggleable
 */
static int outfitL_toggleable( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   lua_pushboolean( L, outfit_isToggleable( o ) );
   return 1;
}

/**
 * @brief Gets a shipstat from an Outfit by name, or a table containing all the
 * ship stats if not specified.
 *
 *    @luatparam Outfit o Outfit to get ship stat of.
 *    @luatparam[opt=nil] string name Name of the ship stat to get.
 *    @luatparam[opt=false] boolean internal Whether or not to use the internal
 * representation.
 *    @luareturn Value of the ship stat or a tale containing all the ship stats
 * if name is not specified.
 * @luafunc shipstat
 */
static int outfitL_getShipStat( lua_State *L )
{
   ShipStats     ss;
   const Outfit *o = luaL_validoutfit( L, 1 );
   ss_statsInit( &ss );
   ss_statsMergeFromList( &ss, outfit_stats( o ), 0 );
   const char *str      = luaL_optstring( L, 2, NULL );
   int         internal = lua_toboolean( L, 3 );
   ss_statsGetLua( L, &ss, str, internal );
   return 1;
}

/**
 * @brief Computes statistics for weapons.
 *
 *    @luatparam Outfit o Outfit to compute for.
 *    @luatparam[opt=nil] Pilot p Pilot to use ship stats when computing.
 *    @luatreturn number Damage per second of the outfit.
 *    @luatreturn number Disable per second of the outfit.
 *    @luatreturn number Energy per second of the outfit.
 *    @luatreturn number Range of the outfit.
 *    @luatreturn number trackmin Minimum tracking value of the outfit.
 *    @luatreturn number trackmax Maximum tracking value of the outfit.
 *    @luatreturn number lockon Time to lockon.
 * @luafunc weapstats
 */
static int outfitL_weapStats( lua_State *L )
{
   double        eps, dps, disable, shots;
   double        mod_energy, mod_damage, mod_shots;
   double        sdmg, admg;
   const Damage *dmg;
   const Outfit *o = luaL_validoutfit( L, 1 );
   Pilot        *p = ( lua_ispilot( L, 2 ) ) ? luaL_validpilot( L, 2 ) : NULL;

   /* Just return 0 for non-wapons. */
   if ( !outfit_isWeapon( o ) )
      return 0;

   /* Special case beam weapons .*/
   if ( outfit_isBeam( o ) ) {
      if ( p ) {
         /* Special case due to continuous fire. */
         if ( outfit_isTurret( o ) ) {
            mod_energy = p->stats.tur_energy;
            mod_damage = p->stats.tur_damage;
            mod_shots  = 1. / p->stats.tur_firerate;
         } else {
            mod_energy = p->stats.fwd_energy;
            mod_damage = p->stats.fwd_damage;
            mod_shots  = 1. / p->stats.fwd_firerate;
         }
      } else {
         mod_energy = 1.;
         mod_damage = 1.;
         mod_shots  = 1.;
      }
      shots     = outfit_duration( o );
      mod_shots = shots / ( shots + mod_shots * outfit_delay( o ) );
      dmg       = outfit_damage( o );
      /* Modulate the damage by average of damage types. */
      if ( dtype_raw( dmg->type, &sdmg, &admg, NULL ) != 0 )
         return NLUA_ERROR( L, _( "Outfit has invalid damage type." ) );
      mod_damage *= 0.5 * ( sdmg + admg );
      /* Calculate good damage estimates. */
      dps     = mod_shots * mod_damage * dmg->damage;
      disable = mod_shots * mod_damage * dmg->disable;
      /* Bolts have energy per hit, while beams are sustained energy, so flip.
       */
      eps = mod_shots * mod_energy * outfit_energy( o );
      lua_pushnumber( L, dps );
      lua_pushnumber( L, disable );
      lua_pushnumber( L, eps );
      lua_pushnumber( L, outfit_range( o ) );
      return 4;
   }

   if ( p ) {
      switch ( outfit_type( o ) ) {
      case OUTFIT_TYPE_BOLT:
         mod_energy = p->stats.fwd_energy;
         mod_damage = p->stats.fwd_damage;
         mod_shots  = 1. / p->stats.fwd_firerate;
         break;
      case OUTFIT_TYPE_TURRET_BOLT:
         mod_energy = p->stats.tur_energy;
         mod_damage = p->stats.tur_damage;
         mod_shots  = 1. / p->stats.tur_firerate;
         break;
      case OUTFIT_TYPE_LAUNCHER:
      case OUTFIT_TYPE_TURRET_LAUNCHER:
         mod_energy = 1.;
         mod_damage = p->stats.launch_damage;
         mod_shots  = 1. / p->stats.launch_rate;
         break;
      case OUTFIT_TYPE_BEAM:
      case OUTFIT_TYPE_TURRET_BEAM:
      default:
         return 0;
      }
   } else {
      mod_energy = 1.;
      mod_damage = 1.;
      mod_shots  = 1.;
   }

   shots = 1. / ( mod_shots * outfit_delay( o ) );
   /* Special case: Ammo-based weapons. */
   dmg = outfit_damage( o );
   if ( dmg == NULL ) {
      dps     = 0.;
      disable = 0.;
   } else {
      /* Modulate the damage by average of damage types. */
      dtype_raw( dmg->type, &sdmg, &admg, NULL );
      mod_damage *= 0.5 * ( sdmg + admg );
      /* Calculate good damage estimates. */
      dps     = shots * mod_damage * dmg->damage;
      disable = shots * mod_damage * dmg->disable;
   }
   eps = shots * mod_energy * MAX( outfit_energy( o ), 0. );

   lua_pushnumber( L, dps );
   lua_pushnumber( L, disable );
   lua_pushnumber( L, eps );
   lua_pushnumber( L, outfit_range( o ) );
   lua_pushnumber( L, outfit_trackmin( o ) );
   lua_pushnumber( L, outfit_trackmax( o ) );
   if ( outfit_isLauncher( o ) ) {
      lua_pushnumber( L, outfit_launcherLockon( o ) );
      lua_pushnumber( L, outfit_launcherIFLockon( o ) );
      lua_pushboolean( L, outfit_launcherAI( o ) != AMMO_AI_UNGUIDED );
      return 9;
   }
   return 6;
}

#define SETFIELD( name, value )                                                \
   lua_pushnumber( L, value );                                                 \
   lua_setfield( L, -2, name )
#define SETFIELDI( name, value )                                               \
   lua_pushinteger( L, value );                                                \
   lua_setfield( L, -2, name )
#define SETFIELDB( name, value )                                               \
   lua_pushboolean( L, value );                                                \
   lua_setfield( L, -2, name )
/**
 * @brief Returns raw data specific to each outfit type.
 *
 *    @luatreturn A table containing the raw values.
 * @luafunc specificstats
 */
static int outfitL_specificStats( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   const Damage *dmg;
   lua_newtable( L );
   switch ( outfit_type( o ) ) {
   case OUTFIT_TYPE_AFTERBURNER:
      SETFIELD( "accel", outfit_afterburnerAccel( o ) );
      SETFIELD( "speed", outfit_afterburnerSpeed( o ) );
      SETFIELD( "energy", outfit_energy( o ) );
      SETFIELD( "mass_limit", outfit_afterburnerMassLimit( o ) );
      break;

   case OUTFIT_TYPE_FIGHTER_BAY:
      lua_pushship( L, outfit_bayShip( o ) );
      lua_setfield( L, -2, "ship" );
      SETFIELD( "delay", outfit_delay( o ) );
      SETFIELDI( "amount", outfit_amount( o ) );
      SETFIELD( "reload_time", outfit_reloadTime( o ) );
      break;

   case OUTFIT_TYPE_TURRET_BOLT:
      SETFIELDB( "isturret", 1 );
      FALLTHROUGH;
   case OUTFIT_TYPE_BOLT:
      SETFIELD( "delay", outfit_delay( o ) );
      SETFIELD( "speed", outfit_speed( o ) );
      SETFIELD( "range", outfit_range( o ) );
      SETFIELD( "falloff", outfit_falloff( o ) );
      SETFIELD( "energy", outfit_energy( o ) );
      SETFIELD( "trackmin", outfit_trackmin( o ) );
      SETFIELD( "trackmax", outfit_trackmax( o ) );
      SETFIELD( "swivel", outfit_swivel( o ) );
      /* Damage stuff. */
      dmg = outfit_damage( o );
      SETFIELD( "penetration", dmg->penetration );
      SETFIELD( "damage", dmg->damage );
      SETFIELD( "disable", dmg->disable );
      break;

   case OUTFIT_TYPE_TURRET_BEAM:
      SETFIELDB( "isturret", 1 );
      FALLTHROUGH;
   case OUTFIT_TYPE_BEAM:
      SETFIELD( "delay", outfit_delay( o ) );
      SETFIELD( "min_delay", outfit_beamMinDelay( o ) );
      SETFIELD( "warmup", outfit_beamWarmup( o ) );
      SETFIELD( "duration", outfit_duration( o ) );
      SETFIELD( "range", outfit_range( o ) );
      SETFIELD( "turn", outfit_turn( o ) );
      SETFIELD( "energy", outfit_energy( o ) );
      /* Damage stuff. */
      dmg = outfit_damage( o );
      SETFIELD( "penetration", dmg->penetration );
      SETFIELD( "damage", dmg->damage );
      SETFIELD( "disable", dmg->disable );
      break;

   case OUTFIT_TYPE_TURRET_LAUNCHER:
      SETFIELDB( "isturret", 1 );
      FALLTHROUGH;
   case OUTFIT_TYPE_LAUNCHER:
      SETFIELD( "delay", outfit_delay( o ) );
      SETFIELDI( "amount", outfit_amount( o ) );
      SETFIELD( "reload_time", outfit_reloadTime( o ) );
      SETFIELD( "lockon", outfit_launcherLockon( o ) );
      SETFIELD( "iflockon", outfit_launcherIFLockon( o ) );
      SETFIELD( "trackmin", outfit_trackmin( o ) );
      SETFIELD( "trackmax", outfit_trackmax( o ) );
      SETFIELD( "arc", outfit_launcherArc( o ) );
      SETFIELD( "swivel", outfit_swivel( o ) );
      /* Ammo stuff. */
      SETFIELD( "duration", outfit_duration( o ) );
      SETFIELD( "speed", outfit_speed( o ) );
      SETFIELD( "speed_max", outfit_launcherSpeedMax( o ) );
      SETFIELD( "turn", outfit_launcherTurn( o ) );
      SETFIELD( "accel", outfit_launcherAccel( o ) );
      SETFIELD( "energy", outfit_energy( o ) );
      SETFIELDB( "seek", outfit_launcherAI( o ) != AMMO_AI_UNGUIDED );
      SETFIELDB( "smart", outfit_launcherAI( o ) == AMMO_AI_SMART );
      /* Damage stuff. */
      dmg = outfit_damage( o );
      SETFIELD( "penetration", dmg->penetration );
      SETFIELD( "damage", dmg->damage );
      SETFIELD( "disable", dmg->disable );
      break;

   default:
      break;
   }
   return 1;
}
#undef SETFIELD
#undef SETFIELDI
#undef SETFIELDB

/**
 * @brief Gets the factions to which the outfit is illegal to.
 *
 *    @luatparam o Outfit to get illegality status of.
 *    @luatreturn table Table of all the factions the outfit is illegal to.
 * @luafunc illegality
 */
static int outfitL_illegality( lua_State *L )
{
   const Outfit *o         = luaL_validoutfit( L, 1 );
   int          *illegalto = outfit_illegalTo( o );
   lua_newtable( L );
   for ( int i = 0; i < array_size( illegalto ); i++ ) {
      lua_pushfaction( L, illegalto[i] );
      lua_rawseti( L, -2, i + 1 );
   }
   return 1;
}

/**
 * @brief Gets whether or not the outfit is known to the player, as in they know
 * a spob that sells it or own it.
 */
static int outfitL_known( lua_State *L )
{
   /* TODO cache this and mark as dirty instead of recomputing for each outfit.
    */
   const Outfit         *o  = luaL_validoutfit( L, 1 );
   const PlayerOutfit_t *po = player_getOutfits();
   for ( int i = 0; i < array_size( po ); i++ ) {
      if ( po[i].o == o ) {
         lua_pushboolean( L, 1 );
         return 1;
      }
   }
   const PlayerShip_t *ps = player_getShipStack();
   for ( int i = 0; i < array_size( ps ); i++ ) {
      for ( int j = 0; j < array_size( ps[i].p->outfits ); j++ ) {
         if ( ps[i].p->outfits[j]->outfit == o ) {
            lua_pushboolean( L, 1 );
            return 1;
         }
      }
   }
   const Spob *ss = spob_getAll();
   for ( int i = 0; i < array_size( ss ); i++ ) {
      const Spob *spb = &ss[i];
      if ( !spob_hasService( spb, SPOB_SERVICE_SHIPYARD ) )
         continue;
      if ( !spob_isKnown( spb ) )
         continue;
      if ( tech_hasOutfit( spb->tech, o ) ) {
         lua_pushboolean( L, 1 );
         return 1;
      }
   }
   lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Gets the outfit tags.
 *
 * @usage if o:tags["fancy"] then -- Has "fancy" tag
 *
 *    @luatparam Outfit o Outfit to get tags of.
 *    @luatparam[opt=nil] string tag Tag to check if exists.
 *    @luatreturn table|boolean Table of tags where the name is the key and true
 * is the value or a boolean value if a string is passed as the second parameter
 * indicating whether or not the specified tag exists.
 * @luafunc tags
 */
static int outfitL_tags( lua_State *L )
{
   const Outfit *o = luaL_validoutfit( L, 1 );
   return nlua_helperTags( L, 2, outfit_tags( o ) );
}
