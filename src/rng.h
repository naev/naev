/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef RNG_H
#  define RNG_H


#include <stdlib.h>


#define RNG(L,H)  ((int)L + (int)((double)(H-L+1) * (rand()/(RAND_MAX+1.))))
#define RNGF()		(rand()/(RAND_MAX+1.))


void rng_init (void);


#endif /* RNG_H */
