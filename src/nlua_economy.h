/*
 * See Licensing and Copyright notice in naev.h
 */


// #ifndef NLUA_ECONOMY_H
// #  define NLUA_ECONOMY_H

#include <lua.h>

#ifndef NLUA_ECONOMY_H
#	define NLUA_ECONOMY_H


#define ECONOMY_METATABLE   "economy" /**< Commodity metatable identifier. */

 /*
 * Load the system library.
 */
int nlua_loadEconomy( lua_State *L, int readonly );


#endif /* NLUA_ECONOMY_H */

