/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_misn.c
 *
 * @brief Handles the mission lua bindings.
 */


#include "nlua_misn.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"

#include "nlua.h"
#include "nlua_space.h"
#include "nlua_pilot.h"
#include "nlua_faction.h"
#include "nlua_var.h"
#include "hook.h"
#include "mission.h"
#include "log.h"
#include "naev.h"
#include "rng.h"
#include "space.h"
#include "toolkit.h"
#include "land.h"
#include "pilot.h"
#include "player.h"
#include "ntime.h"
#include "nxml.h"
#include "nluadef.h"
#include "music.h"
#include "unidiff.h"



/*
 * current mission
 */
static Mission *cur_mission = NULL; /**< Contains the current mission for a running script. */
static int misn_delete = 0; /**< if 1 delete current mission */


/*
 * prototypes
 */
/* static */
static unsigned int hook_generic( lua_State *L, char* stack, int pos );
static int misn_runTopStack( Mission *misn, char *func);
/* externed */
int misn_run( Mission *misn, char *func );
/* external */
extern void mission_sysMark (void);


/*
 * libraries
 */
/* misn */
static int misn_setTitle( lua_State *L );
static int misn_setDesc( lua_State *L );
static int misn_setReward( lua_State *L );
static int misn_setMarker( lua_State *L );
static int misn_factions( lua_State *L );
static int misn_accept( lua_State *L );
static int misn_finish( lua_State *L );
static int misn_timerStart( lua_State *L );
static int misn_timerStop( lua_State *L );
static int misn_takeoff( lua_State *L );
static const luaL_reg misn_methods[] = {
   { "setTitle", misn_setTitle },
   { "setDesc", misn_setDesc },
   { "setReward", misn_setReward },
   { "setMarker", misn_setMarker },
   { "factions", misn_factions },
   { "accept", misn_accept },
   { "finish", misn_finish },
   { "timerStart", misn_timerStart },
   { "timerStop", misn_timerStop },
   { "takeoff", misn_takeoff },
   {0,0}
}; /**< Mission lua methods. */
/* player */
static int player_getname( lua_State *L );
static int player_shipname( lua_State *L );
static int player_freeSpace( lua_State *L );
static int player_addCargo( lua_State *L );
static int player_rmCargo( lua_State *L );
static int player_jetCargo( lua_State *L );
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
   { "addCargo", player_addCargo },
   { "rmCargo", player_rmCargo },
   { "jetCargo", player_jetCargo },
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
/* hooks */
static int hook_land( lua_State *L );
static int hook_takeoff( lua_State *L );
static int hook_time( lua_State *L );
static int hook_enter( lua_State *L );
static int hook_pilot( lua_State *L );
static const luaL_reg hook_methods[] = {
   { "land", hook_land },
   { "takeoff", hook_takeoff },
   { "time", hook_time },
   { "enter", hook_enter },
   { "pilot", hook_pilot },
   {0,0}
}; /**< Hook Lua methods. */
/* diffs */
static int diff_applyL( lua_State *L );
static int diff_removeL( lua_State *L );
static int diff_isappliedL( lua_State *L );
static const luaL_reg diff_methods[] = {
   { "apply", diff_applyL },
   { "remove", diff_removeL },
   { "isApplied", diff_isappliedL },
   {0,0}
}; /**< Unidiff Lua methods. */
static const luaL_reg diff_cond_methods[] = {
   { "isApplied", diff_isappliedL },
   {0,0}
}; /**< Unidiff Lua read only methods. */



/**
 * @brief Registers all the mission libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int misn_loadLibs( lua_State *L )
{
   lua_loadNaev(L);
   lua_loadMisn(L);
   lua_loadVar(L,0);
   lua_loadSpace(L,0);
   lua_loadTime(L,0);
   lua_loadPlayer(L,0);
   lua_loadRnd(L);
   lua_loadTk(L);
   lua_loadHook(L);
   lua_loadPilot(L,0);
   lua_loadMusic(L,0);
   lua_loadDiff(L,0);
   lua_loadFaction(L,0);
   return 0;
}
/**
 * @brief Registers all the mission conditional libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int misn_loadCondLibs( lua_State *L )
{
   lua_loadTime(L,1);
   lua_loadSpace(L,1);
   lua_loadVar(L,1);
   lua_loadPlayer(L,1);
   lua_loadDiff(L,1);
   return 0;
}
/*
 * individual library loading
 */
/**
 * @brief Loads the mission lua library.
 *    @param L Lua state.
 */
int lua_loadMisn( lua_State *L )
{  
   luaL_register(L, "misn", misn_methods);
   return 0;
}  
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
 * @brief Loads the hook lua library.
 *    @param L Lua state.
 *    @return 0 on success.
 */
int lua_loadHook( lua_State *L )
{
   luaL_register(L, "hook", hook_methods);
   return 0;
}
/**
 * @brief Loads the diff Lua library.
 *    @param L Lua state.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int lua_loadDiff( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "diff", diff_methods);
   else
      luaL_register(L, "diff", diff_cond_methods);
   return 0;
}


/**
 * @brief Tries to run a mission, but doesn't err if it fails.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted
 *            and 0 normally.
 */
int misn_tryRun( Mission *misn, char *func )
{
   /* Get the function to run. */
   lua_getglobal( misn->L, func );
   if (lua_isnil( misn->L, -1 )) {
      lua_pop(misn->L,1);
      return 0;
   }
   return misn_runTopStack( misn, func );
}


/**
 * @brief Runs a mission function.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted
 *            and 0 normally.
 */
int misn_run( Mission *misn, char *func )
{
   /* Run the function. */
   lua_getglobal( misn->L, func );
   return misn_runTopStack( misn, func );
}


/**
 * @brief Runs the function at the top of the stack.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted
 *            and 0 normally.
 */
static int misn_runTopStack( Mission *misn, char *func)
{
   int i, ret;
   char* err;
   lua_State *L;

   cur_mission = misn;
   misn_delete = 0;
   L = misn->L;

   ret = lua_pcall(L, 0, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? (char*) lua_tostring(L,-1) : NULL;
      if (strcmp(err,"Mission Done")!=0)
         WARN("Mission '%s' -> '%s': %s",
               cur_mission->data->name, func, (err) ? err : "unknown error");
      else
         ret = 1;
   }

   /* mission is finished */
   if (misn_delete) {
      ret = 2;
      mission_cleanup( cur_mission );
      for (i=0; i<MISSION_MAX; i++)
         if (cur_mission == &player_missions[i]) {
            memmove( &player_missions[i], &player_missions[i+1],
                  sizeof(Mission) * (MISSION_MAX-i-1) );
            memset( &player_missions[MISSION_MAX-1], 0, sizeof(Mission) );
            break;
         }
   }

   cur_mission = NULL;

   return ret;
}


/**
 * @defgroup MISN Misn Lua bindings
 *
 * @brief Generic mission Lua bindings.
 *
 * Functions should be called like:
 *
 * @code
 * misn.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @brief setTitle( string title )
 *
 * Sets the current mission title.
 *
 *    @param title Title to use for mission.
 */
static int misn_setTitle( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L, 1)) {
      if (cur_mission->title) /* cleanup old title */
         free(cur_mission->title);
      cur_mission->title = strdup((char*)lua_tostring(L, 1));
   }
   return 0;
}
/**
 * @brief setDesc( string desc )
 *
 * Sets the current mission description.
 *
 *    @param desc Description to use for mission.
 */
static int misn_setDesc( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L, 1)) {    
      if (cur_mission->desc) /* cleanup old description */
         free(cur_mission->desc);
      cur_mission->desc = strdup((char*)lua_tostring(L, 1));
   }
   else NLUA_INVALID_PARAMETER();
   return 0;
}
/**
 * @brief setReward( string reward )
 *
 * Sets the current mission reward description.
 *
 *    @param reward Description of the reward to use.
 */
static int misn_setReward( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L, 1)) {    
      if (cur_mission->reward != NULL) /* cleanup old reward */
         free(cur_mission->reward);
      cur_mission->reward = strdup((char*)lua_tostring(L, 1));
   }
   else NLUA_INVALID_PARAMETER();
   return 0;
}
/**
 * @brief setMarker( [system sys )
 *
 * Sets the mission marker on the system.  If no parameters are passed it
 * unsets the current marker.
 *
 *    @param sys System to mark.
 */
static int misn_setMarker( lua_State *L )
{
   LuaSystem *sys;

   /* No parameter clears the marker */
   if (lua_gettop(L)==0) {
      if (cur_mission->sys_marker != NULL)
         free(cur_mission->sys_marker);
      mission_sysMark(); /* Clear the marker */
   }

   /* Passing in a Star System */
   if (lua_issystem(L,1)) {
      sys = lua_tosystem(L,1);
      cur_mission->sys_marker = strdup(sys->s->name);
      mission_sysMark(); /* mark the system */
   }
   else NLUA_INVALID_PARAMETER();

   return 0;
}
/**
 * @brief table factions( nil )
 *
 * Gets the factions the mission is available for.
 *
 *    @return A containing the factions.
 */
static int misn_factions( lua_State *L )
{
   int i;
   MissionData *dat;
   LuaFaction f;

   dat = cur_mission->data;

   /* we'll push all the factions in table form */
   lua_newtable(L);
   for (i=0; i<dat->avail.nfactions; i++) {
      lua_pushnumber(L,i+1); /* index, starts with 1 */
      f.f = dat->avail.factions[i];
      lua_pushfaction(L, f); /* value */
      lua_rawset(L,-3); /* store the value in the table */
   }
   return 1;
}
/**
 * @brief bool accept( nil )
 *
 * Attempts to accept the mission.
 *
 *    @return true if mission was properly accepted.
 */
static int misn_accept( lua_State *L )
{
   int i, ret;

   ret = 0;

   /* find last mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data == NULL) break;

   /* no missions left */
   if (i>=MISSION_MAX) ret = 1;
   else { /* copy it over */
      memcpy( &player_missions[i], cur_mission, sizeof(Mission) );
      memset( cur_mission, 0, sizeof(Mission) );
      cur_mission = &player_missions[i];
   }

   lua_pushboolean(L,!ret); /* we'll convert C style return to lua */
   return 1;
}
/**
 * @brief finish( bool properly )
 *
 * Finishes the mission.
 *
 *    @param properly If true and the mission is unique it marks the mission
 *                     as completed.  If false it deletes the mission but
 *                     doesn't mark it as completed.  If the parameter isn't
 *                     passed it just ends the mission.
 */
static int misn_finish( lua_State *L )
{
   int b;

   if (lua_isboolean(L,1)) b = lua_toboolean(L,1);
   else {
      lua_pushstring(L, "Mission Done");
      lua_error(L); /* THERE IS NO RETURN */
      return 0;
   }

   misn_delete = 1;

   if (b && mis_isFlag(cur_mission->data,MISSION_UNIQUE))
      player_missionFinished( mission_getID( cur_mission->data->name ) );

   lua_pushstring(L, "Mission Done");
   lua_error(L); /* shouldn't return */

   return 0;
}

/**
 * @brief number timerStart( string func, number delay )
 *
 * Starts a timer.
 *
 *    @param func Function to run when timer is up.
 *    @param delay Milliseconds to wait for timer.
 *    @return The timer being used.
 */
static int misn_timerStart( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int i;
   char *func;
   double delay;

   /* Parse arguments. */
   if (lua_isstring(L,1))
      func = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,2))
      delay = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Add timer */
   for (i=0; i<MISSION_TIMER_MAX; i++) {
      if (cur_mission->timer[i] == 0.) {
         cur_mission->timer[i] = delay / 1000.;
         cur_mission->tfunc[i] = strdup(func);
         break;
      }
   }

   /* No timer found. */
   if (i >= MISSION_TIMER_MAX) {
      return 0;
   }

   /* Returns the timer id. */
   lua_pushnumber(L,i);
   return 1;
}

/**
 * @brief timerStop( number t )
 *
 * Stops a timer previously started with timerStart().
 *
 *    @param t Timer to stop.
 */
static int misn_timerStop( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   int t;

   /* Parse parameters. */
   if (lua_isnumber(L,1))
      t = (int)lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   /* Stop the timer. */
   if (cur_mission->timer[t] != 0.) {
      cur_mission->timer[t] = 0.;
      if (cur_mission->tfunc[t] != NULL) {
         free(cur_mission->tfunc[t]);
         cur_mission->tfunc[t] = NULL;
      }
   }

   return 0;
}


/**
 * @brief Forces the player to take off if he is landed.
 *
 * @luafunc takeoff()
 */
static int misn_takeoff( lua_State *L )
{
   (void) L;

   if (landed)
      landed = 0;

   return 0;
}
/**
 * @}
 */



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
 * @brief number addCargo( string cargo, number quantity )
 *
 * Adds some mission cargo to the player.  He cannot sell it nor get rid of it
 *  unless he abandons the mission in which case it'll get eliminated.
 *
 *    @param cargo Name of the cargo to add.
 *    @param quantity Quantity of cargo to add.
 *    @return The id of the cargo which can be used in rmCargo.
 */
static int player_addCargo( lua_State *L )
{
   Commodity *cargo;
   int quantity, ret;

   NLUA_MIN_ARGS(2);

   if (lua_isstring(L,1)) cargo = commodity_get( (char*) lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,2)) quantity = (int) lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   /* First try to add the cargo. */
   ret = pilot_addMissionCargo( player, cargo, quantity );
   mission_linkCargo( cur_mission, ret );

   lua_pushnumber(L, ret);
   return 1;
}
/**
 * @brief bool rmCargo( number cargoid )
 *
 * Removes the mission cargo.
 *
 *    @param cargoid Identifier of the mission cargo.
 *    @return true on success.
 */
static int player_rmCargo( lua_State *L )
{
   int ret;
   unsigned int id;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) id = (unsigned int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   /* First try to remove the cargo from player. */
   if (pilot_rmMissionCargo( player, id, 0 ) != 0) {
      lua_pushboolean(L,0);
      return 1;
   }

   /* Now unlink the mission cargo if it was successful. */
   ret = mission_unlinkCargo( cur_mission, id );

   lua_pushboolean(L,!ret);
   return 1;
}
/**
 * @brief jetCargo( number cargoid )
 *
 * Jettisons the mission cargo.
 *
 *    @param cargoid ID of the cargo to jettison.
 */
static int player_jetCargo( lua_State *L )
{
   int ret;
   unsigned int id;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) id = (unsigned int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   /* First try to remove the cargo from player. */
   if (pilot_rmMissionCargo( player, id, 1 ) != 0) {
      lua_pushboolean(L,0);
      return 1;
   }

   /* Now unlink the mission cargo if it was successful. */
   ret = mission_unlinkCargo( cur_mission, id );

   lua_pushboolean(L,!ret);
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



/**
 * @defgroup HOOK Hook Lua bindings
 *
 * @brief Lua bindings to manipulate hooks.
 *
 * Functions should be called like:
 *
 * @code
 * hook.function( parameters )
 * @endcode
 */
/**
 * @brief Creates a mission hook to a certain stack.
 *
 * Basically a generic approach to hooking.
 *
 *    @param L Lua state.
 *    @param stack Stack to put the hook in.
 *    @param pos Position in the stack of the function name.
 *    @return The hook ID or 0 on error.
 */
static unsigned int hook_generic( lua_State *L, char* stack, int pos )
{
   int i;
   char *func;

   NLUA_MIN_ARGS(1);

   /* Last parameter must be function to hook */
   if (lua_isstring(L,pos)) func = (char*)lua_tostring(L,pos);
   else NLUA_INVALID_PARAMETER();

   /* make sure mission is a player mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].id == cur_mission->id)
         break;
   if (i>=MISSION_MAX) {
      WARN("Mission not in stack trying to hook");
      return 0;
   }

   return hook_add( cur_mission->id, func, stack );
}
/**
 * @ingroup HOOK
 *
 * @brief number land( string func )
 *
 * Hooks the function to the player landing.
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_land( lua_State *L )
{
   hook_generic( L, "land", 1 );
   return 0;
}
/**
 * @ingroup HOOK
 *
 * @brief number takeoff( string func )
 *
 * Hooks the function to the player taking off
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_takeoff( lua_State *L )
{
   hook_generic( L, "takeoff", 1 );
   return 0;
}
/**
 * @ingroup HOOK
 *
 * @brief number time( string func )
 *
 * Hooks the function to a time change.
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_time( lua_State *L )
{
   hook_generic( L, "time", 1 );
   return 0;
}
/**
 * @ingroup HOOK
 *
 * @brief number enter( string func )
 *
 * Hooks the function to the player entering a system (triggers when taking
 *  off too).
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_enter( lua_State *L )
{
   hook_generic( L, "enter", 1 );
   return 0;
}
/**
 * @ingroup HOOK
 *
 * @brief number pilot( Pilot pilot, string type, string func )
 *
 * Hooks the function to a specific pilot.
 *
 * You can hook to different actions.  Curently hook system only supports:
 *    - "death" :  triggered when pilot dies.
 *    - "board" :  triggered when pilot is boarded.
 *    - "disable" :  triggered when pilot is disabled.
 *    - "jump" : triggered when pilot jumps to hyperspace.
 *
 *    @param pilot Pilot identifier to hook.
 *    @param type One of the supported hook types.
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_pilot( lua_State *L )
{
   NLUA_MIN_ARGS(3);
   unsigned int h;
   LuaPilot *p;
   int type;
   char *hook_type;

   /* First parameter parameter - pilot to hook */
   if (lua_ispilot(L,1)) p = lua_topilot(L,1);
   else NLUA_INVALID_PARAMETER();

   /* Second parameter - hook name */
   if (lua_isstring(L,2)) hook_type = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Check to see if hook_type is valid */
   if (strcmp(hook_type,"death")==0) type = PILOT_HOOK_DEATH;
   else if (strcmp(hook_type,"board")==0) type = PILOT_HOOK_BOARD;
   else if (strcmp(hook_type,"disable")==0) type = PILOT_HOOK_DISABLE;
   else if (strcmp(hook_type,"jump")==0) type = PILOT_HOOK_JUMP;
   else { /* hook_type not valid */
      NLUA_DEBUG("Invalid pilot hook type: '%s'", hook_type);
      return 0;
   }

   /* actually add the hook */
   h = hook_generic( L, hook_type, 3 );
   pilot_addHook( pilot_get(p->pilot), type, h );

   return 0;
}


/**
 * @defgroup DIFF Universe Diff Lua Bindings
 *
 * @brief Lua bindings to apply/remove Universe Diffs.
 *
 * Functions should be called like:
 *
 * @code
 * diff.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @brief apply( string name )
 *
 * Applies a diff by name.
 *
 *    @param name Name of the diff to apply.
 */
static int diff_applyL( lua_State *L )
{
   char *name;

   if (lua_isstring(L,1)) name = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   diff_apply( name );
   return 0;
}
/**
 * @brief remove( string name )
 *
 * Removes a diff by name.
 *
 *    @param name Name of the diff to remove.
 */
static int diff_removeL( lua_State *L )
{
   char *name;

   if (lua_isstring(L,1)) name = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   diff_remove( name );
   return 0;
}
/**
 * @brief bool isApplied( string name )
 *
 * Checks to see if a diff is currently applied.
 *
 *    @param name Name of the diff to check.
 *    @return true if is applied, false if it isn't.
 */
static int diff_isappliedL( lua_State *L )
{
   char *name;

   if (lua_isstring(L,1)) name = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   lua_pushboolean(L,diff_isApplied(name));
   return 1;
}
/**
 * @}
 */
