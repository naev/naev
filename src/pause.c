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

#include "pilot.h"


int paused = 0; /**< is paused? */
double dt_mod = 1.; /**< dt modifier. */


/* from pilot.c */
extern Pilot** pilot_stack;
extern int pilot_nstack;
/* from main.c */
extern unsigned int time;

/*
 * prototypes
 */
static void pilot_pause (void);
static void pilot_unpause (void);
static void pilot_delay( unsigned int delay );


/**
 * @brief Pauses the game.
 */
void pause_game (void)
{
   if (paused) return; /* already paused */

   pilot_pause();

   paused = 1; /* officially paused */
}


/**
 * @brief Unpauses the game.
 */
void unpause_game (void)
{
   if (!paused) return; /* already unpaused */

   pilot_unpause();

   paused = 0; /* officially unpaused */
}


/*
 * sets the timers back delay
 */
void pause_delay( unsigned int delay )
{
   pilot_delay(delay);
}

/**
 * @brief Adjusts the game's dt modifier.
 */
void pause_setSpeed( double mod )
{
   dt_mod = mod;
}


/**
 * @brief Pilot_stack pausing/unpausing.
 */
static void pilot_pause (void)
{
   int i, j;
   unsigned int t = SDL_GetTicks();
   for (i=0; i<pilot_nstack; i++) {

      pilot_stack[i]->ptimer -= t;

      /* Pause timers. */
      pilot_stack[i]->tcontrol -= t;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] -= t;

      /* Pause outfits. */
      for (j=0; j<pilot_stack[i]->noutfits; j++) {
         if (pilot_stack[i]->outfits[j].timer > 0)
            pilot_stack[i]->outfits[j].timer -= t;
      }
   }
}
static void pilot_unpause (void)
{
   int i, j;
   unsigned int t = SDL_GetTicks();
   for (i=0; i<pilot_nstack; i++) {
   
      pilot_stack[i]->ptimer += t;

      /* Rerun timers. */
      pilot_stack[i]->tcontrol += t;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] += t;

      /* Pause outfits. */
      for (j=0; j<pilot_stack[i]->noutfits; j++) {
         if (pilot_stack[i]->outfits[j].timer > 0)
            pilot_stack[i]->outfits[j].timer += t;
      }
   }
}
static void pilot_delay( unsigned int delay )
{
   int i, j;
   for (i=0; i<pilot_nstack; i++) {

      pilot_stack[i]->ptimer += delay;

      pilot_stack[i]->tcontrol += delay;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] += delay;

      for (j=0; j<pilot_stack[i]->noutfits; j++) {
         if (pilot_stack[i]->outfits[j].timer > 0)
            pilot_stack[i]->outfits[j].timer += delay;
      }
   }
}

