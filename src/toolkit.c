

#include "toolkit.h"


#include "log.h"
#include "pause.h"


typedef struct {
	unsigned int id; /* unique id */

	double x,y; /* position */
	double w,h; /* dimensions */

	gl_texture *t; /* possible texture */
} Window;


static unsigned int genwid = 0; /* generates unique window ids */


int toolkit = 0; /* toolkit in use */

#define MIN_WINDOWS	3
static Window **windows = NULL;
static int nwindows = 0;
static int mwindows = 0;


/*
 * creates a window
 */
unsigned int window_create( int x, int y, int w, int h, gl_texture* t )
{
	Window *wtemp = NULL;
	if (nwindows == mwindows) { /* at memory limit */
		windows = realloc(windows, sizeof(Window*)*(++mwindows));
		if (windows==NULL) WARN("Out of memory");
	}
	wtemp = malloc(sizeof(Window));
	if (wtemp == NULL) WARN("Out of memory");

	int wid = (++genwid); /* unique id */

	wtemp->id = wid;

	wtemp->x = x;
	wtemp->y = y;
	wtemp->w = w;
	wtemp->h = h;
	wtemp->t = t;

	windows[nwindows++] = wtemp;
	
	if (toolkit==0) toolkit = 1; /* enable toolkit */

	return wid;
}


/* 
 * destroys a window
 */
void window_destroy( unsigned int wid )
{
	int i;

	/* destroy the window */
	for (i=0; i<nwindows; i++)
		if (windows[i]->id == wid) {
			free(windows[i]);
			break;
		}
	
	/* move other windows down a layer */
	for ( ; i<(nwindows-1); i++)
		windows[i] = windows[i+1];

	nwindows--;
	if (nwindows==0) toolkit = 0; /* disable toolkit */
}


/*
 * renders the windows
 */
void toolkit_render (void)
{
}


/*
 * initializes the toolkit
 */
int toolkit_init (void)
{
	windows = malloc(sizeof(Window*)*MIN_WINDOWS);
	nwindows = 0;
	mwindows = MIN_WINDOWS;

	return 0;
}


/*
 * exits the toolkit
 */
void toolkit_exit (void)
{
	int i;
	for (i=0; i<nwindows; i++)
		window_destroy(windows[i]->id);
	free(windows);
}

