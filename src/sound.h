/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "nopenal.h"
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_mutex.h>
#include <vorbis/vorbisfile.h>
/** @endcond */

/*
 * Some OpenAL extension defines.
 */
#ifndef ALC_OUTPUT_LIMITER_SOFT
#define ALC_OUTPUT_LIMITER_SOFT 0x199A
#endif /* ALC_OUTPUT_LIMITER_SOFT */

int  sound_disabled( void );
void sound_set_disabled( int disabled );

#define SOUND_REFERENCE_DISTANCE 500. /**< OpenAL reference distance. */
#define SOUND_MAX_DISTANCE 25e3       /**< OpenAL max distance. */

/*
 * Static configuration.
 */
#define SOUND_PILOT_RELATIVE                                                   \
   1 /**< Whether the sound is relative to the pilot (as opposed to the        \
        camera). */

/*
 * Environmental features.
 */
typedef enum SoundEnv_e {
   SOUND_ENV_NORMAL, /**< Normal space. */
   SOUND_ENV_NEBULA  /**< Nebula space. */
} SoundEnv_t;        /**< Type of environment. */

typedef struct alInfo_s {
   ALCint freq;           /**< Store the frequency. */
   ALCint nmono;          /**< Number of mono sources. */
   ALCint nstereo;        /**< Number of stereo sources. */
   ALint  output_limiter; /**< Whether or not context has output limiter. */
   ALint  efx;            /**< Whether or not context has efx extension. */
   ALint  efx_major;      /**< EFX major version. */
   ALint  efx_minor;      /**< EFX minor version. */
   ALint  efx_auxSends;   /**< Number of auxiliary sends of the context. */
   /* Effect types. */
   ALint efx_reverb; /**< Reverb effect supported. */
   ALint efx_echo;   /**< Echo effect supported. */
} alInfo_t;
extern alInfo_t al_info;

extern ALuint sound_efx_directSlot; /**< Direct 3d source slot. */

/*
 * Vorbis stuff.
 */
extern ov_callbacks sound_al_ovcall;
extern ov_callbacks sound_al_ovcall_noclose;

/*
 * sound subsystem
 */
int    sound_update( double dt );
void   sound_pause( void );
void   sound_resume( void );
int    sound_volume( const double vol );
double sound_getVolume( void );
double sound_getVolumeLog( void );
void   sound_stopAll( void );
void   sound_setSpeed( double s );

/*
 * source management
 */
int source_newRW( SDL_IOStream *rw, const char *name, unsigned int flags );
int source_new( const char *filename, unsigned int flags );

/*
 * sound sample management
 */
typedef struct Sound Sound;
const Sound         *sound_get( const char *name );
double               sound_getLength( const Sound *sound );

/*
 * voice management
 */
typedef struct Voice Voice;
const Voice         *sound_play( const Sound *sound );
const Voice *sound_playPos( const Sound *sound, double px, double py, double vx,
                            double vy );
void         sound_stop( const Voice *voice );
int sound_updatePos( const Voice *voice, double px, double py, double vx,
                     double vy );
int sound_updateListener( double dir, double px, double py, double vx,
                          double vy );

/*
 * Group functions.
 */
typedef struct Group Group;
const Group         *sound_createGroup( int size );
const Voice         *sound_playGroup( const Group *group, const Sound *sound,
                                      int once );
void                 sound_stopGroup( const Group *group );
void                 sound_pauseGroup( const Group *group );
void                 sound_resumeGroup( const Group *group );
void                 sound_speedGroup( const Group *group, int enable );
void                 sound_volumeGroup( const Group *group, double volume );
void                 sound_pitchGroup( const Group *group, double pitch );

/*
 * Environmental functions.
 */
void sound_setAbsorption( double value );
void sound_env( SoundEnv_t env, double param );

/*
 * Vorbis filtering.
 */
#define RG_PREAMP_DB 0.0 /**< Default pre-amp in dB. */
typedef struct rg_filter_param_s {
   float rg_scale_factor;
   float rg_max_scale;
} rg_filter_param_t;
void rg_filter( float **pcm, long channels, long samples, void *filter_param );

/* Lock for OpenAL operations. */
int sound_al_buffer( ALuint *buf, SDL_IOStream *rw, const char *name );
extern SDL_Mutex
   *sound_lock; /**< Global sound lock, used for all OpenAL calls. */
#define soundLock() SDL_LockMutex( sound_lock )
#define soundUnlock() SDL_UnlockMutex( sound_lock )
