

#include "SDL.h"


#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"


int main ( int argc, const char** argv )
{
	gl_info info;

	info.w = 800;
	info.h = 640;
	info.depth = 32;
	info.fullscreen = 0;

	gl_init( &info );

	SDL_Delay(1000);

	gl_exit();

	return 0;
}

