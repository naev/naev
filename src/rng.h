/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef RNG_H
#  define RNG_H


/**
 * @def RNG(L,H)
 *
 * @brief Gets a random number between L and H (L <= RNG <= H).
 *
 * If L is bigger then H it inverts the roles.
 */
#define RNG(L,H)  (((L)>(H)) ? RNG_SANE((H),(L)) : RNG_SANE((L),(H))) /* L <= RNG <= H */
/**
 * @def RNG_SANE(L,H)
 *
 * @brief Gets a number between L and H (L <= RNG <= H).
 *
 * Result unspecified in L is bigger then H>
 */
#define RNG_SANE(L,H) ((int)L + (int)((double)(H-L+1) * randfp())) /* L <= RNG <= H */
/**
 * @def RNGF()
 *
 * @brief Gets a random float between 0 and 1 (0. <= RNGF <= 1.).
 */
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
