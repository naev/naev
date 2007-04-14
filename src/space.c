

#include "space.h"

#include <malloc.h>
#include <math.h>

#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "rng.h"
#include "pilot.h"


#define STAR_BUF	100	/* area to leave around screen */
typedef struct {
	double x,y;
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
		stars[i].brightness = (double)RNG( 50, 200 )/256.;
		stars[i].x = (double)RNG( -STAR_BUF, gl_screen.w + STAR_BUF );
		stars[i].y = (double)RNG( -STAR_BUF, gl_screen.h + STAR_BUF );
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
		stars[i].x -= VX(player->solid->vel)/(15.-10.*stars[i].brightness)*dt;
		stars[i].y -= VY(player->solid->vel)/(15.-10.*stars[i].brightness)*dt;
		if (stars[i].x > gl_screen.w + STAR_BUF) stars[i].x = -STAR_BUF;
		else if (stars[i].x < -STAR_BUF) stars[i].x = gl_screen.w + STAR_BUF;
		if (stars[i].y > gl_screen.h + STAR_BUF) stars[i].y = -STAR_BUF;
		else if (stars[i].y < -STAR_BUF) stars[i].y = gl_screen.h + STAR_BUF;
		/* render */
		glColor4d( 1., 1., 1., stars[i].brightness );
		glVertex2d( stars[i].x, stars[i].y );
	}
	glEnd();

	glPopMatrix(); /* projection translation matrix */
}


void space_exit (void)
{
	free(stars);
}

