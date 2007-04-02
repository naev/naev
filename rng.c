

#include "rng.h"

#include <unistd.h>
#include "SDL.h"


void rng_init (void)
{
	srand( getpid() + SDL_GetTicks() );
}

