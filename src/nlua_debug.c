/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_debug.c
 *
 * @brief Handles the Lua debug bindings.
 *
 * These bindings control the debug flags.
 * @luamod debug
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "nlua_debug.h"

#include "debug.h"
#include "nluadef.h"

/* Debug metatable methods. */

static int debugL_showEmitters( lua_State *L );
static const luaL_Reg debugL_methods[] = {
   { "showEmitters", debugL_showEmitters },
   {0,0}
}; /**< Debug metatable methods. */


/**
 * @brief Loads the Debug library.
 *
 *    @param env Environment to load Debug library into.
 *    @return 0 on success.
 */
int nlua_loadDebug( nlua_env env )
{
   /* Register the values */
   nlua_register(env, "debug", debugL_methods, 0);

   return 0;
}


/**
 * @brief Toggles the emitter marker.
 *
 * @usage debug.showEmitters() -- Trail emitters are marked with crosses.
 * @usage debug.showEmitters(false) -- Remove the markers.
 *
 *    @luatparam[opt=true] boolean state Whether to set or unset markers.
 * @luafunc showEmitters
 */
static int debugL_showEmitters( lua_State *L )
{
   int state;

   NLUA_CHECKRW(L);

   /* Get state. */
   if (lua_gettop(L) > 0)
      state = lua_toboolean(L, 1);
   else
      state = 1;

   /* Toggle the markers. */
#if DEBUGGING
   if (state)
      debug_setFlag(DEBUG_MARK_EMITTER);
   else
      debug_rmFlag(DEBUG_MARK_EMITTER);
#else
   (void) state;
   NLUA_ERROR(L, _("Requires a debug build."));
#endif

   return 0;
}
