/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "outfit.h"

/*
 * stack manipulation
 */
int         dtype_get( const char *name );
const char *dtype_damageTypeToStr( int type );

/*
 * dtype effect loading and freeing
 */
int  dtype_load( void );
void dtype_free( void );

/*
 * misc
 */
int  dtype_raw( int type, double *shield, double *armour, double *knockback );
void dtype_calcDamage( double *dshield, double *darmour, double absorb,
                       double *knockback, const Damage *dmg,
                       const ShipStats *s );
