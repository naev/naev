

#include "space.h"

#include <malloc.h>
#include <math.h>

#include "xml.h"

#include "main.h"
#include "log.h"
#include "rng.h"
#include "pack.h"
#include "player.h"


#define XML_PLANET_ID	"Planets"
#define XML_PLANET_TAG	"planet"

#define XML_SYSTEM_ID	"Systems"
#define XML_SYSTEM_TAG	"ssys"

#define PLANET_DATA		"dat/planet.xml"
#define SYSTEM_DATA		"dat/ssys.xml"

#define PLANET_GFX		"gfx/planet/"

/* used to overcome warnings due to 0 values */
#define FLAG_XSET				   (1<<0)
#define FLAG_YSET    			(1<<1)
#define FLAG_ASTEROIDSSET		(1<<2)
#define FLAG_INTERFERENCESET	(1<<3)


static StarSystem *systems = NULL;
static int nsystems = 0;
StarSystem *cur_system = NULL; /* Current star system */

/* current stardate in nice format */
char* stardate = "Stardate";
unsigned int date = 0; /* time since epoch */


#define STAR_BUF	100	/* area to leave around screen, more = less repitition */
typedef struct {
	double x,y; /* position, lighter to use to doubles then the physics system */
	double brightness; /* self-explanatory */
} Star;
static Star *stars = NULL; /* star array */
static int nstars = 0; /* total stars */


/* 
 * Prototypes
 */
/* intern */
static Planet* planet_get( const char* name );
static StarSystem* system_parse( const xmlNodePtr parent );
static PlanetClass planetclass_get( const char a );
/* extern */
extern void player_message ( const char *fmt, ... );


/*
 * draws the planets. used in player.c
 * matrix mode is already displaced to center of the minimap
 */
#define PIXEL(x,y)      \
	if ((shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
			(shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y))<rc)))	\
	glVertex2i((x),(y))
void planets_minimap( const double res, const double w, const double h,
		const RadarShape shape )
{
	int i;
	int cx, cy, x, y, r, rc;
	double p;

	if (shape==RADAR_CIRCLE) rc = (int)(w*w);

	glBegin(GL_POINTS);
	glMatrixMode(GL_PROJECTION);
	for (i=0; i<cur_system->nplanets; i++) {
		r = (int)(cur_system->planets[i].gfx_space->sw / res);
		cx = (int)((cur_system->planets[i].pos.x - player->solid->pos.x) / res);
		cy = (int)((cur_system->planets[i].pos.y - player->solid->pos.y) / res);

		x = 0;
		y = r;
		p = (5. - (double)(r*4)) / 4.;

		PIXEL( cx,   cy+y );
		PIXEL( cx,   cy-y );
		PIXEL( cx+y, cy   );
		PIXEL( cx-y, cy   );

		while (x<y) {
			x++;
			if (p < 0) p += 2*(double)(x)+1;
			else p += 2*(double)(x-(--y))+1;

			if (x==0) {
				PIXEL( cx,   cy+y );
				PIXEL( cx,   cy-y );
				PIXEL( cx+y, cy   );
				PIXEL( cx-y, cy   );
			}
			else 
				if (x==y) {
					PIXEL( cx+x, cy+y );
					PIXEL( cx-x, cy+y );
					PIXEL( cx+x, cy-y );
					PIXEL( cx-x, cy-y );
				}
				else 
					if (x<y) {
					PIXEL( cx+x, cy+y );
					PIXEL( cx-x, cy+y );
					PIXEL( cx+x, cy-y );
					PIXEL( cx-x, cy-y );
					PIXEL( cx+y, cy+x );
					PIXEL( cx-y, cy+x );
					PIXEL( cx+y, cy-x );
					PIXEL( cx-y, cy-x );
				}
			}
		}

		if (ABS(x) < w/2. && ABS(y) < h/2.) {}
	glEnd(); /* GL_POINTS */
}
#undef PIXEL


/*
 * basically returns a PlanetClass integer from a char
 */
static PlanetClass planetclass_get( const char a )
{
	switch (a) {
		case 'A': return PLANET_CLASS_A;
		case 'B': return PLANET_CLASS_B;
		case 'C': return PLANET_CLASS_C;
		case 'D': return PLANET_CLASS_D;
		case 'E': return PLANET_CLASS_E;
		case 'F': return PLANET_CLASS_F;
		case 'G': return PLANET_CLASS_G;
		case 'H': return PLANET_CLASS_H;
		case 'I': return PLANET_CLASS_I;
		case 'J': return PLANET_CLASS_J;
		case 'K': return PLANET_CLASS_K;
		case 'L': return PLANET_CLASS_L;
		case 'M': return PLANET_CLASS_M;
		case 'N': return PLANET_CLASS_N;
		case 'O': return PLANET_CLASS_O;
		case 'P': return PLANET_CLASS_P;
		case 'Q': return PLANET_CLASS_Q;
		case 'R': return PLANET_CLASS_R;
		case 'S': return PLANET_CLASS_S;
		case 'T': return PLANET_CLASS_T;
		case 'X': return PLANET_CLASS_X;
		case 'Y': return PLANET_CLASS_Y;
		case 'Z': return PLANET_CLASS_Z;

		default: return PLANET_CLASS_NULL;
	};
}


/*
 * hyperspaces, returns 0 if entering hyperspace, or distance otherwise
 */
int space_hyperspace( Pilot* p )
{
	int i;
	double d;
	for (i=0; i < cur_system->nplanets; i++) {
		d = vect_dist(&p->solid->pos, &cur_system->planets[i].pos);
		if (d < MIN_HYPERSPACE_DIST)
			return (int)(MIN_HYPERSPACE_DIST - d);;
	}

	/*
	 * TODO hyperspace stuff
	 */
	if (p == player) {
		/* player stuff */
	}
	else {

	}

	return 0;
}


/*
 * initializes the system
 */
void space_init ( const char* sysname )
{
	int i,j;
	Vector2d v,vn;

	for (i=0; i < nsystems; i++)
		if (strcmp(sysname, systems[i].name)==0)
			break;

	if (i==nsystems) ERR("System %s not found in stack", sysname);
	cur_system = systems+i;

	player_message("Entering System %s on %s", sysname, stardate);

	/* set up stars */
	nstars = (cur_system->stars*gl_screen.w*gl_screen.h+STAR_BUF*STAR_BUF)/(800*640);
	stars = realloc(stars,sizeof(Star)*nstars); /* should realloc, not malloc */
	for (i=0; i < nstars; i++) {
		stars[i].brightness = (double)RNG( 50, 200 )/256.;
		stars[i].x = (double)RNG( -STAR_BUF, gl_screen.w + STAR_BUF );
		stars[i].y = (double)RNG( -STAR_BUF, gl_screen.h + STAR_BUF );
	}

	/* set up fleets -> pilots */
	vectnull(&vn);
	for (i=0; i < cur_system->nfleets; i++)
		if (RNG(0,100) <= cur_system->fleets[i].chance) { /* fleet check */

			/* simulate they came from hyperspace */
			vect_pset( &v, 2*RNG(MIN_HYPERSPACE_DIST/2,MIN_HYPERSPACE_DIST),
					RNG(0,360)*M_PI/180.);

			for (j=0; j < cur_system->fleets[i].fleet->npilots; j++)
				if (RNG(0,100) <= cur_system->fleets[i].fleet->pilots[j].chance) {
					/* other ships in the fleet should start split up */
					vect_cadd(&v, RNG(50,150) * (RNG(0,1) ? 1 : -1),
							RNG(50,150) * (RNG(0,1) ? 1 : -1));

					pilot_create( cur_system->fleets[i].fleet->pilots[j].ship,
							cur_system->fleets[i].fleet->pilots[j].name,
							cur_system->fleets[i].fleet->faction,
							cur_system->fleets[i].fleet->ai,
							vect_angle(&v,&vn),
							&v,
							NULL,
							0 );
				}
		}
}


/*
 * loads the planet of name 'name'
 */
static Planet* planet_get( const char* name )
{
	Planet* temp = NULL;

	char str[PATH_MAX] = "\0";
	char* tstr;

	uint32_t flags = 0;

	uint32_t bufsize;
	char *buf = pack_readfile( DATA, PLANET_DATA, &bufsize );

	xmlNodePtr node, cur;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (strcmp((char*)node->name,XML_PLANET_ID)) {
		ERR("Malformed "PLANET_DATA"file: missing root element '"XML_PLANET_ID"'");
		return NULL;
	}

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed "PLANET_DATA" file: does not contain elements");
		return NULL;
	}


	do {
		if (node->type == XML_NODE_START &&
				strcmp((char*)node->name,XML_PLANET_TAG)==0) {

			tstr = (char*)xmlGetProp(node,(xmlChar*)"name");
			if (strcmp(tstr,name)==0) { /* found */
				temp = CALLOC_ONE(Planet);
				temp->name = tstr;

				node = node->xmlChildrenNode;

				do {
					if (strcmp((char*)node->name,"GFX")==0) {
						cur = node->children;
						if (strcmp((char*)cur->name,"text")==0) {
							snprintf( str, strlen((char*)cur->content)+sizeof(PLANET_GFX),
									PLANET_GFX"%s", (char*)cur->content);
							temp->gfx_space = gl_newImage(str);
						}
					}
					else if (strcmp((char*)node->name,"pos")==0) {
						cur = node->children;
						do {
							if (strcmp((char*)cur->name,"x")==0) {
								flags |= FLAG_XSET;
								temp->pos.x = atof((char*)cur->children->content);
							}
							else if (strcmp((char*)cur->name,"y")==0) {
								flags |= FLAG_YSET;
								temp->pos.y = atof((char*)cur->children->content);
							}
						} while((cur = cur->next));
					}
					else if (strcmp((char*)node->name,"general")==0) {
						cur = node->children;
						do {
							if (strcmp((char*)cur->name,"class")==0)
								temp->class =
									planetclass_get(cur->children->content[0]);
							else if (strcmp((char*)cur->name,"faction")==0)
								temp->faction = faction_get( (char*)cur->children->content);
						} while((cur = cur->next));
					}
				} while ((node = node->next));
				break;
			}
			else free(tstr); /* xmlGetProp mallocs the string */
		}
	} while ((node = node->next));
	
	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	if (temp) {
#define MELEMENT(o,s)	if ((o) == 0) WARN("Planet '%s' missing '"s"' element", temp->name)
		MELEMENT(flags&FLAG_XSET,"x");
		MELEMENT(flags&FLAG_YSET,"y");
		MELEMENT(temp->class,"class");
		MELEMENT(temp->faction,"faction");
#undef MELEMENT
	}
	else
		WARN("No Planet found matching name '%s'", name);

	return temp;
}


/*
 * parses node 'parent' which should be the node of a system
 * returns the StarSystem fully loaded
 */
static StarSystem* system_parse( const xmlNodePtr parent )
{
	Planet* planet = NULL;
	SystemFleet* fleet = NULL;
	StarSystem* temp = CALLOC_ONE(StarSystem);
	char* ptrc;
	xmlNodePtr cur, node;

	uint32_t flags;

	temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name"); /* already mallocs */

	node  = parent->xmlChildrenNode;

	do { /* load all the data */
		if (strcmp((char*)node->name,"pos")==0) {
			cur = node->children;
			do {
				if (strcmp((char*)cur->name,"x")==0) {
					flags |= FLAG_XSET;
					temp->pos.x = atof((char*)cur->children->content);
				}
				else if (strcmp((char*)cur->name,"y")==0) {
					flags |= FLAG_YSET;
					temp->pos.y = atof((char*)cur->children->content);
				}
			} while ((cur = cur->next));
		}
		else if (strcmp((char*)node->name,"general")==0) {
			cur = node->children;
			do {
				if (strcmp((char*)cur->name,"stars")==0) /* non-zero */
					temp->stars = atoi((char*)cur->children->content);
				else if (strcmp((char*)cur->name,"asteroids")==0) {
					flags |= FLAG_ASTEROIDSSET;
					temp->asteroids = atoi((char*)cur->children->content);
				}
				else if (strcmp((char*)cur->name,"interference")==0) {
					flags |= FLAG_INTERFERENCESET;
					temp->interference = atof((char*)cur->children->content)/100.;
				}
			} while ((cur = cur->next));
		}
		/* loads all the planets */
		else if (strcmp((char*)node->name,"planets")==0) {
			cur = node->children;
			do {
				if (strcmp((char*)cur->name,"planet")==0) {
					planet = planet_get((const char*)cur->children->content);
					temp->planets = realloc(temp->planets, sizeof(Planet)*(++temp->nplanets));
					memcpy(temp->planets+(temp->nplanets-1), planet, sizeof(Planet));
					free(planet);
				}
			} while ((cur = cur->next));
		}
		/* loads all the fleets */
		else if (strcmp((char*)node->name,"fleets")==0) {
			cur = node->children;
			do {
				if (strcmp((char*)cur->name,"fleet")==0) {
					fleet = CALLOC_ONE(SystemFleet);

					fleet->fleet = fleet_get((const char*)cur->children->content);
					if (fleet->fleet==NULL)
						WARN("Fleet %s for Star System %s not found",
								(char*)cur->children->content, temp->name);

					ptrc = (char*)xmlGetProp(cur,(xmlChar*)"chance"); /* mallocs ptrc */
					fleet->chance = atoi(ptrc);
					if (fleet->chance == 0)
						WARN("Fleet %s for Star System %s has 0%% chance to appear",
							fleet->fleet->name, temp->name);
					if (ptrc) free(ptrc); /* free the ptrc */

					temp->fleets = realloc(temp->fleets, sizeof(SystemFleet)*(++temp->nfleets));
					memcpy(temp->fleets+(temp->nfleets-1), fleet, sizeof(SystemFleet));
					free(fleet);
				}
			} while ((cur = cur->next));
		}
	} while ((node = node->next));

#define MELEMENT(o,s)      if ((o) == 0) WARN("Star System '%s' missing '"s"' element", temp->name)
	if (temp->name == NULL) WARN("Star System '%s' missing 'name' tag", temp->name);
	MELEMENT(flags&FLAG_XSET,"x");
	MELEMENT(flags&FLAG_YSET,"y");
	MELEMENT(temp->stars,"stars");
	MELEMENT(flags&FLAG_ASTEROIDSSET,"asteroids");
	MELEMENT(flags&FLAG_INTERFERENCESET,"inteference");
#undef MELEMENT

	DEBUG("Loaded Star System '%s' with %d Planet%c", temp->name,
			temp->nplanets, (temp->nplanets == 1) ? ' ' : 's' );

	return temp;
}


/*
 * LOADS THE ENTIRE UNIVERSE INTO RAM - pretty big feat eh?
 */
int space_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, SYSTEM_DATA, &bufsize );

	StarSystem *temp;

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (strcmp((char*)node->name,XML_SYSTEM_ID)) {
		ERR("Malformed "SYSTEM_DATA"file: missing root element '"XML_SYSTEM_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed "SYSTEM_DATA" file: does not contain elements");
		return -1;
	}

	do {
		if (node->type == XML_NODE_START &&           
				strcmp((char*)node->name,XML_SYSTEM_TAG)==0) {

			temp = system_parse(node);               
			systems = realloc(systems, sizeof(StarSystem)*(++nsystems));
			memcpy(systems+nsystems-1, temp, sizeof(StarSystem));
			free(temp);
		}                                                                             
	} while ((node = node->next));                                                   

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	return 0;
}


/*
 * renders the system
 */
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

/*
 * renders the planets
 */
void planets_render (void)
{
	int i;
	for (i=0; i < cur_system->nplanets; i++)
		gl_blitSprite( cur_system->planets[i].gfx_space, &cur_system->planets[i].pos,
				0, 0, NULL );
}


/*
 * cleans up the system
 */
void space_exit (void)
{
	int i,j;
	for (i=0; i < nsystems; i++) {
		free(systems[i].name);
		for (j=0; j < systems[i].nplanets; j++) {
			free(systems[i].planets[j].name);
			if (systems[i].planets[j].gfx_space)
				gl_freeTexture(systems[i].planets[j].gfx_space);
			if (systems[i].fleets)
				free(systems[i].fleets);
		}
		free(systems[i].planets);
	}
	free(systems);

	if (stars) free(stars);
}


