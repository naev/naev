/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SOUND_SDLMIX_H
#  define SOUND_SDLMIX_H


#include "sound_priv.h"


/*
 * Creation.
 */
int sound_mix_init (void);
void sound_mix_exit (void);
void sound_mix_update (void);


/*
 * Sound creation.
 */
int sound_mix_load( alSound *snd, const char *filename );
void sound_mix_free( alSound *snd );


/*
 * Sound settings.
 */
int sound_mix_volume( const double vol );
double sound_mix_getVolume (void);


/*
 * Sound playing.
 */
int sound_mix_play( alVoice *v, alSound *s );
int sound_mix_playPos( alVoice *v, alSound *s,
      double px, double py, double vx, double vy );
int sound_mix_updatePos( alVoice *v,
      double px, double py, double vx, double vy );
void sound_mix_updateVoice( alVoice *v );

/*
 * Sound management.
 */
void sound_mix_stop( alVoice *v );
void sound_mix_pause (void);
void sound_mix_resume (void);


/*
 * Listener.
 */
int sound_mix_updateListener( double dir, double px, double py,
      double vx, double vy );

/*
 * Groups.
 */
int sound_mix_createGroup( int tag, int size );
int sound_mix_playGroup( int tag, alSound *s, int once );
void sound_mix_stopGroup( int group );


#endif /* SOUND_SDLMIX_H */
