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
#include <stdio.h>
#include <stdlib.h>

/** @endcond */

#include "nlua_var.h"

#include "array.h"
#include "lvar.h"
#include "nxml.h"

/*
 * variable stack
 */
static lvar *var_stack = NULL; /**< Stack of mission variables. */
/* externed */

/* var */
static int varL_peek( lua_State *L );
static int varL_pop( lua_State *L );
static int varL_push( lua_State *L );

static const luaL_Reg var_methods[] = {
   { "peek", varL_peek },
   { "pop", varL_pop },
   { "push", varL_push },
   { 0, 0 } }; /**< Mission variable Lua methods. */

/**
 * @brief Loads the mission variable Lua library.
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadVar( nlua_env env )
{
   nlua_register( env, "var", var_methods, 0 );
   return 0;
}

/**
 * @brief Gets a mission var by name.
 */
static lvar *var_get( const char *str )
{
   return lvar_get( var_stack, str );
}

/**
 * @brief Saves the mission variables.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int var_save( xmlTextWriterPtr writer )
{
   xmlw_startElem( writer, "vars" );
   lvar_save( var_stack, writer );
   xmlw_endElem( writer ); /* "vars" */
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
      xml_onlyNodes( node );
      if ( !xml_isNode( node, "vars" ) )
         continue;
      var_stack = lvar_load( node );
   } while ( xml_nextNode( node ) );
   return 0;
}

/**
 * @brief Adds a var to the stack, strings will be SHARED, don't free.
 *
 *    @param new_var Variable to add.
 *    @param sort Whether or not to sort.
 *    @return 0 on success.
 */
static int var_add( lvar *new_var, int sort )
{
   if ( var_stack == NULL )
      var_stack = array_create( lvar );
   return lvar_addArray( &var_stack, new_var, sort );
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
int var_checkflag( const char *str )
{
   return var_get( str ) != NULL;
}

/**
 * @brief Gets the mission variable value of a certain name.
 *
 *    @luatparam string name Name of the mission variable to get.
 *    @luareturn The value of the mission variable which will depend on what
 * type it is.
 * @luafunc peek
 */
static int varL_peek( lua_State *L )
{
   const char *str = luaL_checkstring( L, 1 );
   lvar       *mv  = var_get( str );
   if ( mv == NULL )
      return 0;
   return lvar_push( L, mv );
}

/**
 * @brief Pops a mission variable off the stack, destroying it.
 *
 * This does not give you any value and destroys it permanently (or until
 * recreated).
 *
 *    @luatparam string name Name of the mission variable to pop.
 * @luafunc pop
 */
static int varL_pop( lua_State *L )
{
   const char *str = luaL_checkstring( L, 1 );
   lvar       *mv  = var_get( str );
   if ( mv == NULL )
      return 0;
   lvar_rmArray( &var_stack, mv );
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
   const char *str = luaL_checkstring( L, 1 );
   lvar        var = lvar_tovar( L, str, 2 );
   var_add( &var, 1 );
   return 0;
}

/**
 * @brief Cleans up all the mission variables.
 */
void var_cleanup( void )
{
   lvar_freeArray( var_stack );
   var_stack = NULL;
}
