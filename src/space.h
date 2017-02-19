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
#include "tech.h"


#define SYSTEM_SIMULATE_TIME  15. /**< Time to simulate system before player is added. */

#define MAX_HYPERSPACE_VEL    25 /**< Speed to brake to before jumping. */

#define ASSET_VIRTUAL         0 /**< The asset is virtual. */
#define ASSET_REAL            1 /**< The asset is real. */


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
#define PLANET_SERVICES_MAX         (PLANET_SERVICE_SHIPYARD<<1)
#define planet_hasService(p,s)      ((p)->services & s) /**< Checks if planet has service. */


/*
 * Planet flags.
 */
#define PLANET_KNOWN       (1<<0) /**< Planet is known. */
#define PLANET_BLACKMARKET (1<<1) /**< Planet is a black market. */
#define planet_isFlag(p,f)    ((p)->flags & (f)) /**< Checks planet flag. */
#define planet_setFlag(p,f)   ((p)->flags |= (f)) /**< Sets a planet flag. */
#define planet_rmFlag(p,f)    ((p)->flags &= ~(f)) /**< Removes a planet flag. */
#define planet_isKnown(p)     planet_isFlag(p,PLANET_KNOWN) /**< Checks if planet is known. */
#define planet_isBlackMarket(p) planet_isFlag(p,PLANET_BLACKMARKET) /**< Checks if planet is a black market. */


/**
 * @struct Planet
 *
 * @brief Represents a planet.
 */
typedef struct Planet_ {
   int id; /**< Planet ID. */
   char* name; /**< planet name */
   Vector2d pos; /**< position in star system */
   double radius; /**< Radius of the planet. */

   /* Planet details. */
   char *class; /**< planet type */
   int faction; /**< planet faction */
   uint64_t population; /**< Population of the planet. */

   /* Asset details. */
   double presenceAmount; /**< The amount of presence this asset exerts. */
   int presenceRange; /**< The range of presence exertion of this asset. */
   int real; /**< If the asset is tangible or not. */
   double hide; /**< The ewarfare hide value for an asset. */

   /* Landing details. */
   int land_override; /**< Forcibly allows the player to either be able to land or not (+1 is land, -1 is not, 0 otherwise). */
   char *land_func; /**< Landing function to execute. */
   int can_land; /**< Whether or not the player can land. */
   char *land_msg; /**< Message on landing. */
   credits_t bribe_price; /**< Cost of bribing. */
   char *bribe_msg; /**< Bribe message. */
   char *bribe_ack_msg; /**< Bribe ACK message. */
   int bribed; /**< If planet has been bribed. */

   /* Landed details. */
   char* description; /**< planet description */
   char* bar_description; /**< spaceport bar description */
   unsigned int services; /**< what services they offer */
   Commodity **commodities; /**< what commodities they sell */
   int ncommodities; /**< the amount they have */
   tech_group_t *tech; /**< Planet tech. */

   /* Graphics. */
   glTexture* gfx_space; /**< graphic in space */
   char *gfx_spaceName; /**< Name to load texture quickly with. */
   char *gfx_spacePath; /**< Name of the gfx_space for saving purposes. */
   char *gfx_exterior; /**< Don't actually load the texture */
   char *gfx_exteriorPath; /**< Name of the gfx_exterior for saving purposes. */

   /* Misc. */
   unsigned int flags; /**< flags for planet properties */
} Planet;


/*
 * star system flags
 */
#define SYSTEM_KNOWN       (1<<0) /**< System is known. */
#define SYSTEM_MARKED      (1<<1) /**< System is marked by a regular mission. */
#define SYSTEM_CMARKED     (1<<2) /**< System is marked by a computer mission. */
#define SYSTEM_CLAIMED     (1<<3) /**< System is claimed by a mission. */
#define sys_isFlag(s,f)    ((s)->flags & (f)) /**< Checks system flag. */
#define sys_setFlag(s,f)   ((s)->flags |= (f)) /**< Sets a system flag. */
#define sys_rmFlag(s,f)    ((s)->flags &= ~(f)) /**< Removes a system flag. */
#define sys_isKnown(s)     sys_isFlag(s,SYSTEM_KNOWN) /**< Checks if system is known. */
#define sys_isMarked(s)    sys_isFlag(s,SYSTEM_MARKED) /**< Checks if system is marked. */


/*
 * Forward declaration.
 */
typedef struct StarSystem_ StarSystem;


/**
 * @brief Represents presence in a system
 */
typedef struct SystemPresence_ {
   int faction; /**< Faction of this presence. */
   double value; /**< Amount of presence. */
   double curUsed; /**< Presence currently used. */
   double timer; /**< Current faction timer. */
   int disabled; /**< Whether or not spawning is disabled for this presence. */
} SystemPresence;


/*
 * Jump point flags.
 */
#define JP_AUTOPOS      (1<<0) /**< Automatically position jump point based on system radius. */
#define JP_KNOWN        (1<<1) /**< Jump point is known. */
#define JP_HIDDEN       (1<<2) /**< Jump point is hidden. */
#define JP_EXITONLY     (1<<3) /**< Jump point is exit only */
#define jp_isFlag(j,f)    ((j)->flags & (f)) /**< Checks jump flag. */
#define jp_setFlag(j,f)   ((j)->flags |= (f)) /**< Sets a jump flag. */
#define jp_rmFlag(j,f)    ((j)->flags &= ~(f)) /**< Removes a jump flag. */
#define jp_isKnown(j)     jp_isFlag(j,JP_KNOWN) /**< Checks if jump is known. */
#define jp_isUsable(j)    (jp_isKnown(j) && !jp_isFlag(j,JP_EXITONLY))



/**
 * @brief Represents a jump lane.
 */
typedef struct JumpPoint_ {
   StarSystem *target; /**< Target star system to jump to. */
   int targetid; /**< ID of the target star system. */
   Vector2d pos; /**< Position in the system. */
   double radius; /**< Radius of jump range. */
   unsigned int flags; /**< Flags related to the jump point's status. */
   double hide; /**< ewarfare hide value for the jump point */
   double angle; /**< Direction the jump is facing. */
   double cosa; /**< Cosinus of the angle. */
   double sina; /**< Sinus of the angle. */
   int sx; /**< X sprite to use. */
   int sy; /**< Y sprite to use. */
} JumpPoint;
extern glTexture *jumppoint_gfx; /**< Jump point graphics. */


/**
 * @brief Represents a star system.
 *
 * The star system is the basic setting in Naev.
 */
struct StarSystem_ {
   int id; /**< Star system index. */

   /* General. */
   char* name; /**< star system name */
   Vector2d pos; /**< position */
   int stars; /**< Amount of "stars" it has. */
   double interference; /**< in % @todo implement interference. */
   double nebu_density; /**< Nebula density (0. - 1000.) */
   double nebu_volatility; /**< Nebula volatility (0. - 1000.) */
   double radius; /**< Default system radius for standard jump points. */
   char *background; /**< Background script. */

   /* Planets. */
   Planet **planets; /**< planets */
   int *planetsid; /**< IDs of the planets. */
   int nplanets; /**< total number of planets */
   int faction; /**< overall faction */

   /* Jumps. */
   JumpPoint *jumps; /**< Jump points in the system */
   int njumps; /**< number of adjacent jumps */

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
   double ownerpresence; /**< Amount of presence the owning faction has in a system. */

   /* Markers. */
   int markers_computer; /**< Number of mission computer markers. */
   int markers_low; /**< Number of low mission markers. */
   int markers_high; /**< Number of high mission markers. */
   int markers_plot; /**< Number of plot level mission markers. */

   /* Misc. */
   unsigned int flags; /**< flags for system properties */
};


extern StarSystem *cur_system; /**< current star system */
extern int space_spawn; /**< 1 if spawning is enabled. */


/*
 * loading/exiting
 */
void space_init( const char* sysname );
int space_load (void);
void space_exit (void);

/*
 * planet stuff
 */
Planet *planet_new (void);
char* planet_getSystem( const char* planetname );
Planet* planet_getAll( int *n );
Planet* planet_get( const char* planetname );
Planet* planet_getIndex( int ind );
void planet_setKnown( Planet *p );
void planet_setBlackMarket( Planet *p );
int planet_index( const Planet *p );
int planet_exists( const char* planetname );
const char *planet_existsCase( const char* planetname );
char **planet_searchFuzzyCase( const char* planetname, int *n );
char* planet_getServiceName( int service );
int planet_getService( char *name );
credits_t planet_commodityPrice( const Planet *p, const Commodity *c );
/* Misc modification. */
int planet_setFaction( Planet *p, int faction );
/* Land related stuff. */
char planet_getColourChar( Planet *p );
const glColour* planet_getColour( Planet *p );
void planet_updateLand( Planet *p );
int planet_setRadiusFromGFX(Planet* planet);


/*
 * jump stuff
 */
JumpPoint* jump_get( const char* jumpname, const StarSystem* sys );
JumpPoint* jump_getTarget( StarSystem* target, const StarSystem* sys );

/*
 * system adding/removing stuff.
 */
void system_reconstructJumps (StarSystem *sys);
void systems_reconstructJumps (void);
void systems_reconstructPlanets (void);
StarSystem *system_new (void);
int system_addPlanet( StarSystem *sys, const char *planetname );
int system_rmPlanet( StarSystem *sys, const char *planetname );
int system_addJump( StarSystem *sys, xmlNodePtr node );
int system_addJumpDiff( StarSystem *sys, xmlNodePtr node );
int system_rmJump( StarSystem *sys, const char *jumpname );
int system_addFleet( StarSystem *sys, Fleet *fleet );
int system_rmFleet( StarSystem *sys, Fleet *fleet );

/*
 * render
 */
void space_render( const double dt );
void space_renderOverlay( const double dt );
void planets_render (void);

/*
 * Presence stuff.
 */
void system_presenceCleanupAll( void );
void system_addPresence( StarSystem *sys, int faction, double amount, int range );
double system_getPresence( StarSystem *sys, int faction );
void system_addAllPlanetsPresence( StarSystem *sys );
void space_reconstructPresences( void );
void system_rmCurrentPresence( StarSystem *sys, int faction, double amount );

/*
 * update.
 */
void space_update( const double dt );

/*
 * Graphics.
 */
void space_gfxLoad( StarSystem *sys );
void space_gfxUnload( StarSystem *sys );

/*
 * Getting stuff.
 */
StarSystem* system_getAll( int *nsys );
int system_exists( const char* sysname );
const char *system_existsCase( const char* sysname );
char **system_searchFuzzyCase( const char* sysname, int *n );
StarSystem* system_get( const char* sysname );
StarSystem* system_getIndex( int id );
int system_index( StarSystem *sys );
int space_sysReachable( StarSystem *sys );
int space_sysReallyReachable( char* sysname );
int space_sysReachableFromSys( StarSystem *target, StarSystem *sys );
char** space_getFactionPlanet( int *nplanets, int *factions, int nfactions, int landable );
char* space_getRndPlanet( int landable, unsigned int services,
      int (*filter)(Planet *p));
double system_getClosest( const StarSystem *sys, int *pnt, int *jp, double x, double y );
double system_getClosestAng( const StarSystem *sys, int *pnt, int *jp, double x, double y, double ang );


/*
 * Markers.
 */
int space_addMarker( int sys, SysMarker type );
int space_rmMarker( int sys, SysMarker type );
void space_clearKnown (void);
void space_clearMarkers (void);
void space_clearComputerMarkers (void);
int system_hasPlanet( const StarSystem *sys );


/*
 * Hyperspace.
 */
int space_canHyperspace( Pilot* p);
int space_hyperspace( Pilot* p );
int space_calcJumpInPos( StarSystem *in, StarSystem *out, Vector2d *pos, Vector2d *vel, double *dir );


/*
 * Misc.
 */
void system_setFaction( StarSystem *sys );
void space_factionChange (void);


#endif /* SPACE_H */
