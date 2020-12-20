/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MUSIC_H
#  define MUSIC_H


#include <lua.h>


#define MUSIC_FADEOUT_DELAY   1000 /**< Time it takes to fade out. */
#define MUSIC_FADEIN_DELAY    2000 /**< Time it takes to fade in. */


extern int music_disabled;


/*
 * updating
 */
void music_update( double dt );


/*
 * init/exit
 */
int music_init (void);
void music_exit (void);


/*
 * music control
 */
int music_volume( const double vol );
double music_getVolume (void);
double music_getVolumeLog(void);
int music_load( const char* name );
void music_play (void);
void music_stop (void);
void music_pause (void);
void music_resume (void);
void music_setPos( double sec );
int music_isPlaying (void);
const char *music_playingName (void);
double music_playingTime (void);


/*
 * Lua control
 */
int music_choose( const char* situation );
int music_chooseDelay( const char* situation, double delay );
void music_rechoose (void);


#endif /* MUSIC_H */
