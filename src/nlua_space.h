/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SPACE_H
#  define NLUA_SPACE_H


#include <lua.h>

#include "nlua_planet.h"
#include "nlua_system.h"


/*
 * Load the space library.
 */
int nlua_loadSpace( lua_State *L, int readonly );


#endif /* NLUA_SPACE_H */

