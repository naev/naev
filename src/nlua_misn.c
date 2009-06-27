/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_misn.c
 *
 * @brief Handles the mission lua bindings.
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
#include "nlua_hook.h"
#include "nlua_player.h"
#include "nlua_tk.h"
#include "nlua_faction.h"
#include "nlua_space.h"
#include "player.h"
#include "mission.h"
#include "log.h"
#include "rng.h"
#include "toolkit.h"
#include "land.h"
#include "nxml.h"
#include "nluadef.h"
#include "music.h"
#include "gui_osd.h"



/*
 * current mission
 */
Mission *cur_mission = NULL; /**< Contains the current mission for a running script. */
static int misn_delete = 0; /**< if 1 delete current mission */


/*
 * prototypes
 */
/* static */
static int misn_runTopStack( Mission *misn, const char *func);
/* externed */
int misn_run( Mission *misn, const char *func );
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
static int misn_addCargo( lua_State *L );
static int misn_rmCargo( lua_State *L );
static int misn_jetCargo( lua_State *L );
static int misn_osdCreate( lua_State *L );
static int misn_osdDestroy( lua_State *L );
static int misn_osdActive( lua_State *L );
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
   { "addCargo", misn_addCargo },
   { "rmCargo", misn_rmCargo },
   { "jetCargo", misn_jetCargo },
   { "osdCreate", misn_osdCreate },
   { "osdDestroy", misn_osdDestroy },
   { "osdActive", misn_osdActive },
   {0,0}
}; /**< Mission lua methods. */


/**
 * @brief Registers all the mission libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int misn_loadLibs( lua_State *L )
{
   nlua_loadStandard(L,0);
   lua_loadMisn(L);
   lua_loadTk(L);
   lua_loadHook(L);
   lua_loadMusic(L,0);
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
 * @brief Tries to run a mission, but doesn't err if it fails.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted
 *            and 0 normally.
 */
int misn_tryRun( Mission *misn, const char *func )
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
int misn_run( Mission *misn, const char *func )
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
static int misn_runTopStack( Mission *misn, const char *func)
{
   int i, ret;
   const char* err;
   lua_State *L;

   cur_mission = misn;
   misn_delete = 0;
   L = misn->L;

   ret = lua_pcall(L, 0, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
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
 * @brief Sets the mission OSD if applicable.
 */
static void setOSD (void)
{
   /* OSD set explicitly. */
   if (cur_mission->osd_set)
      return;

   /* Needs title and description. */
   if ((cur_mission->title==NULL) || (cur_mission->desc==NULL))
      return;

   /* Mission must be accepted. */
   if (!cur_mission->accepted)
      return;

   /* Destroy existing OSD. */
   if (cur_mission->osd > 0)
      osd_destroy(cur_mission->osd);

   /* Set the OSD. */
   cur_mission->osd = osd_create( cur_mission->title, 1,
         (const char **)&cur_mission->desc );
}


/**
 * @brief Sets the current mission title.
 *
 *    @luaparam title Title to use for mission.
 * @luafunc setTitle( title )
 */
static int misn_setTitle( lua_State *L )
{
   const char *str;

   str = luaL_checkstring(L,1);

   if (cur_mission->title) /* cleanup old title */
      free(cur_mission->title);
   cur_mission->title = strdup(str);

   /* Set the OSD if needed. */
   setOSD();

   return 0;
}
/**
 * @brief Sets the current mission description.
 *
 *    @luaparam desc Description to use for mission.
 * @luafunc setDesc( desc )
 */
static int misn_setDesc( lua_State *L )
{
   const char *str;

   str = luaL_checkstring(L,1);

   if (cur_mission->desc) /* cleanup old description */
      free(cur_mission->desc);
   cur_mission->desc = strdup(str);

   /* Set the OSD if needed. */
   setOSD();

   return 0;
}
/**
 * @brief Sets the current mission reward description.
 *
 *    @luaparam reward Description of the reward to use.
 * @luafunc setReward( reward )
 */
static int misn_setReward( lua_State *L )
{
   const char *str;

   str = luaL_checkstring(L,1);

   if (cur_mission->reward) /* cleanup old reward */
      free(cur_mission->reward);
   cur_mission->reward = strdup(str);
   return 0;
}
/**
 * @brief Sets the mission marker on the system.  If no parameters are passed it
 * unsets the current marker.
 *
 * There are basically three different types of markers:
 *
 *  - "misc" : These markers are for unique or non-standard missions.
 *  - "cargo" : These markers are for regular cargo hauling missions.
 *  - "rush" : These markers are for timed missions.
 *
 * @usage misn.setMarker() -- Clears the marker
 * @usage misn.setMarker( sys, "misc" ) -- Misc mission marker.
 * @usage misn.setMarker( sys, "cargo" ) -- Cargo mission marker.
 * @usage misn.setMarker( sys, "rush" ) -- Rush mission marker.
 *
 *    @luaparam sys System to mark.  Unmarks if no parameter or nil is passed.
 *    @luaparam type Optional parameter that specifies mission type.  Can be one of
 *          "misc", "rush" or "cargo".
 * @luafunc setMarker( sys, type )
 */
static int misn_setMarker( lua_State *L )
{
   const char *str;
   LuaSystem *sys;

   /* No parameter clears the marker */
   if (lua_gettop(L)==0) {
      if (cur_mission->sys_marker != NULL)
         free(cur_mission->sys_marker);
      mission_sysMark(); /* Clear the marker */
   }

   /* Passing in a Star System */
   sys = luaL_checksystem(L,1);
   sys = lua_tosystem(L,1);
   cur_mission->sys_marker = strdup(sys->s->name);

   /* Get the type. */
   if (lua_gettop(L) > 1) {
      str = luaL_checkstring(L,2);
      if (strcmp(str, "misc")==0)
         cur_mission->sys_markerType = SYSMARKER_MISC;
      else if (strcmp(str, "rush")==0)
         cur_mission->sys_markerType = SYSMARKER_RUSH;
      else if (strcmp(str, "cargo")==0)
         cur_mission->sys_markerType = SYSMARKER_CARGO;
      else
         NLUA_DEBUG("Unknown marker type: %s", str);
   }

   mission_sysMark(); /* mark the system */

   return 0;
}
/**
 * @brief Gets the factions the mission is available for.
 *
 * @usage f = misn.factions()
 *    @luareturn A containing the factions for whom the mission is available.
 * @luafunc factions()
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
 * @brief Attempts to accept the mission.
 *
 * @usage if not misn.accept() then return end
 *    @luareturn true if mission was properly accepted.
 * @luafunc accept()
 */
static int misn_accept( lua_State *L )
{
   int i, ret;

   ret = 0;

   /* find last mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data == NULL)
         break;

   /* no missions left */
   if (i>=MISSION_MAX)
      ret = 1;
   else { /* copy it over */
      memcpy( &player_missions[i], cur_mission, sizeof(Mission) );
      memset( cur_mission, 0, sizeof(Mission) );
      cur_mission = &player_missions[i];
      cur_mission->accepted = 1; /* Mark as accepted. */
      setOSD(); /* Set OSD if applicable. */
   }

   lua_pushboolean(L,!ret); /* we'll convert C style return to lua */
   return 1;
}
/**
 * @brief Finishes the mission.
 *
 *    @luaparam properly If true and the mission is unique it marks the mission
 *                     as completed.  If false it deletes the mission but
 *                     doesn't mark it as completed.  If the parameter isn't
 *                     passed it just ends the mission.
 * @luafunc finish( properly )
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
 * @brief Starts a timer.
 *
 *    @luaparam funcname Name of the function to run when timer is up.
 *    @luaparam delay Milliseconds to wait for timer.
 *    @luareturn The timer being used.
 * @luafunc timerStart( funcname, delay )
 */
static int misn_timerStart( lua_State *L )
{
   int i;
   const char *func;
   double delay;

   /* Parse arguments. */
   func  = luaL_checkstring(L,1);
   delay = luaL_checknumber(L,2);

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
 * @brief Stops a timer previously started with timerStart().
 *
 *    @luaparam t Timer to stop.
 * @luafunc timerStop( t )
 */
static int misn_timerStop( lua_State *L )
{
   int t;

   /* Parse parameters. */
   t = luaL_checkint(L,1);

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
 * @brief Adds some mission cargo to the player.  He cannot sell it nor get rid of it
 *  unless he abandons the mission in which case it'll get eliminated.
 *
 *    @luaparam cargo Name of the cargo to add.
 *    @luaparam quantity Quantity of cargo to add.
 *    @luareturn The id of the cargo which can be used in rmCargo.
 * @luafunc addCargo( cargo, quantity )
 */
static int misn_addCargo( lua_State *L )
{
   const char *cname;
   Commodity *cargo;
   int quantity, ret;

   /* Parameters. */
   cname    = luaL_checkstring(L,1);
   quantity = luaL_checkint(L,2);
   cargo = commodity_get( cname );

   /* First try to add the cargo. */
   ret = pilot_addMissionCargo( player, cargo, quantity );
   mission_linkCargo( cur_mission, ret );

   lua_pushnumber(L, ret);
   return 1;
}
/**
 * @brief Removes the mission cargo.
 *
 *    @luaparam cargoid Identifier of the mission cargo.
 *    @luareturn true on success.
 * @luafunc rmCargo( cargoid )
 */
static int misn_rmCargo( lua_State *L )
{
   int ret;
   unsigned int id;

   id = luaL_checklong(L,1);

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
 * @brief Jettisons the mission cargo.
 *
 *    @luaparam cargoid ID of the cargo to jettison.
 *    @luareturn true on success.
 * @luafunc jetCargo( cargoid )
 */
static int misn_jetCargo( lua_State *L )
{
   int ret;
   unsigned int id;

   id = luaL_checklong(L,1);

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
 * @brief Creates a mission OSD.
 *
 * @note You can index elements by using '\t' as first character of an element.
 *
 * @usage misn.osdCreate( "My OSD", {"Element 1", "Element 2"})
 *
 *    @luaparam Title Title to give the OSD.
 *    @luaparam list List of elements to put in the OSD.
 * @luafunc osdCreate( title, list )
 */
static int misn_osdCreate( lua_State *L )
{
   const char *title;
   int nitems;
   const char **items;
   int i;

   /* Check parameters. */
   title  = luaL_checkstring(L,1);
   luaL_checktype(L,2,LUA_TTABLE);
   nitems = lua_objlen(L,2);

   /* Destroy OSD if already exists. */
   if (cur_mission->osd != 0) {
      osd_destroy( cur_mission->osd );
      cur_mission->osd = 0;
   }

   /* Allocate items. */
   items = calloc( nitems, sizeof(char *) );

   /* Get items. */
   i = 0;
   lua_pushnil(L); /* table, nil */
   while (lua_next(L,-2) != 0) { /* table, key, val */
      if (!lua_isstring(L,-1)) {
         free(items);
         luaL_typerror(L, -1, "string");
         return 0;
      }
      items[i] = lua_tostring(L, -1);
      lua_pop(L,1);
      i++;
      if (i >= nitems)
         break;
   }

   /* Create OSD. */
   cur_mission->osd = osd_create( title, nitems, items );
   cur_mission->osd_set = 1; /* OSD was explicitly set. */

   /* Free items. */
   free(items);

   return 0;
}


/**
 * @brief Destroys the mission OSD.
 *
 * @luafunc osdDestroy()
 */
static int misn_osdDestroy( lua_State *L )
{
   (void) L;

   if (cur_mission->osd != 0) {
      osd_destroy( cur_mission->osd );
      cur_mission->osd = 0;
   }

   return 0;
}


/**
 * @brief Sets active in mission OSD.
 *
 *    @luaparam n Element of the OSD to make active. 
 * @luafunc osdActive( n )
 */
static int misn_osdActive( lua_State *L )
{
   int n;

   n = luaL_checkint(L,1);

   if (cur_mission->osd != 0)
      osd_active( cur_mission->osd, n );

   return 0;
}

