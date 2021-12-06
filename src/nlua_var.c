/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_var.c
 *
 * @brief Lua Variable module.
 */
/** @cond */
#include <lauxlib.h>
#include <lua.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_var.h"

#include "array.h"
#include "log.h"
#include "nluadef.h"
#include "nstring.h"
#include "nxml.h"
#include "nlua_time.h"

/* similar to Lua vars, but with less variety */
enum {
   MISN_VAR_NIL, /**< Nil type. */
   MISN_VAR_NUM, /**< Number type. */
   MISN_VAR_BOOL,/**< Boolean type. */
   MISN_VAR_STR, /**< String type. */
   MISN_VAR_TIME,/**< Time type. */
};

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
      ntime_t time;
   } d; /**< Variable data. */
} misn_var;

/*
 * variable stack
 */
static misn_var* var_stack = NULL; /**< Stack of mission variables. */

/*
 * prototypes
 */
/* static */
static int var_cmp( const void *p1, const void *p2 );
static misn_var *var_get( const char *str );
static int var_add( misn_var *new_var, int sort );
static void var_free( misn_var* var );
/* externed */
int var_save( xmlTextWriterPtr writer );
int var_load( xmlNodePtr parent );

/* var */
static int varL_peek( lua_State *L );
static int varL_pop( lua_State *L );
static int varL_push( lua_State *L );
static const luaL_Reg var_methods[] = {
   { "peek", varL_peek },
   { "pop", varL_pop },
   { "push", varL_push },
   {0,0}
}; /**< Mission variable Lua methods. */

/**
 * @brief Loads the mission variable Lua library.
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadVar( nlua_env env )
{
   nlua_register(env, "var", var_methods, 0);
   return 0;
}

static int var_cmp( const void *p1, const void *p2 )
{
   const misn_var *mv1, *mv2;
   mv1 = (const misn_var*) p1;
   mv2 = (const misn_var*) p2;
   return strcmp(mv1->name,mv2->name);
}

/**
 * @brief Gets a mission var by name.
 */
static misn_var *var_get( const char *str )
{
   misn_var mv = {.name=(char*)str};
   if (var_stack == NULL)
      return NULL;
   return bsearch( &mv, var_stack, array_size(var_stack), sizeof(misn_var), var_cmp );
}

/**
 * @brief Saves the mission variables.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int var_save( xmlTextWriterPtr writer )
{
   xmlw_startElem(writer,"vars");

   for (int i=0; i<array_size(var_stack); i++) {
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
         case MISN_VAR_TIME:
            xmlw_attr(writer,"type","time");
            xmlw_str(writer,"%"TIME_PRI,var_stack[i].d.time);
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
   xmlNodePtr node = parent->xmlChildrenNode;

   var_cleanup();

   do {
      if (xml_isNode(node,"vars")) {
         xmlNodePtr cur = node->xmlChildrenNode;

         do {
            if (xml_isNode(cur,"var")) {
               misn_var var;
               char *str;
               xmlr_attr_strd(cur,"name",var.name);
               xmlr_attr_strd(cur,"type",str);
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
               else if (strcmp(str,"time")==0) {
                  var.type = MISN_VAR_TIME;
                  var.d.time = xml_getLong(cur);
               }
               else { /* super error checking */
                  WARN(_("Unknown var type '%s'"), str);
                  free(var.name);
                  free(str);
                  continue;
               }
               free(str);
               var_add( &var, 0 );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
   qsort( var_stack, array_size(var_stack), sizeof(misn_var), var_cmp );

   return 0;
}

/**
 * @brief Adds a var to the stack, strings will be SHARED, don't free.
 *
 *    @param new_var Variable to add.
 *    @param sort Whether or not to sort.
 *    @return 0 on success.
 */
static int var_add( misn_var *new_var, int sort )
{
   misn_var *mv;

   if (var_stack==NULL)
      var_stack = array_create( misn_var );

   /* check if already exists */
   mv = var_get( new_var->name );
   if (mv != NULL) {
      var_free( mv );
      *mv = *new_var;
      return 0;
   }

   /* need new one. */
   mv = &array_grow( &var_stack );
   *mv = *new_var;

   /* Sort if necessary. */
   if (sort)
      qsort( var_stack, array_size(var_stack), sizeof(misn_var), var_cmp );

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
int var_checkflag( const char* str )
{
   return var_get( str ) != NULL;
}

/**
 * @brief Gets the mission variable value of a certain name.
 *
 *    @luatparam string name Name of the mission variable to get.
 *    @luareturn The value of the mission variable which will depend on what type
 *             it is.
 * @luafunc peek
 */
static int varL_peek( lua_State *L )
{
   const char *str = luaL_checkstring(L,1);
   misn_var *mv = var_get( str );
   if (mv == NULL)
      return 0;

   switch (mv->type) {
      case MISN_VAR_NIL:
         lua_pushnil(L);
         break;
      case MISN_VAR_NUM:
         lua_pushnumber(L,mv->d.num);
         break;
      case MISN_VAR_BOOL:
         lua_pushboolean(L,mv->d.b);
         break;
      case MISN_VAR_STR:
         lua_pushstring(L,mv->d.str);
         break;
      case MISN_VAR_TIME:
         lua_pushtime(L,mv->d.time);
         break;
   }
   return 1;
}
/**
 * @brief Pops a mission variable off the stack, destroying it.
 *
 * This does not give you any value and destroys it permanently (or until recreated).
 *
 *    @luatparam string name Name of the mission variable to pop.
 * @luafunc pop
 */
static int varL_pop( lua_State *L )
{
   NLUA_CHECKRW(L);
   const char* str = luaL_checkstring(L,1);
   misn_var *mv = var_get( str );
   if (mv == NULL)
      return 0;
   var_free( mv );
   array_erase( &var_stack, mv, mv+1 );
   return 0;
}
/**
 * @brief Creates a new mission variable.
 *
 * This will overwrite existing vars, so it's a good way to update the values
 *  of different mission variables.
 *
 *    @luatparam string name Name to use for the new mission variable.
 *    @luaparam value Value of the new mission variable.  Accepted types are:
 *                  nil, bool, string or number.
 * @luafunc push
 */
static int varL_push( lua_State *L )
{
   NLUA_CHECKRW(L);
   const char *str = luaL_checkstring(L,1);
   misn_var var;

   /* store appropriate data */
   if (lua_isnil(L,2))
      var.type = MISN_VAR_NIL;
   else if (lua_istime(L,2)) {
      var.type = MISN_VAR_TIME;
      var.d.time = luaL_validtime(L,2);
   }
   else if (lua_type(L,2) == LUA_TNUMBER) {
      var.type = MISN_VAR_NUM;
      var.d.num = (double) lua_tonumber(L,2);
   }
   else if (lua_isboolean(L,2)) {
      var.type = MISN_VAR_BOOL;
      var.d.b = lua_toboolean(L,2);
   }
   else if (lua_type(L,2) == LUA_TSTRING) {
      var.type = MISN_VAR_STR;
      var.d.str = strdup( lua_tostring(L,2) );
   }
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }
   /* Set name. */
   var.name = strdup(str);
   var_add( &var, 1 );

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
         free(var->d.str);
         var->d.str = NULL;
         break;
      case MISN_VAR_NIL:
      case MISN_VAR_NUM:
      case MISN_VAR_BOOL:
      case MISN_VAR_TIME:
         break;
   }
   free(var->name);
   var->name = NULL;
}
/**
 * @brief Cleans up all the mission variables.
 */
void var_cleanup (void)
{
   for (int i=0; i<array_size(var_stack); i++)
      var_free( &var_stack[i] );

   array_free( var_stack );
   var_stack   = NULL;
}
