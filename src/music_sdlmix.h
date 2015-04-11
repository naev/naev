/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
