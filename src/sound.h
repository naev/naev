/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "AL/al.h"
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_mutex.h>
/** @endcond */

int  sound_disabled( void );
void sound_set_disabled( int disabled );

/*
 * Environmental features.
 */
typedef enum SoundEnv_e {
   SOUND_ENV_NORMAL, /**< Normal space. */
   SOUND_ENV_NEBULA  /**< Nebula space. */
} SoundEnv_t;        /**< Type of environment. */

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
void                 sound_ingameGroup( const Group *group );

/*
 * Environmental functions.
 */
void sound_setAbsorption( double value );
void sound_env( SoundEnv_t env, double param );

/* Lock for OpenAL operations. */
int sound_al_buffer( ALuint *buf, SDL_IOStream *rw, const char *name );
#define soundLock() SDL_LockMutex( sound_lock )
#define soundUnlock() SDL_UnlockMutex( sound_lock )
