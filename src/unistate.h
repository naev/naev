/**
 * @file unistate.h
 * 
 * @brief Header to unistate.c.
 */

#include <lua.h>
#include "nxml.h"

#ifndef UNISTATE_H
#define UNISTATE_H



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

//pointer to first element of global list
extern assetStatePtr unistateList;

//functions
int unistate_save(xmlTextWriterPtr writer);
int unistate_load(xmlNodePtr rootNode);
void unistate_freeList(assetStatePtr list);

#endif