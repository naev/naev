/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

int nlua_loadCLI( nlua_env env ); /* always write only */
int cli_warn( lua_State *L );
int cli_print( lua_State *L );
