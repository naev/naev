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


#if USE_SDLMIX


#ifndef SOUND_SDLMIX_H
#  define SOUND_SDLMIX_H


#include "sound.h"
#include "sound_priv.h"


/*
 * Creation.
 */
int sound_mix_init (void);
void sound_mix_exit (void);


/*
 * Sound creation.
 */
int sound_mix_load( alSound *snd, const char *filename );
void sound_mix_free( alSound *snd );


/*
 * Sound settings.
 */
int sound_mix_volume( const double vol );
double sound_mix_getVolume (void);


/*
 * Sound playing.
 */
int sound_mix_play( alVoice *v, alSound *s );
int sound_mix_playPos( alVoice *v, alSound *s,
      double px, double py, double vx, double vy );
int sound_mix_updatePos( alVoice *v,
      double px, double py, double vx, double vy );
void sound_mix_updateVoice( alVoice *v );

/*
 * Sound management.
 */
void sound_mix_update (void);
void sound_mix_stop( alVoice *v );
void sound_mix_pause (void);
void sound_mix_resume (void);
void sound_mix_setSpeed( double s );
void sound_mix_setSpeedVolume( double vol );


/*
 * Listener.
 */
int sound_mix_updateListener( double dir, double px, double py,
      double vx, double vy );

/*
 * Groups.
 */
int sound_mix_createGroup( int size );
int sound_mix_playGroup( int group, alSound *s, int once );
void sound_mix_stopGroup( int group );
void sound_mix_pauseGroup( int group );
void sound_mix_resumeGroup( int group );
void sound_mix_speedGroup( int group, int enable );
void sound_mix_volumeGroup( int group, double volume );


/*
 * Env.
 */
int sound_mix_env( SoundEnv_t env, double param );



#endif /* SOUND_SDLMIX_H */


#endif /* USE_SDLMIX */
