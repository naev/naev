/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SOUND_H
#  define SOUND_H


#define SOUND_SPEED_PLAY_LIMIT      4.0 /**< Speed modifier at which sounds do not play. */


extern int sound_disabled;


/*
 * Environmental features.
 */
typedef enum SoundEnv_e {
   SOUND_ENV_NORMAL, /**< Normal space. */
   SOUND_ENV_NEBULA /**< Nebula space. */
} SoundEnv_t; /**< Type of environment. */


/*
 * sound subsystem
 */
int sound_init (void);
void sound_exit (void);
int sound_update( double dt );
void sound_pause (void);
void sound_resume (void);


/*
 * sound manipulation functions
 */
int sound_get( char* name );
double sound_length( int sound );
int sound_volume( const double vol );
double sound_getVolume (void);
double sound_getVolumeLog (void);
int sound_play( int sound );
int sound_playPos( int sound, double px, double py, double vx, double vy );
void sound_stop( int voice );
void sound_stopAll (void);
int sound_updatePos( int voice, double px, double py, double vx, double vy );
int sound_updateListener( double dir, double px, double py,
      double vx, double vy );
void sound_setSpeed( double s );


/*
 * Group functions.
 */
int sound_reserve( int num );
int sound_createGroup( int size );
int sound_playGroup( int group, int sound, int once );
void sound_stopGroup( int group );
void sound_pauseGroup( int group );
void sound_resumeGroup( int group );
void sound_speedGroup( int group, int enable );
void sound_volumeGroup( int group, double volume );


/*
 * Environmental functions.
 */
int sound_env( SoundEnv_t env, double param );


#endif /* SOUND_H */
