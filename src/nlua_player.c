/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_player.c
 *
 * @brief Lua player module.
 */


#include "nlua_misn.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_space.h"
#include "nlua_pilot.h"
#include "log.h"
#include "naev.h"
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
 * @defgroup PLAYER Player Lua bindings
 *
 * @brief Lua bindings to interact with the player.
 *
 * Functions should be called like:
 *
 * @code
 * player.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @brief string name( nil )
 *
 * Gets the player's name.
 *
 *    @return The name of the player.
 */
static int player_getname( lua_State *L )
{
   lua_pushstring(L,player_name);
   return 1;
}
/**
 * @brief string ship( nil )
 *
 * Gets the player's ship's name.
 *
 *    @return The name of the ship the player is currently in.
 */
static int player_shipname( lua_State *L )
{
   lua_pushstring(L,player->name);
   return 1;
}
/**
 * @brief number freeCargo( nil )
 *
 * Gets the free cargo space the player has.
 *
 *    @return The free cargo space in tons of the player.
 */
static int player_freeSpace( lua_State *L )
{
   lua_pushnumber(L, pilot_cargoFree(player) );
   return 1;
}
/**
 * @brief pay( number amount )
 *
 * Pays the player an amount of money.
 *
 *    @param amount Amount of money to pay the player in credits.
 */
static int player_pay( lua_State *L )
{
   int money;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) money = (int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   player->credits += money;

   return 0;
}
/**
 * @brief msg( string message )
 *
 * Sends the player an ingame message.
 *
 *    @param message Message to send the player.
 */
static int player_msg( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   char* str;

   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else NLUA_INVALID_PARAMETER();

   player_message(str);
   return 0;
}
/**
 * @brief modFaction( string faction, number mod )
 *
 * Increases the player's standing to a faction by an amount.  This will
 *  affect player's standing with that faction's allies and enemies also.
 *
 *    @param faction Name of the faction.
 *    @param mod Amount to modify standing by.
 */
static int player_modFaction( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int f;
   double mod;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   if (lua_isnumber(L,2)) mod = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   faction_modPlayer( f, mod );

   return 0;
}
/**
 * @brief modFactionRaw( string faction, number mod )
 *
 * Increases the player's standing to a faction by a fixed amount without
 *  touching other faction standings.
 *
 *    @param faction Name of the faction.
 *    @param mod Amount to modify standing by.
 */
static int player_modFactionRaw( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int f;
   double mod;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   if (lua_isnumber(L,2)) mod = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   faction_modPlayerRaw( f, mod );

   return 0;
}
/**
 * @brief number getFaction( string faction )
 *
 * Gets the standing of the player with a certain faction.
 *
 *    @param faction Faction to get the standing of.
 *    @return The faction standing.
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
 * @brief number, string getRating( nil )
 *
 * Gets the player's combat rating.
 *
 *    @return Returns the combat rating (in raw number) and the actual
 *             standing in human readable form.
 */
static int player_getRating( lua_State *L )
{
   lua_pushnumber(L, player_crating);
   lua_pushstring(L, player_rating());
   return 2;
}

/**
 * @brief Vec2 getPos( nil )
 *
 * Gets the player's position.
 *
 *    @return The position of the player.
 */
static int player_getPosition( lua_State *L )
{
   LuaVector v;

   vectcpy( &v.vec, &player->solid->pos );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Pilot getPilot( nil )
 *
 * Gets the player's associated pilot.
 *
 *    @return The player's pilot.
 */
static int player_getPilot( lua_State *L )
{
   LuaPilot lp;
   lp.pilot = PLAYER_ID;
   lua_pushpilot(L, lp);
   return 1;
}
/**
 * @}
 */

