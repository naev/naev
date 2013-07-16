/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file music_sdlmix.c
 *
 * @brief SDL_mixer backend for the music.
 */


#if USE_SDLMIX


#include "music_sdlmix.h"

#include "naev.h"

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_mutex.h"

#include "sound_priv.h"
#include "music.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_var.h"
#include "nlua_music.h"
#include "log.h"
#include "ndata.h"
#include "conf.h"


static double music_curVolume = 0.; /**< Music volume. */


/*
 * The current music.
 */
static SDL_RWops *music_rw    = NULL; /**< Current music RWops. */
static Mix_Music *music_music = NULL; /**< Current music. */


/**
 * @brief Initializes the music subsystem.
 *
 *    @return 0 on success.
 */
int music_mix_init (void)
{
   /* Reset some variables. */
   music_rw       = NULL;
   music_music    = NULL;

   return 0;
}


/**
 * @brief Exits the music subsystem.
 */
void music_mix_exit (void)
{
}


/**
 * @brief Frees the current playing music.
 */
void music_mix_free (void)
{
   if (music_music != NULL) {
      Mix_HookMusicFinished(NULL);
      Mix_HaltMusic();
      Mix_FreeMusic(music_music);
      /*SDL_FreeRW(music_rw);*/ /* FreeMusic frees it itself */
      music_music = NULL;
      music_rw    = NULL;
   }
}


/**
 * @brief Sets the music volume.
 *
 *    @param vol Volume to set to (between 0 and 1).
 *    @return 0 on success.
 */
int music_mix_volume( const double vol )
{
   music_curVolume = MIX_MAX_VOLUME * CLAMP(0.,1.,vol);
   return Mix_VolumeMusic(music_curVolume);
}


/**
 * @brief Gets the current music volume.
 *
 *    @return The current music volume.
 */
double music_mix_getVolume (void)
{
   return music_curVolume / MIX_MAX_VOLUME;
}


/**
 * @brief Loads the music by name.
 *
 *    @param name Name of the file to load.
 */
int music_mix_load( const char* name, SDL_RWops *rw )
{
   (void) name;

   /* Load the data */
   music_rw = rw;
#if SDL_VERSION_ATLEAST(2,0,0)
   music_music = Mix_LoadMUS_RW(music_rw, 1);
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   music_music = Mix_LoadMUS_RW(music_rw);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
   if (music_music == NULL) {
      WARN("SDL_Mixer: %s", Mix_GetError());
      Mix_HookMusicFinished(music_rechoose);
      return -1;
   }

   /* Rechoose music on finish. */
   Mix_HookMusicFinished(music_rechoose);

   return 0;
}


/**
 * @brief Plays the loaded music.
 */
void music_mix_play (void)
{
   if (music_music == NULL) return;

   if (Mix_FadeInMusic( music_music, 0, MUSIC_FADEIN_DELAY ) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/**
 * @brief Stops the loaded music.
 */
void music_mix_stop (void)
{
   if (music_music == NULL) return;

   if (Mix_FadeOutMusic( MUSIC_FADEOUT_DELAY ) < 0)
      WARN("SDL_Mixer: %s", Mix_GetError());
}


/**
 * @brief Pauses the music.
 */
void music_mix_pause (void)
{
   if (music_music == NULL) return;

   Mix_PauseMusic();
}


/**
 * @brief Resumes the music.
 */
void music_mix_resume (void)
{
   if (music_music == NULL) return;

   Mix_ResumeMusic();
}


/**
 * @brief Checks to see if the music is playing.
 *
 *    @return 0 if music isn't playing, 1 if is playing.
 */
int music_mix_isPlaying (void)
{
   return Mix_PlayingMusic();
}


/**
 * @brief Sets the music to a position in seconds.
 *
 *    @param sec Position to go to in seconds.
 */
void music_mix_setPos( double sec )
{
   if (music_music == NULL) return;

   Mix_FadeInMusicPos( music_music, 1, MUSIC_FADEIN_DELAY, sec );
}


#endif /* USE_SDLMIX */
