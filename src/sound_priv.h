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


#ifndef SOUND_PRIV_H
#  define SOUND_PRIV_H

/*
 * Private sound header, do not use outside of the sound subsystem.
 */

#if USE_OPENAL
#include OPENAL_AL_H
#endif /* USE_OPENAL */

#if USE_SDLMIX
#include "SDL_mixer.h"
#endif /* USE_SDLMIX */


/*
 * Flags.
 */
#define VOICE_LOOPING      (1<<10) /* voice loops */
#define VOICE_STATIC       (1<<11) /* voice isn't relative */


#define MUSIC_FADEOUT_DELAY   1000 /**< Time it takes to fade out. */
#define MUSIC_FADEIN_DELAY    2000 /**< Time it takes to fade in. */


/**
 * @struct alSound
 *
 * @brief Contains a sound buffer.
 */
typedef struct alSound_ {
   char *name; /**< Buffer's name. */
   double length; /**< Length of the buffer. */

   /*
    * Backend specific.
    */
   union {
#if USE_OPENAL
      struct {
         ALuint buf; /**< Buffer data. */
      } al; /**< For OpenAL backend. */
#endif /* USE_OPENAL */
#if USE_SDLMIX
      struct {
         Mix_Chunk *buf;
      } mix; /**< For SDL_mixer backend. */
#endif /* USE_SDLMIX */
   } u; /**< For backend. */
} alSound;


/**
 * @typedef voice_state_t
 * @brief The state of a voice.
 * @sa alVoice
 */
typedef enum voice_state_ {
   VOICE_STOPPED, /**< Voice is stopped. */
   VOICE_PLAYING, /**< Voice is playing. */
   VOICE_FADEOUT, /**< Voice is fading out. */
   VOICE_DESTROY  /**< Voice should get destroyed asap. */
} voice_state_t;


/**
 * @struct alVoice
 *
 * @brief Represents a voice in the game.
 *
 * A voice would be any object that is creating sound.
 */
typedef struct alVoice_ {
   struct alVoice_ *prev; /**< Linked list previous member. */
   struct alVoice_ *next; /**< Linked list next member. */

   int id; /**< Identifier of the voice. */

   voice_state_t state; /**< Current state of the sound. */
   unsigned int flags; /**< Voice flags. */

   /*
    * Backend specific.
    */
   union {
#if USE_OPENAL
      struct {
         ALfloat pos[3]; /**< Position of the voice. */
         ALfloat vel[3]; /**< Velocity of the voice. */
         ALuint source; /**< Source current in use. */
         ALuint buffer; /**< Buffer attached to the voice. */
      } al; /**< For OpenAL backend. */
#endif /* USE_OPENAL */
#if USE_SDLMIX
      struct {
         int channel; /**< Channel sound is playing on. */
      } mix; /**< For SDL_mixer backend. */
#endif /* USE_SDLMIX */
   } u; /**< For backend. */
} alVoice;


/*
 * Sound list.
 */
extern alVoice *voice_active; /**< Active voices. */


/*
 * Voice management.
 */
void voice_lock (void);
void voice_unlock (void);
alVoice* voice_new (void);
int voice_add( alVoice* v );
alVoice* voice_get( int id );


#endif /* SOUND_PRIV_H */
