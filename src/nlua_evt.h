/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_EVT
#  define NLUA_EVT


#include "event.h"
#include "lua.h"


/* Run Lua for an event. */
int event_runLua( Event_t *ev, const char *func );


/* individual library stuff */
int nlua_loadEvt( lua_State *L );


#endif /* NLUA_EVT */
