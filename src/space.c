/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file space.c
 *
 * @brief Handles all the space stuff, namely systems and planets.
 */

#include "space.h"

#include "naev.h"

#include <stdlib.h>
#include <math.h>

#include "nxml.h"

#include "opengl.h"
#include "log.h"
#include "rng.h"
#include "ndata.h"
#include "pilot.h"
#include "player.h"
#include "pause.h"
#include "weapon.h"
#include "toolkit.h"
#include "spfx.h"
#include "ntime.h"
#include "nebula.h"
#include "sound.h"
#include "music.h"
#include "gui.h"
#include "fleet.h"
#include "mission.h"
#include "conf.h"
#include "queue.h"
#include "nlua.h"
#include "nlua_pilot.h"
#include "npng.h"
#include "background.h"
#include "map_overlay.h"
#include "menu.h"
#include "nstring.h"
#include "nmath.h"
#include "map.h"


#define XML_PLANET_ID         "Assets" /**< Planet xml document tag. */
#define XML_PLANET_TAG        "asset" /**< Individual planet xml tag. */

#define XML_SYSTEM_ID         "Systems" /**< Systems xml document tag. */
#define XML_SYSTEM_TAG        "ssys" /**< Individual systems xml tag. */

#define PLANET_DATA           "dat/asset.xml" /**< XML file containing planets. */
#define SYSTEM_DATA           "dat/ssys.xml" /**< XML file containing systems. */

#define PLANET_GFX_SPACE      "gfx/planet/space/" /**< Location of planet space graphics. */
#define PLANET_GFX_EXTERIOR   "gfx/planet/exterior/" /**< Location of planet exterior graphics (when landed). */

#define PLANET_GFX_EXTERIOR_W 400 /**< Planet exterior graphic width. */
#define PLANET_GFX_EXTERIOR_H 400 /**< Planet exterior graphic height. */

#define CHUNK_SIZE            32 /**< Size to allocate by. */
#define CHUNK_SIZE_SMALL       8 /**< Smaller size to allocate chunks by. */

/* used to overcome warnings due to 0 values */
#define FLAG_XSET             (1<<0) /**< Set the X position value. */
#define FLAG_YSET             (1<<1) /**< Set the Y position value. */
#define FLAG_ASTEROIDSSET     (1<<2) /**< Set the asteroid value. */
#define FLAG_INTERFERENCESET  (1<<3) /**< Set the interference value. */
#define FLAG_SERVICESSET      (1<<4) /**< Set the service value. */
#define FLAG_FACTIONSET       (1<<5) /**< Set the faction value. */


/*
 * planet <-> system name stack
 */
static char** planetname_stack = NULL; /**< Planet name stack corresponding to system. */
static char** systemname_stack = NULL; /**< System name stack corresponding to planet. */
static int spacename_nstack = 0; /**< Size of planet<->system stack. */
static int spacename_mstack = 0; /**< Size of memory in planet<->system stack. */


/*
 * Star system stack.
 */
StarSystem *systems_stack = NULL; /**< Star system stack. */
int systems_nstack = 0; /**< Number of star systems. */
static int systems_mstack = 0; /**< Number of memory allocated for star system stack. */

/*
 * Planet stack.
 */
static Planet *planet_stack = NULL; /**< Planet stack. */
static int planet_nstack = 0; /**< Planet stack size. */
static int planet_mstack = 0; /**< Memory size of planet stack. */

/*
 * Misc.
 */
static int systems_loading = 1; /**< Systems are loading. */
StarSystem *cur_system = NULL; /**< Current star system. */
glTexture *jumppoint_gfx = NULL; /**< Jump point graphics. */


/*
 * fleet spawn rate
 */
int space_spawn = 1; /**< Spawn enabled by default. */
extern int pilot_nstack;
extern Pilot** pilot_stack;


/*
 * Interference.
 */
extern double interference_alpha; /* gui.c */
static double interference_target = 0.; /**< Target alpha level. */
static double interference_timer = 0.; /**< Interference timer. */


/*
 * Internal Prototypes.
 */
/* planet load */
static int planet_parse( Planet* planet, const xmlNodePtr parent );
/* system load */
static void system_init( StarSystem *sys );
static int systems_load (void);
static StarSystem* system_parse( StarSystem *system, const xmlNodePtr parent );
static int system_parseJumpPoint( const xmlNodePtr node, StarSystem *sys );
static void system_parseJumps( const xmlNodePtr parent );
/* misc */
static void system_setFaction( StarSystem *sys );
static PlanetClass planetclass_get( const char a );
static int getPresenceIndex( StarSystem *sys, int faction );
static void presenceCleanup( StarSystem *sys );
static void system_scheduler( double dt, int init );
/* Render. */
static void space_renderJumpPoint( JumpPoint *jp, int i );
static void space_renderPlanet( Planet *p );
/*
 * Externed prototypes.
 */
int space_sysSave( xmlTextWriterPtr writer );
int space_sysLoad( xmlNodePtr parent );
/*
 * External prototypes.
 */
extern credits_t economy_getPrice( const Commodity *com,
      const StarSystem *sys, const Planet *p ); /**< from economy.c */


/**
 * @brief Basically returns a PlanetClass integer from a char
 *
 *    @param a Char to get class from.
 *    @return Identifier matching the char.
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

      default:
         WARN("Invalid planet class.");
         return PLANET_CLASS_NULL;
   };
}
/**
 * @brief Gets the char representing the planet class from the planet.
 *
 *    @param p Planet to get the class char from.
 *    @return The planet's class char.
 */
char planet_getClass( const Planet *p )
{
   switch (p->class) {
      case PLANET_CLASS_A: return 'A';
      case PLANET_CLASS_B: return 'B';
      case PLANET_CLASS_C: return 'C';
      case PLANET_CLASS_D: return 'D';
      case PLANET_CLASS_E: return 'E';
      case PLANET_CLASS_F: return 'F';
      case PLANET_CLASS_G: return 'G';
      case PLANET_CLASS_H: return 'H';
      case PLANET_CLASS_I: return 'I';
      case PLANET_CLASS_J: return 'J';
      case PLANET_CLASS_K: return 'K';
      case PLANET_CLASS_L: return 'L';
      case PLANET_CLASS_M: return 'M';
      case PLANET_CLASS_N: return 'N';
      case PLANET_CLASS_O: return 'O';
      case PLANET_CLASS_P: return 'P';
      case PLANET_CLASS_Q: return 'Q';
      case PLANET_CLASS_R: return 'R';
      case PLANET_CLASS_S: return 'S';
      case PLANET_CLASS_T: return 'T';
      case PLANET_CLASS_X: return 'X';
      case PLANET_CLASS_Y: return 'Y';
      case PLANET_CLASS_Z: return 'Z';
      /* Stations */
      case STATION_CLASS_A: return '0';
      case STATION_CLASS_B: return '1';
      case STATION_CLASS_C: return '2';
      case STATION_CLASS_D: return '3';

      default:
         WARN("Invalid planet class.");
         return 0;
   };
}


/**
 * @brief Gets the price of a commodity at a planet.
 *
 *    @param p Planet to get price at.
 *    @param c Commodity to get price of.
 */
credits_t planet_commodityPrice( const Planet *p, const Commodity *c )
{
   char *sysname;
   StarSystem *sys;

   sysname = planet_getSystem( p->name );
   sys = system_get( sysname );

   return economy_getPrice( c, sys, p );
}


/**
 * @brief Checks to make sure if pilot is far enough away to hyperspace.
 *
 *    @param p Pilot to check if he can hyperspace.
 *    @return 1 if he can hyperspace, 0 else.
 */
int space_canHyperspace( Pilot* p )
{
   double d, r;
   JumpPoint *jp;

   /* Must not have the nojump flag. */
   if (pilot_isFlag(p, PILOT_NOJUMP))
      return 0;

   /* Must have fuel. */
   if (p->fuel < HYPERSPACE_FUEL)
      return 0;

   /* Must have hyperspace target. */
   if (p->nav_hyperspace < 0)
      return 0;

   /* Get the jump. */
   jp = &cur_system->jumps[ p->nav_hyperspace ];

   /* Check distance. */
   r = jp->radius * p->stats.jump_range;
   d = vect_dist2( &p->solid->pos, &jp->pos );
   if (d > r*r)
      return 0;
   return 1;
}


/**
 * @brief Tries to get the pilot into hyperspace.
 *
 *    @param p Pilot to try to start hyperspacing.
 *    @return 0 on success.
 */
int space_hyperspace( Pilot* p )
{
   if (pilot_isFlag(p, PILOT_NOJUMP))
      return -2;
   if (p->fuel < HYPERSPACE_FUEL)
      return -3;
   if (!space_canHyperspace(p))
      return -1;

   /* pilot is now going to get automatically ready for hyperspace */
   pilot_setFlag(p, PILOT_HYP_PREP);
   return 0;
}


/**
 * @brief Calculates the jump in pos for a pilot.
 *
 *    @param in Star system entering.
 *    @param out Star system exitting.
 *    @param[out] pos Position calculated.
 *    @param[out] vel Velocity calculated.
 */
int space_calcJumpInPos( StarSystem *in, StarSystem *out, Vector2d *pos, Vector2d *vel, double *dir )
{
   int i;
   JumpPoint *jp;
   double a, d, x, y;
   double ea, ed;

   /* Find the entry system. */
   jp = NULL;
   for (i=0; i<in->njumps; i++)
      if (in->jumps[i].target == out)
         jp = &in->jumps[i];

   /* Must have found the jump. */
   if (jp == NULL) {
      WARN("Unable to find jump in point for '%s' in '%s': not connected", out->name, in->name);
      return -1;
   }

   /* Base position target. */
   x = jp->pos.x;
   y = jp->pos.y;

   /* Calculate offset from target position. */
   a = 2*M_PI - jp->angle;
   d = RNGF()*(HYPERSPACE_ENTER_MAX-HYPERSPACE_ENTER_MIN) + HYPERSPACE_ENTER_MIN;

   /* Calculate new position. */
   x += d*cos(a);
   y += d*sin(a);

   /* Add some error. */
   ea = 2*M_PI*RNGF();
   ed = jp->radius/2.;
   x += ed*cos(ea);
   y += ed*sin(ea);

   /* Set new position. */
   vect_cset( pos, x, y );

   /* Set new velocity. */
   a += M_PI;
   vect_cset( vel, HYPERSPACE_VEL*cos(a), HYPERSPACE_VEL*sin(a) );

   /* Set direction. */
   *dir = a;

   return 0;
}

/**
 * @brief Gets the name of all the planets that belong to factions.
 *
 *    @param[out] nplanets Number of planets found.
 *    @param factions Factions to check against.
 *    @param nfactions Number of factions in factions.
 *    @return An array of faction names.  Individual names are not allocated.
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
         planet = systems_stack[i].planets[j];
         for (k=0; k<nfactions; k++)
            if (planet->real == ASSET_REAL &&
                planet->faction == factions[k] &&
                space_sysReallyReachable(planet_getSystem(planet->name))) {
               ntmp++;
               if (ntmp > mtmp) { /* need more space */
                  mtmp *= 2;
                  tmp = realloc(tmp, sizeof(char*) * mtmp);
               }
               tmp[ntmp-1] = planet->name;
               break; /* no need to check all factions */
            }
      }

   (*nplanets) = ntmp;
   return tmp;
}


/**
 * @brief Gets the name of a random planet.
 *
 *    @return The name of a random planet.
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
         if (systems_stack[i].planets[j]->real == ASSET_REAL) {
            ntmp++;
            if (ntmp > mtmp) { /* need more space */
               mtmp *= 2;
               tmp = realloc(tmp, sizeof(char*) * mtmp);
            }
            tmp[ntmp-1] = systems_stack[i].planets[j]->name;
         }
      }

   tmp = arrayShuffle(tmp, ntmp);
   for (i=0; i < ntmp; i++) {
      if (space_sysReallyReachable(planet_getSystem( tmp[i] ))) {
         res = tmp[i];
         break;
      }
   }
   free(tmp);

   return res;
}


/**
 * @brief Gets the closest feature to a position in the system.
 *
 *    @param sys System to get closest feature from a position.
 *    @param[out] pnt ID of closest planet or -1 if a jump point is closer (or none is close).
 *    @param[out] jp ID of closest jump point or -1 if a planet is closer (or none is close).
 *    @param x X position to get closest from.
 *    @param y Y position to get closest from.
 */
double system_getClosest( const StarSystem *sys, int *pnt, int *jp, double x, double y )
{
   int i;
   double d, td;
   Planet *p;
   JumpPoint *j;

   /* Default output. */
   *pnt = -1;
   *jp  = -1;
   d    = 10e10;

   /* Planets. */
   for (i=0; i<sys->nplanets; i++) {
      p  = sys->planets[i];
      if (p->real != ASSET_REAL)
         continue;
      td = pow2(x-p->pos.x) + pow2(y-p->pos.y);
      if (td < d) {
         *pnt  = i;
         d     = td;
      }
   }

   /* Jump points. */
   for (i=0; i<sys->njumps; i++) {
      j  = &sys->jumps[i];
      td = pow2(x-j->pos.x) + pow2(y-j->pos.y);
      if (td < d) {
         *pnt  = -1; /* We must clear planet target as jump point is closer. */
         *jp   = i;
         d     = td;
      }
   }
   return d;
}


/**
 * @brief Gets the closest feature to a position in the system.
 *
 *    @param sys System to get closest feature from a position.
 *    @param[out] pnt ID of closest planet or -1 if a jump point is closer (or none is close).
 *    @param[out] jp ID of closest jump point or -1 if a planet is closer (or none is close).
 *    @param x X position to get closest from.
 *    @param y Y position to get closest from.
 */
double system_getClosestAng( const StarSystem *sys, int *pnt, int *jp, double x, double y, double ang )
{
   int i;
   double a, ta;
   Planet *p;
   JumpPoint *j;

   /* Default output. */
   *pnt = -1;
   *jp  = -1;
   a    = 10e10;

   /* Planets. */
   for (i=0; i<sys->nplanets; i++) {
      p  = sys->planets[i];
      if (p->real != ASSET_REAL)
         continue;
      ta = atan2( y - p->pos.y, x - p->pos.x);
      if ( ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
         *pnt  = i;
         a     = ta;
      }
   }

   /* Jump points. */
   for (i=0; i<sys->njumps; i++) {
      j  = &sys->jumps[i];
      ta = atan2( y - j->pos.y, x - j->pos.x);
      if ( ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
         *pnt  = -1; /* We must clear planet target as jump point is closer. */
         *jp   = i;
         a     = ta;
      }
   }
   return a;
}


/**
 * @brief Sees if a system is reachable.
 *
 *    @return 1 if target system is reachable, 0 if it isn't.
 */
int space_sysReachable( StarSystem *sys )
{
   int i;

   if (sys_isKnown(sys)) return 1; /* it is known */

   /* check to see if it is adjacent to known */
   for (i=0; i<sys->njumps; i++)
      if (sys_isKnown( sys->jumps[i].target ))
         return 1;

   return 0;
}


/**
 * @brief Sees if a system can be reached via jumping.
 *
 *    @return 1 if target system is reachable, 0 if it isn't.
 */
int space_sysReallyReachable ( char* sysname )
{
   int njumps;

   if (strcmp(sysname,cur_system->name)==0 || map_getJumpPath( &njumps,
         cur_system->name, sysname, 1, NULL ) != NULL)
      return 1;
   return 0;
}


/**
 * @brief Gets all the star systems.
 *
 *    @param[out] Number of star systems gotten.
 *    @return The star systems gotten.
 */
StarSystem* system_getAll( int *nsys )
{
   *nsys = systems_nstack;
   return systems_stack;
}


/**
 * @brief Checks to see if a system exists.
 *
 *    @param sysname Name of the system to match.
 *    @return 1 if the system exists.
 */
int system_exists( const char* sysname )
{
   int i;
   for (i=0; i<systems_nstack; i++)
      if (strcmp(sysname, systems_stack[i].name)==0)
         return 1;
   return 0;
}


/**
 * @brief Checks to see if a system exists case insensitively.
 *
 *    @param sysname Name of the system to match (case insensitive).
 *    @return The actual name of the system of NULL if not found.
 */
const char *system_existsCase( const char* sysname )
{
   int i;
   for (i=0; i<systems_nstack; i++)
      if (strcasecmp(sysname, systems_stack[i].name)==0)
         return systems_stack[i].name;
   return NULL;
}


/**
 * @brief Does a fuzzy case matching.
 */
char **system_searchFuzzyCase( const char* sysname, int *n )
{
   int i, len;
   char **names;

   /* Overallocate to maximum. */
   names = malloc( sizeof(char*) * systems_nstack );

   /* Do fuzzy search. */
   len = 0;
   for (i=0; i<systems_nstack; i++) {
      if (nstrcasestr( systems_stack[i].name, sysname ) != NULL) {
         names[len] = systems_stack[i].name;
         len++;
      }
   }

   /* Free if empty. */
   if (len == 0) {
      free(names);
      names = NULL;
   }

   *n = len;
   return names;
}




/**
 * @brief Get the system from it's name.
 *
 *    @param sysname Name to match.
 *    @return System matching sysname.
 */
StarSystem* system_get( const char* sysname )
{
   int i;

   for (i=0; i<systems_nstack; i++)
      if (strcmp(sysname, systems_stack[i].name)==0)
         return &systems_stack[i];

   WARN("System '%s' not found in stack", sysname);
   return NULL;
}


/**
 * @brief Get the system by it's index.
 *
 *    @param id Index to match.
 *    @return System matching index.
 */
StarSystem* system_getIndex( int id )
{
   return &systems_stack[ id ];
}


/**
 * @brief Gets the index of a star system.
 *
 *    @param sys System to get index of.
 *    @return The index of the system.
 */
int system_index( StarSystem *sys )
{
   return sys->id;
}


/**
 * @brief Get the name of a system from a planetname.
 *
 *    @param planetname Planet name to match.
 *    @return Name of the system planet belongs to.
 */
char* planet_getSystem( const char* planetname )
{
   int i;

   for (i=0; i<spacename_nstack; i++)
      if (strcmp(planetname_stack[i],planetname)==0)
         return systemname_stack[i];

   DEBUG("Planet '%s' not found in planetname stack", planetname);
   return NULL;
}


/**
 * @brief Gets a planet based on it's name.
 *
 *    @param planetname Name to match.
 *    @return Planet matching planetname.
 */
Planet* planet_get( const char* planetname )
{
   int i;

   if (planetname==NULL) {
      WARN("Trying to find NULL planet...");
      return NULL;
   }

   for (i=0; i<planet_nstack; i++)
      if (strcmp(planet_stack[i].name,planetname)==0)
         return &planet_stack[i];

   WARN("Planet '%s' not found in the universe", planetname);
   return NULL;
}


/**
 * @brief Gets planet by index.
 *
 *    @param ind Index of the planet to get.
 *    @return The planet gotten.
 */
Planet* planet_getIndex( int ind )
{
   /* Sanity check. */
   if ((ind < 0) || (ind >= planet_nstack)) {
      WARN("Planet index '%d' out of range (max %d)", ind, planet_nstack);
      return NULL;
   }

   return &planet_stack[ ind ];
}


/**
 * @brief Gets the ID of a planet.
 *
 *    @param p Planet to get ID of.
 *    @return The ID of the planet.
 */
int planet_index( const Planet *p )
{
   return p->id;
}


/**
 * @brief Gets the number of planets.
 *
 *    @return The number of planets.
 */
int planet_getNum (void)
{
   return planet_nstack;
}


/**
 * @brief Gets all the planets.
 *
 *    @param n Number of planets gotten.
 *    @return Array of gotten planets.
 */
Planet* planet_getAll( int *n )
{
   *n = planet_nstack;
   return planet_stack;
}


/**
 * @brief Check to see if a planet exists.
 *
 *    @param planetname Name of the planet to see if it exists.
 *    @return 1 if planet exists.
 */
int planet_exists( const char* planetname )
{
   int i;
   for (i=0; i<planet_nstack; i++)
      if (strcmp(planet_stack[i].name,planetname)==0)
         return 1;
   return 0;
}


/**
 * @brief Check to see if a planet exists (case insensitive).
 *
 *    @param planetname Name of the planet to see if it exists.
 *    @return The actual name of the planet or NULL if not found.
 */
const char* planet_existsCase( const char* planetname )
{
   int i;
   for (i=0; i<planet_nstack; i++)
      if (strcasecmp(planet_stack[i].name,planetname)==0)
         return planet_stack[i].name;
   return NULL;
}


/**
 * @brief Does a fuzzy case matching.
 */
char **planet_searchFuzzyCase( const char* planetname, int *n )
{
   int i, len;
   char **names;

   /* Overallocate to maximum. */
   names = malloc( sizeof(char*) * planet_nstack );

   /* Do fuzzy search. */
   len = 0;
   for (i=0; i<planet_nstack; i++) {
      if (nstrcasestr( planet_stack[i].name, planetname ) != NULL) {
         names[len] = planet_stack[i].name;
         len++;
      }
   }

   /* Free if empty. */
   if (len == 0) {
      free(names);
      names = NULL;
   }

   *n = len;
   return names;
}


/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 *    @param init Should be 1 to initialize the scheduler.
 */
static void system_scheduler( double dt, int init )
{
   int i, n;
   lua_State *L;
   SystemPresence *p;
   LuaPilot *lp;
   Pilot *pilot;

   /* Go through all the factions and reduce the timer. */
   for (i=0; i < cur_system->npresence; i++) {
      p = &cur_system->presence[i];
      L = faction_getState( p->faction );

      /* Must have a valid scheduler. */
      if (L==NULL)
         continue;

      /* Run the appropriate function. */
      if (init) {
         lua_getglobal( L, "create" ); /* f */
         if (lua_isnil(L,-1)) {
            WARN("Lua Spawn script for faction '%s' missing obligatory entry point 'create'.",
                  faction_name( p->faction ) );
            continue;
         }
         n = 0;
      }
      else {
         /* Decerement dt, only continue  */
         p->timer -= dt;
         if (p->timer >= 0.)
            continue;

         lua_getglobal( L, "spawn" ); /* f */
         if (lua_isnil(L,-1)) {
            WARN("Lua Spawn script for faction '%s' missing obligatory entry point 'spawn'.",
                  faction_name( p->faction ) );
            lua_pop(L,1);
            continue;
         }
         lua_pushnumber( L, p->curUsed ); /* f, presence */
         n = 1;
      }
      lua_pushnumber( L, p->value ); /* f, [arg,], max */

      /* Actually run the function. */
      if (lua_pcall(L, n+1, 2, 0)) { /* error has occured */
         WARN("Lua Spawn script for faction '%s' : %s",
               faction_name( p->faction ), lua_tostring(L,-1));
         lua_pop(L,1);
         continue;
      }

      /* Output is handled the same way. */
      if (!lua_isnumber(L,-2)) {
         WARN("Lua spawn script for faction '%s' failed to return timer value.",
               faction_name( p->faction ) );
         lua_pop(L,2);
         continue;
      }
      p->timer    += lua_tonumber(L,-2);
      /* Handle table if it exists. */
      if (lua_istable(L,-1)) {
         lua_pushnil(L); /* t, k */
         while (lua_next(L,-2) != 0) { /* tk, k, v */
            /* Must be table. */
            if (!lua_istable(L,-1)) {
               WARN("Lua spawn script for faction '%s' returns invalid data (not a table).",
                     faction_name( p->faction ) );
               lua_pop(L,2); /* tk, k */
               continue;
            }

            lua_getfield( L, -1, "pilot" ); /* tk, k, v, p */
            if (!lua_ispilot(L,-1)) {
               WARN("Lua spawn script for faction '%s' returns invalid data (not a pilot).",
                     faction_name( p->faction ) );
               lua_pop(L,2); /* tk, k */
               continue;
            }
            lp    = lua_topilot(L,-1);
            pilot = pilot_get( lp->pilot );
            if (pilot == NULL) {
               lua_pop(L,2); /* tk, k */
               continue;
            }
            lua_pop(L,1); /* tk, k, v */
            lua_getfield( L, -1, "presence" ); /* tk, k, v, p */
            if (!lua_isnumber(L,-1)) {
               WARN("Lua spawn script for faction '%s' returns invalid data (not a number).",
                     faction_name( p->faction ) );
               lua_pop(L,2); /* tk, k */
               continue;
            }
            pilot->presence = lua_tonumber(L,-1);
            p->curUsed     += pilot->presence;
            lua_pop(L,2); /* tk, k */
         }
      }
      lua_pop(L,2); /* Clear arguments. */
   }
}


/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 */
void space_update( const double dt )
{
   int i;
   Pilot *p;

   /* Needs a current system. */
   if (cur_system == NULL)
      return;

   /* If spawning is enabled, call the scheduler. */
   if (space_spawn)
      system_scheduler( dt, 0 );

   /*
    * Volatile systems.
    */
   if (cur_system->nebu_volatility > 0.) {
      /* Damage pilots in volatile systems. */
      for (i=0; i<pilot_nstack; i++) {
         p = pilot_stack[i];
         pilot_hit( p, NULL, 0, DAMAGE_TYPE_RADIATION,
                  p->stats.nebula_damage * pow2(cur_system->nebu_volatility) / 500. * dt, 1. ); /* 100% penetration. */
      }
   }


   /*
    * Interference.
    */
   if (cur_system->interference > 0.) {
      /* Always dark. */
      if (cur_system->interference >= 1000.)
         interference_alpha = 1.;

      /* Normal scenario. */
      else {
         interference_timer -= dt;
         if (interference_timer < 0.) {
            /* 0    ->  [   1,   5   ]
             * 250  ->  [ 0.75, 3.75 ]
             * 500  ->  [  0.5, 2.5  ]
             * 750  ->  [ 0.25, 1.25 ]
             * 1000 ->  [   0,   0   ] */
            interference_timer += (1000. - cur_system->interference) / 1000. *
                  (3. + RNG_2SIGMA() );

            /* 0    ->  [  0,   0  ]
             * 250  ->  [-0.5, 1.5 ]
             * 500  ->  [ -1,   3  ]
             * 1000 ->  [  0,   6  ] */
            interference_target = cur_system->interference/1000. * 2. *
                  (1. + RNG_2SIGMA() );
         }

         /* Head towards target. */
         if (fabs(interference_alpha - interference_target) > 1e-05) {
            /* Assymptotic. */
            interference_alpha += (interference_target - interference_alpha) * dt;

            /* Limit alpha to [0.-1.]. */
            if (interference_alpha > 1.)
               interference_alpha = 1.;
            else if (interference_alpha < 0.)
               interference_alpha = 0.;
         }
      }
   }
}


/**
 * @brief Initializes the system.
 *
 *    @param sysname Name of the system to initialize.
 */
void space_init ( const char* sysname )
{
   char* nt;
   int i, n, s;

   /* cleanup some stuff */
   player_clear(); /* clears targets */
   ovr_mrkClear(); /* Clear markers when jumping. */
   pilots_clean(); /* destroy all the current pilots, except player */
   weapon_clear(); /* get rid of all the weapons */
   spfx_clear(); /* get rid of the explosions */
   background_clear(); /* Get rid of the background. */
   space_spawn = 1; /* spawn is enabled by default. */
   interference_timer = 0.; /* Restart timer. */
   if (player.p != NULL) {
      pilot_clearTimers(player.p); /* Clear timers. */
      pilot_heatReset(player.p); /* Resets the player's heat. */
      player_clearEscorts(); /* Must clear escorts to keep deployment sane. */
   }

   if ((sysname==NULL) && (cur_system==NULL))
      ERR("Cannot reinit system if there is no system previously loaded");
   else if (sysname!=NULL) {
      for (i=0; i < systems_nstack; i++)
         if (strcmp(sysname, systems_stack[i].name)==0)
            break;

      if (i>=systems_nstack)
         ERR("System %s not found in stack", sysname);
      cur_system = &systems_stack[i];

      nt = ntime_pretty(0, 2);
      player_message("\epEntering System %s on %s.", sysname, nt);
      free(nt);

      /* Handle background */
      if (cur_system->nebu_density > 0.) {
         /* Background is Nebula */
         nebu_prep( cur_system->nebu_density, cur_system->nebu_volatility );

         /* Set up sound. */
         sound_env( SOUND_ENV_NEBULA, cur_system->nebu_density );
      }
      else {
         /* Backrgound is Stary */
         background_initStars( cur_system->stars );

         /* Set up sound. */
         sound_env( SOUND_ENV_NORMAL, 0. );
      }
   }

   /* Iterate through planets to clear bribes. */
   for (i=0; i<cur_system->nplanets; i++)
      cur_system->planets[i]->bribed = 0;

   /* Clear interference if you leave system with interference. */
   if (cur_system->interference == 0.)
      interference_alpha = 0.;

   /* See if we should get a new music song. */
   if (player.p != NULL)
      music_choose(NULL);

   /* Reset player enemies. */
   player.enemies = 0;

   /* Update the pilot sensor range. */
   pilot_updateSensorRange();

   /* Reset any system fleets. */
   cur_system->nsystemFleets = 0;

   /* Reset any schedules and used presence. */
   for (i=0; i < cur_system->npresence; i++) {
      cur_system->presence[i].curUsed  = 0;
      cur_system->presence[i].timer    = 0.;
   }

   /* Load graphics. */
   space_gfxLoad( cur_system );

   /* Call the scheduler. */
   system_scheduler( 0., 1 );

   /* we now know this system */
   sys_setFlag(cur_system,SYSTEM_KNOWN);

   /* Simulate system. */
   if (player.p != NULL)
      pilot_setFlag( player.p, PILOT_INVISIBLE );
   player_messageToggle( 0 );
   s = sound_disabled;
   sound_disabled = 1;
   ntime_allowUpdate( 0 );
   n = SYSTEM_SIMULATE_TIME / fps_min;
   for (i=0; i<n; i++)
      update_routine( fps_min, 1 );
   ntime_allowUpdate( 1 );
   sound_disabled = s;
   player_messageToggle( 1 );
   if (player.p != NULL)
      pilot_rmFlag( player.p, PILOT_INVISIBLE );

   /* Refresh overlay if necessary (player kept it open). */
   ovr_refresh();

   /* Update gui. */
   gui_setSystem();

   /* Start background. */
   background_load( cur_system->background );
}


/**
 * @brief Creates a new planet.
 */
Planet *planet_new (void)
{
   Planet *p;
   int realloced;

   /* See if stack must grow. */
   planet_nstack++;
   realloced = 0;
   if (planet_nstack > planet_mstack) {
      planet_mstack *= 2;
      planet_stack   = realloc( planet_stack, sizeof(Planet) * planet_mstack );
      realloced      = 1;
   }

   /* Clean up memory. */
   p           = &planet_stack[ planet_nstack-1 ];
   memset( p, 0, sizeof(Planet) );
   p->id       = planet_nstack-1;
   p->faction  = -1;
   p->class    = PLANET_CLASS_A;

   /* Reconstruct the jumps. */
   if (!systems_loading && realloced)
      systems_reconstructPlanets();

   return p;
}


/**
 * @brief Loads all the planets in the game.
 *
 *    @return 0 on success.
 */
static int planets_load ( void )
{
   uint32_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;
   Planet *p;

   buf = ndata_read( PLANET_DATA, &bufsize );
   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL) {
      ERR(PLANET_DATA" file is invalid xml!");
      return -1;
   }

   node = doc->xmlChildrenNode;
   if (strcmp((char*)node->name,XML_PLANET_ID)) {
      ERR("Malformed "PLANET_DATA" file: missing root element '"XML_PLANET_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed "PLANET_DATA" file: does not contain elements");
      return -1;
   }

   /* Initialize stack if needed. */
   if (planet_stack == NULL) {
      planet_mstack = CHUNK_SIZE;
      planet_stack = malloc( sizeof(Planet) * planet_mstack );
      planet_nstack = 0;
   }

   do {
      if (xml_isNode(node,XML_PLANET_TAG)) {
         p = planet_new();
         planet_parse( p, node );
      }
   } while (xml_nextNode(node));

   /*
    * free stuff
    */
   xmlFreeDoc(doc);
   free(buf);

   return 0;
}


/**
 * @brief Loads all the graphics for a star system.
 *
 *    @param sys System to load graphics for.
 */
void space_gfxLoad( StarSystem *sys )
{
   int i;
   Planet *planet;
   for (i=0; i<sys->nplanets; i++) {
      planet = sys->planets[i];

      if (planet->real != ASSET_REAL)
         continue;

      if (planet->gfx_space == NULL)
         planet->gfx_space = gl_newImage( planet->gfx_spaceName, OPENGL_TEX_MIPMAPS );
   }
}


/**
 * @brief Unloads all the graphics for a star system.
 *
 *    @param sys System to unload graphics for.
 */
void space_gfxUnload( StarSystem *sys )
{
   int i;
   Planet *planet;
   for (i=0; i<sys->nplanets; i++) {
      planet = sys->planets[i];
      if (planet->gfx_space != NULL) {
         gl_freeTexture( planet->gfx_space );
         planet->gfx_space = NULL;
      }
   }
}


/**
 * @brief Parses a planet from an xml node.
 *
 *    @param planet Planet to fill up.
 *    @param parent Node that contains planet data.
 *    @return 0 on success.
 */
static int planet_parse( Planet *planet, const xmlNodePtr parent )
{
   int mem;
   char str[PATH_MAX];
   xmlNodePtr node, cur, ccur;
   unsigned int flags;
   SDL_RWops *rw;
   npng_t *npng;
   png_uint_32 w, h;
   int nbuf;
   char *buf;

   /* Clear up memory for sane defaults. */
   flags          = 0;
   planet->real   = ASSET_REAL;

   /* Get the name. */
   xmlr_attr( parent, "name", planet->name );

   node = parent->xmlChildrenNode;
   do {

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"virtual")) {
         planet->real   = ASSET_VIRTUAL;
         continue;
      }
      else if (xml_isNode(node,"GFX")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"space")) { /* load space gfx */
               snprintf( str, PATH_MAX, PLANET_GFX_SPACE"%s", xml_get(cur));
               planet->gfx_spaceName = strdup(str);
               planet->gfx_spacePath = xml_getStrd(cur);
               rw = ndata_rwops( planet->gfx_spaceName );
               if (rw == NULL) {
                  WARN("Planet '%s' has inexisting graphic '%s'!", planet->name, planet->gfx_spaceName );
               }
               else {
                  npng = npng_open( rw );
                  if (npng != NULL) {
                     npng_dim( npng, &w, &h );
                     nbuf = npng_metadata( npng, "radius", &buf );
                     if (nbuf > 0) {
                        strncpy( str, buf, MIN( (unsigned int)nbuf, sizeof(str) ) );
                        str[ nbuf ] = '\0';
                        planet->radius = atof( str );
                     }
                     else {
                        planet->radius = 0.8 * (double)(w+h)/4.; /* (w+h)/2 is diameter, /2 for radius */
                     }
                     npng_close( npng );
                  }
                  SDL_RWclose( rw );
               }
            }
            else if (xml_isNode(cur,"exterior")) { /* load land gfx */
               snprintf( str, PATH_MAX, PLANET_GFX_EXTERIOR"%s", xml_get(cur));
               planet->gfx_exterior = strdup(str);
               planet->gfx_exteriorPath = xml_getStrd(cur);
            }
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"pos")) {
         cur          = node->children;
         do {
            if (xml_isNode(cur,"x")) {
               flags |= FLAG_XSET;
               planet->pos.x = xml_getFloat(cur);
            }
            else if (xml_isNode(cur,"y")) {
               flags |= FLAG_YSET;
               planet->pos.y = xml_getFloat(cur);
            }
         } while(xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node, "presence")) {
         cur = node->children;
         do {
            xmlr_float(cur, "value", planet->presenceAmount);
            xmlr_int(cur, "range", planet->presenceRange);
            if (xml_isNode(cur,"faction")) {
               flags |= FLAG_FACTIONSET;
               planet->faction = faction_get( xml_get(cur) );
               continue;
            }
         } while(xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"general")) {
         cur = node->children;
         do {
            /* Direct reads. */
            xmlr_strd(cur, "bar", planet->bar_description);
            xmlr_strd(cur, "description", planet->description );
            xmlr_ulong(cur, "population", planet->population );

            if (xml_isNode(cur,"class"))
               planet->class =
                  planetclass_get(cur->children->content[0]);
            else if (xml_isNode(cur, "services")) {
               flags |= FLAG_SERVICESSET;
               ccur = cur->children;
               planet->services = 0;
               do {
                  xml_onlyNodes(ccur);

                  if (xml_isNode(ccur, "land"))
                     planet->services |= PLANET_SERVICE_LAND;
                  else if (xml_isNode(ccur, "refuel"))
                     planet->services |= PLANET_SERVICE_REFUEL | PLANET_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "bar"))
                     planet->services |= PLANET_SERVICE_BAR | PLANET_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "missions"))
                     planet->services |= PLANET_SERVICE_MISSIONS | PLANET_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "commodity"))
                     planet->services |= PLANET_SERVICE_COMMODITY | PLANET_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "outfits"))
                     planet->services |= PLANET_SERVICE_OUTFITS | PLANET_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "shipyard"))
                     planet->services |= PLANET_SERVICE_SHIPYARD | PLANET_SERVICE_INHABITED;
                  else
                     WARN("Planet '%s' has unknown services tag '%s'", planet->name, ccur->name);

               } while (xml_nextNode(ccur));
            }

            else if (xml_isNode(cur, "commodities")) {
               ccur = cur->children;
               mem = 0;
               do {
                  if (xml_isNode(ccur,"commodity")) {
                     planet->ncommodities++;
                     /* Memory must grow. */
                     if (planet->ncommodities > mem) {
                        if (mem == 0)
                           mem = CHUNK_SIZE_SMALL;
                        else
                           mem *= 2;
                        planet->commodities = realloc(planet->commodities,
                              mem * sizeof(Commodity*));
                     }
                     planet->commodities[planet->ncommodities-1] =
                        commodity_get( xml_get(ccur) );
                  }
               } while (xml_nextNode(ccur));
               /* Shrink to minimum size. */
               planet->commodities = realloc(planet->commodities,
                     planet->ncommodities * sizeof(Commodity*));
            }
         } while(xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node, "tech")) {
         planet->tech = tech_groupCreate( node );
         continue;
      }

      DEBUG("Unknown node '%s' in planet '%s'",node->name,planet->name);
   } while (xml_nextNode(node));

/*
 * verification
 */
#define MELEMENT(o,s)   if (o) WARN("Planet '%s' missing '"s"' element", planet->name)
   /* Issue warnings on missing items only it the asset is real. */
   if (planet->real == ASSET_REAL) {
      MELEMENT(planet->gfx_spaceName==NULL,"GFX space");
      MELEMENT( planet_hasService(planet,PLANET_SERVICE_LAND) &&
            planet->gfx_exterior==NULL,"GFX exterior");
      MELEMENT( planet_hasService(planet,PLANET_SERVICE_INHABITED) &&
            (planet->population==0), "population");
      MELEMENT((flags&FLAG_XSET)==0,"x");
      MELEMENT((flags&FLAG_YSET)==0,"y");
      MELEMENT(planet->class==PLANET_CLASS_NULL,"class");
      MELEMENT( planet_hasService(planet,PLANET_SERVICE_LAND) &&
            planet->description==NULL,"description");
      MELEMENT( planet_hasService(planet,PLANET_SERVICE_BAR) &&
            planet->bar_description==NULL,"bar");
      MELEMENT( planet_hasService(planet,PLANET_SERVICE_INHABITED) &&
            (flags&FLAG_FACTIONSET)==0,"faction");
      MELEMENT((flags&FLAG_SERVICESSET)==0,"services");
      MELEMENT( (planet_hasService(planet,PLANET_SERVICE_OUTFITS) ||
               planet_hasService(planet,PLANET_SERVICE_SHIPYARD)) &&
            (planet->tech==NULL), "tech" );
      MELEMENT( planet_hasService(planet,PLANET_SERVICE_COMMODITY) &&
            (planet->ncommodities==0),"commodity" );
      MELEMENT( (flags&FLAG_FACTIONSET) && (planet->presenceAmount == 0.),
            "presence" );
   }
#undef MELEMENT

   return 0;
}


/**
 * @brief Adds a planet to a star system.
 *
 *    @param sys Star System to add planet to.
 *    @param planetname Name of the planet to add.
 *    @return 0 on success.
 */
int system_addPlanet( StarSystem *sys, const char *planetname )
{
   Planet *planet;

   if (sys == NULL)
      return -1;

   /* Check if need to grow the star system planet stack. */
   sys->nplanets++;
   if (sys->planets == NULL) {
      sys->planets   = malloc( sizeof(Planet*) * CHUNK_SIZE_SMALL );
      sys->planetsid = malloc( sizeof(int) * CHUNK_SIZE_SMALL );
   }
   else if (sys->nplanets > CHUNK_SIZE_SMALL) {
      sys->planets   = realloc( sys->planets, sizeof(Planet*) * sys->nplanets );
      sys->planetsid = realloc( sys->planetsid, sizeof(int) * sys->nplanets );
   }
   planet = planet_get(planetname);
   if (planet == NULL) {
      sys->nplanets--; /* Try to keep sanity if possible. */
      return -1;
   }
   sys->planets[sys->nplanets-1]    = planet;
   sys->planetsid[sys->nplanets-1]  = planet->id;

   /* add planet <-> star system to name stack */
   spacename_nstack++;
   if (spacename_nstack > spacename_mstack) {
      if (spacename_mstack == 0)
         spacename_mstack = CHUNK_SIZE;
      else
         spacename_mstack *= 2;
      planetname_stack = realloc(planetname_stack,
            sizeof(char*) * spacename_mstack);
      systemname_stack = realloc(systemname_stack,
            sizeof(char*) * spacename_mstack);
   }
   planetname_stack[spacename_nstack-1] = planet->name;
   systemname_stack[spacename_nstack-1] = sys->name;

   system_setFaction(sys);

   /* Regenerate the economy stuff. */
   economy_refresh();

   /* Add the presence. */
   if (!systems_loading)
      system_addPresence( sys, planet->faction, planet->presenceAmount, planet->presenceRange );

   /* Reload graphics if necessary. */
   if (cur_system != NULL)
      space_gfxLoad( cur_system );

   return 0;
}


/**
 * @brief Removes a planet from a star system.
 *
 *    @param sys Star System to remove planet from.
 *    @param planetname Name of the planet to remove.
 *    @return 0 on success.
 */
int system_rmPlanet( StarSystem *sys, const char *planetname )
{
   int i, found;
   Planet *planet ;

   if (sys == NULL) {
      WARN("Unable to remove planet '%s' from NULL system.", planetname);
      return -1;
   }

   /* Try to find planet. */
   planet = planet_get( planetname );
   for (i=0; i<sys->nplanets; i++)
      if (sys->planets[i] == planet)
         break;

   /* Planet not found. */
   if (i>=sys->nplanets) {
      WARN("Planet '%s' not found in system '%s' for removal.", planetname, sys->name);
      return -1;
   }

   /* Remove planet from system. */
   sys->nplanets--;
   memmove( &sys->planets[i], &sys->planets[i+1], sizeof(Planet*) * (sys->nplanets-i) );
   memmove( &sys->planetsid[i], &sys->planetsid[i+1], sizeof(int) * (sys->nplanets-i) );

   /* Remove the presence. */
   system_addPresence( sys, planet->faction, -(planet->presenceAmount), planet->presenceRange );

   /* Remove from the name stack thingy. */
   found = 0;
   for (i=0; i<spacename_nstack; i++)
      if (strcmp(planetname, planetname_stack[i])==0) {
         spacename_nstack--;
         memmove( &planetname_stack[i], &planetname_stack[i+1],
               sizeof(char*) * (spacename_nstack-i) );
         memmove( &systemname_stack[i], &systemname_stack[i+1],
               sizeof(char*) * (spacename_nstack-i) );
         found = 1;
         break;
      }
   if (found == 0)
      WARN("Unable to find planet '%s' and system '%s' in planet<->system stack.",
            planetname, sys->name );

   system_setFaction(sys);

   /* Regenerate the economy stuff. */
   economy_refresh();

   return 0;
}


/**
 * @brief Adds a fleet to a star system.
 *
 *    @param sys Star System to add fleet to.
 *    @param fleet Fleet to add.
 *    @return 0 on success.
 */
int system_addFleet( StarSystem *sys, Fleet *fleet )
{
   if (sys == NULL)
      return -1;

   /* Add the fleet. */
   sys->nfleets++;
   sys->fleets = realloc( sys->fleets, sizeof(Fleet*) * sys->nfleets );
   sys->fleets[sys->nfleets - 1] = fleet;

   /* Adjust the system average. */
   sys->avg_pilot += fleet->npilots;

   return 0;
}


/**
 * @brief Removes a fleet from a star system.
 *
 *    @param sys Star System to remove fleet from.
 *    @param fleet Fleet to remove.
 *    @return 0 on success.
 */
int system_rmFleet( StarSystem *sys, Fleet *fleet )
{
   int i;

   if (sys == NULL)
      return -1;

   /* Find a matching fleet (will grab first since can be duplicates). */
   for (i=0; i<sys->nfleets; i++)
      if (fleet == sys->fleets[i])
         break;

   /* Not found. */
   if (i >= sys->nfleets)
      return -1;

   /* Remove the fleet. */
   sys->nfleets--;
   memmove(&sys->fleets[i], &sys->fleets[i + 1], sizeof(Fleet*) * (sys->nfleets - i));
   sys->fleets = realloc(sys->fleets, sizeof(Fleet*) * sys->nfleets);

   /* Adjust the system average. */
   sys->avg_pilot -= fleet->npilots;

   return 0;
}


/**
 * @brief Initializes a new star system with null memory.
 */
static void system_init( StarSystem *sys )
{
   memset( sys, 0, sizeof(StarSystem) );
   sys->faction   = -1;
}


/**
 * @brief Creates a new star system.
 */
StarSystem *system_new (void)
{
   StarSystem *sys;
   int realloced;

   /* Check if memory needs to grow. */
   systems_nstack++;
   realloced = 0;
   if (systems_nstack > systems_mstack) {
      systems_mstack   *= 2;
      systems_stack     = realloc( systems_stack, sizeof(StarSystem) * systems_mstack );
      realloced         = 1;
   }
   sys = &systems_stack[ systems_nstack-1 ];

   /* Initialize system and id. */
   system_init( sys );
   sys->id = systems_nstack-1;

   /* Reconstruct the jumps. */
   if (!systems_loading && realloced)
      systems_reconstructJumps();

   return sys;
}


/**
 * @brief Reconstructs the jumps.
 */
void systems_reconstructJumps (void)
{
   StarSystem *sys;
   JumpPoint *jp;
   int i, j;
   double a;

   for (i=0; i<systems_nstack; i++) {
      sys = &systems_stack[i];
      for (j=0; j<sys->njumps; j++) {
         jp          = &sys->jumps[j];
         jp->target  = system_getIndex( jp->targetid );

         /* Get heading. */
         a = atan2( jp->target->pos.y - sys->pos.y, jp->target->pos.x - sys->pos.x );
         if (a < 0.)
            a += 2.*M_PI;

         /* Update position if needed.. */
         if (jp->flags & JP_AUTOPOS) {
            jp->pos.x   = sys->radius*cos(a);
            jp->pos.y   = sys->radius*sin(a);
         }

         /* Update jump specific data. */
         gl_getSpriteFromDir( &jp->sx, &jp->sy, jumppoint_gfx, a );
         jp->angle = 2.*M_PI-a;
         jp->cosa  = cos(jp->angle);
         jp->sina  = sin(jp->angle);
      }
   }
}


/**
 * @brief Updates the system planet pointers.
 */
void systems_reconstructPlanets (void)
{
   StarSystem *sys;
   int i, j;

   for (i=0; i<systems_nstack; i++) {
      sys = &systems_stack[i];
      for (j=0; j<sys->nplanets; j++) {
         sys->planets[j] = &planet_stack[ sys->planetsid[j] ];
      }
   }
}


/**
 * @brief Creates a system from an XML node.
 *
 *    @param parent XML node to get system from.
 *    @return System matching parent data.
 */
static StarSystem* system_parse( StarSystem *sys, const xmlNodePtr parent )
{
   Planet* planet;
   char *ptrc;
   xmlNodePtr cur, node;
   uint32_t flags;
   int size;

   /* Clear memory for sane defaults. */
   flags          = 0;
   planet         = NULL;
   size           = 0;
   sys->presence  = NULL;
   sys->npresence = 0;
   sys->systemFleets  = NULL;
   sys->nsystemFleets = 0;

   sys->name = xml_nodeProp(parent,"name"); /* already mallocs */

   node  = parent->xmlChildrenNode;
   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"pos")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"x")) {
               flags |= FLAG_XSET;
               sys->pos.x = xml_getFloat(cur);
            }
            else if (xml_isNode(cur,"y")) {
               flags |= FLAG_YSET;
               sys->pos.y = xml_getFloat(cur);
            }
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"general")) {
         cur = node->children;
         do {
            xmlr_strd( cur, "background", sys->background );
            xmlr_int( cur, "stars", sys->stars );
            xmlr_float( cur, "radius", sys->radius );
            if (xml_isNode(cur,"asteroids")) {
               flags |= FLAG_ASTEROIDSSET;
               sys->asteroids = xml_getInt(cur);
            }
            else if (xml_isNode(cur,"interference")) {
               flags |= FLAG_INTERFERENCESET;
               sys->interference = xml_getFloat(cur);
            }
            else if (xml_isNode(cur,"nebula")) {
               ptrc = xml_nodeProp(cur,"volatility");
               if (ptrc != NULL) { /* Has volatility  */
                  sys->nebu_volatility = atof(ptrc);
                  free(ptrc);
               }
               sys->nebu_density = xml_getFloat(cur);
            }
         } while (xml_nextNode(cur));
         continue;
      }
      /* Loads all the assets. */
      else if (xml_isNode(node,"assets")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"asset"))
               system_addPlanet( sys, xml_get(cur) );
         } while (xml_nextNode(cur));
         continue;
      }

      /* Avoid warning. */
      if (xml_isNode(node,"jumps"))
         continue;

      DEBUG("Unknown node '%s' in star system '%s'",node->name,sys->name);
   } while (xml_nextNode(node));

#define MELEMENT(o,s)      if (o) WARN("Star System '%s' missing '"s"' element", sys->name)
   if (sys->name == NULL) WARN("Star System '%s' missing 'name' tag", sys->name);
   MELEMENT((flags&FLAG_XSET)==0,"x");
   MELEMENT((flags&FLAG_YSET)==0,"y");
   MELEMENT(sys->stars==0,"stars");
   MELEMENT(sys->radius==0.,"radius");
   MELEMENT((flags&FLAG_ASTEROIDSSET)==0,"asteroids");
   MELEMENT((flags&FLAG_INTERFERENCESET)==0,"inteference");
#undef MELEMENT

   /* post-processing */
   system_setFaction( sys );

   return 0;
}


/**
 * @brief Sets the system faction based on the planets it has.
 *
 *    @param sys System to set the faction of.
 */
static void system_setFaction( StarSystem *sys )
{
   int i;
   sys->faction = -1;
   for (i=0; i<sys->nplanets; i++) /** @todo Handle multiple different factions. */
      if (sys->planets[i]->real == ASSET_REAL && sys->planets[i]->faction > 0) {
         sys->faction = sys->planets[i]->faction;
         break;
      }
}


/**
 * @brief Parses a single jump point for a system.
 *
 *    @param node Parent node containing jump point information.
 *    @param sys System to which the jump point belongs.
 *    @return 0 on success.
 */
static int system_parseJumpPoint( const xmlNodePtr node, StarSystem *sys )
{
   JumpPoint *j;
   char *buf;
   xmlNodePtr cur, cur2;
   double x, y;
   StarSystem *target;

   /* Get target. */
   xmlr_attr( node, "target", buf );
   if (buf == NULL) {
      WARN("JumpPoint node for system '%s' has no target attribute.", sys->name);
      return -1;
   }
   target = system_get(buf);
   if (target == NULL) {
      WARN("JumpPoint node for system '%s' has invalid target '%s'.", sys->name, buf );
      free(buf);
      return -1;
   }

#ifdef DEBUGGING
   int i;
   for (i=0; i<sys->njumps; i++) {
      j = &sys->jumps[i];
      if (j->targetid != target->id)
         continue;

      WARN("Star System '%s' has duplicate jump point to '%s'.",
            sys->name, target->name );
      break;
   }
#endif /* DEBUGGING */

   /* Allocate more space. */
   sys->jumps = realloc( sys->jumps, (sys->njumps+1)*sizeof(JumpPoint) );
   j = &sys->jumps[ sys->njumps ];
   memset( j, 0, sizeof(JumpPoint) );

   /* Set some stuff. */
   j->target = target;
   free(buf);
   j->targetid = j->target->id;

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xmlr_float( cur, "radius", j->radius );

      /* Handle position. */
      if (xml_isNode(cur,"pos")) {
         xmlr_attr( cur, "x", buf );
         if (buf==NULL) {
            WARN("JumpPoint for system '%s' has position node missing 'x' position, using 0.", sys->name);
            x = 0.;
         }
         else {
            x = atof(buf);
            free(buf);
         }
         xmlr_attr( cur, "y", buf );
         if (buf==NULL) {
            WARN("JumpPoint for system '%s' has position node missing 'y' position, using 0.", sys->name);
            y = 0.;
         }
         else {
            y = atof(buf);
            free(buf);
         }

         /* Set position. */
         vect_cset( &j->pos, x, y );
      }

      /* Handle flags. */
      if (xml_isNode(cur,"flags")) {
         cur2 = cur->xmlChildrenNode;
         do {
            if (xml_isNode(cur2,"autopos"))
               j->flags |= JP_AUTOPOS;
         } while (xml_nextNode(cur2));
      }
   } while (xml_nextNode(cur));

   /* Added jump. */
   sys->njumps++;

   return 0;
}


/**
 * @brief Loads the jumps into a system.
 *
 *    @param parent System parent node.
 */
static void system_parseJumps( const xmlNodePtr parent )
{
   int i;
   StarSystem *sys;
   char* name;
   xmlNodePtr cur, node;

   name = xml_nodeProp(parent,"name"); /* already mallocs */
   sys = NULL;
   for (i=0; i<systems_nstack; i++) {
      if (strcmp( systems_stack[i].name, name)==0) {
         sys = &systems_stack[i];
         break;
      }
   }
   if (sys == NULL) {
      WARN("System '%s' was not found in the stack for some reason",name);
      return;
   }
   free(name); /* no more need for it */

   node  = parent->xmlChildrenNode;

   do { /* load all the data */
      if (xml_isNode(node,"jumps")) {
         cur = node->children;
         do {
            if (xml_isNode(cur,"jump")) {
               system_parseJumpPoint( cur, sys );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
}


/**
 * @brief Loads the entire universe into ram - pretty big feat eh?
 *
 *    @return 0 on success.
 */
int space_load (void)
{
   int i, j;
   int ret;
   StarSystem *sys;

   /* Loading. */
   systems_loading = 1;

   /* Load jump point graphic - must be before systems_load(). */
   jumppoint_gfx = gl_newSprite( "gfx/planet/space/jumppoint.png", 4, 4, OPENGL_TEX_MIPMAPS );

   /* Load planets. */
   ret = planets_load();
   if (ret < 0)
      return ret;

   /* Load systems. */
   ret = systems_load();
   if (ret < 0)
      return ret;

   /* Done loading. */
   systems_loading = 0;

   /* Apply all the presences. */
   for (i=0; i<systems_nstack; i++)
      system_addAllPlanetsPresence(&systems_stack[i]);

   /* Reconstruction. */
   systems_reconstructJumps();
   systems_reconstructPlanets();

   /* Fine tuning. */
   for (i=0; i<systems_nstack; i++) {
      sys = &systems_stack[i];

      /* Save jump indexes. */
      for (j=0; j<sys->njumps; j++)
         sys->jumps[j].targetid = sys->jumps[j].target->id;
   }

   return 0;
}


/**
 * @brief Loads the entire systems, needs to be called after planets_load.
 *
 * Does multiple passes to load:
 *
 *  - First loads the star systems.
 *  - Next sets the jump routes.
 *
 *    @return 0 on success.
 */
static int systems_load (void)
{
   uint32_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;
   StarSystem *sys;

   /* Load the file. */
   buf = ndata_read( SYSTEM_DATA, &bufsize );
   if (buf == NULL)
      return -1;

   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL) {
      WARN("'%s' is not a valid XML file.", SYSTEM_DATA);
      return -1;
   }

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_SYSTEM_ID)) {
      ERR("Malformed "SYSTEM_DATA" file: missing root element '"XML_SYSTEM_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed "SYSTEM_DATA" file: does not contain elements");
      return -1;
   }

   /* Allocate if needed. */
   if (systems_stack == NULL) {
      systems_mstack = CHUNK_SIZE;
      systems_stack = malloc( sizeof(StarSystem) * systems_mstack );
      systems_nstack = 0;
   }


   /*
    * First pass - loads all the star systems_stack.
    */
   do {
      if (xml_isNode(node,XML_SYSTEM_TAG)) {
         sys = system_new();
         system_parse( sys, node );
      }
   } while (xml_nextNode(node));


   /*
    * Second pass - loads all the jump routes.
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

   DEBUG("Loaded %d Star System%s with %d Planet%s",
         systems_nstack, (systems_nstack==1) ? "" : "s",
         planet_nstack, (planet_nstack==1) ? "" : "s" );

   return 0;
}


/**
 * @brief Renders the system.
 *
 *    @param dt Current delta tick.
 */
void space_render( const double dt )
{
   if (cur_system == NULL)
      return;

   if (cur_system->nebu_density > 0.)
      nebu_render(dt);
   else
      background_render(dt);
}


/**
 * @brief Renders the system overlay.
 *
 *    @param dt Current delta tick.
 */
void space_renderOverlay( const double dt )
{
   if (cur_system == NULL)
      return;

   if ((cur_system->nebu_density > 0.) &&
         !menu_isOpen( MENU_MAIN ))
      nebu_renderOverlay(dt);
}


/**
 * @brief Renders the current systemsplanets.
 */
void planets_render (void)
{
   int i;

   /* Must be a system. */
   if (cur_system==NULL)
      return;

   /* Render the jumps. */
   for (i=0; i < cur_system->njumps; i++)
      space_renderJumpPoint( &cur_system->jumps[i], i );

   /* Render the planets. */
   for (i=0; i < cur_system->nplanets; i++)
      if (cur_system->planets[i]->real == ASSET_REAL)
         space_renderPlanet( cur_system->planets[i] );
}


/**
 * @brief Renders a jump point.
 */
static void space_renderJumpPoint( JumpPoint *jp, int i )
{
   glColour *c;

   if ((player.p != NULL) && (i==player.p->nav_hyperspace) &&
         (pilot_isFlag(player.p, PILOT_HYPERSPACE) || space_canHyperspace(player.p)))
      c = &cGreen;
   else
      c = NULL;

   gl_blitSprite( jumppoint_gfx, jp->pos.x, jp->pos.y, jp->sx, jp->sy, c );
}


/**
 * @brief Renders a planet.
 */
static void space_renderPlanet( Planet *p )
{
   gl_blitSprite( p->gfx_space, p->pos.x, p->pos.y, 0, 0, NULL );
}


/**
 * @brief Cleans up the system.
 */
void space_exit (void)
{
   int i;

   /* Free jump point graphic. */
   if (jumppoint_gfx != NULL)
      gl_freeTexture(jumppoint_gfx);
   jumppoint_gfx = NULL;

   /* Free the names. */
   if (planetname_stack != NULL)
      free(planetname_stack);
   if (systemname_stack != NULL)
      free(systemname_stack);
   spacename_nstack = 0;

   /* Free the planets. */
   for (i=0; i < planet_nstack; i++) {
      free(planet_stack[i].name);

      if (planet_stack[i].description != NULL)
         free(planet_stack[i].description);
      if (planet_stack[i].bar_description != NULL)
         free(planet_stack[i].bar_description);

      /* graphics */
      if (planet_stack[i].gfx_spaceName != NULL) {
         if (planet_stack[i].gfx_space != NULL)
            gl_freeTexture( planet_stack[i].gfx_space );
         free(planet_stack[i].gfx_spaceName);
         free(planet_stack[i].gfx_spacePath);
      }
      if (planet_stack[i].gfx_exterior != NULL) {
         free(planet_stack[i].gfx_exterior);
         free(planet_stack[i].gfx_exteriorPath);
      }

      /* tech */
      if (planet_stack[i].tech != NULL)
         tech_groupDestroy( planet_stack[i].tech );

      /* commodities */
      free(planet_stack[i].commodities);
   }
   free(planet_stack);
   planet_stack = NULL;
   planet_nstack = 0;
   planet_mstack = 0;

   /* Free the systems. */
   for (i=0; i < systems_nstack; i++) {
      free(systems_stack[i].name);
      if (systems_stack[i].fleets)
         free(systems_stack[i].fleets);
      if (systems_stack[i].jumps)
         free(systems_stack[i].jumps);
      if (systems_stack[i].background)
         free(systems_stack[i].background);

      if(systems_stack[i].presence)
         free(systems_stack[i].presence);

      if (systems_stack[i].planets != NULL)
         free(systems_stack[i].planets);
      if (systems_stack[i].planetsid != NULL)
         free(systems_stack[i].planetsid);
   }
   free(systems_stack);
   systems_stack = NULL;
   systems_nstack = 0;
   systems_mstack = 0;
}


/**
 * @brief Clears all system knowledge.
 */
void space_clearKnown (void)
{
   int i;
   for (i=0; i<systems_nstack; i++)
      sys_rmFlag(&systems_stack[i],SYSTEM_KNOWN);
}


/**
 * @brief Clears all system markers.
 */
void space_clearMarkers (void)
{
   int i;
   for (i=0; i<systems_nstack; i++) {
      sys_rmFlag(&systems_stack[i], SYSTEM_MARKED);
      systems_stack[i].markers_computer = 0;
      systems_stack[i].markers_plot  = 0;
      systems_stack[i].markers_high  = 0;
      systems_stack[i].markers_low   = 0;
   }
}


/**
 * @brief Clears all the system computer markers.
 */
void space_clearComputerMarkers (void)
{
   int i;
   for (i=0; i<systems_nstack; i++)
      sys_rmFlag(&systems_stack[i],SYSTEM_CMARKED);
}


/**
 * @brief Adds a marker to a system.
 *
 *    @param sys ID of the system to add marker to.
 *    @param type Type of the marker to add.
 *    @return 0 on success.
 */
int space_addMarker( int sys, SysMarker type )
{
   StarSystem *ssys;
   int *markers;

   /* Get the system. */
   ssys = system_getIndex(sys);
   if (ssys == NULL)
      return -1;

   /* Get the marker. */
   switch (type) {
      case SYSMARKER_COMPUTER:
         markers = &ssys->markers_computer;
         break;
      case SYSMARKER_LOW:
         markers = &ssys->markers_low;
         break;
      case SYSMARKER_HIGH:
         markers = &ssys->markers_high;
         break;
      case SYSMARKER_PLOT:
         markers = &ssys->markers_plot;
         break;
      default:
         WARN("Unknown marker type.");
         return -1;
   }

   /* Decrement markers. */
   (*markers)++;
   sys_setFlag(ssys, SYSTEM_MARKED);

   return 0;
}


/**
 * @brief Removes a marker from a system.
 *
 *    @param sys ID of the system to remove marker from.
 *    @param type Type of the marker to remove.
 *    @return 0 on success.
 */
int space_rmMarker( int sys, SysMarker type )
{
   StarSystem *ssys;
   int *markers;

   /* Get the system. */
   ssys = system_getIndex(sys);
   if (ssys == NULL)
      return -1;

   /* Get the marker. */
   switch (type) {
      case SYSMARKER_COMPUTER:
         markers = &ssys->markers_computer;
         break;
      case SYSMARKER_LOW:
         markers = &ssys->markers_low;
         break;
      case SYSMARKER_HIGH:
         markers = &ssys->markers_high;
         break;
      case SYSMARKER_PLOT:
         markers = &ssys->markers_plot;
         break;
      default:
         WARN("Unknown marker type.");
         return -1;
   }

   /* Decrement markers. */
   (*markers)--;
   if (*markers <= 0) {
      sys_rmFlag(ssys, SYSTEM_MARKED);
      (*markers) = 0;
   }

   return 0;
}


/**
 * @brief Saves what is needed to be saved for space.
 *
 *    @param writer XML writer to use.
 *    @return 0 on success.
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


/**
 * @brief Loads player's space properties from an XML node.
 *
 *    @param parent Parent node for space.
 *    @return 0 on success.
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


/**
 * @brief Gets the index of the presence element for a faction.
 *          Creates one if it doesn't exist.
 *
 *    @param sys Pointer to the system to check.
 *    @param faction The index of the faction to search for.
 *    @return The index of the presence array for faction.
 */
static int getPresenceIndex( StarSystem *sys, int faction )
{
   int i;

   /* Check for NULL and display a warning. */
   if(sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }

   /* If there is no array, create one and return 0 (the index). */
   if (sys->presence == NULL) {
      sys->npresence = 1;
      sys->presence = malloc(sizeof(SystemPresence));

      /* Set the defaults. */
      sys->presence[0].faction   = faction;
      sys->presence[0].value     = 0 ;
      sys->presence[0].curUsed   = 0 ;
      sys->presence[0].timer     = 0.;
      return 0;
   }

   /* Go through the array, looking for the faction. */
   for (i = 0; i < sys->npresence; i++)
      if (sys->presence[i].faction == faction)
         return i;

   /* Grow the array. */
   i = sys->npresence;
   sys->npresence++;
   sys->presence = realloc(sys->presence, sizeof(SystemPresence) * sys->npresence);
   sys->presence[i].faction = faction;
   sys->presence[i].value = 0;

   return i;
}


/**
 * @brief Do some cleanup work after presence values have been adjusted.
 *
 *    @param sys Pointer to the system to cleanup.
 */
static void presenceCleanup( StarSystem *sys )
{
   int i;

   /* Reset the spilled variable for the entire universe. */
   for (i=0; i < systems_nstack; i++)
      systems_stack[i].spilled = 0;

   /* Check for NULL and display a warning. */
   if (sys == NULL) {
      WARN("sys == NULL");
      return;
   }

   /* Check the system for 0 value presences. */
   for (i=0; i < sys->npresence; i++) {
      if (sys->presence[i].value != 0)
         continue;

      /* Remove the element with 0 value. */
      memmove(&sys->presence[i], &sys->presence[i + 1],
              sizeof(SystemPresence) * (sys->npresence - (i + 1)));
      sys->npresence--;
      sys->presence = realloc(sys->presence, sizeof(SystemPresence) * sys->npresence);
      i--;  /* We'll want to check the new value we just copied in. */
   }

   return;
}


/**
 * @brief Adds (or removes) some presence to a system.
 *
 *    @param sys Pointer to the system to add to or remove from.
 *    @param faction The index of the faction to alter presence for.
 *    @param amount The amount of presence to add (negative to subtract).
 *    @param range The range of spill of the presence.
 */
void system_addPresence( StarSystem *sys, int faction, double amount, int range )
{
   int i, x, curSpill;
   Queue q, qn;
   StarSystem *cur;

   /* Check for NULL and display a warning. */
   if (sys == NULL) {
      WARN("sys == NULL");
      return;
   }

   /* Check that we have a sane faction. (-1 == bobbens == insane)*/
   if (faction_isFaction(faction) == 0)
      return;

   /* Check that we're actually adding any. */
   if (amount == 0)
      return;

   /* Add the presence to the current system. */
   i = getPresenceIndex(sys, faction);
   sys->presence[i].value += amount;

   /* If there's no range, we're done here. */
   if (range < 1)
      return;

   /* Add the spill. */
   sys->spilled   = 1;
   curSpill       = 0;
   q              = q_create();
   qn             = q_create();

   /* Create the initial queue consisting of sys adjacencies. */
   for (i=0; i < sys->njumps; i++) {
      if (sys->jumps[i].target->spilled == 0) {
         q_enqueue( q, sys->jumps[i].target );
         sys->jumps[i].target->spilled = 1;
      }
   }

   /* If it's empty, something's wrong. */
   if (q_isEmpty(q)) {
      /* Means system isn't connected. */
      /*WARN("q is empty after getting adjancies of %s.", sys->name);*/
      q_destroy(q);
      q_destroy(qn);
      presenceCleanup(sys);
      return;
   }

   while (curSpill < range) {
      /* Pull one off the current range queue. */
      cur = q_dequeue(q);

      /* Enqueue all its adjancencies to the next range queue. */
      for (i=0; i < cur->njumps; i++) {
         if (cur->jumps[i].target->spilled == 0) {
            q_enqueue( qn, cur->jumps[i].target );
            cur->jumps[i].target->spilled = 1;
         }
      }

      /* Spill some presence. */
      x = getPresenceIndex(cur, faction);
      cur->presence[x].value += amount / (2 + curSpill);

      /* Check to see if we've finished this range and grab the next queue. */
      if (q_isEmpty(q)) {
         curSpill++;
         q_destroy(q);
         q  = qn;
         qn = q_create();
      }
   }

   /* Destroy the queues. */
   q_destroy(q);
   q_destroy(qn);

   /* Clean up our mess. */
   presenceCleanup(sys);

   return;
}


/**
 * @brief Get the presence of a faction in a system.
 *
 *    @param sys Pointer to the system to process.
 *    @param faction The faction to get the presence for.
 *    @return The amount of presence the faction has in the system.
 */
double system_getPresence( StarSystem *sys, int faction )
{
   int i;

   /* Check for NULL and display a warning. */
   if(sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }

   /* If there is no array, there is no presence. */
   if (sys->presence == NULL)
      return 0;

   /* Go through the array, looking for the faction. */
   for (i = 0; i < sys->npresence; i++) {
      if (sys->presence[i].faction == faction)
         return sys->presence[i].value;
   }

   /* If it's not in there, it's zero. */
   return 0;
}


/**
 * @brief Go through all the assets and call system_addPresence().
 *
 *    @param sys Pointer to the system to process.
 */
void system_addAllPlanetsPresence( StarSystem *sys )
{
   int i;

   /* Check for NULL and display a warning. */
   if(sys == NULL) {
      WARN("sys == NULL");
      return;
   }

   for(i = 0; i < sys->nplanets; i++)
      system_addPresence(sys, sys->planets[i]->faction, sys->planets[i]->presenceAmount, sys->planets[i]->presenceRange);

   return;
}


/**
 * @brief See if the system has a planet or station.
 *
 *    @param sys Pointer to the system to process.
 *    @return 0 If empty; otherwise 1.
 */
int system_hasPlanet( StarSystem *sys )
{
   int i;

   /* Check for NULL and display a warning. */
   if(sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }

   /* Go through all the assets and look for a real one. */
   for(i = 0; i < sys->nplanets; i++)
      if(sys->planets[i]->real == ASSET_REAL)
         return 1;

   return 0;
}


/**
 * @brief Removes active presence.
 */
void system_rmCurrentPresence( StarSystem *sys, int faction, double amount )
{
   int id;
   lua_State *L;
   SystemPresence *presence;

   /* Remove the presence. */
   id = getPresenceIndex( cur_system, faction );
   sys->presence[id].curUsed -= amount;

   /* Sanity. */
   presence = &sys->presence[id];
   presence->curUsed = MAX( 0, sys->presence[id].curUsed );

   /* Run lower hook. */
   L = faction_getState( faction );

   /* Run decrease function if applicable. */
   lua_getglobal( L, "decrease" ); /* f */
   if (lua_isnil(L,-1)) {
      lua_pop(L,1);
      return;
   }
   lua_pushnumber( L, presence->curUsed ); /* f, cur */
   lua_pushnumber( L, presence->value ); /* f, cur, max */
   lua_pushnumber( L, presence->timer ); /* f, cur, max, timer */

   /* Actually run the function. */
   if (lua_pcall(L, 3, 1, 0)) { /* error has occured */
      WARN("Lua decrease script for faction '%s' : %s",
            faction_name( faction ), lua_tostring(L,-1));
      lua_pop(L,1);
      return;
   }

   /* Output is handled the same way. */
   if (!lua_isnumber(L,-1)) {
      WARN("Lua spawn script for faction '%s' failed to return timer value.",
            faction_name( presence->faction ) );
      lua_pop(L,1);
      return;
   }
   presence->timer = lua_tonumber(L,-1);
   lua_pop(L,1);
}



