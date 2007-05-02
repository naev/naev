

#ifndef RNG_H
#  define RNG_H


#include <stdlib.h>


#define RNG(L,H)	(rand()%(int)(H-L+1)+L)


void rng_init (void);


#endif /* RNG_H */
