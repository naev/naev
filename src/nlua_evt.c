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
#include "log.h"
#include "event.h"
#include "mission.h"


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
 * current mission
 */
static Event_t *cur_event = NULL; /**< Contains the current event for a running script. */
static int evt_delete = 0; /**< if 1 delete current event */


/*
 * libraries
 */
/* evt */
static int evt_misnStart( lua_State *L );
static int evt_timerStart( lua_State *L );
static int evt_timerStop( lua_State *L );
static int evt_finish( lua_State *L );
static const luaL_reg evt_methods[] = {
   { "misnStart", evt_misnStart },
   { "timerStart", evt_timerStart },
   { "timerStop", evt_timerStop },
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
 * @brief Runs the Lua for an event.
 */
int event_runLua( Event_t *ev, const char *func )
{
   int ret;
   const char* err;
   lua_State *L;

   /* Load event. */
   L = ev->L;
   cur_event = ev;
   evt_delete = 0;

   /* Get function. */
   lua_getglobal(L, func );

   ret = lua_pcall(L, 0, 0, 0);
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      if (strcmp(err,"Event Done")!=0) {
         WARN("Event '%s' -> '%s': %s",
               event_getData(ev->id), func, (err) ? err : "unknown error");
         lua_pop(L, 1);
      }
      else
         ret = 1;
   }

   /* Time to remove the event. */
   if (evt_delete) {
      event_remove( cur_event->id );
   }

   /* Unload event. */
   cur_event = NULL;

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
   if (mission_start( str ))
      NLUA_ERROR(L,"Failed to start mission.");

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
static int evt_timerStart( lua_State *L )
{
   int i;
   const char *func;
   double delay;

   /* Parse arguments. */
   func  = luaL_checkstring(L,1);
   delay = luaL_checknumber(L,2);

   /* Add timer */
   for (i=0; i<EVENT_TIMER_MAX; i++) {
      if (cur_event->timer[i] == 0.) {
         cur_event->timer[i] = delay / 1000.;
         cur_event->tfunc[i] = strdup(func);
         break;
      }
   }

   /* No timer found. */
   if (i >= EVENT_TIMER_MAX) {
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
static int evt_timerStop( lua_State *L )
{
   int t;

   /* Parse parameters. */
   t = luaL_checkint(L,1);

   /* Stop the timer. */
   if (cur_event->timer[t] != 0.) {
      cur_event->timer[t] = 0.;
      if (cur_event->tfunc[t] != NULL) {
         free(cur_event->tfunc[t]);
         cur_event->tfunc[t] = NULL;
      }
   }

   return 0;
}


/**
 * @brief Finishes the event.
 *
 * @luafunc finish()
 */
static int evt_finish( lua_State *L )
{
   evt_delete = 1;

   lua_pushstring(L, "Event Done");
   lua_error(L); /* shouldn't return */

   return 0;            
}

