/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NXML_LUA_H
#  define NXML_LUA_H


#include "nxml.h"
#include "nlua.h"


int nxml_persistLua( lua_State *L, xmlTextWriterPtr writer );
int nxml_persistLua_env( nlua_env env, xmlTextWriterPtr writer );
int nxml_unpersistLua( lua_State *L, xmlNodePtr parent );
int nxml_unpersistLua_env( nlua_env env, xmlNodePtr parent );


#endif /* NXML_LUA_H */


