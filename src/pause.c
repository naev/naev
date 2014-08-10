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


