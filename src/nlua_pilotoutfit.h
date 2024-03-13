/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "pilot.h"

#define PILOTOUTFIT_METATABLE                                                  \
   "pilotoutfit" /**< Pilot outfit metatable identifier. */

extern int pilotoutfit_modified;

/*
 * Library loading
 */
int nlua_loadPilotOutfit( nlua_env env );

/*
 * Outfit operations
 */
PilotOutfitSlot  *lua_topilotpilotoutfit( lua_State *L, int ind );
PilotOutfitSlot  *luaL_checkpilotoutfit( lua_State *L, int ind );
PilotOutfitSlot  *luaL_validpilotoutfit( lua_State *L, int ind );
PilotOutfitSlot **lua_pushpilotoutfit( lua_State *L, PilotOutfitSlot *po );
int               lua_ispilotoutfit( lua_State *L, int ind );
