/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef RNG_H
#  define RNG_H


#define RNG(L,H)  (((L)>(H)) ? RNG_SANE((H),(L)) : RNG_SANE((L),(H)))
#define RNG_SANE(L,H) ((int)L + (int)((double)(H-L+1) * randfp())) /* L <= RNG <= H */
#define RNGF()    (randfp()) /* 0. <= RNGF <= 1. */


/* Init */
void rng_init (void);

/* Random functions */
unsigned int randint (void);
double randfp (void);

/* Probability functions */
double Normal( double x );
double NormalInverse( double p );


#endif /* RNG_H */
