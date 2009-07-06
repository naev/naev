/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_HOOK
#  define NLUA_HOOK

#include "lua.h"
#include "mission.h"
#include "event.h"


/* Sets the hook target. */
void nlua_hookTarget( Mission *m, Event_t *ev );


/* individual library stuff */
int nlua_loadHook( lua_State *L );


#endif /* NLUA_HOOK */
