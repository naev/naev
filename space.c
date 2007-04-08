

#include "space.h"

#include "SDL.h"
#include "SDL_opengl.h"
#include <malloc.h>

#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "rng.h"
#include "pilot.h"


#define STAR_BUF	100	/* area to leave around screen */
typedef struct {
	Vector2d pos;
	double brightness;
} Star;
Star *stars;
int nstars;


void space_init (void)
{
	int i;
	nstars = (500*gl_screen.w*gl_screen.h+STAR_BUF*STAR_BUF)/(800*640);
	stars = malloc(sizeof(Star)*nstars);
	for (i=0; i < nstars; i++) {
		stars[i].brightness = (float)RNG( 50, 200 )/256.;
		stars[i].pos.x = (float)RNG( -STAR_BUF, gl_screen.w + STAR_BUF );
		stars[i].pos.y = (float)RNG( -STAR_BUF, gl_screen.h + STAR_BUF );
	}
}

void space_render( double dt )
{
	int i;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* projection translation matrix */
		glTranslated( -(double)gl_screen.w/2., -(double)gl_screen.h/2., 0.);

	glBegin(GL_POINTS);
	for (i=0; i < nstars; i++) {
		/* update position */
		stars[i].pos.x -= player->solid->vel.x/(15.-10.*stars[i].brightness)*dt;
		stars[i].pos.y -= player->solid->vel.y/(15.-10.*stars[i].brightness)*dt;
		/* render */
		glColor4d( 1., 1., 1., stars[i].brightness );
		glVertex2d( stars[i].pos.x, stars[i].pos.y );
	}
	glEnd();

	glPopMatrix(); /* projection translation matrix */
}


void space_exit (void)
{
	free(stars);
}

