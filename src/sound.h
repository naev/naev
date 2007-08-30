

#ifndef SOUND_H
#  define SOUND_H


#include <AL/al.h>

#include "physics.h"


/*
 * sound subsystem
 */
int sound_init (void);
void sound_exit (void);


/*
 * sound manipulation functions
 */
ALuint sound_sndCreate( char* filename );
void sound_sndFree( const ALuint snd );


/*
 * source manipulation function
 */
#define sound_initSource(s)		(alGenSources(1,&(s)))
#define sound_delSource(s)			(alDeleteSources(1,&(s))


#endif /* SOUND_H */
