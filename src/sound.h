

#ifndef SOUND_H
#  define SOUND_H


#include <AL/al.h>

#include "physics.h"


#define SOUND_REFERENCE_DIST	500.
#define SOUND_MAX_DIST			1000.


/*
 * virtual voice
 */
typedef struct {
	ALuint source; /* source itself, 0 if not set */
	ALuint buffer; /* buffer */

	int priority; /* base priority */

	double px, py; /* position */
	double vx, vy; /* velocity */

	unsigned int start; /* time started in ms */
	unsigned int flags; /* flags to set properties */
} alVoice;



/*
 * sound subsystem
 */
int sound_init (void);
void sound_exit (void);
void sound_update (void);


/*
 * sound manipulation functions
 */
ALuint sound_get( char* name );


/*
 * voice manipulation function
 */
alVoice* sound_addVoice( int priority, double px, double py,
		double vx, double vy, const ALuint buffer, const int looping );
void sound_delVoice( alVoice* voice );
void voice_update( alVoice* voice, double px, double py, 
		double vx, double vy );
void sound_listener( double dir, double px, double py,
		double vx, double vy );


#endif /* SOUND_H */
