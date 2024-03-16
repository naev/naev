/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "shipstats.h"

typedef struct Difficulty_ {
   char *name;        /**< Name of the difficulty. */
   char *description; /**< Description of the difficulty. */
   ShipStatList
      *stats; /**< Modifications done to the player on the difficulty. */
   int def;   /**< Whether or not the default difficulty. */
} Difficulty;

int  difficulty_load( void );
void difficulty_free( void );

const Difficulty *difficulty_cur( void );
const Difficulty *difficulty_getAll( void );
const Difficulty *difficulty_get( const char *name );
void              difficulty_setGlobal( const Difficulty *d );
void              difficulty_setLocal( const Difficulty *d );

char *difficulty_display( const Difficulty *d );

int difficulty_apply( ShipStats *s );
