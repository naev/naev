/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SOUND_OPENAL_H
#  define SOUND_OPENAL_H


#if USE_OPENAL


#include "sound_priv.h"

#include "ncompat.h"

#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>
#include <vorbis/vorbisfile.h>


/*
 * Vorbis stuff.
 */
extern ov_callbacks sound_al_ovcall;


/*
 * OpenAL stuff.
 */
#if DEBUG == 1
void al_checkErr (void);
#else /* DEBUG */
#define al_checkErr() /**< Hack to ignore errors when debugging. */
#endif /* DEBUG */
#if HAS_BIGENDIAN
#  define VORBIS_ENDIAN    1
#else /* HAS_BIGENDIAN */
#  define VORBIS_ENDIAN    0
#endif /* HAS_BIGENDIAN */

/*
 * EFX stuff.
 */
#ifndef ALC_EXT_EFX
#define AL_FILTER_TYPE                                     0x8001
#define AL_EFFECT_TYPE                                     0x8001
#define AL_FILTER_NULL                                     0x0000
#define AL_FILTER_LOWPASS                                  0x0001
#define AL_FILTER_HIGHPASS                                 0x0002
#define AL_FILTER_BANDPASS                                 0x0003
#define AL_EFFECT_NULL                                     0x0000
#define AL_EFFECT_EAXREVERB                                0x8000
#define AL_EFFECT_REVERB                                   0x0001
#define AL_EFFECT_CHORUS                                   0x0002
#define AL_EFFECT_DISTORTION                               0x0003
#define AL_EFFECT_ECHO                                     0x0004
#define AL_EFFECT_FLANGER                                  0x0005
#define AL_EFFECT_FREQUENCY_SHIFTER                        0x0006
#define AL_EFFECT_VOCAL_MORPHER                            0x0007
#define AL_EFFECT_PITCH_SHIFTER                            0x0008
#define AL_EFFECT_RING_MODULATOR                           0x0009
#define AL_EFFECT_AUTOWAH                                  0x000A
#define AL_EFFECT_COMPRESSOR                               0x000B
#define AL_EFFECT_EQUALIZER                                0x000C
#define ALC_EFX_MAJOR_VERSION                              0x20001
#define ALC_EFX_MINOR_VERSION                              0x20002
#define ALC_MAX_AUXILIARY_SENDS                            0x20003
#endif
ALvoid (AL_APIENTRY *nalGenFilters)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalDeleteFilters)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalFilteri)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *nalGenEffects)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalDeleteEffects)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *nalEffecti)(ALuint,ALenum,ALint);


/*
 * Context info.
 */
typedef struct alInfo_s {
   ALint efx; /**< Whether or not context has efx extension. */
   ALint efx_major; /**< EFX major version. */
   ALint efx_minor; /**< EFX minor version. */
   ALint efx_auxSends; /**< Number of auxiliary sends of the context. */
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


#endif /* USE_OPENAL */


#endif /* SOUND_OPENAL_H */
