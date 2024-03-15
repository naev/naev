/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file lvar.c
 *
 * @brief Lua Variables
 */
#include "lvar.h"

#include "array.h"
#include "nlua_time.h"
#include "nluadef.h"

/*
 * prototypes
 */
static int  lvar_cmp( const void *p1, const void *p2 );
static void lvar_free( lvar *var );

/**
 * @brief Compares two lua variable names. For use with qsort/bsearch.
 */
static int lvar_cmp( const void *p1, const void *p2 )
{
   const lvar *mv1, *mv2;
   mv1 = (const lvar *)p1;
   mv2 = (const lvar *)p2;
   return strcmp( mv1->name, mv2->name );
}

/**
 * @brief Gets a lua var by name.
 *
 *    @param arr Array to search in.
 *    @param str Name to use as a key.
 *    @return Found element or NULL if not found.
 */
lvar *lvar_get( const lvar *arr, const char *str )
{
   const lvar mv = { .name = (char *)str };
   if ( arr == NULL )
      return NULL;
   return bsearch( &mv, arr, array_size( arr ), sizeof( lvar ), lvar_cmp );
}

/**
 * @brief Pushes a lua var to a lua state.
 *
 *    @param L Lua state to push to.
 *    @param v Lua variable to push.
 *    @return Number of elements pushed onto the stack.
 */
int lvar_push( lua_State *L, const lvar *v )
{
   switch ( v->type ) {
   case LVAR_NIL:
      lua_pushnil( L );
      break;
   case LVAR_NUM:
      lua_pushnumber( L, v->d.num );
      break;
   case LVAR_BOOL:
      lua_pushboolean( L, v->d.b );
      break;
   case LVAR_STR:
      lua_pushstring( L, v->d.str );
      break;
   case LVAR_TIME:
      lua_pushtime( L, v->d.time );
      break;
   }
   return 1;
}

/**
 * @brief Gets a lua variable from an index from a lua state.
 *
 *    @param L Lua state to get var from.
 *    @param name Name of the variable.
 *    @param idx Index to get value from.
 *    @return New lua variable that should be freed with lvar_free().
 */
lvar lvar_tovar( lua_State *L, const char *name, int idx )
{
   lvar var;

   /* Store appropriate data */
   if ( lua_isnil( L, idx ) )
      var.type = LVAR_NIL;
   else if ( lua_istime( L, idx ) ) {
      var.type   = LVAR_TIME;
      var.d.time = luaL_validtime( L, idx );
   } else if ( lua_type( L, idx ) == LUA_TNUMBER ) {
      var.type  = LVAR_NUM;
      var.d.num = (double)lua_tonumber( L, idx );
   } else if ( lua_isboolean( L, idx ) ) {
      var.type = LVAR_BOOL;
      var.d.b  = lua_toboolean( L, idx );
   } else if ( lua_type( L, idx ) == LUA_TSTRING ) {
      var.type  = LVAR_STR;
      var.d.str = strdup( lua_tostring( L, idx ) );
   } else {
      /* Hack because we don't want to return 0 and can't use
       * NLUA_INVALID_PARAMETER */
      DEBUG( "Invalid parameter for %s.", __func__ );
      luaL_error( L, "Invalid parameter for %s.", __func__ );
      var.type = LVAR_NIL;
      return var;
   }
   /* Set name. */
   var.name = strdup( name );
   return var;
}

/**
 * @brief Frees a lua variable.
 *
 *    @param var Lua variable to free.
 */
static void lvar_free( lvar *var )
{
   switch ( var->type ) {
   case LVAR_STR:
      free( var->d.str );
      var->d.str = NULL;
      break;
   case LVAR_NIL:
   case LVAR_NUM:
   case LVAR_BOOL:
   case LVAR_TIME:
      break;
   }
   free( var->name );
   var->name = NULL;
}

/**
 * @brief Frees a variable array.
 *
 *    @param arr Array to free.
 */
void lvar_freeArray( lvar *arr )
{
   for ( int i = 0; i < array_size( arr ); i++ )
      lvar_free( &arr[i] );
   array_free( arr );
}

/**
 * @brief Adds a var to a var array.
 *
 *    @param arr Array to add var to (should be already initialized).
 *    @param new_var New variable to add to array.
 *    @param sort Whether or not to sort.
 *    @return 0 on success.
 */
int lvar_addArray( lvar **arr, const lvar *new_var, int sort )
{
   /* Avoid Duplicates. */
   lvar *mv = lvar_get( *arr, new_var->name );
   if ( mv != NULL ) {
      lvar_free( mv );
      *mv = *new_var;
      return 0;
   }

   /* need new one. */
   mv  = &array_grow( arr );
   *mv = *new_var;

   /* Sort if necessary. */
   if ( sort )
      qsort( *arr, array_size( *arr ), sizeof( lvar ), lvar_cmp );

   return 0;
}

/**
 * @brief Removes a var from a var array.
 *
 *    @param arr Array to remove var from.
 *    @param rm_var Var to remove.
 */
void lvar_rmArray( lvar **arr, lvar *rm_var )
{
   lvar_free( rm_var );
   array_erase( arr, rm_var, rm_var + 1 );
}

/**
 * @brief Saves the mission variables.
 *
 *    @param arr Array to save.
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int lvar_save( const lvar *arr, xmlTextWriterPtr writer )
{
   for ( int i = 0; i < array_size( arr ); i++ ) {
      const lvar *v = &arr[i];
      xmlw_startElem( writer, "var" );

      xmlw_attr( writer, "name", "%s", v->name );

      switch ( v->type ) {
      case LVAR_NIL:
         xmlw_attr( writer, "type", "nil" );
         break;
      case LVAR_NUM:
         xmlw_attr( writer, "type", "num" );
         xmlw_str( writer, "%f", v->d.num );
         break;
      case LVAR_BOOL:
         xmlw_attr( writer, "type", "bool" );
         xmlw_str( writer, "%d", v->d.b );
         break;
      case LVAR_STR:
         xmlw_attr( writer, "type", "str" );
         xmlw_str( writer, "%s", v->d.str );
         break;
      case LVAR_TIME:
         xmlw_attr( writer, "type", "time" );
         xmlw_str( writer, "%" TIME_PRI, v->d.time );
         break;
      }
      xmlw_endElem( writer ); /* "var" */
   }

   return 0;
}

/**
 * @brief Loads the vars from XML file.
 *
 *    @param parent Parent node containing the variables.
 *    @return Newly allocated lua variable array or NULL on error.
 */
lvar *lvar_load( xmlNodePtr parent )
{
   lvar      *arr  = array_create( lvar );
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes( node );
      if ( !xml_isNode( node, "var" ) ) {
         WARN( _( "Lua Var stack has unknown node '%s'!" ), xml_get( node ) );
         continue;
      }
      lvar  var;
      char *str;
      xmlr_attr_strd( node, "name", var.name );
      xmlr_attr_strd( node, "type", str );
      if ( strcmp( str, "nil" ) == 0 )
         var.type = LVAR_NIL;
      else if ( strcmp( str, "num" ) == 0 ) {
         var.type  = LVAR_NUM;
         var.d.num = xml_getFloat( node );
      } else if ( strcmp( str, "bool" ) == 0 ) {
         var.type = LVAR_BOOL;
         var.d.b  = xml_getInt( node );
      } else if ( strcmp( str, "str" ) == 0 ) {
         var.type  = LVAR_STR;
         var.d.str = xml_getStrd( node );
      } else if ( strcmp( str, "time" ) == 0 ) {
         var.type   = LVAR_TIME;
         var.d.time = xml_getLong( node );
      } else { /* super error checking */
         WARN( _( "Unknown var type '%s'" ), str );
         free( var.name );
         free( str );
         continue;
      }
      free( str );
      lvar_addArray( &arr, &var, 0 );
   } while ( xml_nextNode( node ) );
   qsort( arr, array_size( arr ), sizeof( lvar ), lvar_cmp );

   return arr;
}
