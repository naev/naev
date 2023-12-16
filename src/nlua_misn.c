/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_misn.c
 *
 * @brief Handles the mission Lua bindings.
 */
/** @cond */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_misn.h"

#include "array.h"
#include "gui_osd.h"
#include "land.h"
#include "log.h"
#include "mission.h"
#include "music.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_audio.h"
#include "nlua_bkg.h"
#include "nlua_camera.h"
#include "nlua_commodity.h"
#include "nlua_faction.h"
#include "nlua_hook.h"
#include "nlua_music.h"
#include "nlua_spob.h"
#include "nlua_player.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_tk.h"
#include "nluadef.h"
#include "npc.h"
#include "nstring.h"
#include "nxml.h"
#include "player.h"
#include "rng.h"
#include "shiplog.h"
#include "toolkit.h"

/**
 * @brief Mission Lua bindings.
 *
 * An example would be:
 * @code
 * misn.setNPC( "Keer", "empire/unique/keer.webp", _("You see here Commodore Keer.") )
 * @endcode
 *
 * @luamod misn
 */

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
static int misn_osdGet( lua_State *L );
static int misn_osdGetActiveItem( lua_State *L );
static int misn_npcAdd( lua_State *L );
static int misn_npcRm( lua_State *L );
static int misn_claim( lua_State *L );
static const luaL_Reg misn_methods[] = {
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
   { "osdGet", misn_osdGet },
   { "osdGetActive", misn_osdGetActiveItem },
   { "npcAdd", misn_npcAdd },
   { "npcRm", misn_npcRm },
   { "claim", misn_claim },
   {0,0}
}; /**< Mission Lua methods. */

/**
 * @brief Registers all the mission libraries.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int misn_loadLibs( nlua_env env )
{
   nlua_loadStandard(env);
   nlua_loadMisn(env);
   nlua_loadHook(env);
   nlua_loadCamera(env);
   nlua_loadTex(env);
   nlua_loadBackground(env);
   nlua_loadMusic(env);
   nlua_loadTk(env);
   return 0;
}
/*
 * individual library loading
 */
/**
 * @brief Loads the mission Lua library.
 *    @param env Lua environment.
 */
int nlua_loadMisn( nlua_env env )
{
   nlua_register(env, "misn", misn_methods, 0);
   return 0;
}

/**
 * @brief Tries to run a mission, but doesn't err if it fails.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted,
 *          3 if the mission got accepted, and 0 normally.
 */
int misn_tryRun( Mission *misn, const char *func )
{
   int ret;
   /* Get the function to run. */
   misn_runStart( misn, func );
   if (lua_isnil( naevL, -1 )) {
      lua_pop(naevL,1);
      return 0;
   }
   ret = misn_runFunc( misn, func, 0 );
   return ret;
}

/**
 * @brief Runs a mission function.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted,
 *          3 if the mission got accepted, and 0 normally.
 */
int misn_run( Mission *misn, const char *func )
{
   misn_runStart( misn, func );
   return misn_runFunc( misn, func, 0 );
}

/**
 * @brief Gets the mission that's being currently run in Lua.
 *
 * This should ONLY be called below an nlua_pcall, so __NLUA_CURENV is set
 */
Mission* misn_getFromLua( lua_State *L )
{
   Mission *misn, **misnptr;

   nlua_getenv( L, __NLUA_CURENV, "__misn" );
   misnptr = lua_touserdata( L, -1 );
   misn = misnptr ? *misnptr : NULL;
   lua_pop( L, 1 );

   return misn;
}

/**
 * @brief Sets up the mission to run misn_runFunc.
 */
void misn_runStart( Mission *misn, const char *func )
{
   Mission **misnptr;
   misnptr = lua_newuserdata( naevL, sizeof(Mission*) );
   *misnptr = misn;
   nlua_setenv( naevL, misn->env, "__misn" );

   /* Set the Lua state. */
   nlua_getenv( naevL, misn->env, func );
}

/**
 * @brief Runs a mission set up with misn_runStart.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @param nargs Number of arguments to pass.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted,
 *          3 if the mission got accepted, and 0 normally.
 */
int misn_runFunc( const Mission *misn, const char *func, int nargs )
{
   int ret, misn_delete;
   Mission *cur_mission;
   nlua_env env;

   /* Check to see if it is accepted first. */
   int isaccepted = misn->accepted;

   /* Set up and run function. */
   env = misn->env;
   ret = nlua_pcall(env, nargs, 0);

   /* The mission can change if accepted. */
   nlua_getenv(naevL, env, "__misn");
   cur_mission = *(Mission**) lua_touserdata(naevL, -1);
   lua_pop(naevL, 1);

   if (ret != 0) { /* error has occurred */
      const char* err = (lua_isstring(naevL,-1)) ? lua_tostring(naevL,-1) : NULL;
      if ((err==NULL) || (strcmp(err,NLUA_DONE)!=0)) {
         WARN(_("Mission '%s' -> '%s': %s"),
               cur_mission->data->name, func, (err) ? err : _("unknown error"));
         ret = -1;
      }
      else
         ret = 1;
      lua_pop(naevL,1);
   }

   /* Get delete. */
   nlua_getenv(naevL, env, "__misn_delete");
   misn_delete = lua_toboolean(naevL,-1);
   lua_pop(naevL,1);

   /* Mission is finished */
   if (misn_delete) {
      ret = 2;
      mission_cleanup( cur_mission );
      for (int i=0; i<array_size(player_missions); i++) {
         if (cur_mission != player_missions[i])
            continue;

         mission_shift(i);
         break;
      }
   }
   /* Mission became accepted. */
   else if (!isaccepted && cur_mission->accepted)
      ret = 3;

   return ret;
}

/**
 * @brief Sets the current mission title.
 *
 *    @luatparam string title Title to use for mission.
 * @luafunc setTitle
 */
static int misn_setTitle( lua_State *L )
{
   const char *str = luaL_checkstring(L,1);
   Mission *cur_mission = misn_getFromLua(L);
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
 *    @luatparam string desc Description to use for mission.
 * @luafunc setDesc
 */
static int misn_setDesc( lua_State *L )
{
   const char *str = luaL_checkstring(L,1);
   Mission *cur_mission = misn_getFromLua(L);
   free(cur_mission->desc);
   cur_mission->desc = strdup(str);
   return 0;
}
/**
 * @brief Sets the current mission reward description.
 *
 *    @luatparam string|number reward Description of the reward to use. Can pass a number to signify a monetary reward, and allow for sorting.
 * @luafunc setReward
 */
static int misn_setReward( lua_State *L )
{
   Mission *cur_mission = misn_getFromLua(L);
   free(cur_mission->reward);
   cur_mission->reward_value = -1.;
   if (lua_isnumber(L,1)) {
      char buf[ECON_CRED_STRLEN];
      cur_mission->reward_value = CLAMP( CREDITS_MIN, CREDITS_MAX, (credits_t)round(luaL_checknumber(L,1)) );
      credits2str( buf, cur_mission->reward_value, 2 );
      cur_mission->reward = strdup(buf);
   }
   else {
      const char *str = luaL_checkstring(L,1);
      cur_mission->reward = strdup(str);
   }
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
 *    @luatparam System|Spob target System or spob to mark.
 *    @luatparam[opt="high"] string type Colouring scheme to use.
 *    @luatreturn number A marker ID to be used with markerMove and markerRm.
 * @luafunc markerAdd
 */
static int misn_markerAdd( lua_State *L )
{
   int id, issys, objid;
   const char *stype;
   MissionMarkerType type;
   Mission *cur_mission;

   /* Check parameters. */
   if (lua_isspob(L,1)) {
      issys = 0;
      objid = luaL_checkspob(L,1);
   }
   else {
      issys = 1;
      objid = system_index( luaL_validsystem(L,1) );
   }
   stype = luaL_optstring( L, 2, "high" );

   /* Handle types. */
   if (strcmp(stype, "computer")==0)
      type = SPOBMARKER_COMPUTER;
   else if (strcmp(stype, "low")==0)
      type = SPOBMARKER_LOW;
   else if (strcmp(stype, "high")==0)
      type = SPOBMARKER_HIGH;
   else if (strcmp(stype, "plot")==0)
      type = SPOBMARKER_PLOT;
   else
      return NLUA_ERROR(L, _("Unknown marker type: %s"), stype);

   /* Convert spob -> system. */
   if (issys)
      type = mission_markerTypeSpobToSystem( type );

   cur_mission = misn_getFromLua(L);

   /* Add the marker. */
   id = mission_addMarker( cur_mission, -1, objid, type );

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
 *    @luatparam number id ID of the mission marker to move.
 *    @luatparam System sys System to move the marker to.
 * @luafunc markerMove
 */
static int misn_markerMove( lua_State *L )
{
   int id, objid, issys;
   MissionMarker *marker;
   Mission *cur_mission;

   /* Handle parameters. */
   id    = luaL_checkinteger( L, 1 );
   if (lua_isspob(L,2)) {
      issys = 0;
      objid = luaL_checkspob(L,2);
   }
   else {
      issys = 1;
      objid = luaL_checksystem(L,2);
   }

   cur_mission = misn_getFromLua(L);

   /* Check id. */
   marker = NULL;
   for (int i=0; i<array_size(cur_mission->markers); i++) {
      if (id == cur_mission->markers[i].id) {
         marker = &cur_mission->markers[i];
         break;
      }
   }
   if (marker == NULL)
      return NLUA_ERROR( L, _("Mission does not have a marker with id '%d'"), id );

   /* Update system. */
   if (issys)
      marker->type = mission_markerTypeSpobToSystem( marker->type );
   else
      marker->type = mission_markerTypeSystemToSpob( marker->type );
   marker->objid = objid;

   /* Update system markers. */
   mission_sysMark();
   return 0;
}

/**
 * @brief Removes a mission system marker.
 *
 * @usage misn.markerRm( my_marker )
 *
 *    @luatparam[opt] number id ID of the marker to remove. If no parameter is passed, all markers associated with the mission are removed.
 * @luafunc markerRm
 */
static int misn_markerRm( lua_State *L )
{
   int id;
   MissionMarker *marker;
   Mission *cur_mission = misn_getFromLua(L);

   /* Remove all markers. */
   if (lua_isnoneornil(L,1)) {
      array_erase( &cur_mission->markers, array_begin(cur_mission->markers), array_end(cur_mission->markers) );
      mission_sysMark();
      return 0;
   }

   /* Handle parameters. */
   id    = luaL_checkinteger( L, 1 );

   /* Check id. */
   marker = NULL;
   for (int i=0; i<array_size( cur_mission->markers ); i++) {
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
   array_erase( &cur_mission->markers, &marker[0], &marker[1] );

   /* Update system markers. */
   mission_sysMark();
   return 0;
}

/**
 * @brief Sets the current mission NPC.
 *
 * This is used in bar missions where you talk to a person. The portraits are
 *  the ones found in GFX_PATH/portraits. (For GFX_PATH/portraits/none.webp
 *  you would use "none.webp".)
 *
 * Note that this NPC will disappear when either misn.accept() or misn.finish()
 *  is called.
 *
 * @usage misn.setNPC( "Invisible Man", "none.webp", _("You see a levitating mug drain itself.") )
 *
 *    @luatparam string name Name of the NPC.
 *    @luatparam string portrait File name of the portrait to use for the NPC.
 *    @luatparam string desc Description of the NPC to use.
 * @luafunc setNPC
 */
static int misn_setNPC( lua_State *L )
{
   const char *name, *desc;
   Mission *cur_mission;

   cur_mission = misn_getFromLua(L);

   gl_freeTexture(cur_mission->portrait);
   cur_mission->portrait = NULL;

   free(cur_mission->npc);
   cur_mission->npc = NULL;

   free(cur_mission->npc_desc);
   cur_mission->npc_desc = NULL;

   /* For no parameters just leave having freed NPC. */
   if (lua_gettop(L) == 0)
      return 0;

   /* Get parameters. */
   name = luaL_checkstring(L,1);
   cur_mission->portrait = luaL_validtex(L,2,GFX_PATH"portraits/");
   desc = luaL_checkstring(L,3);

   /* Set NPC name and description. */
   cur_mission->npc = strdup(name);
   cur_mission->npc_desc = strdup(desc);

   return 0;
}

/**
 * @brief Gets the factions the mission is available for.
 *
 * @usage f = misn.factions()
 *    @luatreturn {Faction,...} A table containing the factions for whom the mission is available.
 * @luafunc factions
 */
static int misn_factions( lua_State *L )
{
   Mission *cur_mission = misn_getFromLua(L);
   const MissionData *dat = cur_mission->data;

   /* we'll push all the factions in table form */
   lua_newtable(L);
   for (int i=0; i<array_size(dat->avail.factions); i++) {
      LuaFaction f = dat->avail.factions[i];
      lua_pushfaction(L, f); /* value */
      lua_rawseti(L,-2,i+1); /* store the value in the table */
   }
   return 1;
}
/**
 * @brief Attempts to accept the mission.
 *
 * Note: there is no limit on the maximum number of missions a player can have simultaneously.
 *
 * @usage if not misn.accept() then return end
 *    @luatreturn boolean true if mission was properly accepted.
 * @luafunc accept
 */
static int misn_accept( lua_State *L )
{
   Mission *new_misn, *cur_mission;
   int ret = 0;

   if (player_missions == NULL)
      player_missions = array_create( Mission* );

   /* Clean up old stale stuff if necessary. */
   for (int i=array_size(player_missions)-1; i>=0; i--) {
      Mission *m = player_missions[i];
      if (m->id != 0)
         continue;
      mission_cleanup( m );
      free( m );
      array_erase( &player_missions, &player_missions[i], &player_missions[i+1] );
   }

   /* Create the new mission. */
   new_misn = calloc( 1, sizeof(Mission) );
   array_push_back( &player_missions, new_misn );

   cur_mission = misn_getFromLua(L);

   /* no missions left */
   if (cur_mission->accepted)
      return NLUA_ERROR(L, _("Mission already accepted!"));
   else { /* copy it over */
      Mission **misnptr;
      *new_misn = *cur_mission;
      memset( cur_mission, 0, sizeof(Mission) );
      cur_mission->env = LUA_NOREF;
      cur_mission->accepted = 1; /* Propagated to the mission computer. */
      cur_mission = new_misn;
      cur_mission->accepted = 1; /* Mark as accepted. */

      /* Need to change pointer. */
      misnptr = lua_newuserdata( L, sizeof(Mission*) );
      *misnptr = cur_mission;
      nlua_setenv( L, cur_mission->env, "__misn" );
   }

   lua_pushboolean(L,!ret); /* we'll convert C style return to Lua */
   return 1;
}
/**
 * @brief Finishes the mission.
 *
 *    @luatparam[opt] boolean properly If true and the mission is unique it marks the mission
 *                     as completed.  If false it deletes the mission but
 *                     doesn't mark it as completed.  If the parameter isn't
 *                     passed it just ends the mission (without removing it
 *                     from the player's list of active missions).
 * @luafunc finish
 */
static int misn_finish( lua_State *L )
{
   int b = lua_toboolean(L,1);
   Mission *cur_mission = misn_getFromLua(L);

   lua_pushboolean( L, 1 );
   nlua_setenv( L, cur_mission->env, "__misn_delete" );

   if (b)
      player_missionFinished( mission_getID( cur_mission->data->name ) );

   lua_pushstring(L, NLUA_DONE);
   lua_error(L); /* shouldn't return */

   return 0;
}

/**
 * @brief Adds some mission cargo to the player. They cannot sell it nor get rid of it
 *  unless they abandons the mission in which case it'll get eliminated.
 *
 *    @luatparam Commodity|string cargo Type of cargo to add, either as
 *       a Commodity object or as the raw (untranslated) name of a
 *       commodity.
 *    @luatparam number quantity Quantity of cargo to add.
 *    @luatreturn number The id of the cargo which can be used in cargoRm.
 * @luafunc cargoAdd
 */
static int misn_cargoAdd( lua_State *L )
{
   Commodity *cargo;
   int quantity, ret;
   Mission *cur_mission;

   /* Parameters. */
   cargo    = luaL_validcommodity(L,1);
   quantity = luaL_checkint(L,2);

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
 *    @luatparam number cargoid Identifier of the mission cargo.
 *    @luatreturn boolean true on success.
 * @luafunc cargoRm
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
 *    @luatparam number cargoid ID of the cargo to jettison.
 *    @luatreturn boolean true on success.
 * @luafunc cargoJet
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
 * @note You can index elements by using '\\t' as first character of an element.
 * @note Destroys an osd if it already exists.
 *
 * @usage misn.osdCreate( "My OSD", {"Element 1", "Element 2"})
 *
 *    @luatparam string title Title to give the OSD.
 *    @luatparam {string,...} list List of elements to put in the OSD.
 * @luafunc osdCreate
 */
static int misn_osdCreate( lua_State *L )
{
   const char *title;
   int nitems;
   char **items;
   Mission *cur_mission = misn_getFromLua(L);

   /* Must be accepted. */
   if (!cur_mission->accepted) {
      WARN(_("Can't create an OSD on an unaccepted mission!"));
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
   for (int i=0; i<nitems; i++) {
      lua_pushnumber(L,i+1);
      lua_gettable(L,2);
      if (!lua_isstring(L,-1)) {
         free(items);
         luaL_typerror(L, -1, "string");
         return 0;
      }
      items[i] = strdup( lua_tostring(L, -1) );
      lua_pop(L,1);
   }

   /* Create OSD. */
   cur_mission->osd = osd_create( title, nitems, (const char**) items,
         cur_mission->data->avail.priority );
   cur_mission->osd_set = 1; /* OSD was explicitly set. */

   /* Free items. */
   for (int i=0; i<nitems; i++)
      free(items[i]);
   free(items);

   return 0;
}

/**
 * @brief Destroys the mission OSD.
 *
 * @luafunc osdDestroy
 */
static int misn_osdDestroy( lua_State *L )
{
   Mission *cur_mission = misn_getFromLua(L);

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
 *    @luatparam number n Element of the OSD to make active.
 * @luafunc osdActive
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
 * @brief Gets the active OSD element.
 *
 *    @luatreturn string Th ename of the active element or nil if none.
 * @luafunc osdGetActive
 */
static int misn_osdGetActiveItem( lua_State *L )
{
   const Mission *cur_mission = misn_getFromLua(L);
   char **items = osd_getItems(cur_mission->osd);
   int active   = osd_getActive(cur_mission->osd);

   if (!items || active < 0) {
      lua_pushnil(L);
      return 1;
   }

   lua_pushstring(L, items[active]);
   return 1;
}

/**
 * @brief Gets the current mission OSD information.
 *
 *    @luatparam string Title of the OSD.
 *    @luatparam table List of items in the OSD.
 *    @luatparam number ID of the current active OSD element.
 * @luafunc osdGet
 */
static int misn_osdGet( lua_State *L )
{
   const Mission *cur_mission = misn_getFromLua(L);
   char **items;

   if (cur_mission->osd == 0)
      return 0;

   lua_pushstring( L, osd_getTitle(cur_mission->osd) );
   lua_newtable(L);
   items = osd_getItems(cur_mission->osd);
   for (int i=0; i<array_size(items); i++) {
      lua_pushstring( L, items[i] );
      lua_rawseti( L, -2, i+1 );
   }
   lua_pushinteger( L, osd_getActive(cur_mission->osd) );
   return 3;
}

/**
 * @brief Adds an NPC.
 *
 * @usage npc_id = misn.npcAdd( "my_func", "Mr. Test", "none.webp", "A test." ) -- Creates an NPC.
 *
 *    @luatparam string func Name of the function to run when approaching, gets passed the npc_id when called.
 *    @luatparam string name Name of the NPC
 *    @luatparam string portrait Portrait file name to use for the NPC (from GFX_PATH/portraits/).
 *    @luatparam string desc Description associated to the NPC.
 *    @luatparam[opt=5] number priority Optional priority argument (highest is 0, lowest is 10).
 *    @luatparam[opt=nil] string background Background file name to use (from GFX_PATH/portraits/).
 *    @luatreturn number The ID of the NPC to pass to npcRm.
 * @luafunc npcAdd
 */
static int misn_npcAdd( lua_State *L )
{
   unsigned int id;
   int priority;
   const char *func, *name, *desc;
   glTexture *portrait, *bg;
   Mission *cur_mission;

   /* Handle parameters. */
   func = luaL_checkstring(L, 1);
   name = luaL_checkstring(L, 2);
   portrait = luaL_validtex( L, 3, GFX_PATH"portraits/" );
   desc = luaL_checkstring(L, 4);

   /* Optional parameters. */
   priority = luaL_optinteger(L,5,5);
   if (!lua_isnoneornil(L,6))
      bg = luaL_validtex( L, 6, GFX_PATH"portraits/" );
   else
      bg = NULL;

   cur_mission = misn_getFromLua(L);

   /* Add npc. */
   id = npc_add_mission( cur_mission->id, func, name, priority, portrait, desc, bg );

   /* Regenerate bar. */
   bar_regen();

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
 *    @luatparam number id ID of the NPC to remove.
 * @luafunc npcRm
 */
static int misn_npcRm( lua_State *L )
{
   unsigned int id = luaL_checklong(L, 1);
   const Mission *cur_mission = misn_getFromLua(L);
   int ret = npc_rm_mission( id, cur_mission->id );

   /* Regenerate bar. */
   bar_regen();

   if (ret != 0)
      return NLUA_ERROR(L, _("Invalid NPC ID!"));
   return 0;
}

/**
 * @brief Tries to claim systems or strings.
 *
 * Claiming systems and strings is a way to avoid mission collisions preemptively.
 *
 * Note it does not actually perform the claim if it fails to claim. It also
 *  does not work more than once.
 *
 * @usage if not misn.claim( { system.get("Gamma Polaris") } ) then misn.finish( false ) end
 * @usage if not misn.claim( system.get("Gamma Polaris") ) then misn.finish( false ) end
 * @usage if not misn.claim( 'some_string' ) then misn.finish( false ) end
 * @usage if not misn.claim( { system.get("Gamma Polaris"), 'some_string' } ) then misn.finish( false ) end
 *
 *    @luatparam System|String|{System,String...} params Table of systems/strings to claim or a single system/string.
 *    @luatparam[opt=false] boolean inclusive Whether or not to allow the claim to include other inclusive claims. Multiple missions/events can inclusively claim the same system, but only one system can exclusively claim it.
 *    @luatreturn boolean true if was able to claim, false otherwise.
 * @luafunc claim
 */
static int misn_claim( lua_State *L )
{
   Claim_t *claim;
   Mission *cur_mission;
   int inclusive;

   /* Get mission. */
   cur_mission = misn_getFromLua(L);

   inclusive = lua_toboolean(L,2);

   /* Check to see if already claimed. */
   if (!claim_isNull(cur_mission->claims))
      return NLUA_ERROR(L, _("Mission trying to claim but already has."));

   /* Create the claim. */
   claim = claim_create( !inclusive );

   if (lua_istable(L,1)) {
      /* Iterate over table. */
      lua_pushnil(L);
      while (lua_next(L, 1) != 0) {
         if (lua_issystem(L,-1))
            claim_addSys( claim, lua_tosystem( L, -1 ) );
         else if (lua_isstring(L,-1))
            claim_addStr( claim, lua_tostring( L, -1 ) );
         lua_pop(L,1);
      }
   }
   else if (lua_issystem(L, 1))
      claim_addSys( claim, lua_tosystem( L, 1 ) );
   else if (lua_isstring(L, 1))
      claim_addStr( claim, lua_tostring( L, 1 ) );
   else
      NLUA_INVALID_PARAMETER(L,1);

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

void misn_pushMissionData( lua_State *L, const MissionData *md )
{
   const MissionAvail_t *ma = &md->avail;

   lua_newtable(L);

   lua_pushstring( L, md->name );
   lua_setfield(L,-2,"name");

   lua_pushstring( L, mission_availabilityStr(ma->loc) );
   lua_setfield(L,-2,"loc");

   lua_pushinteger( L, ma->chance );
   lua_setfield(L,-2,"chance");

   if (ma->spob != NULL) {
      lua_pushstring( L, ma->spob );
      lua_setfield(L,-2,"spob");
   }

   if (ma->system != NULL) {
      lua_pushstring( L, ma->system );
      lua_setfield(L,-2,"system");
   }

   if (ma->chapter != NULL) {
      lua_pushstring( L, ma->chapter );
      lua_setfield(L,-2,"chapter");
   }

   /* TODO factions. */

   if (ma->cond != NULL) {
      lua_pushstring( L, ma->cond );
      lua_setfield(L,-2,"cond");
   }

   if (ma->done != NULL) {
      lua_pushstring( L, ma->done );
      lua_setfield(L,-2,"done");
   }

   lua_pushinteger( L, ma->priority );
   lua_setfield(L,-2,"priority");

   if (mis_isFlag(md,MISSION_UNIQUE)) {
      lua_pushboolean( L, 1 );
      lua_setfield(L,-2,"unique");
   }

   lua_newtable(L);
   for (int t=0; t<array_size(md->tags); t++) {
      lua_pushstring( L, md->tags[t] );
      lua_pushboolean( L, 1 );
      lua_rawset( L, -3 );
   }
   lua_setfield(L,-2,"tags");
}
