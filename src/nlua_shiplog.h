/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SHIPLIG
#  define NLUA_SHIPLOG

#include <lua.h>
#include <nlua.h>
#include "shiplog.h"


/* load the libraries for a Lua state */
int shiplog_loadLibs( nlua_env env );

/* individual library stuff */
int nlua_loadShiplog( nlua_env env );



#endif /* NLUA_SHIPLOG */
