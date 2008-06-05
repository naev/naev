/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef RNG_H
#  define RNG_H


#define RNG(L,H)  ((int)L + (int)((double)(H-L+1) * randfp())) /* L <= RNG <= H */
#define RNGF()    (randfp()) /* 0. <= RNGF <= 1. */


void rng_init (void);
unsigned int randint (void);
double randfp (void);


#endif /* RNG_H */
