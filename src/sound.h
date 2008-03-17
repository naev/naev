/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SOUND_H
#  define SOUND_H


#include <AL/al.h>

#include "physics.h"

#define VOICE_LOOPING      (1<<10) /* voice loops */
#define VOICE_STATIC       (1<<11) /* voice isn't relative */


struct alVoice;
typedef struct alVoice alVoice;

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
void sound_volume( const double vol );


/*
 * voice manipulation function
 */
alVoice* sound_addVoice( int priority, double px, double py, /* new voice */
      double vx, double vy, const ALuint buffer, const unsigned int flags );
void sound_delVoice( alVoice* voice ); /* delete a voice */
void voice_update( alVoice* voice, double px, double py, 
      double vx, double vy );
void voice_buffer( alVoice* voice, const ALuint buffer, const unsigned int flags );
void voice_stop( alVoice* voice );


/*
 * listener manipulation functions
 */
void sound_listener( double dir, double px, double py,
      double vx, double vy );


#endif /* SOUND_H */
