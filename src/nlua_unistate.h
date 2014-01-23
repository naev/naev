#include <lua.h>

#ifndef NLUA_UNISTATE_H
#define NLUA_UNISTATE_H

#define UNISTATE_METATABLE  "unistate"

int nlua_loadUnistate( lua_State *L, int readonly );
//int unistateL_changeowner(lua_State *L);
//int unistateL_changepresence(lua_State *L);
//int unistateL_getpresence(lua_State *L);

#endif