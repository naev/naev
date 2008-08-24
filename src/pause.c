/*
 * See Licensing and Copyright notice in naev.h
 */



#include "pause.h"

#include "pilot.h"


/*
 * main trick to pausing/unpausing is to allow things based on time to
 * behavie properly when the toolkit opens a window
 */


int paused = 0; /* is paused? */


/* from pilot.c */
extern Pilot** pilot_stack;
extern int pilot_nstack;
/* from main.c */
extern unsigned int time;

/*
 * prototypes
 */
static void pilot_nstack_pause (void);
static void pilot_nstack_unpause (void);
static void pilot_nstack_delay( unsigned int delay );


/*
 * pauses the game
 */
void pause_game (void)
{
   if (paused) return; /* already paused */

   pilot_nstack_pause();

   paused = 1; /* officially paused */
}


/*
 * unpauses the game
 */
void unpause_game (void)
{
   if (!paused) return; /* already unpaused */

   pilot_nstack_unpause();

   paused = 0; /* officially unpaused */
}


/*
 * sets the timers back delay
 */
void pause_delay( unsigned int delay )
{
   pilot_nstack_delay(delay);
}


/*
 * pilot_nstack pausing/unpausing
 */
static void pilot_nstack_pause (void)
{
   int i, j;
   unsigned int t = SDL_GetTicks();
   for (i=0; i<pilot_nstack; i++) {

      pilot_stack[i]->ptimer -= t;

      pilot_stack[i]->tcontrol -= t;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] -= t;
   }
}
static void pilot_nstack_unpause (void)
{
   int i, j;
   unsigned int t = SDL_GetTicks();
   for (i=0; i<pilot_nstack; i++) {
   
       pilot_stack[i]->ptimer += t;

      pilot_stack[i]->tcontrol += t;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] += t;
   }
}
static void pilot_nstack_delay( unsigned int delay )
{
   int i, j;
   for (i=0; i<pilot_nstack; i++) {

      pilot_stack[i]->ptimer += delay;

      pilot_stack[i]->tcontrol += delay;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] += delay;
   }
}

