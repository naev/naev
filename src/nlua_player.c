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
#include "nstring.h"
#include <math.h>

#include <lua.h>
#include <lauxlib.h>

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
#include "nlua_planet.h"
#include "map.h"
#include "map_overlay.h"
#include "hook.h"
#include "comm.h"
#include "land_outfits.h"
#include "gui.h"
#include "gui_omsg.h"
#include "pause.h"


/* Player methods. */
static int playerL_getname( lua_State *L );
static int playerL_shipname( lua_State *L );
static int playerL_pay( lua_State *L );
static int playerL_credits( lua_State *L );
static int playerL_msg( lua_State *L );
static int playerL_msgClear( lua_State *L );
static int playerL_omsgAdd( lua_State *L );
static int playerL_omsgChange( lua_State *L );
static int playerL_omsgRm( lua_State *L );
static int playerL_allowSave( lua_State *L );
/* Faction stuff. */
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
static int playerL_landWindow( lua_State *L );
/* Hail stuff. */
static int playerL_commclose( lua_State *L );
/* Cargo stuff. */
static int playerL_numOutfit( lua_State *L );
static int playerL_addOutfit( lua_State *L );
static int playerL_rmOutfit( lua_State *L );
static int playerL_addShip( lua_State *L );
static int playerL_swapShip( lua_State *L );
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
   { "msgClear", playerL_msgClear },
   { "omsgAdd", playerL_omsgAdd },
   { "omsgChange", playerL_omsgChange },
   { "omsgRm", playerL_omsgRm },
   { "allowSave", playerL_allowSave },
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
   { "landWindow", playerL_landWindow },
   { "commClose", playerL_commclose },
   { "numOutfit", playerL_numOutfit },
   { "addOutfit", playerL_addOutfit },
   { "rmOutfit", playerL_rmOutfit },
   { "addShip", playerL_addShip },
   { "swapShip", playerL_swapShip },
   { "misnActive", playerL_misnActive },
   { "misnDone", playerL_misnDone },
   { "evtActive", playerL_evtActive },
   { "evtDone", playerL_evtDone },
   { "teleport", playerL_teleport },
   {0,0}
}; /**< Player Lua methods. */
static const luaL_reg playerL_cond_methods[] = {
   { "name", playerL_getname },
   { "ship", playerL_shipname },
   { "credits", playerL_credits },
   { "getRating", playerL_getRating },
   { "pos", playerL_getPosition },
   { "pilot", playerL_getPilot },
   { "fuel", playerL_fuel },
   { "autonav", playerL_autonav },
   { "autonavDest", playerL_autonavDest },
   { "numOutfit", playerL_numOutfit },
   { "misnActive", playerL_misnActive },
   { "misnDone", playerL_misnDone },
   { "evtActive", playerL_evtActive },
   { "evtDone", playerL_evtDone },
   {0,0}
}; /**< Conditional player Lua methods. */


/*
 * Prototypes.
 */
static Pilot* playerL_newShip( lua_State *L );


/**
 * @brief Loads the player Lua library.
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
   double money;

   money = luaL_checknumber(L,1);
   player_modCredits( (credits_t)round(money) );

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
 * @brief Clears the player's message buffer.
 *
 * @luafunc msgClear()
 */
static int playerL_msgClear( lua_State *L )
{
   (void) L;
   gui_clearMessages();
   return 0;
}
/**
 * @brief Adds an overlay message.
 *
 * @usage player.omsgAdd( "some_message", 5 )
 *    @luaparam msg Message to add.
 *    @luaparam duration Duration to add message (if 0. is infinite).
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

   /* Infinity. */
   if (duration < 1e-10)
      duration = INFINITY;

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
 *    @luaparam duration New duration to set (0. for infinity).
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
 * @brief Sets player save ability.
 *
 * @usage player.allowSave( b )
 *    @luaparam b true if the player is allowed to save, false otherwise. Defaults to true.
 * @luafunc allowSave( b )
 */
static int playerL_allowSave( lua_State *L )
{
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
 *  <li>abort : (string) autonav abort message</li>
 *  <li>no2x : (boolean) whether to prevent the player from engaging double-speed, default false</li>
 *  <li>gui : (boolean) enables the player's gui, default disabled</li>
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
   f_gui     = 0;
   f_2x      = 0;

   /* Parse parameters. */
   b = lua_toboolean( L, 1 );
   if (lua_gettop(L) > 1) {
      if (!lua_istable(L,2)) {
         NLUA_ERROR( L, "Second parameter to cinematics should be a table of options or omitted!" );
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
   }

   /* Remove doublespeed. */
   if (player_isFlag( PLAYER_DOUBLESPEED )) {
      player_rmFlag( PLAYER_DOUBLESPEED );
      pause_setSpeed(1.);
   }

   if (b) {
      /* Do stuff. */
      player_autonavAbort( abort_msg );
      player_rmFlag( PLAYER_DOUBLESPEED );
      ovr_setOpen(0);
      pause_setSpeed(1.);

      if (!f_gui)
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
   if (!landed) {
      NLUA_ERROR(L,"Player must be landed to force takeoff.");
      return 0;
   }

   land_queueTakeoff();

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
 *    @luaparam b Whether or not to allow the player to land (defaults to true if omitted).
 *    @luaparam msg Message displayed when player tries to land (only if disallowed to land). Can be omitted to use default.
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
 *    @luaparam winname Name of the window.
 *    @luareturn True on success.
 * @luafunc landWindow( winname )
 */
static int playerL_landWindow( lua_State *L )
{
   int ret;
   const char *str;
   int win;

   if (!landed) {
      NLUA_ERROR(L, "Must be landed to set the active land window.");
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
 * @brief Gets the number of outfits the player owns in his list (excludes equipped on ships).
 *
 * @usage q = player.numOutfit( "Laser Cannon" ) -- Number of 'Laser Cannons' the player owns (unequipped)
 *
 *    @luaparam name Name of the outfit to give.
 *    @luareturn The quantity the player owns.
 * @luafunc addOutfit( name )
 */
static int playerL_numOutfit( lua_State *L )
{
   const char *str;
   Outfit *o;
   int q;

   /* Handle parameters. */
   str = luaL_checkstring(L, 1);

   /* Get outfit. */
   o = outfit_get( str );
   if (o==NULL) {
      NLUA_ERROR(L, "Outfit '%s' not found.", str);
      return 0;
   }

   /* Count the outfit. */
   q = player_outfitOwned( o );
   lua_pushnumber( L, q );

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

   /* Update equipment list. */
   outfits_updateEquipmentOutfits();

   return 0;
}
/**
 * @brief Removes an outfit from the player's outfit list.
 *
 * "all" will remove all outfits.
 *
 * @usage player.rmOutfit( "Plasma Blaster", 2 ) -- Removes two plasma blasters from the player
 *
 *    @luaparam name Name of the outfit to give.
 *    @luaparam q Optional parameter that sets the quantity to give (default 1).
 * @luafunc rmOutfit( name, q )
 */
static int playerL_rmOutfit( lua_State *L )
{
   const char *str;
   char **outfits;
   Outfit *o;
   int i, q, noutfits;

   /* Defaults. */
   q = 1;

   /* Handle parameters. */
   str = luaL_checkstring(L, 1);
   if (lua_gettop(L) > 1)
      q = luaL_checkint(L, 2);

   if (strcmp(str,"all")==0) {
      noutfits = player_numOutfits();
      /* Removing nothing is a bad idea. */
      if (noutfits == 0)
         return 0;

      outfits = malloc( sizeof(char*) * noutfits );
      player_getOutfits(outfits, NULL);
      for (i=0; i<noutfits; i++) {
         o = outfit_get(outfits[i]);
         q = player_outfitOwned(o);
         player_rmOutfit(o, q);
         /* Free memory. */
         free( outfits[i] );
      }
      /* Clean up. */
      free(outfits);
   }
   else {
      /* Get outfit. */
      o = outfit_get( str );
      if (o==NULL) {
         NLUA_ERROR(L, "Outfit '%s' not found.", str);
         return 0;
      }

      /* Remove the outfits. */
      player_rmOutfit( o, q );
   }

   /* Update equipment list. */
   outfits_updateEquipmentOutfits();

   return 0;
}


/**
 * @brief Helper function for playerL_addShip.
 */
static Pilot* playerL_newShip( lua_State *L )
{
   const char *str, *name, *pntname;
   Ship *s;
   Pilot *new_ship;
   Planet *pnt, *t;
   int noname;

   /* Defaults. */
   t = NULL;

   /* Handle parameters. */
   str  = luaL_checkstring(L, 1);
   if (lua_gettop(L) > 1)
      name = luaL_checkstring(L,2);
   else
      name = str;
   if (lua_isstring(L,3))
      pntname = luaL_checkstring (L,3);
   else {
      if (!landed)
         NLUA_ERROR(L,"Must be landed to add a new ship to the player without specifying planet to add to!");
      pntname = NULL;
   }
   noname = lua_toboolean(L,4);

   /* Get planet. */
   if (pntname != NULL) {
      pnt = planet_get( pntname );
      if (pnt == NULL) {
         NLUA_ERROR(L, "Planet '%s' not found!", pntname);
         return 0;
      }
      /* Horrible hack to swap variables. */
      t = land_planet;
      land_planet = pnt;
   }
   else
      pnt = NULL;

   /* Must be landed if pnt is NULL. */
   if ((pnt == NULL) && (land_planet==NULL)) {
      NLUA_ERROR(L, "Player must be landed to add a ship without location parameter.");
      return 0;
   }

   /* Get ship. */
   s = ship_get(str);
   if (s==NULL) {
      NLUA_ERROR(L, "Ship '%s' not found.", str);
      return 0;
   }

   /* Add the ship, look in case it's cancelled. */
   do {
      new_ship = player_newShip( s, name, 0, noname );
   } while (new_ship == NULL);

   /* Undo the horrible hack. */
   if (t != NULL)
      land_planet = t;

   return new_ship;
}


/**
 * @brief Gives the player a new ship.
 *
 * @note Should be given when landed, ideally on a planet with a shipyard.
 *
 * @usage player.addShip( "Pirate Kestrel", "Seiryuu" ) -- Gives the player a Pirate Kestrel named Seiryuu if player cancels the naming.
 *
 *    @luaparam ship Name of the ship to add.
 *    @luaparam name Name to give the ship if player refuses to name it (defaults to shipname if omitted).
 *    @luaparam loc Location to add to, if nil or omitted it adds it to local planet (must be landed).
 *    @luaparam noname If true does not let the player name the ship (defaults to false).
 * @luafunc addShip( ship, name, loc, noname )
 */
static int playerL_addShip( lua_State *L )
{
   playerL_newShip( L );
   return 0;
}


/**
 * @brief Swaps the player's current ship with a new ship given to him.
 *    @luaparam ship Name of the ship to add.
 *    @luaparam name Name to give the ship if player refuses to name it (defaults to shipname if omitted).
 *    @luaparam loc Location to add to, if nil or omitted it adds it to local planet (must be landed).
 *    @luaparam noname If true does not let the player name the ship (defaults to false).
 *    @luaparam remove If true removes the player's current ship (so it replaces and doesn't swap).
 * @luafunc swapShip( ship, name, loc, noname, remove )
 */
static int playerL_swapShip( lua_State *L )
{
   Pilot *p;
   char *cur;
   int remship;

   remship = lua_toboolean(L,5);
   p       = playerL_newShip( L );
   cur     = player.p->name;
   player_swapShip( p->name );
   if (remship)
      player_rmShip( cur );

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
 * This only works with missions that have the unique flag.
 *
 * @usage if player.misnDone( "The Space Family" ) then -- Player finished mission
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
 * This only works with events that have the unique flag.
 *
 * @usage if player.evtDone( "Shipwreck" ) then -- Player finished event
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
 * @brief Teleports the player to a new planet or system (only if not landed).
 *
 * If the destination is a system, the coordinates of the player will not change.
 * If the destination is a planet, the player will be placed over that planet.
 *
 * @usage player.teleport( system.get("Arcanis") ) -- Teleports the player to Arcanis.
 * @usage player.teleport( "Arcanis" ) -- Teleports the player to Arcanis.
 * @usage player.teleport( "Dvaer Prime" ) -- Teleports the player to Dvaer, and relocates him to Dvaer Prime.
 *
 *    @luaparam dest System or name of a system or planet or name of a planet to teleport the player to.
 * @luafunc teleport( dest )
 */
static int playerL_teleport( lua_State *L )
{
   Planet *pnt;
   StarSystem *sys;
   const char *name, *pntname;

   /* Must not be landed. */
   if (landed)
      NLUA_ERROR(L,"Can not teleport the player while landed!");
   if (comm_isOpen())
      NLUA_ERROR(L,"Can not teleport the player while the comm is open!");
   if (player_isBoarded())
      NLUA_ERROR(L,"Can not teleport the player while he is boarded!");

   pnt = NULL;

   /* Get a system. */
   if (lua_issystem(L,1)) {
      sys   = luaL_validsystem(L,1);
      name  = system_getIndex(sys->id)->name;
   }
   /* Get a planet. */
   else if (lua_isplanet(L,1)) {
      pnt   = luaL_validplanet(L,1);
      name  = planet_getSystem( pnt->name );
      if (name == NULL) {
         NLUA_ERROR( L, "Planet '%s' does not belong to a system..", pnt->name );
         return 0;
      }
   }
   /* Get destination from string. */
   else if (lua_isstring(L,1)) {
      name = lua_tostring(L,1);
      if (!system_exists( name )) {
         if (!planet_exists( name )) {
            NLUA_ERROR( L, "'%s' is not a valid teleportation target.", name );
            return 0;
         }

         /* No system found, assume destination string is the name of a planet. */
         pntname = name;
         name = planet_getSystem( name );
         pnt  = planet_get( pntname );
         if (name == NULL) {
            NLUA_ERROR( L, "Planet '%s' does not belong to a system..", pntname );
            return 0;
         }
      }
   }
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
   pilot_rmFlag( player.p, PILOT_HYP_BRAKE );
   pilot_rmFlag( player.p, PILOT_HYP_PREP );

   /* Free graphics. */
   space_gfxUnload( cur_system );

   /* Reset targets when teleporting.
    * Both of these functions invoke gui_setNav(), which updates jump and
    * planet targets simultaneously. Thus, invalid reads may arise and the
    * target reset must be done prior to calling space_init and destroying
    * the old system.
    */
   player_targetHyperspaceSet( -1 );
   player_targetPlanetSet( -1 );

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
   missions_run( MIS_AVAIL_SPACE, -1, NULL, NULL );

   /* Move to planet. */
   if (pnt != NULL)
      vectcpy( &player.p->solid->pos, &pnt->pos );

   return 0;
}


