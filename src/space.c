/*
 * See Licensing and Copyright notice in naev.h
 */



#include "space.h"

#include <malloc.h>
#include <math.h>

#include "xml.h"

#include "naev.h"
#include "log.h"
#include "rng.h"
#include "pack.h"
#include "player.h"
#include "pause.h"
#include "weapon.h"
#include "toolkit.h"


#define XML_PLANET_ID			"Planets"
#define XML_PLANET_TAG			"planet"

#define XML_SYSTEM_ID			"Systems"
#define XML_SYSTEM_TAG			"ssys"

#define PLANET_DATA				"dat/planet.xml"
#define SYSTEM_DATA				"dat/ssys.xml"

#define PLANET_GFX_SPACE		"gfx/planet/space/"
#define PLANET_GFX_EXTERIOR	"gfx/planet/exterior/"

#define PLANET_GFX_EXTERIOR_W	400
#define PLANET_GFX_EXTERIOR_H	400

/* used to overcome warnings due to 0 values */
#define FLAG_XSET				   (1<<0)
#define FLAG_YSET    			(1<<1)
#define FLAG_ASTEROIDSSET		(1<<2)
#define FLAG_INTERFERENCESET	(1<<3)


StarSystem *systems = NULL;
static int nsystems = 0;
StarSystem *cur_system = NULL; /* Current star system */

/* current stardate in nice format */
char* stardate = "Stardate";
unsigned int date = 0; /* time since epoch */


#define STAR_BUF	100	/* area to leave around screen, more = less repitition */
typedef struct Star_ {
	double x,y; /* position, lighter to use to doubles then the physics system */
	double brightness; /* self-explanatory */
} Star;
static Star *stars = NULL; /* star array */
static int nstars = 0; /* total stars */
static int mstars = 0;


/* 
 * Prototypes
 */
/* intern */
static Planet* planet_get( const char* name );
static StarSystem* system_parse( const xmlNodePtr parent );
static void system_parseJumps( const xmlNodePtr parent );
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
 * checks to make sure if player is far enough away to hyperspace
 */
int space_canHyperspace( Pilot* p)
{
	int i;
	double d;
	for (i=0; i < cur_system->nplanets; i++) {
		d = vect_dist(&p->solid->pos, &cur_system->planets[i].pos);
		if (d < MIN_HYPERSPACE_DIST)
			return 0;
	}
	return 1;
}
/*
 * hyperspaces, returns 0 if entering hyperspace, or distance otherwise
 */
int space_hyperspace( Pilot* p )
{
	if (!space_canHyperspace(p)) return -1;

	/* pilot is now going to get automatically ready for hyperspace */
	pilot_setFlag(p, PILOT_HYP_PREP);

	return 0;
}


/*
 * initializes the system
 */
void space_init ( const char* sysname )
{
	int i,j;
	Vector2d v,vn;

	/* cleanup some stuff */
	player_clear(); /* clears targets */
	pilots_clean(); /* destroy all the current pilots, except player */
	weapon_clear(); /* get rid of all the weapons */

	if ((sysname==NULL) && (cur_system==NULL))
		ERR("Cannot reinit system if there is no system previously loaded");
	else if (sysname!=NULL) {
		for (i=0; i < nsystems; i++)
			if (strcmp(sysname, systems[i].name)==0)
				break;

		if (i==nsystems) ERR("System %s not found in stack", sysname);
		cur_system = systems+i;

		player_message("Entering System %s on %s", sysname, stardate);

		/* set up stars */
		nstars = (cur_system->stars*gl_screen.w*gl_screen.h+STAR_BUF*STAR_BUF)/(800*640);
		if (mstars < nstars)
			stars = realloc(stars,sizeof(Star)*nstars); /* should realloc, not malloc */
		for (i=0; i < nstars; i++) {
			stars[i].brightness = (double)RNG( 50, 200 )/256.;
			stars[i].x = (double)RNG( -STAR_BUF, gl_screen.w + STAR_BUF );
			stars[i].y = (double)RNG( -STAR_BUF, gl_screen.h + STAR_BUF );
		}
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
					vect_cadd(&v, RNG(75,150) * (RNG(0,1) ? 1 : -1),
							RNG(75,150) * (RNG(0,1) ? 1 : -1));

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
		if (xml_isNode(node,XML_PLANET_TAG)) {

			tstr = xml_nodeProp(node,"name");
			if (strcmp(tstr,name)==0) { /* found */
				temp = CALLOC_ONE(Planet);
				temp->name = tstr;

				node = node->xmlChildrenNode;

				do {
					if (xml_isNode(node,"GFX")) {
						cur = node->children;
						do {
							if (xml_isNode(cur,"space")) { /* load space gfx */
								snprintf( str, strlen(xml_get(cur))+sizeof(PLANET_GFX_SPACE),
										PLANET_GFX_SPACE"%s", xml_get(cur));
								temp->gfx_space = gl_newImage(str);
							}
							else if (xml_isNode(cur,"exterior")) { /* load land gfx */
								snprintf( str, strlen(xml_get(cur))+sizeof(PLANET_GFX_EXTERIOR),
										PLANET_GFX_EXTERIOR"%s", xml_get(cur));
								temp->gfx_exterior = gl_newImage(str);
							}
						} while ((cur = cur->next));
					}
					else if (xml_isNode(node,"pos")) {
						cur = node->children;
						do {
							if (xml_isNode(cur,"x")) {
								flags |= FLAG_XSET;
								temp->pos.x = xml_getFloat(cur);
							}
							else if (xml_isNode(cur,"y")) {
								flags |= FLAG_YSET;
								temp->pos.y = xml_getFloat(cur);
							}
						} while((cur = cur->next));
					}
					else if (xml_isNode(node,"general")) {
						cur = node->children;
						do {
							if (xml_isNode(cur,"class"))
								temp->class =
									planetclass_get(cur->children->content[0]);
							else if (xml_isNode(cur,"faction"))
								temp->faction = faction_get( xml_get(cur) );

							else if (xml_isNode(cur, "description"))
								temp->description = strdup( xml_get(cur) );

							else if (xml_isNode(cur, "bar"))
								temp->bar_description = strdup( xml_get(cur) );

							else if (xml_isNode(cur, "services"))
								temp->services = xml_getInt(cur); /* flags gotten by data */

						} while((cur = cur->next));
					}
				} while ((node = node->next));
				break;
			}
			else free(tstr); /* xmlGetProp mallocs the string */
		}
	} while ((node = node->next));


	/*
	 * free stuff
	 */
	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	/* 
	 * verification
	 */
	if (temp) {
#define MELEMENT(o,s)	if ((o) == 0) WARN("Planet '%s' missing '"s"' element", temp->name)
		MELEMENT(temp->gfx_space,"GFX space");
		MELEMENT(temp->gfx_exterior,"GFX exterior");
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

	temp->name = xml_nodeProp(parent,"name"); /* already mallocs */

	node  = parent->xmlChildrenNode;

	do { /* load all the data */
		if (xml_isNode(node,"pos")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"x")) {
					flags |= FLAG_XSET;
					temp->pos.x = xml_getFloat(cur);
				}
				else if (xml_isNode(cur,"y")) {
					flags |= FLAG_YSET;
					temp->pos.y = xml_getFloat(cur);
				}
			} while ((cur = cur->next));
		}
		else if (xml_isNode(node,"general")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"stars")) /* non-zero */
					temp->stars = xml_getInt(cur);
				else if (xml_isNode(cur,"asteroids")) {
					flags |= FLAG_ASTEROIDSSET;
					temp->asteroids = xml_getInt(cur);
				}
				else if (xml_isNode(cur,"interference")) {
					flags |= FLAG_INTERFERENCESET;
					temp->interference = xml_getFloat(cur)/100.;
				}
			} while ((cur = cur->next));
		}
		/* loads all the planets */
		else if (xml_isNode(node,"planets")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"planet")) {
					planet = planet_get(xml_get(cur));
					temp->planets = realloc(temp->planets, sizeof(Planet)*(++temp->nplanets));
					memcpy(temp->planets+(temp->nplanets-1), planet, sizeof(Planet));
					free(planet);
				}
			} while ((cur = cur->next));
		}
		/* loads all the fleets */
		else if (xml_isNode(node,"fleets")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"fleet")) {
					fleet = CALLOC_ONE(SystemFleet);

					fleet->fleet = fleet_get(xml_get(cur));
					if (fleet->fleet==NULL)
						WARN("Fleet %s for Star System %s not found",
								xml_get(cur), temp->name);

					ptrc = xml_nodeProp(cur,"chance"); /* mallocs ptrc */
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
 * loads the jumps into a system
 */
static void system_parseJumps( const xmlNodePtr parent )
{
	int i;
	StarSystem *system;
	char* name;
	xmlNodePtr cur, node;

	name = xml_nodeProp(parent,"name"); /* already mallocs */
	for (i=0; i<nsystems; i++)
		if (strcmp( systems[i].name, name)==0) {
			system = &systems[i];
			break;
		}
	if (i==nsystems) WARN("System '%s' was not found in the stack for some reason",name);
	free(name); /* no more need for it */

	node  = parent->xmlChildrenNode;

	do { /* load all the data */
		if (xml_isNode(node,"jumps")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"jump")) {
					for (i=0; i<nsystems; i++)
						if (strcmp( systems[i].name, xml_get(cur))==0) {
							system->njumps++;
							system->jumps = realloc(system->jumps, system->njumps*sizeof(int));
							system->jumps[system->njumps-1] = i;
							break;
						}
					if (i==nsystems)
						WARN("System '%s' not found for jump linking",xml_get(cur));
				}
			} while ((cur = cur->next));
		}
	} while ((node = node->next));
}


/*
 * LOADS THE ENTIRE UNIVERSE INTO RAM - pretty big feat eh?
 *
 * uses a two system pass to first load the star systems and then set jump routes
 */
int space_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, SYSTEM_DATA, &bufsize );

	StarSystem *temp;

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (!xml_isNode(node,XML_SYSTEM_ID)) {
		ERR("Malformed "SYSTEM_DATA"file: missing root element '"XML_SYSTEM_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed "SYSTEM_DATA" file: does not contain elements");
		return -1;
	}

	/*
	 * first pass - loads all the star systems
	 */
	do {
		if (xml_isNode(node,XML_SYSTEM_TAG)) {

			temp = system_parse(node);
			systems = realloc(systems, sizeof(StarSystem)*(++nsystems));
			memcpy(systems+nsystems-1, temp, sizeof(StarSystem));
			free(temp);
		}                                                                             
	} while ((node = node->next));                                       

	/*
	 * second pass - loads all the jump routes
	 */
	node = doc->xmlChildrenNode->xmlChildrenNode;
	do {

		if (xml_isNode(node,XML_SYSTEM_TAG))
			system_parseJumps(node); /* will automatically load the jumps into the system */

	} while ((node = node->next));


	/*
	 * cleanup
	 */
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

	/*
	 * gprof claims it's the slowest thing in the game!
	 */
	glBegin(GL_POINTS);
	for (i=0; i < nstars; i++) {
		if (!paused && !toolkit) {
			/* update position */
			stars[i].x -= VX(player->solid->vel)/(13.-10.*stars[i].brightness)*dt;
			stars[i].y -= VY(player->solid->vel)/(13.-10.*stars[i].brightness)*dt;
			if (stars[i].x > gl_screen.w + STAR_BUF) stars[i].x = -STAR_BUF;
			else if (stars[i].x < -STAR_BUF) stars[i].x = gl_screen.w + STAR_BUF;
			if (stars[i].y > gl_screen.h + STAR_BUF) stars[i].y = -STAR_BUF;
			else if (stars[i].y < -STAR_BUF) stars[i].y = gl_screen.h + STAR_BUF;
		}
		/* render */
		glColor4d( 1., 1., 1., stars[i].brightness );
		glVertex2d( stars[i].x - (double)gl_screen.w/2.,
				stars[i].y - (double)gl_screen.h/2. );
	}
	glEnd();
}

/*
 * renders the planets
 */
void planets_render (void)
{
	int i;
	for (i=0; i < cur_system->nplanets; i++)
		gl_blitSprite( cur_system->planets[i].gfx_space,
				cur_system->planets[i].pos.x, cur_system->planets[i].pos.y,
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
		if (systems[i].fleets)
			free(systems[i].fleets);
		if (systems[i].jumps)
			free(systems[i].jumps);


		for (j=0; j < systems[i].nplanets; j++) {
			free(systems[i].planets[j].name);

			if (systems[i].planets[j].description)
				free(systems[i].planets[j].description);
			if (systems[i].planets[j].bar_description)
				free(systems[i].planets[j].bar_description);

			/* graphics */
			if (systems[i].planets[j].gfx_space)
				gl_freeTexture(systems[i].planets[j].gfx_space);
			if (systems[i].planets[j].gfx_exterior)
				gl_freeTexture(systems[i].planets[j].gfx_exterior);
		}

		free(systems[i].planets);
	}
	free(systems);

	if (stars) free(stars);
}


