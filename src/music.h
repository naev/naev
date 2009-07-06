/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MUSIC_H
#  define MUSIC_H


#include "lua.h"


extern int music_disabled;


/*
 * updating
 */
void music_update (void);


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
int music_load( const char* name );
void music_play (void);
void music_stop (void);
void music_pause (void);
void music_resume (void);
void music_setPos( double sec );
int music_isPlaying (void);
const char *music_playingName (void);


/*
 * lua control
 */
int nlua_loadMusic( lua_State *L, int read_only );
int music_choose( const char* situation );
void music_rechoose (void);


#endif /* MUSIC_H */
