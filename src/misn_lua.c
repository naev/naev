/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file misn_lua.c
 *
 * @brief Handles the mission lua bindings.
 */


#include "misn_lua.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "lua.h"
#include "lauxlib.h"

#include "nlua.h"
#include "nlua_space.h"
#include "nlua_pilot.h"
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
#include "music.h"
#include "unidiff.h"



/* similar to lua vars, but with less variety */
#define MISN_VAR_NIL    0 /**< Nil type. */
#define MISN_VAR_NUM    1 /**< Number type. */
#define MISN_VAR_BOOL   2 /**< Boolean type. */
#define MISN_VAR_STR    3 /**< String type. */
/**
 * @struct misn_var
 *
 * @brief Contains a mission variable.
 */
typedef struct misn_var_ {
   char* name; /**< Name of the variable. */
   char type; /**< Type of the variable. */
   union {
      double num; /**< Used if type is number. */
      char* str; /**< Used if type is string. */
      int b; /**< Used if type is boolean. */
   } d; /**< Variable data. */
} misn_var;


/*
 * variable stack
 */
static misn_var* var_stack = NULL; /**< Stack of mission variables. */
static int var_nstack = 0; /**< Number of mission variables. */
static int var_mstack = 0; /**< Memory size of the mission variable stack. */


/*
 * current mission
 */
static Mission *cur_mission = NULL; /**< Contains the current mission for a running script. */
static int misn_delete = 0; /**< if 1 delete current mission */


/*
 * prototypes
 */
/* static */
static int var_add( misn_var *var );
static void var_free( misn_var* var );
static unsigned int hook_generic( lua_State *L, char* stack, int pos );
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
static int misn_timerStart( lua_State *L );
static int misn_timerStop( lua_State *L );
static const luaL_reg misn_methods[] = {
   { "setTitle", misn_setTitle },
   { "setDesc", misn_setDesc },
   { "setReward", misn_setReward },
   { "setMarker", misn_setMarker },
   { "factions", misn_factions },
   { "accept", misn_accept },
   { "finish", misn_finish },
   { "timerStart", misn_timerStart },
   { "timerStop", misn_timerStop },
   {0,0}
}; /**< Mission lua methods. */
/* var */
static int var_peek( lua_State *L );
static int var_pop( lua_State *L );
static int var_push( lua_State *L );
static const luaL_reg var_methods[] = {
   { "peek", var_peek },
   { "pop", var_pop },
   { "push", var_push },
   {0,0}
}; /**< Mission variable lua methods. */
static const luaL_reg var_cond_methods[] = {
   { "peek", var_peek },
   {0,0}
}; /**< Conditional mission variable lua methods. */
/* player */
static int player_getname( lua_State *L );
static int player_shipname( lua_State *L );
static int player_freeSpace( lua_State *L );
static int player_addCargo( lua_State *L );
static int player_rmCargo( lua_State *L );
static int player_pay( lua_State *L );
static int player_msg( lua_State *L );
static int player_modFaction( lua_State *L );
static int player_modFactionRaw( lua_State *L );
static int player_getFaction( lua_State *L );
static int player_getRating( lua_State *L );
static int player_getPosition( lua_State *L );
static int player_getPilot( lua_State *L );
static const luaL_reg player_methods[] = {
   { "name", player_getname },
   { "ship", player_shipname },
   { "freeCargo", player_freeSpace },
   { "addCargo", player_addCargo },
   { "rmCargo", player_rmCargo },
   { "pay", player_pay },
   { "msg", player_msg },
   { "modFaction", player_modFaction },
   { "modFactionRaw", player_modFactionRaw },
   { "getFaction", player_getFaction },
   { "getRating", player_getRating },
   { "pos", player_getPosition },
   { "pilot", player_getPilot },
   {0,0}
}; /**< Player lua methods. */
static const luaL_reg player_cond_methods[] = {
   { "name", player_getname },
   { "ship", player_shipname },
   { "getFaction", player_getFaction },
   { "getRating", player_getRating },
   {0,0}
}; /**< Conditional player lua methods. */
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
}; /**< Hook lua methods. */
/* diffs */
static int diff_applyL( lua_State *L );
static int diff_removeL( lua_State *L );
static int diff_isappliedL( lua_State *L );
static const luaL_reg diff_methods[] = {
   { "apply", diff_applyL },
   { "remove", diff_removeL },
   { "isApplied", diff_isappliedL },
   {0,0}
};



/**
 * @fn int misn_loadLibs( lua_State *L )
 *
 * @brief Registers all the mission libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int misn_loadLibs( lua_State *L )
{
   lua_loadNaev(L);
   lua_loadMisn(L);
   lua_loadVar(L,0);
   lua_loadSpace(L,0);
   lua_loadTime(L,0);
   lua_loadPlayer(L,0);
   lua_loadRnd(L);
   lua_loadTk(L);
   lua_loadHook(L);
   lua_loadPilot(L,0);
   lua_loadMusic(L,0);
   return 0;
}
/**
 * @fn int misn_loadCondLibs( lua_State *L )
 *
 * @brief Registers all the mission conditional libraries.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int misn_loadCondLibs( lua_State *L )
{
   lua_loadTime(L,1);
   lua_loadSpace(L,1);
   lua_loadVar(L,1);
   lua_loadPlayer(L,1);
   return 0;
}
/*
 * individual library loading
 */
/**
 * @fn int lua_loadMisn( lua_State *L )
 * @brief Loads the mission lua library.
 *    @param L Lua state.
 */
int lua_loadMisn( lua_State *L )
{  
   luaL_register(L, "misn", misn_methods);
   return 0;
}  
/**
 * @fn int lua_loadVar( lua_State *L )
 * @brief Loads the mission variable lua library.
 *    @param L Lua state.
 */
int lua_loadVar( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "var", var_methods);
   else
      luaL_register(L, "var", var_cond_methods);
   return 0;
}  
/**
 * @fn int lua_loadPlayer( lua_State *L )
 * @brief Loads the player lua library.
 *    @param L Lua state.
 */
int lua_loadPlayer( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "player", player_methods);
   else
      luaL_register(L, "player", player_cond_methods);
   return 0;
}  
/**
 * @fn int lua_loadHook( lua_State *L )
 * @brief Loads the hook lua library.
 *    @param L Lua state.
 */
int lua_loadHook( lua_State *L )
{
   luaL_register(L, "hook", hook_methods);
   return 0;
}


/**
 * @fn int misn_run( Mission *misn, char *func )
 * 
 * @brief Runs a mission function.
 *
 *    @param misn Mission that owns the function.
 *    @param func Name of the function to call.
 *    @return -1 on error, 1 on misn.finish() call and 0 normally.
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
      if (strcmp(err,"Mission Done")!=0)
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


/**
 * @fn int var_save( xmlTextWriterPtr writer )
 *
 * @brief Saves the mission variables.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
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


/**
 * @fn int var_load( xmlNodePtr parent )
 *
 * @brief Loads the vars from XML file.
 *
 *    @param parent Parent node containing the variables.
 *    @return 0 on success.
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
               else if (strcmp(str,"bool")==0) {
                  var.type = MISN_VAR_BOOL;
                  var.d.b = atoi( xml_get(cur) );
               }
               else if (strcmp(str,"str")==0) {
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


/**
 * @fn static int var_add( misn_var *new_var )
 *
 * @brief Adds a var to the stack, strings will be SHARED, don't free.
 *
 *    @param new_var Variable to add.
 *    @return 0 on success.
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




/**
 * @defgroup MISN Misn Lua bindings
 *
 * @brief Generic mission Lua bindings.
 *
 * Functions should be called like:
 *
 * @code
 * misn.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int misn_setTitle( lua_State *L )
 *
 * @brief setTitle( string title )
 *
 * Sets the current mission title.
 *
 *    @param title Title to use for mission.
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
/**
 * @fn static int misn_setDesc( lua_State *L )
 *
 * @brief setDesc( string desc )
 *
 * Sets the current mission description.
 *
 *    @param desc Description to use for mission.
 */
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
/**
 * @fn static int misn_setReward( lua_State *L )
 *
 * @brief setReward( string reward )
 *
 * Sets the current mission reward description.
 *
 *    @param reward Description of the reward to use.
 */
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
/**
 * @fn static int misn_setMarker( lua_State *L )
 *
 * @brief setMarker( [system sys )
 *
 * Sets the mission marker on the system.  If no parameters are passed it
 * unsets the current marker.
 *
 *    @param sys System to mark.
 */
static int misn_setMarker( lua_State *L )
{
   LuaSystem *sys;

   /* No parameter clears the marker */
   if (lua_gettop(L)==0) {
      if (cur_mission->sys_marker != NULL)
         free(cur_mission->sys_marker);
      mission_sysMark(); /* Clear the marker */
   }

   /* Passing in a Star System */
   if (lua_issystem(L,1)) {
      sys = lua_tosystem(L,1);
      cur_mission->sys_marker = strdup(sys->s->name);
      mission_sysMark(); /* mark the system */
   }
   else NLUA_INVALID_PARAMETER();

   return 0;
}
/**
 * @fn static int misn_factions( lua_State *L )
 *
 * @brief table factions( nil )
 *
 * Gets the factions the mission is available for.
 *
 *    @return A containing the factions.
 */
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
/**
 * @fn static int misn_accept( lua_State *L )
 *
 * @brief bool accept( nil )
 *
 * Attempts to accept the mission.
 *
 *    @return true if mission was properly accepted.
 */
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
/**
 * @fn static int misn_finish( lua_State *L )
 *
 * @brief finish( bool properly )
 *
 * Finishes the mission.
 *
 *    @param properly If true and the mission is unique it marks the mission
 *                     as completed.  If false it deletes the mission but
 *                     doesn't mark it as completed.  If the parameter isn't
 *                     passed it just ends the mission.
 */
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

/**
 * @fn static int misn_timerStart( lua_State *L )
 *
 * @brief number timerStart( string func, number delay )
 *
 * Starts a timer.
 *
 *    @param func Function to run when timer is up.
 *    @param delay Milliseconds to wait for timer.
 *    @return The timer being used.
 */
static int misn_timerStart( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int i;
   char *func;
   double delay;

   /* Parse arguments. */
   if (lua_isstring(L,1))
      func = (char*) lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,2))
      delay = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Add timer */
   for (i=0; i<MISSION_TIMER_MAX; i++) {
      if (cur_mission->timer[i] == 0.) {
         cur_mission->timer[i] = delay / 1000.;
         cur_mission->tfunc[i] = strdup(func);
         break;
      }
   }

   /* No timer found. */
   if (i >= MISSION_TIMER_MAX) {
      return 0;
   }

   /* Returns the timer id. */
   lua_pushnumber(L,i);
   return 1;
}

/**
 * @fn static int misn_timerStop( lua_State *L )
 *
 * @brief timerStop( number t )
 *
 * Stops a timer previously started with timerStart().
 *
 *    @param t Timer to stop.
 */
static int misn_timerStop( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   int t;

   /* Parse parameters. */
   if (lua_isnumber(L,1))
      t = (int)lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   /* Stop the timer. */
   if (cur_mission->timer[t] != 0.) {
      cur_mission->timer[t] = 0.;
      if (cur_mission->tfunc[t] != NULL) {
         free(cur_mission->tfunc[t]);
         cur_mission->tfunc[t] = NULL;
      }
   }

   return 0;
}
/**
 * @}
 */



/**
 * @defgroup VAR Mission Variable Lua bindings
 *
 * @brief Mission variable Lua bindings.
 *
 * Mission variables are similar to Lua variables, but are conserved for each
 *  player across all the missions.  They are good for storing campaign or
 *  other global values.
 *
 * Functions should be called like:
 *
 * @code
 * var.function( parameters )
 * @endcode
 */
/**
 * @fn int var_checkflag( char* str )
 *
 * @brief Checks to see if a mission var exists.
 *
 *    @param str Name of the mission var.
 *    @return 1 if it exists, 0 if it doesn't.
 */
int var_checkflag( char* str )
{
   int i;

   for (i=0; i<var_nstack; i++)
      if (strcmp(var_stack[i].name,str)==0)
         return 1;
   return 0;
}
/**
 * @fn static int var_peek( lua_State *L )
 * @ingroup VAR
 *
 * @brief misn_var peek( string name )
 *
 * Gets the mission variable value of a certain name.
 *
 *    @param name Name of the mission variable to get.
 *    @return The value of the mission variable which will depend on what type
 *             it is.
 */
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
/**
 * @fn static int var_pop( lua_State *L )
 * @ingroup VAR
 *
 * @brief pop( string name )
 *
 * Pops a mission variable off the stack, destroying it.
 *
 *    @param name Name of the mission variable to pop.
 */
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
/**
 * @fn static int var_push( lua_State *L )
 * @ingroup VAR
 *
 * @brief push( string name, value )
 *
 * Creates a new mission variable.
 *
 *    @param name Name to use for the new mission variable.
 *    @param value Value of the new mission variable.  Accepted types are:
 *                  nil, bool, string or number.
 */
static int var_push( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   char *str;
   misn_var var;

   if (lua_isstring(L,1)) str = (char*) lua_tostring(L,1);
   else {
      NLUA_DEBUG("Trying to push a var with non-string name");
      return 0;
   }
   var.name = strdup(str);
   
   /* store appropriate data */
   if (lua_isnil(L,2)) 
      var.type = MISN_VAR_NIL;
   else if (lua_isnumber(L,2)) {
      var.type = MISN_VAR_NUM;
      var.d.num = (double) lua_tonumber(L,2);
   }
   else if (lua_isboolean(L,2)) {
      var.type = MISN_VAR_BOOL;
      var.d.b = lua_toboolean(L,2);
   }
   else if (lua_isstring(L,2)) {
      var.type = MISN_VAR_STR;
      var.d.str = strdup( (char*) lua_tostring(L,2) );
   }
   else {
      NLUA_DEBUG("Trying to push a var of invalid data type to stack");
      return 0;
   }
   var_add( &var );

   return 0;
}
/**
 * @fn static void var_free( misn_var* var )
 *
 * @brief Frees a mission variable.
 *
 *    @param var Mission variable to free.
 */
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
/**
 * @fn void var_cleanup (void)
 *
 * @brief Cleans up all the mission variables.
 */
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



/**
 * @defgroup PLAYER Player Lua bindings
 *
 * @brief Lua bindings to interact with the player.
 *
 * Functions should be called like:
 *
 * @code
 * player.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int player_getname( lua_State *L )
 *
 * @brief string name( nil )
 *
 * Gets the player's name.
 *
 *    @return The name of the player.
 */
static int player_getname( lua_State *L )
{
   lua_pushstring(L,player_name);
   return 1;
}
/**
 * @fn static int player_shipname( lua_State *L )
 *
 * @brief string ship( nil )
 *
 * Gets the player's ship's name.
 *
 *    @return The name of the ship the player is currently in.
 */
static int player_shipname( lua_State *L )
{
   lua_pushstring(L,player->name);
   return 1;
}
/**
 * @fn static int player_freeSpace( lua_State *L )
 *
 * @brief number freeCargo( nil )
 *
 * Gets the free cargo space the player has.
 *
 *    @return The free cargo space in tons of the player.
 */
static int player_freeSpace( lua_State *L )
{
   lua_pushnumber(L, pilot_cargoFree(player) );
   return 1;
}
/**
 * @fn static int player_addCargo( lua_State *L )
 *
 * @brief number addCargo( string cargo, number quantity )
 *
 * Adds some mission cargo to the player.  He cannot sell it nor get rid of it
 *  unless he abandons the mission in which case it'll get eliminated.
 *
 *    @param cargo Name of the cargo to add.
 *    @param quantity Quantity of cargo to add.
 *    @return The id of the cargo which can be used in rmCargo.
 */
static int player_addCargo( lua_State *L )
{
   Commodity *cargo;
   int quantity, ret;

   NLUA_MIN_ARGS(2);

   if (lua_isstring(L,1)) cargo = commodity_get( (char*) lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,2)) quantity = (int) lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   ret = pilot_addMissionCargo( player, cargo, quantity );
   mission_linkCargo( cur_mission, ret );

   lua_pushnumber(L, ret);
   return 1;
}
/**
 * @fn static int player_rmCargo( lua_State *L )
 *
 * @brief bool rmCargo( number cargoid )
 *
 * Removes the mission cargo.
 *
 *    @param cargoid Identifier of the mission cargo.
 *    @return true on success.
 */
static int player_rmCargo( lua_State *L )
{
   int ret;
   unsigned int id;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) id = (unsigned int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   ret = pilot_rmMissionCargo( player, id );
   mission_unlinkCargo( cur_mission, id );

   lua_pushboolean(L,!ret);
   return 1;
}
/**
 * @fn static int player_pay( lua_State *L )
 *
 * @brief pay( number amount )
 *
 * Pays the player an amount of money.
 *
 *    @param amount Amount of money to pay the player in credits.
 */
static int player_pay( lua_State *L )
{
   int money;

   NLUA_MIN_ARGS(1);

   if (lua_isnumber(L,1)) money = (int) lua_tonumber(L,1);
   else NLUA_INVALID_PARAMETER();

   player->credits += money;

   return 0;
}
/**
 * @fn static int player_msg( lua_State *L )
 *
 * @brief msg( string message )
 *
 * Sends the player an ingame message.
 *
 *    @param message Message to send the player.
 */
static int player_msg( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   char* str;

   if (lua_isstring(L,-1)) str = (char*) lua_tostring(L,-1);
   else NLUA_INVALID_PARAMETER();

   player_message(str);
   return 0;
}
/**
 * @fn static int player_modFaction( lua_State *L )
 *
 * @brief modFaction( string faction, number mod )
 *
 * Increases the player's standing to a faction by an amount.  This will
 *  affect player's standing with that faction's allies and enemies also.
 *
 *    @param faction Name of the faction.
 *    @param mod Amount to modify standing by.
 */
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
/**
 * @fn static int player_modFactionRaw( lua_State *L )
 *
 * @brief modFactionRaw( string faction, number mod )
 *
 * Increases the player's standing to a faction by a fixed amount without
 *  touching other faction standings.
 *
 *    @param faction Name of the faction.
 *    @param mod Amount to modify standing by.
 */
static int player_modFactionRaw( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   int f, mod;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   if (lua_isnumber(L,2)) mod = (int) lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   faction_modPlayerRaw( f, mod );

   return 0;
}
/**
 * @fn static int player_getFaction( lua_State *L )
 *
 * @brief number getFaction( string faction )
 *
 * Gets the standing of the player with a certain faction.
 *
 *    @param faction Faction to get the standing of.
 *    @return The faction standing.
 */
static int player_getFaction( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   int f;

   if (lua_isstring(L,1)) f = faction_get( lua_tostring(L,1) );
   else NLUA_INVALID_PARAMETER();

   lua_pushnumber(L, faction_getPlayer(f));

   return 1;
}
/**
 * @fn static int player_getRating( lua_State *L )
 *
 * @brief number, string getRating( nil )
 *
 * Gets the player's combat rating.
 *
 *    @return Returns the combat rating (in raw number) and the actual
 *             standing in human readable form.
 */
static int player_getRating( lua_State *L )
{
   lua_pushnumber(L, player_crating);
   lua_pushstring(L, player_rating());
   return 2;
}

/**
 * @fn static int player_getPosition( lua_State *L )
 *
 * @brief Vec2 getPos( nil )
 *
 * Gets the player's position.
 *
 *    @return The position of the player.
 */
static int player_getPosition( lua_State *L )
{
   LuaVector v;

   vectcpy( &v.vec, &player->solid->pos );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @fn static int player_getPilot( lua_State *L )
 *
 * @brief Pilot getPilot( nil )
 *
 * Gets the player's associated pilot.
 *
 *    @return The player's pilot.
 */
static int player_getPilot( lua_State *L )
{
   LuaPilot lp;
   lp.pilot = PLAYER_ID;
   lua_pushpilot(L, lp);
   return 1;
}
/**
 * @}
 */



/**
 * @defgroup HOOK Hook Lua bindings
 *
 * @brief Lua bindings to manipulate hooks.
 *
 * Functions should be called like:
 *
 * @code
 * hook.function( parameters )
 * @endcode
 */
/**
 * @fn static unsigned int hook_generic( lua_State *L, char* stack, int pos )
 *
 * @brief Creates a mission hook to a certain stack.
 *
 * Basically a generic approach to hooking.
 *
 *    @param L Lua state.
 *    @param stack Stack to put the hook in.
 *    @param pos Position in the stack of the function name.
 *    @return The hook ID or 0 on error.
 */
static unsigned int hook_generic( lua_State *L, char* stack, int pos )
{
   int i;
   char *func;

   NLUA_MIN_ARGS(1);

   /* Last parameter must be function to hook */
   if (lua_isstring(L,pos)) func = (char*)lua_tostring(L,pos);
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
/**
 * @fn static int hook_land( lua_State *L )
 * @ingroup HOOK
 *
 * @brief number land( string func )
 *
 * Hooks the function to the player landing.
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_land( lua_State *L )
{
   hook_generic( L, "land", 1 );
   return 0;
}
/**
 * @fn static int hook_takeoff( lua_State *L )
 * @ingroup HOOK
 *
 * @brief number takeoff( string func )
 *
 * Hooks the function to the player taking off
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_takeoff( lua_State *L )
{
   hook_generic( L, "takeoff", 1 );
   return 0;
}
/**
 * @fn static int hook_time( lua_State *L )
 * @ingroup HOOK
 *
 * @brief number time( string func )
 *
 * Hooks the function to a time change.
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_time( lua_State *L )
{
   hook_generic( L, "time", 1 );
   return 0;
}
/**
 * @fn static int hook_enter( lua_State *L )
 * @ingroup HOOK
 *
 * @brief number enter( string func )
 *
 * Hooks the function to the player entering a system (triggers when taking
 *  off too).
 *
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_enter( lua_State *L )
{
   hook_generic( L, "enter", 1 );
   return 0;
}
/**
 * @fn static int hook_pilot( lua_State *L )
 * @ingroup HOOK
 *
 * @brief number pilot( Pilot pilot, string type, string func )
 *
 * Hooks the function to a specific pilot.
 *
 * You can hook to different actions.  Curently hook system only supports:
 *    - "death" :  triggered when pilot dies.
 *    - "board" :  triggered when pilot is boarded.
 *    - "disable" :  triggered when pilot is disabled.
 *    - "jump" : triggered when pilot jumps to hyperspace.
 *
 *    @param pilot Pilot identifier to hook.
 *    @param type One of the supported hook types.
 *    @param func Function to run when hook is triggered.
 *    @return Hook identifier.
 */
static int hook_pilot( lua_State *L )
{
   NLUA_MIN_ARGS(3);
   unsigned int h;
   LuaPilot *p;
   int type;
   char *hook_type;

   /* First parameter parameter - pilot to hook */
   if (lua_ispilot(L,1)) p = lua_topilot(L,1);
   else NLUA_INVALID_PARAMETER();

   /* Second parameter - hook name */
   if (lua_isstring(L,2)) hook_type = (char*) lua_tostring(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Check to see if hook_type is valid */
   if (strcmp(hook_type,"death")==0) type = PILOT_HOOK_DEATH;
   else if (strcmp(hook_type,"board")==0) type = PILOT_HOOK_BOARD;
   else if (strcmp(hook_type,"disable")==0) type = PILOT_HOOK_DISABLE;
   else if (strcmp(hook_type,"jump")==0) type = PILOT_HOOK_JUMP;
   else { /* hook_type not valid */
      NLUA_DEBUG("Invalid pilot hook type: '%s'", hook_type);
      return 0;
   }

   /* actually add the hook */
   h = hook_generic( L, hook_type, 3 );
   pilot_addHook( pilot_get(p->pilot), type, h );

   return 0;
}


/**
 * @defgroup DIFF Universe Diff Lua Bindings
 *
 * @brief Lua bindings to apply/remove Universe Diffs.
 *
 * Functions should be called like:
 *
 * @code
 * diff.function( parameters )
 * @endcode
 *
 * @{
 */
/**
 * @fn static int diff_applyL( lua_State *L )
 *
 * @brief apply( string name )
 *
 * Applies a diff by name.
 *
 *    @param name Name of the diff to apply.
 */
static int diff_applyL( lua_State *L )
{
   char *name;

   if (lua_isstring(L,1)) name = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   diff_apply( name );
   return 0;
}
/**
 * @fn static int diff_removeL( lua_State *L )
 *
 * @brief remove( string name )
 *
 * Removes a diff by name.
 *
 *    @param name Name of the diff to remove.
 */
static int diff_removeL( lua_State *L )
{
   char *name;

   if (lua_isstring(L,1)) name = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   diff_remove( name );
   return 0;
}
/**
 * @fn static int diff_isappliedL( lua_State *L )
 *
 * @brief bool isApplied( string name )
 *
 * Checks to see if a diff is currently applied.
 *
 *    @param name Name of the diff to check.
 *    @return true if is applied, false if it isn't.
 */
static int diff_isappliedL( lua_State *L )
{
   char *name;

   if (lua_isstring(L,1)) name = (char*)lua_tostring(L,1);
   else NLUA_INVALID_PARAMETER();

   lua_pushboolean(L,diff_isApplied(name));
   return 1;
}
/**
 * @}
 */
