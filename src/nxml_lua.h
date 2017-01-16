/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NXML_LUA_H
#  define NXML_LUA_H


#include "nxml.h"
#include "nlua.h"


int nxml_persistLua( nlua_env env, xmlTextWriterPtr writer );
int nxml_unpersistLua( nlua_env env, xmlNodePtr parent );


#endif /* NXML_LUA_H */


