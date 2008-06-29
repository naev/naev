/*
 * See Licensing and Copyright notice in naev.h
 */


#include "space.h"

#include <malloc.h>
#include <math.h>

#include "xml.h"

#include "naev.h"
#include "opengl.h"
#include "log.h"
#include "rng.h"
#include "pack.h"
#include "player.h"
#include "pause.h"
#include "weapon.h"
#include "toolkit.h"
#include "spfx.h"
#include "ntime.h"
#include "nebulae.h"


#define XML_PLANET_ID         "Planets"
#define XML_PLANET_TAG        "planet"

#define XML_SYSTEM_ID         "Systems"
#define XML_SYSTEM_TAG        "ssys"

#define PLANET_DATA           "dat/planet.xml"
#define SYSTEM_DATA           "dat/ssys.xml"

#define PLANET_GFX_SPACE      "gfx/planet/space/"
#define PLANET_GFX_EXTERIOR   "gfx/planet/exterior/"

#define PLANET_GFX_EXTERIOR_W 400
#define PLANET_GFX_EXTERIOR_H 400

#define CHUNK_SIZE            32

/* used to overcome warnings due to 0 values */
#define FLAG_XSET             (1<<0)
#define FLAG_YSET             (1<<1)
#define FLAG_ASTEROIDSSET     (1<<2)
#define FLAG_INTERFERENCESET  (1<<3)
#define FLAG_SERVICESSET      (1<<4)
#define FLAG_TECHSET          (1<<5)
#define FLAG_FACTIONSET       (1<<6)


/*
 * planet <-> system name stack
 */
static char** planetname_stack = NULL;
static char** systemname_stack = NULL;
static int spacename_nstack = 0;


/* 
 * star system stack and friends
 */
StarSystem *systems_stack = NULL; /* star system stack */
int systems_nstack = 0; /* number of star systems */
static int total_planets = 0; /* total number of loaded planets - pretty silly */
StarSystem *cur_system = NULL; /* Current star system */


/*
 * fleet spawn rate
 */
unsigned int spawn_timer = 0; /* timer that controls spawn rate */


/*
 * star stack and friends
 */
#define STAR_BUF  100   /* area to leave around screen, more = less repitition */
typedef struct Star_ {
   double x,y; /* position, lighter to use to doubles then the physics system */
   double brightness; /* self-explanatory */
} Star;
static Star *stars = NULL; /* star array */
static int nstars = 0; /* total stars */
static int mstars = 0; /* memory stars are taking */


/* 
 * Prototypes
 */
/* intern */
static Planet* planet_pull( const char* name );
static void space_renderStars( const double dt );
static void space_addFleet( Fleet* fleet );
static StarSystem* system_parse( const xmlNodePtr parent );
static void system_parseJumps( const xmlNodePtr parent );
static PlanetClass planetclass_get( const char a );
/* extern */
extern void player_message ( const char *fmt, ... );
/* externed */
void planets_minimap( const double res, const double w,
      const double h, const RadarShape shape );
int space_sysSave( xmlTextWriterPtr writer );
int space_sysLoad( xmlNodePtr parent );



/*
 * draws the planets. used in player.c
 * matrix mode is already displaced to center of the minimap
 */
#define PIXEL(x,y)      \
   if ((shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
         (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y))<rc)))   \
   glVertex2i((x),(y))
void planets_minimap( const double res, const double w,
      const double h, const RadarShape shape )
{
   int i;
   int cx, cy, x, y, r, rc;
   double p;
   Planet *planet;
   glColour *col;

   if (shape==RADAR_CIRCLE) rc = (int)(w*w);

   glBegin(GL_POINTS);
   for (i=0; i<cur_system->nplanets; i++) {
      planet = &cur_system->planets[i];

      if (!planet_hasService(planet,PLANET_SERVICE_BASIC))
         col = &cInert;
      else
         col = faction_getColour(planet->faction);
      COLOUR(*col);

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
   glEnd(); /* GL_POINTS */
}
#undef PIXEL


/*
 * basically returns a PlanetClass integer from a char
 */
static PlanetClass planetclass_get( const char a )
{
   switch (a) {
      /* planets use letters */
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
      /* stations use numbers - not as many types */
      case '0': return STATION_CLASS_A;
      case '1': return STATION_CLASS_B;
      case '2': return STATION_CLASS_C;
      case '3': return STATION_CLASS_D;

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
   if (p->fuel < HYPERSPACE_FUEL) return 0;

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
   if (p->fuel < HYPERSPACE_FUEL) return -3;
   if (!space_canHyperspace(p)) return -1;

   /* pilot is now going to get automatically ready for hyperspace */
   pilot_setFlag(p, PILOT_HYP_PREP);

   return 0;
}


/*
 * returns the name of all the planets that belong to factions
 */
char** space_getFactionPlanet( int *nplanets, int *factions, int nfactions )
{
   int i,j,k;
   Planet* planet;
   char **tmp;
   int ntmp;
   int mtmp;

   ntmp = 0;
   mtmp = CHUNK_SIZE;
   tmp = malloc(sizeof(char*) * mtmp);

   for (i=0; i<systems_nstack; i++)
      for (j=0; j<systems_stack[i].nplanets; j++) {
         planet = &systems_stack[i].planets[j];
         for (k=0; k<nfactions; k++)
            if (planet->faction == factions[k]) {
               ntmp++;
               if (ntmp > mtmp) { /* need more space */
                  mtmp += CHUNK_SIZE;
                  tmp = realloc(tmp, sizeof(char*) * mtmp);
               }
               tmp[ntmp-1] = planet->name;
               break; /* no need to check all factions */
            }
      }

   (*nplanets) = ntmp;
   return tmp;
}


/*
 * returns the name of a random planet
 */
char* space_getRndPlanet (void)
{
   int i,j;
   char **tmp;
   int ntmp;
   int mtmp;
   char *res;

   ntmp = 0;
   mtmp = CHUNK_SIZE;
   tmp = malloc(sizeof(char*) * mtmp);

   for (i=0; i<systems_nstack; i++)
      for (j=0; j<systems_stack[i].nplanets; j++) {
         ntmp++;
         if (ntmp > mtmp) { /* need more space */
            mtmp += CHUNK_SIZE;
            tmp = realloc(tmp, sizeof(char*) * mtmp);
         }
         tmp[ntmp-1] = systems_stack[i].planets[j].name;
      }
   
   res = tmp[RNG(0,ntmp-1)];
   free(tmp);

   return res; 
}


/*
 * returns 1 if target system is reachable
 */
int space_sysReachable( StarSystem *sys )
{
   int i;

   if (sys_isKnown(sys)) return 1; /* it is known */

   /* check to see if it is adjacent to known */
   for (i=0; i<sys->njumps; i++)
      if (sys_isKnown(&systems_stack[ sys->jumps[i]]))
         return 1;

   return 0;
}


/*
 * get the system from it's name
 */
StarSystem* system_get( const char* sysname )
{
   int i;

   for (i=0; i<systems_nstack; i++)
      if (strcmp(sysname, systems_stack[i].name)==0)
         return &systems_stack[i];

   DEBUG("System '%s' not found in stack", sysname);
   return NULL;
}


/*
 * get the name of a system from a planetname
 */
char* planet_getSystem( char* planetname )
{
   int i;

   for (i=0; i<spacename_nstack; i++)
      if (strcmp(planetname_stack[i],planetname)==0)
         return systemname_stack[i];
   
   DEBUG("Planet '%s' not found in planetname stack", planetname);
   return NULL;
}


/*
 * gets a planet based on it's name
 */
Planet* planet_get( char* planetname )
{
   int i;
   char *sysname;
   StarSystem *sys;

   sysname = planet_getSystem( planetname );
   sys = system_get(sysname);

   for (i=0; i<sys->nplanets; i++)
      if (strcmp(planetname,sys->planets[i].name)==0)
         return &sys->planets[i];
   DEBUG("Planet '%s' not found in the universe", planetname);
   return NULL;
}


/*
 * basically used for spawning fleets and such
 */
void space_update( const double dt )
{
   unsigned int t;
   int i, j, f;
   (void)dt; /* not used for now */

   if (cur_system == NULL) return; /* can't update a null system */

   t = SDL_GetTicks();

   if (cur_system->nfleets == 0) /* stop checking if there are no fleets */
      spawn_timer = t + 300000;
   
   if (spawn_timer < t) { /* time to possibly spawn */

      /* spawn chance is based on overall % */
      f = RNG(0,100*cur_system->nfleets);
      j = 0;
      for (i=0; i < cur_system->nfleets; i++) {
         j += cur_system->fleets[i].chance;
         if (f < j) { /* add one fleet */
            space_addFleet( cur_system->fleets[i].fleet );
            break;
         }
      }

      spawn_timer = t + 60000./(float)cur_system->nfleets;
   }
}


/*
 * creates a fleet
 */
static void space_addFleet( Fleet* fleet )
{
   FleetPilot *plt;
   int i;
   double a;
   Vector2d vv,vp, vn;

   /* simulate they came from hyperspace */
   vect_pset( &vp, RNG(MIN_HYPERSPACE_DIST, MIN_HYPERSPACE_DIST*3.),
         RNG(0,360)*M_PI/180.);
   vectnull(&vn);

   for (i=0; i < fleet->npilots; i++)
      plt = &fleet->pilots[i];
      if (RNG(0,100) <= plt->chance) {
         /* other ships in the fleet should start split up */
         vect_cadd(&vp, RNG(75,150) * (RNG(0,1) ? 1 : -1),
               RNG(75,150) * (RNG(0,1) ? 1 : -1));

         a = vect_angle(&vp,&vn);
         vect_pset( &vv, plt->ship->speed * 2., a );

         pilot_create( plt->ship,
               plt->name,
               fleet->faction,
               (plt->ai != NULL) ? plt->ai : fleet->ai, /* Pilot AI override */
               a,
               &vp,
               &vv,
               0 );
      }
}


/*
 * initializes the system
 */
void space_init ( const char* sysname )
{
   char* nt;
   int i;

   /* cleanup some stuff */
   player_clear(); /* clears targets */
   pilots_clean(); /* destroy all the current pilots, except player */
   weapon_clear(); /* get rid of all the weapons */
   spfx_clear(); /* get rid of the explosions */

   if ((sysname==NULL) && (cur_system==NULL))
      ERR("Cannot reinit system if there is no system previously loaded");
   else if (sysname!=NULL) {
      for (i=0; i < systems_nstack; i++)
         if (strcmp(sysname, systems_stack[i].name)==0)
            break;

      if (i==systems_nstack) ERR("System %s not found in stack", sysname);
      cur_system = systems_stack+i;

      nt = ntime_pretty(0);
      player_message("Entering System %s on %s.", sysname, nt);
      free(nt);

      /* Handle background */
      if (cur_system->nebu_density > 0.) {
         /* Background is Nebulae */
         nebu_prep( cur_system->nebu_density, cur_system->nebu_volatility );
      }
      else {
         /* Backrgound is Stary */
         nstars = (cur_system->stars*SCREEN_W*SCREEN_H+STAR_BUF*STAR_BUF)/(800*640);
         if (mstars < nstars)
            stars = realloc(stars,sizeof(Star)*nstars); /* should realloc, not malloc */
         for (i=0; i < nstars; i++) {
            stars[i].brightness = (double)RNG( 50, 200 )/256.;
            stars[i].x = (double)RNG( -STAR_BUF, SCREEN_W + STAR_BUF );
            stars[i].y = (double)RNG( -STAR_BUF, SCREEN_H + STAR_BUF );
         }
      }
   }

   /* set up fleets -> pilots */
   for (i=0; i < cur_system->nfleets; i++)
      if (RNG(0,100) <= (cur_system->fleets[i].chance/2)) /* fleet check (50% chance) */
         space_addFleet( cur_system->fleets[i].fleet );
   
   /* start the spawn timer */
   spawn_timer = SDL_GetTicks() + 120000./(float)(cur_system->nfleets+1);

   /* we now know this system */
   sys_setFlag(cur_system,SYSTEM_KNOWN);
}


/*
 * loads the planet of name 'name'
 */
static Planet* planet_pull( const char* name )
{
   int i;

   Planet* temp = NULL;

   char str[PATH_MAX] = "\0";
   char* tstr;

   uint32_t flags = 0;

   uint32_t bufsize;
   char *buf = pack_readfile( DATA, PLANET_DATA, &bufsize );

   xmlNodePtr node, cur, ccur;
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
            temp->faction = -1; /* No faction */
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
                        temp->gfx_exterior = malloc( strlen(xml_get(cur))+sizeof(PLANET_GFX_EXTERIOR) );
                        snprintf( temp->gfx_exterior, strlen(xml_get(cur))+sizeof(PLANET_GFX_EXTERIOR),                            
                              PLANET_GFX_EXTERIOR"%s", xml_get(cur));
                     }
                  } while (xml_nextNode(cur));
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
                  } while(xml_nextNode(cur));
               }
               else if (xml_isNode(node,"general")) {
                  cur = node->children;
                  do {
                     if (xml_isNode(cur,"class"))
                        temp->class =
                           planetclass_get(cur->children->content[0]);
                     else if (xml_isNode(cur,"faction")) {
                        flags |= FLAG_FACTIONSET;
                        temp->faction = faction_get( xml_get(cur) );
                     }
                     else if (xml_isNode(cur, "description"))
                        temp->description = strdup( xml_get(cur) );

                     else if (xml_isNode(cur, "bar"))
                        temp->bar_description = strdup( xml_get(cur) );

                     else if (xml_isNode(cur, "services")) {
                        flags |= FLAG_SERVICESSET;
                        temp->services = xml_getInt(cur); /* flags gotten by data */
                     }

                     else if (xml_isNode(cur, "tech")) {
                        ccur = cur->children;
                        do {
                           if (xml_isNode(ccur,"main")) {
                              flags |= FLAG_TECHSET;
                              temp->tech[0] = xml_getInt(ccur);
                           }
                           else if (xml_isNode(ccur,"special")) {
                              for (i=1; i<PLANET_TECH_MAX; i++)
                                 if (temp->tech[i]==0) {
                                    temp->tech[i] = xml_getInt(ccur);
                                    break;
                                 }
                              if (i==PLANET_TECH_MAX) WARN("Planet '%s' has too many"
                                    "'special tech' entries", temp->name);
                           }
                        } while (xml_nextNode(ccur));
                     }
                     
                     else if (xml_isNode(cur, "commodities")) {
                        ccur = cur->children;
                        do {
                           if (xml_isNode(ccur,"commodity")) {
                              temp->commodities = realloc(temp->commodities,
                                    (temp->ncommodities+1) * sizeof(Commodity*));
                              temp->commodities[temp->ncommodities] =
                                    commodity_get( xml_get(ccur) );
                              temp->ncommodities++;
                           }
                        } while (xml_nextNode(ccur));
                     }
                  } while(xml_nextNode(cur));
               }
            } while (xml_nextNode(node));
            break;
         }
         else free(tstr); /* xmlGetProp mallocs the string */
      }
   } while (xml_nextNode(node));


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
#define MELEMENT(o,s)   if (o) WARN("Planet '%s' missing '"s"' element", temp->name)
      MELEMENT(temp->gfx_space==NULL,"GFX space");
      MELEMENT( planet_hasService(temp,PLANET_SERVICE_LAND) &&
            temp->gfx_exterior==NULL,"GFX exterior");
      MELEMENT((flags&FLAG_XSET)==0,"x");
      MELEMENT((flags&FLAG_YSET)==0,"y");
      MELEMENT(temp->class==PLANET_CLASS_NULL,"class");
      MELEMENT( planet_hasService(temp,PLANET_SERVICE_LAND) &&
            temp->description==NULL,"desription");
      MELEMENT( planet_hasService(temp,PLANET_SERVICE_BASIC) &&
            temp->bar_description==NULL,"bar");
      MELEMENT( planet_hasService(temp,PLANET_SERVICE_BASIC) &&
            (flags&FLAG_FACTIONSET)==0,"faction");
      MELEMENT((flags&FLAG_SERVICESSET)==0,"services");
      MELEMENT( (planet_hasService(temp,PLANET_SERVICE_OUTFITS) ||
            planet_hasService(temp,PLANET_SERVICE_SHIPYARD)) &&
            (flags&FLAG_TECHSET)==0, "tech" );
      MELEMENT( planet_hasService(temp,PLANET_SERVICE_COMMODITY) &&
            (temp->ncommodities==0),"commodity" );
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
         } while (xml_nextNode(cur));
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
            else if (xml_isNode(cur,"nebulae")) {
               ptrc = xml_nodeProp(cur,"volatility");
               if (ptrc != NULL) { /* Has volatility  */
                  temp->nebu_volatility = atof(ptrc);
                  free(ptrc);
               }
               temp->nebu_density = xml_getFloat(cur);
            }
         } while (xml_nextNode(cur));
      }
      /* loads all the planets */
      else if (xml_isNode(node,"planets")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"planet")) {
               /* add planet to system */
               total_planets++; /* increase planet counter */
               planet = planet_pull(xml_get(cur));
               temp->planets = realloc(temp->planets, sizeof(Planet)*(++temp->nplanets));
               memcpy(temp->planets+(temp->nplanets-1), planet, sizeof(Planet));

               /* add planet <-> star system to name stack */
               spacename_nstack++;
               planetname_stack = realloc(planetname_stack,
                     sizeof(char*)*spacename_nstack);
               systemname_stack = realloc(systemname_stack,
                     sizeof(char*)*spacename_nstack);
               planetname_stack[spacename_nstack-1] = planet->name;
               systemname_stack[spacename_nstack-1] = temp->name;

               free(planet);
            }
         } while (xml_nextNode(cur));
      }
      /* loads all the fleets */
      else if (xml_isNode(node,"fleets")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"fleet")) {
               fleet = CALLOC_ONE(SystemFleet);

               fleet->fleet = fleet_get(xml_get(cur));
               if (fleet->fleet==NULL)
                  WARN("Fleet '%s' for Star System '%s' not found",
                        xml_get(cur), temp->name);

               ptrc = xml_nodeProp(cur,"chance"); /* mallocs ptrc */
               if (ptrc==NULL) fleet->chance = 0; /* gives warning */
               else fleet->chance = atoi(ptrc);
               if (fleet->chance == 0)
                  WARN("Fleet '%s' for Star System '%s' has 0%% chance to appear",
                     fleet->fleet->name, temp->name);
               if (ptrc) free(ptrc); /* free the ptrc */

               temp->fleets = realloc(temp->fleets, sizeof(SystemFleet)*(++temp->nfleets));
               memcpy(temp->fleets+(temp->nfleets-1), fleet, sizeof(SystemFleet));
               free(fleet);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

#define MELEMENT(o,s)      if ((o) == 0) WARN("Star System '%s' missing '"s"' element", temp->name)
   if (temp->name == NULL) WARN("Star System '%s' missing 'name' tag", temp->name);
   MELEMENT(flags&FLAG_XSET,"x");
   MELEMENT(flags&FLAG_YSET,"y");
   MELEMENT(temp->stars,"stars");
   MELEMENT(flags&FLAG_ASTEROIDSSET,"asteroids");
   MELEMENT(flags&FLAG_INTERFERENCESET,"inteference");
#undef MELEMENT

   /* post-processing */
   if (temp->nplanets > 0) /* TODO make dependent on overall planet faction */
      temp->faction = temp->planets[0].faction;

   return temp;
}


/*
 * loads the jumps into a system
 */
static void system_parseJumps( const xmlNodePtr parent )
{
   int i;
   StarSystem *sys;
   char* name;
   xmlNodePtr cur, node;

   name = xml_nodeProp(parent,"name"); /* already mallocs */
   for (i=0; i<systems_nstack; i++)
      if (strcmp( systems_stack[i].name, name)==0) {
         sys = &systems_stack[i];
         break;
      }
   if (i==systems_nstack) WARN("System '%s' was not found in the stack for some reason",name);
   free(name); /* no more need for it */

   node  = parent->xmlChildrenNode;

   do { /* load all the data */
      if (xml_isNode(node,"jumps")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"jump")) {
               for (i=0; i<systems_nstack; i++)
                  if (strcmp( systems_stack[i].name, xml_get(cur))==0) {
                     sys->njumps++;
                     sys->jumps = realloc(sys->jumps, sys->njumps*sizeof(int));
                     sys->jumps[sys->njumps-1] = i;
                     break;
                  }
               if (i==systems_nstack)
                  WARN("System '%s' not found for jump linking",xml_get(cur));
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
}


/*
 * LOADS THE ENTIRE UNIVERSE INTO RAM - pretty big feat eh?
 *
 * uses a two system pass to first load the star systems_stack and then set jump routes
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
    * first pass - loads all the star systems_stack
    */
   do {
      if (xml_isNode(node,XML_SYSTEM_TAG)) {

         temp = system_parse(node);
         systems_stack = realloc(systems_stack, sizeof(StarSystem)*(++systems_nstack));
         memcpy(systems_stack+systems_nstack-1, temp, sizeof(StarSystem));
         free(temp);
      }                                                                             
   } while (xml_nextNode(node));                                       

   /*
    * second pass - loads all the jump routes
    */
   node = doc->xmlChildrenNode->xmlChildrenNode;
   do {

      if (xml_isNode(node,XML_SYSTEM_TAG))
         system_parseJumps(node); /* will automatically load the jumps into the system */

   } while (xml_nextNode(node));


   /*
    * cleanup
    */
   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Star System%s with %d Planet%s",
         systems_nstack, (systems_nstack==1) ? "" : "s",
         total_planets, (total_planets==1) ? "" : "s" );

   return 0;
}


/*
 * renders the system
 */
void space_render( const double dt )
{
   if (cur_system == NULL) return;

   if (cur_system->nebu_density > 0.)
      nebu_render(dt);
   else
      space_renderStars(dt);
}


/*
 * Renders the overlay
 */
void space_renderOverlay( const double dt )
{
   if (cur_system == NULL) return;

   if (cur_system->nebu_density > 0.)
      nebu_renderOverlay(dt);
}


/*
 * Renders stars
 */
static void space_renderStars( const double dt )
{
   int i;
   unsigned int t, timer;
   double x, y, m, b;

   /*
    * gprof claims it's the slowest thing in the game!
    */

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix(); /* translation matrix */
      glTranslated( -(double)SCREEN_W/2., -(double)SCREEN_H/2., 0);


   t = SDL_GetTicks();
   if (!player_isFlag(PLAYER_DESTROYED) && !player_isFlag(PLAYER_CREATING) &&
         pilot_isFlag(player,PILOT_HYPERSPACE) && /* hyperspace fancy effects */
         (player->ptimer-HYPERSPACE_STARS_BLUR < t)) {

      timer = player->ptimer - HYPERSPACE_STARS_BLUR;

      glShadeModel(GL_SMOOTH);

      glBegin(GL_LINES);

      /* lines will be based on velocity */
      m = HYPERSPACE_STARS_LENGTH * (double)(t-timer) / (HYPERSPACE_STARS_BLUR);
      x = m*cos(VANGLE(player->solid->vel)+M_PI);
      y = m*sin(VANGLE(player->solid->vel)+M_PI);

      for (i=0; i < nstars; i++) {
         glColor4d( 1., 1., 1., stars[i].brightness );
         glVertex2d( stars[i].x, stars[i].y );
         glColor4d( 1., 1., 1., 0. );
         glVertex2d( stars[i].x + x*stars[i].brightness,
               stars[i].y + y*stars[i].brightness );
      }
      glEnd(); /* GL_LINES */

      glShadeModel(GL_FLAT);
   }
   else { /* normal rendering */
      glBegin(GL_POINTS);

      if (!paused && !player_isFlag(PLAYER_DESTROYED) &&
            !player_isFlag(PLAYER_CREATING)) { /* update position */
         for (i=0; i < nstars; i++) {

            /* calculate new position */
            b = 13.-10.*stars[i].brightness;
            stars[i].x -= player->solid->vel.x/b*dt;
            stars[i].y -= player->solid->vel.y/b*dt;

            /* check boundries */
            if (stars[i].x > SCREEN_W + STAR_BUF) stars[i].x = -STAR_BUF;
            else if (stars[i].x < -STAR_BUF) stars[i].x = SCREEN_W + STAR_BUF;
            if (stars[i].y > SCREEN_H + STAR_BUF) stars[i].y = -STAR_BUF;
            else if (stars[i].y < -STAR_BUF) stars[i].y = SCREEN_H + STAR_BUF;

            /* render */
            if ((stars[i].x < SCREEN_W) && (stars[i].x > 0) &&
                  (stars[i].y < SCREEN_H) && (stars[i].y > 0)) {
               glColor4d( 1., 1., 1., stars[i].brightness );
               glVertex2d( stars[i].x, stars[i].y );
            }
         }
      }
      else { /* just render */
         for (i=0; i < nstars; i++) {
            if ((stars[i].x < SCREEN_W) && (stars[i].x > 0) &&
                  (stars[i].y < SCREEN_H) && (stars[i].y > 0)) {
               glColor4d( 1., 1., 1., stars[i].brightness );
               glVertex2d( stars[i].x, stars[i].y );
            }
         }
      }
      glEnd(); /* GL_POINTS */
   }

   glPopMatrix(); /* translation matrix */
}

/*
 * renders the planets
 */
void planets_render (void)
{
   if (cur_system==NULL) return;

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

   /* free the names */
   if (planetname_stack) free(planetname_stack);
   if (systemname_stack) free(systemname_stack);
   spacename_nstack = 0;
   
   /* free the systems */
   for (i=0; i < systems_nstack; i++) {
      free(systems_stack[i].name);
      if (systems_stack[i].fleets)
         free(systems_stack[i].fleets);
      if (systems_stack[i].jumps)
         free(systems_stack[i].jumps);

      /* free some planets */
      for (j=0; j < systems_stack[i].nplanets; j++) {
         free(systems_stack[i].planets[j].name);

         if (systems_stack[i].planets[j].description)
            free(systems_stack[i].planets[j].description);
         if (systems_stack[i].planets[j].bar_description)
            free(systems_stack[i].planets[j].bar_description);

         /* graphics */
         if (systems_stack[i].planets[j].gfx_space)
            gl_freeTexture(systems_stack[i].planets[j].gfx_space);
         if (systems_stack[i].planets[j].gfx_exterior)
            free(systems_stack[i].planets[j].gfx_exterior);

         /* commodities */
         free(systems_stack[i].planets[j].commodities);
      }

      free(systems_stack[i].planets);
   }
   free(systems_stack);
   systems_stack = NULL;
   systems_nstack = 0;

   /* stars must be free too */
   if (stars) free(stars);
   stars = NULL;
   nstars = 0;
}


/*
 * clears all system knowledge
 */
void space_clearKnown (void)
{
   int i;
   for (i=0; i<systems_nstack; i++)
      sys_rmFlag(&systems_stack[i],SYSTEM_KNOWN);
}


/*
 * clears all system markers
 */
void space_clearMarkers (void)
{
   int i;
   for (i=0; i<systems_nstack; i++)
      sys_rmFlag(&systems_stack[i],SYSTEM_MARKED);
}


/*
 * saves what is needed to be saved for space
 */
int space_sysSave( xmlTextWriterPtr writer )
{
   int i;

   xmlw_startElem(writer,"space");

   for (i=0; i<systems_nstack; i++) {

      if (!sys_isKnown(&systems_stack[i])) continue; /* not known */

      xmlw_elem(writer,"known","%s",systems_stack[i].name);
   }

   xmlw_endElem(writer); /* "space" */

   return 0;
}


/*
 * loads space
 */
int space_sysLoad( xmlNodePtr parent )
{
   xmlNodePtr node, cur;
   StarSystem *sys;

   space_clearKnown();

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"space")) {
         cur = node->xmlChildrenNode;

         do {
            if (xml_isNode(cur,"known")) {
               sys = system_get(xml_get(cur));
               if (sys != NULL) /* Must exist */
                  sys_setFlag(sys,SYSTEM_KNOWN);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;
}


