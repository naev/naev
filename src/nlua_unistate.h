#include <lua.h>
#include "nxml.h"

#ifndef NLUA_UNISTATE_H
#define NLUA_UNISTATE_H

#define UNISTATE_METATABLE  "unistate"

//structure for the list of asset mods. 
//used when interacting with unistate.xml and saving games
typedef struct unistate_entry {
   char *name;
   char *faction;
   int presence;
   struct unistate_entry *next;
} assetState;

//pointer version of the previous
typedef assetState *assetStatePtr;

//functions
int nlua_loadUnistate( lua_State *L, int readonly );
int unistate_save(xmlTextWriterPtr writer);

#endif