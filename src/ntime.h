/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NTIME_H
#  define NTIME_H


#include <stdint.h>


#define NT_CYCLE_PERIODS   (5000)      /**< periods in a cycle */
#define NT_PERIOD_SECONDS   (10000)     /**< seconds in a period */


typedef int64_t ntime_t;         /**< Core time type. */

/* Create. */
ntime_t ntime_create( int scu, int stp, int stu );

/* update */
void ntime_update( double dt );

/* get */
ntime_t ntime_get (void);
void ntime_getR( int *cycles, int *periods, int *seconds, double *rem );
int ntime_getCycles( ntime_t t );
int ntime_getPeriods( ntime_t t );
int ntime_getSeconds( ntime_t t );
double ntime_convertSeconds( ntime_t t );
double ntime_getRemainder( ntime_t t );
char* ntime_pretty( ntime_t t, int d );
void ntime_prettyBuf( char *str, int max, ntime_t t, int d );

/* set */
void ntime_set( ntime_t t );
void ntime_setR( int cycles, int periods, int seconds, double rem );
void ntime_inc( ntime_t t );
void ntime_incLagged( ntime_t t );

/* misc */
void ntime_refresh (void);
void ntime_allowUpdate( int enable );


#endif /* NTIME_H */
