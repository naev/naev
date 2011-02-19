/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_player.c
 *
 * @brief Lua player module.
 */


#include "nlua_player.h"

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
#include "comm.h"
#include "land_outfits.h"
#include "gui.h"
#include "gui_omsg.h"


/* Player methods. */
static int playerL_getname( lua_State *L );
static int playerL_shipname( lua_State *L );
static int playerL_pay( lua_State *L );
static int playerL_credits( lua_State *L );
static int playerL_msg( lua_State *L );
static int playerL_omsgAdd( lua_State *L );
static int playerL_omsgChange( lua_State *L );
static int playerL_omsgRm( lua_State *L );
/* Faction stuff. */
static int playerL_modFaction( lua_State *L );
static int playerL_modFactionRaw( lua_State *L );
static int playerL_getFaction( lua_State *L );
static int playerL_getRating( lua_State *L );
static int playerL_getPosition( lua_State *L );
static int playerL_getPilot( lua_State *L );
/* Fuel stuff. */
static int playerL_fuel( lua_State *L );
static int playerL_refuel( lua_State *L );
static int playerL_autonav( lua_State *L );
static int playerL_autonavDest( lua_State *L );
/* Cinematics. */
static int playerL_cinematics( lua_State *L );
/* Board stuff. */
static int playerL_unboard( lua_State *L );
/* Land stuff. */
static int playerL_takeoff( lua_State *L );
static int playerL_allowLand( lua_State *L );
/* Hail stuff. */
static int playerL_commclose( lua_State *L );
/* Cargo stuff. */
static int playerL_addOutfit( lua_State *L );
static int playerL_addShip( lua_State *L );
static int playerL_misnActive( lua_State *L );
static int playerL_misnDone( lua_State *L );
static int playerL_evtActive( lua_State *L );
static int playerL_evtDone( lua_State *L );
static int playerL_teleport( lua_State *L );
static const luaL_reg playerL_methods[] = {
   { "name", playerL_getname },
   { "ship", playerL_shipname },
   { "pay", playerL_pay },
   { "credits", playerL_credits },
   { "msg", playerL_msg },
   { "omsgAdd", playerL_omsgAdd },
   { "omsgChange", playerL_omsgChange },
   { "omsgRm", playerL_omsgRm },
   { "modFaction", playerL_modFaction },
   { "modFactionRaw", playerL_modFactionRaw },
   { "getFaction", playerL_getFaction },
   { "getRating", playerL_getRating },
   { "pos", playerL_getPosition },
   { "pilot", playerL_getPilot },
   { "fuel", playerL_fuel },
   { "refuel", playerL_refuel },
   { "autonav", playerL_autonav },
   { "autonavDest", playerL_autonavDest },
   { "cinematics", playerL_cinematics },
   { "unboard", playerL_unboard },
   { "takeoff", playerL_takeoff },
   { "allowLand", playerL_allowLand },
   { "commClose", playerL_commclose },
   { "addOutfit", playerL_addOutfit },
   { "addShip", playerL_addShip },
   { "misnActive", playerL_misnActive },
   { "misnDone", playerL_misnDone },
   { "evtActive", playerL_evtActive },
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
   { "pos", playerL_getPosition },
   { "pilot", playerL_getPilot },
   { "fuel", playerL_fuel },
   { "autonav", playerL_autonav },
   { "autonavDest", playerL_autonavDest },
   { "misnActive", playerL_misnActive },
   { "misnDone", playerL_misnDone },
   { "evtActive", playerL_evtActive },
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
 * @usage monies, readable = player.credits( 2 )
 *
 *    @luaparam decimal Optional argument that makes it return human readable form with so many decimals.
 *    @luareturn The amount of credits the player has on him in both numerical and human-readable form.
 * @luafunc credits( decimal )
 */
static int playerL_credits( lua_State *L )
{
   char buf[ ECON_CRED_STRLEN ];
   int has_dec, decimals;

   /* Parse parameters. */
   if (lua_isnumber(L,1)) {
      has_dec  = 1;
      decimals = lua_tointeger(L,1);
   }
   else
      has_dec  = 0;

   /* Push return. */
   lua_pushnumber(L, player.p->credits);
   if (has_dec) {
      credits2str( buf, player.p->credits, decimals );
      lua_pushstring(L, buf);
   }

   return 1 + has_dec;
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
 * @brief Adds an overlay message.
 *
 * @usage player.omsgAdd( "some_message", 5 )
 *    @luaparam msg Message to add.
 *    @luaparam duration Duration to add message.
 *    @luaparam fontsize Size of the font to use, optional parameter.
 *    @luareturn ID of the created overlay message.
 * @luafunc omsgAdd( msg, duration, fontsize )
 */
static int playerL_omsgAdd( lua_State *L )
{
   const char *str;
   double duration;
   unsigned int id;
   int fontsize;

   /* Input. */
   str      = luaL_checkstring(L,1);
   duration = luaL_checknumber(L,2);
   if (lua_gettop(L) > 2)
      fontsize = luaL_checkint(L,3);
   else
      fontsize = OMSG_FONT_DEFAULT_SIZE;

   /* Output. */
   id       = omsg_add( str, duration, fontsize );
   lua_pushnumber( L, id );
   return 1;
}
/**
 * @brief Changes an overlay message.
 *
 * @usage player.omsgChange( omsg_id, "new message", 3 )
 *    @luaparam id ID of the overlay message to change.
 *    @luaparam msg Message to change to.
 *    @luaparam duration New duration to set.
 *    @luareturn true if all went well, false otherwise.
 * @luafunc omsgChange( id, msg, duration )
 */
static int playerL_omsgChange( lua_State *L )
{
   const char *str;
   double duration;
   unsigned int id;
   int ret;

   /* Input. */
   id       = luaL_checklong(L,1);
   str      = luaL_checkstring(L,2);
   duration = luaL_checknumber(L,3);

   /* Output. */
   ret      = omsg_change( id, str, duration );
   lua_pushboolean(L,!ret);
   return 1;
}
/**
 * @brief Removes an overlay message.
 *
 * @usage player.omsgRm( msg_id )
 *    @luaparam id ID of the overlay message to remove.
 * @luafunc omsgRm( id )
 */
static int playerL_omsgRm( lua_State *L )
{
   unsigned int id;
   id       = luaL_checklong(L,1);
   omsg_rm( id );
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

   if (lua_isstring(L,1))
      f = faction_get( lua_tostring(L,1) );
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

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

   if (lua_isstring(L,1))
      f = faction_get( lua_tostring(L,1) );
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

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

   if (lua_isstring(L,1))
      f = faction_get( lua_tostring(L,1) );
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

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
 * @brief Checks to see if the player has autonav enabled.
 *
 * @usage autonav = player.autonav()
 *    @luareturn true if the player has autonav enabled.
 * @luafunc autonav()
 */
static int playerL_autonav( lua_State *L )
{
   lua_pushboolean( L, player_isFlag( PLAYER_AUTONAV ) );
   return 1;
}


/**
 * @brief Gets the player's long term autonav destination.
 *
 * @usage sys = player.autonavDest()
 *
 *    @luareturn The system the player wants to get to or nil if none selected.
 * @luafunc autonavDest()
 */
static int playerL_autonavDest( lua_State *L )
{
   LuaSystem ls;
   StarSystem *dest;

   /* Get destination. */
   dest = map_getDestination();
   if (dest == NULL)
      return 0;

   ls.id = system_index( dest );
   lua_pushsystem( L, ls );
   return 1;
}


/**
 * @brief Puts the game in cinematics mode or back to regular mode.
 *
 * Possible options are:<br/>
 * <ul>
 *  <li>abort : (string) autonav abort message
 *  <li>2x : (boolean) allows the player to enable doublespeed to "skip", default disabled
 *  <li>gui : (boolean) enables the player's gui, default disabled
 * </ul>
 *
 * @usage player.cinematics( true, { gui = true } ) -- Enables cinematics without hiding gui.
 *
 *    @luaparam enable If true sets cinematics mode, if false disables. Defaults to disable.
 *    @luaparam options Table of options.
 * @luafunc cinematics( enable, options )
 */
static int playerL_cinematics( lua_State *L )
{
   int b;
   const char *abort_msg;
   int f_gui, f_2x;

   /* Defaults. */
   abort_msg = NULL;

   /* Parse parameters. */
   b = lua_toboolean( L, 1 );
   if (lua_gettop(L) > 1) {
      if (!lua_istable(L,2)) {
         NLUA_ERROR( L, "Second parameter to cinematics should be a table of options or ommitted!" );
         return 0;
      }

      lua_getfield( L, 2, "abort" );
      if (!lua_isnil( L, -1 ))
         abort_msg = luaL_checkstring( L, -1 );
      lua_pop( L, 1 );

      lua_getfield( L, 2, "gui" );
      f_gui = lua_toboolean(L, -1);
      lua_pop( L, 1 );

      lua_getfield( L, 2, "2x" );
      f_2x = lua_toboolean(L, -1);
      lua_pop( L, 1 );
   }

   if (b) {
      /* Do stuff. */
      player_autonavAbort( abort_msg );
      player_rmFlag( PLAYER_DOUBLESPEED );

      if (f_gui)
         player_setFlag( PLAYER_CINEMATICS_GUI );
   
      if (f_2x)
         player_setFlag( PLAYER_CINEMATICS_2X );
   }
   else {
      player_rmFlag( PLAYER_CINEMATICS_GUI );
      player_rmFlag( PLAYER_CINEMATICS_2X );
   }

   return 0;
}


/**
 * @brief Unboards the player from its boarded target.
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
 * @brief Allows or disallows the player to land.
 *
 * This will allow or disallow landing on a system level and is reset when the
 *  player enters another system.
 *
 * @usage player.allowLand() -- Allows the player to land
 * @usage player.allowLand( false ) -- Doesn't allow the player to land.
 * @usage player.allowLand( false, "No landing." ) -- Doesn't allow the player to land with the message "No landing."
 *
 *    @luaparam b Whether or not to allow the player to land (defaults to true if ommitted).
 *    @luaparam msg Message displayed when player tries to land (only if disallowed to land). Can be ommitted to use default.
 * @luafunc allowLand( b, msg )
 */
static int playerL_allowLand( lua_State *L )
{
   int b;
   const char *str;
   
   str = NULL;
   if (lua_gettop(L) > 0) {
      b     = lua_toboolean(L,1);
      if (lua_isstring(L,2))
         str   = lua_tostring(L,2);
   }
   else
      b     = 1;

   if (b)
      player_rmFlag( PLAYER_NOLAND );
   else {
      player_setFlag( PLAYER_NOLAND );
      player_nolandMsg( str );
   }
   return 0;
}


/**
 * @brief Forces the player to close comm if he is chatting.
 *
 * @luafunc commClose()
 */
static int playerL_commclose( lua_State *L )
{
   (void) L;
   comm_queueClose();
   return 0;
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

   /* Update equipment list. */
   outfits_updateEquipmentOutfits();

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
 *    @luaparam name Name to give the ship if player refuses to name it (defaults to shipname if ommitted).
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
   if (lua_isstring(L, 2))
      name = lua_tostring(L, 2);
   else
      name = str;

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
 * @brief Checks to see if the player has a mission active.
 *
 * @usage if player.misnActive( "The Space Family" ) then -- Player is doing space family mission
 *
 *    @luaparam name Name of the mission to check.
 *    @luareturn true if the mission is active, false if it isn't.
 * @luafunc misnActive( name )
 */
static int playerL_misnActive( lua_State *L )
{
   MissionData *misn;
   const char *str;

   str  = luaL_checkstring(L,1);
   misn = mission_getFromName( str );
   if (misn == NULL) {
      NLUA_ERROR(L, "Mission '%s' not found in stack", str);
      return 0;
   }

   lua_pushboolean( L, mission_alreadyRunning( misn ) );
   return 1;
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
 * @brief Checks to see if the player has an event active.
 *
 * @usage if player.evtActive( "Shipwreck" ) then -- The shipwreck event is active
 *
 *    @luaparam name Name of the mission to check.
 *    @luareturn true if the mission is active, false if it isn't.
 * @luafunc evtActive( name )
 */
static int playerL_evtActive( lua_State *L )
{
   int evtid;
   const char *str;

   str  = luaL_checkstring(L,1);
   evtid = event_dataID( str );
   if (evtid < 0) {
      NLUA_ERROR(L, "Event '%s' not found in stack", str);
      return 0;
   }

   lua_pushboolean( L, event_alreadyRunning( evtid ) );
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
      name  = system_getIndex(sys->id)->name;
   }
   else if (lua_isstring(L,1))
      name = lua_tostring(L,1);
   else
      NLUA_INVALID_PARAMETER(L);

   /* Check if system exists. */
   if (!system_exists( name )) {
      NLUA_ERROR( L, "System '%s' does not exist.", name );
      return 0;
   }

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
   player_targetPlanetSet( -1 );
   player_targetHyperspaceSet( -1 );
   gui_setNav();
   return 0;
}


