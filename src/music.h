/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <lua.h>
/** @endcond */

#define MUSIC_FADEOUT_DELAY   1000 /**< Time it takes to fade out. */
#define MUSIC_FADEIN_DELAY    2000 /**< Time it takes to fade in. */

extern int music_disabled;

typedef struct MusicInfo_e {
   int playing;
   char *name;
   double pos;
} MusicInfo_t;

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
int music_volume( double vol );
double music_getVolume (void);
double music_getVolumeLog(void);
int music_load( const char* name );
void music_play( const char *filename );
void music_stop (void);
void music_pause (void);
void music_resume (void);
void music_setPos( double sec );
MusicInfo_t* music_info (void);
int music_isPlaying (void);
const char *music_playingName (void);
double music_playingTime (void);

/*
 * Lua control
 */
int music_choose( const char* situation );
void music_rechoose (void);
void music_tempDisable( int disable );
void music_repeat( int repeat );
