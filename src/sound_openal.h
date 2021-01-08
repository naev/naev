/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef SOUND_OPENAL_H
#  define SOUND_OPENAL_H


/** @cond */
#include <vorbis/vorbisfile.h>
#include "al.h"
/** @endcond */

#include "ncompat.h"
#include "nopenal.h"
#include "sound.h"


/**
 * @struct alSound
 *
 * @brief Contains a sound buffer.
 */
typedef struct alSound_ {
   char *filename; /**< Name of the file loaded from. */
   char *name; /**< Buffer's name. */
   double length; /**< Length of the buffer. */
   ALuint buf; /**< Buffer data. */
} alSound;


/**
 * @typedef voice_state_t
 * @brief The state of a voice.
 * @sa alVoice
 */
typedef enum voice_state_ {
   VOICE_STOPPED, /**< Voice is stopped. */
   VOICE_PLAYING, /**< Voice is playing. */
   VOICE_FADEOUT, /**< Voice is fading out. */
   VOICE_DESTROY  /**< Voice should get destroyed asap. */
} voice_state_t;


/**
 * @struct alVoice
 *
 * @brief Represents a voice in the game.
 *
 * A voice would be any object that is creating sound.
 */
typedef struct alVoice_ {
   struct alVoice_ *prev; /**< Linked list previous member. */
   struct alVoice_ *next; /**< Linked list next member. */

   int id; /**< Identifier of the voice. */

   voice_state_t state; /**< Current state of the sound. */
   unsigned int flags; /**< Voice flags. */

   ALfloat pos[3]; /**< Position of the voice. */
   ALfloat vel[3]; /**< Velocity of the voice. */
   ALuint source; /**< Source current in use. */
   ALuint buffer; /**< Buffer attached to the voice. */
} alVoice;


/*
 * Sound list.
 */
extern alVoice *voice_active; /**< Active voices. */


/*
 * Voice management.
 */
void voice_lock (void);
void voice_unlock (void);
alVoice* voice_new (void);
int voice_add( alVoice* v );
alVoice* voice_get( int id );


/*
 * Vorbis stuff.
 */
extern ov_callbacks sound_al_ovcall;
extern ov_callbacks sound_al_ovcall_noclose;


/*
 * Context info.
 */
typedef struct alInfo_s {
   ALint efx; /**< Whether or not context has efx extension. */
   ALint efx_major; /**< EFX major version. */
   ALint efx_minor; /**< EFX minor version. */
   ALint efx_auxSends; /**< Number of auxiliary sends of the context. */
   /* Effect types. */
   ALint efx_reverb; /**< Reverb effect supported. */
   ALint efx_echo; /**< Echo effect supported. */
} alInfo_t;
extern alInfo_t al_info;


/*
 * Creation.
 */
int sound_al_init (void);
void sound_al_exit (void);


/*
 * Sound creation.
 */
int sound_al_load( alSound *snd, SDL_RWops *rw, const char *name );
void sound_al_free( alSound *snd );


/*
 * Sound settings.
 */
int sound_al_volume( const double vol );
double sound_al_getVolume (void);
double sound_al_getVolumeLog(void);


/*
 * Sound playing.
 */
int sound_al_play( alVoice *v, alSound *s );
int sound_al_playPos( alVoice *v, alSound *s,
      double px, double py, double vx, double vy );
int sound_al_updatePos( alVoice *v,
      double px, double py, double vx, double vy );
void sound_al_updateVoice( alVoice *v );

/*
 * Sound management.
 */
void sound_al_update (void);
void sound_al_stop( alVoice *v );
void sound_al_pause (void);
void sound_al_resume (void);
void sound_al_setSpeed( double s );
void sound_al_setSpeedVolume( double vol );


/*
 * Listener.
 */
int sound_al_updateListener( double dir, double px, double py,
      double vx, double vy );

/*
 * Groups.
 */
int sound_al_createGroup( int size );
int sound_al_playGroup( int group, alSound *s, int once );
void sound_al_stopGroup( int group );
void sound_al_pauseGroup( int group );
void sound_al_resumeGroup( int group );
void sound_al_speedGroup( int group, int enable );
void sound_al_volumeGroup( int group, double volume );


/*
 * Env.
 */
int sound_al_env( SoundEnv_t env, double param );


#endif /* SOUND_OPENAL_H */
