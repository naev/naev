/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SOUND_H
#  define SOUND_H


extern int sound_disabled;

/*
 * sound subsystem
 */
int sound_init (void);
void sound_exit (void);


/*
 * sound manipulation functions
 */
int sound_get( char* name );
int sound_volume( const double vol );
int sound_play( int sound );
int sound_playPos( int sound, double x, double y );
int sound_updateListener( double dir, double x, double y );

/*
 * Group functions.
 */
int sound_reserve( int num );
int sound_createGroup( int tag, int start, int size );
int sound_playGroup( int group, int sound, int once );
void sound_stopGroup( int group );


#endif /* SOUND_H */
