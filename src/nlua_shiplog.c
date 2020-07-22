/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_shiplog.c
 *
 * @brief Handles the shiplog Lua bindings.
 */


#include "nlua_shiplog.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"
#include <math.h>

#include <lua.h>
#include <lauxlib.h>

#include "nlua.h"
#include "nlua_hook.h"
#include "nlua_player.h"
#include "nlua_tk.h"
#include "nlua_faction.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_camera.h"
#include "nlua_music.h"
#include "nlua_bkg.h"
#include "nlua_misn.h"
#include "player.h"
#include "mission.h"
#include "log.h"
#include "rng.h"
#include "toolkit.h"
#include "land.h"
#include "nxml.h"
#include "nluadef.h"
#include "music.h"
#include "gui_osd.h"
#include "npc.h"
#include "array.h"
#include "ndata.h"
#include "shiplog.h"

int shiplog_loadShiplog( nlua_env env );
static int shiplog_createLog( lua_State *L );
static int shiplog_appendLog( lua_State *L );
static const luaL_Reg shiplog_methods[] = {
   { "createLog", shiplog_createLog },
   { "appendLog", shiplog_appendLog },
   {0,0}
}; /**< Shiplog Lua methods. */


/**
 * @brief Registers the lua libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int shiplog_loadLibs( nlua_env env )
{
   nlua_loadStandard(env);
   nlua_loadShiplog(env);
   return 0;
}
/*
 * individual library loading
 */
/**
 * @brief Loads the mission Lua library.
 *    @param L Lua state.
 */
int nlua_loadShiplog( nlua_env env )
{
   nlua_register(env, "shiplog", shiplog_methods, 0);
   return 0;
}


/**
 * @brief Creates a shiplog for this mission.
 *
 *    @luatparam string idstr ID string to identify this log, or empty string for unnamed logsets.
 *    @luatparam string name Name for this log.
 *    @luatparam string type Type of log (e.g travel, trade, etc, can be anything.)
 *    @luatparam boolean overwrite Whether to overwrite existing mission with this logname and logtype.  Warning, removes previous entries of this logname and type if 1, or logs of this type if 2.
 *    @luatreturn number The logid of the mission.
 * @luafunc createLog( logname, logtype, overwrite )
 * @usage misn.createLog("MyID","My mission title","Mission type",false) -- where myID can be a string for this mission set (e.g. "shadow"), or an empty string.
 * @usage misn.createLog("","Any title","Anything can be a type",true) with true to replace existing missions of this title and type.
 */
static int shiplog_createLog( lua_State *L )
{
   const char *logname;
   const char *logtype;
   const char *nidstr;
   char *idstr=NULL;
   int overwrite,logid;
   /* Parameters. */
   nidstr      = luaL_checkstring(L,1);
   logname    = luaL_checkstring(L,2);
   logtype    = luaL_checkstring(L,3);
   //overwrite = luaL_checkint(L,4);
   overwrite = lua_toboolean(L,4);
   if( nidstr!=NULL && strlen(nidstr) > 0 )
      idstr = strdup(nidstr);
   /* Create a new shiplog */
   logid = shiplog_create( idstr, logname, logtype, overwrite );
   if ( idstr != NULL )
      free( idstr );
   lua_pushnumber(L, logid);
   return 1;
}
  
/**
 * @brief Appends to the shiplog.
 *
 *    @luatparam string message Message to append to the log.
 * @luafunc appendLog( logid, message )
 * @usage misn.appendLog(5, "Some message here")
 */
static int shiplog_appendLog( lua_State *L )
{
   const char *msg;
   int ret;
   int logid;
   logid = luaL_checkint(L, 1);
   msg = luaL_checkstring(L, 2);
   ret = shiplog_append(logid, msg);

   lua_pushnumber(L, ret); /* 0 on success, -1 on failure */
   return 1;
}

