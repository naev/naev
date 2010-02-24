/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPACE_H
#  define SPACE_H


#include "faction.h"
#include "opengl.h"
#include "pilot.h"
#include "economy.h"
#include "fleet.h"
#include "mission.h"


#define MAX_HYPERSPACE_VEL    25 /**< Speed to brake to before jumping. */


#define PLANET_TECH_MAX       8 /**< Amount of special techs a planet can have. */

#define ASSET_UNREAL          0 /**< The asset is "unreal". */
#define ASSET_REAL            1 /**< The asset is "real". */


/**
 * @brief Different planet classes.
 *
 * Planets types, taken from
 * http://en.wikipedia.org/wiki/Star_Trek_planet_classifications
 */
typedef enum PlanetClass_ {
   PLANET_CLASS_NULL=0, /**< Null/Not defined */
   PLANET_CLASS_A,   /**< Geothermal */
   PLANET_CLASS_B,   /**< Geomorteus */
   PLANET_CLASS_C,   /**< Geoinactive */
   PLANET_CLASS_D,   /**< Asteroid/Moon */
   PLANET_CLASS_E,   /**< Geoplastic */
   PLANET_CLASS_F,   /**< Geometallic */
   PLANET_CLASS_G,   /**< GeoCrystaline */
   PLANET_CLASS_H,   /**< Desert */
   PLANET_CLASS_I,   /**< Gas Supergiant */
   PLANET_CLASS_J,   /**< Gas Giant */
   PLANET_CLASS_K,   /**< Adaptable */
   PLANET_CLASS_L,   /**< Marginal */
   PLANET_CLASS_M,   /**< Terrestrial */
   PLANET_CLASS_N,   /**< Reducing */
   PLANET_CLASS_O,   /**< Pelagic - Aquatic */
   PLANET_CLASS_P,   /**< Glaciated */
   PLANET_CLASS_Q,   /**< Variable */
   PLANET_CLASS_R,   /**< Rogue */
   PLANET_CLASS_S,   /**< Ultragiant */
   PLANET_CLASS_T,   /**< Ultragiant */
   PLANET_CLASS_X,   /**< Demon */
   PLANET_CLASS_Y,   /**< Demon */
   PLANET_CLASS_Z,   /**< Demon */
   STATION_CLASS_A,  /**< Civilian Station */
   STATION_CLASS_B,  /**< Military Station */
   STATION_CLASS_C,  /**< Interfactional Station */
   STATION_CLASS_D   /**< Robotic Station */
} PlanetClass;

/*
 * planet services
 */
#define PLANET_SERVICE_LAND         (1<<0) /**< Can land. */
#define PLANET_SERVICE_INHABITED    (1<<1) /**< Planet is inhabited. */
#define PLANET_SERVICE_REFUEL       (1<<2) /**< Has refueling. */
#define PLANET_SERVICE_BAR          (1<<3) /**< Has bar and thus news. */
#define PLANET_SERVICE_MISSIONS     (1<<4) /**< Has mission computer. */
#define PLANET_SERVICE_COMMODITY    (1<<5) /**< Can trade commodities. */
#define PLANET_SERVICE_OUTFITS      (1<<6) /**< Can trade outfits. */
#define PLANET_SERVICE_SHIPYARD     (1<<7) /**< Can trade ships. */
#define planet_hasService(p,s)      ((p)->services & s) /**< Checks if planet has service. */


/**
 * @struct Planet
 *
 * @brief Represents a planet.
 */
typedef struct Planet_ {
   char* name; /**< planet name */
   Vector2d pos; /**< position in star system */

   /* Planet details. */
   PlanetClass class; /**< planet type */
   int faction; /**< planet faction */
   int population; /**< Population of the planet. */
   double prodfactor; /**< Default Production factor of the planet. */
   double cur_prodfactor; /**< Current real production factor of the planet. */

   /* Asset details. */
   double presenceAmount; /**< The amount of presence this asset exerts. */
   int presenceRange; /**< The range of presence exertion of this asset. */
   int real; /**< If the asset is tangible or not. */
  
   /* Landing details. */
   char* description; /**< planet description */
   char* bar_description; /**< spaceport bar description */
   unsigned int services; /**< what services they offer */
   Commodity **commodities; /**< what commodities they sell */
   int ncommodities; /**< the amount they have */
   int tech[PLANET_TECH_MAX]; /**< tech[0] stores global tech level
                                   (everything that and below) while
                                   tech[1-PLANET_TECH_MAX] store the
                                   "unique" tech levels (only matches */
   int bribed; /**< If planet has been bribed. */

   /* Graphics. */
   glTexture* gfx_space; /**< graphic in space */
   char *gfx_exterior; /**< Don't actually load the texture */
} Planet;


/* 
 * star system flags
 */
#define SYSTEM_KNOWN       (1<<0) /**< System is known. */
#define SYSTEM_MARKED      (1<<1) /**< System is marked by a regular mission. */
#define SYSTEM_CMARKED     (1<<2) /**< System is marked by a computer mission. */
#define sys_isFlag(s,f)    ((s)->flags & (f)) /**< Checks system flag. */
#define sys_setFlag(s,f)   ((s)->flags |= (f)) /**< Sets a system flag. */
#define sys_rmFlag(s,f)    ((s)->flags &= ~(f)) /**< Removes a system flag. */
#define sys_isKnown(s)     sys_isFlag(s,SYSTEM_KNOWN) /**< Checks if system is known. */
#define sys_isMarked(s)    sys_isFlag(s,SYSTEM_MARKED) /**< Checks if system is marked. */


/**
 * @struct SystemFleet
 *
 * @brief Used for freeing presence when fleets in the system get destroyed.
 */
typedef struct SystemFleet_ {
   int npilots; /**< The number of pilots. */
   int faction; /**< The faction of the fleet. */
   double presenceUsed; /** < The amount of presence used by this fleet. */
} SystemFleet;


/**
 * @struct Schedule
 *
 * @brief Represents the schedule for the arrival of a fleet in a system.
 */
typedef struct Schedule_ {
   Fleet *fleet; /**< Pointer to the fleet to spawn. */
   double time; /**< Time for this schedule to finish. */
   double penalty; /**< The penalty for the arrival of the next fleet. */
   int chain; /**< Whether or not the next fleet will chain. */
} Schedule;

/**
 * @struct SystemPresence
 *
 * @brief Represents presence in a system
 */
typedef struct SystemPresence_ {
   int faction; /**< Faction of this presence. */
   double value; /**< Amount of presence. */
   double curUsed; /**< Presence currently used. */
   Schedule schedule; /**< Schedule for the arrival of fleets. */
} SystemPresence;

/**
 * @struct StarSystem
 *
 * @brief Represents a star system.
 *
 * The star system is the basic setting in NAEV.
 */
typedef struct StarSystem_ {

   /* General. */
   char* name; /**< star system name */
   Vector2d pos; /**< position */
   int *jumps; /**< adjacent star system index numbers */
   int njumps; /**< number of adjacent jumps */
   Planet **planets; /**< planets */
   int nplanets; /**< total number of planets */
   int faction; /**< overall faction */

   /* System details. */
   int stars; /**< Amount of "stars" it has. */
   int asteroids; /**< @todo implement asteroids */
   double interference; /**< in % @todo implement interference. */
   double nebu_density; /**< Nebula density (0. - 1000.) */
   double nebu_volatility; /**< Nebula volatility (0. - 1000.) */

   /* Fleets. */
   Fleet** fleets; /**< fleets that can appear in the current system */
   int nfleets; /**< total number of fleets */
   double avg_pilot; /**< Target amount of pilots in the system. */

   /* Calculated. */
   double *prices; /**< Handles the prices in the system. */

   /* Presence. */
   SystemPresence *presence; /**< Pointer to an array of presences in this system. */
   int npresence; /**< Number of elements in the presence array. */
   int spilled; /**< If the system has been spilled to yet. */
   int nsystemFleets; /**< The number of fleets in the system. */
   SystemFleet *systemFleets; /**< Array of pointers to the fleets in the system. */

   /* Markers. */
   int markers_misc; /**< Number of misc mission markers on system. */
   int markers_rush; /**< Number of rush mission markers on system. */
   int markers_cargo; /**< Number of cargo mission markers on system. */

   /* Misc. */
   unsigned int flags; /**< flags for system properties */
} StarSystem;


extern StarSystem *cur_system; /**< current star system */
extern int space_spawn; /**< 1 if spawning is enabled. */


/*
 * loading/exiting
 */
void space_initStars( int n );
void space_init( const char* sysname );
int space_load (void);
void space_exit (void);

/*
 * planet stuff
 */
char* planet_getSystem( const char* planetname );
Planet* planet_get( const char* planetname );
char planet_getClass( Planet *p );

/*
 * system adding/removing stuff.
 */
int system_addPlanet( StarSystem *sys, const char *planetname );
int system_rmPlanet( StarSystem *sys, const char *planetname );
int system_addFleet( StarSystem *sys, Fleet *fleet );
int system_rmFleet( StarSystem *sys, Fleet *fleet );

/*
 * render
 */
void space_renderStars( const double dt );
void space_render( const double dt );
void space_renderOverlay( const double dt );
void planets_render (void);

/*
 * Presence stuff
 */
void system_addPresence( StarSystem *sys, int faction, double amount, int range );
double system_getPresence( StarSystem *sys, int faction );
void system_addAllPlanetsPresence( StarSystem *sys );
void system_removePilotFromSystemFleet( const int systemFleet );

/*
 * update
 */
void space_update( const double dt );

/*
 * misc
 */
StarSystem* system_get( const char* sysname );
StarSystem* system_getIndex( int id );
int space_canHyperspace( Pilot* p);
int space_hyperspace( Pilot* p );
int space_sysReachable( StarSystem *sys );
char** space_getFactionPlanet( int *nplanets, int *factions, int nfactions );
char* space_getRndPlanet (void);
int space_addMarker( const char *sys, SysMarker type );
int space_rmMarker( const char *sys, SysMarker type );
void space_clearKnown (void);
void space_clearMarkers (void);
void space_clearComputerMarkers (void);
int system_hasPlanet( StarSystem *sys );
extern char* stardate;


#endif /* SPACE_H */
