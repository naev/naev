/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef RNG_H
#  define RNG_H


/**
 * @brief Gets a random number between L and H (L <= RNG <= H).
 *
 * If L is bigger then H it inverts the roles.
 */
#define RNG(L,H)  (((L)>(H)) ? RNG_SANE((H),(L)) : RNG_SANE((L),(H))) /* L <= RNG <= H */
/**
 * @brief Gets a number between L and H (L <= RNG <= H).
 *
 * Result unspecified in L is bigger then H>
 */
#define RNG_SANE(L,H) ((int)L + (int)((double)(H-L+1) * randfp())) /* L <= RNG <= H */
/**
 * @brief Gets a random float between 0 and 1 (0. <= RNGF <= 1.).
 */
#define RNGF()    (randfp()) /* 0. <= RNGF <= 1. */
/**
 * @brief Gets a random mu within one-sigma (-1 to 1).
 *
 * 63% Confidence interval.
 */
#define RNG_1SIGMA()       NormalInverse(0.158 + RNGF()*(1.-0.341*2.))
/**
 * @brief Gets a random mu within two-sigma (-2 to 2).
 *
 * 95% Confidence interval.
 */
#define RNG_2SIGMA()       NormalInverse(0.021 + RNGF()*(1.-0.021*2.))
/**
 * @brief Gets a random mu within three-sigma (-3 to 3).
 *
 * 99.8% Confidence interval.
 */
#define RNG_3SIGMA()       NormalInverse(0.001 + RNGF()*(1.-0.001*2.))


/* Init */
void rng_init (void);

/* Random functions */
unsigned int randint (void);
double randfp (void);

/* Probability functions */
double Normal( double x );
double NormalInverse( double p );


#endif /* RNG_H */
