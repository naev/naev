/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef SOUND_OPENAL_H
#  define SOUND_OPENAL_H


#include "sound.h"
#include "sound_priv.h"

#include "ncompat.h"
#include "nopenal.h"

#include <vorbis/vorbisfile.h>


/*
 * Vorbis stuff.
 */
extern ov_callbacks sound_al_ovcall;
extern ov_callbacks sound_al_ovcall_noclose;


/*
 * OpenAL stuff.
 */
#ifdef DEBUGGING
#define al_checkErr()      al_checkHandleError( __func__ )
void al_checkHandleError( const char *func );
#else /* DEBUG */
#define al_checkErr() /**< Hack to ignore errors when debugging. */
#endif /* DEBUG */
#if HAS_BIGENDIAN
#  define VORBIS_ENDIAN    1
#else /* HAS_BIGENDIAN */
#  define VORBIS_ENDIAN    0
#endif /* HAS_BIGENDIAN */


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
int sound_al_load( alSound *snd, const char *filename );
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
