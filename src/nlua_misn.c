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
#include "nlua_tex.h"
#include "nlua_camera.h"
#include "nlua_music.h"
#include "nlua_bkg.h"
#include "nlua_tut.h"
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
#include "npc.h"
#include "array.h"


/**
 * @brief Mission Lua bindings.
 *
 * An example would be:
 * @code
 * misn.setNPC( "Keer", "keer" )
 * misn.setDesc( "You see here Commodore Keer." )
 * @endcode
 *
 * @luamod misn
 */


/*
 * prototypes
 */
/* static */
static void misn_setEnv( lua_State *L, Mission *misn );


/*
 * libraries
 */
/* Mission methods */
static int misn_setTitle( lua_State *L );
static int misn_setDesc( lua_State *L );
static int misn_setReward( lua_State *L );
static int misn_setNPC( lua_State *L );
static int misn_factions( lua_State *L );
static int misn_accept( lua_State *L );
static int misn_finish( lua_State *L );
static int misn_markerAdd( lua_State *L );
static int misn_markerMove( lua_State *L );
static int misn_markerRm( lua_State *L );
static int misn_cargoAdd( lua_State *L );
static int misn_cargoRm( lua_State *L );
static int misn_cargoJet( lua_State *L );
static int misn_osdCreate( lua_State *L );
static int misn_osdDestroy( lua_State *L );
static int misn_osdActive( lua_State *L );
static int misn_npcAdd( lua_State *L );
static int misn_npcRm( lua_State *L );
static int misn_claim( lua_State *L );
static const luaL_reg misn_methods[] = {
   { "setTitle", misn_setTitle },
   { "setDesc", misn_setDesc },
   { "setReward", misn_setReward },
   { "setNPC", misn_setNPC },
   { "factions", misn_factions },
   { "accept", misn_accept },
   { "finish", misn_finish },
   { "markerAdd", misn_markerAdd },
   { "markerMove", misn_markerMove },
   { "markerRm", misn_markerRm },
   { "cargoAdd", misn_cargoAdd },
   { "cargoRm", misn_cargoRm },
   { "cargoJet", misn_cargoJet },
   { "osdCreate", misn_osdCreate },
   { "osdDestroy", misn_osdDestroy },
   { "osdActive", misn_osdActive },
   { "npcAdd", misn_npcAdd },
   { "npcRm", misn_npcRm },
   { "claim", misn_claim },
   {0,0}
}; /**< Mission Lua methods. */


/**
 * @brief Registers all the mission libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int misn_loadLibs( lua_State *L )
{
   nlua_loadStandard(L,0);
   nlua_loadMisn(L);
   nlua_loadTk(L);
   nlua_loadHook(L);
   nlua_loadMusic(L,0);
   nlua_loadTex(L,0);
   nlua_loadBackground(L,0);
   nlua_loadCamera(L,0);
   if (player_isTut())
      nlua_loadTut(L);
   return 0;
}
/*
 * individual library loading
 */
/**
 * @brief Loads the mission lua library.
 *    @param L Lua state.
 */
int nlua_loadMisn( lua_State *L )
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
   misn_runStart( misn, func );
   if (lua_isnil( misn->L, -1 )) {
      lua_pop(misn->L,1);
      return 0;
   }
   return misn_runFunc( misn, func, 0 );
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
   misn_runStart( misn, func );
   return misn_runFunc( misn, func, 0 );
}


/**
 * @brief Sets the mission environment.
 */
static void misn_setEnv( lua_State *L, Mission *misn )
{
   lua_pushlightuserdata( L, misn );
   lua_setglobal( L, "__misn" );
}


/**
 * @brief Gets the mission that's being currently run in Lua.
 */
Mission* misn_getFromLua( lua_State *L )
{
   Mission *misn;

   lua_getglobal( L, "__misn" );
   misn = (Mission*) lua_touserdata( L, -1 );
   lua_pop( L, 1 );

   return misn;
}


/**
 * @brief Sets up the mission to run misn_runFunc.
 */
lua_State *misn_runStart( Mission *misn, const char *func )
{
   lua_State *L;

   L = misn->L;

   /* Set environment. */
   misn_setEnv( L, misn );

   /* Set the Lua state. */
   lua_getglobal( L, func );

   return L;
}


/**
 * @brief Runs a mission set up with misn_runStart.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted
 *            and 0 normally.
 */
int misn_runFunc( Mission *misn, const char *func, int nargs )
{
   int i, ret;
   const char* err;
   lua_State *L;
   int misn_delete;
   Mission *cur_mission;

   /* For comfort. */
   L = misn->L;

   ret = lua_pcall(L, nargs, 0, 0);
   cur_mission = misn_getFromLua(L); /* The mission can change if accepted. */
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      if ((err==NULL) || (strcmp(err,"__done__")!=0)) {
         WARN("Mission '%s' -> '%s': %s",
               cur_mission->data->name, func, (err) ? err : "unknown error");
         ret = -1;
      }
      else
         ret = 1;
      lua_pop(L,1);
   }

   /* Get delete. */
   lua_getglobal(L,"__misn_delete");
   misn_delete = lua_toboolean(L,-1);
   lua_pop(L,1);

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

   return ret;
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
   Mission *cur_mission;

   str = luaL_checkstring(L,1);

   cur_mission = misn_getFromLua(L);
   if (cur_mission->title) /* cleanup old title */
      free(cur_mission->title);
   cur_mission->title = strdup(str);

   return 0;
}
/**
 * @brief Sets the current mission description.
 *
 * Also sets the mission OSD unless you explicitly force an OSD, however you
 *  can't specify bullet points or other fancy things like with the real OSD.
 *
 *    @luaparam desc Description to use for mission.
 * @luafunc setDesc( desc )
 */
static int misn_setDesc( lua_State *L )
{
   const char *str;
   Mission *cur_mission;

   str = luaL_checkstring(L,1);

   cur_mission = misn_getFromLua(L);
   if (cur_mission->desc) /* cleanup old description */
      free(cur_mission->desc);
   cur_mission->desc = strdup(str);

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
   Mission *cur_mission;

   str = luaL_checkstring(L,1);

   cur_mission = misn_getFromLua(L);
   if (cur_mission->reward) /* cleanup old reward */
      free(cur_mission->reward);
   cur_mission->reward = strdup(str);
   return 0;
}

/**
 * @brief Adds a new marker.
 *
 * @usage my_marker = misn.markerAdd( system.get("Gamma Polaris"), "low" )
 *
 * Valid marker types are:<br/>
 *  - "plot": Important plot marker.<br/>
 *  - "high": High importance mission marker (lower than plot).<br/>
 *  - "low": Low importance mission marker (lower than high).<br/>
 *  - "computer": Mission computer marker.<br/>
 *
 *    @luaparam sys System to mark.
 *    @luaparam type Colouring scheme to use.
 *    @luareturn A marker ID to be used with markerMove and markerRm.
 * @luafunc markerAdd( sys, type )
 */
static int misn_markerAdd( lua_State *L )
{
   int id;
   LuaSystem *sys;
   const char *stype;
   SysMarker type;
   Mission *cur_mission;

   /* Check parameters. */
   sys   = luaL_checksystem( L, 1 );
   stype = luaL_checkstring( L, 2 );

   /* Handle types. */
   if (strcmp(stype, "computer")==0)
      type = SYSMARKER_COMPUTER;
   else if (strcmp(stype, "low")==0)
      type = SYSMARKER_LOW;
   else if (strcmp(stype, "high")==0)
      type = SYSMARKER_HIGH;
   else if (strcmp(stype, "plot")==0)
      type = SYSMARKER_PLOT;
   else {
      NLUA_ERROR(L, "Unknown marker type: %s", stype);
      return 0;
   }

   cur_mission = misn_getFromLua(L);

   /* Add the marker. */
   id = mission_addMarker( cur_mission, -1, sys->id, type );

   /* Update system markers. */
   mission_sysMark();

   /* Return the ID. */
   lua_pushnumber( L, id );
   return 1;
}

/**
 * @brief Moves a marker to a new system.
 *
 * @usage misn.markerMove( my_marker, system.get("Delta Pavonis") )
 *
 *    @luaparam id ID of the mission marker to move.
 *    @luaparam sys System to move the marker to.
 * @luafunc markerMove( id, sys )
 */
static int misn_markerMove( lua_State *L )
{
   int id;
   LuaSystem *sys;
   MissionMarker *marker;
   int i, n;
   Mission *cur_mission;

   /* Handle parameters. */
   id    = luaL_checkinteger( L, 1 );
   sys   = luaL_checksystem( L, 2 );

   cur_mission = misn_getFromLua(L);

   /* Mission must have markers. */
   if (cur_mission->markers == NULL) {
      NLUA_ERROR( L, "Mission has no markers set!" );
      return 0;
   }

   /* Check id. */
   marker = NULL;
   n = array_size( cur_mission->markers );
   for (i=0; i<n; i++) {
      if (id == cur_mission->markers[i].id) {
         marker = &cur_mission->markers[i];
         break;
      }
   }
   if (marker == NULL) {
      NLUA_ERROR( L, "Mission does not have a marker with id '%d'", id );
      return 0;
   }

   /* Update system. */
   marker->sys = sys->id;

   /* Update system markers. */
   mission_sysMark();
   return 0;
}

/**
 * @brief Removes a mission system marker.
 *
 * @usage misn.markerRm( my_marker )
 *
 *    @luaparam id ID of the marker to remove.
 * @luafunc markerRm( id )
 */
static int misn_markerRm( lua_State *L )
{
   int id;
   int i, n;
   MissionMarker *marker;
   Mission *cur_mission;

   /* Handle parameters. */
   id    = luaL_checkinteger( L, 1 );

   cur_mission = misn_getFromLua(L);

   /* Mission must have markers. */
   if (cur_mission->markers == NULL) {
      /* Already removed. */
      return 0;
   }

   /* Check id. */
   marker = NULL;
   n = array_size( cur_mission->markers );
   for (i=0; i<n; i++) {
      if (id == cur_mission->markers[i].id) {
         marker = &cur_mission->markers[i];
         break;
      }
   }
   if (marker == NULL) {
      /* Already removed. */
      return 0;
   }

   /* Remove the marker. */
   array_erase( &cur_mission->markers, marker, &marker[1] );

   /* Update system markers. */
   mission_sysMark();
   return 0;
}


/**
 * @brief Sets the current mission NPC.
 *
 * This is used in bar missions where you talk to a person. The portraits are
 *  the ones found in gfx/portraits without the png extension. So for
 *  gfx/portraits/none.png you would just use "none".
 *
 * @usage misn.setNPC( "Invisible Man", "none" )
 *
 *    @luaparam name Name of the NPC.
 *    @luaparam portrait Name of the portrait to use for the NPC.
 * @luafunc setNPC( name, portrait )
 */
static int misn_setNPC( lua_State *L )
{
   char buf[PATH_MAX];
   const char *name, *str;
   Mission *cur_mission;

   cur_mission = misn_getFromLua(L);

   /* Free if portrait is already set. */
   if (cur_mission->portrait != NULL) {
      gl_freeTexture(cur_mission->portrait);
      cur_mission->portrait = NULL;
   }

   /* Free NPC name. */
   if (cur_mission->npc != NULL) {
      free(cur_mission->npc);
      cur_mission->npc = NULL;
   }

   /* For no parameters just leave having freed NPC. */
   if (lua_gettop(L) == 0)
      return 0;

   /* Get parameters. */
   name = luaL_checkstring(L,1);
   str  = luaL_checkstring(L,2);

   /* Set NPC name. */
   cur_mission->npc = strdup(name);

   /* Set portrait. */
   snprintf( buf, PATH_MAX, "gfx/portraits/%s.png", str );
   cur_mission->portrait = gl_newImage( buf, 0 );

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
   Mission *cur_mission;

   cur_mission = misn_getFromLua(L);
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
   Mission *cur_mission;

   ret = 0;

   /* find last mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data == NULL)
         break;

   cur_mission = misn_getFromLua(L);

   /* no missions left */
   if (i>=MISSION_MAX)
      ret = 1;
   else { /* copy it over */
      memcpy( &player_missions[i], cur_mission, sizeof(Mission) );
      memset( cur_mission, 0, sizeof(Mission) );
      cur_mission = &player_missions[i];
      cur_mission->accepted = 1; /* Mark as accepted. */

      /* Need to change pointer. */
      lua_pushlightuserdata(L,cur_mission);
      lua_setglobal(L,"__misn");
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
 *                     passed it just ends the mission (without removing it
 *                     from the player's list of active missions).
 * @luafunc finish( properly )
 */
static int misn_finish( lua_State *L )
{
   int b;
   Mission *cur_mission;

   if (lua_isboolean(L,1))
      b = lua_toboolean(L,1);
   else {
      lua_pushstring(L, "__done__");
      lua_error(L); /* THERE IS NO RETURN */
      return 0;
   }

   lua_pushboolean( L, 1 );
   lua_setglobal( L, "__misn_delete" );
   cur_mission = misn_getFromLua(L);

   if (b && mis_isFlag(cur_mission->data,MISSION_UNIQUE))
      player_missionFinished( mission_getID( cur_mission->data->name ) );

   lua_pushstring(L, "__done__");
   lua_error(L); /* shouldn't return */

   return 0;
}


/**
 * @brief Adds some mission cargo to the player.  He cannot sell it nor get rid of it
 *  unless he abandons the mission in which case it'll get eliminated.
 *
 *    @luaparam cargo Name of the cargo to add.
 *    @luaparam quantity Quantity of cargo to add.
 *    @luareturn The id of the cargo which can be used in cargoRm.
 * @luafunc cargoAdd( cargo, quantity )
 */
static int misn_cargoAdd( lua_State *L )
{
   const char *cname;
   Commodity *cargo;
   int quantity, ret;
   Mission *cur_mission;

   /* Parameters. */
   cname    = luaL_checkstring(L,1);
   quantity = luaL_checkint(L,2);
   cargo = commodity_get( cname );

   /* Check if the cargo exists. */
   if(cargo == NULL) {
      NLUA_ERROR(L, "Cargo '%s' not found.", cname);
      return 0;
   }

   cur_mission = misn_getFromLua(L);

   /* First try to add the cargo. */
   ret = pilot_addMissionCargo( player.p, cargo, quantity );
   mission_linkCargo( cur_mission, ret );

   lua_pushnumber(L, ret);
   return 1;
}
/**
 * @brief Removes the mission cargo.
 *
 *    @luaparam cargoid Identifier of the mission cargo.
 *    @luareturn true on success.
 * @luafunc cargoRm( cargoid )
 */
static int misn_cargoRm( lua_State *L )
{
   int ret;
   unsigned int id;
   Mission *cur_mission;

   id = luaL_checklong(L,1);

   /* First try to remove the cargo from player. */
   if (pilot_rmMissionCargo( player.p, id, 0 ) != 0) {
      lua_pushboolean(L,0);
      return 1;
   }

   cur_mission = misn_getFromLua(L);

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
 * @luafunc cargoJet( cargoid )
 */
static int misn_cargoJet( lua_State *L )
{
   int ret;
   unsigned int id;
   Mission *cur_mission;

   id = luaL_checklong(L,1);

   /* First try to remove the cargo from player. */
   if (pilot_rmMissionCargo( player.p, id, 1 ) != 0) {
      lua_pushboolean(L,0);
      return 1;
   }

   cur_mission = misn_getFromLua(L);

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
 *    @luaparam title Title to give the OSD.
 *    @luaparam list List of elements to put in the OSD.
 * @luafunc osdCreate( title, list )
 */
static int misn_osdCreate( lua_State *L )
{
   const char *title;
   int nitems;
   char **items;
   int i;
   Mission *cur_mission;

   cur_mission = misn_getFromLua(L);

   /* Must be accepted. */
   if (!cur_mission->accepted) {
      WARN("Can't create an OSD on an unaccepted mission!");
      return 0;
   }

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
      items[i] = strdup( lua_tostring(L, -1) );
      lua_pop(L,1);
      i++;
      if (i >= nitems)
         break;
   }

   /* Create OSD. */
   cur_mission->osd = osd_create( title, nitems, (const char**) items,
         cur_mission->data->avail.priority );
   cur_mission->osd_set = 1; /* OSD was explicitly set. */

   /* Free items. */
   for (i=0; i<nitems; i++)
      free(items[i]);
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
   Mission *cur_mission;
   cur_mission = misn_getFromLua(L);

   if (cur_mission->osd != 0) {
      osd_destroy( cur_mission->osd );
      cur_mission->osd = 0;
   }

   return 0;
}


/**
 * @brief Sets active in mission OSD.
 *
 * @note Uses Lua indexes, so 1 is first member, 2 is second and so on.
 *
 *    @luaparam n Element of the OSD to make active.
 * @luafunc osdActive( n )
 */
static int misn_osdActive( lua_State *L )
{
   int n;
   Mission *cur_mission;

   n = luaL_checkint(L,1);
   n = n-1; /* Convert to C index. */

   cur_mission = misn_getFromLua(L);

   if (cur_mission->osd != 0)
      osd_active( cur_mission->osd, n );

   return 0;
}


/**
 * @brief Adds an NPC.
 *
 * @note Do not use this at all in the "create" function. Use setNPC, setDesc and the "accept" function instead.
 *
 * @usage npc_id = misn.npcAdd( "my_func", "Mr. Test", "none", "A test." ) -- Creates an NPC.
 *
 *    @luaparam func Name of the function to run when approaching.
 *    @luaparam name Name of the NPC
 *    @luaparam portrait Portrait to use for the NPC (from gfx/portraits*.png).
 *    @luaparam desc Description assosciated to the NPC.
 *    @luaparam priority Optional priority argument (defaults to 5, highest is 0, lowest is 10).
 *    @luareturn The ID of the NPC to pass to npcRm.
 * @luafunc npcAdd( func, name, portrait, desc, priority )
 */
static int misn_npcAdd( lua_State *L )
{
   unsigned int id;
   int priority;
   const char *func, *name, *gfx, *desc;
   char portrait[PATH_MAX];
   Mission *cur_mission;

   /* Handle parameters. */
   func = luaL_checkstring(L, 1);
   name = luaL_checkstring(L, 2);
   gfx  = luaL_checkstring(L, 3);
   desc = luaL_checkstring(L, 4);

   /* Optional priority. */
   if (lua_gettop(L) > 4)
      priority = luaL_checkint( L, 5 );
   else
      priority = 5;

   /* Set path. */
   snprintf( portrait, PATH_MAX, "gfx/portraits/%s.png", gfx );

   cur_mission = misn_getFromLua(L);

   /* Add npc. */
   id = npc_add_mission( cur_mission, func, name, priority, portrait, desc );

   /* Return ID. */
   if (id > 0) {
      lua_pushnumber( L, id );
      return 1;
   }
   return 0;
}


/**
 * @brief Removes an NPC.
 *
 * @usage misn.npcRm( npc_id )
 *
 *    @luaparam id ID of the NPC to remove.
 * @luafunc npcRm( id )
 */
static int misn_npcRm( lua_State *L )
{
   unsigned int id;
   int ret;
   Mission *cur_mission;

   id = luaL_checklong(L, 1);
   cur_mission = misn_getFromLua(L);
   ret = npc_rm_mission( id, cur_mission );

   if (ret != 0)
      NLUA_ERROR(L, "Invalid NPC ID!");
   return 0;
}


/**
 * @brief Tries to claim systems.
 *
 * Claiming systems is a way to avoid mission collisions preemptively.
 *
 * Note it does not actually claim the systems if it fails to claim. It also
 *  does not work more then once.
 *
 * @usage if not misn.claim( { system.get("Gamma Polaris") } ) then misn.finish( false ) end
 * @usage if not misn.claim( system.get("Gamma Polaris") ) then misn.finish( false ) end
 *
 *    @luaparam systems Table of systems to claim or a single system.
 *    @luareturn true if was able to claim, false otherwise.
 * @luafunc claim( systems )
 */
static int misn_claim( lua_State *L )
{
   LuaSystem *ls;
   SysClaim_t *claim;
   Mission *cur_mission;

   /* Get mission. */
   cur_mission = misn_getFromLua(L);

   /* Check to see if already claimed. */
   if (cur_mission->claims != NULL) {
      NLUA_ERROR(L, "Mission trying to claim but already has.");
      return 0;
   }

   /* Create the claim. */
   claim = claim_create();

   if (lua_istable(L,1)) {
      /* Iterate over table. */
      lua_pushnil(L);
      while (lua_next(L, 1) != 0) {
         if (!lua_issystem(L,-1)) {
            claim_destroy( claim );
            NLUA_ERROR(L,"Claim table should contain only systems!");
            return 0;
         }
         else {
            ls = lua_tosystem( L, -1 );
            claim_add( claim, ls->id );
         }
         lua_pop(L,1);
      }
   }
   else if (lua_issystem(L, 1)) {
      ls = lua_tosystem( L, 1 );
      claim_add( claim, ls->id );
   }
   else
      NLUA_INVALID_PARAMETER(L);

   /* Test claim. */
   if (claim_test( claim )) {
      claim_destroy( claim );
      lua_pushboolean(L,0);
      return 1;
   }

   /* Set the claim. */
   cur_mission->claims = claim;
   claim_activate( claim );
   lua_pushboolean(L,1);
   return 1;
}



