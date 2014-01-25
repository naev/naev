/**
 * @file nlua_unistate.h
 * 
 * @brief Header to nlua_unistate.c.
 */

#include <lua.h>

#ifndef NLUA_UNISTATE_H
#define NLUA_UNISTATE_H

#define UNISTATE_METATABLE  "unistate"

//functions
int nlua_loadUnistate( lua_State *L, int readonly );

#endif