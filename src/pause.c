

#include "pause.h"

#include "weapon.h"
#include "pilot.h"


/*
 * main trick to pausing/unpausing is to allow things based on time to
 * behavie properly when the toolkit opens a window
 */


int paused = 0; /* is paused? */


/* from pilot.c */
extern Pilot** pilot_stack;
extern int pilots;
/* from main.c */
extern unsigned int time;

/*
 * prototypes
 */
static void pilots_pause (void);
static void pilots_unpause (void);


/*
 * pauses the game
 */
void pause (void)
{
	if (paused) return; /* already paused */

	pilots_pause();
	weapons_pause();

	paused = 1; /* officially paused */
}


/*
 * unpauses the game
 */
void unpause (void)
{
	if (!paused) return; /* already unpaused */

	pilots_unpause();
	weapons_unpause();

	paused = 0; /* officially unpaused */
}


/*
 * pilots pausing/unpausing
 */
static void pilots_pause (void)
{
	int i, j;
	unsigned int t = SDL_GetTicks();
	for (i=0; i<pilots; i++) {
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
		pilot_stack[i]->tcontrol += t;

		for (j=0; j<MAX_AI_TIMERS; j++)
			pilot_stack[i]->timer[j] += t;
	}
}


