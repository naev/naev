/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_CLI_H
#  define NLUA_CLI_H


#include <lua.h>


int nlua_loadCLI( lua_State *L ); /* always write only */
int cli_print( lua_State *L );
int nlua_regPrint( lua_State *L );


#endif /* NLUA_CLI_H */


