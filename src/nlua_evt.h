/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_EVT
#  define NLUA_EVT


#include "event.h"
#include <lua.h>


/* Run Lua for an event. */
Event_t *event_getFromLua( lua_State *L );
void event_setupLua( Event_t *ev, const char *func );
int event_runLuaFunc( Event_t *ev, const char *func, int nargs );
int event_runLua( Event_t *ev, const char *func );

/* individual library stuff */
int nlua_loadEvt( nlua_env env );


#endif /* NLUA_EVT */
