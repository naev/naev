/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 *    @luaparam name Name of the faction to get.
 *    @luareturn The faction matching name.
 * @luafunc get( name )
 */
static int factionL_get( lua_State *L )
{
   LuaFaction f;
   const char *name;

   name = luaL_checkstring(L,1);
   f.f = faction_get(name);
   if (f.f < 0) {
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
LuaFaction* lua_tofaction( lua_State *L, int ind )
{
   return (LuaFaction*) lua_touserdata(L,ind);
}


/**
 * @brief Gets faction (or faction name) at index, raising an error if type isn't a valid faction.
 *
 *    @param L Lua state to get faction from.
 *    @param ind Index position to find the faction.
 *    @return Faction found at the index in the state.
 */
int luaL_validfaction( lua_State *L, int ind )
{
   int id;

   if (lua_isfaction(L,ind))
      id = lua_tofaction(L,ind)->f;
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
 *    @luaparam f Faction comparing.
 *    @luaparam comp faction to compare against.
 *    @luareturn true if both factions are the same.
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
 *    @luaparam f The faction to get the name of.
 *    @luareturn The name of the faction.
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
 *    @luaparam f Faction to get long name of.
 *    @luareturn The long name of the faction.
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
 *    @luaparam f Faction to check against.
 *    @luaparam e Faction to check if is an enemy.
 *    @luareturn true if they are enemies, false if they aren't.
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
 *    @luaparam f Faction to check against.
 *    @luaparam a Faction to check if is an enemy.
 *    @luareturn true if they are enemies, false if they aren't.
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
 *    @luaparam f Faction to modify player's standing with.
 *    @luaparam mod The modifier to modify faction by.
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
 *    @luaparam f Faction to modify player's standing with.
 *    @luaparam mod The modifier to modify faction by.
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
 *    @luaparam f Faction to modify player's standing with.
 *    @luaparam mod The modifier to modify faction by.
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
 *    @luaparam f Faction to set the player's standing for.
 *    @luaparam value Value to set the player's standing to (from -100 to 100).
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
 *    @luaparam f Faction to get player's standing with.
 *    @luareturn The value of the standing and the human readable string.
 * @luafunc playerStanding( f )
 */
static int factionL_playerstanding( lua_State *L )
{
   int f;
   double n;

   f = luaL_validfaction(L,1);
   n = faction_getPlayer(f);

   lua_pushnumber(L, n);
   lua_pushstring(L, faction_getStanding(n));

   return 2;
}

/**
 * @brief Gets the enemies of the faction.
 *
 * @usage for k,v in pairs(f:enemies()) do -- Iterates over enemies
 *
 *    @luaparam f Faction to get enemies of.
 *    @luareturn A table containing the enemies of the faction.
 * @luafunc enemies( f )
 */
static int factionL_enemies( lua_State *L )
{
   int i, n, f;
   int *factions;
   LuaFaction fe;

   f = luaL_validfaction(L,1);

   /* Push the enemies in a table. */
   lua_newtable(L);
   factions = faction_getEnemies( f, &n );
   for (i=0; i<n; i++) {
      lua_pushnumber(L, i+1); /* key */
      fe.f = factions[i];
      lua_pushfaction(L, fe); /* value */
      lua_rawset(L, -3);
   }

   return 1;
}

/**
 * @brief Gets the allies of the faction.
 *
 * @usage for k,v in pairs(f:allies()) do -- Iterate over faction allies
 *
 *    @luaparam f Faction to get allies of.
 *    @luareturn A table containing the allies of the faction.
 * @luafunc allies( f )
 */
static int factionL_allies( lua_State *L )
{
   int i, n, f;
   int *factions;
   LuaFaction fa;

   f = luaL_validfaction(L,1);

   /* Push the enemies in a table. */
   lua_newtable(L);
   factions = faction_getAllies( f, &n );
   for (i=0; i<n; i++) {
      lua_pushnumber(L, i+1); /* key */
      fa.f = factions[i];
      lua_pushfaction(L, fa); /* value */
      lua_rawset(L, -3);
   }

   return 1;
}


/**
 * @brief Gets the small faction logo which is 64x64 or smaller.
 *
 *    @luaparam f Faction to get logo from.
 *    @luareturn The small faction logo or nil if not applicable.
 * @luafunc logoSmall( f )
 */
static int factionL_logoSmall( lua_State *L )
{
   int lf;
   LuaTex lt;
   glTexture *tex;
   lf = luaL_validfaction(L,1);
   tex = faction_logoSmall( lf );
   if (tex == NULL)
      return 0;
   lt.tex = gl_dupTexture( tex );
   lua_pushtex( L, lt );
   return 1;
}


/**
 * @brief Gets the tiny faction logo which is 24x24 or smaller.
 *
 *    @luaparam f Faction to get logo from.
 *    @luareturn The tiny faction logo or nil if not applicable.
 * @luafunc logoTiny( f )
 */
static int factionL_logoTiny( lua_State *L )
{
   int lf;
   LuaTex lt;
   glTexture *tex;
   lf = luaL_validfaction(L,1);
   tex = faction_logoTiny( lf );
   if (tex == NULL)
      return 0;
   lt.tex = gl_dupTexture( tex );
   lua_pushtex( L, lt );
   return 1;
}


/**
 * @brief Gets the faction colour.
 *
 *    @luaparam f Faction to get colour from.
 *    @luareturn The faction colour or nil if not applicable.
 * @luafunc colour( f )
 */
static int factionL_colour( lua_State *L )
{
   int lf;
   LuaColour lc;
   const glColour *col;
   lf = luaL_validfaction(L,1);
   col = faction_getColour(lf);
   if (col == NULL)
      return 0;
   memcpy( &lc.col, col, sizeof(glColour) );
   lua_pushcolour( L, lc );
   return 1;
}


/**
 * @brief Checks to see if a faction is known by the player.
 *
 * @usage b = f:known()
 *
 *    @luaparam f Faction to check if the player knows.
 *    @luareturn true if the player knows the faction.
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
 *    @luaparam f Faction to set known.
 *    @luaparam b Whether or not to set as known (defaults to false).
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

