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
#include "nstring.h"
#include <math.h>

#include <lua.h>
#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_pilot.h"
#include "nlua_time.h"
#include "nlua_misn.h"
#include "nlua_evt.h"
#include "hook.h"
#include "log.h"
#include "event.h"
#include "mission.h"


/* Hook methods. */
static int hookL_rm( lua_State *L );
static int hook_load( lua_State *L );
static int hook_land( lua_State *L );
static int hook_takeoff( lua_State *L );
static int hook_jumpout( lua_State *L );
static int hook_jumpin( lua_State *L );
static int hook_enter( lua_State *L );
static int hook_hail( lua_State *L );
static int hook_board( lua_State *L );
static int hook_timer( lua_State *L );
static int hook_date( lua_State *L );
static int hook_commbuy( lua_State *L );
static int hook_commsell( lua_State *L );
static int hook_input( lua_State *L );
static int hook_mouse( lua_State *L );
static int hook_safe( lua_State *L );
static int hook_standing( lua_State *L );
static int hook_discover( lua_State *L );
static int hook_pilot( lua_State *L );
static const luaL_reg hook_methods[] = {
   { "rm", hookL_rm },
   { "load", hook_load },
   { "land", hook_land },
   { "takeoff", hook_takeoff },
   { "jumpout", hook_jumpout },
   { "jumpin", hook_jumpin },
   { "enter", hook_enter },
   { "hail", hook_hail },
   { "board", hook_board },
   { "timer", hook_timer },
   { "date", hook_date },
   { "comm_buy", hook_commbuy },
   { "comm_sell", hook_commsell },
   { "input", hook_input },
   { "mouse", hook_mouse },
   { "safe", hook_safe },
   { "standing", hook_standing },
   { "discover", hook_discover },
   { "pilot", hook_pilot },
   {0,0}
}; /**< Hook Lua methods. */


/*
 * Prototypes.
 */
static int hookL_setarg( lua_State *L, unsigned int hook, int ind );
static unsigned int hook_generic( lua_State *L, const char* stack, double ms, int pos, ntime_t date );


/**
 * @brief Loads the hook Lua library.
 *    @param L Lua state.
 *    @return 0 on success.
 */
int nlua_loadHook( lua_State *L )
{
   luaL_register(L, "hook", hook_methods);
   return 0;
}


/**
 * @brief Lua bindings to manipulate hooks.
 *
 * Hooks allow you to trigger functions to certain actions like when the player
 *  jumps or a pilot dies.
 *
 * They can have arguments passed to them which will then get passed to the
 *  called hook function.
 *
 * Example usage would be:
 * @code
 * function penter( arg )
 *    -- Function to run when player enters a system
 * end
 *
 * hookid = hook.enter( "penter", 5 )
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
 * @luafunc rm( h )
 */
static int hookL_rm( lua_State *L )
{
   unsigned int h;

   /* Remove the hook. */
   h = luaL_checklong( L, 1 );
   hook_rm( h );

   /* Clean up hook data. */
   lua_getglobal( L, "__hook_arg" );
   if (!lua_isnil(L,-1)) {
      lua_pushnumber( L, h ); /* t, n */
      lua_pushnil( L );       /* t, n, nil */
      lua_settable( L, -3 );  /* t */
   }
   lua_pop( L, 1 );        /* */

   return 0;
}


/**
 * @brief Sets a Lua argument for a hook.
 *
 *    @param L State to set hook argument for.
 *    @param hook Hook to set argument for.
 *    @param ind Index of argument to set.
 *    @return 0 on success.
 */
static int hookL_setarg( lua_State *L, unsigned int hook, int ind )
{
   lua_pushvalue( L, ind );   /* v */
   /* If a table set __save, this won't work for tables of tables however. */
   if (lua_istable(L, -1)) {
      lua_pushboolean( L, 1 );/* v, b */
      lua_setfield( L, -2, "__save" ); /* v */
   }
   /* Create if necessary the actual hook argument table. */
   lua_getglobal( L, "__hook_arg" ); /* v, t */
   if (lua_isnil(L,-1)) {     /* v, nil */
      lua_pop( L, 1 );        /* v */
      lua_newtable( L );      /* v, t */
      lua_pushvalue( L, -1 ); /* v, t, t */
      lua_setglobal( L, "__hook_arg" ); /* v, t */
      lua_pushboolean( L, 1 ); /* v, t, s */
      lua_setfield( L, -2, "__save" ); /* v, t */
   }
   lua_pushnumber( L, hook ); /* v, t, k */
   lua_pushvalue( L, -3 );    /* v, t, k, v */
   lua_settable( L, -3 );     /* v, t */
   lua_pop( L, 2 );           /* */
   return 0;
}


/**
 * @brief Unsets a Lua argument.
 */
void hookL_unsetarg( lua_State *L, unsigned int hook )
{
   lua_getglobal( L, "__hook_arg" ); /* t */
   if (lua_isnil(L,-1)) {            /* */
      lua_pop(L,1);
   }
   lua_pushnumber( L, hook );       /* t, h */
   lua_pushnil( L );                /* t, h, n */
   lua_settable( L, -3 );           /* t */
   lua_pop( L, 1 );
}


/**
 * @brief Gets a Lua argument for a hook.
 *
 *    @param L Lua state to put argument in.
 *    @param hook Hook to get argument of.
 *    @return 0 on success.
 */
int hookL_getarg( lua_State *L, unsigned int hook )
{
   lua_getglobal( L, "__hook_arg" ); /* t */
   if (!lua_isnil(L,-1)) {    /* t */
      lua_pushnumber( L, hook ); /* t, k */
      lua_gettable( L, -2 );  /* t, v */
      lua_remove( L, -2 );    /* v */
   }
   return 0;
}


/**
 * @brief Creates a mission hook to a certain stack.
 *
 * Basically a generic approach to hooking.
 *
 *    @param L Lua state.
 *    @param stack Stack to put the hook in.
 *    @param ms Milliseconds to delay (pass stack as NULL to set as timer).
 *    @param pos Position in the stack of the function name.
 *    @return The hook ID or 0 on error.
 */
static unsigned int hook_generic( lua_State *L, const char* stack, double ms, int pos, ntime_t date )
{
   int i;
   const char *func;
   unsigned int h;
   Event_t *running_event;
   Mission *running_mission;

   /* Last parameter must be function to hook */
   func = luaL_checkstring(L,pos);

   /* Get stuff. */
   running_event = event_getFromLua(L);
   running_mission = misn_getFromLua(L);

   h = 0;
   if (running_mission != NULL) {
      /* make sure mission is a player mission */
      for (i=0; i<MISSION_MAX; i++)
         if (player_missions[i].id == running_mission->id)
            break;
      if (i>=MISSION_MAX) {
         WARN("Mission not in stack trying to hook, forgot to run misn.accept()?");
         return 0;
      }

      if (stack != NULL)
         h = hook_addMisn( running_mission->id, func, stack );
      else if (date != 0)
         h = hook_addDateMisn( running_mission->id, func, date );
      else
         h = hook_addTimerMisn( running_mission->id, func, ms );
   }
   else if (running_event != NULL) {
      if (stack != NULL)
         h = hook_addEvent( running_event->id, func, stack );
      else if (date != 0)
         h = hook_addDateEvt( running_event->id, func, date );
      else
         h = hook_addTimerEvt( running_event->id, func, ms );
   }

   if (h == 0) {
      NLUA_ERROR(L,"No hook target was set.");
      return 0;
   }

   /* Check parameter. */
   if (!lua_isnil(L,pos+1))
      hookL_setarg( L, h, pos+1 );

   return h;
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
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc land( funcname, where, arg )
 */
static int hook_land( lua_State *L )
{
   const char *where;
   unsigned int h;

   if (lua_gettop(L) < 2)
      h = hook_generic( L, "land", 0., 1, 0 );
   else {
      where = luaL_checkstring(L, 2);
      h = hook_generic( L, where, 0., 1, 0 );
   }

   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player loading the game (starts landed).
 *
 * @usage hook.load( "my_function" ) -- Load calls my_function
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc load( funcname, arg )
 */
static int hook_load( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "load", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player taking off.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc takeoff( funcname, arg )
 */
static int hook_takeoff( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "takeoff", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player jumping (before changing systems).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc jumpout( funcname, arg )
 */
static int hook_jumpout( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "jumpout", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player jumping (after changing systems).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc jumpin( funcname, arg )
 */
static int hook_jumpin( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "jumpin", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player entering a system (triggers when taking
 *  off too).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc enter( funcname, arg )
 */
static int hook_enter( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "enter", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player hailing any ship (not a planet).
 *
 * The hook receives a single parameter which is the ship being hailed.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc hail( funcname, arg )
 */
static int hook_hail( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "hail", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player boarding any ship.
 *
 * The hook receives a single parameter which is the ship being boarded.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc board( funcname, arg )
 */
static int hook_board( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "board", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks a timer.
 *
 * The hook receives only the optional argument.
 *
 *    @luaparam ms Milliseconds to delay.
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc timer( ms, funcname, arg )
 */
static int hook_timer( lua_State *L )
{
   unsigned int h;
   double ms;
   ms = luaL_checknumber( L, 1 );
   h  = hook_generic( L, NULL, ms/1000., 2, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks a date change with custom resolution.
 *
 * The hook receives only the optional argument.
 *
 * @usage hook.date( time.create( 0, 0, 1000 ), "some_func", nil ) -- Hooks with a 1000 STU resolution
 *
 *    @luaparam resolution Resolution of the timer (should be a time structure).
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc date( resolution, funcname, arg )
 */
static int hook_date( lua_State *L )
{
   unsigned int h;
   ntime_t t;
   t  = luaL_validtime( L, 1 );
   h  = hook_generic( L, NULL, 0., 2, t );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player buying any sort of commodity.
 *
 * The hook receives the name of the commodity and the quantity being bought.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc comm_buy( funcname, arg )
 */
static int hook_commbuy( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "comm_buy", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player selling any sort of commodity.
 *
 * The hook receives the name of the commodity and the quantity being bought.
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc comm_sell( funcname, arg )
 */
static int hook_commsell( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "comm_sell", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player pressing any input.
 *
 * It returns the name of the key being pressed like "accel" and whether or not it's a press.<br/>
 * <br/>
 * Functions should be in format:<br/>
 *   function f( inputname, inputpress, args )
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc input( funcname, arg )
 */
static int hook_input( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "input", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to the player clicking the mouse.
 *
 * The parameter passed to the function is the button pressed (1==left,2==middle,3==right).
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc mouse( funcname, arg )
 */
static int hook_mouse( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "mouse", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to any faction standing change.
 *
 * The parameters passed to the function are faction whose standing is being
 * changed and the amount changed:<br/>
 * function f( faction, change, args )
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc standing( funcname, arg )
 */
static int hook_standing( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "standing", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to when the player discovers an asset, jump point or the likes.
 *
 * The parameters passed to the function are the type which can be one of:<br/>
 * - "asset" <br/>
 * - "jump" <br/>
 * and the actual asset or jump point discovered with the following format: <br/>
 * function f( type, discovery )
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc discover( funcname, arg )
 */
static int hook_discover( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "discover", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hook run at the end of each frame.
 *
 * This hook is a good way to do possibly breaking stuff like for example player.teleport().
 *
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc safe( funcname, arg )
 */
static int hook_safe( lua_State *L )
{
   unsigned int h;
   h = hook_generic( L, "safe", 0., 1, 0 );
   lua_pushnumber( L, h );
   return 1;
}
/**
 * @brief Hooks the function to a specific pilot.
 *
 * You can hook to different actions.  Currently hook system only supports:<br />
 * <ul>
 *    <li> "death" : triggered when pilot dies (before marked as dead). </li>
 *    <li> "exploded" : triggered when pilot has died and the final explosion has begun. </li>
 *    <li> "board" : triggered when pilot is boarded.</li>
 *    <li> "disable" : triggered when pilot is disabled (with disable set).</li>
 *    <li> "undisable" : triggered when pilot recovers from being disabled.</li>
 *    <li> "jump" : triggered when pilot jumps to hyperspace (before he actually jumps out).</li>
 *    <li> "hail" : triggered when pilot is hailed.</li>
 *    <li> "land" : triggered when pilot is landing (right when starting land descent).</li>
 *    <li> "attacked" : triggered when the pilot is attacked. </li>
 *    <li> "idle" : triggered when the pilot becomes idle in manual control.</li>
 *    <li> "lockon" : triggered when the pilot locked on a missile on it's target.</li>
 * </ul>
 * <br />
 * If you pass nil as pilot, it will set it as a global hook that will jump for all pilots.<br />
 * <br />
 * DO NOT DO UNSAFE THINGS IN PILOT HOOKS. THIS MEANS STUFF LIKE player.teleport(). IF YOU HAVE DOUBTS USE A "safe" HOOK.<br />
 * <br />
 * These hooks all pass the pilot triggering the hook as a parameter, so they should have the structure of:<br />
 * <br />
 * function my_hook( pilot, arg )<br />
 * end<br />
 * <br />
 * The combat hooks also pass the pilot acting on it, so for example the pilot
 *  that disabled, attacked or killed the selected pilot. They have the
 *  following format:<br />
 * <br />
 * function combat_hook( pilot, attacker, arg )<br />
 * end<br />
 * <br />
 * Please note that in the case of disable or death hook the attacker may be nil
 *  indicating that it was killed by other means like for example the shockwave
 *  of a dying ship or nebula volatility.
 *
 *    @luaparam pilot Pilot identifier to hook (or nil for all).
 *    @luaparam type One of the supported hook types.
 *    @luaparam funcname Name of function to run when hook is triggered.
 *    @luaparam arg Argument to pass to hook.
 *    @luareturn Hook identifier.
 * @luafunc pilot( pilot, type, funcname, arg )
 */
static int hook_pilot( lua_State *L )
{
   unsigned int h;
   LuaPilot *p;
   int type;
   const char *hook_type;
   char buf[ PATH_MAX ];

   /* Parameters. */
   if (lua_ispilot(L,1))
      p           = luaL_checkpilot(L,1);
   else if (lua_isnil(L,1))
      p           = NULL;
   else {
      NLUA_ERROR(L, "Invalid parameter #1 for hook.pilot, expecting pilot or nil.");
      return 0;
   }
   hook_type   = luaL_checkstring(L,2);

   /* Check to see if hook_type is valid */
   if (strcmp(hook_type,"death")==0)         type = PILOT_HOOK_DEATH;
   else if (strcmp(hook_type,"exploded")==0) type = PILOT_HOOK_EXPLODED;
   else if (strcmp(hook_type,"board")==0)    type = PILOT_HOOK_BOARD;
   else if (strcmp(hook_type,"disable")==0)  type = PILOT_HOOK_DISABLE;
   else if (strcmp(hook_type,"undisable")==0) type = PILOT_HOOK_UNDISABLE;
   else if (strcmp(hook_type,"jump")==0)     type = PILOT_HOOK_JUMP;
   else if (strcmp(hook_type,"hail")==0)     type = PILOT_HOOK_HAIL;
   else if (strcmp(hook_type,"land")==0)     type = PILOT_HOOK_LAND;
   else if (strcmp(hook_type,"attacked")==0) type = PILOT_HOOK_ATTACKED;
   else if (strcmp(hook_type,"idle")==0)     type = PILOT_HOOK_IDLE;
   else if (strcmp(hook_type,"lockon")==0)   type = PILOT_HOOK_LOCKON;
   else { /* hook_type not valid */
      NLUA_ERROR(L, "Invalid pilot hook type: '%s'", hook_type);
      return 0;
   }

   /* actually add the hook */
   nsnprintf( buf, sizeof(buf), "p_%s", hook_type );
   h = hook_generic( L, buf, 0., 3, 0 );
   if (p==NULL)
      pilots_addGlobalHook( type, h );
   else
      pilot_addHook( pilot_get(p->pilot), type, h );

   lua_pushnumber( L, h );
   return 1;
}

