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


#ifndef MUSIC_H
#  define MUSIC_H


#include <lua.h>


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
