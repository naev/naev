/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_CLI_H
#  define NLUA_CLI_H


#include <lua.h>

#include "nlua.h"


int nlua_loadCLI( nlua_env env ); /* always write only */
int cli_warn( lua_State *L );
int cli_print( lua_State *L );


#endif /* NLUA_CLI_H */


