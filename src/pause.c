/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file pause.c
 *
 * @brief Handles pausing and resuming the game.
 *
 * Main trick to pausing/unpausing is to allow things based on time to
 * behavie properly when the toolkit opens a window.
 *
 * @todo Should probably be eliminated by making everything use the dt system.
 */


#include "pause.h"

#include "player.h"


int paused = 0; /**< is paused? */
double dt_mod = 1.; /**< dt modifier. */


extern unsigned int time; /**< from naev.c */


/**
 * @brief Pauses the game.
 */
void pause_game (void)
{
   if (paused) return; /* already paused */

   player_soundPause();

   paused = 1; /* officially paused */
}


/**
 * @brief Unpauses the game.
 */
void unpause_game (void)
{
   if (!paused) return; /* already unpaused */

   player_soundResume();

   paused = 0; /* officially unpaused */
}


/**
 * @brief Sets the timers back.
 *    @param delay Delay to set timers back.
 */
void pause_delay( unsigned int delay )
{
   (void) delay;
}

/**
 * @brief Adjusts the game's dt modifier.
 */
void pause_setSpeed( double mod )
{
   dt_mod = mod;
}


