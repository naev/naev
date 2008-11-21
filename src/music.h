/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MUSIC_H
#  define MUSIC_H


#include "lua.h"


extern int music_disabled;
extern double music_defVolume;


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
void music_load( const char* name );
void music_play (void);
void music_stop (void);
void music_pause (void);
void music_resume (void);
void music_setPos( double sec );


/*
 * lua control
 */
int lua_loadMusic( lua_State *L, int read_only );
int music_choose( char* situation );


#endif /* MUSIC_H */
