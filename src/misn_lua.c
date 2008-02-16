/*
 * See Licensing and Copyright notice in naev.h
 */

#include "misn_lua.h"

#include "lua.h"
#include "lauxlib.h"

#include "hook.h"
#include "mission.h"
#include "log.h"
#include "naev.h"
#include "rng.h"
#include "space.h"
#include "toolkit.h"
#include "land.h"
#include "player.h"


#define MISN_DEBUG(str, args...)  (fprintf(stdout,"Mission '%s': "str"\n", cur_mission->data->name, ## args))

#define MIN_ARGS(n)     \
if (lua_gettop(L) < n) { \
   MISN_DEBUG("[%s] Too few arguments", __func__); \
   return 0; \
}



/* similar to lua vars, but with less variety */
#define MISN_VAR_NIL    0
#define MISN_VAR_NUM    1
#define MISN_VAR_BOOL   2
#define MISN_VAR_STR    3
typedef struct misn_var_ {
   char* name;
   char type;
   union {
      double num;
      char* str;
      int b;
   } d;
} misn_var;


/*
 * variable stack
 */
static misn_var* var_stack = NULL;
static int var_nstack = 0;
static int var_mstack = 0;


/*
 * current mission
 */
static Mission *cur_mission = NULL;
static int misn_delete = 0; /* if 1 delete current mission */


/*
 * prototypes
 */
static void var_free( misn_var* var );

/*
 * libraries
 */
/* naev */
static int naev_lang( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
   {0,0}
};
/* misn */
static int misn_setTitle( lua_State *L );
static int misn_setDesc( lua_State *L );
static int misn_setReward( lua_State *L );
static int misn_factions( lua_State *L );
static int misn_accept( lua_State *L );
static int misn_finish( lua_State *L );
static const luaL_reg misn_methods[] = {
   { "setTitle", misn_setTitle },
   { "setDesc", misn_setDesc },
   { "setReward", misn_setReward },
   { "factions", misn_factions },
   { "accept", misn_accept },
   { "finish", misn_finish },
   {0,0}
};
/* var */
static int var_peek( lua_State *L );
static int var_pop( lua_State *L );
static int var_push( lua_State *L );
static const luaL_reg var_methods[] = {
   { "peek", var_peek },
   { "pop", var_pop },
   { "push", var_push },
   {0,0}
};
/* space */
static int space_getPlanet( lua_State *L );
static int space_getSystem( lua_State *L );
static int space_landName( lua_State *L );
static const luaL_reg space_methods[] = {
   { "getPlanet", space_getPlanet },
   { "getSystem", space_getSystem },
   { "landName", space_landName },
   {0,0}
};
/* player */
static int player_freeSpace( lua_State *L );
static int player_addCargo( lua_State *L );
static int player_rmCargo( lua_State *L );
static int player_pay( lua_State *L );
static const luaL_reg player_methods[] = {
   { "freeCargo", player_freeSpace },
   { "addCargo", player_addCargo },
   { "rmCargo", player_rmCargo },
   { "pay", player_pay },
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
/* hooks */
static int hook_land( lua_State *L );
static const luaL_reg hook_methods[] = {
   { "land", hook_land },
   {0,0}
};


/*
 * register all the libraries here
 */
int misn_loadLibs( lua_State *L )
{
   luaL_register(L, "naev", naev_methods);
   luaL_register(L, "misn", misn_methods);
   luaL_register(L, "var", var_methods);
   luaL_register(L, "space", space_methods);
   luaL_register(L, "player", player_methods);
   luaL_register(L, "rnd", rnd_methods);
   luaL_register(L, "tk", tk_methods);
   luaL_register(L, "hook", hook_methods);
   return 0;
}


/*
 * runs a mission function
 */
int misn_run( Mission *misn, char *func )
{
   int i, ret;
   char* err;

   cur_mission = misn;
   misn_delete = 0;

   lua_getglobal( misn->L, func );
   if ((ret = lua_pcall(misn->L, 0, 0, 0))) { /* error has occured */
      err = (lua_isstring(misn->L,-1)) ? (char*) lua_tostring(misn->L,-1) : NULL;
      if (strcmp(err,"Mission Finished"))
         WARN("Mission '%s' -> '%s': %s",
               cur_mission->data->name, func, (err) ? err : "unknown error");
      else ret = 0;
   }

   /* mission is finished */
   if (misn_delete) {
      mission_cleanup( cur_mission );
      for (i=0; i<MISSION_MAX; i++)
         if (cur_mission == &player_missions[i]) {
            memmove( &player_missions[i], &player_missions[i+1],
                  sizeof(Mission) * (MISSION_MAX-i-1) );
            break;
         }
   }

   cur_mission = NULL;

   return ret;
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
 *   M I S N
 */
static int misn_setTitle( lua_State *L )
{
   MIN_ARGS(1);
   if (lua_isstring(L, -1)) {
      if (cur_mission->title) /* cleanup old title */
         free(cur_mission->title);
      cur_mission->title = strdup((char*)lua_tostring(L, -1));
   }
   return 0;
}
static int misn_setDesc( lua_State *L )
{
   MIN_ARGS(1);
   if (lua_isstring(L, -1)) {    
      if (cur_mission->desc) /* cleanup old description */
         free(cur_mission->desc);
      cur_mission->desc = strdup((char*)lua_tostring(L, -1));
   }
   return 0;
}
static int misn_setReward( lua_State *L )
{
   MIN_ARGS(1);
   if (lua_isstring(L, -1)) {    
      if (cur_mission->reward) /* cleanup old reward */
         free(cur_mission->reward);
      cur_mission->reward = strdup((char*)lua_tostring(L, -1));
   }
   return 0;
}
static int misn_factions( lua_State *L )
{
   int i;
   MissionData *dat;

   dat = cur_mission->data;

   /* we'll push all the factions in table form */
   lua_newtable(L);
   for (i=0; i<dat->avail.nfactions; i++) {
      lua_pushnumber(L,i+1); /* index, starts with 1 */
      lua_pushnumber(L,dat->avail.factions[i]); /* value */
      lua_rawset(L,-3); /* store the value in the table */
   }
   return 1;
}
static int misn_accept( lua_State *L )
{
   int i, ret;

   ret = 0;

   /* find last mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data == NULL) break;

   /* no missions left */
   if (i>=MISSION_MAX) ret = 1;
   else { /* copy it over */
      memcpy( &player_missions[i], cur_mission, sizeof(Mission) );
      memset( cur_mission, 0, sizeof(Mission) );
      cur_mission = &player_missions[i];
   }

   lua_pushboolean(L,!ret); /* we'll convert C style return to lua */
   return 1;
}
static int misn_finish( lua_State *L )
{
   int b;

   if (lua_isboolean(L,-1)) b = lua_toboolean(L,-1);
   else {
      MISN_DEBUG("Trying to finish without specifying if mission is complete");
      return 0;
   }

   misn_delete = 1;

   if (b && mis_isFlag(cur_mission->data,MISSION_UNIQUE))
      player_missionFinished( mission_getID( cur_mission->data ) );

   lua_pushstring(L, "Mission Finished");
   lua_error(L); /* shouldn't return */

   return 0;
}



/*
 *   V A R
 */
/* basically checks if a variable exists */
int var_checkflag( char* str )
{
   int i;

   for (i=0; i<var_nstack; i++)
      if (strcmp(var_stack[i].name,str)==0)
         return 1;
   return 0;
}
static int var_peek( lua_State *L )
{
   MIN_ARGS(1);
   int i;
   char *str;

   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else {
      MISN_DEBUG("Trying to peek a var with non-string name");
      return 0;
   }

   for (i=0; i<var_nstack; i++)
      if (strcmp(str,var_stack[i].name)==0) {
         switch (var_stack[i].type) {
            case MISN_VAR_NIL:
               lua_pushnil(L);
               break;
            case MISN_VAR_NUM:
               lua_pushnumber(L,var_stack[i].d.num);
               break;
            case MISN_VAR_BOOL:
               lua_pushboolean(L,var_stack[i].d.b);
               break;
            case MISN_VAR_STR:
               lua_pushstring(L,var_stack[i].d.str);
               break;
         }
         return 1;
      }

   lua_pushnil(L);
   return 1;
}
static int var_pop( lua_State *L )
{
   MIN_ARGS(1);
   int i;
   char* str;

   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else {
      MISN_DEBUG("Trying to pop a var with non-string name");
      return 0;
   }

   for (i=0; i<var_nstack; i++)
      if (strcmp(str,var_stack[i].name)==0) {
         var_free( &var_stack[i] );
         memmove( &var_stack[i], &var_stack[i+1], sizeof(misn_var)*(var_nstack-i-1) );
         var_stack--;
         return 0;
      } 

   MISN_DEBUG("Var '%s' not found in stack", str);
   return 0;
}
static int var_push( lua_State *L )
{
   MIN_ARGS(2);
   int i;
   char *str;
   misn_var *var;

   if (lua_isstring(L,-2)) str = (char*) lua_tostring(L,-2);
   else {
      MISN_DEBUG("Trying to push a var with non-string name");
      return 0;
   }

   
   if (var_nstack+1 > var_mstack) { /* more memory */
      var_mstack += 10;
      var_stack = realloc( var_stack, var_mstack * sizeof(misn_var) );
   }

   /* check if already exists */
   var = NULL;
   for (i=0; i<var_nstack; i++)
      if (strcmp(str,var_stack[i].name)==0)
         var = &var_stack[i];

   if (var==NULL)
      var = &var_stack[var_nstack];
   else if ((var->type==MISN_VAR_STR) && (var->d.str!=NULL)) { /* must free */
      free( var->d.str );
      var->d.str = NULL;
   }

   /* store appropriate data */
   if (lua_isnil(L,-1)) 
      var->type = MISN_VAR_NIL;
   else if (lua_isnumber(L,-1)) {
      var->type = MISN_VAR_NUM;
      var->d.num = (double) lua_tonumber(L,-1);
   }
   else if (lua_isboolean(L,-1)) {
      var->type = MISN_VAR_BOOL;
      var->d.b = lua_toboolean(L,-1);
   }
   else if (lua_isstring(L,-1)) {
      var->type = MISN_VAR_STR;
      var->d.str = strdup( (char*) lua_tostring(L,-1) );
   }
   else {
      MISN_DEBUG("Trying to push a var of invalid data type to stack");
      return 0;
   }

   if (i>=var_nstack) { /* var is new */
      var->name = strdup(str);
      var_nstack++;
   }

   return 0;
}
static void var_free( misn_var* var )
{
   switch (var->type) {
      case MISN_VAR_STR:
         if (var->d.str!=NULL) {
            free(var->d.str);
            var->d.str = NULL;
         }
         break;
      case MISN_VAR_NIL:
      case MISN_VAR_NUM:
      case MISN_VAR_BOOL:
         break;
   }

   if (var->name!=NULL) {
      free(var->name);
      var->name = NULL;
   }
}
void var_cleanup (void)
{
   int i;
   for (i=0; i<var_nstack; i++)
      var_free( &var_stack[i] );

   if (var_stack!=NULL) free( var_stack );
   var_stack = NULL;
   var_nstack = 0;
   var_mstack = 0;
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
   char *planetname, *system;

   MIN_ARGS(1);
   if (lua_isstring(L,-1)) planetname = (char*) lua_tostring(L,-1);
   else return 0;

   system = planet_getSystem( planetname );
   lua_pushstring(L,system);
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



/* 
 *   P L A Y E R
 */
static int player_freeSpace( lua_State *L )
{
   lua_pushnumber(L, pilot_freeCargo(player) );
   return 1;
}
static int player_addCargo( lua_State *L )
{
   Commodity *cargo;
   int quantity, ret;

   MIN_ARGS(2);

   if (lua_isstring(L,-2)) cargo = commodity_get( (char*) lua_tostring(L,-2) );
   else return 0;
   if (lua_isnumber(L,-1)) quantity = (int) lua_tonumber(L,-1);
   else return 0;

   ret = pilot_addMissionCargo( player, cargo, quantity );
   mission_linkCargo( cur_mission, ret );

   lua_pushnumber(L, ret);
   return 1;
}
static int player_rmCargo( lua_State *L )
{
   int ret;
   unsigned int id;

   MIN_ARGS(1);

   if (lua_isnumber(L,-1)) id = (unsigned int) lua_tonumber(L,-1);
   else return 0;

   ret = pilot_rmMissionCargo( player, id );
   mission_unlinkCargo( cur_mission, id );

   lua_pushboolean(L,!ret);
   return 1;
}
static int player_pay( lua_State *L )
{
   int money;

   MIN_ARGS(1);

   if (lua_isnumber(L,-1)) money = (int) lua_tonumber(L,-1);
   else return 0;

   player->credits += money;

   return 0;
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
   MIN_ARGS(2);

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
   MIN_ARGS(2);

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
   MIN_ARGS(4);

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



/*
 *   H O O K
 */
static int hook_land( lua_State *L )
{
   int i;
   char *func;

   MIN_ARGS(1);

   /* make sure mission is a player mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].id == cur_mission->id)
         break;
   if (i>=MISSION_MAX) {
      WARN("Mission not in stack trying to hook");
      return 0;
   }

   if (lua_isstring(L,-1)) func = (char*)lua_tostring(L,-1);
   else {
      WARN("mission '%s': trying to push non-valid function hook",
            cur_mission->data->name);
      return 0;
   }
   hook_add( cur_mission->id, func, "land" );
   return 0;
}
