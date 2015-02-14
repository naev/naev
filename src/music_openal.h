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


#ifndef MUSIC_OPENAL_H
#  define MUSIC_OPENAL_H


#if USE_OPENAL


#include "SDL_rwops.h"

#include "nopenal.h"


/*
 * Shared.
 */
extern ALuint music_source;


/*
 * Init/exit.
 */
int music_al_init (void);
void music_al_exit (void);


/*
 * Loading.
 */
int music_al_load( const char* name, SDL_RWops *rw );
void music_al_free (void);


/*
 * Music control.
 */
int music_al_volume( const double vol );
double music_al_getVolume (void);
double music_al_getVolumeLog(void);
void music_al_play (void);
void music_al_stop (void);
void music_al_pause (void);
void music_al_resume (void);
void music_al_setPos( double sec );
int music_al_isPlaying (void);


#endif /* USE_OPENAL */


#endif /* MUSIC_OPENAL_H */
