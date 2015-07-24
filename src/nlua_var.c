/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_var.c
 *
 * @brief Lua Variable module.
 */


#include "nlua_var.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"
#include <math.h>

#include <lua.h>
#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "nxml.h"



/* similar to Lua vars, but with less variety */
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
static int var_nstack      = 0; /**< Number of mission variables. */
static int var_mstack      = 0; /**< Memory size of the mission variable stack. */


/*
 * prototypes
 */
/* static */
static int var_add( misn_var *var );
static void var_free( misn_var* var );
/* externed */
int var_save( xmlTextWriterPtr writer );
int var_load( xmlNodePtr parent );


/* var */
static int var_peek( lua_State *L );
static int var_pop( lua_State *L );
static int var_push( lua_State *L );
static const luaL_reg var_methods[] = {
   { "peek", var_peek },
   { "pop", var_pop },
   { "push", var_push },
   {0,0}
}; /**< Mission variable Lua methods. */
static const luaL_reg var_cond_methods[] = {
   { "peek", var_peek },
   {0,0}
}; /**< Conditional mission variable Lua methods. */


/**
 * @brief Loads the mission variable Lua library.
 *    @param L Lua state.
 *    @param readonly Whether to open in read-only form.
 *    @return 0 on success.
 */
int nlua_loadVar( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "var", var_methods);
   else
      luaL_register(L, "var", var_cond_methods);
   return 0;
}


/**
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

      xmlw_attr(writer,"name","%s",var_stack[i].name);

      switch (var_stack[i].type) {
         case MISN_VAR_NIL:
            xmlw_attr(writer,"type","nil");
            break;
         case MISN_VAR_NUM:
            xmlw_attr(writer,"type","num");
            xmlw_str(writer,"%f",var_stack[i].d.num);
            break;
         case MISN_VAR_BOOL:
            xmlw_attr(writer,"type","bool");
            xmlw_str(writer,"%d",var_stack[i].d.b);
            break;
         case MISN_VAR_STR:
            xmlw_attr(writer,"type","str");
            xmlw_str(writer,"%s",var_stack[i].d.str);
            break;
      }

      xmlw_endElem(writer); /* "var" */
   }

   xmlw_endElem(writer); /* "vars" */

   return 0;
}


/**
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
                  var.d.num = xml_getFloat(cur);
               }
               else if (strcmp(str,"bool")==0) {
                  var.type = MISN_VAR_BOOL;
                  var.d.b = xml_getInt(cur);
               }
               else if (strcmp(str,"str")==0) {
                  var.type = MISN_VAR_STR;
                  var.d.str = xml_getStrd(cur);
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
 * @brief Mission variable Lua bindings.
 *
 * Mission variables are similar to Lua variables, but are conserved for each
 *  player across all the missions.  They are good for storing campaign or
 *  other global values.
 *
 * Typical usage would be:
 * @code
 * v = var.peek( "es_misn" ) -- Get the value
 * if v == nil then -- Doesn't exist, so create
 *    var.push( "es_misn", 1 )
 * else
 *    var.push( "es_misn", v+1 ) -- Increment value
 * end
 * @endcode
 *
 * @luamod var
 */
/**
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
 * @brief Gets the mission variable value of a certain name.
 *
 *    @luaparam name Name of the mission variable to get.
 *    @luareturn The value of the mission variable which will depend on what type
 *             it is.
 * @luafunc peek( name )
 */
static int var_peek( lua_State *L )
{
   int i;
   const char *str;

   /* Get the parameter. */
   str = luaL_checkstring(L,1);

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

   return 0;
}
/**
 * @brief Pops a mission variable off the stack, destroying it.
 *
 * This does not give you any value and destroys it permanently (or until recreated).
 *
 *    @luaparam name Name of the mission variable to pop.
 * @luafunc pop( name )
 */
static int var_pop( lua_State *L )
{
   int i;
   const char* str;

   str = luaL_checkstring(L,1);

   for (i=0; i<var_nstack; i++)
      if (strcmp(str,var_stack[i].name)==0) {
         var_free( &var_stack[i] );
         memmove( &var_stack[i], &var_stack[i+1], sizeof(misn_var)*(var_nstack-i-1) );
         var_nstack--;
         return 0;
      }

   /*NLUA_DEBUG("Var '%s' not found in stack", str);*/
   return 0;
}
/**
 * @brief Creates a new mission variable.
 *
 * This will overwrite existing vars, so it's a good way to update the values
 *  of different mission variables.
 *
 *    @luaparam name Name to use for the new mission variable.
 *    @luaparam value Value of the new mission variable.  Accepted types are:
 *                  nil, bool, string or number.
 * @luafunc push( name, value )
 */
static int var_push( lua_State *L )
{
   const char *str;
   misn_var var;

   str = luaL_checkstring(L,1);

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
      var.d.str = strdup( lua_tostring(L,2) );
   }
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }
   /* Set name. */
   var.name = strdup(str);
   var_add( &var );

   return 0;
}
/**
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
 * @brief Cleans up all the mission variables.
 */
void var_cleanup (void)
{
   int i;
   for (i=0; i<var_nstack; i++)
      var_free( &var_stack[i] );

   if (var_stack!=NULL)
      free( var_stack );
   var_stack   = NULL;
   var_nstack  = 0;
   var_mstack  = 0;
}

