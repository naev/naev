

#include "toolkit.h"


#include "log.h"
#include "pause.h"
#include "opengl.h"


typedef enum {
	WIDGET_NULL,
	WIDGET_BUTTON,
	WIDGET_TEXT
} WidgetType;

typedef enum {
	WIDGET_STATUS_NORMAL,
	WIDGET_STATUS_MOUSEOVER,
	WIDGET_STATUS_MOUSEDOWN
} WidgetStatus;

typedef struct {
	char* name; /* widget's name */
	WidgetType type; /* type */

	double x,y; /* position */
	double w,h; /* dimensions */

	WidgetStatus status;

	void (*fptr) (char*); /* callback */
	char *string; /* stored text */
} Widget;


typedef struct {
	unsigned int id; /* unique id */

	double x,y; /* position */
	double w,h; /* dimensions */

	Widget *widgets; /* widget storage */
	int nwidgets; /* total number of widgets */
} Window;


static unsigned int genwid = 0; /* generates unique window ids */


int toolkit = 0; /* toolkit in use */

#define MIN_WINDOWS	3
static Window *windows = NULL;
static int nwindows = 0;
static int mwindows = 0;

/*
 * prototypes
 */
static Widget* window_newWidget( const unsigned int wid );
static void widget_cleanup( Widget *widget );
static void window_render( Window* w );


/*
 * adds a button that when pressed will trigger call passing it's name as
 * only parameter
 */
void window_addButton( const unsigned int wid,
		const int x, const int y,
		const int w, const int h,
		char* name, char* display,
		void (*call) (char*) )
{
	Widget *widget = window_newWidget(wid);

	widget->type = WIDGET_BUTTON;
	widget->name = strdup(name);
	widget->string = strdup(display);

	/* set the properties */
	widget->x = (double) x;
	widget->y = (double) y;
	widget->w = (double) w;
	widget->h = (double) h;
	widget->fptr = call;
}


/*
 * returns pointer to a newly alloced Widget
 */
static Widget* window_newWidget( const unsigned int wid )
{
	int i;
	for (i=0; i<nwindows; i++)
		if (windows[i].id == wid)
			break;
	if (i == nwindows) return NULL;

	Widget* w = NULL;

	windows[i].widgets = realloc( windows[i].widgets,
			sizeof(Widget)*(++windows[i].nwidgets) );
	if (windows[i].widgets == NULL) WARN("Out of Memory");

	w = &windows[i].widgets[ windows[i].nwidgets - 1 ]; 

	w->type = WIDGET_NULL;
	w->status = WIDGET_STATUS_NORMAL;
	return w;
}


/*
 * creates a window
 */
unsigned int window_create( const int x, const int y, const int w, const int h )
{
	if (nwindows >= mwindows) { /* at memory limit */
		windows = realloc(windows, sizeof(Window)*(++mwindows));
		if (windows==NULL) WARN("Out of memory");
	}

	const int wid = (++genwid); /* unique id */

	windows[nwindows].id = wid;

	windows[nwindows].w = (double) w;
	windows[nwindows].h = (double) h;
	if ((x==-1) && (y==-1)) { /* center */
		windows[nwindows].x = windows[nwindows].w/2.;
		windows[nwindows].y = windows[nwindows].h/2.;
	}
	else {
		windows[nwindows].x = (double) x;
		windows[nwindows].y = (double) y;
	}

	windows[nwindows].widgets = NULL;
	windows[nwindows].nwidgets = 0;

	nwindows++;
	
	if (toolkit==0) { /* toolkit is on */
		SDL_ShowCursor(SDL_ENABLE);
		toolkit = 1; /* enable toolkit */
	}

	return wid;
}


/*
 * destroys a widget
 */
static void widget_cleanup( Widget *widget )
{
	if (widget->name) free(widget->name);

	if ((widget->type==WIDGET_TEXT) && widget->string)
		free(widget->string);
}


/* 
 * destroys a window
 */
void window_destroy( unsigned int wid )
{
	int i,j;

	/* destroy the window */
	for (i=0; i<nwindows; i++)
		if (windows[i].id == wid) {
			for (j=0; j<windows[i].nwidgets; j++)
				widget_cleanup(&windows[i].widgets[j]);
			free(windows[i].widgets);
			break;
		}
	
	/* move other windows down a layer */
	for ( ; i<(nwindows-1); i++)
		windows[i] = windows[i+1];

	nwindows--;
	if (nwindows==0) { /* no windows left */
		SDL_ShowCursor(SDL_DISABLE);
		toolkit = 0; /* disable toolkit */
	}
}


/*
 * renders a window
 */
static void window_render( Window* w )
{
	int i,j;
	double x, y;
	Widget* wgt;
	Vector2d v;

	x = w->x - (double)gl_screen.w/2.;
	y = w->y - (double)gl_screen.h/2.;

	/* translate to window position (bottom left) */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* projection translation matrix */
		glTranslated( x, y, 0.);


	/*
	 * window bg
	 */
	glBegin(GL_TRIANGLE_STRIP);
		COLOUR(cLightGrey);

		glVertex2d( 0.,	0. );
		glVertex2d( w->w,	0. );          
		glVertex2d( 0.,	w->h );
		glVertex2d( w->w,	w->h );

	glEnd(); /* GL_TRIANGLE_STRIP */

	glPopMatrix(); /* GL_PROJECTION */


	/*
	 * widgets
	 */
	for (i=0; i<w->nwidgets; i++) {

		wgt = &w->widgets[i];

		switch (wgt->type) {
			case WIDGET_NULL: break;

			case WIDGET_BUTTON:

				glMatrixMode(GL_PROJECTION);
				glPushMatrix(); /* projection translation matrix */
					glTranslated( x + wgt->x, y + wgt->y, 0. );

				glBegin(GL_TRIANGLE_STRIP);

					switch (wgt->status) { /* set the colour */
						case WIDGET_STATUS_NORMAL: COLOUR(cDarkGrey); break;
						case WIDGET_STATUS_MOUSEOVER: COLOUR(cGrey); break;
						case WIDGET_STATUS_MOUSEDOWN: COLOUR(cGreen); break;
					}

					glVertex2d( 0,			0.     );
					glVertex2d( wgt->w,	0.     );
					glVertex2d( 0,			wgt->h );
					glVertex2d( wgt->w,	wgt->h );

				glEnd(); /* GL_TRIANGLE_STRIP */

				glPopMatrix(); /* GL_PROJECTION */

				j = gl_printWidth( NULL, wgt->string );
				vect_csetmin( &v, w->x + wgt->x + (wgt->w - (double)j)/2.,
						w->y + wgt->y + (wgt->h - gl_defFont.h)/2. );
				gl_print( NULL, &v, &cRed, wgt->string );
				break;

			case WIDGET_TEXT:
				break;
		}
	}
}


/*
 * renders the windows
 */
void toolkit_render (void)
{
	int i;

	for (i=0; i<nwindows; i++)
		window_render(&windows[i]);
}


/*
 * input
 */
static int mouse_down = 0;
void toolkit_mouseEvent( SDL_Event* event )
{
	int i;
	double x, y;
	Window *w;
	Widget *wgt;

	/* set mouse button status */
	if (event->type==SDL_MOUSEBUTTONDOWN) mouse_down = 1;
	else if (event->type==SDL_MOUSEBUTTONUP) mouse_down = 0;
	/* ignore movements if mouse is down */
	else if ((event->type==SDL_MOUSEMOTION) && mouse_down) return;

	/* absolute positions */
	if (event->type==SDL_MOUSEMOTION) {
		x = (double)event->motion.x;
		y = gl_screen.h - (double)event->motion.y;
	}
	else if ((event->type==SDL_MOUSEBUTTONDOWN) || (event->type==SDL_MOUSEBUTTONUP)) {
		x = (double)event->button.x;
		y = gl_screen.h - (double)event->button.y;
	}

	w = &windows[nwindows-1];

	if ((x < w->x) || (x > (w->x + w->w)) || (y < w->y) || (y > (w->y + w->h)))
		return; /* not in current window */

	/* relative positions */
	x -= w->x;
	y -= w->y;

	for (i=0; i<w->nwidgets; i++) {
		wgt = &w->widgets[i];
		if ((x > wgt->x) && (x < (wgt->x + wgt->w)) &&
				(y > wgt->y) && (y < (wgt->y + wgt->h)))
			switch (event->type) {
				case SDL_MOUSEMOTION:
					wgt->status = WIDGET_STATUS_MOUSEOVER;
					break;

				case SDL_MOUSEBUTTONDOWN:
					wgt->status = WIDGET_STATUS_MOUSEDOWN;
					break;

				case SDL_MOUSEBUTTONUP:
					if (wgt->status==WIDGET_STATUS_MOUSEDOWN) {
						if (wgt->type==WIDGET_BUTTON) (*wgt->fptr)(wgt->name);
					}
					wgt->status = WIDGET_STATUS_NORMAL;
					break;
			}
		else
			wgt->status = WIDGET_STATUS_NORMAL;
	}
}


/*
 * initializes the toolkit
 */
int toolkit_init (void)
{
	windows = malloc(sizeof(Window)*MIN_WINDOWS);
	nwindows = 0;
	mwindows = MIN_WINDOWS;
	SDL_ShowCursor(SDL_DISABLE);

	return 0;
}


/*
 * exits the toolkit
 */
void toolkit_exit (void)
{
	int i;
	for (i=0; i<nwindows; i++)
		window_destroy(windows[i].id);
	free(windows);
}

