/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_player.c
 *
 * @brief Lua player module.
 */


#include "nlua_misn.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_pilot.h"
#include "nlua_vec2.h"
#include "log.h"
#include "player.h"


/* player */
static int player_getname( lua_State *L );
static int player_shipname( lua_State *L );
static int player_freeSpace( lua_State *L );
static int player_pay( lua_State *L );
static int player_msg( lua_State *L );
static int player_modFaction( lua_State *L );
static int player_modFactionRaw( lua_State *L );
static int player_getFaction( lua_State *L );
static int player_getRating( lua_State *L );
static int player_getPosition( lua_State *L );
static int player_getPilot( lua_State *L );
static const luaL_reg player_methods[] = {
   { "name", player_getname },
   { "ship", player_shipname },
   { "freeCargo", player_freeSpace },
   { "pay", player_pay },
   { "msg", player_msg },
   { "modFaction", player_modFaction },
   { "modFactionRaw", player_modFactionRaw },
   { "getFaction", player_getFaction },
   { "getRating", player_getRating },
   { "pos", player_getPosition },
   { "pilot", player_getPilot },
   {0,0}
}; /**< Player lua methods. */
static const luaL_reg player_cond_methods[] = {
   { "name", player_getname },
   { "ship", player_shipname },
   { "getFaction", player_getFaction },
   { "getRating", player_getRating },
   {0,0}
}; /**< Conditional player lua methods. */


/**
 * @brief Loads the player lua library.
 *    @param L Lua state.
 *    @param readonly Whether to open in read-only form.
 */
int lua_loadPlayer( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "player", player_methods);
   else
      luaL_register(L, "player", player_cond_methods);
   return 0;
}  


/**
 * @brief Lua bindings to interact with the player.
 *
 * These bindings let you modify stuff about the player and find out special
 *  information. General usage would be calls like:
 * @code
 * pname = player.name()
 * shipname = player.ship()
 * freecargo = player.freeCargo()
 * rating = player.getRating()
 * @endcode
 * @luamod player
 */
/**
 * @brief Gets the player's name.
 *
 *    @luareturn The name of the player.
 * @luafunc name()
 */
static int player_getname( lua_State *L )
{
   lua_pushstring(L,player_name);
   return 1;
}
/**
 * @brief Gets the player's ship's name.
 *
 *    @luareturn The name of the ship the player is currently in.
 * @luafunc ship()
 */
static int player_shipname( lua_State *L )
{
   lua_pushstring(L,player->name);
   return 1;
}
/**
 * @brief Gets the free cargo space the player has.
 *
 *    @luareturn The free cargo space in tons of the player.
 * @brief freeCargo()
 */
static int player_freeSpace( lua_State *L )
{
   lua_pushnumber(L, pilot_cargoFree(player) );
   return 1;
}
/**
 * @brief Pays the player an amount of money.
 *
 *    @luaparam amount Amount of money to pay the player in credits.
 * @luafunc pay( amount )
 */
static int player_pay( lua_State *L )
{
   int money;

   money = luaL_checkint(L,1);
   player->credits += money;

   return 0;
}
/**
 * @brief Sends the player an ingame message.
 *
 *    @luaparam message Message to send the player.
 * @luafunc msg( message )
 */
static int player_msg( lua_State *L )
{
   const char* str;

   str = luaL_checkstring(L,1);
   player_messageRaw(str);

   return 0;
}
/**
 * @brief Increases the player's standing to a faction by an amount.  This will
 *  affect player's standing with that faction's allies and enemies also.
 *
 *    @luaparam faction Name of the faction.
 *    @luaparam mod Amount to modify standing by.
 * @luafunc modFaction( faction, mod )
 */
static int player_modFaction( lua_State *L )
{
   int f;
   double mod;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   mod = luaL_checknumber(L,2);
   faction_modPlayer( f, mod );

   return 0;
}
/**
 * @brief Increases the player's standing to a faction by a fixed amount without
 *  touching other faction standings.
 *
 *    @luaparam faction Name of the faction.
 *    @luaparam mod Amount to modify standing by.
 * @luafunc modFactionRaw( faction, mod )
 */
static int player_modFactionRaw( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int f;
   double mod;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();
   mod = luaL_checknumber(L,2);
   faction_modPlayerRaw( f, mod );

   return 0;
}
/**
 * @brief Gets the standing of the player with a certain faction.
 *
 *    @luaparam faction Faction to get the standing of.
 *    @luareturn The faction standing.
 * @luafunc getFaction( faction )
 */
static int player_getFaction( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   int f;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   lua_pushnumber(L, faction_getPlayer(f));

   return 1;
}
/**
 * @brief Gets the player's combat rating.
 *
 *    @luareturn Returns the combat rating (in raw number) and the actual
 *             standing in human readable form.
 * @luafunc getRating()
 */
static int player_getRating( lua_State *L )
{
   lua_pushnumber(L, player_crating);
   lua_pushstring(L, player_rating());
   return 2;
}

/**
 * @brief Gets the player's position.
 *
 * @usage v = player.pos()
 *
 *    @luareturn The position of the player (Vec2).
 * @luafunc pos()
 */
static int player_getPosition( lua_State *L )
{
   LuaVector v;

   vectcpy( &v.vec, &player->solid->pos );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Gets the player's associated pilot.
 *
 *    @luareturn The player's pilot.
 * @luafunc pilot()
 */
static int player_getPilot( lua_State *L )
{
   LuaPilot lp;
   lp.pilot = PLAYER_ID;
   lua_pushpilot(L, lp);
   return 1;
}

