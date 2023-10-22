/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_player.c
 *
 * @brief Lua player module.
 */
/** @cond */
#include <lauxlib.h>
#include <lua.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_player.h"

#include "array.h"
#include "board.h"
#include "comm.h"
#include "event.h"
#include "gui.h"
#include "gui_omsg.h"
#include "hook.h"
#include "info.h"
#include "land.h"
#include "land_outfits.h"
#include "log.h"
#include "map.h"
#include "map_overlay.h"
#include "menu.h"
#include "mission.h"
#include "ndata.h"
#include "nlua_colour.h"
#include "nlua_commodity.h"
#include "nlua_outfit.h"
#include "nlua_pilot.h"
#include "nlua_spob.h"
#include "nlua_ship.h"
#include "nlua_system.h"
#include "nlua_time.h"
#include "nlua_vec2.h"
#include "nlua_misn.h"
#include "nluadef.h"
#include "nstring.h"
#include "pause.h"
#include "player.h"
#include "player_fleet.h"
#include "player_inventory.h"
#include "player_gui.h"
#include "save.h"
#include "start.h"

#define PLAYER_CHECK() if (player.p == NULL) return 0

/* Player methods. */
static int playerL_getname( lua_State *L );
static int playerL_shipname( lua_State *L );
static int playerL_pay( lua_State *L );
static int playerL_credits( lua_State *L );
static int playerL_wealth( lua_State *L );
static int playerL_msg( lua_State *L );
static int playerL_msgClear( lua_State *L );
static int playerL_msgToggle( lua_State *L );
static int playerL_omsgAdd( lua_State *L );
static int playerL_omsgChange( lua_State *L );
static int playerL_omsgRm( lua_State *L );
static int playerL_allowSave( lua_State *L );
/* Faction stuff. */
static int playerL_getPosition( lua_State *L );
static int playerL_getPilot( lua_State *L );
/* Fuel stuff. */
static int playerL_jumps( lua_State *L );
static int playerL_fuel( lua_State *L );
static int playerL_refuel( lua_State *L );
static int playerL_autonav( lua_State *L );
static int playerL_autonavSetPos( lua_State *L );
static int playerL_autonavDest( lua_State *L );
static int playerL_autonavAbort( lua_State *L );
static int playerL_autonavReset( lua_State *L );
static int playerL_autonavEnd( lua_State *L );
/* Cinematics. */
static int playerL_dt_default( lua_State *L );
static int playerL_speed( lua_State *L );
static int playerL_setSpeed( lua_State *L );
static int playerL_cinematics( lua_State *L );
static int playerL_damageSPFX( lua_State *L );
static int playerL_screenshot( lua_State *L );
/* Board stuff. */
static int playerL_tryBoard( lua_State *L );
static int playerL_unboard( lua_State *L );
/* Land stuff. */
static int playerL_isLanded( lua_State *L );
static int playerL_takeoff( lua_State *L );
static int playerL_tryLand( lua_State *L );
static int playerL_land( lua_State *L );
static int playerL_landAllow( lua_State *L );
static int playerL_landWindow( lua_State *L );
/* Hail stuff. */
static int playerL_commclose( lua_State *L );
/* Shipvars. */
static int playerL_shipvarPeek( lua_State *L );
static int playerL_shipvarPush( lua_State *L );
static int playerL_shipvarPop( lua_State *L );
/* Outfit and ship management stuff. */
static int playerL_ships( lua_State *L );
static int playerL_shipOutfits( lua_State *L );
static int playerL_shipMetadata( lua_State *L );
static int playerL_shipDeploy( lua_State *L );
static int playerL_outfits( lua_State *L );
static int playerL_outfitNum( lua_State *L );
static int playerL_outfitAdd( lua_State *L );
static int playerL_outfitRm( lua_State *L );
static int playerL_shipAdd( lua_State *L );
static int playerL_shipSwap( lua_State *L );
/* Mission/event management stuff. */
static int playerL_missions( lua_State *L );
static int playerL_misnActive( lua_State *L );
static int playerL_misnDone( lua_State *L );
static int playerL_misnDoneList( lua_State *L );
static int playerL_evtActive( lua_State *L );
static int playerL_evtDone( lua_State *L );
static int playerL_evtDoneList( lua_State *L );
/* Handle GUI. */
static int playerL_gui( lua_State *L );
static int playerL_guiList( lua_State *L );
static int playerL_guiSet( lua_State *L );
/* Fleet stuff. */
static int playerL_fleetList( lua_State *L );
static int playerL_fleetCargoFree( lua_State *L );
static int playerL_fleetCargoUsed( lua_State *L );
static int playerL_fleetCargoOwned( lua_State *L );
static int playerL_fleetCargoAdd( lua_State *L );
static int playerL_fleetCargoRm( lua_State *L );
static int playerL_fleetCargoJet( lua_State *L );
static int playerL_fleetCargoList( lua_State *L );
/* Inventory. */
static int playerL_inventory( lua_State *L );
static int playerL_inventoryAdd( lua_State *L );
static int playerL_inventoryRm( lua_State *L );
static int playerL_inventoryOwned( lua_State *L );
/* Misc stuff. */
static int playerL_teleport( lua_State *L );
static int playerL_dt_mod( lua_State *L );
static int playerL_fleetCapacity( lua_State *L );
static int playerL_fleetCapacitySet( lua_State *L );
static int playerL_chapter( lua_State *L );
static int playerL_chapterSet( lua_State *L );
static int playerL_infoButtonRegister( lua_State *L );
static int playerL_infoButtonUnregister( lua_State *L );
static int playerL_canDiscover( lua_State *L );
static int playerL_save( lua_State *L );
static int playerL_saveBackup( lua_State *L );
static int playerL_gameover( lua_State *L );
static int playerL_start( lua_State *L );
static const luaL_Reg playerL_methods[] = {
   { "name", playerL_getname },
   { "ship", playerL_shipname },
   { "pay", playerL_pay },
   { "credits", playerL_credits },
   { "wealth", playerL_wealth },
   { "msg", playerL_msg },
   { "msgClear", playerL_msgClear },
   { "msgToggle", playerL_msgToggle },
   { "omsgAdd", playerL_omsgAdd },
   { "omsgChange", playerL_omsgChange },
   { "omsgRm", playerL_omsgRm },
   { "allowSave", playerL_allowSave },
   { "pos", playerL_getPosition },
   { "pilot", playerL_getPilot },
   { "jumps", playerL_jumps },
   { "fuel", playerL_fuel },
   { "refuel", playerL_refuel },
   { "autonav", playerL_autonav },
   { "autonavSetPos", playerL_autonavSetPos },
   { "autonavDest", playerL_autonavDest },
   { "autonavAbort", playerL_autonavAbort },
   { "autonavReset", playerL_autonavReset },
   { "autonavEnd", playerL_autonavEnd },
   { "dt_default", playerL_dt_default },
   { "speed", playerL_speed },
   { "setSpeed", playerL_setSpeed },
   { "cinematics", playerL_cinematics },
   { "damageSPFX", playerL_damageSPFX },
   { "screenshot", playerL_screenshot },
   { "tryBoard", playerL_tryBoard },
   { "unboard", playerL_unboard },
   { "isLanded", playerL_isLanded },
   { "takeoff", playerL_takeoff },
   { "tryLand", playerL_tryLand },
   { "land", playerL_land },
   { "landAllow", playerL_landAllow },
   { "landWindow", playerL_landWindow },
   { "commClose", playerL_commclose },
   { "shipvarPeek", playerL_shipvarPeek },
   { "shipvarPush", playerL_shipvarPush },
   { "shipvarPop", playerL_shipvarPop },
   { "ships", playerL_ships },
   { "shipOutfits", playerL_shipOutfits },
   { "shipMetadata", playerL_shipMetadata },
   { "shipDeploy", playerL_shipDeploy },
   { "outfits", playerL_outfits },
   { "outfitNum", playerL_outfitNum },
   { "outfitAdd", playerL_outfitAdd },
   { "outfitRm", playerL_outfitRm },
   { "shipAdd", playerL_shipAdd },
   { "shipSwap", playerL_shipSwap },
   { "missions", playerL_missions },
   { "misnActive", playerL_misnActive },
   { "misnDone", playerL_misnDone },
   { "misnDoneList", playerL_misnDoneList },
   { "evtActive", playerL_evtActive },
   { "evtDone", playerL_evtDone },
   { "evtDoneList", playerL_evtDoneList },
   { "gui", playerL_gui },
   { "guiList", playerL_guiList },
   { "guiSet", playerL_guiSet },
   { "fleetList", playerL_fleetList },
   { "fleetCargoFree", playerL_fleetCargoFree },
   { "fleetCargoUsed", playerL_fleetCargoUsed },
   { "fleetCargoOwned", playerL_fleetCargoOwned },
   { "fleetCargoAdd", playerL_fleetCargoAdd },
   { "fleetCargoRm", playerL_fleetCargoRm },
   { "fleetCargoJet", playerL_fleetCargoJet },
   { "fleetCargoList", playerL_fleetCargoList },
   { "inventory", playerL_inventory },
   { "inventoryAdd", playerL_inventoryAdd },
   { "inventoryRm", playerL_inventoryRm },
   { "inventoryOwned", playerL_inventoryOwned },
   { "teleport", playerL_teleport },
   { "dt_mod", playerL_dt_mod },
   { "fleetCapacity", playerL_fleetCapacity },
   { "fleetCapacitySet", playerL_fleetCapacitySet },
   { "chapter", playerL_chapter },
   { "chapterSet", playerL_chapterSet },
   { "infoButtonRegister", playerL_infoButtonRegister },
   { "infoButtonUnregister", playerL_infoButtonUnregister },
   { "canDiscover", playerL_canDiscover },
   { "save", playerL_save },
   { "saveBackup", playerL_saveBackup },
   { "gameover", playerL_gameover },
   { "start", playerL_start },
   {0,0}
}; /**< Player Lua methods. */

/**
 * @brief Loads the player Lua library.
 *    @param env Lua environment.
 */
int nlua_loadPlayer( nlua_env env )
{
   nlua_register(env, "player", playerL_methods, 0);
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
 * @endcode
 * @luamod player
 */
/**
 * @brief Gets the player's name.
 *
 *    @luatreturn string The name of the player.
 * @luafunc name
 */
static int playerL_getname( lua_State *L )
{
   PLAYER_CHECK();
   lua_pushstring(L,player.name);
   return 1;
}
/**
 * @brief Gets the player's ship's name (given by the player).
 *
 * @note Not to be confused with getting the player's ship that can be done with player.pilot():ship().
 *
 *    @luatreturn string The name of the ship the player is currently in.
 * @luafunc ship
 */
static int playerL_shipname( lua_State *L )
{
   PLAYER_CHECK();
   lua_pushstring(L,player.p->name);
   return 1;
}
/**
 * @brief Pays the player an amount of money.
 *
 * @usage player.pay( 500 ) -- Gives player 500 credits
 *
 *    @luatparam number amount Amount of money to pay the player in credits.
 *    @luatparam[opt] boolean|string nohooks Set to true to not trigger pay hooks, or a string value to pass that to the pay hook instead.
 * @luafunc pay
 */
static int playerL_pay( lua_State *L )
{
   PLAYER_CHECK();

   HookParam p[3];
   credits_t money;
   int nohooks;
   const char *reason;

   money = CLAMP( CREDITS_MIN, CREDITS_MAX, (credits_t)round(luaL_checknumber(L,1)) );
   player_modCredits( money );
   if (lua_isstring(L,2)) {
      nohooks = 0;
      reason = lua_tostring(L,2);
   }
   else {
      nohooks = lua_toboolean(L,2);
      reason = NULL;
   }

   if (!nohooks) {
      p[0].type = HOOK_PARAM_NUMBER;
      p[0].u.num = (double)money;
      if (reason != NULL) {
         p[1].type = HOOK_PARAM_STRING;
         p[1].u.str = reason;
      }
      else {
         p[1].type = HOOK_PARAM_NIL;
      }
      p[2].type = HOOK_PARAM_SENTINEL;
      hooks_runParam( "pay", p );
   }

   return 0;
}

/**
 * @brief Gets how many credits the player has on him.
 *
 * @usage monies = player.credits()
 * @usage monies, readable = player.credits( 2 )
 *
 *    @luatparam[opt] number decimal Optional argument that makes it return human readable form with so many decimals.
 *    @luatreturn number The amount of credits in numerical form.
 *    @luatreturn string The amount of credits in human-readable form.
 * @luafunc credits
 */
static int playerL_credits( lua_State *L )
{
   credits_t creds = (player.p==NULL) ? 0 : player.p->credits;

   /* Parse parameters. */
   int decimals = luaL_optinteger(L,1,-1);

   /* Push return. */
   lua_pushnumber(L, creds);
   if (decimals >= 0) {
      char buf[ ECON_CRED_STRLEN ];
      credits2str( buf, creds, decimals );
      lua_pushstring(L, buf);
      return 2;
   }
   return 1;
}

/**
 * @brief Gets how many credits the player owns both directly, and in the form of assets (ships, outfits, ...).
 *
 * @usage monies = player.wealth()
 * @usage monies, readable = player.wealth( 2 )
 *
 *    @luatparam[opt] number decimal Optional argument that makes it return human readable form with so many decimals.
 *    @luatreturn number The player's wealth in numerical form.
 *    @luatreturn string The player's wealth in human-readable form.
 * @luafunc wealth
 */
static int playerL_wealth( lua_State *L )
{
   credits_t wealth;
   /* Parse parameters. */
   int decimals = luaL_optinteger(L,1,-1);

   if (player.p != NULL) {
      const PlayerShip_t *ps = player_getShipStack();
      const PlayerOutfit_t *po = player_getOutfits();
      wealth = player.p->credits + pilot_worth( player.p, 0 );

      /* Compute total wealth. */
      for (int i=0; i<array_size(ps); i++)
         wealth += pilot_worth( ps[i].p, 0 );
      for (int i=0; i<array_size(po); i++)
         wealth += po[i].q * po[i].o->price;
   }
   else
      wealth = 0;

   /* Push return. */
   lua_pushnumber(L, wealth);
   if (decimals >= 0) {
      char buf[ ECON_CRED_STRLEN ];
      credits2str( buf, wealth, decimals );
      lua_pushstring(L, buf);
      return 2;
   }
   return 1;
}

/**
 * @brief Sends the player an in-game message.
 *
 *    @luatparam string message Message to send the player.
 *    @luatparam[opt=false] boolean display Display the message over the player's ship (like a broadcast message).
 * @luafunc msg
 */
static int playerL_msg( lua_State *L )
{
   PLAYER_CHECK();

   const char *str = luaL_checkstring(L,1);
   int display = lua_toboolean(L,2);
   player_messageRaw(str);
   if (display)
      pilot_setCommMsg( player.p, str );

   return 0;
}
/**
 * @brief Clears the player's message buffer.
 *
 * @luafunc msgClear
 */
static int playerL_msgClear( lua_State *L )
{
   (void) L;
   PLAYER_CHECK();
   gui_clearMessages();
   return 0;
}
/**
 * @brief Clears the player's message buffer.
 *
 *    @luatparam( boolean enable Whether or not to enable the player receiving messages.
 * @luafunc msgToggle
 */
static int playerL_msgToggle( lua_State *L )
{
   PLAYER_CHECK();
   player_messageToggle( lua_toboolean(L,1) );
   return 0;
}
/**
 * @brief Adds an overlay message.
 *
 * @usage player.omsgAdd( "some_message", 5 )
 *    @luatparam string msg Message to add.
 *    @luatparam[opt=10] number duration Duration to add message in seconds (if 0. the duration is infinite).
 *    @luatparam[opt=16] number fontsize Size of the font to use.
 *    @luatparam[opt=white] Colour col Colour to use for the text or white if not specified.
 *    @luatreturn number ID of the created overlay message.
 * @luafunc omsgAdd
 */
static int playerL_omsgAdd( lua_State *L )
{
   const char *str;
   double duration;
   unsigned int id;
   int fontsize;
   const glColour *col;

   PLAYER_CHECK();

   /* Input. */
   str      = luaL_checkstring(L,1);
   duration = luaL_optnumber(L,2,10.);
   fontsize = luaL_optinteger(L,3,OMSG_FONT_DEFAULT_SIZE);
   col      = luaL_optcolour(L,4,&cWhite);

   /* Infinity. */
   if (duration < 1e-10)
      duration = INFINITY;

   /* Output. */
   id       = omsg_add( str, duration, fontsize, col );
   lua_pushnumber( L, id );
   return 1;
}
/**
 * @brief Changes an overlay message.
 *
 * @usage player.omsgChange( omsg_id, "new message", 3 )
 *    @luatparam number id ID of the overlay message to change.
 *    @luatparam string msg Message to change to.
 *    @luatparam number duration New duration to set in seconds (0. for infinity).
 *    @luatreturn boolean true if all went well, false otherwise.
 * @luafunc omsgChange
 */
static int playerL_omsgChange( lua_State *L )
{
   const char *str;
   double duration;
   unsigned int id;
   int ret;

   PLAYER_CHECK();

   /* Input. */
   id       = luaL_checklong(L,1);
   str      = luaL_checkstring(L,2);
   duration = luaL_checknumber(L,3);

   /* Infinity. */
   if (duration < 1e-10)
      duration = INFINITY;

   /* Output. */
   ret      = omsg_change( id, str, duration );
   lua_pushboolean(L,!ret);
   return 1;
}
/**
 * @brief Removes an overlay message.
 *
 * @usage player.omsgRm( msg_id )
 *    @luatparam number id ID of the overlay message to remove.
 * @luafunc omsgRm
 */
static int playerL_omsgRm( lua_State *L )
{
   PLAYER_CHECK();
   unsigned int id = luaL_checklong(L,1);
   omsg_rm( id );
   return 0;
}
/**
 * @brief Sets player save ability.
 *
 * @usage player.allowSave( b )
 *    @luatparam[opt=true] boolean b true if the player is allowed to save, false otherwise.
 * @luafunc allowSave
 */
static int playerL_allowSave( lua_State *L )
{
   PLAYER_CHECK();
   unsigned int b;
   if (lua_gettop(L)==0)
      b = 1;
   else
      b = lua_toboolean(L, 1);

   if (b)
      player_rmFlag(PLAYER_NOSAVE);
   else
      player_setFlag(PLAYER_NOSAVE);
   return 0;
}

/**
 * @brief Gets the player's position.
 *
 * @usage v = player.pos()
 *
 *    @luatreturn Vec2 The position of the player.
 * @luafunc pos
 */
static int playerL_getPosition( lua_State *L )
{
   PLAYER_CHECK();
   lua_pushvector(L, player.p->solid.pos);
   return 1;
}

/**
 * @brief Gets the player's associated pilot.
 *
 *    @luatreturn Pilot The player's pilot.
 * @luafunc pilot
 */
static int playerL_getPilot( lua_State *L )
{
   /* No need to run check here or stuff that depends on player.pilot() working will fail. */
   lua_pushpilot(L, PLAYER_ID);
   return 1;
}

/**
 * @brief Gets a player's jump range based on their remaining fuel.
 *
 * @usage jumps = player.jumps()
 *
 *    @luatreturn number The player's maximum number of jumps.
 * @luafunc jumps
 */
static int playerL_jumps( lua_State *L )
{
   if (player.p == NULL)
      lua_pushnumber(L, 0);
   else
      lua_pushnumber(L, pilot_getJumps(player.p));
   return 1;
}

/**
 * @brief Gets the amount of fuel a player has.
 *
 * @usage fuel, consumption = player.fuel()
 *
 *    @luatreturn number The player's fuel and
 *    @luatreturn number The amount of fuel needed per jump.
 * @luafunc fuel
 */
static int playerL_fuel( lua_State *L )
{
   if (player.p == NULL) {
      lua_pushnumber(L,0);
      lua_pushnumber(L,0);
   }
   else {
      lua_pushnumber(L,player.p->fuel);
      lua_pushnumber(L,player.p->fuel_consumption);
   }
   return 2;
}

/**
 * @brief Refuels the player.
 *
 * @usage player.refuel() -- Refuel fully
 * @usage player.refuel( 200 ) -- Refuels partially
 *
 *    @luatparam[opt] number fuel Amount of fuel to add, will set to max if nil.
 * @luafunc refuel
 */
static int playerL_refuel( lua_State *L )
{
   PLAYER_CHECK();

   if (lua_gettop(L) > 0) {
      double f = luaL_checknumber(L,1);
      player.p->fuel += f;
   }
   else
      player.p->fuel = player.p->fuel_max;

   /* Make sure value is valid. */
   player.p->fuel = CLAMP(0, player.p->fuel_max, player.p->fuel);

   return 0;
}

/**
 * @brief Checks to see if the player has autonav enabled.
 *
 * @usage autonav = player.autonav()
 *    @luatreturn boolean true if the player has autonav enabled.
 * @luafunc autonav
 */
static int playerL_autonav( lua_State *L )
{
   PLAYER_CHECK();
   lua_pushboolean( L, player_isFlag( PLAYER_AUTONAV ) );
   return 1;
}

/**
 * @brief Indicates the player where their autonav target position is.
 *
 *    @luatparam Vec2|nil pos Position to mark as autonav destination or nil to clear.
 * @luafunc autonavSetPos
 */
static int playerL_autonavSetPos( lua_State *L )
{
   const vec2 *pos = luaL_optvector(L,1,NULL);
   if (pos==NULL)
      ovr_autonavClear();
   else
      ovr_autonavPos( pos->x, pos->y );
   return 0;
}

/**
 * @brief Gets the player's long term autonav destination.
 *
 * @usage sys, jumps = player.autonavDest()
 *
 *    @luatreturn System|nil The destination system (or nil if none selected).
 *    @luatreturn number|nil The number of jumps left.
 * @luafunc autonavDest
 */
static int playerL_autonavDest( lua_State *L )
{
   LuaSystem ls;
   StarSystem *dest;
   int jumps;

   /* Get destination. */
   dest = map_getDestination( &jumps );
   if (dest == NULL)
      return 0;

   ls = system_index( dest );
   lua_pushsystem( L, ls );
   lua_pushnumber( L, jumps );
   return 2;
}

/**
 * @brief Stops the players autonav if active.
 *
 * @usage sys, jumps = player.autonavAbort()
 *
 * @note Does not do anything if the player is not in autonav.
 *
 *    @luatparam[opt] string msg Abort message.
 * @luafunc autonavAbort
 */
static int playerL_autonavAbort( lua_State *L )
{
   const char *str = luaL_optstring(L,1,NULL);
   player_autonavAbort( str );
   return 0;
}

/**
 * @brief Resets the game speed without disabling autonav.
 *
 * @note Does not do anything if the player is not in autonav.
 *
 *    @luatparam[opt=0.] number timer How many seconds to wait before starting autonav up again.
 * @luafunc autonavReset
 */
static int playerL_autonavReset( lua_State *L )
{
   double timer = luaL_optnumber(L,1,0.);
   player_autonavReset( timer );
   return 0;
}

/**
 * @brief Ends the autonav system. You probably want to use player.autonavAbort instead of this.
 *
 * @luafunc autonavEnd
 * @see autonavAbort
 */
static int playerL_autonavEnd( lua_State *L )
{
   (void) L;
   player_autonavEnd();
   return 0;
}

static int playerL_dt_default( lua_State *L )
{
   lua_pushnumber( L, player_dt_default() );
   return 1;
}

static int playerL_speed( lua_State *L )
{
   lua_pushnumber( L, player.speed );
   return 1;
}

/**
 * @brief Sets the game speed directly.
 *
 *    @luatparam number speed Speed to set the game to. If omitted it will reset the game speed.
 *    @luatparam[opt=speed] number sound Sound speed to set to.
 *    @luatparam[opt=false] boolean noset Avoid setting player.speed. Useful for autonav.
 * @luafunc setSpeed
 */
static int playerL_setSpeed( lua_State *L )
{
   double speed = luaL_optnumber( L, 1, -1 );
   double sound = luaL_optnumber( L, 2, speed );
   int noset = lua_toboolean( L, 3 );

   if (speed > 0.) {
      if (!noset)
         player.speed = speed;
      pause_setSpeed( speed );
      sound_setSpeed( sound );
   }
   else {
      if (!noset)
         player.speed = 1.;
      player_resetSpeed();
   }

   return 0;
}

/**
 * @brief Puts the game in cinematics mode or back to regular mode.
 *
 * Possible options are:<br/>
 * <ul>
 *  <li>abort : (string) autonav abort message</li>
 *  <li>no2x : (boolean) whether to prevent the player from increasing the speed, default false</li>
 *  <li>gui : (boolean) enables the player's gui, default disabled</li>
 * </ul>
 *
 * @usage player.cinematics( true, { gui = true } ) -- Enables cinematics without hiding gui.
 *
 *    @luatparam boolean enable If true sets cinematics mode, if false disables. Defaults to disable.
 *    @luatparam table options Table of options.
 * @luafunc cinematics
 */
static int playerL_cinematics( lua_State *L )
{
   int b, f_gui, f_2x;
   const char *abort_msg;
   double speed;

   /* Defaults. */
   abort_msg= NULL;
   f_gui    = 0;
   f_2x     = 0;
   speed    = 1.;

   /* Parse parameters. */
   b = lua_toboolean( L, 1 );
   if (!lua_isnoneornil(L,2)) {
      if (!lua_istable(L,2)) {
         NLUA_ERROR( L, _("Second parameter to cinematics should be a table of options or omitted!") );
         return 0;
      }

      lua_getfield( L, 2, "abort" );
      if (!lua_isnil( L, -1 ))
         abort_msg = luaL_checkstring( L, -1 );
      lua_pop( L, 1 );

      lua_getfield( L, 2, "gui" );
      f_gui = lua_toboolean(L, -1);
      lua_pop( L, 1 );

      lua_getfield( L, 2, "no2x" );
      f_2x = lua_toboolean(L, -1);
      lua_pop( L, 1 );

      lua_getfield( L, 2, "speed" );
      speed = luaL_optnumber(L,-1,1.);
      lua_pop( L, 1 );
   }

   if (b) {
      /* Reset speeds. This will override the player's ship base speed. */
      player.speed = speed;
      sound_setSpeed( speed );
      pause_setSpeed( speed );

      /* Get rid of stuff that could be bothersome. */
      player_autonavAbort( abort_msg );
      ovr_setOpen(0);

      /* Handle options. */
      player_setFlag( PLAYER_CINEMATICS );
      if (!f_gui)
         player_setFlag( PLAYER_CINEMATICS_GUI );
      if (f_2x)
         player_setFlag( PLAYER_CINEMATICS_2X );

      /* Redo viewport. */
      gl_setDefViewport( 0., 0., gl_screen.nw, gl_screen.nh );
   }
   else {
      /* Reset speed properly to player speed. */
      player_resetSpeed();

      /* Clean up flags. */
      player_rmFlag( PLAYER_CINEMATICS );
      player_rmFlag( PLAYER_CINEMATICS_GUI );
      player_rmFlag( PLAYER_CINEMATICS_2X );

      /* Reload GUI. */
      gui_reload();
   }

   return 0;
}

/**
 * @brief Applies the damage effects to the player.
 *
 *    @luatparam number mod How strong the effect should be. It should be a value between 0 and 1, usually corresponding to how much armour damage the player has taken with respect to their total armour.
 * @luafunc damageSPFX
 */
static int playerL_damageSPFX( lua_State *L )
{
   double spfx_mod = luaL_checknumber(L,1);
   spfx_shake( spfx_mod );
   spfx_damage( spfx_mod );
   return 0;
}

/**
 * @brief Takes a screenshot (same as the keyboard action).
 *
 * @luafunc screenshot
 */
static int playerL_screenshot( lua_State *L )
{
   (void) L;
   player_screenshot();
   return 0;
}

/**
 * @brief Tries to make the player board their target.
 *
 *    @luatparam boolean noisy Whether or not to do player messages.
 *    @luatreturn string Status of the boarding attempt. Can be "impossible", "retry", "ok", or "error".
 * @luafunc tryBoard
 */
static int playerL_tryBoard( lua_State *L )
{
   int ret = player_tryBoard( lua_toboolean(L,1) );
   switch (ret) {
      case PLAYER_BOARD_IMPOSSIBLE:
         lua_pushstring(L,"impossible");
         break;
      case PLAYER_BOARD_RETRY:
         lua_pushstring(L,"retry");
         break;
      case PLAYER_BOARD_OK:
         lua_pushstring(L,"ok");
         break;
      default:
         lua_pushstring(L,"error");
         break;
   }
   return 1;
}

/**
 * @brief Unboards the player from its boarded target.
 *
 * Use from inside a board hook.
 *
 * @usage player.unboard()
 *
 * @luafunc unboard
 */
static int playerL_unboard( lua_State *L )
{
   (void) L;
   board_unboard();
   return 0;
}

/**
 * @brief Checks to see if the player is landed or not.
 *
 *    @luatreturn boolean Whether or not the player is currently landed.
 * @luafunc isLanded
 */
static int playerL_isLanded( lua_State *L )
{
   lua_pushboolean( L, landed );
   return 1;
}

/**
 * @brief Forces the player to take off if they are landed.
 *
 * Assume the pilot is still landed until the current running function returns
 *  If you want to create pilots on take off please hook the takeoff/land hooks.
 *
 * @luafunc takeoff
 */
static int playerL_takeoff( lua_State *L )
{
   PLAYER_CHECK();

   if (!landed) {
      NLUA_ERROR(L,_("Player must be landed to force takeoff."));
      return 0;
   }
   if (!pilot_isSpaceworthy( player.p )) {
      NLUA_ERROR(L,_("Player must be spaceworthy to force takeoff!"));
      return 0;
   }

   land_queueTakeoff();

   return 0;
}

/**
 * @brief Tries to make the player land.
 *
 *    @luatparam boolean noisy Whether or not to do player messages.
 *    @luatreturn string Status of the boarding attempt. Can be "impossible", "retry", "ok", or "error".
 * @luafunc tryBoard
 */
static int playerL_tryLand( lua_State *L )
{
   int ret = player_land( lua_toboolean(L,1) );
   switch (ret) {
      case PLAYER_LAND_DENIED:
         lua_pushstring(L,"impossible");
         break;
      case PLAYER_LAND_OK:
         lua_pushstring(L,"ok");
         break;
      case PLAYER_LAND_AGAIN:
         lua_pushstring(L,"retry");
         break;
      default:
         lua_pushstring(L,"error");
         break;
   }
   return 1;
}

/**
 * @brief Automagically lands the player on a spob.
 *
 * Note that this will teleport the player to the system in question as necessary.
 *
 *    @luatparam Spob spb Spob to land the player on.
 * @luafunc land
 */
static int playerL_land( lua_State *L )
{
   PLAYER_CHECK();

   Spob *spob = luaL_validspob(L,1);
   const char *sysname = spob_getSystem( spob->name );
   if (sysname == NULL)
      NLUA_ERROR(L,_("Spob '%s' is not in a system!"), spob->name);

   /* Unboard just in case. */
   board_unboard();

   if (strcmp(sysname,cur_system->name) != 0) {
      /* Refer to playerL_teleport for the voodoo that happens here. */
      pilot_rmFlag( player.p, PILOT_HYPERSPACE );
      pilot_rmFlag( player.p, PILOT_HYP_BEGIN );
      pilot_rmFlag( player.p, PILOT_HYP_BRAKE );
      pilot_rmFlag( player.p, PILOT_HYP_PREP );
      player_accelOver();
      player_autonavEnd();

      space_gfxUnload( cur_system );

      player_targetClearAll();

      space_init( sysname, 0 );

      ovr_initAlpha();
   }
   player.p->solid.pos = spob->pos; /* Set position to target. */

   /* Do whatever the spob wants to do. */
   if (spob->lua_land != LUA_NOREF) {
      spob_luaInitMem( spob );
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, spob->lua_land); /* f */
      lua_pushspob( naevL, spob_index(spob) );
      lua_pushpilot( naevL, player.p->id );
      if (nlua_pcall( spob->lua_env, 2, 0 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), spob->name, "land", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
      }

      return 0;
   }

   space_queueLand( spob );
   return 0;
}

/**
 * @brief Allows or disallows the player to land.
 *
 * This will allow or disallow landing on a system level and is reset when the
 *  player enters another system.
 *
 * @usage player.landAllow() -- Allows the player to land
 * @usage player.landAllow( false ) -- Doesn't allow the player to land.
 * @usage player.landAllow( false, "No landing." ) -- Doesn't allow the player to land with the message "No landing."
 *
 *    @luatparam[opt=true] boolean b Whether or not to allow the player to land.
 *    @luatparam[opt] string msg Message displayed when player tries to land (only if disallowed to land). Can be omitted to use default.
 * @luafunc landAllow
 */
static int playerL_landAllow( lua_State *L )
{
   int b;
   const char *str = NULL;

   if (lua_gettop(L) > 0) {
      b  = lua_toboolean(L,1);
      if (lua_isstring(L,2))
         str   = lua_tostring(L,2);
   }
   else
      b  = 1;

   if (b)
      player_rmFlag( PLAYER_NOLAND );
   else {
      player_setFlag( PLAYER_NOLAND );
      player_nolandMsg( str );
   }
   return 0;
}

/**
 * @brief Sets the active land window.
 *
 * Valid windows are:<br/>
 *  - main<br/>
 *  - bar<br/>
 *  - missions<br/>
 *  - outfits<br/>
 *  - shipyard<br/>
 *  - equipment<br/>
 *  - commodity<br/>
 *
 * @usage player.landWindow( "outfits" )
 *    @luatparam string winname Name of the window.
 *    @luatreturn boolean True on success.
 * @luafunc landWindow
 */
static int playerL_landWindow( lua_State *L )
{
   int ret;
   const char *str;
   int win;

   if (!landed) {
      NLUA_ERROR(L, _("Must be landed to set the active land window."));
      return 0;
   }

   str = luaL_checkstring(L,1);
   if (strcasecmp(str,"main")==0)
      win = LAND_WINDOW_MAIN;
   else if (strcasecmp(str,"bar")==0)
      win = LAND_WINDOW_BAR;
   else if (strcasecmp(str,"missions")==0)
      win = LAND_WINDOW_MISSION;
   else if (strcasecmp(str,"outfits")==0)
      win = LAND_WINDOW_OUTFITS;
   else if (strcasecmp(str,"shipyard")==0)
      win = LAND_WINDOW_SHIPYARD;
   else if (strcasecmp(str,"equipment")==0)
      win = LAND_WINDOW_EQUIPMENT;
   else if (strcasecmp(str,"commodity")==0)
      win = LAND_WINDOW_COMMODITY;
   else
      NLUA_INVALID_PARAMETER(L);

   /* Sets the window. */
   ret = land_setWindow( win );

   lua_pushboolean( L, !ret );
   return 1;
}

/**
 * @brief Forces the player to close comm if they are chatting.
 *
 * @luafunc commClose
 */
static int playerL_commclose( lua_State *L )
{
   (void) L;
   comm_queueClose();
   return 0;
}

static PlayerShip_t *playerL_shipvarShip( lua_State *L, int idx )
{
   if (lua_isnoneornil(L,idx))
      return &player.ps;
   return player_getPlayerShip( luaL_checkstring(L,idx) );
}

/**
 * @brief Peeks at a ship variable.
 *
 * @usage local exp = player.shipvarPeek( "exp" ) -- Checks the value of the "exp" ship var on the player's current ship
 *
 *    @luatparam string varname Name of the variable to check value of.
 *    @luatparam[opt] string shipname Name of the ship to check variable of. Defaults to player's current ship.
 * @luafunc shipvarPeek
 */
static int playerL_shipvarPeek( lua_State *L )
{
   PLAYER_CHECK();
   const char *str  = luaL_checkstring(L,1);
   PlayerShip_t *ps = playerL_shipvarShip(L,2);
   lvar *var        = lvar_get( ps->p->shipvar, str );
   if (var != NULL)
      return lvar_push( L, var );
   return 0;
}

/**
 * @brief Pushes a ship variable.
 *
 *    @luatparam string varname Name of the variable to set value of.
 *    @luaparam val Value to push.
 *    @luatparam[opt] string shipname Name of the ship to push variable to. Defaults to player's current ship.
 * @luafunc shipvarPush
 */
static int playerL_shipvarPush( lua_State *L )
{
   PLAYER_CHECK();
   const char *str  = luaL_checkstring(L,1);
   lvar var         = lvar_tovar( L, str, 2 );
   PlayerShip_t *ps = playerL_shipvarShip(L,3);
   if (ps->p->shipvar==NULL)
      ps->p->shipvar = array_create( lvar );
   lvar_addArray( &ps->p->shipvar, &var, 1 );
   return 0;
}

/**
 * @brief Pops a ship variable.
 *
 *    @luatparam string varname Name of the variable to pop.
 *    @luatparam[opt] string shipname Name of the ship to pop variable from. Defaults to player's current ship.
 * @luafunc shipvarPop
 */
static int playerL_shipvarPop( lua_State *L )
{
   PLAYER_CHECK();
   const char *str  = luaL_checkstring(L,1);
   PlayerShip_t *ps = playerL_shipvarShip(L,2);
   lvar *var        = lvar_get( ps->p->shipvar, str );
   if (var != NULL)
      lvar_rmArray( &ps->p->shipvar, var );
   return 0;
}

/**
 * @brief Gets the names of the player's ships.
 *
 * @usage names = player.ships() -- The player's ship names.
 *
 *   @luatreturn {String,...} Table of ship names.
 * @luafunc ships
 */
static int playerL_ships( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }

   const PlayerShip_t *ships = player_getShipStack();
   lua_newtable(L);  /* t */
   for (int i=0; i<array_size(ships); i++) {
      lua_newtable(L);        /* t, k, t */

      lua_pushstring(L, ships[i].p->name); /* t, k, t, s */
      lua_setfield(L, -2, "name"); /* t, k, t */

      lua_pushship(L, ships[i].p->ship); /* t, k, t, s */
      lua_setfield(L, -2, "ship"); /* t, k, t */

      lua_pushboolean(L, ships[i].deployed); /* t, k, t, s */
      lua_setfield(L, -2, "deployed"); /* t, k, t */

      lua_rawseti(L, -2, i+1); /* t */
   }
   return 1;
}

/**
 * @brief Gets the outfits for one of the player's ships.
 *
 * @usage outfits = player.shipOutfits("Llama") -- Gets the Llama's outfits
 *
 *   @luatparam[nil=Current ship] string name Name of the ship to get the outfits of.
 *   @luatreturn {Outfit,...} Table of outfits.
 * @luafunc shipOutfits
 */
static int playerL_shipOutfits( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }

   const char *str;
   int j;
   const PlayerShip_t *ships;
   Pilot *p;

   /* Get name. */
   str = luaL_optstring(L, 1, NULL);
   ships = player_getShipStack();

   /* Get outfit. */
   lua_newtable(L);

   p = NULL;
   if ((str==NULL) || (strcmp(str, player.p->name)==0))
      p = player.p;
   else {
      for (int i=0; i<array_size(ships); i++) {
         if (strcmp(str, ships[i].p->name)==0) {
            p = ships[i].p;
            break;
         }
      }
   }

   if (p == NULL) {
      NLUA_ERROR( L, _("Player does not own a ship named '%s'"), str );
      return 0;
   }

   lua_newtable( L );
   j = 1;
   for (int i=0; i<array_size(p->outfits); i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;

      /* Set the outfit. */
      lua_pushoutfit( L, p->outfits[i]->outfit );
      lua_rawseti( L, -2, j++ );
   }

   return 1;
}

/**
 * @brief Gets meta-data about one of the player's ships.
 *
 *   @luatparam[nil=Current ship] string name Name of the ship to get the meta-data of.
 *   @luatreturn Table of meta-data.
 * @luafunc shipMetadata
 */
static int playerL_shipMetadata( lua_State *L )
{
   PLAYER_CHECK();

   int destroyed = 0;
   const char *str = luaL_optstring(L, 1, NULL);
   const PlayerShip_t *ships = player_getShipStack();
   const PlayerShip_t *ps = NULL;
   if ((str==NULL) || (strcmp(str, player.p->name)==0))
      ps = &player.ps;
   else {
      for (int i=0; i<array_size(ships); i++) {
         if (strcmp(str, ships[i].p->name)==0) {
            ps = &ships[i];
            break;
         }
      }
   }
   if (ps == NULL) {
      NLUA_ERROR( L, _("Player does not own a ship named '%s'"), str );
      return 0;
   }

   lua_newtable(L);

   lua_pushnumber( L, ps->time_played );
   lua_setfield( L, -2, "time_played" );

   lua_pushstring( L, ps->acquired );
   lua_setfield( L, -2, "acquired" );

   lua_pushtime( L, ps->acquired_date );
   lua_setfield( L, -2, "acquired_date" );

   lua_pushnumber( L, ps->dmg_done_shield );
   lua_setfield( L, -2, "dmg_done_shield" );

   lua_pushnumber( L, ps->dmg_done_armour );
   lua_setfield( L, -2, "dmg_done_armour" );

   lua_pushnumber( L, ps->dmg_done_shield+ps->dmg_done_armour );
   lua_setfield( L, -2, "dmg_done" );

   lua_pushnumber( L, ps->dmg_taken_shield );
   lua_setfield( L, -2, "dmg_taken_shield" );

   lua_pushnumber( L, ps->dmg_taken_armour );
   lua_setfield( L, -2, "dmg_taken_armour" );

   lua_pushnumber( L, ps->dmg_taken_shield+ps->dmg_taken_armour );
   lua_setfield( L, -2, "dmg_taken" );

   lua_pushinteger( L, ps->jumped_times );
   lua_setfield( L, -2, "jumped_times" );

   lua_pushinteger( L, ps->landed_times );
   lua_setfield( L, -2, "landed_times" );

   lua_pushinteger( L, ps->death_counter );
   lua_setfield( L, -2, "death_counter" );

   for (int i=0; i<SHIP_CLASS_TOTAL; i++)
      destroyed += ps->ships_destroyed[i];
   lua_pushinteger( L, destroyed );
   lua_setfield( L, -2, "ships_destroyed" );

   return 1;
}

/**
 * @brief Sets the deployed status of a player's ship.
 *
 *    @luatparam string shipname Name of the ship to set deployed status of.
 *    @luatparam[opt=false] boolean deploy Whether or not to set the deployed status of the ship.
 * @luafunc shipDeploy
 */
static int playerL_shipDeploy( lua_State *L )
{
   PLAYER_CHECK();
   const char *shipname = luaL_checkstring(L,1);
   int deploy = lua_toboolean(L,2);
   PlayerShip_t *ps = player_getPlayerShip( shipname );
   ps->deployed = deploy;
   return 0;
}

/**
 * @brief Gets all the outfits the player owns.
 *
 * If you want the quantity, call player.outfitNum() on the individual outfit.
 *
 * @usage player.outfits() -- A table of all the player's outfits.
 *
 *    @luatparam[opt=false] bool unequipped_only Whether or not to check only the unequipped outfits and not equipped outfits. Defaults to false.
 *   @luatreturn {Outfit,...} Table of outfits.
 * @luafunc outfits
 */
static int playerL_outfits( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }

   const PlayerOutfit_t *outfits = player_getOutfits();
   int unequipped_only = lua_toboolean(L, 1);
   const PlayerShip_t* pstack = player_getShipStack();
   int n = 1;
   /* Set up and get owned outfits. */
   lua_newtable(L);
   for (int i=0; i<array_size(outfits); i++) {
      lua_pushoutfit(L, outfits[i].o );
      lua_rawseti(L, -2, n++);
   }
   /* Add all outfits on player ships. */
   if (!unequipped_only) {
      for (int i=0; i<array_size(pstack); i++) {
         for (int j=0; j<array_size(pstack[i].p->outfits); j++) {
            const Outfit *o = pstack[i].p->outfits[j]->outfit;
            int found = 0;
            for (int k=0; k<array_size(outfits); k++) {
               if (outfits[k].o==o) {
                  found = 1;
                  break;
               }
            }
            if (found) {
               lua_pushoutfit(L, o);
               lua_rawseti(L, -2, n++);
            }
         }
      }
   }
   return 1;
}

/**
 * @brief Gets the number of outfits the player owns in their list (excludes equipped on ships).
 *
 * @usage q = player.outfitNum( "Laser Cannon MK0", true ) -- Number of 'Laser Cannon MK0' the player owns (unequipped)
 *
 *    @luatparam string name Name of the outfit to remove.
 *    @luatparam[opt=false] bool unequipped_only Whether or not to check only the unequipped outfits and not equipped outfits. Defaults to false.
 *    @luatreturn number The quantity the player owns.
 * @luafunc outfitNum
 */
static int playerL_outfitNum( lua_State *L )
{
   if (player.p==NULL) {
      lua_pushnumber(L,0.);
      return 1;
   }

   const Outfit *o;
   int q, unequipped_only;

   /* Handle parameters. */
   o = luaL_validoutfit(L, 1);
   unequipped_only = lua_toboolean(L, 2);

   /* Count the outfit. */
   if (unequipped_only)
      q = player_outfitOwned( o );
   else
      q = player_outfitOwnedTotal( o );
   lua_pushnumber( L, q );

   return 1;
}
/**
 * @brief Adds an outfit to the player's outfit list.
 *
 * @usage player.outfitAdd( "Laser Cannon" ) -- Gives the player a laser cannon
 * @usage player.outfitAdd( "Plasma Blaster", 2 ) -- Gives the player two plasma blasters
 *
 *    @luatparam string name Name of the outfit to give.
 *    @luatparam[opt=1] number q Quantity to give.
 * @luafunc outfitAdd
 */
static int playerL_outfitAdd( lua_State *L  )
{
   PLAYER_CHECK();

   /* Handle parameters. */
   const Outfit *o = luaL_validoutfit(L, 1);
   int q = luaL_optinteger(L, 2, 1);

   /* Add the outfits. */
   player_addOutfit( o, q );

   /* Update equipment list. */
   outfits_updateEquipmentOutfits();

   return 0;
}
/**
 * @brief Removes an outfit from the player's outfit list.
 *
 * "all" will remove all outfits.
 *
 * @usage player.outfitRm( "Plasma Blaster", 2 ) -- Removes two plasma blasters from the player
 *
 *    @luatparam string name Name of the outfit to give.
 *    @luatparam[opt] number q Quantity to remove (default 1).
 * @luafunc outfitRm
 */
static int playerL_outfitRm( lua_State *L )
{
   const Outfit *o;
   int q = luaL_optinteger(L, 2, 1);

   /* Handle special case it's "all". */
   if (lua_isstring(L, 1)) {
      const char *str = luaL_checkstring(L, 1);

      if (strcmp(str,"all")==0) {
         const PlayerOutfit_t *poutfits = player_getOutfits();
         const Outfit **outfits = array_create_size( const Outfit*, array_size( poutfits ) );
         for (int i=0; i<array_size(poutfits); i++)
            array_push_back( &outfits, poutfits[i].o );

         for (int i=0; i<array_size(outfits); i++) {
            o = outfits[i];
            q = player_outfitOwned(o);
            player_rmOutfit(o, q);
         }
         /* Clean up. */
         array_free(outfits);

         /* Update equipment list. */
         outfits_updateEquipmentOutfits();
         return 0;
      }
   }

   /* Usual case. */
   o = luaL_validoutfit(L, 1);
   player_rmOutfit( o, q );

   /* Update equipment list. */
   outfits_updateEquipmentOutfits();

   return 0;
}

/**
 * @brief Gives the player a new ship.
 *
 * @note Should be given when landed, ideally on a spob with a shipyard. Furthermore, this invalidates all player.pilot() references.
 *
 * @usage player.shipAdd( "Pirate Kestrel", _("Seiryuu"), _("") ) -- Gives the player a Pirate Kestrel named Seiryuu if player cancels the naming.
 *
 *    @luatparam string ship Name of the ship to add.
 *    @luatparam[opt=ship.get(ship):name()] string name Name to give the ship if player refuses to name it (defaults to shipname if omitted).
 *    @luatparam[opt] string acquired A description of how the ship was acquired.
 *    @luatparam[opt=false] boolean noname If true does not let the player name the ship.
 *    @luatreturn string The new ship's name.
 * @luafunc shipAdd
 */
static int playerL_shipAdd( lua_State *L )
{
   PlayerShip_t *new_ship;
   /* Handle parameters. */
   const Ship *s     = luaL_validship(L, 1);
   const char *name  = luaL_optstring(L, 2, _(s->name));
   const char *acquired = luaL_optstring(L, 3, NULL);
   int noname        = lua_toboolean(L, 4);
   /* Add the ship, look in case it's cancelled. */
   do {
      new_ship = player_newShip( s, name, 0, acquired, noname );
   } while (new_ship == NULL);
   /* Return the new name. */
   lua_pushstring( L, new_ship->p->name );
   return 1;
}

/**
 * @brief Swaps the player's current ship with a ship they own by name.
 *
 * @note You shouldn't use this directly unless you know what you are doing. If the player's cargo doesn't fit in the new ship, it won't be moved over and can lead to a whole slew of issues.
 * @note This invalidates all player.pilot() references!
 *
 *    @luatparam string ship Name of the ship to swap to. (this is name given by the player, not ship name)
 *    @luatparam[opt=false] boolean ignore_cargo Whether or not to ignore cargo and not move it, or try to move it over.
 *    @luatparam[opt=false] boolean remove If true removes the player's current ship (so it replaces and doesn't swap).
 * @luafunc shipSwap
 */
static int playerL_shipSwap( lua_State *L )
{
   PLAYER_CHECK();

   const char *str = luaL_checkstring(L,1);
   int ignore_cargo= lua_toboolean(L,2);
   const char *cur = player.p->name;
   player_swapShip( str, !ignore_cargo );
   if (lua_toboolean(L,3))
      player_rmShip( cur );

   return 0;
}

/**
 * @brief Gets the list of the player's active missions.
 *
 * @usage n = \#player.missions() -- number of active missions
 *
 *    @luatreturn table Table containing the metadat of active missions as tables. Fields include "name", "desc", "reward", "loc", "chance", "spob", "system", "chapter", "cond", "done", "priority", "unique", and "tags".
 * @luafunc missions
 */
static int playerL_missions( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }
   int j = 1;
   lua_newtable(L);
   for (int i=0; i<array_size(player_missions); i++) {
      const Mission *pm = player_missions[i];
      const MissionData *md = pm->data;
      if (pm->id == 0)
         continue;

      misn_pushMissionData( L, md );
      /* Add player mission-specific data. */
      if (pm->title != NULL) {
         lua_pushstring( L, pm->title );
         lua_setfield(L,-2,"title");
      }
      if (pm->desc != NULL) {
         lua_pushstring( L, pm->desc );
         lua_setfield(L,-2,"desc");
      }
      if (pm->reward != NULL) {
         lua_pushstring( L, pm->reward );
         lua_setfield(L,-2,"reward");
      }
      lua_rawseti( L, -2, j++ );
   }
   return 1;
}

/**
 * @brief Checks to see if the player has a mission active.
 *
 * @usage if player.misnActive( "The Space Family" ) then -- Player is doing space family mission
 *
 *    @luatparam string name Name of the mission to check.
 *    @luatreturn boolean|number Number of instances if the mission is active, false if it isn't.
 * @luafunc misnActive
 */
static int playerL_misnActive( lua_State *L )
{
   PLAYER_CHECK();
   const char *str = luaL_checkstring(L,1);
   const MissionData *misn = mission_getFromName( str );
   if (misn == NULL) {
      NLUA_ERROR(L, _("Mission '%s' not found in stack"), str);
      return 0;
   }
   int n = mission_alreadyRunning( misn );
   if (n > 0)
      lua_pushinteger( L, n );
   else
      lua_pushboolean( L, 0 );
   return 1;
}

/**
 * @brief Checks to see if player has done a mission.
 *
 * This only works with missions that have the unique flag.
 *
 * @usage if player.misnDone( "The Space Family" ) then -- Player finished mission
 *    @luatparam string name Name of the mission to check.
 *    @luatreturn boolean true if mission was finished, false if it wasn't.
 * @luafunc misnDone
 */
static int playerL_misnDone( lua_State *L )
{
   PLAYER_CHECK();
   const char *str = luaL_checkstring(L, 1);
   int id          = mission_getID( str );
   if (id == -1) {
      NLUA_ERROR(L, _("Mission '%s' not found in stack"), str);
      return 0;
   }
   lua_pushboolean( L, player_missionAlreadyDone( id ) );
   return 1;
}

/**
 * @brief Gets a list of all the missions the player has done.
 *
 *    @luatreturn table List of all the missions the player has done.
 * @luafunc misnDoneList
 */
static int playerL_misnDoneList( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }
   int *done = player_missionsDoneList();
   lua_newtable(L);
   for (int i=0; i<array_size(done); i++) {
      mission_toLuaTable( L, mission_get( done[i] ) );
      lua_rawseti(L,-2,i+1);
   }
   return 1;
}

/**
 * @brief Checks to see if the player has an event active.
 *
 * @usage if player.evtActive( "Shipwreck" ) then -- The shipwreck event is active
 *
 *    @luatparam string name Name of the mission to check.
 *    @luatreturn boolean true if the mission is active, false if it isn't.
 * @luafunc evtActive
 */
static int playerL_evtActive( lua_State *L )
{
   PLAYER_CHECK();
   const char *str= luaL_checkstring(L,1);
   int evtid      = event_dataID( str );
   if (evtid < 0) {
      NLUA_ERROR(L, _("Event '%s' not found in stack"), str);
      return 0;
   }
   lua_pushboolean( L, event_alreadyRunning( evtid ) );
   return 1;
}

/**
 * @brief Checks to see if player has done an event.
 *
 * This only works with events that have the unique flag.
 *
 * @usage if player.evtDone( "Shipwreck" ) then -- Player finished event
 *    @luatparam string name Name of the event to check.
 *    @luatreturn boolean true if event was finished, false if it wasn't.
 * @luafunc evtDone
 */
static int playerL_evtDone( lua_State *L )
{
   PLAYER_CHECK();
   const char *str = luaL_checkstring(L, 1);
   int id          = event_dataID( str );
   if (id == -1) {
      NLUA_ERROR(L, _("Event '%s' not found in stack"), str);
      return 0;
   }
   lua_pushboolean( L, player_eventAlreadyDone( id ) );
   return 1;
}

/**
 * @brief Gets a list of all the events the player has done.
 *
 *    @luatreturn table List of all the events the player has done.
 * @luafunc evtDoneList
 */
static int playerL_evtDoneList( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }
   int *done = player_eventsDoneList();
   lua_newtable(L);
   for (int i=0; i<array_size(done); i++) {
      event_toLuaTable( L, done[i] );
      lua_rawseti(L,-2,i+1);
   }
   return 1;
}

/**
 * @brief Gets the player's current GUI.
 *
 *    @luatreturn string The player's current GUI.
 * @luafunc gui
 */
static int playerL_gui( lua_State *L )
{
   lua_pushstring(L,gui_pick());
   return 1;
}

/**
 * @brief Lists the GUIs the player can use.
 *
 *    @luatreturn table Table containing the names of all the GUIs the player can use.
 * @luafunc guiList
 */
static int playerL_guiList( lua_State *L )
{
   const char **gui = player_guiList();

   lua_newtable(L);
   for (int i=0; i<array_size(gui); i++) {
      lua_pushstring( L, gui[i] );
      lua_rawseti( L, -2, i+1 );
   }

   return 1;
}

/**
 * @brief Sets the player's GUI.
 *
 *    @luatreturn string The player's current GUI.
 * @luafunc guiSet
 */
static int playerL_guiSet( lua_State *L )
{
   const char *name = luaL_checkstring(L,1);
   if (!gui_exists(name))
      NLUA_ERROR(L,_("GUI '%s' does not exist!"),name);
   free( player.gui );
   player.gui = strdup(name);
   gui_load( gui_pick() );
   return 0;
}

/**
 * @brief Lists the ships in the player's fleet.
 *
 *    @luatreturn table Table containing the pilots in the player fleet or false if they are not spawned yet.
 * @luafunc fleetList
 */
static int playerL_fleetList( lua_State *L )
{
   if (player.p == NULL) {
      lua_newtable(L);
      return 1;
   }
   int n = 1;
   const PlayerShip_t* pstack = player_getShipStack();
   lua_newtable(L);
   for (int i=0; i<array_size(pstack); i++) {
      const PlayerShip_t *ps = &pstack[i];
      if (!ps->deployed)
         continue;
      /* We can avoid the spaceWorthy check as they will be set to false due to not having a pilot. */
      /*if (!!pilot_isSpaceworthy(ps->p))
         continue;*/

      if (ps->p==NULL)
         lua_pushboolean( L, 0 );
      else
         lua_pushpilot( L, ps->p->id );
      lua_rawseti( L, -2, n++ );
   }
   return 1;
}

/**
 * @brief Gets the amount of cargo space free in the player's fleet.
 *
 *    @luatreturn number Amount of free cargo space.
 * @luafunc fleetCargoFree
 */
static int playerL_fleetCargoFree( lua_State *L )
{
   if (player.p==NULL)
      lua_pushinteger(L,0);
   else
      lua_pushinteger( L, pfleet_cargoFree() );
   return 1;
}

/**
 * @brief Gets the amount of cargo space used in the player's fleet.
 *
 *    @luatreturn number Amount of used cargo space.
 * @luafunc fleetCargoUsed
 */
static int playerL_fleetCargoUsed( lua_State *L )
{
   if (player.p==NULL)
      lua_pushinteger(L,0);
   else
      lua_pushinteger( L, pfleet_cargoUsed() );
   return 1;
}

/**
 * @brief Gets the amount of cargo space used by a specific commodity in the player's fleet.
 *
 *    @luatparam Commodity c Commodity to check how much the player has in their fleet.
 *    @luatreturn number Amount of used cargo space for the commodity.
 * @luafunc fleetCargoOwned
 */
static int playerL_fleetCargoOwned( lua_State *L )
{
   Commodity *c = luaL_validcommodity( L, 1 );
   if (player.p==NULL)
      lua_pushinteger(L,0);
   else
      lua_pushinteger( L, pfleet_cargoOwned( c ) );
   return 1;
}

/**
 * @brief Tries to add an amount of commodity to the player's fleet.
 *
 *    @luatparam Commodity c Commodity to add to the player fleet.
 *    @luatparam number q Amount to add.
 *    @luatreturn number Amount of commodity added to the player fleet.
 * @luafunc fleetCargoAdd
 */
static int playerL_fleetCargoAdd( lua_State *L )
{
   Commodity *c = luaL_validcommodity( L, 1 );
   int q = luaL_checkinteger( L, 2 );
   if (player.p==NULL)
      lua_pushinteger(L,0);
   else
      lua_pushinteger( L, pfleet_cargoAdd( c, q ) );
   return 1;
}

/**
 * @brief Tries to remove an amount of commodity to the player's fleet.
 *
 *    @luatparam Commodity c Commodity to remove from the player fleet.
 *    @luatparam number q Amount to remove.
 *    @luatreturn number Amount of commodity removed from the player fleet.
 * @luafunc fleetCargoRm
 */
static int playerL_fleetCargoRm( lua_State *L )
{
   Commodity *c = luaL_validcommodity( L, 1 );
   int q = luaL_checkinteger( L, 2 );
   if (player.p==NULL)
      lua_pushinteger(L,0);
   else
      lua_pushinteger( L, pfleet_cargoRm( c, q, 0 ) );
   return 1;
}

/**
 * @brief Tries to remove an amount of commodity to the player's fleet and jettisons it into space.
 *
 *    @luatparam Commodity c Commodity to remove from the player fleet.
 *    @luatparam number q Amount to remove.
 *    @luatreturn number Amount of commodity removed from the player fleet.
 * @luafunc fleetCargoJet
 */
static int playerL_fleetCargoJet( lua_State *L )
{
   Commodity *c = luaL_validcommodity( L, 1 );
   int q = luaL_checkinteger( L, 2 );
   if (player.p==NULL)
      lua_pushinteger(L,0);
   else
      lua_pushinteger( L, pfleet_cargoRm( c, q, 1 ) );
   return 1;
}

/**
 * @brief Gets the list of all the cargos in the player's fleet.
 *
 * @usage for k,v in ipairs( player.fleetCargoList() ) do print( v.c, v.q ) end
 *
 *    @luatreturn table A table containing table entries of the form {c = commodity, q = quantity }.
 * @luafunc fleetCargoList
 */
static int playerL_fleetCargoList( lua_State *L )
{
   if (player.p==NULL) {
      lua_newtable(L);
      return 1;
   }
   Commodity *call = commodity_getAll();
   int n = 0;
   lua_newtable(L);                 /* t */
   for (int i=0; i<array_size(call); i++) {
      Commodity *c = &call[i];
      int q = pfleet_cargoOwned( c );
      if (q <= 0)
         continue;

      lua_newtable(L);              /* t, t */

      lua_pushcommodity( L, c );    /* t, t, c */
      lua_setfield( L, -2, "c" );   /* t, t  */

      lua_pushinteger( L, q );      /* t, t, q */
      lua_setfield( L, -2, "q" );   /* t, t */

      lua_rawseti( L, -2, ++n );    /* t */
   }
   return 1;
}

/**
 * @brief Gets the contents of the player's inventory.
 *
 * Note that this does not get licenses.
 *
 *    @luatreturn table A table containing inventory items in the form of subtables with a "name" and "quantity" field.
 * @luafunc inventory
 */
static int playerL_inventory( lua_State *L )
{
   if (player.p==NULL) {
      lua_newtable(L);
      return 1;
   }
   const PlayerItem *inv = player_inventory();
   lua_newtable(L);
   for (int i=0; i<array_size(inv); i++) {
      const PlayerItem *pi = &inv[i];
      lua_newtable(L);

      lua_pushstring(L,pi->name);
      lua_setfield(L,-2,"name");

      lua_pushinteger(L,pi->quantity);
      lua_setfield(L,-2,"quantity");

      lua_rawseti(L,-2,i+1);
   }
   return 1;
}

/**
 * @brief Adds an item to the player's in-game inventory.
 *
 *    @luatparam string name The name of the item to add.
 *    @luatparam integer quantity The amount of the item to add.
 *    @luatreturn integer The amount of the item added.
 * @luafunc inventoryAdd
 */
static int playerL_inventoryAdd( lua_State *L )
{
   const char *name = luaL_checkstring(L,1);
   int q = luaL_optinteger(L,2,1);
   if (player.p==NULL)
      lua_pushinteger( L, 0 );
   else
      lua_pushinteger( L, player_inventoryAdd( name, q ) );
   return 1;
}

/**
 * @brief Removes an item to the player's in-game inventory.
 *
 *    @luatparam string name The name of the item to remove.
 *    @luatparam integer quantity The amount of the item to remove.
 *    @luatreturn integer The amount of the item removed.
 * @luafunc inventoryAdd
 */
static int playerL_inventoryRm( lua_State *L )
{
   const char *name = luaL_checkstring(L,1);
   int q = luaL_optinteger(L,2,1);
   if (player.p==NULL)
      lua_pushinteger( L, 0 );
   else
      lua_pushinteger( L, player_inventoryRemove( name, q ) );
   return 1;
}

/**
 * @brief Checks to see how much of an item the player has in their inventory.
 *
 *    @luatparam string name The name of the item to check.
 *    @luatreturn integer The amount of the item the player has (0 if none).
 * @luafunc inventoryAdd
 */
static int playerL_inventoryOwned( lua_State *L )
{
   const char *name = luaL_checkstring(L,1);
   if (player.p==NULL)
      lua_pushinteger( L, 0 );
   else
      lua_pushinteger( L, player_inventoryAmount( name ) );
   return 1;
}

/**
 * @brief Teleports the player to a new spob or system (only if not landed).
 *
 * If the destination is a system, the coordinates of the player will not change.
 * If the destination is a spob, the player will be placed over that spob.
 *
 * @usage player.teleport( system.get("Arcanis") ) -- Teleports the player to Arcanis.
 * @usage player.teleport( "Arcanis" ) -- Teleports the player to Arcanis.
 * @usage player.teleport( "Dvaer Prime" ) -- Teleports the player to Dvaer, and relocates him to Dvaer Prime.
 * @usage player.teleport( "Sol", true ) -- Teleports the player to SOL, but doesn't run any initial simulation
 *
 *    @luatparam System|Spob|string dest System or name of a system or spob or name of a spob to teleport the player to.
 *    @luatparam[opt=false] boolean no_simulate Don't simulate the when teleporting if true.
 *    @luatparam[opt=false] boolean silent Doesn't display any entering system messages.
 * @luafunc teleport
 */
static int playerL_teleport( lua_State *L )
{
   PLAYER_CHECK();
   Spob *pnt;
   const char *name, *pntname;
   int no_simulate, silent;

   /* Must not be landed. */
   if (landed)
      NLUA_ERROR(L,_("Can not teleport the player while landed!"));
   if (comm_isOpen())
      NLUA_ERROR(L,_("Can not teleport the player while the comm is open!"));
   if (player_isBoarded())
      NLUA_ERROR(L,_("Can not teleport the player while they are boarded!"));
   pnt = NULL;

   /* Get a system. */
   if (lua_issystem(L,1)) {
      StarSystem *sys = luaL_validsystem(L,1);
      name = system_getIndex(sys->id)->name;
   }
   /* Get a spob. */
   else if (lua_isspob(L,1)) {
      pnt   = luaL_validspob(L,1);
      name  = spob_getSystem( pnt->name );
      if (name == NULL) {
         NLUA_ERROR( L, _("Spob '%s' does not belong to a system."), pnt->name );
         return 0;
      }
   }
   /* Get destination from string. */
   else if (lua_isstring(L,1)) {
      const char *sysname;
      name = lua_tostring(L,1);
      sysname = system_existsCase( name );
      if (sysname == NULL) {
         /* No system found, assume destination string is the name of a spob. */
         pntname = name;
         name = spob_getSystem( pntname );
         pnt  = spob_get( pntname );
         if (pnt == NULL) {
            NLUA_ERROR( L, _("'%s' is not a valid teleportation target."), name );
            return 0;
         }

         if (name == NULL) {
            NLUA_ERROR( L, _("Spob '%s' does not belong to a system."), pntname );
            return 0;
         }
      }
      else
         name = sysname;
   }
   else
      NLUA_INVALID_PARAMETER(L);

   no_simulate = lua_toboolean(L,2);
   silent = lua_toboolean(L,3);

   /* Check if system exists. */
   if (system_get( name ) == NULL) {
      NLUA_ERROR( L, _("System '%s' does not exist."), name );
      return 0;
   }

   /* Unboard just in case. */
   board_unboard();

   /* Jump out hook is run first. */
   hooks_run( "jumpout" );

   /* Just in case remove hyperspace flags. */
   pilot_rmFlag( player.p, PILOT_HYPERSPACE );
   pilot_rmFlag( player.p, PILOT_HYP_BEGIN );
   pilot_rmFlag( player.p, PILOT_HYP_BRAKE );
   pilot_rmFlag( player.p, PILOT_HYP_PREP );
   /* Don't follow anything. */
   player_accelOver();
   player_autonavEnd();

   /* Free graphics. */
   space_gfxUnload( cur_system );

   /* Reset targets when teleporting.
    * Both of these functions invoke gui_setNav(), which updates jump and
    * spob targets simultaneously. Thus, invalid reads may arise and the
    * target reset must be done prior to calling space_init and destroying
    * the old system. */
   player_targetClearAll();

   /* Hide messages if not needed. */
   if (silent)
      player_messageToggle(0); /* space_init will reset it, so no need to. */

   /* Go to the new system. */
   space_init( name, !no_simulate );

   /* Map gets deformed when jumping this way. */
   map_clear();

   /* Initialize alpha as needed. */
   ovr_initAlpha();

   /* Run hooks - order is important. */
   pilot_outfitLOnjumpin( player.p );
   hooks_run( "jumpin" );
   hooks_run( "enter" );
   events_trigger( EVENT_TRIGGER_ENTER );
   missions_run( MIS_AVAIL_ENTER, -1, NULL, NULL );

   /* Move to spob. */
   if (pnt != NULL)
      player.p->solid.pos = pnt->pos;

   /* Move all escorts to new position. */
   Pilot *const* pilot_stack = pilot_getAll();
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];
      if (p->parent == PLAYER_ID) {
         memcpy( &p->solid.pos, &player.p->solid.pos, sizeof(vec2) );
         vec2_padd( &p->solid.pos, 200.+200.*RNGF(), 2.*M_PI*RNGF() );

         /* Clean up trails. */
         pilot_clearTrails( p );
      }
   }

   return 0;
}

/**
 * @brief Gets the dt_mod of the player, which multiplies all time stuff.
 *
 *    @luatreturn number The dt_mod of the player.
 * @luafunc dt_mod
 */
static int playerL_dt_mod( lua_State *L )
{
   lua_pushnumber(L,dt_mod);
   return 1;
}

/**
 * @brief Gets the fleet capacity (and used) of the player.
 *
 *    @luatreturn number Total fleet capacity of the player.
 *    @luatreturn number Currently used fleet capacity of the player.
 *    @luatreturn boolean Can take off.
 * @luafunc fleetCapacity
 */
static int playerL_fleetCapacity( lua_State *L )
{
   int nships = 0;
   const PlayerShip_t *pships;
   pfleet_update();
   lua_pushnumber(L,player.fleet_capacity);
   lua_pushnumber(L,player.fleet_used);
   pships = player_getShipStack();
   for (int i=0; i<array_size(pships); i++) {
      if (!pships[i].deployed)
         continue;
      nships++;
   }
   lua_pushboolean(L, (nships==0) || (player.fleet_used <= player.fleet_capacity));
   return 3;
}

/**
 * @brief Sets the fleet capacity of the player.
 *
 *    @luatparam number capacity Fleet capacity to set the player to.
 * @luafunc fleetCapacitySet
 */
static int playerL_fleetCapacitySet( lua_State *L )
{
   player.fleet_capacity = luaL_checkinteger(L,1);
   return 0;
}

/**
 * @brief Gets the player's current chapter.
 *
 *    @luatreturn string The player's current chapter.
 * @luafunc chapter
 */
static int playerL_chapter( lua_State *L )
{
   lua_pushstring( L, player.chapter );
   return 1;
}

/**
 * @brief Sets the player's current chapter.
 *
 *    @luatparam string chapter The name of the chapter to set the player to.
 * @luafunc chapterSet
 */
static int playerL_chapterSet( lua_State *L )
{
   const char *str = luaL_checkstring(L,1);
   free( player.chapter );
   player.chapter = strdup(str);
   return 0;
}

/**
 * @brief Registers a button in the info window.
 *
 *    @luatparam string caption Caption of the button.
 *    @luatparam function func Function to run when clicked.
 *    @luatparam[opt] number priority Button priority, lower is more important.
 *    @luatparam[opt] string key Hotkey for using the button without it being focused.
 *    @luatreturn number ID of the info window button for use with player.infoButtonUnregister.
 * @luafunc infoButtonRegister
 */
static int playerL_infoButtonRegister( lua_State *L )
{
   int id;
   const char *caption = luaL_checkstring( L, 1 );
   int priority = luaL_optinteger( L, 3, 5 );
   const char *key = luaL_optstring( L, 4, "" );
   luaL_checktype( L, 2, LUA_TFUNCTION );
   lua_pushvalue( L, 2 );
   id = info_buttonRegister( caption, priority, SDL_GetKeyFromName( key ) );
   lua_pushinteger( L, id );
   return 1;
}

/**
 * @brief Unregisters a button in the info window.
 *
 *    @luatparam number ID of the button to unregister.
 * @luafunc infoButtonUnregister
 */
static int playerL_infoButtonUnregister( lua_State *L )
{
   int id = luaL_checkinteger(L,1);
   int ret = info_buttonUnregister( id );
   if (ret != 0)
      WARN(_("Failed to unregister info button with id '%d'!"), id);
   return 0;
}

/**
 * @brief Global toggle to whether or not the player can discover space objects and jumps. Meant to be used with cutscenes that use player.teleport().
 *
 *    @luatparam[opt=false] Whether or not the player can discover objects.
 * @luafunc canDiscover
 */
static int playerL_canDiscover( lua_State *L )
{
   player.discover_off = !lua_toboolean(L,1);
   return 0;
}

/**
 * @brief Saves the game.
 *
 *    @luatparam[opt="autosave"] string name What to name the save.
 *    @luatparam[opt=nil] Spob|string Spob or name of spob to save the player at.
 * @luafunc save
 */
static int playerL_save( lua_State *L )
{
   const char *savename = luaL_optstring( L, 1, "autosave" );
   Spob *savespob = NULL;
   Spob *prevspob;
   if (!lua_isnoneornil(L,2))
      savespob = luaL_validspob(L,2);

   if (!landed && (savespob==NULL))
      NLUA_ERROR(L,_("Unable to save when not landed and land spob is not specified!"));
   else if (landed && (savespob!=NULL))
      NLUA_ERROR(L,_("Unable to save when landed and land_spob does not match landed spob!"));

   if (savespob != NULL) {
      prevspob = land_spob;
      land_spob = savespob;
   }
   lua_pushboolean( L, save_all_with_name( savename ) );
   if (savespob != NULL)
      land_spob = prevspob;

   return 1;
}

/**
 * @brief Backs up the player's last autosave with a custom name.
 *
 *    @luatparam string name Name to give the copy of the autosave.
 *    @luatreturn boolean true on success.
 * @luafunc saveBackup
 */
static int playerL_saveBackup( lua_State *L )
{
   char file[PATH_MAX], backup[PATH_MAX];
   const char *filename = luaL_checkstring(L,1); /* TODO sanitize path and such. */
   if (strcmp(filename,"autosave")==0)
      NLUA_ERROR(L,_("Can not back up save to 'autosave'."));
   snprintf( file, sizeof(file), "saves/%s/autosave.ns", player.name );
   snprintf( backup, sizeof(backup), "saves/%s/%s.ns", player.name, filename );
   lua_pushboolean( L, ndata_copyIfExists(file, backup) );
   return 1;
}

/**
 * @brief Gives the player a game over message.
 *
 * @luafunc gameover
 */
static int playerL_gameover( lua_State *L )
{
   (void) L;
   player_setFlag( PLAYER_DESTROYED );
   menu_death();
   return 0;
}

/**
 * @brief Gets information about the player's starting point.
 *
 *    @luatreturn Returns a table containing the different start information as keys and the corresponding information as values. Fields include things such as "name" for the campaign name, "ship" for the starting ship, "shipname" for the starting shp name, etc. Please refer to `dat/start.xml` for more details of available fields.
 * @luafunc start
 */
static int playerL_start( lua_State *L )
{
   vec2 v;
   lua_newtable(L);

   lua_pushstring( L, start_name() );
   lua_setfield( L, -2, "name" );

   lua_pushship( L, ship_get(start_ship()) );
   lua_setfield( L, -2, "ship" );

   lua_pushstring( L, start_shipname() );
   lua_setfield( L, -2, "shipname" );

   lua_pushstring( L, start_acquired() );
   lua_setfield( L, -2, "acquired" );

   lua_pushstring( L, start_gui() );
   lua_setfield( L, -2, "gui" );

   lua_pushinteger( L, start_credits() );
   lua_setfield( L, -2, "credits" );

   lua_pushtime( L, start_date() );
   lua_setfield( L, -2, "date" );

   lua_pushsystem( L, system_index(system_get(start_system())) );
   lua_setfield( L, -2, "system" );

   start_position( &v.x, &v.y );
   lua_pushvector( L, v );
   lua_setfield( L, -2, "position" );

   if (start_mission() != NULL) {
      lua_pushstring( L, start_mission() );
      lua_setfield( L, -2, "mission" );
   }

   if (start_event() != NULL) {
      lua_pushstring( L, start_event() );
      lua_setfield( L, -2, "event" );
   }

   lua_pushstring( L, start_chapter() );
   lua_setfield( L, -2, "chapter" );

   lua_pushstring( L, start_spob_lua_default() );
   lua_setfield( L, -2, "spob_lua_default" );

   lua_pushstring( L, start_dtype_default() );
   lua_setfield( L, -2, "dtype_default" );

   return 1;
}
