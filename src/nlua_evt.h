/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_EVT
#  define NLUA_EVT


/** @cond */
#include <lua.h>
/** @endcond */

#include "event.h"


/* Run Lua for an event. */
Event_t *event_getFromLua( lua_State *L );
void event_runStart( unsigned int eventid, const char *func );
int event_runFunc( unsigned int eventid, const char *func, int nargs );
int event_run( unsigned int eventid, const char *func );

/* individual library stuff */
int nlua_loadEvt( nlua_env env );


#endif /* NLUA_EVT */
