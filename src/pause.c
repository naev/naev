/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file pause.c
 *
 * @brief Handles pausing and resuming the game.
 *
 * Main trick to pausing/unpausing is to allow things based on time to
 *  behavie properly when the toolkit opens a window.
 */


#include "pause.h"

#include "naev.h"

#include "player.h"
#include "sound.h"


int paused     = 0; /**< is paused? */
double dt_mod  = 1.; /**< dt modifier. */


/**
 * @brief Pauses the game.
 */
void pause_game (void)
{
   if (paused)
      return; /* already paused */

   /* Pause sounds. */
   if (player.p != NULL) {
      player_soundPause();
      sound_pause();
   }

   paused = 1; /* officially paused */
}


/**
 * @brief Unpauses the game.
 */
void unpause_game (void)
{
   if (!paused)
      return; /* already unpaused */

   /* Resume sounds. */
   if (player.p != NULL) {
      player_soundResume();
      sound_resume();
   }

   paused = 0; /* officially unpaused */
}

/**
 * @brief Adjusts the game's dt modifier.
 */
void pause_setSpeed( double mod )
{
   dt_mod = mod;
   sound_setSpeed( mod );
}


