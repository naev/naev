/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef MUSIC_SDLMIX_H
#  define MUSIC_SDLMIX_H

#if USE_SDLMIX

#include "SDL_rwops.h"


/*
 * Init/exit.
 */
int music_mix_init (void);
void music_mix_exit (void);


/*
 * Loading.
 */
int music_mix_load( const char* name, SDL_RWops *rw );
void music_mix_free (void);


/*
 * Music control.
 */
int music_mix_volume( const double vol );
double music_mix_getVolume (void);
void music_mix_play (void);
void music_mix_stop (void);
void music_mix_pause (void);
void music_mix_resume (void);
void music_mix_setPos( double sec );
int music_mix_isPlaying (void);


#endif /* USE_SDLMIX */

#endif /* MUSIC_SDLMIX_H */
