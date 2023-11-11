/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space_fdecl.h"

#include "commodity.h"
#include "explosion.h"
#include "faction.h"
#include "mission_markers.h"
#include "opengl.h"
#include "pilot.h"
#include "shipstats.h"
#include "tech.h"
#include "asteroid.h"

#define SYSTEM_SIMULATE_TIME_PRE   25. /**< Time to simulate system before player is added, during this time special effect creation is disabled. */
#define SYSTEM_SIMULATE_TIME_POST   5. /**< Time to simulate the system before the player is added, however, effects are added. */
#define MAX_HYPERSPACE_VEL    25. /**< Speed to brake to before jumping. */

/*
 * Spob services.
 */
#define SPOB_SERVICE_LAND         (1<<0) /**< Can land. */
#define SPOB_SERVICE_REFUEL       (1<<1) /**< Has refueling. */
#define SPOB_SERVICE_MISSIONS     (1<<2) /**< Has mission computer. */
#define SPOB_SERVICE_COMMODITY    (1<<3) /**< Can trade commodities. */
#define SPOB_SERVICE_OUTFITS      (1<<4) /**< Can trade outfits. */
#define SPOB_SERVICE_SHIPYARD     (1<<5) /**< Can trade ships. */
#define SPOB_SERVICE_BAR          (1<<6) /**< Has bar and thus news. */
#define SPOB_SERVICE_INHABITED    (1<<7) /**< Spob is inhabited. */
#define SPOB_SERVICE_BLACKMARKET  (1<<8) /**< Disables license restrictions on goods. */
#define SPOB_SERVICES_MAX         (SPOB_SERVICE_BLACKMARKET<<1)
#define spob_hasService(p,s)      ((p)->services & s) /**< Checks if spob has a service. */

/*
 * Spob flags.
 */
#define SPOB_KNOWN       (1<<0) /**< Spob is known. */
#define SPOB_BLACKMARKET (1<<1) /**< Spob is a black market. */
#define SPOB_NOMISNSPAWN (1<<2) /**< No missions spawn nor trigger on this spob. */
#define SPOB_UNINHABITED (1<<3) /**< Force spob to be uninhabited. */
#define SPOB_MARKED      (1<<4) /**< Spob is marked. */
#define SPOB_NOLANES     (1<<5) /**< Spob doesn't connect with lanes. */
#define SPOB_RADIUS      (1<<10) /**< Spob has radius defined. */
#define spob_isFlag(p,f)    ((p)->flags & (f)) /**< Checks spob flag. */
#define spob_setFlag(p,f)   ((p)->flags |= (f)) /**< Sets a spob flag. */
#define spob_rmFlag(p,f)    ((p)->flags &= ~(f)) /**< Removes a spob flag. */
#define spob_isKnown(p) spob_isFlag(p,SPOB_KNOWN) /**< Checks if spob is known. */

/**
 * @struct MapOverlayPos
 *
 * @brief Saves the layout decisions from positioning labeled objects on the overlay.
 */
typedef struct MapOverlayPos_ {
   float radius;     /**< Radius for display on the map overlay. */
   float text_offx;  /**< x offset of the caption text. */
   float text_offy;  /**< y offset of the caption text. */
   float text_width; /**< width of the caption text. */
} MapOverlayPos;

/**
 * @brief Represents the presence of a spob. */
typedef struct SpobPresence_ {
   int faction;   /**< Faction generating presence. */
   double base;   /**< Base presence. */
   double bonus;  /**< Bonus presence. */
   int range;     /**< Range effect of the presence (in jumps). */
} SpobPresence;

/**
 * @struct VirtualSpob
 *
 * @brief Basically modifies system parameters without creating any real objects.
 */
typedef struct VirtualSpob_ {
   char *name;                /**< Virtual spob name. */
   SpobPresence *presences;  /**< Virtual spob presences (Array from array.h). */
} VirtualSpob;

/**
 * @struct Spob
 *
 * @brief Represents a Space Object (SPOB), including and not limited to
 * planets, stations, wormholes, hypergates, etc...
 */
typedef struct Spob_ {
   int id;        /**< Spob ID. */
   char *name;    /**< Spob name */
   char *display; /**< Name to be displayed to the player. Defaults to name if not set. */
   char *feature; /**< Name of the feature the spob provides if applicable. */
   vec2 pos;  /**< position in star system */
   double radius; /**< Radius of the space object. WARNING: lazy-loaded with gfx_space. */
   const SimpleShader *marker; /**< GUI marker. */
   double marker_scale; /**< Scales the marker. */

   /* Spob details. */
   char *class;         /**< Spob type. Uses Star Trek classification system for planets (https://stexpanded.fandom.com/wiki/Spob_classifications) */
   uint64_t population; /**< Population of the spob. */

   /* Spob details. */
   SpobPresence presence; /**< Presence details (faction, etc.) */
   double hide;         /**< The ewarfare hide value for the spob. */

   /* Landing details. */
   int can_land;        /**< Whether or not the player can land. */
   int land_override;   /**< Forcibly allows the player to either be able to land or not (+1 is land, -1 is not, 0 otherwise). */
   char *land_msg;      /**< Message on landing. */

   /* Landed details. */
   char *description;      /**< Spob description. */
   char *bar_description;  /**< Spob spaceport bar description */
   unsigned int services;  /**< What services they offer */
   Commodity **commodities;/**< array: what commodities they sell */
   CommodityPrice *commodityPrice; /**< array: the base cost of a commodity on this spob */
   tech_group_t *tech;     /**< Spob tech. */

   /* Graphics. */
   glTexture *gfx_space;   /**< Graphic in space */
   char *gfx_spaceName;    /**< Name to load texture quickly with. */
   char *gfx_spacePath;    /**< Name of the gfx_space for saving purposes. */
   char *gfx_exterior;     /**< Don't actually load the texture */
   char *gfx_exteriorPath; /**< Name of the gfx_exterior for saving purposes. */
   char *gfx_comm;         /**< Name of the texture. */
   char *gfx_commPath;     /**< Communication image path. */

   /* Misc. */
   char **tags;         /**< Spob tags. */
   unsigned int flags;  /**< flags for spob properties */
   MapOverlayPos mo;    /**< Overlay layout data. */
   double map_alpha;    /**< Alpha to display on the map. */
   int markers;         /**< Markers enabled on the spob. */

   /* Lua stuff. */
   char *lua_file;   /**,< Lua File. */
   nlua_env lua_env; /**< Lua environment. */
   int lua_mem;      /**< Memory of the current instance. */
   int lua_init;     /**< Run when initializing the spob. */
   int lua_load;     /**< Run when player enters system. */
   int lua_unload;   /**< Run when player exits system. */
   int lua_can_land; /**< Checks to see if the player can land on the spob. */
   int lua_land;     /**< Run when a pilot "lands". */
   int lua_render;   /**< Run when rendering. */
   int lua_update;   /**< Run when updating. */
   int lua_comm;     /**< Run when player communicates with the spob. */
   int lua_population; /**< Run when getting a string representing the population of the spob. */
   int lua_barbg;    /**< Run to generate bar backgrounds as necessary. */
} Spob;

/*
 * Star system flags
 */
#define SYSTEM_KNOWN       (1<<0) /**< System is known. */
#define SYSTEM_MARKED      (1<<1) /**< System is marked by a regular mission. */
#define SYSTEM_CMARKED     (1<<2) /**< System is marked by a computer mission. */
#define SYSTEM_CLAIMED     (1<<3) /**< System is claimed by a mission. */
#define SYSTEM_DISCOVERED  (1<<4) /**< System has been discovered. This is a temporary flag used by the map. */
#define SYSTEM_HIDDEN      (1<<5) /**< System is temporarily hidden from view. */
#define SYSTEM_HAS_KNOWN_LANDABLE (1<<6) /**< System has potentially landable spobs that are known (temporary use by map!) */
#define SYSTEM_HAS_LANDABLE (1<<7) /**< System has landable spobs (temporary use by map!) */
#define SYSTEM_NOLANES     (1<<8) /**< System should not use safe lanes at all. */
#define SYSTEM_PMARKED     (1<<9) /**< System is marked by a player. */
#define SYSTEM_INTEREST    (1<<10) /**< System is temporary marked as "of interest". Used when rendering map. */
#define sys_isFlag(s,f)    ((s)->flags & (f)) /**< Checks system flag. */
#define sys_setFlag(s,f)   ((s)->flags |= (f)) /**< Sets a system flag. */
#define sys_rmFlag(s,f)    ((s)->flags &= ~(f)) /**< Removes a system flag. */
#define sys_isKnown(s)     (sys_isFlag((s),SYSTEM_KNOWN)) /**< Checks if system is known. */
#define sys_isMarked(s)    sys_isFlag((s),SYSTEM_MARKED) /**< Checks if system is marked. */

/**
 * @brief Represents presence in a system
 */
typedef struct SystemPresence_ {
   int faction;      /**< Faction of this presence. */
   double base;      /**< Base presence value. */
   double bonus;     /**< Bonus presence value. */
   double value;     /**< Amount of presence (base+bonus). */
   double curUsed;   /**< Presence currently used. */
   double timer;     /**< Current faction timer. */
   int disabled;     /**< Whether or not spawning is disabled for this presence. */
} SystemPresence;

/*
 * Jump point flags.
 */
#define JP_AUTOPOS      (1<<0) /**< Automatically position jump point based on system radius. */
#define JP_KNOWN        (1<<1) /**< Jump point is known. */
#define JP_HIDDEN       (1<<2) /**< Jump point is hidden. */
#define JP_EXITONLY     (1<<3) /**< Jump point is exit only */
#define JP_NOLANES      (1<<4) /**< Jump point doesn't create lanes. */
#define jp_isFlag(j,f)    ((j)->flags & (f)) /**< Checks jump flag. */
#define jp_setFlag(j,f)   ((j)->flags |= (f)) /**< Sets a jump flag. */
#define jp_rmFlag(j,f)    ((j)->flags &= ~(f)) /**< Removes a jump flag. */
#define jp_isKnown(j)     jp_isFlag(j,JP_KNOWN) /**< Checks if jump is known. */
#define jp_isUsable(j)    (jp_isKnown(j) && !jp_isFlag(j,JP_EXITONLY))

/**
 * @brief Represents a jump lane.
 */
typedef struct JumpPoint_ JumpPoint;
struct JumpPoint_ {
   StarSystem *from; /**< System containing this jump point. */
   int targetid;     /**< ID of the target star system. */
   StarSystem *target; /**< Target star system to jump to. */
   JumpPoint *returnJump; /**< How to get back. Can be NULL */
   vec2 pos;     /**< Position in the system. */
   double radius;    /**< Radius of jump range. */
   unsigned int flags;/**< Flags related to the jump point's status. */
   double hide;      /**< ewarfare hide value for the jump point */
   double angle;     /**< Direction the jump is facing. */
   double map_alpha; /**< Alpha to display on the map. */
   /* Cached stuff. */
   double cosa;      /**< Cosinus of the angle. */
   double sina;      /**< Sinus of the angle. */
   int sx;           /**< X sprite to use. */
   int sy;           /**< Y sprite to use. */
   MapOverlayPos mo; /**< Overlay layout data. */
};
extern glTexture *jumppoint_gfx; /**< Jump point graphics. */

/**
 * @brief Map shader.
 */
typedef struct MapShader_ {
   char *name;       /**< Name of the map shader. */
   GLuint program;   /**< Program for map shader. */
   GLuint vertex;    /**< Vertex attribute for map shader. */
   GLuint projection;/**< Projection matrix for map shader. */
   GLuint alpha;     /**< Transparency for map shader. */
   GLuint time;      /**< Time for map shader. */
   GLuint globalpos; /**< Global position of system for map shader. */
} MapShader;

/**
 * @brief Represents a star system.
 *
 * The star system is the basic setting in Naev.
 */
struct StarSystem_ {
   int id;                 /**< Star system index. */
   char *filename;         /** Name of the file loaded from. */

   /* General. */
   char* name;             /**< star system name */
   vec2 pos;               /**< Position */
   int spacedust;          /**< Amount of "space dust" it has. */
   double interference;    /**< Modifies global ew with a factor of 1/(1+interference/100). So 100 would half visibility. */
   double nebu_hue;        /**< Hue of the nebula (0. - 1.) */
   double nebu_density;    /**< Nebula density (0. - 1000.) */
   double nebu_volatility; /**< Damage per second. */
   double radius;          /**< Default system radius for standard jump points. */
   char *background;       /**< Background script. */
   char *features;         /**< Extra text on the map indicating special features of the system. */

   /* Spobs. */
   Spob **spobs;           /**< Array (array.h): spobs */
   int *spobsid;           /**< Array (array.h): IDs of the spobs. */
   int faction;            /**< overall faction */
   VirtualSpob **spobs_virtual; /**< Array (array.h): virtual spobs. */

   /* Jumps. */
   JumpPoint *jumps;       /**< Array (array.h): Jump points in the system */

   /* Asteroids. */
   AsteroidAnchor *asteroids; /**< Array (array.h): Asteroid fields in the system */
   AsteroidExclusion *astexclude; /**< Array (array.h): Asteroid exclusion zones in the system */
   double asteroid_density;/**< Represents totaly density of asteroids in the system. */

   /* Calculated. */
   double *prices;      /**< Handles the prices in the system. */

   /* Presence. */
   SystemPresence *presence; /**< Array (array.h): Pointer to an array of presences in this system. */
   int spilled;         /**< If the system has been spilled to yet. */
   double ownerpresence;/**< Amount of presence the owning faction has in a system. */

   /* Markers. */
   int markers_computer;/**< Number of mission computer markers. */
   int markers_low;     /**< Number of low mission markers. */
   int markers_high;    /**< Number of high mission markers. */
   int markers_plot;    /**< Number of plot level mission markers. */

   /* Map shader. */
   char *map_shader;    /**< Name of the map shader file for saving. */
   const MapShader *ms; /**< Map shader. */

   /* Economy. */
   CommodityPrice *averagePrice;

   /* Misc. */
   char **tags;         /**< Star system tags. */
   unsigned int flags;  /**< flags for system properties */
   ShipStatList *stats; /**< System stats. */
   char *note;          /**< Note to player marked system */
   int claims_soft;     /**< Number of soft claims on the system. */
};

/* Some useful externs. */
extern StarSystem *cur_system; /**< current star system */
extern int space_spawn; /**< 1 if spawning is enabled. */

/*
 * loading/exiting
 */
void space_init( const char* sysname, int do_simulate );
int space_load (void);
int space_loadLua (void);
void space_exit (void);

/*
 * spob stuff
 */
Spob *spob_new (void);
const char *spob_name( const Spob *p );
int spob_luaInit( Spob *spb );
void spob_gfxLoad( Spob *p );
int spob_hasSystem( const Spob *spb );
const char* spob_getSystem( const char* spobname );
Spob* spob_getAll (void);
Spob* spob_get( const char* spobname );
Spob* spob_getIndex( int ind );
void spob_setKnown( Spob *p );
int spob_index( const Spob *p );
int spob_exists( const char* spobname );
const char *spob_existsCase( const char* spobname );
char **spob_searchFuzzyCase( const char* spobname, int *n );
const char* spob_getServiceName( int service );
int spob_getService( const char *name );
const char* spob_getClassName( const char *class );
credits_t spob_commodityPrice( const Spob *p, const Commodity *c );
credits_t spob_commodityPriceAtTime( const Spob *p, const Commodity *c, ntime_t t );
int spob_averageSpobPrice( const Spob *p, const Commodity *c, credits_t *mean, double *std);
void spob_averageSeenPricesAtTime( const Spob *p, const ntime_t tupdate );
/* Misc modification. */
int spob_setFaction( Spob *p, int faction );
int spob_addCommodity( Spob *p, Commodity *c );
int spob_addService( Spob *p, int service );
int spob_rmService( Spob *p, int service );
int spob_rename( Spob *p, char *newname );
/* Land related stuff. */
char spob_getColourChar( const Spob *p );
const char *spob_getSymbol( const Spob *p );
const glColour* spob_getColour( const Spob *p );
void spob_updateLand( Spob *p );
/* Lua stuff. */
void spob_luaInitMem( const Spob *spob );

/*
 * Virtual spob stuff.
 */
VirtualSpob* virtualspob_getAll (void);
VirtualSpob* virtualspob_get( const char *name );

/*
 * jump stuff
 */
JumpPoint* jump_get( const char* jumpname, const StarSystem* sys );
JumpPoint* jump_getTarget( const StarSystem* target, const StarSystem* sys );
const char *jump_getSymbol( const JumpPoint *jp );

/*
 * system adding/removing stuff.
 */
void system_reconstructJumps( StarSystem *sys );
void systems_reconstructJumps (void);
void systems_reconstructSpobs (void);
StarSystem *system_new (void);
int system_addSpob( StarSystem *sys, const char *spobname );
int system_rmSpob( StarSystem *sys, const char *spobname );
int system_addVirtualSpob( StarSystem *sys, const char *spobname );
int system_rmVirtualSpob( StarSystem *sys, const char *spobname );
int system_addJumpDiff( StarSystem *sys, xmlNodePtr node );
int system_rmJump( StarSystem *sys, const char *jumpname );

/*
 * render
 */
void space_render( const double dt );
void space_renderOverlay( const double dt );
void spobs_render (void);

/*
 * Presence stuff.
 */
void system_presenceCleanupAll (void);
void system_presenceAddSpob( StarSystem *sys, const SpobPresence *ap );
double system_getPresence( const StarSystem *sys, int faction );
double system_getPresenceFull( const StarSystem *sys, int faction, double *base, double *bonus );
void system_addAllSpobsPresence( StarSystem *sys );
void space_reconstructPresences( void );
void system_rmCurrentPresence( StarSystem *sys, int faction, double amount );

/*
 * update.
 */
void space_update( double dt, double real_dt );
int space_isSimulation (void);
int space_needsEffects (void);

/*
 * Graphics.
 */
void space_gfxLoad( StarSystem *sys );
void space_gfxUnload( StarSystem *sys );

/*
 * Getting stuff.
 */
StarSystem* system_getAll (void);
const char *system_existsCase( const char* sysname );
char **system_searchFuzzyCase( const char* sysname, int *n );
StarSystem* system_get( const char* sysname );
StarSystem* system_getIndex( int id );
int system_index( const StarSystem *sys );
int space_sysReachable( const StarSystem *sys );
int space_sysReallyReachable( const char* sysname );
int space_sysReachableFromSys( const StarSystem *target, const StarSystem *sys );
char** space_getFactionSpob( int *factions, int landable );
const char* space_getRndSpob( int landable, unsigned int services,
      int (*filter)(Spob *p));
double system_getClosest( const StarSystem *sys, int *pnt, int *jp, int *ast, int *fie, double x, double y );
double system_getClosestAng( const StarSystem *sys, int *pnt, int *jp, int *ast, int *fie, double x, double y, double ang );

/*
 * Markers.
 */
int space_addMarker( int sys, MissionMarkerType type );
int space_rmMarker( int sys, MissionMarkerType type );
void space_clearKnown (void);
void space_clearMarkers (void);
void space_clearComputerMarkers (void);
int system_hasSpob( const StarSystem *sys );

/*
 * Hyperspace.
 */
int space_jumpDistance( const Pilot *p, const JumpPoint *jp );
int space_canHyperspace( const Pilot *p);
int space_hyperspace( Pilot *p );
int space_calcJumpInPos( const StarSystem *in, const StarSystem *out, vec2 *pos, vec2 *vel, double *dir, const Pilot *p );

/*
 * Misc.
 */
void system_updateAsteroids( StarSystem *sys );
void system_setFaction( StarSystem *sys );
void space_checkLand (void);
void space_factionChange (void);
void space_queueLand( Spob *pnt );
const char *space_populationStr( const Spob *spb );
