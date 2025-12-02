/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_faction.c
 *
 * @brief Handles the Lua faction bindings.
 *
 * These bindings control the factions.
 */
/** @cond */
#include <lauxlib.h>
/** @endcond */

#include "nlua_faction.h"

#include "array.h"
#include "faction.h"
#include "nlua.h"
#include "nlua_colour.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nluadef.h"

/* Internal useful functions. */
static LuaFaction luaL_validfactionSilent( lua_State *L, int ind );
/* Faction metatable methods */
static int factionL_exists( lua_State *L );
static int factionL_get( lua_State *L );
static int factionL_getAll( lua_State *L );
static int factionL_player( lua_State *L );
static int factionL_eq( lua_State *L );
static int factionL_name( lua_State *L );
static int factionL_nameRaw( lua_State *L );
static int factionL_longname( lua_State *L );
static int factionL_areneutral( lua_State *L );
static int factionL_areenemies( lua_State *L );
static int factionL_areallies( lua_State *L );
static int factionL_usesHiddenJumps( lua_State *L );
static int factionL_hit( lua_State *L );
static int factionL_hitTest( lua_State *L );
static int factionL_reputationGlobal( lua_State *L );
static int factionL_reputationText( lua_State *L );
static int factionL_reputationDefault( lua_State *L );
static int factionL_setReputationGlobal( lua_State *L );
static int factionL_applyLocalThreshold( lua_State *L );
static int factionL_enemies( lua_State *L );
static int factionL_allies( lua_State *L );
static int factionL_logo( lua_State *L );
static int factionL_colour( lua_State *L );
static int factionL_isKnown( lua_State *L );
static int factionL_setKnown( lua_State *L );
static int factionL_isInvisible( lua_State *L );
static int factionL_isStatic( lua_State *L );
static int factionL_reputationOverride( lua_State *L );
static int factionL_setReputationOverride( lua_State *L );
static int factionL_tags( lua_State *L );
static int factionL_dynAdd( lua_State *L );
static int factionL_dynAlly( lua_State *L );
static int factionL_dynEnemy( lua_State *L );

static const luaL_Reg faction_methods[] = {
   { "exists", factionL_exists },
   { "get", factionL_get },
   { "getAll", factionL_getAll },
   { "player", factionL_player },
   { "__eq", factionL_eq },
   { "__tostring", factionL_name },
   { "name", factionL_name },
   { "nameRaw", factionL_nameRaw },
   { "longname", factionL_longname },
   { "areNeutral", factionL_areneutral },
   { "areEnemies", factionL_areenemies },
   { "areAllies", factionL_areallies },
   { "hit", factionL_hit },
   { "hitTest", factionL_hitTest },
   { "reputationGlobal", factionL_reputationGlobal },
   { "reputationText", factionL_reputationText },
   { "reputationDefault", factionL_reputationDefault },
   { "setReputationGlobal", factionL_setReputationGlobal },
   { "applyLocalThreshold", factionL_applyLocalThreshold },
   { "enemies", factionL_enemies },
   { "allies", factionL_allies },
   { "usesHiddenJumps", factionL_usesHiddenJumps },
   { "logo", factionL_logo },
   { "colour", factionL_colour },
   { "known", factionL_isKnown },
   { "setKnown", factionL_setKnown },
   { "invisible", factionL_isInvisible },
   { "static", factionL_isStatic },
   { "reputationOverride", factionL_reputationOverride },
   { "setReputationOverride", factionL_setReputationOverride },
   { "tags", factionL_tags },
   { "dynAdd", factionL_dynAdd },
   { "dynAlly", factionL_dynAlly },
   { "dynEnemy", factionL_dynEnemy },
   { 0, 0 } }; /**< Faction metatable methods. */

/**
 * @brief Loads the faction library.
 *
 *    @param env Environment to load faction library into.
 *    @return 0 on success.
 */
int nlua_loadFaction( nlua_env *env )
{
   nlua_register( env, FACTION_METATABLE, faction_methods, 1 );
   return 0; /* No error */
}

/**
 * @brief Lua bindings to deal with factions.
 *
 * Use like:
 * @code
 * f = faction.get( "Empire" )
 * if f:playerStanding() < 0 then
 *    -- player is hostile to Empire
 * end
 * @endcode
 *
 * @luamod faction
 */
/**
 * @brief Gets a faction if it exists.
 *
 * @usage f = faction.exists( "Empire" )
 *
 *    @luatparam string name Name of the faction to get if exists.
 *    @luatreturn Faction The faction matching name or nil if not matched.
 * @luafunc exists
 */
static int factionL_exists( lua_State *L )
{
   const char *s = luaL_checkstring( L, 1 );
   if ( faction_exists( s ) ) {
      lua_pushfaction( L, faction_get( s ) );
      return 1;
   }
   return 0;
}

/**
 * @brief Gets the faction based on its name.
 *
 * @usage f = faction.get( "Empire" )
 *
 *    @luatparam string name Name of the faction to get.
 *    @luatreturn Faction The faction matching name.
 * @luafunc get
 */
static int factionL_get( lua_State *L )
{
   LuaFaction f = luaL_validfaction( L, 1 );
   lua_pushfaction( L, f );
   return 1;
}

/**
 * @brief Gets all the factions.
 *
 *    @luatreturn {Faction,...} An ordered table containing all of the factions.
 * @luafunc getAll
 */
static int factionL_getAll( lua_State *L )
{
   const int *factions = faction_getAll();
   lua_newtable( L );
   for ( int i = 0; i < array_size( factions ); i++ ) {
      lua_pushfaction( L, factions[i] );
      lua_rawseti( L, -2, i + 1 );
   }
   array_free( factions );
   return 1;
}

/**
 * @brief Gets the player's faction.
 *
 * @usage pf = faction.player()
 *
 *    @luareturn Faction The player's faction.
 * @luafunc player
 */
static int factionL_player( lua_State *L )
{
   lua_pushfaction( L, faction_player );
   return 1;
}

/**
 * @brief Gets faction at index.
 *
 *    @param L Lua state to get faction from.
 *    @param ind Index position to find the faction.
 *    @return Faction found at the index in the state.
 */
LuaFaction lua_tofaction( lua_State *L, int ind )
{
   return *( (LuaFaction *)lua_touserdata( L, ind ) );
}

LuaFaction lua_checkfaction( lua_State *L, int ind )
{
   if ( lua_isfaction( L, ind ) )
      return lua_tofaction( L, ind );
   luaL_typerror( L, ind, FACTION_METATABLE );
   return -1;
}

static LuaFaction luaL_validfactionSilent( lua_State *L, int ind )
{
   if ( lua_isfaction( L, ind ) )
      return lua_tofaction( L, ind );
   else if ( lua_isstring( L, ind ) )
      return faction_get( lua_tostring( L, ind ) );
   luaL_typerror( L, ind, FACTION_METATABLE );
   return 0;
}

/**
 * @brief Gets faction (or faction name) at index, raising an error if type
 * isn't a valid faction.
 *
 *    @param L Lua state to get faction from.
 *    @param ind Index position to find the faction.
 *    @return Faction found at the index in the state.
 */
LuaFaction luaL_validfaction( lua_State *L, int ind )
{
   int id = luaL_validfactionSilent( L, ind );
   if ( id == -1 )
      return NLUA_ERROR( L, _( "Faction '%s' not found in stack." ),
                         lua_tostring( L, ind ) );
   return id;
}

/**
 * @brief Pushes a faction on the stack.
 *
 *    @param L Lua state to push faction into.
 *    @param faction Faction to push.
 *    @return Newly pushed faction.
 */
LuaFaction *lua_pushfaction( lua_State *L, LuaFaction faction )
{
   LuaFaction *f = (LuaFaction *)lua_newuserdata( L, sizeof( LuaFaction ) );
   *f            = faction;
   luaL_getmetatable( L, FACTION_METATABLE );
   lua_setmetatable( L, -2 );
   return f;
}
/**
 * @brief Checks to see if ind is a faction.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a faction.
 */
int lua_isfaction( lua_State *L, int ind )
{
   int ret;

   if ( lua_getmetatable( L, ind ) == 0 )
      return 0;
   lua_getfield( L, LUA_REGISTRYINDEX, FACTION_METATABLE );

   ret = 0;
   if ( lua_rawequal( L, -1, -2 ) ) /* does it have the correct mt? */
      ret = 1;

   lua_pop( L, 2 ); /* remove both metatables */
   return ret;
}

/**
 * @brief equality (`__eq()`) metamethod for factions.
 *
 * You can use the `==` operator within Lua to compare factions with this.
 *
 * @usage if f == faction.get( "Dvaered" ) then
 *
 *    @luatparam Faction f Faction comparing.
 *    @luatparam Faction comp faction to compare against.
 *    @luatreturn boolean true if both factions are the same.
 * @luafunc __eq
 */
static int factionL_eq( lua_State *L )
{
   int a = luaL_validfaction( L, 1 );
   int b = luaL_validfaction( L, 2 );
   lua_pushboolean( L, a == b );
   return 1;
}

/**
 * @brief Gets the faction's translated short name.
 *
 * This translated name should be used for display purposes (e.g.
 * messages) where the shorter version of the faction's display name
 * should be used. It cannot be used as an identifier for the faction;
 * for that, use faction.nameRaw() instead.
 *
 * @usage shortname = f:name()
 *
 *    @luatparam Faction f The faction to get the name of.
 *    @luatreturn string The name of the faction.
 * @luafunc name
 */
static int factionL_name( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   lua_pushstring( L, faction_shortname( f ) );
   return 1;
}

/**
 * @brief Gets the faction's raw / "real" (untranslated, internal) name.
 *
 * This untranslated name should be used for identification purposes
 * (e.g. can be passed to faction.get()). It should not be used for
 * display purposes; for that, use faction.name() or faction.longname()
 * instead.
 *
 * @usage name = f:nameRaw()
 *
 *    @luatparam Faction f The faction to get the name of.
 *    @luatreturn string The name of the faction.
 * @luafunc nameRaw
 */
static int factionL_nameRaw( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   lua_pushstring( L, faction_name( f ) );
   return 1;
}

/**
 * @brief Gets the faction's translated long name.
 *
 * This translated name should be used for display purposes (e.g.
 * messages) where the longer version of the faction's display name
 * should be used. It cannot be used as an identifier for the faction;
 * for that, use faction.nameRaw() instead.
 *
 * @usage longname = f:longname()
 *    @luatparam Faction f Faction to get long name of.
 *    @luatreturn string The long name of the faction (translated).
 * @luafunc longname
 */
static int factionL_longname( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   lua_pushstring( L, faction_longname( f ) );
   return 1;
}

/**
 * @brief Checks to see if two factions are truly neutral with respect to each
 * other.
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam Faction n Faction to check if is true neutral.
 *    @luatreturn boolean true if they are truly neutral, false if they aren't.
 * @luafunc areNeutral
 */
static int factionL_areneutral( lua_State *L )
{
   int f  = luaL_validfaction( L, 1 );
   int ff = luaL_validfaction( L, 2 );
   lua_pushboolean( L, areNeutral( f, ff ) );
   return 1;
}

/**
 * @brief Checks to see if f is an enemy of e.
 *
 * @usage if f:areEnemies( faction.get( "Dvaered" ) ) then
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam Faction e Faction to check if is an enemy.
 *    @luatparam[opt] System sys System to check to see if they are enemies in.
 * Mainly for when comparing to the player's faction.
 *    @luatreturn boolean true if they are enemies, false if they aren't.
 * @luafunc areEnemies
 */
static int factionL_areenemies( lua_State *L )
{
   int f  = luaL_validfaction( L, 1 );
   int ff = luaL_validfaction( L, 2 );
   if ( !lua_isnoneornil( L, 3 ) )
      lua_pushboolean( L, areEnemiesSystem( f, ff, luaL_validsystem( L, 3 ) ) );
   else
      lua_pushboolean( L, areEnemies( f, ff ) );
   return 1;
}

/**
 * @brief Checks to see if f is an ally of a.
 *
 * @usage if f:areAllies( faction.get( "Pirate" ) ) then
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam faction a Faction to check if is an enemy.
 *    @luatparam[opt] System sys System to check to see if they are allies in.
 * Mainly for when comparing to the player's faction.
 *    @luatreturn boolean true if they are enemies, false if they aren't.
 * @luafunc areAllies
 */
static int factionL_areallies( lua_State *L )
{
   int f  = luaL_validfaction( L, 1 );
   int ff = luaL_validfaction( L, 2 );
   if ( !lua_isnoneornil( L, 3 ) )
      lua_pushboolean( L, areAlliesSystem( f, ff, luaL_validsystem( L, 3 ) ) );
   else
      lua_pushboolean( L, areAllies( f, ff ) );
   return 1;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Also can modify the standing with allies and enemies of the faction.
 *
 * @usage f:hit( -5, system.cur() ) -- Lowers faction by 5 at the current system
 * @usage f:hit( 10 ) -- Globally increases faction by 10
 * @usage f:hit( 10, nil, nil, true ) -- Globally increases faction by 10, but
 * doesn't affect allies nor enemies of the faction.
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod Amount of reputation to change.
 *    @luatparam System|nil Whether to make the faction hit local at a system,
 * or global affecting all systems of the faction.
 *    @luatparam[opt="script"] string reason Reason behind it. This is passed as
 * a string to the faction `hit` function. The engine can generate `destroy` and
 * `distress` sources. For missions the default is `script`.
 *    @luatparam[opt=false] boolean single Whether or not the hit should affect
 * allies/enemies of the faction getting a hit.
 *    @luatreturn How much the reputation was actually changed after Lua script
 * was run.
 * @luafunc hit
 */
static int factionL_hit( lua_State *L )
{
   int               f   = luaL_validfaction( L, 1 );
   double            mod = luaL_checknumber( L, 2 );
   const StarSystem *sys =
      ( lua_isnoneornil( L, 3 ) ) ? NULL : luaL_validsystem( L, 3 );
   const char *reason = luaL_optstring( L, 4, "script" );
   double      ret = faction_hit( f, sys, mod, reason, lua_toboolean( L, 5 ) );
   lua_pushnumber( L, ret );
   return 1;
}

/**
 * @brief Simulates modifying the player's standing with a faction and computes
 * how much would be changed.
 *
 *    @luatparam Faction f Faction to simulate player's standing with.
 *    @luatparam number mod Amount of reputation to simulate change.
 *    @luatparam System|nil Whether to make the faction hit local at a system,
 * or global.
 *    @luatparam[opt="script"] string reason Reason behind it. This is passed as
 * a string to the faction `hit` function. The engine can generate `destroy` and
 * `distress` sources. For missions the default is `script`.
 *    @luatreturn How much the reputation was actually changed after Lua script
 * was run.
 * @luafunc hitTest
 */
static int factionL_hitTest( lua_State *L )
{
   int               f   = luaL_validfaction( L, 1 );
   double            mod = luaL_checknumber( L, 2 );
   const StarSystem *sys =
      ( lua_isnoneornil( L, 3 ) ) ? NULL : luaL_validsystem( L, 3 );
   const char *reason = luaL_optstring( L, 4, "script" );
   double      ret    = faction_hitTest( f, sys, mod, reason );
   lua_pushnumber( L, ret );
   return 1;
}

/**
 * @brief Gets the player's global reputation with the faction.
 *
 * @usage if f:reputationGlobal() >= 0 then -- Player is not hostile
 *
 *    @luatparam Faction f Faction to get player's standing with.
 *    @luatreturn number The value of the standing.
 * @luafunc reputationGlobal
 */
static int factionL_reputationGlobal( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   lua_pushnumber( L, faction_reputation( f ) );
   return 1;
}

/**
 * @brief Gets the human readable standing text corresponding (translated).
 *
 *    @luatparam faction f Faction to get standing text from.
 *    @luatparam[opt=f:reputationGlobal()] number|nil val Value to get the
 * standing text of, or nil to use the global faction standing.
 *    @luatreturn string Translated text corresponding to the faction value.
 * @luafunc reputationText
 */
static int factionL_reputationText( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   if ( lua_isnoneornil( L, 2 ) )
      lua_pushstring( L, faction_getStandingTextAtValue(
                            f, faction_reputationDefault( f ) ) );
   else
      lua_pushstring(
         L, faction_getStandingTextAtValue( f, luaL_checknumber( L, 2 ) ) );
   return 1;
}

/**
 * @brief Gets the player's default reputation with the faction.
 *
 *    @luatparam Faction f Faction to get player's default standing with.
 *    @luatreturn number The value of the standing.
 * @luafunc reputationDefault
 */
static int factionL_reputationDefault( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   lua_pushnumber( L, faction_reputationDefault( f ) );
   return 1;
}

/**
 * @brief Overrides the player's faction global standing with a faction. Use
 * sparingly as it overwrites local standings at all systems.
 *
 *    @luatparam Faction f Faction to set the player's global reputation with.
 *    @luatparam number The value of the reputation to set to.
 * @luafunc setReputationGlobal
 */
static int factionL_setReputationGlobal( lua_State *L )
{
   int    f = luaL_validfaction( L, 1 );
   double n = luaL_checknumber( L, 2 );
   /* TODO finish. */
   faction_setReputation( f, n );
   return 0;
}

/**
 * @brief Enforces the local threshold of a faction starting at a particular
 * system. Meant to be used when computing faction hits from the faction
 * standing Lua scripts. Not meant for use elsewhere.
 *
 *    @luatparam Faction f Faction to apply local threshold to.
 *    @luatparam System sys System to compute the threshold from. This will
 * be the reference and will not have its value modified.
 * @luafunc applyLocalThreshold
 */
static int factionL_applyLocalThreshold( lua_State *L )
{
   int         f   = luaL_validfaction( L, 1 );
   StarSystem *sys = luaL_validsystem( L, 2 );
   faction_applyLocalThreshold( f, sys );
   return 0;
}

/**
 * @brief Gets the enemies of the faction.
 *
 * @usage for k,v in pairs(f:enemies()) do -- Iterates over enemies
 *
 *    @luatparam Faction f Faction to get enemies of.
 *    @luatreturn {Faction,...} A table containing the enemies of the faction.
 * @luafunc enemies
 */
static int factionL_enemies( lua_State *L )
{
   const int *factions;
   int        f = luaL_validfaction( L, 1 );

   /* Push the enemies in a table. */
   lua_newtable( L );
   factions = faction_getEnemies( f );
   for ( int i = 0; i < array_size( factions ); i++ ) {
      lua_pushfaction( L, factions[i] ); /* value */
      lua_rawseti( L, -2, i + 1 );
   }

   return 1;
}

/**
 * @brief Gets the allies of the faction.
 *
 * @usage for k,v in pairs(f:allies()) do -- Iterate over faction allies
 *
 *    @luatparam Faction f Faction to get allies of.
 *    @luatreturn {Faction,...} A table containing the allies of the faction.
 * @luafunc allies
 */
static int factionL_allies( lua_State *L )
{
   const int *factions;
   int        f = luaL_validfaction( L, 1 );

   /* Push the enemies in a table. */
   lua_newtable( L );
   factions = faction_getAllies( f );
   for ( int i = 0; i < array_size( factions ); i++ ) {
      lua_pushfaction( L, factions[i] ); /* value */
      lua_rawseti( L, -2, i + 1 );
   }

   return 1;
}

/**
 * @brief Gets whether or not a faction uses hidden jumps.
 *
 *    @luatparam Faction f Faction to get whether or not they use hidden jumps.
 *    @luatreturn boolean true if the faction uses hidden jumps, false
 * otherwise.
 * @luafunc usesHiddenJumps
 */
static int factionL_usesHiddenJumps( lua_State *L )
{
   int f = luaL_validfaction( L, 1 );
   lua_pushboolean( L, faction_usesHiddenJumps( f ) );
   return 1;
}

/**
 * @brief Gets the faction logo.
 *
 *    @luatparam Faction f Faction to get logo from.
 *    @luatreturn Tex The faction logo or nil if not applicable.
 * @luafunc logo
 */
static int factionL_logo( lua_State *L )
{
   int              lf  = luaL_validfaction( L, 1 );
   const glTexture *tex = faction_logo( lf );
   if ( tex == NULL )
      return 0;
   lua_pushtex( L, gl_dupTexture( tex ) );
   return 1;
}

/**
 * @brief Gets the faction colour.
 *
 *    @luatparam Faction f Faction to get colour from.
 *    @luatreturn Colour|nil The faction colour or nil if not applicable.
 * @luafunc colour
 */
static int factionL_colour( lua_State *L )
{
   int             lf  = luaL_validfaction( L, 1 );
   const glColour *col = faction_reputationColour( lf );
   if ( col == NULL )
      return 0;
   lua_pushcolour( L, *col );
   return 1;
}

/**
 * @brief Checks to see if a faction is known by the player.
 *
 * @usage b = f:known()
 *
 *    @luatparam Faction f Faction to check if the player knows.
 *    @luatreturn boolean true if the player knows the faction.
 * @luafunc known
 */
static int factionL_isKnown( lua_State *L )
{
   int fac = luaL_validfaction( L, 1 );
   lua_pushboolean( L, faction_isKnown( fac ) );
   return 1;
}

/**
 * @brief Sets a faction's known state.
 *
 * @usage f:setKnown( false ) -- Makes faction unknown.
 *    @luatparam Faction f Faction to set known.
 *    @luatparam[opt=false] boolean b Whether or not to set as known.
 * @luafunc setKnown
 */
static int factionL_setKnown( lua_State *L )
{
   int fac = luaL_validfaction( L, 1 );
   int b   = lua_toboolean( L, 2 );
   faction_setKnown( fac, b );
   return 0;
}

/**
 * @brief Checks to see if a faction is invisible the player.
 *
 * @usage b = f:invisible()
 *
 *    @luatparam Faction f Faction to check if is invisible to the player.
 *    @luatreturn boolean true if the faction is invisible to the player.
 * @luafunc invisible
 */
static int factionL_isInvisible( lua_State *L )
{
   int fac = luaL_validfaction( L, 1 );
   lua_pushboolean( L, faction_isInvisible( fac ) );
   return 1;
}

/**
 * @brief Checks to see if a faction has a static standing with the player.
 *
 * @usage b = f:static()
 *
 *    @luatparam Faction f Faction to check if has a static standing to the
 * player.
 *    @luatreturn boolean true if the faction is static to the player.
 * @luafunc static
 */
static int factionL_isStatic( lua_State *L )
{
   int fac = luaL_validfaction( L, 1 );
   lua_pushboolean( L, faction_isStatic( fac ) );
   return 1;
}

/**
 * @brief Gets the overridden reputation value of a faction.
 *
 *    @luatparam Faction f Faction to get whether or not the reputation is
 * overridden and the value.
 *    @luatreturn number|nil The override reputation value or nil if not
 * overridden.
 * @luafunc reputationOverride
 */
static int factionL_reputationOverride( lua_State *L )
{
   int    fac = luaL_validfaction( L, 1 );
   int    set;
   double val = faction_reputationOverride( fac, &set );
   if ( !set )
      return 0;
   lua_pushnumber( L, val );
   return 1;
}

/**
 * @brief Gets the overridden reputation value of a faction.
 *
 *    @luatparam Faction f Faction to enable/disable reputation override of.
 *    @luatparam number|nil Sets the faction reputation override to the value if
 * a number, or disables it if nil.
 * @luafunc setReputationOverride
 */
static int factionL_setReputationOverride( lua_State *L )
{
   int fac = luaL_validfaction( L, 1 );
   int set = !lua_isnoneornil( L, 2 );
   faction_setReputationOverride( fac, set, luaL_optnumber( L, 2, 0. ) );
   return 1;
}

/**
 * @brief Gets the tags a faction has.
 *
 * @usage for k,v in ipairs(f:tags()) do ... end
 * @usage if f:tags().likes_cheese then ... end
 * @usage if f:tags("generic") then ... end
 *
 *    @luatparam[opt=nil] string tag Tag to check if exists.
 *    @luatreturn table|boolean Table of tags where the name is the key and true
 * is the value or a boolean value if a string is passed as the second parameter
 * indicating whether or not the specified tag exists.
 * @luafunc tags
 */
static int factionL_tags( lua_State *L )
{
   int fac = luaL_validfaction( L, 1 );
   return nlua_helperTags( L, 2, (char *const *)faction_tags( fac ) );
}

/**
 * @brief Adds a faction dynamically. Note that if the faction already exists as
 * a dynamic faction, the existing one is returned.
 *
 * @note Defaults to known.
 *
 *    @luatparam Faction|nil base Faction to base it off of or nil for new
 * faction.
 *    @luatparam string name Name to give the faction.
 *    @luatparam[opt] string display Display name to give the faction.
 *    @luatparam[opt] table params Table of parameters. Options include `ai`
 * (string) to set the AI to use, `clear_allies` (boolean) to clear all allies,
 * `clear_enemies` (boolean) to clear all enemies, `player` (number) to set the
 * default player standing, `colour` (string|colour) which represents the
 * factional colours.
 * @luafunc dynAdd
 */
static int factionL_dynAdd( lua_State *L )
{
   LuaFaction      fac, newfac;
   const char     *name, *display, *ai;
   const glColour *colour;
   int             clear_allies, clear_enemies;
   double          player;
   int             set_player;

   if ( !lua_isnoneornil( L, 1 ) )
      fac = luaL_validfactionSilent( L, 1 ); /* Won't error. */
   else
      fac = -1;
   name       = luaL_checkstring( L, 2 );
   display    = luaL_optstring( L, 3, name );
   set_player = 0;

   /* Just return existing and ignore the rest. */
   if ( faction_exists( name ) ) {
      int f = faction_get( name );
      if ( !faction_isDynamic( f ) )
         return NLUA_ERROR( L,
                            _( "Trying to overwrite existing faction '%s' with "
                               "dynamic faction!" ),
                            name );

      lua_pushfaction( L, f );
      return 1;
   }

   /* Parse parameters. */
   if ( lua_istable( L, 4 ) ) {
      lua_getfield( L, 4, "ai" );
      ai = luaL_optstring( L, -1, NULL );
      lua_pop( L, 1 );

      lua_getfield( L, 4, "clear_allies" );
      clear_allies = lua_toboolean( L, -1 );
      lua_pop( L, 1 );

      lua_getfield( L, 4, "clear_enemies" );
      clear_enemies = lua_toboolean( L, -1 );
      lua_pop( L, 1 );

      lua_getfield( L, 4, "player" );
      if ( lua_isnumber( L, -1 ) ) {
         player     = lua_tonumber( L, -1 );
         set_player = 1;
      }
      lua_pop( L, 1 );

      lua_getfield( L, 4, "colour" );
      if ( lua_isstring( L, -1 ) )
         colour = col_fromName( lua_tostring( L, -1 ) );
      else
         colour = lua_tocolour( L, -1 );
      lua_pop( L, 1 );
   } else {
      ai            = NULL;
      clear_allies  = 0;
      clear_enemies = 0;
      player        = 0.;
      colour        = NULL;
   }

   /* Create new faction. */
   newfac = faction_dynAdd( fac, name, display, ai, colour );

   /* Clear if necessary. */
   if ( clear_allies )
      faction_clearAlly( newfac );
   if ( clear_enemies )
      faction_clearEnemy( newfac );
   if ( set_player )
      faction_setReputation( newfac, player );

   lua_pushfaction( L, newfac );
   return 1;
}

/**
 * @brief Adds or removes allies to a faction. Only works with dynamic factions.
 *
 *    @luatparam Faction fac Faction to add ally to.
 *    @luatparam Faction ally Faction to add as an ally.
 *    @luatparam[opt=false] boolean remove Whether or not to remove the ally
 * from the faction instead of adding it.
 * @luafunc dynAlly
 */
static int factionL_dynAlly( lua_State *L )
{
   LuaFaction fac, ally;
   int        remove;
   fac = luaL_validfaction( L, 1 );
   if ( !faction_isDynamic( fac ) )
      return NLUA_ERROR( L, _( "Can only add allies to dynamic factions" ) );
   ally   = luaL_validfaction( L, 2 );
   remove = lua_toboolean( L, 3 );
   if ( remove )
      faction_rmAlly( fac, ally );
   else
      faction_addAlly( fac, ally );
   return 0;
}

/**
 * @brief Adds or removes enemies to a faction. Only works with dynamic
 * factions.
 *
 *    @luatparam Faction fac Faction to add enemy to.
 *    @luatparam Faction enemy Faction to add as an enemy.
 *    @luatparam[opt=false] boolean remove Whether or not to remove the enemy
 * from the faction instead of adding it.
 * @luafunc dynEnemy
 */
static int factionL_dynEnemy( lua_State *L )
{
   LuaFaction fac, enemy;
   int        remove;
   fac = luaL_validfaction( L, 1 );
   if ( !faction_isDynamic( fac ) )
      return NLUA_ERROR( L, _( "Can only add allies to dynamic factions" ) );
   enemy  = luaL_validfaction( L, 2 );
   remove = lua_toboolean( L, 3 );
   if ( remove )
      faction_rmEnemy( fac, enemy );
   else
      faction_addEnemy( fac, enemy );
   return 0;
}
