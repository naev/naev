/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_evt.c
 *
 * @brief Handles the event lua bindings.
 */


#include "nlua_evt.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_system.h"
#include "nlua_hook.h"
#include "log.h"
#include "event.h"
#include "mission.h"
#include "player.h"
#include "npc.h"


/**
 * @brief Event system Lua bindings.
 *
 * An example would be:
 * @code
 * evt.finish() -- finishes the event
 * @endcode
 *
 * @luamod evt
 */


/*
 * libraries
 */
/* evt */
static int evt_npcAdd( lua_State *L );
static int evt_npcRm( lua_State *L );
static int evt_finish( lua_State *L );
static int evt_save( lua_State *L );
static int evt_claim( lua_State *L );
static const luaL_reg evt_methods[] = {
   { "npcAdd", evt_npcAdd },
   { "npcRm", evt_npcRm },
   { "save", evt_save },
   { "finish", evt_finish },
   { "claim", evt_claim },
   {0,0}
}; /**< Mission lua methods. */


/*
 * individual library loading
 */
/**
 * @brief Loads the event lua library.
 *    @param L Lua state.
 */
int nlua_loadEvt( lua_State *L )
{
   luaL_register(L, "evt", evt_methods);
   return 0;
}


/**
 * @brief Sets up the Lua environment to run a function.
 */
lua_State *event_setupLua( Event_t *ev, const char *func )
{
   lua_State *L;

   /* Load event. */
   L = ev->L;

   /* Set up event pointer. */
   lua_pushlightuserdata( L, ev );
   lua_setglobal( L, "__evt" );

   /* Get function. */
   lua_getglobal(L, func );

   return L;
}


/**
 * @brief Runs the Lua for an event.
 */
int event_runLua( Event_t *ev, const char *func )
{
   event_setupLua( ev, func );
   return event_runLuaFunc( ev, func, 0 );
}


/**
 * @brief Gets the current running event from user data.
 */
Event_t *event_getFromLua( lua_State *L )
{
   Event_t *ev;

   lua_getglobal( L, "__evt" );
   ev = (Event_t*) lua_touserdata( L, -1 );
   lua_pop( L, 1 );
   return ev;
}


/**
 * @brief Runs a Lua func with nargs.
 *
 *    @return -1 on error, 1 on misn.finish() call, 2 if event got deleted
 *            and 0 normally.
 */
int event_runLuaFunc( Event_t *ev, const char *func, int nargs )
{
   int ret;
   const char* err;
   lua_State *L;
   int evt_delete;

   /* Comfortability. */
   L = ev->L;

   ret = lua_pcall(L, nargs, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      if ((err==NULL) || (strcmp(err,"__done__")!=0)) {
         WARN("Event '%s' -> '%s': %s",
               event_getData(ev->id), func, (err) ? err : "unknown error");
         ret = -1;
      }
      else
         ret = 1;
      lua_pop(L, 1);
   }

   /* Time to remove the event. */
   lua_getglobal( L, "__evt_delete" );
   evt_delete = lua_toboolean(L,-1);
   lua_pop(L,1);
   if (evt_delete) {
      ret = 2;
      event_remove( ev->id );
   }

   return ret;
}


/**
 * @brief Adds an NPC.
 *
 * @usage npc_id = evt.npcAdd( "my_func", "Mr. Test", "none", "A test." ) -- Creates an NPC.
 *
 *    @luaparam func Name of the function to run when approaching.
 *    @luaparam name Name of the NPC
 *    @luaparam portrait Portrait to use for the NPC (from gfx/portraits/).
 *    @luaparam desc Description assosciated to the NPC.
 *    @luaparam priority Optional priority argument (defaults to 5, highest is 0, lowest is 10).
 *    @luareturn The ID of the NPC to pass to npcRm.
 * @luafunc npcAdd( func, name, portrait, desc, priority )
 */
static int evt_npcAdd( lua_State *L )
{
   unsigned int id;
   int priority;
   const char *func, *name, *gfx, *desc;
   char portrait[PATH_MAX];
   Event_t *cur_event;

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

   cur_event = event_getFromLua(L);

   /* Add npc. */
   id = npc_add_event( cur_event->id, func, name, priority, portrait, desc );

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
 * @usage evt.npcRm( npc_id )
 *
 *    @luaparam id ID of the NPC to remove.
 * @luafunc npcRm( id )
 */
static int evt_npcRm( lua_State *L )
{
   unsigned int id;
   int ret;
   Event_t *cur_event;

   id = luaL_checklong(L, 1);

   cur_event = event_getFromLua(L);
   ret = npc_rm_event( id, cur_event->id );

   if (ret != 0)
      NLUA_ERROR(L, "Invalid NPC ID!");
   return 0;
}


/**
 * @brief Finishes the event.
 *
 *    @luaparam properly If true and the event is unique it marks the event
 *                     as completed.  If false or nil it deletes the event but
 *                     doesn't mark it as completed.
 * @luafunc finish( properly )
 */
static int evt_finish( lua_State *L )
{
   int b;
   Event_t *cur_event;

   b = lua_toboolean(L,1);
   lua_pushboolean( L, b );
   lua_setglobal( L, "__evt_delete" );

   cur_event = event_getFromLua(L);
   if (b && event_isUnique(cur_event->id))
      player_eventFinished( cur_event->data );

   lua_pushstring(L, "__done__");
   lua_error(L); /* shouldn't return */

   return 0;
}


/**
 * @brief Saves an event.
 *
 * @usage evt.save() -- Saves an event, which is by default disabled.
 *
 *    @luaparam enable If true or nil sets the event to save, otherwise tells the event to not save.
 * @luafunc save( enable )
 */
static int evt_save( lua_State *L )
{
   int b;
   Event_t *cur_event;
   if (lua_gettop(L)==0)
      b = 1;
   else
      b = lua_toboolean(L,1);
   cur_event = event_getFromLua(L);
   cur_event->save = b;
   return 0;
}


/**
 * @brief Tries to claim systems.
 *
 * Claiming systems is a way to avoid mission/event collisions preemptively.
 *
 * Note it does not actually claim the systems if it fails to claim. It also
 *  does not work more then once.
 *
 * @usage if not evt.claim( { system.get("Gamma Polaris") } ) then evt.finish( false ) end
 * @usage if not evt.claim( system.get("Gamma Polaris") ) then evt.finish( false ) end
 *
 *    @luaparam systems Table of systems to claim or a single system.
 *    @luareturn true if was able to claim, false otherwise.
 * @luafunc claim( systems )
 */
static int evt_claim( lua_State *L )
{
   LuaSystem *ls;
   SysClaim_t *claim;
   Event_t *cur_event;

   /* Get current event. */
   cur_event = event_getFromLua(L);

   /* Check to see if already claimed. */
   if (cur_event->claims != NULL) {
      NLUA_ERROR(L, "Event trying to claim but already has.");
      return 0;
   }

   /* Create the claim. */
   claim = claim_create();

   /* Handle parameters. */
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
   cur_event->claims = claim;
   claim_activate( claim );
   lua_pushboolean(L,1);
   return 1;
}

