/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <lua.h>
/** @endcond */

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
int music_play( const char *filename );
int music_stop( int disable );
int music_pause( int disable );
int music_resume (void);
MusicInfo_t* music_info (void);

/*
 * Lua control
 */
int music_choose( const char* situation );
void music_rechoose (void);
void music_tempDisable( int disable );
