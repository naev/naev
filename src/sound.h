

#ifndef SOUND_H
#  define SOUND_H


#include <AL/al.h>

#include "physics.h"


#define SOUND_REFERENCE_DIST	500.
#define SOUND_MAX_DIST			1000.


/*
 * sound subsystem
 */
int sound_init (void);
void sound_exit (void);


/*
 * sound manipulation functions
 */
ALuint sound_get( char* name );


/*
 * source manipulation function
 */
ALuint sound_dynSource( double px, double py, double vx, double vy, int looping );


#endif /* SOUND_H */
