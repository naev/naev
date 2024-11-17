/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "ntime.h"
#include "nxml.h"

/* similar to Lua vars, but with less variety */
typedef enum lvar_type_ {
   LVAR_NIL,  /**< Nil type. */
   LVAR_NUM,  /**< Number type. */
   LVAR_BOOL, /**< Boolean type. */
   LVAR_STR,  /**< String type. */
   LVAR_TIME, /**< Time type. */
} lvar_type;

/**
 * @struct lvar
 *
 * @brief Contains a mission variable.
 */
typedef struct lvar_ {
   char     *name; /**< Name of the variable. */
   lvar_type type; /**< Type of the variable. */
   union {
      double  num;  /**< Used if type is number. */
      char   *str;  /**< Used if type is string. */
      int     b;    /**< Used if type is boolean. */
      ntime_t time; /**< Used if type is time. */
   } d;             /**< Variable data. */
} lvar;

/*
 * Creating and stuff.
 */
int   lvar_addArray( lvar **arr, const lvar *new_var, int sort );
void  lvar_rmArray( lvar **arr, lvar *rm_var );
void  lvar_freeArray( lvar *var );
lvar *lvar_get( const lvar *arr, const char *str );

/*
 * Lua stuff.
 */
int  lvar_push( lua_State *L, const lvar *v );
lvar lvar_tovar( lua_State *L, const char *name, int idx );

/*
 * XML save/load.
 */
int   lvar_save( const lvar *arr, xmlTextWriterPtr writer );
lvar *lvar_load( xmlNodePtr parent );
