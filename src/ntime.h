/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NTIME_H
#  define NTIME_H


#include <stdint.h>


typedef int64_t ntime_t;

/* Create. */
ntime_t ntime_create( int scu, int stp, int stu );

/* update */
void ntime_update( double dt );

/* get */
ntime_t ntime_get (void);
int ntime_getSCU( ntime_t t );
int ntime_getSTP( ntime_t t );
int ntime_getSTU( ntime_t t );
char* ntime_pretty( ntime_t t, int d );
void ntime_prettyBuf( char *str, int max, ntime_t t, int d );

/* set */
void ntime_set( ntime_t t );
void ntime_inc( ntime_t t );
void ntime_incLagged( ntime_t t );

/* misc */
void ntime_refresh (void);
void ntime_allowUpdate( int enable );


#endif /* NTIME_H */
