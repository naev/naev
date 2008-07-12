/*
 * See Licensing and Copyright notice in naev.h
 */


#include "nlua.h"

#include "lauxlib.h"

#include "log.h"
#include "naev.h"
#include "rng.h"
#include "ntime.h"
#include "toolkit.h"
#include "space.h"
#include "land.h"
#include "nluadef.h"
#include "map.h"
#include "pack.h"


/*
 * prototypes
 */
static int nlua_packfileLoader( lua_State* L );


/*
 * libraries
 */
/* naev */
static int naev_lang( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
   {0,0}
};
/* time */
static int time_get( lua_State *L );
static int time_str( lua_State *L );
static int time_units( lua_State *L );
static const luaL_reg time_methods[] = {
   { "get", time_get },
   { "str", time_str },
   { "units", time_units },
   {0,0}                                                                  
};                                                                        
/* rnd */
static int rnd_int( lua_State *L );
static const luaL_reg rnd_methods[] = {
   { "int", rnd_int },
   {0,0}
};
/* toolkit */
static int tk_msg( lua_State *L );
static int tk_yesno( lua_State *L );
static int tk_input( lua_State *L );
static const luaL_reg tk_methods[] = {
   { "msg", tk_msg },
   { "yesno", tk_yesno },
   { "input", tk_input },
   {0,0}
};



/*
 * wrapper around luaL_newstate
 */
lua_State *nlua_newState (void)
{
   lua_State *L;

   /* try to create the new state */
   L = luaL_newstate();
   if (L == NULL) {
      WARN("Failed to create new lua state.");
      return NULL;
   }

   return L;
}


/*
 * opens a lua library
 */
int nlua_load( lua_State* L, lua_CFunction f )
{
   lua_pushcfunction(L, f);
   if (lua_pcall(L, 0, 0, 0))
      WARN("nlua include error: %s",lua_tostring(L,1));

   return 0;
}


/**
 * @fn int nlua_loadBasic( lua_State* L )
 *
 * @brief Loads specially modified basic stuff.
 *
 *    @param L Lua State to load the basic stuff into.
 *    @return 0 on success.
 */
int nlua_loadBasic( lua_State* L )
{
   int i;
   const char *override[] = { /* unsafe functions */
         "collectgarbage",
         "dofile",
         "getfenv",
         "getmetatable",
         "load",
         "loadfile",
         "loadstring",
         "rawequal",
         "rawget",
         "rawset",
         "setfenv",
         "setmetatable",
         "END"
   };


   nlua_load(L,luaopen_base); /* open base */

   /* replace non-safe functions */
   for (i=0; strcmp(override[i],"END")!=0; i++) {
      lua_pushnil(L);
      lua_setglobal(L, override[i]);
   }

   /* add our own */
   lua_register(L, "include", nlua_packfileLoader);

   return 0;
}
static int nlua_packfileLoader( lua_State* L )
{
   char *buf, *filename;
   uint32_t bufsize;

   NLUA_MIN_ARGS(1);

   if (!lua_isstring(L,1)) {
      NLUA_INVALID_PARAMETER();
      return 0;
   }

   filename = (char*) lua_tostring(L,1);

   /* try to locate the data */
   buf = pack_readfile( DATA, filename, &bufsize );
   if (buf == NULL) {
      lua_pushfstring(L, "%s not found in packfile %s", filename, DATA);
      return 1;
   }
   
   /* run the buffer */
   if (luaL_dobuffer(L, buf, bufsize, filename) != 0) {
      /* will push the current error from the dobuffer */
      return 1;
   }

   /* cleanup, success */
   free(buf);
   return 0;
}



/*
 * individual library loading
 */
int lua_loadNaev( lua_State *L )
{  
   luaL_register(L, "naev", naev_methods);
   return 0;
}
int lua_loadTime( lua_State *L, int readonly )
{
   (void)readonly;
   luaL_register(L, "time", time_methods);
   return 0;
}
int lua_loadRnd( lua_State *L )
{
   luaL_register(L, "rnd", rnd_methods);
   return 0;
}
int lua_loadTk( lua_State *L )
{
   luaL_register(L, "tk", tk_methods);
   return 0;
}




/*
 *   N A E V
 */
static int naev_lang( lua_State *L )
{  
   /** @todo multilanguage stuff */
   lua_pushstring(L,"en");
   return 1;
}


/*
 *   T I M E
 */
static int time_get( lua_State *L )
{
   lua_pushnumber( L, ntime_get() );
   return 1;
}
static int time_str( lua_State *L )
{
   char *nt;
   if ((lua_gettop(L) > 0) && (lua_isnumber(L,1)))
      nt = ntime_pretty( (unsigned int) lua_tonumber(L,1) );
   else
      nt = ntime_pretty( ntime_get() );
   lua_pushstring(L, nt);
   free(nt);
   return 1;
}
static int time_units( lua_State *L )
{  
   if ((lua_gettop(L) > 0) && (lua_isnumber(L,1)))
      lua_pushnumber( L, (unsigned int)lua_tonumber(L,1) * NTIME_UNIT_LENGTH );
   else
      lua_pushnumber( L, NTIME_UNIT_LENGTH );
   return 1;
}



/*
 *   R N D
 */
static int rnd_int( lua_State *L )
{  
   int o;
   
   o = lua_gettop(L);
   
   if (o==0) lua_pushnumber(L, RNGF() ); /* random double 0 <= x <= 1 */
   else if (o==1) { /* random int 0 <= x <= parameter */
      if (lua_isnumber(L, 1))
         lua_pushnumber(L, RNG(0, (int)lua_tonumber(L, 1)));
      else return 0;
   }
   else if (o>=2) { /* random int paramater 1 <= x <= parameter 2 */
      if (lua_isnumber(L, 1) && lua_isnumber(L, 2))
         lua_pushnumber(L,
               RNG((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2)));
      else return 0;
   }
   else NLUA_INVALID_PARAMETER();
   
   return 1; /* unless it's returned 0 already it'll always return a parameter */
}



/*
 *   T O O L K I T
 */
static int tk_msg( lua_State *L )
{  
   char *title, *str;
   NLUA_MIN_ARGS(2);
   
   if (lua_isstring(L,1)) title = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,2)) str = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();
   
   dialogue_msg( title, str );
   return 0;
}
static int tk_yesno( lua_State *L )
{  
   int ret;
   char *title, *str;
   NLUA_MIN_ARGS(2);
   
   if (lua_isstring(L,1)) title = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,2)) str = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();
   
   ret = dialogue_YesNo( title, str );
   lua_pushboolean(L,ret);
   return 1;
}
static int tk_input( lua_State *L )
{  
   char *title, *str;
   int min, max;
   NLUA_MIN_ARGS(4);

   if (lua_isstring(L,1)) title = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,2)) min = (int) lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,3)) max = (int) lua_tonumber(L,3);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,4)) str = (char*) lua_tostring(L,4);
   else NLUA_INVALID_PARAMETER();
   
   dialogue_input( title, min, max, str );
   return 0;
}

