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
 * current event
 */
static Event_t *cur_event = NULL; /**< Contains the current event for a running script. */
static int evt_delete = 0; /**< if 1 delete current event */


/*
 * libraries
 */
/* evt */
static int evt_misnStart( lua_State *L );
static int evt_npcAdd( lua_State *L );
static int evt_npcRm( lua_State *L );
static int evt_finish( lua_State *L );
static int evt_save( lua_State *L );
static const luaL_reg evt_methods[] = {
   { "misnStart", evt_misnStart },
   { "npcAdd", evt_npcAdd },
   { "npcRm", evt_npcRm },
   { "save", evt_save },
   { "finish", evt_finish },
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
   cur_event = ev;
   evt_delete = 0;
   nlua_hookTarget( NULL, cur_event );

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

   /* Comfortability. */
   L = ev->L;

   ret = lua_pcall(L, nargs, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      if ((err==NULL) || (strcmp(err,"Event Done")!=0)) {
         WARN("Event '%s' -> '%s': %s",
               event_getData(ev->id), func, (err) ? err : "unknown error");
         ret = -1;
      }
      else
         ret = 1;
      lua_pop(L, 1);
   }

   /* Time to remove the event. */
   if (evt_delete) {
      ret = 2;
      event_remove( cur_event->id );
   }

   /* Unload event. */
   cur_event = NULL;
   nlua_hookTarget( NULL, NULL );

   return ret;
}


/**
 * @brief Starts a mission.
 *
 * @usage evt.misnStart( "Tutorial" ) -- Starts the tutorial
 *
 *    @luaparam misn Name of the mission to start, should match mission in dat/mission.xml.
 * @luafunc misnStart( misn )
 */
static int evt_misnStart( lua_State *L )
{
   const char *str;

   str = luaL_checkstring(L, 1);
   if (mission_start( str )) {
      /* Reset the hook. */
      nlua_hookTarget( NULL, cur_event );
      NLUA_ERROR(L,"Failed to start mission.");
   }

   /* Has to reset the hook target since mission overrides. */
   nlua_hookTarget( NULL, cur_event );

   return 0;
}


/**
 * @brief Adds an NPC.
 *
 * @usage npc_id = evt.npcAdd( "my_func", "Mr. Test", "none", "A test." ) -- Creates an NPC.
 *
 *    @luaparam func Name of the function to run when approaching.
 *    @luaparam name Name of the NPC
 *    @luaparam portrait Portrait to use for the NPC (from gfx/portraits*.png).
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

   id = luaL_checklong(L, 1);
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

   b = lua_toboolean(L,1);
   evt_delete = 1;

   if (b && event_isUnique(cur_event->id))
      player_eventFinished( cur_event->data );

   lua_pushstring(L, "Event Done");
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
   if (lua_gettop(L)==0)
      b = 1;
   else
      b = lua_toboolean(L,1);
   cur_event->save = b;
   return 0;
}
