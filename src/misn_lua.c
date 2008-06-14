/*
 * See Licensing and Copyright notice in naev.h
 */

#include "misn_lua.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"

#include "nlua.h"
#include "hook.h"
#include "mission.h"
#include "log.h"
#include "naev.h"
#include "rng.h"
#include "space.h"
#include "toolkit.h"
#include "land.h"
#include "pilot.h"
#include "player.h"
#include "ntime.h"
#include "xml.h"
#include "nluadef.h"



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
/* static */
static int var_add( misn_var *var );
static void var_free( misn_var* var );
static unsigned int hook_generic( lua_State *L, char* stack );
/* externed */
int misn_run( Mission *misn, char *func );
int var_save( xmlTextWriterPtr writer );
int var_load( xmlNodePtr parent );
/* external */
extern void mission_sysMark (void);


/*
 * libraries
 */
/* misn */
static int misn_setTitle( lua_State *L );
static int misn_setDesc( lua_State *L );
static int misn_setReward( lua_State *L );
static int misn_setMarker( lua_State *L );
static int misn_factions( lua_State *L );
static int misn_accept( lua_State *L );
static int misn_finish( lua_State *L );
static const luaL_reg misn_methods[] = {
   { "setTitle", misn_setTitle },
   { "setDesc", misn_setDesc },
   { "setReward", misn_setReward },
   { "setMarker", misn_setMarker },
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
static const luaL_reg var_cond_methods[] = { /* only conditional */
   { "peek", var_peek },
   {0,0}
};
/* player */
static int player_getname( lua_State *L );
static int player_shipname( lua_State *L );
static int player_freeSpace( lua_State *L );
static int player_addCargo( lua_State *L );
static int player_rmCargo( lua_State *L );
static int player_pay( lua_State *L );
static int player_msg( lua_State *L );
static int player_modFaction( lua_State *L );
static int player_getFaction( lua_State *L );
static const luaL_reg player_methods[] = {
   { "name", player_getname },
   { "ship", player_shipname },
   { "freeCargo", player_freeSpace },
   { "addCargo", player_addCargo },
   { "rmCargo", player_rmCargo },
   { "pay", player_pay },
   { "msg", player_msg },
   { "modFaction", player_modFaction },
   { "getFaction", player_getFaction },
   {0,0}
};
/* hooks */
static int hook_land( lua_State *L );
static int hook_takeoff( lua_State *L );
static int hook_time( lua_State *L );
static int hook_enter( lua_State *L );
static int hook_pilot( lua_State *L );
static const luaL_reg hook_methods[] = {
   { "land", hook_land },
   { "takeoff", hook_takeoff },
   { "time", hook_time },
   { "enter", hook_enter },
   { "pilot", hook_pilot },
   {0,0}
};
/* pilots */
static int pilot_addFleet( lua_State *L );
static int pilot_rename( lua_State *L );
static const luaL_reg pilot_methods[] = {
   { "add", pilot_addFleet },
   { "rename", pilot_rename },
   {0,0}
};


/*
 * register all the libraries here
 */
int misn_loadLibs( lua_State *L )
{
   lua_loadNaev(L);
   lua_loadMisn(L);
   lua_loadVar(L,0);
   lua_loadSpace(L,0);
   lua_loadTime(L,0);
   lua_loadPlayer(L);
   lua_loadRnd(L);
   lua_loadTk(L);
   lua_loadHook(L);
   lua_loadPilot(L);
   return 0;
}
int misn_loadCondLibs( lua_State *L )
{
   lua_loadTime(L,1);
   lua_loadVar(L,1);
   return 0;
}
/*
 * individual library loading
 */
int lua_loadMisn( lua_State *L )
{  
   luaL_register(L, "misn", misn_methods);
   return 0;
}  
int lua_loadVar( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "var", var_methods);
   else
      luaL_register(L, "var", var_cond_methods);
   return 0;
}  
int lua_loadPlayer( lua_State *L )
{  
   luaL_register(L, "player", player_methods);
   return 0;
}  
int lua_loadHook( lua_State *L )
{
   luaL_register(L, "hook", hook_methods);
   return 0;
}
int lua_loadPilot( lua_State *L )
{
   luaL_register(L, "pilot", pilot_methods);
   return 0;
}




/*
 * runs a mission function
 *
 * -1 on error, 1 on misn.finish() call and 0 normally
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
      if (strcmp(err,"Mission Done"))
         WARN("Mission '%s' -> '%s': %s",
               cur_mission->data->name, func, (err) ? err : "unknown error");
      else ret = 1;
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
 * saves the mission variables
 */
int var_save( xmlTextWriterPtr writer )
{
   int i;

   xmlw_startElem(writer,"vars");

   for (i=0; i<var_nstack; i++) {
      xmlw_startElem(writer,"var");

      xmlw_attr(writer,"name",var_stack[i].name);

      switch (var_stack[i].type) {
         case MISN_VAR_NIL:
            xmlw_attr(writer,"type","nil");
            break;
         case MISN_VAR_NUM:
            xmlw_attr(writer,"type","num");
            xmlw_str(writer,"%d",var_stack[i].d.num);
            break;
         case MISN_VAR_BOOL:
            xmlw_attr(writer,"type","bool");
            xmlw_str(writer,"%d",var_stack[i].d.b);
            break;
         case MISN_VAR_STR:
            xmlw_attr(writer,"type","str");
            xmlw_str(writer,var_stack[i].d.str);
            break;
      }

      xmlw_endElem(writer); /* "var" */
   }

   xmlw_endElem(writer); /* "vars" */

   return 0;
}


/*
 * loads the vars
 */
int var_load( xmlNodePtr parent )
{
   char *str;
   xmlNodePtr node, cur;
   misn_var var;

   var_cleanup();

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"vars")) {
         cur = node->xmlChildrenNode;
         
         do {
            if (xml_isNode(cur,"var")) {
               xmlr_attr(cur,"name",var.name);
               xmlr_attr(cur,"type",str);
               if (strcmp(str,"nil")==0)
                  var.type = MISN_VAR_NIL;
               else if (strcmp(str,"num")==0) {
                  var.type = MISN_VAR_NUM;
                  var.d.num = atoi( xml_get(cur) );
               }
               else if (strcmp(str,"bool")) {
                  var.type = MISN_VAR_BOOL;
                  var.d.b = atoi( xml_get(cur) );
               }
               else if (strcmp(str,"str")) {
                  var.type = MISN_VAR_STR;
                  var.d.str = strdup( xml_get(cur) );
               }
               else { /* super error checking */
                  WARN("Unknown var type '%s'", str);
                  free(var.name);
                  continue;
               }
               free(str);
               var_add( &var );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;
}


/*
 * adds a var to the stack, strings will be SHARED, don't free
 */
static int var_add( misn_var *new_var )
{
   int i;

   if (var_nstack+1 > var_mstack) { /* more memory */
      var_mstack += 64; /* overkill ftw */
      var_stack = realloc( var_stack, var_mstack * sizeof(misn_var) );
   }

   /* check if already exists */
   for (i=0; i<var_nstack; i++)
      if (strcmp(new_var->name,var_stack[i].name)==0) { /* overwrite */
         var_free( &var_stack[i] );
         memcpy( &var_stack[i], new_var, sizeof(misn_var) );
         return 0;
      }
   
   memcpy( &var_stack[var_nstack], new_var, sizeof(misn_var) );
   var_nstack++;

   return 0;
}




/*
 *   M I S N
 */
static int misn_setTitle( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L, 1)) {
      if (cur_mission->title) /* cleanup old title */
         free(cur_mission->title);
      cur_mission->title = strdup((char*)lua_tostring(L, 1));
   }
   return 0;
}
static int misn_setDesc( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L, 1)) {    
      if (cur_mission->desc) /* cleanup old description */
         free(cur_mission->desc);
      cur_mission->desc = strdup((char*)lua_tostring(L, 1));
   }
   else NLUA_INVALID_PARAMETER();
   return 0;
}
static int misn_setReward( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   if (lua_isstring(L, 1)) {    
      if (cur_mission->reward != NULL) /* cleanup old reward */
         free(cur_mission->reward);
      cur_mission->reward = strdup((char*)lua_tostring(L, 1));
   }
   else NLUA_INVALID_PARAMETER();
   return 0;
}
static int misn_setMarker( lua_State *L )
{
   if (lua_isstring(L, 1)) {
      if (cur_mission->sys_marker != NULL) /* cleanup old marker */
         free(cur_mission->sys_marker);
      cur_mission->sys_marker = strdup((char*)lua_tostring(L,1));
#ifdef DEBUG
      if (system_get(cur_mission->sys_marker)==NULL)
         NLUA_DEBUG("Marking unexistant system '%s'",cur_mission->sys_marker);
#endif
      mission_sysMark();
   }
   else if (cur_mission->sys_marker != NULL) /* no parameter nullifies */
      free(cur_mission->sys_marker);
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

   if (lua_isboolean(L,1)) b = lua_toboolean(L,1);
   else {
      lua_pushstring(L, "Mission Done");
      lua_error(L); /* THERE IS NO RETURN */
      return 0;
   }

   misn_delete = 1;

   if (b && mis_isFlag(cur_mission->data,MISSION_UNIQUE))
      player_missionFinished( mission_getID( cur_mission->data->name ) );

   lua_pushstring(L, "Mission Done");
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
   NLUA_MIN_ARGS(1);
   int i;
   char *str;

   if (lua_isstring(L,1)) str = (char*) lua_tostring(L,1);
   else {
      NLUA_DEBUG("Trying to peek a var with non-string name");
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
   NLUA_MIN_ARGS(1);
   int i;
   char* str;

   if (lua_isstring(L,1)) str = (char*) lua_tostring(L,1);
   else {
      NLUA_DEBUG("Trying to pop a var with non-string name");
      return 0;
   }

   for (i=0; i<var_nstack; i++)
      if (strcmp(str,var_stack[i].name)==0) {
         var_free( &var_stack[i] );
         memmove( &var_stack[i], &var_stack[i+1], sizeof(misn_var)*(var_nstack-i-1) );
         var_stack--;
         return 0;
      } 

   NLUA_DEBUG("Var '%s' not found in stack", str);
   return 0;
}
static int var_push( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   char *str;
   misn_var var;

   if (lua_isstring(L,-2)) str = (char*) lua_tostring(L,-2);
   else {
      NLUA_DEBUG("Trying to push a var with non-string name");
      return 0;
   }
   var.name = strdup(str);
   
   /* store appropriate data */
   if (lua_isnil(L,1)) 
      var.type = MISN_VAR_NIL;
   else if (lua_isnumber(L,1)) {
      var.type = MISN_VAR_NUM;
      var.d.num = (double) lua_tonumber(L,1);
   }
   else if (lua_isboolean(L,1)) {
      var.type = MISN_VAR_BOOL;
      var.d.b = lua_toboolean(L,1);
   }
   else if (lua_isstring(L,1)) {
      var.type = MISN_VAR_STR;
      var.d.str = strdup( (char*) lua_tostring(L,1) );
   }
   else {
      NLUA_DEBUG("Trying to push a var of invalid data type to stack");
      return 0;
   }
   var_add( &var );

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
 *   P L A Y E R
 */
static int player_getname( lua_State *L )
{
   lua_pushstring(L,player_name);
   return 1;
}
static int player_shipname( lua_State *L )
{
   lua_pushstring(L,player->name);
   return 1;
}
static int player_freeSpace( lua_State *L )
{
   lua_pushnumber(L, pilot_cargoFree(player) );
   return 1;
}
static int player_addCargo( lua_State *L )
{
   Commodity *cargo;
   int quantity, ret;

   NLUA_MIN_ARGS(2);

   if (lua_isstring(L,1)) cargo = commodity_get( (char*) lua_tostring(L,1) );
   else return 0;
   if (lua_isnumber(L,2)) quantity = (int) lua_tonumber(L,2);
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

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) id = (unsigned int) lua_tonumber(L,1);
   else return 0;

   ret = pilot_rmMissionCargo( player, id );
   mission_unlinkCargo( cur_mission, id );

   lua_pushboolean(L,!ret);
   return 1;
}
static int player_pay( lua_State *L )
{
   int money;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) money = (int) lua_tonumber(L,1);
   else return 0;

   player->credits += money;

   return 0;
}
static int player_msg( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   char* str;

   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else return 0;

   player_message(str);
   return 0;
}
static int player_modFaction( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int f, mod;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   if (lua_isnumber(L,2)) mod = (int) lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   faction_modPlayer( f, mod );

   return 0;
}
static int player_getFaction( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   int f;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   lua_pushnumber(L, faction_getPlayer(f));

   return 1;
}



/*
 *   H O O K
 */
static unsigned int hook_generic( lua_State *L, char* stack )
{
   int i;
   char *func;

   NLUA_MIN_ARGS(1);

   /* Last parameter must be function to hook */
   if (lua_isstring(L,-1)) func = (char*)lua_tostring(L,-1);
   else NLUA_INVALID_PARAMETER();

   /* make sure mission is a player mission */
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].id == cur_mission->id)
         break;
   if (i>=MISSION_MAX) {
      WARN("Mission not in stack trying to hook");
      return 0;
   }

   return hook_add( cur_mission->id, func, stack );
}
static int hook_land( lua_State *L )
{
   hook_generic( L, "land" );
   return 0;
}
static int hook_takeoff( lua_State *L )
{
   hook_generic( L, "takeoff" );
   return 0;
}
static int hook_time( lua_State *L )
{
   hook_generic( L, "time" );
   return 0;
}
static int hook_enter( lua_State *L )
{
   hook_generic( L, "enter" );
   return 0;
}
static int hook_pilot( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   unsigned int h,p;
   int type;
   char *hook_type;

   /* First parameter parameter - pilot to hook */
   if (lua_isnumber(L,1)) p = (unsigned int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   /* Second parameter - hook name */
   if (lua_isstring(L,2)) hook_type = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Check to see if hook_type is valid */
   if (strcmp(hook_type,"death")==0) type = PILOT_HOOK_DEATH;
   else if (strcmp(hook_type,"board")==0) type = PILOT_HOOK_BOARD;
   else if (strcmp(hook_type,"disable")==0) type = PILOT_HOOK_DISABLE;
   else { /* hook_type not valid */
      NLUA_DEBUG("Invalid pilot hook type: '%s'", hook_type);
      return 0;
   }

   /* actually add the hook */
   h = hook_generic( L, hook_type );
   pilot_addHook( pilot_get(p), type, h );

   return 0;
}



/*
 *   P I L O T
 */
static int pilot_addFleet( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   Fleet *flt;
   char *fltname, *fltai;
   int i, j;
   unsigned int p;
   double a;
   Vector2d vv,vp, vn;

   /* Parse first argument - Fleet Name */
   if (lua_isstring(L,1)) fltname = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   
   /* Parse second argument - Fleet AI Override */
   if (lua_isstring(L,2)) fltai = (char*) lua_tostring(L,2);
   else fltai = NULL;

   /* pull the fleet */
   flt = fleet_get( fltname );
   if (flt == NULL) {
      NLUA_DEBUG("Fleet not found!");
      return 0;
   }

   /* this should probable be done better */
   vect_pset( &vp, RNG(MIN_HYPERSPACE_DIST, MIN_HYPERSPACE_DIST*1.5),
         RNG(0,360)*M_PI/180.);
   vectnull(&vn);

   /* now we start adding pilots and toss ids into the table we return */
   j = 0;
   lua_newtable(L);
   for (i=0; i<flt->npilots; i++) {

      if (RNG(0,100) <= flt->pilots[i].chance) {

         /* fleet displacement */
         vect_cadd(&vp, RNG(75,150) * (RNG(0,1) ? 1 : -1),
               RNG(75,150) * (RNG(0,1) ? 1 : -1));

         a = vect_angle(&vp,&vn);
         vectnull(&vv);
         p = pilot_create( flt->pilots[i].ship,
               flt->pilots[i].name,
               flt->faction,
               (fltai != NULL) ? /* AI Override */
                     ai_getProfile(fltai) : 
                     flt->ai,
               a,
               &vp,
               &vv,
               0 );

         /* we push each pilot created into a table and return it */
         lua_pushnumber(L,++j); /* index, starts with 1 */
         lua_pushnumber(L,p); /* value = pilot id */
         lua_rawset(L,-3); /* store the value in the table */
      }
   }
   return 1;
}
static int pilot_rename( lua_State* L )
{
   NLUA_MIN_ARGS(2);
   char *name;
   unsigned int id;
   Pilot *p;

   if (lua_isnumber(L,1)) id = (unsigned int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isstring(L,2)) name = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();

   p = pilot_get( id );
   free(p->name);
   p->name = strdup(name);
   return 0; 
}


