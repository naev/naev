/*
 * See Licensing and Copyright notice in naev.h
 */



#include "toolkit.h"


#include "log.h"
#include "pause.h"
#include "opengl.h"


typedef enum WidgetType_ {
	WIDGET_NULL,
	WIDGET_BUTTON,
	WIDGET_TEXT,
	WIDGET_IMAGE,
	WIDGET_LIST
} WidgetType;

typedef enum WidgetStatus_ {
	WIDGET_STATUS_NORMAL,
	WIDGET_STATUS_MOUSEOVER,
	WIDGET_STATUS_MOUSEDOWN
} WidgetStatus;

typedef struct Widget_ {
	char* name; /* widget's name */
	WidgetType type; /* type */

	double x,y; /* position */
	double w,h; /* dimensions */

	WidgetStatus status;

	union {
		struct { /* WIDGET_BUTTON */
			void (*fptr) (char*); /* callback */
			char *display; /* stored text */
		} btn;
		struct { /* WIDGET_TEXT */
			char *text; /* text to display, using printMid if centered, else printText */
			glFont* font;
			glColour* colour;
			int centered; /* is centered? */
		} txt;
		struct { /* WIDGET_IMAGE */
			glTexture* image;
		} img;
		struct { /* WIDGET_LIST */
			char **options; /* pointer to the options */
			int noptions; /* total number of options */
			int selected; /* which option is currently selected */
		} lst;
	} dat;
} Widget;


typedef struct Window_ {
	unsigned int id; /* unique id */
	char *name; /* name */

	int hidden; /* is it hidden? */
	int focus; /* what bugger is focused? */

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
static Widget* window_newWidget( Window* w );
static void widget_cleanup( Widget *widget );
static Window* window_wget( const unsigned int wid );
static Widget* window_getwgt( const unsigned int wid, char* name );
/* input */
static void toolkit_mouseEvent( SDL_Event* event );
static int toolkit_keyEvent( SDL_Event* event );
static void toolkit_nextFocus (void);
static void toolkit_triggerFocus (void);
/* render */
static void window_render( Window* w );
static void toolkit_renderButton( Widget* btn, double bx, double by );
static void toolkit_renderText( Widget* txt, double bx, double by );
static void toolkit_renderImage( Widget* img, double bx, double by );
static void toolkit_drawOutline( double x, double y,
		double w, double h, double b,
		glColour* c, glColour* lc );



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
	Window *wdw = window_wget(wid);
	Widget *wgt = window_newWidget(wdw);

	wgt->type = WIDGET_BUTTON;
	wgt->name = strdup(name);
	wgt->dat.btn.display = strdup(display);

	/* set the properties */
	wgt->w = (double) w;
	wgt->h = (double) h;
	if (x < 0) wgt->x = wdw->w - wgt->w + x;
	else wgt->x = (double) x;
	if (y < 0) wgt->y = wdw->h - wgt->h + y;
	else wgt->y = (double) y;
	wgt->dat.btn.fptr = call;

	if (wdw->focus == -1) /* initialize the focus */
		toolkit_nextFocus();
}


/*
 * adds text to the window
 */
void window_addText( const unsigned int wid,
		const int x, const int y,
		const int w, const int h,
		const int centered, char* name,
		glFont* font, glColour* colour, char* string )
{
	Window *wdw = window_wget(wid);
	Widget *wgt = window_newWidget(wdw);

	wgt->type = WIDGET_TEXT;
	wgt->name = strdup(name); /* displays it's name */

	/* set the properties */
	wgt->w = (double) w;
	wgt->h = (double) h;
	if (font==NULL) wgt->dat.txt.font = &gl_defFont;
	else wgt->dat.txt.font = font;
	if (x < 0) wgt->x = wdw->w - wgt->w + x - w;
	else wgt->x = (double) x;
	if (y < 0) wgt->y = wdw->h + y - h;
	else wgt->y = (double) y;
	wgt->dat.txt.colour = colour;
	wgt->dat.txt.centered = centered;
	wgt->dat.txt.text = strdup(string);
}


/*
 * adds a graphic to the window
 */
void window_addImage( const unsigned int wid,
		const int x, const int y,
		char* name, glTexture* image )
{
	Window *wdw = window_wget(wid);
	Widget *wgt = window_newWidget(wdw);

	wgt->type = WIDGET_IMAGE;
	wgt->name = strdup(name);

	/* set the properties */
	wgt->dat.img.image = image;
	if (x < 0) wgt->x = wdw->w - wgt->dat.img.image->sw + x;
	else wgt->x = (double) x;
	if (y < 0) wgt->y = wdw->h + y;
	else wgt->y = (double) y;
}


/*
 * adds a list to the window
 */
void window_addList( const unsigned int wid,
		const int x, const int y,
		const int w, const int h,
		char* name, char **items, int nitems, int defitem )
{
	Window *wdw = window_wget(wid);
	Widget *wgt = window_newWidget(wdw);

	wgt->type = WIDGET_LIST;
	wgt->name = strdup(name);

	wgt->dat.lst.options = items;
	wgt->dat.lst.noptions = nitems;
	wgt->dat.lst.selected = defitem; /* -1 would be none */
	wgt->w = (double) w;
	wgt->h = (double) h;
	if (x < 0) wgt->x = wdw->w - wgt->w + x;
	else wgt->x = (double) x;
	if (y < 0) wgt->y = wdw->h - wgt->h + y;
	else wgt->y = (double) y;
}


/*
 * returns pointer to a newly alloced Widget
 */
static Widget* window_newWidget( Window* w )
{
	Widget* wgt = NULL;

	w->widgets = realloc( w->widgets,
			sizeof(Widget)*(++w->nwidgets) );
	if (w->widgets == NULL) WARN("Out of Memory");

	wgt = &w->widgets[ w->nwidgets - 1 ]; 

	wgt->type = WIDGET_NULL;
	wgt->status = WIDGET_STATUS_NORMAL;
	return wgt;
}


/*
 * returns the window of id wid
 */
static Window* window_wget( const unsigned int wid )
{
	int i;
	for (i=0; i<nwindows; i++)
		if (windows[i].id == wid)
			return &windows[i];
	DEBUG("Window '%d' not found in windows stack", wid);
	return NULL;
}


/*
 * gets the wgt from the window
 */
static Widget* window_getwgt( const unsigned int wid, char* name )
{
	int i;
	Window *wdw = window_wget(wid);

	for (i=0; i<wdw->nwidgets; i++)
		if (strcmp(wdw->widgets[i].name, name)==0)
			return &wdw->widgets[i];
	return NULL;
}


/*
 * modifies an existing text string
 */
void window_modifyText( const unsigned int wid,
      char* name, char* newstring )
{
	Widget *wgt = window_getwgt(wid,name);

	if (wgt->dat.txt.text) free(wgt->dat.txt.text);
	wgt->dat.txt.text = strdup(newstring);
}


/*
 * modifies an existing image's image
 */
void window_modifyImage( const unsigned int wid,
		char* name, glTexture* image )
{
	Widget *wgt = window_getwgt(wid,name);

	wgt->dat.img.image = image;
}



/*
 * checks to see if a window exists
 */
int window_exists( const char* wdwname )
{
	int i;
	for (i=0; i<nwindows; i++)
		if (strcmp(windows[i].name,wdwname)==0)
			return 1; /* exists */
	return 0; /* doesn't exist */
}


/*
 *	returns the id of a window
 */
unsigned int window_get( const char* wdwname )
{
	int i;
	for (i=0; i<nwindows; i++)
		if (strcmp(windows[i].name,wdwname)==0)
			return windows[i].id;
	DEBUG("Window '%s' not found in windows stack", wdwname);
	return 0;
}


/*
 * creates a window
 */
unsigned int window_create( char* name,
		const int x, const int y, const int w, const int h )
{
	if (nwindows >= mwindows) { /* at memory limit */
		windows = realloc(windows, sizeof(Window)*(++mwindows));
		if (windows==NULL) WARN("Out of memory");
	}

	const int wid = (++genwid); /* unique id */

	windows[nwindows].id = wid;
	windows[nwindows].name = strdup(name);

	windows[nwindows].hidden = 0;
	windows[nwindows].focus = -1;

	windows[nwindows].w = (double) w;
	windows[nwindows].h = (double) h;
	if ((x==-1) && (y==-1)) { /* center */
		windows[nwindows].x = gl_screen.w/2. - windows[nwindows].w/2.;
		windows[nwindows].y = gl_screen.h/2. - windows[nwindows].h/2.;
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

	switch (widget->type) {
		case WIDGET_BUTTON:
			if (widget->dat.btn.display) free(widget->dat.btn.display);
			break;

		case WIDGET_TEXT:
			if (widget->dat.txt.text) free(widget->dat.txt.text);
			break;

		default:
			break;
	}
}


/* 
 * destroys a window
 */
void window_destroy( const unsigned int wid )
{
	int i,j;

	/* destroy the window */
	for (i=0; i<nwindows; i++)
		if (windows[i].id == wid) {
			if (windows[i].name) free(windows[i].name);
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
		if (paused) unpause();
	}
}


/*
 * destroys a widget on a window
 */
void window_destroyWidget( unsigned int wid, const char* wgtname )
{
	Window *w = window_wget(wid);
	int i;

	for (i=0; i<w->nwidgets; i++)
		if (strcmp(wgtname,w->widgets[i].name)==0)
			break;
	if (i >= w->nwidgets) {
		DEBUG("widget '%s' not found in window %d", wgtname, wid);
		return;
	}
	
	widget_cleanup(&w->widgets[i]);
	if (i<w->nwidgets-1) /* not last widget */
		w->widgets[i] = w->widgets[i-1];
	w->nwidgets--; /* note that we don't actually realloc the space */
}


/* 
 * draws an outline
 * if bc is NULL, colour will be flat
 */
static void toolkit_drawOutline( double x, double y, 
		double w, double h, double b,
		glColour* c, glColour* lc )
{
	glShadeModel( (lc==NULL) ? GL_FLAT : GL_SMOOTH );
	if (!lc) COLOUR(*c);
	glBegin(GL_LINE_LOOP);
		/* left */
		if (lc) COLOUR(*lc);
		glVertex2d( x - b,      y         );
		if (lc) COLOUR(*c);
		glVertex2d( x - b,      y + h     );
		/* top */
		glVertex2d( x,          y + h + b );
		glVertex2d( x + w,      y + h + b );
		/* right */
		glVertex2d( x + w + b, y + h      );
		if (lc) COLOUR(*lc);
		glVertex2d( x + w + b, y          );
		/* bottom */
		glVertex2d( x + w,      y - b     );
		glVertex2d( x,          y - b     );
		glVertex2d( x - b,      y         );
	glEnd(); /* GL_LINES */

}


/*
 * renders a window
 */
static void window_render( Window* w )
{
	int i;
	double x, y, wid, hei;
	glColour *lc, *c, *dc, *oc;

	/* position */
	x = w->x - (double)gl_screen.w/2.;
	y = w->y - (double)gl_screen.h/2.;

	/* colours */
	lc = &cGrey90;
	c = &cGrey70;
	dc = &cGrey50;
	oc = &cGrey30;

	/*
	 * window shaded bg
	 */
	/* main body */
	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);
		COLOUR(*dc);
		glVertex2d( x + 21.,        y            );
		glVertex2d( x + w->w - 21., y            );

		COLOUR(*c);
		glVertex2d( x + w->w - 21., y + 0.6*w->h );
		glVertex2d( x + 21.,        y + 0.6*w->h );

	glEnd(); /* GL_QUADS */
	glShadeModel(GL_FLAT);
	glBegin(GL_QUADS);
		COLOUR(*c);
		glVertex2d( x + 21.,        y + 0.6*w->h );
		glVertex2d( x + w->w - 21., y + 0.6*w->h );
		glVertex2d( x + w->w - 21., y + w->h     );
		glVertex2d( x + 21.,        y + w->h     );
	glEnd(); /* GL_QUADS */
	glShadeModel(GL_SMOOTH);
	/* left side */
	glBegin(GL_POLYGON);
		COLOUR(*c);
		glVertex2d( x + 21., y + 0.6*w->h ); /* center */
		COLOUR(*dc);
		glVertex2d( x + 21., y       );
		glVertex2d( x + 15., y + 1.  );
		glVertex2d( x + 10., y + 3.  );
		glVertex2d( x + 6.,  y + 6.  );
		glVertex2d( x + 3.,  y + 10. );
		glVertex2d( x + 1.,  y + 15. );
		glVertex2d( x,       y + 21. );
		COLOUR(*c);
		glVertex2d( x,       y + 0.6*w->h ); /* infront of center */
		glVertex2d( x,       y + w->h - 21. );
		glVertex2d( x + 1.,  y + w->h - 15. );
		glVertex2d( x + 3.,  y + w->h - 10. );
		glVertex2d( x + 6.,  y + w->h - 6.  );
		glVertex2d( x + 10., y + w->h - 3.  );
		glVertex2d( x + 15., y + w->h - 1.  );
		glVertex2d( x + 21., y + w->h       );
	glEnd(); /* GL_POLYGON */
	/* right side */
	glBegin(GL_POLYGON);
		COLOUR(*c);
		glVertex2d( x + w->w - 21., y + 0.6*w->h ); /* center */
		COLOUR(*dc);
		glVertex2d( x + w->w - 21., y       );
		glVertex2d( x + w->w - 15., y + 1.  );
		glVertex2d( x + w->w - 10., y + 3.  );
		glVertex2d( x + w->w - 6.,  y + 6.  );
		glVertex2d( x + w->w - 3.,  y + 10. );
		glVertex2d( x + w->w - 1.,  y + 15. );
		glVertex2d( x + w->w,       y + 21. );
		COLOUR(*c);
		glVertex2d( x + w->w,       y + 0.6*w->h ); /* infront of center */
		glVertex2d( x + w->w,       y + w->h - 21. );
		glVertex2d( x + w->w - 1.,  y + w->h - 15. );
		glVertex2d( x + w->w - 3.,  y + w->h - 10. );
		glVertex2d( x + w->w - 6.,  y + w->h - 6.  );
		glVertex2d( x + w->w - 10., y + w->h - 3.  );
		glVertex2d( x + w->w - 15., y + w->h - 1.  );
		glVertex2d( x + w->w - 21., y + w->h       );
	glEnd(); /* GL_POLYGON */


	/* 
	 * inner outline
	 */
	glShadeModel(GL_SMOOTH);
	glBegin(GL_LINE_LOOP);
		/* left side */
		COLOUR(*c);
		glVertex2d( x + 21.+1., y+1.       );
		glVertex2d( x + 15.+1., y + 1.+1.  );
		glVertex2d( x + 10.+1., y + 3.+1.  );
		glVertex2d( x + 6.+1.,  y + 6.+1.  );
		glVertex2d( x + 3.+1.,  y + 10.+1. );
		glVertex2d( x + 1.+1.,  y + 15.+1. );
		glVertex2d( x+1.,       y + 21.+1. );
		COLOUR(*lc);
		glVertex2d( x+1.,       y + 0.6*w->h ); /* infront of center */
		glVertex2d( x+1.,       y + w->h - 21.-1. );
		glVertex2d( x + 1.+1.,  y + w->h - 15.-1. );
		glVertex2d( x + 3.+1.,  y + w->h - 10.-1. );
		glVertex2d( x + 6.+1.,  y + w->h - 6.-1.  );
		glVertex2d( x + 10.+1., y + w->h - 3.-1.  );
		glVertex2d( x + 15.+1., y + w->h - 1.-1.  );
		glVertex2d( x + 21.+1., y + w->h-1.       );
		/* switch to right via top */
		glVertex2d( x + w->w - 21.-1., y + w->h-1.       );
		glVertex2d( x + w->w - 15.-1., y + w->h - 1.-1.  );
		glVertex2d( x + w->w - 10.-1., y + w->h - 3.-1.  );
		glVertex2d( x + w->w - 6.-1.,  y + w->h - 6.-1.  );
		glVertex2d( x + w->w - 3.-1.,  y + w->h - 10.-1. );
		glVertex2d( x + w->w - 1.-1.,  y + w->h - 15.-1. );
		glVertex2d( x + w->w-1.,       y + w->h - 21.-1. );
		glVertex2d( x + w->w-1.,       y + 0.6*w->h ); /* infront of center */
		COLOUR(*c);
		glVertex2d( x + w->w-1.,       y + 21.+1. );
		glVertex2d( x + w->w - 1.-1.,  y + 15.+1. );
		glVertex2d( x + w->w - 3.-1.,  y + 10.+1. );
		glVertex2d( x + w->w - 6.-1.,  y + 6.+1.  );
		glVertex2d( x + w->w - 10.-1., y + 3.+1.  );
		glVertex2d( x + w->w - 15.-1., y + 1.+1.  );
		glVertex2d( x + w->w - 21.-1., y+1.       );
		glVertex2d( x + 21.+1., y+1.       ); /* back to beginning */
	glEnd(); /* GL_LINE_LOOP */


	/*
	 * outter outline
	 */
	glShadeModel(GL_FLAT);
	glBegin(GL_LINE_LOOP);
		/* left side */
		COLOUR(*oc);
		glVertex2d( x + 21., y       );
		glVertex2d( x + 15., y + 1.  );
		glVertex2d( x + 10., y + 3.  );
		glVertex2d( x + 6.,  y + 6.  );
		glVertex2d( x + 3.,  y + 10. );
		glVertex2d( x + 1.,  y + 15. );
		glVertex2d( x,       y + 21. );
		glVertex2d( x,       y + 0.6*w->h ); /* infront of center */
		glVertex2d( x,       y + w->h - 21. );
		glVertex2d( x + 1.,  y + w->h - 15. );
		glVertex2d( x + 3.,  y + w->h - 10. );
		glVertex2d( x + 6.,  y + w->h - 6.  );
		glVertex2d( x + 10., y + w->h - 3.  );
		glVertex2d( x + 15., y + w->h - 1.  );
		glVertex2d( x + 21., y + w->h       );
		/* switch to right via top */
		glVertex2d( x + w->w - 21., y + w->h       );
		glVertex2d( x + w->w - 15., y + w->h - 1.  );
		glVertex2d( x + w->w - 10., y + w->h - 3.  );
		glVertex2d( x + w->w - 6.,  y + w->h - 6.  );
		glVertex2d( x + w->w - 3.,  y + w->h - 10. );
		glVertex2d( x + w->w - 1.,  y + w->h - 15. );
		glVertex2d( x + w->w,       y + w->h - 21. );
		glVertex2d( x + w->w,       y + 0.6*w->h ); /* infront of center */
		glVertex2d( x + w->w,       y + 21. );
		glVertex2d( x + w->w - 1.,  y + 15. );
		glVertex2d( x + w->w - 3.,  y + 10. );
		glVertex2d( x + w->w - 6.,  y + 6.  );
		glVertex2d( x + w->w - 10., y + 3.  );
		glVertex2d( x + w->w - 15., y + 1.  );
		glVertex2d( x + w->w - 21., y       );
		glVertex2d( x + 21., y       ); /* back to beginning */
	glEnd(); /* GL_LINE_LOOP */

	/*
	 * render window name
	 */
	gl_printMid( &gl_defFont, w->w,
			x + (double)gl_screen.w/2.,
			y + w->h - 20. + (double)gl_screen.h/2.,
			&cBlack, w->name );

	/*
	 * widgets
	 */
	for (i=0; i<w->nwidgets; i++) {

		switch (w->widgets[i].type) {
			case WIDGET_NULL: break;

			case WIDGET_BUTTON:
				toolkit_renderButton( &w->widgets[i], x, y );
				break;

			case WIDGET_TEXT:
				toolkit_renderText( &w->widgets[i], x, y );
				break;

			case WIDGET_IMAGE:
				toolkit_renderImage( &w->widgets[i], x, y );
				break;

			case WIDGET_LIST:
				/* TODO widget list rendering */
				break;
		}
	}

	/*
	 * focused widget
	 */
	if (w->focus != -1) {
		x += w->widgets[w->focus].x;
		y += w->widgets[w->focus].y;
		wid = w->widgets[w->focus].w;
		hei = w->widgets[w->focus].h;
		toolkit_drawOutline( x, y, wid, hei, 3, &cBlack, NULL );
	}
}


/*
 * renders a button
 */
static void toolkit_renderButton( Widget* btn, double bx, double by )
{
	glColour *c, *dc, *oc, *lc;
	double x, y;

	x = bx + btn->x;
	y = by + btn->y;

	/* set the colours */
	switch (btn->status) {
		case WIDGET_STATUS_NORMAL:
			lc = &cGrey80;
			c = &cGrey60;
			dc = &cGrey40;
			oc = &cGrey20;
			break;
		case WIDGET_STATUS_MOUSEOVER:
			lc = &cWhite;
			c = &cGrey80;
			dc = &cGrey60;
			oc = &cGrey40;
			break;
		case WIDGET_STATUS_MOUSEDOWN:
			lc = &cGreen;
			c = &cGreen;
			dc = &cGrey40;
			oc = &cGrey20;
			break;
		default:
			break;
	}  


	/*
	 * shaded base
	 */
	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);

		COLOUR(*dc);
		glVertex2d( x,				y              );
		glVertex2d( x + btn->w,	y              );

		COLOUR(*c);
		glVertex2d( x + btn->w, y + 0.6*btn->h );
		glVertex2d( x,				y + 0.6*btn->h );

	glEnd(); /* GL_QUADS */

	glShadeModel(GL_FLAT);
	glBegin(GL_QUADS);
		COLOUR(*c);
		
		glVertex2d( x,          y + 0.6*btn->h );
		glVertex2d( x + btn->w, y + 0.6*btn->h );
		glVertex2d( x + btn->w, y + btn->h     );
		glVertex2d( x,          y + btn->h     );

	glEnd(); /* GL_QUADS */


	/* inner outline */
	toolkit_drawOutline( x, y, btn->w, btn->h, 0., lc, c );
	/* outter outline */
	toolkit_drawOutline( x, y, btn->w, btn->h, 1., &cBlack, NULL );

	gl_printMid( NULL, (int)btn->w,
			bx + (double)gl_screen.w/2. + btn->x,
			by + (double)gl_screen.h/2. + btn->y + (btn->h - gl_defFont.h)/2.,
			&cDarkRed, btn->dat.btn.display );
}
/*
 * renders the text
 */
static void toolkit_renderText( Widget* txt, double bx, double by )
{
	if (txt->dat.txt.centered)
		gl_printMid( txt->dat.txt.font, txt->w,
				bx + (double)gl_screen.w/2. + txt->x,
				by + (double)gl_screen.h/2. + txt->y,
				txt->dat.txt.colour, txt->dat.txt.text );
	else
		gl_printText( txt->dat.txt.font, txt->w, txt->h,
				bx + (double)gl_screen.w/2. + txt->x,
				by + (double)gl_screen.h/2. + txt->y,
				txt->dat.txt.colour, txt->dat.txt.text );
}
/*
 * renders the image
 */
static void toolkit_renderImage( Widget* img, double bx, double by )
{
	glColour *lc, *c, *oc;
	double x,y;

	if (img->dat.img.image == NULL) return;

	x = bx + img->x;
	y = by + img->y;

	lc = &cGrey90;
	c = &cGrey70;
	oc = &cGrey30;

	/*
	 * image
	 */
	gl_blitStatic( img->dat.img.image,
			x + (double)gl_screen.w/2.,
			y + (double)gl_screen.h/2., NULL );

	/* inner outline (outwards) */
	toolkit_drawOutline( x, y+1, img->dat.img.image->sw-1,
		img->dat.img.image->sh-1, 1., lc, c );
	/* outter outline */
	toolkit_drawOutline( x, y+1, img->dat.img.image->sw-1,
			img->dat.img.image->sh-1, 2., oc, NULL );
}


/*
 * renders the windows
 */
void toolkit_render (void)
{
	int i;

	if (gl_has(OPENGL_AA_LINE)) glEnable(GL_LINE_SMOOTH);
	if (gl_has(OPENGL_AA_POLYGON)) glEnable(GL_POLYGON_SMOOTH);

	for (i=0; i<nwindows; i++)
		window_render(&windows[i]);
	
	if (gl_has(OPENGL_AA_LINE)) glDisable(GL_LINE_SMOOTH);
	if (gl_has(OPENGL_AA_POLYGON)) glDisable(GL_POLYGON_SMOOTH);
}


/*
 * toolkit input handled here
 * if return is 1, then the input isn't passed along
 */
int toolkit_input( SDL_Event* event )
{
	switch (event->type) {
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			toolkit_mouseEvent(event);
			return 1; /* block input */

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			return toolkit_keyEvent(event);

	}
	return 0; /* don't block input */
}


/*
 * input
 */
static int mouse_down = 0;
static void toolkit_mouseEvent( SDL_Event* event )
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
						if (wgt->type==WIDGET_BUTTON) {
							if (wgt->dat.btn.fptr==NULL)
								DEBUG("Toolkit: Button '%s' of Window '%s' "
										"doesn't have a function trigger",
										wgt->name, w->name );
							else (*wgt->dat.btn.fptr)(wgt->name);
						}
					}
					wgt->status = WIDGET_STATUS_NORMAL;
					break;
			}
		else
			wgt->status = WIDGET_STATUS_NORMAL;
	}
}


/*
 * handles the key events
 */
static int toolkit_keyEvent( SDL_Event* event )
{
	SDLKey key = event->key.keysym.sym;

	switch (key) {
		case SDLK_TAB:
			if (event->type == SDL_KEYDOWN)
				toolkit_nextFocus();
			return 1;

		case SDLK_RETURN:
			if (event->type == SDL_KEYDOWN)
				toolkit_triggerFocus();
			return 1;

		default:
			return 0;
	}
}


/*
 * focus next widget
 */
static void toolkit_nextFocus (void)
{
	Window* wdw = &windows[nwindows-1]; /* get active window */

	if (wdw->nwidgets==0) 
		wdw->focus = -1;
	else if (wdw->focus >= wdw->nwidgets)
		wdw->focus = -1;
	else if ((++wdw->focus+1) && /* just increment */
		(wdw->widgets[wdw->focus].type == WIDGET_BUTTON))
		return;
	else
		toolkit_nextFocus();
}


/*
 * trigger the focused widget
 */
static void toolkit_triggerFocus (void)
{
	Window* wdw;
	Widget* wgt;

	wdw = &windows[nwindows-1];

	if (wdw->focus == -1) return;

	wgt = &wdw->widgets[wdw->focus];

	switch (wgt->type) {
	
		case WIDGET_BUTTON:
			(*wgt->dat.btn.fptr)(wgt->name);

		default:
			break;
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

