/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "outfit.h"

#define FULL_PENETRATION 1000. /* Not sure if INFINITY would give issues. */

/*
 * stack manipulation
 */
int         dtype_get( const char *name );
const char *dtype_damageTypeToStr( int type );

/*
 * misc
 */
int  dtype_raw( int type, double *shield, double *armour );
void dtype_calcDamage( double *dshield, double *darmour, double absorb,
                       const Damage *dmg, const ShipStats *s );
