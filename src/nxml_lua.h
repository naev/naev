/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NXML_LUA_H
#  define NXML_LUA_H


#include "nxml.h"
#include "nlua.h"


int nxml_persistLua( lua_State *L, xmlTextWriterPtr writer );
int nxml_unpersistLua( lua_State *L, xmlNodePtr parent );


#endif /* NXML_LUA_H */


