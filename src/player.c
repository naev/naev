

#include "player.h"

#include <malloc.h>

#include "libxml/parser.h"

#include "main.h"
#include "pilot.h"
#include "log.h"
#include "opengl.h"
#include "pack.h"


#define XML_NODE_START	1
#define XML_NODE_TEXT	3

#define XML_GUI_ID	"GUIs"   /* XML section identifier */
#define XML_GUI_TAG	"gui"

#define GUI_DATA	"dat/gui.xml"
#define GUI_GFX	"gfx/gui/"


#define POW2(x)	((x)*(x))

#define GFX_GUI_FRAME		"gfx/gui/frame.png"
#define GFX_GUI_TARG_PILOT	"gfx/gui/pilot.png"
#define GFX_GUI_TARG_PLANET "gfx/gui/planet.png"


#define KEY_PRESS		1.
#define KEY_RELEASE	-1.
/* keybinding structure */
typedef struct {
	char *name; /* keybinding name, taken from keybindNames */
	KeybindType type; /* type, defined in playe.h */
	unsigned int key; /* key/axis/button event number */
	double reverse; /* 1. if normal, -1. if reversed, only useful for joystick axis */
} Keybind;
static Keybind** player_input; /* contains the players keybindings */
/* name of each keybinding */
const char *keybindNames[] = { "accel", "left", "right", "primary", "target",
		"target_nearest", "mapzoomin", "mapzoomout", 
		"end" }; /* must terminate in "end" */


/*
 * player stuff
 */
Pilot* player = NULL; /* ze player */
static double player_turn = 0.; /* turn velocity from input */
static double player_acc = 0.; /* accel velocity from input */
static int player_primary = 0; /* player is shooting primary weapon */
static unsigned int player_target = PLAYER_ID; /* targetted pilot */


/*
 * pilot stuff for GUI
 */
extern Pilot** pilot_stack;
extern int pilots;

/*
 * weapon stuff for GUI
 */
extern void weapon_minimap( double res, double w, double h );
extern void planets_minimap( double res, double w, double h );


/*
 * GUI stuff
 */
/*  colors */
typedef struct {
	double r, g, b, a;
} Color;
#define COLOR(x)		glColor4d((x).r,(x).g,(x).b,(x).a)
/* standard colors */
Color cInert			=	{ .r = 0.6, .g = 0.6, .b = 0.6, .a = 1. };
Color cNeutral			=	{ .r = 0.9, .g = 1.0, .b = 0.3, .a = 1. };
Color cFriend			=	{ .r = 0.0, .g = 1.0, .b = 0.0, .a = 1. };
Color cHostile			=	{ .r = 0.9, .g = 0.2, .b = 0.2, .a = 1. };

Color cRadar_player  =  { .r = 0.4, .g = 0.8, .b = 0.4, .a = 1. };
Color cRadar_targ		=	{ .r = 0.0,	.g = 0.7, .b = 1.0, .a = 1. };
Color cRadar_weap		=	{ .r = 0.8, .g = 0.2, .b = 0.2, .a = 1. };

Color cShield			=	{ .r = 0.2, .g = 0.2, .b = 0.8, .a = 1. };
Color cArmor			=	{ .r = 0.5, .g = 0.5, .b = 0.5, .a = 1. };
Color cEnergy			=	{ .r = 0.2, .g = 0.8, .b = 0.2, .a = 1. };
typedef enum { RADAR_RECT, RADAR_CIRCLE } RadarShape;
typedef struct {
	double w,h; /* dimensions */
	RadarShape shape;
	double res; /* resolution */
} Radar;
/* radar resolutions */
#define RADAR_RES_MAX		100.
#define RADAR_RES_MIN		10.
#define RADAR_RES_INTERVAL	10.
#define RADAR_RES_DEFAULT	40.

typedef struct {
	double w,h;
} Rect;

typedef struct {
	/* graphics */
	gl_font smallFont;
	gl_texture* gfx_frame;
	gl_texture* gfx_targetPilot, *gfx_targetPlanet;
	Radar radar;
	Rect shield, armor, energy;

	/* positions */
	Vector2d pos_frame;
	Vector2d pos_radar;
	Vector2d pos_shield, pos_armor, pos_energy;
	Vector2d pos_target, pos_target_health, pos_target_name, pos_target_faction;
	Vector2d pos_mesg;

} GUI;
GUI gui; /* ze GUI */
/* needed to render properly */
double gui_xoff = 0.;
double gui_yoff = 0.;

/* messages */
#define MESG_SIZE_MAX	80
int mesg_timeout = 3000;
int mesg_max = 5; /* maximum messages onscreen */
typedef struct {
	char str[MESG_SIZE_MAX];
	unsigned int t;
} Mesg;
static Mesg* mesg_stack;


/* 
 * prototypes
 */
/* external */
extern void pilot_render( Pilot* pilot ); /* from pilot.c */
/* internal */
static void rect_parse( const xmlNodePtr parent,
		double *x, double *y, double *w, double *h );
static int gui_parse( const xmlNodePtr parent, const char *name );
static void gui_renderPilot( const Pilot* p );
static void gui_renderBar( const Color* c, const Vector2d* p,
		const Rect* r, const double w );


/*
 * adds a mesg to the queue to be displayed on screen
 */
void player_message ( const char *fmt, ... )
{
	va_list ap;
	int i;

	if (fmt == NULL) return; /* message not valid */

	/* copy old messages back */
	for (i=1; i<mesg_max; i++)
		if (mesg_stack[mesg_max-i-1].str[0] != '\0') {
			strcpy(mesg_stack[mesg_max-i].str, mesg_stack[mesg_max-i-1].str);
			mesg_stack[mesg_max-i].t = mesg_stack[mesg_max-i-1].t;
		}

	/* add the new one */
	va_start(ap, fmt);
	vsprintf( mesg_stack[0].str, fmt, ap );
	va_end(ap);

	mesg_stack[0].t = SDL_GetTicks() + mesg_timeout;
}

/*
 * renders the player
 */
void player_render (void)
{
	int i, j;
	Pilot* p;
	Vector2d v;

	/* renders the player target graphics */
	if (player_target != PLAYER_ID) {
		p = pilot_get(player_target);

		vect_csetmin( &v, VX(p->solid->pos) - p->ship->gfx_space->sw * PILOT_SIZE_APROX/2.,
				VY(p->solid->pos) + p->ship->gfx_space->sh * PILOT_SIZE_APROX/2. );
		gl_blitSprite( gui.gfx_targetPilot, &v, 0, 0 ); /* top left */
		VX(v) += p->ship->gfx_space->sw * PILOT_SIZE_APROX;
		gl_blitSprite( gui.gfx_targetPilot, &v, 1, 0 ); /* top right */
		VY(v) -= p->ship->gfx_space->sh * PILOT_SIZE_APROX;
		gl_blitSprite( gui.gfx_targetPilot, &v, 1, 1 ); /* bottom right */
		VX(v) -= p->ship->gfx_space->sw * PILOT_SIZE_APROX;
		gl_blitSprite( gui.gfx_targetPilot, &v, 0, 1 ); /* bottom left */
	}

	/* render the player */
	pilot_render(player);

	/*
	 *    G U I
	 */
	/* frame */
	gl_blitStatic( gui.gfx_frame, &gui.pos_frame );

	/* radar */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glTranslated( VX(gui.pos_radar) - gl_screen.w/2. + gui.radar.w/2.,
				VY(gui.pos_radar) - gl_screen.h/2. - gui.radar.h/2., 0.);

	switch (gui.radar.shape) {
		case RADAR_RECT:

			/* planets */
			COLOR(cFriend);
			planets_minimap(gui.radar.res, gui.radar.w, gui.radar.h);

			/* weapons */
			glBegin(GL_POINTS);
				COLOR(cRadar_weap);
				weapon_minimap(gui.radar.res, gui.radar.w, gui.radar.h);
			glEnd(); /* GL_POINTS */


			/* render the pilots */
			for (j=0, i=1; i<pilots; i++) { /* skip the player */
				if (pilot_stack[i]->id == player_target) j = i;
				else gui_renderPilot(pilot_stack[i]);
			}
			/* render the targetted pilot */
			if (j!=0) gui_renderPilot(pilot_stack[j]);
			

			glBegin(GL_POINTS); /* for the player */
			break;

		case RADAR_CIRCLE:
			glBegin(GL_POINTS);
			for  (i=1; i<pilots; i++) {
				p = pilot_stack[i];
				COLOR(cNeutral);
				glVertex2d( (p->solid->pos.x - player->solid->pos.x) / gui.radar.res,
						(p->solid->pos.y - player->solid->pos.y) / gui.radar.res );
			}
			break;
	}

		/* player - drawn last*/
		COLOR(cRadar_player);
		glVertex2d(  0.,  2. ); /* we represent the player with a small + */
		glVertex2d(  0.,  1. ); 
		glVertex2d(  0.,  0. );
		glVertex2d(  0., -1. );                                             
		glVertex2d(  0., -2. );
		glVertex2d(  2.,  0. );
		glVertex2d(  1.,  0. );
		glVertex2d( -1.,  0. );
		glVertex2d( -2.,  0. );
	glEnd(); /* GL_POINTS */

	glPopMatrix(); /* GL_PROJECTION */

	/* health */
	gui_renderBar( &cShield, &gui.pos_shield, &gui.shield,
			player->shield / player->shield_max );
	gui_renderBar( &cArmor, &gui.pos_armor, &gui.armor, 
			player->armor / player->armor_max );
	gui_renderBar( &cEnergy, &gui.pos_energy, &gui.energy,
			player->energy / player->energy_max );


	/* target */
	if (player_target != PLAYER_ID) {
		p = pilot_get(player_target);

		gl_blitStatic( p->ship->gfx_target, &gui.pos_target );

		/* target name */
		gl_print( NULL, &gui.pos_target_name, "%s", p->name );
		gl_print( &gui.smallFont, &gui.pos_target_faction, "%s", p->faction->name );

		/* target status */
		if (p->armor < p->armor_max*PILOT_DISABLED) /* pilot is disabled */
			gl_print( &gui.smallFont, &gui.pos_target_health, "Disabled" );
		else if (p->shield > p->shield_max/100.) /* on shields */
			gl_print( &gui.smallFont, &gui.pos_target_health, "%s: %.0f%%",
				"Shield", p->shield/p->shield_max*100. );
		else /* on armour */
			gl_print( &gui.smallFont, &gui.pos_target_health, "%s: %.0f%%",
				"Armour", p->armor/p->armor_max*100. );
	}

	/* messages */
	VX(v) = VX(gui.pos_mesg);
	VY(v) = VY(gui.pos_mesg) + (double)(gl_defFont.h*mesg_max)*1.2;
	for (i=0; i<mesg_max; i++) {
		VY(v) -= (double)gl_defFont.h*1.2;
		if (mesg_stack[mesg_max-i-1].str[0]!='\0') {
			if (mesg_stack[mesg_max-i-1].t < SDL_GetTicks())
				mesg_stack[mesg_max-i-1].str[0] = '\0';
			else gl_print( NULL, &v, "%s", mesg_stack[mesg_max-i-1].str );
		}
	}
}

/*
 * renders a pilot
 */
static void gui_renderPilot( const Pilot* p )
{
	int x, y, sx, sy;

	x = (p->solid->pos.x - player->solid->pos.x) / gui.radar.res;
	y = (p->solid->pos.y - player->solid->pos.y) / gui.radar.res;
	sx = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sw / gui.radar.res;
	sy = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sh / gui.radar.res;
	if (sx < 1.) sx = 1.;
	if (sy < 1.) sy = 1.;

	if ( (ABS(x) > gui.radar.w/2+sx) || (ABS(y) > gui.radar.h/2.+sy) )
		return; /* pilot not in range */

	glBegin(GL_QUADS);
		/* colors */
		if (p->id == player_target) COLOR(cRadar_targ);
		else if (pilot_isFlag(p,PILOT_HOSTILE)) COLOR(cHostile);
		else COLOR(cNeutral);

		/* image */
		glVertex2d( MAX(x-sx,-gui.radar.w/2.),/* top-left */
			MIN(y+sy, gui.radar.h/2.) );
		glVertex2d( MIN(x+sx, gui.radar.w/2.), /* top-right */
			MIN(y+sy, gui.radar.h/2.) );
		glVertex2d( MIN(x+sx, gui.radar.w/2.), /* bottom-right */
			MAX(y-sy,-gui.radar.h/2.) );
		glVertex2d( MAX(x-sx,-gui.radar.w/2.), /* bottom-left */
			MAX(y-sy,-gui.radar.h/2.) );
	glEnd(); /* GL_QUADS */
}


/*
 * renders a bar (health)
 */
static void gui_renderBar( const Color* c, const Vector2d* p,
		const Rect* r, const double w )
{
	int x, y, sx, sy;

	glBegin(GL_QUADS); /* shield */
		COLOR(*c); 
		x = VX(*p) - gl_screen.w/2.;
		y = VY(*p) - gl_screen.h/2.;
		sx = w * r->w;
		sy = r->h;
		glVertex2d( x, y );
		glVertex2d( x + sx, y );
		glVertex2d( x + sx, y - sy );
		glVertex2d( x, y - sy );                                            
	glEnd(); /* GL_QUADS */

}


/*
 * initializes the GUI
 */
int gui_init (void)
{
	/*
	 * font
	 */
	gl_fontInit( &gui.smallFont, NULL, 10 );

	/*
	 * radar
	 */
	gui.radar.res = RADAR_RES_DEFAULT;

	/*
	 * messages
	 */
   vect_csetmin( &gui.pos_mesg, 20, 30 );
   mesg_stack = calloc(mesg_max, sizeof(Mesg));

	return 0;
}


/*
 * attempts to load the actual gui
 */
int gui_load (const char* name)
{
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, GUI_DATA, &bufsize );
	char *tmp;
	int found = 0;

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (strcmp((char*)node->name,XML_GUI_ID)) {
		ERR("Malformed '"GUI_DATA"' file: missing root element '"XML_GUI_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed '"GUI_DATA"' file: does not contain elements");
		return -1;
	}                                                                                       
	do {
		if (node->type == XML_NODE_START &&
				strcmp((char*)node->name,XML_GUI_TAG)==0) {

			tmp = (char*)xmlGetProp(node,(xmlChar*)"name"); /* mallocs */

			/* is the gui we are looking for? */
			if (strcmp(tmp,name)==0) {
				found = 1;

				/* parse the xml node */
				if (gui_parse(node,name)) WARN("Trouble loading GUI '%s'", name);
				free(tmp);
				break;
			}

			free(tmp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	if (!found) {
		WARN("GUI '%s' not found in '"GUI_DATA"'",name);
		return -1;
	}

	return 0;
}


/*
 * used to pull out a rect from an xml node (<x><y><w><h>)
 */
static void rect_parse( const xmlNodePtr parent,
		double *x, double *y, double *w, double *h )
{
	xmlNodePtr cur;
	int param;

	param = 0;

	cur = parent->children;
	do {
		if (strcmp((char*)cur->name,"x")==0) {
			if (x!=NULL) {
				*x = (double)atoi((char*)cur->children->content);
				param |= (1<<0);
			}
			else WARN("Extra parameter 'x' found for GUI node '%s'", parent->name);
		}
		else if (strcmp((char*)cur->name,"y")==0) {
			if (y!=NULL) {
				*y = (double)atoi((char*)cur->children->content);
				param |= (1<<1);
			}
			else WARN("Extra parameter 'y' found for GUI node '%s'", parent->name);
		}
		else if (strcmp((char*)cur->name,"w")==0) {
			if (w!=NULL) {
				*w = (double)atoi((char*)cur->children->content);
				param |= (1<<2);
			}
			else WARN("Extra parameter 'w' found for GUI node '%s'", parent->name);
		}
		else if (strcmp((char*)cur->name,"h")==0) {
			if (h!=NULL) {
				*h = (double)atoi((char*)cur->children->content);
				param |= (1<<3);
			}
			else WARN("Extra parameter 'h' found for GUI node '%s'", parent->name);
		}
	} while ((cur = cur->next));

	/* check to see if we got everything we asked for */
	if (x && !(param & (1<<0)))
		WARN("Missing parameter 'x' for GUI node '%s'", parent->name);
	else if (y && !(param & (1<<1))) 
		WARN("Missing parameter 'y' for GUI node '%s'", parent->name);
	else if (w && !(param & (1<<2))) 
		WARN("Missing parameter 'w' for GUI node '%s'", parent->name);
	else if (h && !(param & (1<<3))) 
		WARN("Missing parameter 'h' for GUI node '%s'", parent->name);
}


/*
 * parse a gui node
 */
static int gui_parse( const xmlNodePtr parent, const char *name )
{
	xmlNodePtr cur, node;
	double x, y;
	char *tmp, *tmp2;


	/*
	 * gfx
	 */
	/* set as a property and not a node because it must be loaded first */
	tmp2 = (char*)xmlGetProp(parent,(xmlChar*)"gfx");
	if (tmp2==NULL) {
		ERR("GUI '%s' has no gfx property",name);
		return -1;
	}

	/* load gfx */
	tmp = malloc( (strlen(tmp2)+strlen(GUI_GFX)+12) * sizeof(char) );
	/* frame */
	snprintf( tmp, strlen(tmp2)+strlen(GUI_GFX)+5, GUI_GFX"%s.png", tmp2 );
	gui.gfx_frame = gl_newImage( tmp );
	/* pilot */
	snprintf( tmp, strlen(tmp2)+strlen(GUI_GFX)+11, GUI_GFX"%s_pilot.png", tmp2 );
	gui.gfx_targetPilot = gl_newSprite( tmp, 2, 2 );
	/* planet */
	snprintf( tmp, strlen(tmp2)+strlen(GUI_GFX)+12, GUI_GFX"%s_planet.png", tmp2 );
	gui.gfx_targetPlanet = gl_newSprite( tmp, 2, 2 );
	free(tmp);
	free(tmp2);

	/*
	 * frame (based on gfx)
	 */
	vect_csetmin( &gui.pos_frame,
			gl_screen.w - gui.gfx_frame->w,     /* x */
			gl_screen.h - gui.gfx_frame->h );   /* y */
	/* used for rendering the player, displaces him a bit so he's centered onscreen */
	gui_xoff = - gui.gfx_frame->w/2.;


	/* now actually parse the data */
	node = parent->children;
	do { /* load all the data */

		/*
		 * radar
		 */
		if (strcmp((char*)node->name,"radar")==0) {

			tmp = (char*)xmlGetProp(node,(xmlChar*)"type");

			/* make sure type is valid */
			if (strcmp(tmp,"rectangle")==0) gui.radar.shape = RADAR_RECT;
			else if (strcmp(tmp,"circle")==0) gui.radar.shape = RADAR_CIRCLE;
			else {
				WARN("Radar for GUI '%s' is missing 'type' tag or has invalid 'type' tag",name);
				gui.radar.shape = RADAR_RECT;
			}

			free(tmp);

			rect_parse( node, &x, &y, &gui.radar.w, &gui.radar.h );
			vect_csetmin( &gui.pos_radar,
					VX(gui.pos_frame) + x,
					VY(gui.pos_frame) + gui.gfx_frame->h - y );
		}

		/*
		 * health bars
		 */
		else if (strcmp((char*)node->name,"health")==0) {
			cur = node->children;
			do {
				if (strcmp((char*)cur->name,"shield")==0) {
					rect_parse( cur, &x, &y, &gui.shield.w, &gui.shield.h );
					vect_csetmin( &gui.pos_shield,
						VX(gui.pos_frame) + x,
						VY(gui.pos_frame) + gui.gfx_frame->h - y );
				}
				else if (strcmp((char*)cur->name,"armor")==0) {
					rect_parse( cur, &x, &y, &gui.armor.w, &gui.armor.h );
					vect_csetmin( &gui.pos_armor,              
							VX(gui.pos_frame) + x,                   
							VY(gui.pos_frame) + gui.gfx_frame->h - y );
				}
				else if (strcmp((char*)cur->name,"energy")==0) {
					rect_parse( cur, &x, &y, &gui.energy.w, &gui.energy.h );
					vect_csetmin( &gui.pos_energy,              
							VX(gui.pos_frame) + x,                   
							VY(gui.pos_frame) + gui.gfx_frame->h - y );
				}
			} while ((cur = cur->next));
		}

		/*
		 * target
		 */
		else if (strcmp((char*)node->name,"target")==0) {
			cur = node->children;
			do {
				if (strcmp((char*)cur->name,"gfx")==0) {
					rect_parse( cur, &x, &y, NULL, NULL );
					vect_csetmin( &gui.pos_target,
							VX(gui.pos_frame) + x,
							VY(gui.pos_frame) + gui.gfx_frame->h - y - SHIP_TARGET_H );
				}
				else if (strcmp((char*)cur->name,"name")==0) {
					rect_parse( cur, &x, &y, NULL, NULL );
					vect_csetmin( &gui.pos_target_name,
							VX(gui.pos_frame) + x,
							VY(gui.pos_frame) + gui.gfx_frame->h - y - gl_defFont.h);
				}
				else if (strcmp((char*)cur->name,"faction")==0) {
					rect_parse( cur, &x, &y, NULL, NULL );
					vect_csetmin( &gui.pos_target_faction,
							VX(gui.pos_frame) + x,
							VY(gui.pos_frame) + gui.gfx_frame->h - y - gui.smallFont.h );
				}
				else if (strcmp((char*)cur->name,"health")==0) {
					rect_parse( cur, &x, &y, NULL, NULL );
					vect_csetmin( &gui.pos_target_health,
							VX(gui.pos_frame) + x,
							VY(gui.pos_frame) + gui.gfx_frame->h - y - gui.smallFont.h );
				}
			} while ((cur = cur->next));
		}
	} while ((node = node->next));

	return 0;
}
/*
 * frees the GUI
 */
void gui_free (void)
{
	gl_freeFont( &gui.smallFont );

	gl_freeTexture( gui.gfx_frame );
	gl_freeTexture( gui.gfx_targetPilot );
	gl_freeTexture( gui.gfx_targetPlanet );

	free(mesg_stack);
}


/*
 * used in pilot.c
 *
 * basically uses keyboard input instead of AI input
 */
void player_think( Pilot* player )
{
	player->solid->dir_vel = 0.;
	if (player_turn)
		player->solid->dir_vel -= player->ship->turn * player_turn;

	if (player_primary) pilot_shoot(player,0);

	vect_pset( &player->solid->force, player->ship->thrust * player_acc, player->solid->dir );
}


/*
 *
 *
 *		I N P U T
 *
 */
/*
 * sets the default input keys
 */
void input_setDefault (void)
{
	input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, 0 );
	input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, 0 );
	input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, 0 );
	input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, 0 );
	input_setKeybind( "target", KEYBIND_KEYBOARD, SDLK_TAB, 0 );
	input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_r, 0 );
	input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_9, 0 );
	input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_0, 0 );
}

/*
 * initialization/exit functions (does not assign keys)
 */
void input_init (void)
{
	Keybind *temp;
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++); /* gets number of bindings */
	player_input = malloc(i*sizeof(Keybind*));

	/* creates a null keybinding for each */
	for (i=0; strcmp(keybindNames[i],"end"); i++) {
		temp = MALLOC_ONE(Keybind);
		temp->name = (char*)keybindNames[i];
		temp->type = KEYBIND_NULL;
		temp->key = 0;
		temp->reverse = 1.;
		player_input[i] = temp;
	}
}
void input_exit (void)
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		free(player_input[i]);
	free(player_input);
}


/*
 * binds key of type type to action keybind
 *
 * @param keybind is the name of the keybind defined above
 * @param type is the type of the keybind
 * @param key is the key to bind to
 * @param reverse is whether to reverse it or not
 */
void input_setKeybind( char *keybind, KeybindType type, int key, int reverse )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (strcmp(keybind, player_input[i]->name)==0) {
			player_input[i]->type = type;
			player_input[i]->key = key;
			player_input[i]->reverse = (reverse) ? -1. : 1. ;
			return;
		}
	WARN("Unable to set keybinding '%s', that command doesn't exist", keybind);
}


/*
 * runs the input command
 *
 * @param keynum is the index of the player_input keybind
 * @param value is the value of the keypress (defined above)
 * @param abs is whether or not it's an absolute value (for them joystick)
 */
static void input_key( int keynum, double value, int abs )
{
	/* accelerating */
	if (strcmp(player_input[keynum]->name,"accel")==0) {
		if (abs) player_acc = value;
		else player_acc += value;
	/* turning left */
	} else if (strcmp(player_input[keynum]->name,"left")==0) {
		if (abs) player_turn = -value;
		else player_turn -= value;
	/* turning right */
	} else if (strcmp(player_input[keynum]->name,"right")==0) {
		if (abs)	player_turn = value;
		else player_turn += value;
	/* shooting primary weapon */
	} else if (strcmp(player_input[keynum]->name, "primary")==0) {
		if (value==KEY_PRESS) player_primary = 1;
		else if (value==KEY_RELEASE) player_primary = 0;
	/* targetting */
	} else if (strcmp(player_input[keynum]->name, "target")==0) {
		if (value==KEY_PRESS) player_target = pilot_getNext(player_target);
	} else if (strcmp(player_input[keynum]->name, "target_nearest")==0) {
		if (value==KEY_PRESS) player_target = pilot_getHostile();
	/* zooming in */
	} else if (strcmp(player_input[keynum]->name, "mapzoomin")==0) {
		if (value==KEY_PRESS && gui.radar.res < RADAR_RES_MAX)
			gui.radar.res += RADAR_RES_INTERVAL;
	/* zooming out */
	} else if (strcmp(player_input[keynum]->name, "mapzoomout")==0) {
		if (value==KEY_PRESS && gui.radar.res > RADAR_RES_MIN)
			gui.radar.res -= RADAR_RES_INTERVAL;
	}

	/* make sure values are sane */
	player_acc = ABS(player_acc);
	if (player_acc > 1.) player_acc = 1.;
	if (player_turn > 1.) player_turn = 1.;
	else if (player_turn < -1.) player_turn = -1.;
}


/*
 * events
 */
/* prototypes */
static void input_joyaxis( const unsigned int axis, const int value );
static void input_joydown( const unsigned int button );
static void input_joyup( const unsigned int button );
static void input_keydown( SDLKey key );
static void input_keyup( SDLKey key );

/*
 * joystick
 */
/* joystick axis */
static void input_joyaxis( const unsigned int axis, const int value )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (player_input[i]->type == KEYBIND_JAXIS && player_input[i]->key == axis) {
			input_key(i,-(player_input[i]->reverse)*(double)value/32767.,1);
			return;
		}
}
/* joystick button down */
static void input_joydown( const unsigned int button )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (player_input[i]->type == KEYBIND_JBUTTON && player_input[i]->key == button) {
			input_key(i,KEY_PRESS,0);
			return;
		}  
}
/* joystick button up */
static void input_joyup( const unsigned int button )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (player_input[i]->type == KEYBIND_JBUTTON && player_input[i]->key == button) {
			input_key(i,KEY_RELEASE,0);
			return;
		} 
}


/*
 * keyboard
 */
/* key down */
static void input_keydown( SDLKey key )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (player_input[i]->type == KEYBIND_KEYBOARD && player_input[i]->key == key) {
			input_key(i,KEY_PRESS,0);
			return;
		}

	/* ESC = quit */
	SDL_Event quit;
	if (key == SDLK_ESCAPE) {
		quit.type = SDL_QUIT;
		SDL_PushEvent(&quit);
	}

}
/* key up */
static void input_keyup( SDLKey key )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (player_input[i]->type == KEYBIND_KEYBOARD && player_input[i]->key == key) {
			input_key(i,KEY_RELEASE,0);
			return;
		}
}


/*
 * global input
 *
 * basically seperates the event types
 */
void input_handle( SDL_Event* event )
{
	switch (event->type) {
		case SDL_JOYAXISMOTION:
			input_joyaxis(event->jaxis.axis, event->jaxis.value);
			break;

		case SDL_JOYBUTTONDOWN:
			input_joydown(event->jbutton.button);
			break;

		case SDL_JOYBUTTONUP:
			input_joyup(event->jbutton.button);
			break;

		case SDL_KEYDOWN:
			input_keydown(event->key.keysym.sym);
			break;

		case SDL_KEYUP:
			input_keyup(event->key.keysym.sym);
			break;
	}
}



