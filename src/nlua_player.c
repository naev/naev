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
#include "board.h"
#include "mission.h"
#include "event.h"
#include "land.h"
#include "nlua_system.h"
#include "map.h"
#include "hook.h"


/* player */
static int playerL_getname( lua_State *L );
static int playerL_shipname( lua_State *L );
static int playerL_freeSpace( lua_State *L );
static int playerL_pay( lua_State *L );
static int playerL_credits( lua_State *L );
static int playerL_msg( lua_State *L );
static int playerL_modFaction( lua_State *L );
static int playerL_modFactionRaw( lua_State *L );
static int playerL_getFaction( lua_State *L );
static int playerL_getRating( lua_State *L );
static int playerL_getPosition( lua_State *L );
static int playerL_getPilot( lua_State *L );
static int playerL_fuel( lua_State *L );
static int playerL_refuel( lua_State *L );
static int playerL_unboard( lua_State *L );
static int playerL_takeoff( lua_State *L );
static int playerL_cargoHas( lua_State *L );
static int playerL_cargoAdd( lua_State *L );
static int playerL_cargoRm( lua_State *L );
static int playerL_addOutfit( lua_State *L );
static int playerL_addShip( lua_State *L );
static int playerL_misnDone( lua_State *L );
static int playerL_evtDone( lua_State *L );
static int playerL_teleport( lua_State *L );
static const luaL_reg playerL_methods[] = {
   { "name", playerL_getname },
   { "ship", playerL_shipname },
   { "freeCargo", playerL_freeSpace },
   { "pay", playerL_pay },
   { "credits", playerL_credits },
   { "msg", playerL_msg },
   { "modFaction", playerL_modFaction },
   { "modFactionRaw", playerL_modFactionRaw },
   { "getFaction", playerL_getFaction },
   { "getRating", playerL_getRating },
   { "pos", playerL_getPosition },
   { "pilot", playerL_getPilot },
   { "fuel", playerL_fuel },
   { "refuel", playerL_refuel },
   { "unboard", playerL_unboard },
   { "takeoff", playerL_takeoff },
   { "cargoHas", playerL_cargoHas },
   { "cargoAdd", playerL_cargoAdd },
   { "cargoRm", playerL_cargoRm },
   { "addOutfit", playerL_addOutfit },
   { "addShip", playerL_addShip },
   { "misnDone", playerL_misnDone },
   { "evtDone", playerL_evtDone },
   { "teleport", playerL_teleport },
   {0,0}
}; /**< Player lua methods. */
static const luaL_reg playerL_cond_methods[] = {
   { "name", playerL_getname },
   { "ship", playerL_shipname },
   { "credits", playerL_credits },
   { "getFaction", playerL_getFaction },
   { "getRating", playerL_getRating },
   { "fuel", playerL_fuel },
   { "cargoHas", playerL_cargoHas },
   { "misnDone", playerL_misnDone },
   { "evtDone", playerL_evtDone },
   {0,0}
}; /**< Conditional player lua methods. */


/**
 * @brief Loads the player lua library.
 *    @param L Lua state.
 *    @param readonly Whether to open in read-only form.
 */
int nlua_loadPlayer( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "player", playerL_methods);
   else
      luaL_register(L, "player", playerL_cond_methods);
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
static int playerL_getname( lua_State *L )
{
   lua_pushstring(L,player.name);
   return 1;
}
/**
 * @brief Gets the player's ship's name.
 *
 *    @luareturn The name of the ship the player is currently in.
 * @luafunc ship()
 */
static int playerL_shipname( lua_State *L )
{
   lua_pushstring(L,player.p->name);
   return 1;
}
/**
 * @brief Gets the free cargo space the player has.
 *
 *    @luareturn The free cargo space in tons of the player.
 * @brief freeCargo()
 */
static int playerL_freeSpace( lua_State *L )
{
   lua_pushnumber(L, pilot_cargoFree(player.p) );
   return 1;
}
/**
 * @brief Pays the player an amount of money.
 *
 * @usage player.pay( 500 ) -- Gives player 500 credits
 *
 *    @luaparam amount Amount of money to pay the player in credits.
 * @luafunc pay( amount )
 */
static int playerL_pay( lua_State *L )
{
   int money;

   money = luaL_checkint(L,1);
   player_modCredits( money );

   return 0;
}
/**
 * @brief Gets how many credits the player has on him.
 *
 * @usage monies = player.credits()
 *
 *    @luareturn The amount of credits the player has on him.
 * @luafunc credits()
 */
static int playerL_credits( lua_State *L )
{
   lua_pushnumber(L,player.p->credits);
   return 1;
}
/**
 * @brief Sends the player an ingame message.
 *
 *    @luaparam message Message to send the player.
 * @luafunc msg( message )
 */
static int playerL_msg( lua_State *L )
{
   const char* str;

   str = luaL_checkstring(L,1);
   player_messageRaw(str);

   return 0;
}
/**
 * @brief Increases the player's standing to a faction by an amount. This will
 *  affect player's standing with that faction's allies and enemies also.
 *
 *    @luaparam faction Name of the faction.
 *    @luaparam mod Amount to modify standing by.
 * @luafunc modFaction( faction, mod )
 */
static int playerL_modFaction( lua_State *L )
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
static int playerL_modFactionRaw( lua_State *L )
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
static int playerL_getFaction( lua_State *L )
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
static int playerL_getRating( lua_State *L )
{
   lua_pushnumber(L, player.crating);
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
static int playerL_getPosition( lua_State *L )
{
   LuaVector v;

   vectcpy( &v.vec, &player.p->solid->pos );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Gets the player's associated pilot.
 *
 *    @luareturn The player's pilot.
 * @luafunc pilot()
 */
static int playerL_getPilot( lua_State *L )
{
   LuaPilot lp;
   lp.pilot = PLAYER_ID;
   lua_pushpilot(L, lp);
   return 1;
}


/**
 * @brief Gets the amount of fuel a player has.
 *
 * @usage fuel = player.fuel()
 *
 *    @luareturn The player's fuel.
 * @luafunc fuel()
 */
static int playerL_fuel( lua_State *L )
{
   lua_pushnumber(L,player.p->fuel);
   return 1;
}


/**
 * @brief Refuels the player.
 *
 * @usage player.refuel() -- Refuel fully
 * @usage player.refuel( 200 ) -- Refuels partially
 *
 *    @param fuel Amount of fuel to add, will set to max if nil.
 * @luafunc refuel( fuel )
 */
static int playerL_refuel( lua_State *L )
{
   double f;

   if (lua_gettop(L) > 0) {
      f = luaL_checknumber(L,1);
      player.p->fuel += f;
   }
   else
      player.p->fuel = player.p->fuel_max;

   /* Make sure value is sane. */
   player.p->fuel = CLAMP(0, player.p->fuel_max, player.p->fuel);

   return 0;
}


/**
 * @brief Unboards the player from it's boarded target.
 *
 * Use from inside a board hook.
 *
 * @usage player.unboard()
 *
 * @luafunc unboard()
 */
static int playerL_unboard( lua_State *L )
{
   (void) L;
   board_unboard();
   return 0;
}


/**
 * @brief Forces the player to take off if he is landed.
 *
 * Assume the pilot is still landed until the current running function returns
 *  If you want to create pilots on take off please hook the takeoff/land hooks.
 *
 * @luafunc takeoff()
 */
static int playerL_takeoff( lua_State *L )
{
   (void) L;

   if (landed)
      landed = 0;

   return 0;
}


/**
 * @brief Checks to see how much cargo the player has of a type.
 *
 *    @luaparam type Type of cargo to see how much the player has.
 *    @luareturn The amount of cargo the player has.
 * @luafunc cargoHas( type )
 */
static int playerL_cargoHas( lua_State *L )
{
   const char *str;
   int quantity;
   str = luaL_checkstring( L, 1 );
   quantity = player_cargoOwned( str );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Tries to add cargo to the player's ship.
 *
 * @usage n = player.cargoAdd( "Food", 20 )
 *
 *    @luaparam type Name of the cargo to add.
 *    @luaparam quantity Quantity of the cargo to add.
 *    @luareturn The number of cargo added.
 * @luafunc cargoAdd( type, quantity )
 */
static int playerL_cargoAdd( lua_State *L )
{
   const char *str;
   int quantity;
   Commodity *cargo;

   /* Parse parameters. */
   str      = luaL_checkstring( L, 1 );
   quantity = luaL_checknumber( L, 2 );

   /* Get cargo. */
   cargo    = commodity_get( str );
   if (cargo == NULL) {
      NLUA_ERROR( L, "Cargo '%s' does not exist!", str );
      return 0;
   }

   /* Try to add the cargo. */
   quantity = pilot_addCargo( player.p, cargo, quantity );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Tries to remove cargo from the player's ship.
 *
 * @usage n = player.cargoRm( "Food", 20 )
 *
 *    @luaparam type Name of the cargo to remove.
 *    @luaparam quantity Quantity of the cargo to remove.
 *    @luareturn The number of cargo removed.
 * @luafunc cargoRm( type, quantity )
 */
static int playerL_cargoRm( lua_State *L )
{
   const char *str;
   int quantity;
   Commodity *cargo;

   /* Parse parameters. */
   str      = luaL_checkstring( L, 1 );
   quantity = luaL_checknumber( L, 2 );

   /* Get cargo. */
   cargo    = commodity_get( str );
   if (cargo == NULL) {
      NLUA_ERROR( L, "Cargo '%s' does not exist!", str );
      return 0;
   }

   /* Try to add the cargo. */
   quantity = pilot_rmCargo( player.p, cargo, quantity );
   lua_pushnumber( L, quantity );
   return 1;
}


/**
 * @brief Adds an outfit to the player's outfit list.
 *
 * @usage player.addOutfit( "Laser Cannon" ) -- Gives the player a laser cannon
 * @usage player.addOutfit( "Plasma Blaster", 2 ) -- Gives the player two plasma blasters
 *
 *    @luaparam name Name of the outfit to give.
 *    @luaparam q Optional parameter that sets the quantity to give (default 1).
 * @luafunc addOutfit( name, q )
 */
static int playerL_addOutfit( lua_State *L  )
{
   const char *str;
   Outfit *o;
   int q;

   /* Defaults. */
   q = 1;

   /* Handle parameters. */
   str = luaL_checkstring(L, 1);
   if (lua_gettop(L) > 1)
      q = luaL_checkint(L, 2);

   /* Get outfit. */
   o = outfit_get( str );
   if (o==NULL) {
      NLUA_ERROR(L, "Outfit '%s' not found.", str);
      return 0;
   }

   /* Add the outfits. */
   player_addOutfit( o, q );
   return 0;
}


/**
 * @brief Gives the player a new ship.
 *
 * @note Should be given when landed, ideally on a planet with a shipyard.
 *
 * @usage player.addShip( "Pirate Kestrel", "Seiryuu" ) -- Gives the player a Pirate Kestrel named Seiryuu if player cancels the naming.
 *
 *    @luaparam ship Name of the ship to add.
 *    @luaparam name Name to give the ship if player refuses to name it.
 * @luafunc addShip( ship, name )
 */
static int playerL_addShip( lua_State *L )
{
   const char *str, *name;
   Ship *s;
   int ret;

   /* Must be landed. */
   if (land_planet==NULL) {
      NLUA_ERROR(L, "Player must be landed to add a ship.");
      return 0;
   }

   /* Handle parameters. */
   str  = luaL_checkstring(L, 1);
   name = luaL_checkstring(L, 2);

   /* Get ship. */
   s = ship_get(str);
   if (s==NULL) {
      NLUA_ERROR(L, "Ship '%s' not found.", str);
      return 0;
   }

   /* Add the ship. */
   do {
      ret = player_newShip( s, name, 0 );
   } while (ret != 0);

   return 0;
}


/**
 * @brief Checks to see if player has done a mission.
 *
 * @usage if player.misnDone( "The Space Family" ) then -- Player finished mission
 *
 *    @luaparam name Name of the mission to check.
 *    @luareturn true if mission was finished, false if it wasn't.
 * @luafunc misnDone( name )
 */
static int playerL_misnDone( lua_State *L )
{
   const char *str;
   int id;

   /* Handle parameters. */
   str = luaL_checkstring(L, 1);

   /* Get mission ID. */
   id = mission_getID( str );
   if (id == -1) {
      NLUA_ERROR(L, "Mission '%s' not found in stack", str);
      return 0;
   }

   lua_pushboolean( L, player_missionAlreadyDone( id ) );
   return 1;
}


/**
 * @brief Checks to see if player has done an event.
 *
 * @usage if player.evtDone( "Shipwreck" ) then -- Player finished event
 *
 *    @luaparam name Name of the event to check.
 *    @luareturn true if event was finished, false if it wasn't.
 * @luafunc evtDone( name )
 */
static int playerL_evtDone( lua_State *L )
{
   const char *str;
   int id;

   /* Handle parameters. */
   str = luaL_checkstring(L, 1);

   /* Get event ID. */
   id = event_dataID( str );
   if (id == -1) {
      NLUA_ERROR(L, "Event '%s' not found in stack", str);
      return 0;
   }

   lua_pushboolean( L, player_eventAlreadyDone( id ) );
   return 1;
}


/**
 * @brief Teleports the player to a new system.
 *
 * Does not change the position nor velocity of the player.p, which will probably be wrong in the new system.
 *
 * @usage player.teleport( system.get("Arcanis") ) -- Teleports the player to arcanis.
 * @usage player.teleport( "Arcanis" ) -- Teleports the player to arcanis.
 *
 *    @luaparam sys System or name of a system to teleport the player to.
 * @luafunc teleport( sys )
 */
static int playerL_teleport( lua_State *L )
{
   LuaSystem *sys;
   const char *name;

   /* Get a system. */
   if (lua_issystem(L,1)) {
      sys   = lua_tosystem(L,1);
      name  = sys->s->name;
   }
   else if (lua_isstring(L,1))
      name = lua_tostring(L,1);
   else
      NLUA_INVALID_PARAMETER();

   /* Jump out hook is run first. */
   hooks_run( "jumpout" );

   /* Just in case remove hyperspace flags. */
   pilot_rmFlag( player.p, PILOT_HYPERSPACE );
   pilot_rmFlag( player.p, PILOT_HYP_BEGIN );
   pilot_rmFlag( player.p, PILOT_HYP_PREP );

   /* Free graphics. */
   space_gfxUnload( cur_system );

   /* Go to the new system. */
   space_init( name );

   /* Map gets deformed when jumping this way. */
   map_clear();

   /* Add the escorts. */
   player_addEscorts();

   /* Run hooks - order is important. */
   hooks_run( "jumpin" );
   hooks_run( "enter" );
   events_trigger( EVENT_TRIGGER_ENTER );

   /* Reset targets when teleporting */
   player.p->nav_planet = -1;
   player.p->nav_hyperspace = -1;
   return 0;
}


