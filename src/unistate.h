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

//functions
int unistate_save(xmlTextWriterPtr writer);
int unistate_load(xmlNodePtr rootNode);
void unistate_quit(void);
int unistate_setFaction(char *planet, char *faction);
assetStatePtr unistate_getList(void);

#endif