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

#include "nlua_faction.h"

#include "naev.h"

#include <lauxlib.h>

#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "nlua_col.h"
#include "faction.h"


/* Faction metatable methods */
static int factionL_get( lua_State *L );
static int factionL_eq( lua_State *L );
static int factionL_name( lua_State *L );
static int factionL_longname( lua_State *L );
static int factionL_areenemies( lua_State *L );
static int factionL_areallies( lua_State *L );
static int factionL_modplayer( lua_State *L );
static int factionL_modplayersingle( lua_State *L );
static int factionL_modplayerraw( lua_State *L );
static int factionL_setplayerstanding( lua_State *L );
static int factionL_playerstanding( lua_State *L );
static int factionL_enemies( lua_State *L );
static int factionL_allies( lua_State *L );
static int factionL_logoSmall( lua_State *L );
static int factionL_logoTiny( lua_State *L );
static int factionL_colour( lua_State *L );
static int factionL_isknown( lua_State *L );
static int factionL_setknown( lua_State *L );
static const luaL_reg faction_methods[] = {
   { "get", factionL_get },
   { "__eq", factionL_eq },
   { "__tostring", factionL_name },
   { "name", factionL_name },
   { "longname", factionL_longname },
   { "areEnemies", factionL_areenemies },
   { "areAllies", factionL_areallies },
   { "modPlayer", factionL_modplayer },
   { "modPlayerSingle", factionL_modplayersingle },
   { "modPlayerRaw", factionL_modplayerraw },
   { "setPlayerStanding", factionL_setplayerstanding },
   { "playerStanding", factionL_playerstanding },
   { "enemies", factionL_enemies },
   { "allies", factionL_allies },
   { "logoSmall", factionL_logoSmall },
   { "logoTiny", factionL_logoTiny },
   { "colour", factionL_colour },
   { "known", factionL_isknown },
   { "setKnown", factionL_setknown },
   {0,0}
}; /**< Faction metatable methods. */
static const luaL_reg faction_methods_cond[] = {
   { "get", factionL_get },
   { "__eq", factionL_eq },
   { "__tostring", factionL_name },
   { "name", factionL_name },
   { "longname", factionL_longname },
   { "areEnemies", factionL_areenemies },
   { "areAllies", factionL_areallies },
   { "playerStanding", factionL_playerstanding },
   { "enemies", factionL_enemies },
   { "allies", factionL_allies },
   { "logoSmall", factionL_logoSmall },
   { "logoTiny", factionL_logoTiny },
   { "colour", factionL_colour },
   { "known", factionL_isknown },
   {0,0}
}; /**< Factions read only metatable methods. */


/**
 * @brief Loads the faction library.
 *
 *    @param L State to load faction library into.
 *    @param readonly Load as read only?
 *    @return 0 on success.
 */
int nlua_loadFaction( lua_State *L, int readonly )
{
   /* Create the metatable */
   luaL_newmetatable(L, FACTION_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, (readonly) ? faction_methods_cond : faction_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, FACTION_METATABLE);

   return 0; /* No error */
}


/**
 * @brief Lua bindings to deal with factions.
 *
 * Use like:
 * @code
 * f = faction.get( "Empire" )
 * if f:playerStanding() < 0 then
 *    -- player is hostile to "Empire"
 * end
 * @endcode
 *
 * @luamod faction
 */
/**
 * @brief Gets the faction based on its name.
 *
 * @usage f = faction.get( "Empire" )
 *
 *    @luatparam string name Name of the faction to get.
 *    @luatreturn Faction The faction matching name.
 * @luafunc get( name )
 */
static int factionL_get( lua_State *L )
{
   LuaFaction f;
   const char *name;

   name = luaL_checkstring(L,1);
   f = faction_get(name);
   if (f < 0) {
      NLUA_ERROR(L,"Faction '%s' not found in stack.", name );
      return 0;
   }
   lua_pushfaction(L,f);
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
   return *((LuaFaction*) lua_touserdata(L,ind));
}


/**
 * @brief Gets faction (or faction name) at index, raising an error if type isn't a valid faction.
 *
 *    @param L Lua state to get faction from.
 *    @param ind Index position to find the faction.
 *    @return Faction found at the index in the state.
 */
LuaFaction luaL_validfaction( lua_State *L, int ind )
{
   int id;

   if (lua_isfaction(L,ind))
      id = lua_tofaction(L,ind);
   else if (lua_isstring(L,ind))
      id = faction_get( lua_tostring(L, ind) );
   else {
      luaL_typerror(L, ind, FACTION_METATABLE);
      return 0;
   }

   if (id == -1)
      NLUA_ERROR(L,"Faction '%s' not found in stack.", lua_tostring(L,ind) );

   return id;
}


/**
 * @brief Pushes a faction on the stack.
 *
 *    @param L Lua state to push faction into.
 *    @param faction Faction to push.
 *    @return Newly pushed faction.
 */
LuaFaction* lua_pushfaction( lua_State *L, LuaFaction faction )
{
   LuaFaction *f;
   f = (LuaFaction*) lua_newuserdata(L, sizeof(LuaFaction));
   *f = faction;
   luaL_getmetatable(L, FACTION_METATABLE);
   lua_setmetatable(L, -2);
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

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, FACTION_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief __eq (equality) metamethod for factions.
 *
 * You can use the '=' operator within Lua to compare factions with this.
 *
 * @usage if f == faction.get( "Dvaered" ) then
 *
 *    @luatparam Faction f Faction comparing.
 *    @luatparam Faction comp faction to compare against.
 *    @luatreturn boolean true if both factions are the same.
 * @luafunc __eq( f, comp )
 */
static int factionL_eq( lua_State *L )
{
   int a, b;
   a = luaL_validfaction(L,1);
   b = luaL_validfaction(L,2);
   lua_pushboolean(L, a == b);
   return 1;
}

/**
 * @brief Gets the faction's name.
 *
 * @usage name = f:name()
 *
 *    @luatparam Faction f The faction to get the name of.
 *    @luatreturn string The name of the faction.
 * @luafunc name( f )
 */
static int factionL_name( lua_State *L )
{
   int f;
   f = luaL_validfaction(L,1);
   lua_pushstring(L, faction_name(f));
   return 1;
}

/**
 * @brief Gets the faction's long name.
 *
 * @usage longname = f:longname()
 *    @luatparam Faction f Faction to get long name of.
 *    @luatreturn string The long name of the faction.
 * @luafunc longname( f )
 */
static int factionL_longname( lua_State *L )
{
   int f;
   f = luaL_validfaction(L,1);
   lua_pushstring(L, faction_longname(f));
   return 1;
}

/**
 * @brief Checks to see if f is an enemy of e.
 *
 * @usage if f:areEnemies( faction.get( "Dvaered" ) ) then
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam Faction e Faction to check if is an enemy.
 *    @luatreturn string true if they are enemies, false if they aren't.
 * @luafunc areEnemies( f, e )
 */
static int factionL_areenemies( lua_State *L )
{
   int f, ff;
   f  = luaL_validfaction(L,1);
   ff = luaL_validfaction(L,2);

   lua_pushboolean(L, areEnemies( f, ff ));
   return 1;
}

/**
 * @brief Checks to see if f is an ally of a.
 *
 * @usage if f:areAllies( faction.get( "Pirate" ) ) then
 *
 *    @luatparam Faction f Faction to check against.
 *    @luatparam faction a Faction to check if is an enemy.
 *    @luatreturn boolean true if they are enemies, false if they aren't.
 * @luafunc areAllies( f, a )
 */
static int factionL_areallies( lua_State *L )
{
   int f, ff;
   f  = luaL_validfaction(L,1);
   ff = luaL_validfaction(L,2);

   lua_pushboolean(L, areAllies( f, ff ));
   return 1;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Also modifies standing with allies and enemies of the faction.
 *
 * @usage f:modPlayer( -5 ) -- Lowers faction by 5
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod The modifier to modify faction by.
 * @luafunc modPlayer( f, mod )
 */
static int factionL_modplayer( lua_State *L )
{
   int f;
   double n;

   f = luaL_validfaction(L,1);
   n = luaL_checknumber(L,2);
   faction_modPlayer( f, n, "script" );

   return 0;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Does not affect other faction standings.
 *
 * @usage f:modPlayerSingle( 10 )
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod The modifier to modify faction by.
 * @luafunc modPlayerSingle( f, mod )
 */
static int factionL_modplayersingle( lua_State *L )
{
   int f;
   double n;

   f = luaL_validfaction(L,1);
   n = luaL_checknumber(L,2);
   faction_modPlayerSingle( f, n, "script" );

   return 0;
}

/**
 * @brief Modifies the player's standing with the faction.
 *
 * Does not affect other faction standings and is not processed by the faction
 *  Lua script, so it indicates exactly the amount to be changed.
 *
 * @usage f:modPlayerRaw( 10 )
 *
 *    @luatparam Faction f Faction to modify player's standing with.
 *    @luatparam number mod The modifier to modify faction by.
 * @luafunc modPlayerRaw( f, mod )
 */
static int factionL_modplayerraw( lua_State *L )
{
   int f;
   double n;

   f = luaL_validfaction(L,1);
   n = luaL_checknumber(L,2);
   faction_modPlayerRaw( f, n );

   return 0;
}

/**
 * @brief Sets the player's standing with the faction.
 *
 * @usage f:setPlayerStanding(70) -- Make player an ally
 *
 *    @luatparam Faction f Faction to set the player's standing for.
 *    @luatparam number value Value to set the player's standing to (from -100 to 100).
 * @luafunc setPlayerStanding( f, value )
 */
static int factionL_setplayerstanding( lua_State *L )
{
   int f;
   double n;

   f = luaL_validfaction( L, 1 );
   n = luaL_checknumber( L, 2 );
   faction_setPlayer( f, n );

   return 0;
}

/**
 * @brief Gets the player's standing with the faction.
 *
 * @usage if f:playerStanding() > 70 then -- Player is an ally
 *
 *    @luatparam Faction f Faction to get player's standing with.
 *    @luatreturn number The value of the standing and the human readable string.
 * @luafunc playerStanding( f )
 */
static int factionL_playerstanding( lua_State *L )
{
   int f;
   double n;

   f = luaL_validfaction( L, 1 );
   n = faction_getPlayer( f );

   lua_pushnumber( L, n );
   lua_pushstring( L, faction_getStandingText( f ) );

   return 2;
}

/**
 * @brief Gets the enemies of the faction.
 *
 * @usage for k,v in pairs(f:enemies()) do -- Iterates over enemies
 *
 *    @luatparam Faction f Faction to get enemies of.
 *    @luatreturn {Faction,...} A table containing the enemies of the faction.
 * @luafunc enemies( f )
 */
static int factionL_enemies( lua_State *L )
{
   int i, n, f;
   int *factions;

   f = luaL_validfaction(L,1);

   /* Push the enemies in a table. */
   lua_newtable(L);
   factions = faction_getEnemies( f, &n );
   for (i=0; i<n; i++) {
      lua_pushnumber(L, i+1); /* key */
      lua_pushfaction(L, factions[i]); /* value */
      lua_rawset(L, -3);
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
 * @luafunc allies( f )
 */
static int factionL_allies( lua_State *L )
{
   int i, n, f;
   int *factions;

   f = luaL_validfaction(L,1);

   /* Push the enemies in a table. */
   lua_newtable(L);
   factions = faction_getAllies( f, &n );
   for (i=0; i<n; i++) {
      lua_pushnumber(L, i+1); /* key */
      lua_pushfaction(L, factions[i]); /* value */
      lua_rawset(L, -3);
   }

   return 1;
}


/**
 * @brief Gets the small faction logo which is 64x64 or smaller.
 *
 *    @luatparam Faction f Faction to get logo from.
 *    @luatreturn Tex The small faction logo or nil if not applicable.
 * @luafunc logoSmall( f )
 */
static int factionL_logoSmall( lua_State *L )
{
   int lf;
   glTexture *tex;
   lf = luaL_validfaction(L,1);
   tex = faction_logoSmall( lf );
   if (tex == NULL)
      return 0;
   lua_pushtex( L, gl_dupTexture( tex ) );
   return 1;
}


/**
 * @brief Gets the tiny faction logo which is 24x24 or smaller.
 *
 *    @luatparam Faction f Faction to get logo from.
 *    @luatreturn Tex The tiny faction logo or nil if not applicable.
 * @luafunc logoTiny( f )
 */
static int factionL_logoTiny( lua_State *L )
{
   int lf;
   glTexture *tex;
   lf = luaL_validfaction(L,1);
   tex = faction_logoTiny( lf );
   if (tex == NULL)
      return 0;
   lua_pushtex( L, gl_dupTexture( tex ) );
   return 1;
}


/**
 * @brief Gets the faction colour.
 *
 *    @luatparam Faction f Faction to get colour from.
 *    @luatreturn Colour|nil The faction colour or nil if not applicable.
 * @luafunc colour( f )
 */
static int factionL_colour( lua_State *L )
{
   int lf;
   const glColour *col;
   lf = luaL_validfaction(L,1);
   col = faction_getColour(lf);
   if (col == NULL)
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
 * @luafunc known( f )
 */
static int factionL_isknown( lua_State *L )
{
   int fac = luaL_validfaction(L, 1);
   lua_pushboolean(L, faction_isKnown(fac));
   return 1;
}


/**
 * @brief Sets a faction's known state.
 *
 * @usage f:setKnown( false ) -- Makes faction unknown.
 *    @luatparam Faction f Faction to set known.
 *    @luatparam[opt=false] boolean b Whether or not to set as known.
 * @luafunc setKnown( f, b )
 */
static int factionL_setknown( lua_State *L )
{
   int b, fac;

   fac = luaL_validfaction(L, 1);
   b   = lua_toboolean(L, 2);

   faction_setKnown( fac, b );

   return 0;
}

