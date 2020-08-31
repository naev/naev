/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef RNG_H
#  define RNG_H


/**
 * @brief Gets a random number between L and H (L <= RNG <= H).
 *
 * If L is bigger then H it inverts the roles.
 */
#define RNG(L,H)  (((L)>(H)) ? RNG_BASE((H),(L)) : RNG_BASE((L),(H))) /* L <= RNG <= H */
/**
 * @brief Gets a number between L and H (L <= RNG <= H).
 *
 * Result unspecified in L is bigger then H>
 */
#define RNG_BASE(L,H) ((int)L + (int)((double)(H-L+1) * randfp())) /* L <= RNG <= H */
/**
 * @brief Gets a random float between 0 and 1 (0. <= RNGF <= 1.).
 */
#define RNGF()    (randfp()) /* 0. <= RNGF <= 1. */
/**
 * @brief Gets a random mu within one-sigma (-1 to 1).
 *
 * 63% Confidence interval.
 */
#define RNG_1SIGMA()       NormalInverse(0.158655255 + RNGF()*(1.-0.158655255*2.))
/**
 * @brief Gets a random mu within two-sigma (-2 to 2).
 *
 * 95% Confidence interval.
 */
#define RNG_2SIGMA()       NormalInverse(0.022750132 + RNGF()*(1.-0.022750132*2.))
/**
 * @brief Gets a random mu within three-sigma (-3 to 3).
 *
 * 99.8% Confidence interval.
 */
#define RNG_3SIGMA()       NormalInverse(0.0013498985 + RNGF()*(1.-0.0013498985*2.))


/* Init */
void rng_init (void);

/* Random functions */
unsigned int randint (void);
double randfp (void);

/* Probability functions */
double Normal( double x );
double NormalInverse( double p );


#endif /* RNG_H */
