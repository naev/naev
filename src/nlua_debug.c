/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_debug.c
 *
 * @brief Handles the Lua debug bindings.
 *
 * These bindings control the debug flags.
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "nlua_debug.h"

#include "debug.h"
#include "nluadef.h"

/* Debug metatable methods. */

static int debugL_dispEmitters( lua_State *L );
static const luaL_Reg debugL_methods[] = {
   { "dispEmitters", debugL_dispEmitters },
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
 * @usage dispEmitters() -- Trail emitters are marked with crosses.
 * @usage dispEmitters(false) -- Remove the markers.
 *
 *    @luatparam[opt=true] boolean state Whether to set or unset markers.
 * @luafunc dispEmitters
 */
static int debugL_dispEmitters( lua_State *L )
{
   int state;

   NLUA_CHECKRW(L);

   /* Get state. */
   if (lua_gettop(L) > 0)
      state = lua_toboolean(L, 1);
   else
      state = 1;

   /* Set as hostile. */
   if (state)
      debug_setFlag(MARK_EMITTER);
   else
      debug_rmFlag(MARK_EMITTER);

   return 0;
}
