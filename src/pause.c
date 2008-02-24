/*
 * See Licensing and Copyright notice in naev.h
 */



#include "pause.h"

#include "weapon.h"
#include "pilot.h"
#include "spfx.h"


/*
 * main trick to pausing/unpausing is to allow things based on time to
 * behavie properly when the toolkit opens a window
 */


int paused = 0; /* is paused? */


/* from pilot.c */
extern Pilot** pilot_stack;
extern int pilots;
/* from space.c */
extern unsigned int spawn_timer;
/* from main.c */
extern unsigned int time;

/*
 * prototypes
 */
static void pilots_pause (void);
static void pilots_unpause (void);
static void pilots_delay( unsigned int delay );


/*
 * pauses the game
 */
void pause_game (void)
{
   if (paused) return; /* already paused */

   pilots_pause();
   weapons_pause();
   spfx_pause();
   spawn_timer -= SDL_GetTicks();

   paused = 1; /* officially paused */
}


/*
 * unpauses the game
 */
void unpause_game (void)
{
   if (!paused) return; /* already unpaused */

   pilots_unpause();
   weapons_unpause();
   spfx_unpause();
   spawn_timer += SDL_GetTicks();

   paused = 0; /* officially unpaused */
}


/*
 * sets the timers back delay
 */
void pause_delay( unsigned int delay )
{
   pilots_delay(delay);
   weapons_delay(delay);
   spfx_delay(delay);
   spawn_timer += delay;
}


/*
 * pilots pausing/unpausing
 */
static void pilots_pause (void)
{
   int i, j;
   unsigned int t = SDL_GetTicks();
   for (i=0; i<pilots; i++) {

      pilot_stack[i]->ptimer -= t;

      pilot_stack[i]->tcontrol -= t;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] -= t;
   }
}
static void pilots_unpause (void)
{
   int i, j;
   unsigned int t = SDL_GetTicks();
   for (i=0; i<pilots; i++) {
   
       pilot_stack[i]->ptimer += t;

      pilot_stack[i]->tcontrol += t;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] += t;
   }
}
static void pilots_delay( unsigned int delay )
{
   int i, j;
   for (i=0; i<pilots; i++) {

      pilot_stack[i]->ptimer += delay;

      pilot_stack[i]->tcontrol += delay;
      for (j=0; j<MAX_AI_TIMERS; j++)
         pilot_stack[i]->timer[j] += delay;
   }
}

