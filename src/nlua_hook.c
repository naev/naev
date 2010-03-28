/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_hook.c
 *
 * @brief Lua hook module.
 */


#include "nlua_hook.h"

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
#include "hook.h"
#include "log.h"
#include "mission.h"


/*
 * Needed.
 */
static Mission *running_mission = NULL; /**< Current running mission. */
static Event_t *running_event = NULL; /**< Current running event. */


/* hooks */
static int hookL_rm( lua_State *L );
static int hook_land( lua_State *L );
static int hook_takeoff( lua_State *L );
static int hook_time( lua_State *L );
static int hook_jumpout( lua_State *L );
static int hook_jumpin( lua_State *L );
static int hook_enter( lua_State *L );
static int hook_pilot( lua_State *L );
static const luaL_reg hook_methods[] = {
   { "rm", hookL_rm },
   { "land", hook_land },
   { "takeoff", hook_takeoff },
   { "time", hook_time },
   { "jumpout", hook_jumpout },
   { "jumpin", hook_jumpin },
   { "enter", hook_enter },
   { "pilot", hook_pilot },
   {0,0}
}; /**< Hook Lua methods. */


/*
 * Prototypes.
 */
static unsigned int hook_generic( lua_State *L, const char* stack, int pos );


/**
 * @brief Loads the hook lua library.
 *    @param L Lua state.
 *    @return 0 on success.
 */
int nlua_loadHook( lua_State *L )
{
   luaL_register(L, "hook", hook_methods);
   return 0;
}


/**
 * @brief Sets the hook target.
 *
 * The hooks will attach to these targets. Set one to NULL always.
 *
 *    @param m Mission target.
 *    @param ev Event target.
 */
void nlua_hookTarget( Mission *m, Event_t *ev )
{
   running_mission = m;
   running_event   = ev;
}


/**
 * @brief Lua bindings to manipulate hooks.
 *
 * Hooks allow you to trigger functions to certain actions like when the player
 * jumps or a pilot dies.
 *
 * Example usage would be:
 * @code
 * function penter ()
 *    -- Function to run when player enters a system
 * end
 *
 * hookid = hook.enter( "penter" )
 * @endcode
 *
 * @luamod hook
 */


/**
 * @brief Removes a hook previously created.
 *
 * @usage hook.rm( h ) -- Hook is removed
 * 
 *    @luaparam h Identifier of the hook to remove.
 *    @luareturn true if the hook was removed.
 * @luafunc rm( h )
 */
static int hookL_rm( lua_State *L )
{
   unsigned int h;
   int ret;

   h = luaL_checklong( L, 1 );
   ret = hook_rm( h );

   lua_pushboolean( L, ret );
   return 1;
}


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
static unsigned int hook_generic( lua_State *L, const char* stack, int pos )
{
   int i;
   const char *func;

   /* Last parameter must be function to hook */
   func = luaL_checkstring(L,pos);

   if (running_mission != NULL) {
      /* make sure mission is a player mission */
      for (i=0; i<MISSION_MAX; i++)
         if (player_missions[i].id == running_mission->id)
            break;
      if (i>=MISSION_MAX) {
         WARN("Mission not in stack trying to hook");
         return 0;
      }

      return hook_addMisn( running_mission->id, func, stack );
   }
   else if (running_event != NULL) {
      return hook_addEvent( running_event->id, func, stack );
   }

   NLUA_ERROR(L,"No hook target was set.");
   return 0;
}
/**
 * @brief Hooks the function to the player landing.
 *
 * Can also be used to hook the various subparts of the landing menu. Possible targets
 *  for where are:<br />
 *   - "land" - when landed (default with no parameter )<br />
 *   - "outfits" - when visited outfitter<br />
 *   - "shipyard" - when visited shipyard<br />
 *   - "bar" - when visited bar<br />
 *   - "mission" - when visited mission computer<br />
 *   - "commodity" - when visited commodity exchange<br />
 *   - "equipment" - when visiting equipment place<br />
 *
 * @usage hook.land( "my_function" ) -- Land calls my_function
 * @usage hook.land( "my_function", "equipment" ) -- Calls my_function at equipment screen
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam where Optional argument to specify where to hook the function.
 *    @luareturn Hook identifier.
 * @luafunc land( funcname, where )
 */
static int hook_land( lua_State *L )
{
   const char *where;
   unsigned int h;

   if (lua_gettop(L) < 2)
      h = hook_generic( L, "land", 1 );
   else {
      where = luaL_checkstring(L, 2);
      h = hook_generic( L, where, 1 );
   }

   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player taking off.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luareturn Hook identifier.
 * @luafunc takeoff( funcname )
 */
static int hook_takeoff( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "takeoff", 1 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to a time change.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luareturn Hook identifier.
 * @luafunc time( funcname )
 */
static int hook_time( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "time", 1 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player jumping (before changing systems).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luareturn Hook identifier.
 * @luafunc jumpout( funcname )
 */
static int hook_jumpout( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "jumpout", 1 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player jumping (after changing systems).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luareturn Hook identifier.
 * @luafunc jumpin( funcname )
 */
static int hook_jumpin( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "jumpin", 1 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player entering a system (triggers when taking
 *  off too).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luareturn Hook identifier.
 * @luafunc enter( funcname )
 */
static int hook_enter( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "enter", 1 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to a specific pilot.
 *
 * You can hook to different actions.  Curently hook system only supports:<br />
 *    - "death" : triggered when pilot dies.<br />
 *    - "board" : triggered when pilot is boarded.<br />
 *    - "disable" : triggered when pilot is disabled.<br />
 *    - "jump" : triggered when pilot jumps to hyperspace.<br />
 *    - "hail" : triggered when pilot is hailed.<br />
 *    - "land" : triggered when pilot is landing.<br />
 *    - "attacked" : triggered when the pilot is attacked in manual control <br />
 *    - "idle" : triggered when the pilot becomes idle in manual control <br />
 *
 * These hooks are run right after the action that triggers them happens, so when the death
 *  or jump hook is run, the pilot won't be in the system at that time.
 *
 *    @luaparam pilot Pilot identifier to hook.
 *    @luaparam type One of the supported hook types.
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luareturn Hook identifier.
 * @luafunc pilot( pilot, type, funcname )
 */
static int hook_pilot( lua_State *L )
{
   unsigned int h;
   LuaPilot *p;
   int type;
   const char *hook_type;

   /* Parameters. */
   p           = luaL_checkpilot(L,1);
   hook_type   = luaL_checkstring(L,2);

   /* Check to see if hook_type is valid */
   if (strcmp(hook_type,"death")==0)         type = PILOT_HOOK_DEATH;
   else if (strcmp(hook_type,"board")==0)    type = PILOT_HOOK_BOARD;
   else if (strcmp(hook_type,"disable")==0)  type = PILOT_HOOK_DISABLE;
   else if (strcmp(hook_type,"jump")==0)     type = PILOT_HOOK_JUMP;
   else if (strcmp(hook_type,"hail")==0)     type = PILOT_HOOK_HAIL;
   else if (strcmp(hook_type,"land")==0)     type = PILOT_HOOK_LAND;
   else if (strcmp(hook_type,"attacked")==0) type = PILOT_HOOK_ATTACKED;
   else if (strcmp(hook_type,"idle")==0)     type = PILOT_HOOK_IDLE;
   else { /* hook_type not valid */
      NLUA_DEBUG("Invalid pilot hook type: '%s'", hook_type);
      return 0;
   }

   /* actually add the hook */
   h = hook_generic( L, hook_type, 3 );
   pilot_addHook( pilot_get(p->pilot), type, h );

   lua_pushnumber( L, h );
   return 1;
}

