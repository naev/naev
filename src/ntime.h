/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NTIME_H
#  define NTIME_H


/* how long a "unit" of time is */
#define NTIME_UNIT_LENGTH      1000 /**< Length of a MTU (Major Time Unit) */

/* get */
unsigned int ntime_get (void);
char* ntime_pretty( unsigned int t );

/* set */
void ntime_set( unsigned int t );
void ntime_inc( unsigned int t );


#endif /* NTIME_H */
