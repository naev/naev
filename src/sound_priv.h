/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SOUND_PRIV_H
#  define SOUND_PRIV_H

/*
 * Private sound header, do not use outside of the sound subsystem.
 */

#if USE_OPENAL
#include <AL/al.h>
#endif /* USE_OPENAL */

#if USE_SDLMIX
#include "SDL_mixer.h"
#endif /* USE_SDLMIXER */


/*
 * Flags.
 */
#define VOICE_LOOPING      (1<<10) /* voice loops */
#define VOICE_STATIC       (1<<11) /* voice isn't relative */


/**
 * @struct alSound
 *
 * @brief Contains a sound buffer.
 */
typedef struct alSound_ {
   char *name; /**< Buffer's name. */

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
         double pos[2]; /**< Position of the voice. */
         double vel[2]; /**< Velocity of the voice. */
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
alVoice* voice_new (void);
int voice_add( alVoice* v );
alVoice* voice_get( int id );


#endif /* SOUND_PRIV_H */
