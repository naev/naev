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



/*
 * libraries
 */
/* naev */
static int naev_lang( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
   {0,0}
};
/* space */
static int space_getPlanet( lua_State *L );
static int space_getSystem( lua_State *L );
static int space_landName( lua_State *L );
static int space_systemName( lua_State *L );
static int space_jumpDist( lua_State *L );
static const luaL_reg space_methods[] = {
   { "getPlanet", space_getPlanet },
   { "getSystem", space_getSystem },
   { "landName", space_landName },
   { "system", space_systemName },
   { "jumpDist", space_jumpDist },
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
 * loads a specially modified version of base
 */
int nlua_loadBase( lua_State* L )
{
   luaopen_base(L); /* open base */

   /* replace package.loaders with a custom one */

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
int lua_loadSpace( lua_State *L, int readonly )
{
   (void)readonly;
   luaL_register(L, "space", space_methods);
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
   /* TODO multilanguage stuff */
   lua_pushstring(L,"en");
   return 1;
}



/*
 *   S P A C E
 */
static int space_getPlanet( lua_State *L )
{
   int i;
   int *factions;
   int nfactions;
   char **planets;
   int nplanets;
   char *rndplanet;
   
   if (lua_gettop(L) == 0) { /* get random planet */
      lua_pushstring(L, space_getRndPlanet());
      return 1;
   }
   else if (lua_isnumber(L,-1)) {
      i = lua_tonumber(L,-1);
      planets = space_getFactionPlanet( &nplanets, &i, 1 );
   }
   else if (lua_isstring(L,-1)) {
      i = faction_get((char*) lua_tostring(L,-1));
      planets = space_getFactionPlanet( &nplanets, &i, 1 );
   }
   else if (lua_istable(L,-1)) {
      /* load up the table */
      lua_pushnil(L);
      nfactions = (int) lua_gettop(L);
      factions = malloc( sizeof(int) * nfactions );
      i = 0;
      while (lua_next(L, -2) != 0) {
         factions[i++] = (int) lua_tonumber(L,-1);
         lua_pop(L,1);
      }
      
      /* get the planets */
      planets = space_getFactionPlanet( &nplanets, factions, nfactions );
      free(factions);
   }
   else return 0; /* nothing useful */

   /* choose random planet */
   if (nplanets == 0) { /* no suitable planet */
      free(planets);
      return 0;
   }
   rndplanet = planets[RNG(0,nplanets-1)];
   free(planets);

   lua_pushstring(L, rndplanet);
   return 1;
}
static int space_getSystem( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   char *planetname, *sysname;

   if (lua_isstring(L,-1)) planetname = (char*) lua_tostring(L,-1);
   else return 0;

   sysname = planet_getSystem( planetname );
   lua_pushstring(L,sysname);
   return 1;
}
static int space_landName( lua_State *L )
{
   if (landed) {
      lua_pushstring(L, land_planet->name);
      return 1;
   }
   return 0;
}
static int space_systemName( lua_State *L )
{
   lua_pushstring(L, cur_system->name);
   return 1;
}
static int space_jumpDist( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   StarSystem **s;
   int jumps;
   char *start, *goal;

   if (lua_isstring(L,-1))
      start = (char*) lua_tostring(L,-1);
   else NLUA_INVALID_PARAMETER();

   if ((lua_gettop(L) > 1) && lua_isstring(L,-2))
      goal = (char*) lua_tostring(L,-2);
   else
      goal = cur_system->name;

   s = map_getJumpPath( &jumps, start, goal, 1 );
   free(s);

   lua_pushnumber(L,jumps);
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
   if ((lua_gettop(L) > 0) && (lua_isnumber(L,-1)))
      nt = ntime_pretty( (unsigned int) lua_tonumber(L,-1) );
   else
      nt = ntime_pretty( ntime_get() );
   lua_pushstring(L, nt);
   free(nt);
   return 1;
}
static int time_units( lua_State *L )
{  
   if ((lua_gettop(L) > 0) && (lua_isnumber(L,-1)))
      lua_pushnumber( L, (unsigned int)lua_tonumber(L,-1) * NTIME_UNIT_LENGTH );
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
      if (lua_isnumber(L, -1))
         lua_pushnumber(L, RNG(0, (int)lua_tonumber(L, -1)));
      else return 0;
   }
   else if (o>=2) { /* random int paramater 1 <= x <= parameter 2 */
      if (lua_isnumber(L, -1) && lua_isnumber(L, -2))
         lua_pushnumber(L,
               RNG((int)lua_tonumber(L, -2), (int)lua_tonumber(L, -1)));
      else return 0;
   }
   else return 0;
   
   return 1; /* unless it's returned 0 already it'll always return a parameter */
}



/*
 *   T O O L K I T
 */
static int tk_msg( lua_State *L )
{  
   char *title, *str;
   NLUA_MIN_ARGS(2);
   
   if (lua_isstring(L,-2)) title = (char*) lua_tostring(L,-2);
   else return 0;
   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else return 0;
   
   dialogue_msg( title, str );
   return 0;
}
static int tk_yesno( lua_State *L )
{  
   int ret;
   char *title, *str;
   NLUA_MIN_ARGS(2);
   
   if (lua_isstring(L,-2)) title = (char*) lua_tostring(L,-2);
   else return 0;
   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else return 0;
   
   ret = dialogue_YesNo( title, str );
   lua_pushboolean(L,ret);
   return 1;
}
static int tk_input( lua_State *L )
{  
   char *title, *str;
   int min, max;
   NLUA_MIN_ARGS(4);

   if (lua_isstring(L,-4)) title = (char*) lua_tostring(L,-4);
   else return 0;
   if (lua_isnumber(L,-3)) min = (int) lua_tonumber(L,-3);
   else return 0;
   if (lua_isnumber(L,-2)) max = (int) lua_tonumber(L,-2);
   else return 0;
   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else return 0;
   
   dialogue_input( title, min, max, str );
   return 0;
}

