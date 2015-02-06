/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef _DTYPE_H
#  define _DTYPE_H


#include "outfit.h"

/*
 * stack manipulation
 */
int dtype_get( char* name ); 
char* dtype_damageTypeToStr( int type ); 

/*
 * dtype effect loading and freeing
 */
int dtype_load (void);
void dtype_free (void);

/*
 * misc
 */
void dtype_calcDamage( double *dshield, double *darmour, double absorb,
      double *knockback, const Damage *dmg, ShipStats *s );


#endif /* _DTYPE_H */

