/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file space.c
 *
 * @brief Handles all the space stuff, namely systems and space objects (spobs).
 */
/** @cond */
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "space.h"

#include "background.h"
#include "conf.h"
#include "damagetype.h"
#include "dev_uniedit.h"
#include "economy.h"
#include "gatherable.h"
#include "gui.h"
#include "hook.h"
#include "land.h"
#include "log.h"
#include "map.h"
#include "map_overlay.h"
#include "menu.h"
#include "mission.h"
#include "music.h"
#include "ndata.h"
#include "nebula.h"
#include "nfile.h"
#include "nlua.h"
#include "nlua_pilot.h"
#include "nlua_spob.h"
#include "nlua_gfx.h"
#include "nlua_camera.h"
#include "nlua_tex.h"
#include "nluadef.h"
#include "nmath.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "opengl.h"
#include "pause.h"
#include "pilot.h"
#include "player.h"
#include "queue.h"
#include "rng.h"
#include "sound.h"
#include "spfx.h"
#include "start.h"
#include "toolkit.h"
#include "weapon.h"

#define XML_SPOB_TAG   "spob" /**< Individual spob xml tag. */
#define XML_SYSTEM_TAG  "ssys" /**< Individual systems xml tag. */

#define SPOB_GFX_EXTERIOR_PATH_W 400 /**< Spob exterior graphic width. */
#define SPOB_GFX_EXTERIOR_PATH_H 400 /**< Spob exterior graphic height. */

/* used to overcome warnings due to 0 values */
#define FLAG_POSSET           (1<<0) /**< Set the position. */
#define FLAG_INTERFERENCESET  (1<<1) /**< Set the interference value. */
#define FLAG_SERVICESSET      (1<<2) /**< Set the service value. */
#define FLAG_FACTIONSET       (1<<3) /**< Set the faction value. */

#define DEBRIS_BUFFER         1000 /**< Buffer to smooth appearance of debris */

typedef struct spob_lua_file_s {
   const char *filename;   /**< Name of the spob Lua file. */
   nlua_env env;           /**< Lua environment. */
   int lua_mem;            /**< Global memory. */
} spob_lua_file;

static spob_lua_file *spob_lua_stack = NULL; /**< Handles spob Lua chunks. */

/*
 * spob <-> system name stack
 */
static char** spobname_stack = NULL; /**< Spob name stack corresponding to system. */
static char** systemname_stack = NULL; /**< System name stack corresponding to spob. */

/*
 * Arrays.
 */
StarSystem *systems_stack = NULL; /**< Star system stack. */
static Spob *spob_stack = NULL; /**< Spob stack. */
static VirtualSpob *vspob_stack = NULL; /**< Virtual spob stack. */
#ifdef DEBUGGING
static int systemstack_changed = 0; /**< Whether or not the systems_stack was changed after loading. */
#endif /* DEBUGGING */
static int spobstack_changed = 0; /**< Whether or not the spob_stack was changed after loading. */
static MapShader **mapshaders = NULL; /**< Map shaders. */

/*
 * Misc.
 */
static int systems_loading = 1; /**< Systems are loading. */
StarSystem *cur_system = NULL; /**< Current star system. */
glTexture *jumppoint_gfx = NULL; /**< Jump point graphics. */
static glTexture *jumpbuoy_gfx = NULL; /**< Jump buoy graphics. */
static int space_fchg = 0; /**< Faction change counter, to avoid unnecessary calls. */
static int space_simulating = 0; /**< Are we simulating space? */
static int space_simulating_effects = 0; /**< Are we doing special effects? */
static Spob *space_landQueueSpob = NULL;

/*
 * Fleet spawning.
 */
int space_spawn = 1; /**< Spawn enabled by default. */

/*
 * Internal Prototypes.
 */
/* spob load */
static int spob_parse( Spob *spob, const char *filename, Commodity **stdList );
static int space_parseSpobs( xmlNodePtr parent, StarSystem* sys );
static int spob_parsePresence( xmlNodePtr node, SpobPresence *ap );
/* system load */
static void system_init( StarSystem *sys );
static int systems_load (void);
static int system_parse( StarSystem *system, const char *filename );
static int system_parseJumpPoint( const xmlNodePtr node, StarSystem *sys );
static int system_parseJumpPointDiff( const xmlNodePtr node, StarSystem *sys );
static int system_parseJumps( StarSystem *sys );
static int system_parseAsteroidField( const xmlNodePtr node, StarSystem *sys );
static int system_parseAsteroidExclusion( const xmlNodePtr node, StarSystem *sys );
/* misc */
static int spob_cmp( const void *p1, const void *p2 );
static int getPresenceIndex( StarSystem *sys, int faction );
static void system_scheduler( double dt, int init );
/* Markers. */
static int space_addMarkerSystem( int sysid, MissionMarkerType type );
static int space_addMarkerSpob( int pntid, MissionMarkerType type );
static int space_rmMarkerSystem( int sysid, MissionMarkerType type );
static int space_rmMarkerSpob( int pntid, MissionMarkerType type );
/* Render. */
static void space_renderJumpPoint( const JumpPoint *jp, int i );
static void space_renderSpob( const Spob *p );
static void space_updateSpob( const Spob *p, double dt, double real_dt );
/* Map shaders. */
static const MapShader *mapshader_get( const char *name );
/* Lua stuff. */
static int spob_lua_cmp( const void *a, const void *b );
static nlua_env spob_lua_get( int *mem, const char *filename );
static void spob_lua_free( spob_lua_file *lf );
/*
 * Externed prototypes.
 */
int space_sysSave( xmlTextWriterPtr writer );
int space_sysLoad( xmlNodePtr parent );

/**
 * @brief Gets the (English) name for a service code.
 *
 * @param service One of the \p SPOB_SERVICE_* enum values.
 * @return English name, reversible via \p spob_getService()
 * and presentable via \p _().
 */
const char* spob_getServiceName( int service )
{
   switch (service) {
      case SPOB_SERVICE_LAND:        return N_("Land");
      case SPOB_SERVICE_INHABITED:   return N_("Inhabited");
      case SPOB_SERVICE_REFUEL:      return N_("Refuel");
      case SPOB_SERVICE_BAR:         return N_("Bar");
      case SPOB_SERVICE_MISSIONS:    return N_("Missions");
      case SPOB_SERVICE_COMMODITY:   return N_("Commodity");
      case SPOB_SERVICE_OUTFITS:     return N_("Outfits");
      case SPOB_SERVICE_SHIPYARD:    return N_("Shipyard");
      case SPOB_SERVICE_BLACKMARKET: return N_("Blackmarket");
   }
   return NULL;
}

/**
 * @brief Converts name to spob service flag.
 */
int spob_getService( const char *name )
{
   if (strcasecmp(name,"Land")==0)
      return SPOB_SERVICE_LAND;
   else if (strcasecmp(name,"Inhabited")==0)
      return SPOB_SERVICE_INHABITED;
   else if (strcasecmp(name,"Refuel")==0)
      return SPOB_SERVICE_REFUEL;
   else if (strcasecmp(name,"Bar")==0)
      return SPOB_SERVICE_BAR;
   else if (strcasecmp(name,"Missions")==0)
      return SPOB_SERVICE_MISSIONS;
   else if (strcasecmp(name,"Commodity")==0)
      return SPOB_SERVICE_COMMODITY;
   else if (strcasecmp(name,"Outfits")==0)
      return SPOB_SERVICE_OUTFITS;
   else if (strcasecmp(name,"Shipyard")==0)
      return SPOB_SERVICE_SHIPYARD;
   else if (strcasecmp(name,"Blackmarket")==0)
      return SPOB_SERVICE_BLACKMARKET;
   return -1;
}

/**
 * @brief Gets the long class name for a spob.
 *
 *    @param class Name of the class to process.
 *    @return Long name of the class.
 */
const char* spob_getClassName( const char *class )
{
   if (strcmp(class,"0")==0)
      return _("Civilian Station");
   else if (strcmp(class,"1")==0)
      return _("Military Station");
   else if (strcmp(class,"2")==0)
      return _("Pirate Station");
   else if (strcmp(class,"3")==0)
      return _("Robotic Station");
   else if (strcmp(class,"4")==0)
      return _("Artificial Ecosystem");
   else if (strcmp(class,"A")==0)
      return _("Geothermal");
   else if (strcmp(class,"B")==0)
      return _("Geomorteus");
   else if (strcmp(class,"C")==0)
      return _("Geoinactive");
   else if (strcmp(class,"D")==0)
      return _("Asteroid/Moon");
   else if (strcmp(class,"E")==0)
      return _("Geoplastic");
   else if (strcmp(class,"F")==0)
      return _("Geometallic");
   else if (strcmp(class,"G")==0)
      return _("Geocrystaline");
   else if (strcmp(class,"H")==0)
      return _("Desert");
   else if (strcmp(class,"I")==0)
      return _("Gas Supergiant");
   else if (strcmp(class,"J")==0)
      return _("Gas Giant");
   else if (strcmp(class,"K")==0)
      return _("Adaptable");
   else if (strcmp(class,"L")==0)
      return _("Marginal");
   else if (strcmp(class,"M")==0)
      return _("Terrestrial");
   else if (strcmp(class,"N")==0)
      return _("Reducing");
   else if (strcmp(class,"O")==0)
      return _("Pelagic");
   else if (strcmp(class,"P")==0)
      return _("Glaciated");
   else if (strcmp(class,"Q")==0)
      return _("Variable");
   else if (strcmp(class,"R")==0)
      return _("Rogue");
   else if (strcmp(class,"S")==0 || strcmp(class,"T")==0)
      return _("Ultragiants");
   else if (strcmp(class,"X")==0 || strcmp(class,"Y")==0 || strcmp(class,"Z")==0)
      return _("Demon");
   return class;
}

/**
 * @brief Gets the price of a commodity at a spob.
 *
 *    @param p Spob to get price at.
 *    @param c Commodity to get price of.
 */
credits_t spob_commodityPrice( const Spob *p, const Commodity *c )
{
   const char *sysname = spob_getSystem( p->name );
   StarSystem *sys = system_get( sysname );
   return economy_getPrice( c, sys, p );
}

/**
 * @brief Gets the price of a commodity at a spob at given time.
 *
 *    @param p Spob to get price at.
 *    @param c Commodity to get price of.
 *    @param t Time to get price at.
 */
credits_t spob_commodityPriceAtTime( const Spob *p, const Commodity *c, ntime_t t )
{
   const char *sysname = spob_getSystem( p->name );
   StarSystem *sys = system_get( sysname );
   return economy_getPriceAtTime( c, sys, p, t );
}

/**
 * @brief Adds cost of commodities on spob p to known statistics at time t.
 *
 *    @param p Spob to get price at
 *    @param tupdate Time to get prices at
 */
void spob_averageSeenPricesAtTime( const Spob *p, const ntime_t tupdate )
{
   economy_averageSeenPricesAtTime( p, tupdate );
}

/**
 * @brief Gets the average price of a commodity at a spob that has been seen so far.
 *
 *    @param p Spob to get average price at.
 *    @param c Commodity to get average price of.
 *    @param[out] mean Sample mean, rounded to nearest credit.
 *    @param[out] std Sample standard deviation (via uncorrected population formula).
 */
int spob_averageSpobPrice( const Spob *p, const Commodity *c, credits_t *mean, double *std)
{
  return economy_getAverageSpobPrice( c, p, mean, std );
}

/**
 * @brief Updates some internal calculations about asteroids in a system.
 *
 *    @param sys System to update.
 */
void system_updateAsteroids( StarSystem *sys )
{
   double density = 0.;
   for (int i=0; i<array_size(sys->asteroids); i++) {
      AsteroidAnchor *ast = &sys->asteroids[i];
      density += ast->area * ast->density / ASTEROID_REF_AREA;

      /* Have to subtract excluded area. */
      for (int j=0; j<array_size(sys->astexclude); j++) {
         AsteroidExclusion *exc = &sys->astexclude[j];
         density -= CollideCircleIntersection( &ast->pos, ast->radius, &exc->pos, exc->radius ) * ast->density / ASTEROID_REF_AREA;
      }
   }
   sys->asteroid_density = density;
}

/**
 * @brief Changes the spobs faction.
 *
 *    @param p Spob to change faction of.
 *    @param faction Faction to change to.
 *    @return 0 on success.
 */
int spob_setFaction( Spob *p, int faction )
{
   p->presence.faction = faction;
   return 0;
}

/**
 * @brief Adds a commodity to a spob.
 *
 *    @param p Spob to add commodity to.
 *    @param c Commodity to add.
 *    @return 0 on success.
 */
int spob_addCommodity( Spob *p, Commodity *c )
{
   array_grow( &p->commodities ) = c;
   array_grow( &p->commodityPrice ).price = c->price;
   return 0;
}

/**
 * @brief Removes a service from a spob.
 *
 *    @param p Spob to remove service from.
 *    @param service Service flag to remove.
 *    @return 0 on success.
 */
int spob_addService( Spob *p, int service )
{
   p->services |= service;

   if (service & SPOB_SERVICE_COMMODITY) {
      const char *sysname;
      StarSystem *sys;

      /* Only try to add standard commodities if there aren't any. */
      if (p->commodities!=NULL)
         return 0;
      Commodity **stdList = standard_commodities();
      p->commodities = array_create( Commodity* );
      p->commodityPrice = array_create( CommodityPrice );
      for (int i=0; i<array_size(stdList); i++)
         spob_addCommodity( p, stdList[i] );
      array_free( stdList );

      /* Clean up economy status. */
      economy_addQueuedUpdate();
      economy_clearSingleSpob(p);

      /* Try to figure out the system. */
      sysname = spob_getSystem( p->name );
      if (sysname==NULL) {
         DEBUG(_("Spob '%s' not in system. Not initializing economy."), p->name);
         return 0;
      }
      sys = system_get( sysname );
      economy_initialiseSingleSystem( sys, p );
   }

   return 0;
}

/**
 * @brief Removes a service from a spob.
 *
 *    @param p Spob to remove service from.
 *    @param service Service flag to remove.
 *    @return 0 on success.
 */
int spob_rmService( Spob *p, int service )
{
   p->services &= ~service;
   return 0;
}

/**
 * @brief Renames a spob.
 *
 *    @param p Spob to rename.
 *    @param newname New name to give the spob.
 *    @return 0 on success.
 */
int spob_rename( Spob *p, char *newname )
{
   int found = 0;
   for (int i=0; i<array_size(spobname_stack); i++) {
      if (strcmp(spobname_stack[i], p->name)==0) {
         spobname_stack[i] = newname;
         found = 1;
         break;
      }
   }
   if (!found)
      WARN(_("Renaming spob '%s', but not found in name stack!"),p->name);
   free( p->name );
   p->name = newname;

   /* Order changed, so no more bsearch for us. */
   spobstack_changed = 1;

   return 0;
}

/**
 * @brief Distance at which a pilot can jump.
 */
int space_jumpDistance( const Pilot *p, const JumpPoint *jp )
{
   double r = jp->radius * p->stats.jump_distance;
   if (pilot_isFlag( p, PILOT_STEALTH )) /* Stealth gives a jump distance bonus. */
      r *= 3.;
   return r;
}

/**
 * @brief Checks to make sure if pilot is far enough away to hyperspace.
 *
 *    @param p Pilot to check if he can hyperspace.
 *    @return 1 if he can hyperspace, 0 else.
 */
int space_canHyperspace( const Pilot* p )
{
   double d, r;
   JumpPoint *jp;

   /* Must not have the nojump flag. */
   if (pilot_isFlag(p, PILOT_NOJUMP))
      return 0;

   /* Must have fuel. */
   if (p->fuel < p->fuel_consumption)
      return 0;

   /* Must have hyperspace target. */
   if (0 > p->nav_hyperspace || p->nav_hyperspace >= array_size(cur_system->jumps))
      return 0;

   /* Get the jump. */
   jp = &cur_system->jumps[ p->nav_hyperspace ];

   /* Check distance. */
   r = space_jumpDistance( p, jp );
   d = vec2_dist2( &p->solid.pos, &jp->pos );
   if (d > pow2(r))
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
   if (p->fuel < p->fuel_consumption)
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
 *    @param out Star system exiting.
 *    @param[out] pos Position calculated.
 *    @param[out] vel Velocity calculated.
 *    @param[out] dir Angle calculated.
 *    @param p Pilot that is entering to use stats of (or NULL if not important).
 */
int space_calcJumpInPos( const StarSystem *in, const StarSystem *out, vec2 *pos, vec2 *vel, double *dir, const Pilot *p )
{
   JumpPoint *jp;
   double a, d, x, y;
   double ea, ed;

   /* Find the entry system. */
   jp = NULL;
   for (int i=0; i<array_size(in->jumps); i++)
      if (in->jumps[i].target == out)
         jp = &in->jumps[i];

   /* Must have found the jump. */
   if (jp == NULL) {
      WARN(_("Unable to find jump in point for '%s' in '%s': not connected"), out->name, in->name);
      return -1;
   }

   /* Base position target. */
   x = jp->pos.x;
   y = jp->pos.y;

   /* Calculate offset from target position. */
   a = 2.*M_PI - jp->angle;
   d = RNGF()*(HYPERSPACE_ENTER_MAX-HYPERSPACE_ENTER_MIN) + HYPERSPACE_ENTER_MIN;
   if ((p!=NULL) && pilot_isFlag(p, PILOT_STEALTH))
      d *= 1.4; /* Jump in from further out when coming in from stealth. */

   /* Calculate new position. */
   x += d*cos(a);
   y += d*sin(a);

   /* Add some error. */
   ea = 2.*M_PI*RNGF();
   ed = jp->radius/2.;
   if (p != NULL) {
      ed *= p->stats.jump_distance; /* larger variability. */
      if (pilot_isFlag(p, PILOT_STEALTH))
         ed *= 2.;
   }
   x += ed*cos(ea);
   y += ed*sin(ea);

   /* Set new position. */
   vec2_cset( pos, x, y );

   /* Set new velocity. */
   a += M_PI;
   vec2_cset( vel, HYPERSPACE_VEL*cos(a), HYPERSPACE_VEL*sin(a) );

   /* Set direction. */
   *dir = a;

   return 0;
}

/**
 * @brief Gets the name of all the spobs that belong to factions.
 *
 *    @param factions Array (array.h): Factions to check against.
 *    @param landable Whether the search is limited to landable spobs.
 *    @return An array (array.h) of faction names.  Individual names are not allocated.
 */
char** space_getFactionSpob( int *factions, int landable )
{
   char **tmp = array_create( char* );
   for (int i=0; i<array_size(systems_stack); i++) {
      for (int j=0; j<array_size(systems_stack[i].spobs); j++) {
         Spob *spob = systems_stack[i].spobs[j];
         int f = 0;
         for (int k=0; k<array_size(factions); k++) {
            if (spob->presence.faction == factions[k]) {
               f = 1;
               break;
            }
         }
         if (!f)
            continue;

         /* Check landable. */
         if (landable) {
            spob_updateLand( spob );
            if (!spob->can_land)
               continue;
         }

         /* This is expensive so we probably want to do it last. */
         if (!space_sysReallyReachable( systems_stack[i].name ))
            continue;

         array_push_back( &tmp, spob->name );
         break; /* no need to check all factions */
      }
   }

   return tmp;
}

/**
 * @brief Gets the name of a random spob.
 *
 *    @param landable Whether the spob must let the player land normally.
 *    @param services Services the spob must have.
 *    @param filter Filter function for including spobs.
 *    @return The name (internal/English) of a random spob.
 */
const char* space_getRndSpob( int landable, unsigned int services,
      int (*filter)(Spob *p))
{
   char *res = NULL;
   Spob **tmp = array_create( Spob* );

   for (int i=0; i<array_size(systems_stack); i++) {
      for (int j=0; j<array_size(systems_stack[i].spobs); j++) {
         Spob *pnt = systems_stack[i].spobs[j];

         if (services && spob_hasService(pnt, services) != services)
            continue;

         if (filter != NULL && !filter(pnt))
            continue;

         array_push_back( &tmp, pnt );
      }
   }

   /* Second filter. */
   arrayShuffle( (void**)tmp );
   for (int i=0; i < array_size(tmp); i++) {
      Spob *pnt = tmp[i];

      /* We put expensive calculations here to minimize executions. */
      if (landable) {
         spob_updateLand( pnt );
         if (!pnt->can_land)
            continue;
      }
      if (!space_sysReallyReachable( spob_getSystem(pnt->name) ))
         continue;

      /* We want the name, not the actual spob. */
      res = tmp[i]->name;
      break;
   }
   array_free(tmp);

   return res;
}

/**
 * @brief Gets the closest feature to a position in the system.
 *
 *    @param sys System to get closest feature from a position.
 *    @param[out] pnt ID of closest spob or -1 if a jump point is closer (or none is close).
 *    @param[out] jp ID of closest jump point or -1 if a spob is closer (or none is close).
 *    @param[out] ast ID of closest asteroid or -1 if something else is closer (or none is close).
 *    @param[out] fie ID of the asteroid anchor the asteroid belongs to.
 *    @param x X position to get closest from.
 *    @param y Y position to get closest from.
 */
double system_getClosest( const StarSystem *sys, int *pnt, int *jp, int *ast, int *fie, double x, double y )
{
   double d = HUGE_VAL;

   /* Default output. */
   *pnt = -1;
   *jp  = -1;
   *ast = -1;
   *fie = -1;

   /* Spobs. */
   for (int i=0; i<array_size(sys->spobs); i++) {
      double td;
      Spob *p  = sys->spobs[i];
      if (!spob_isKnown(p))
         continue;
      td = pow2(x-p->pos.x) + pow2(y-p->pos.y);
      if (td < d) {
         *pnt  = i;
         d     = td;
      }
   }

   /* Asteroids. */
   for (int i=0; i<array_size(sys->asteroids); i++) {
      AsteroidAnchor *f = &sys->asteroids[i];
      for (int k=0; k<f->nb; k++) {
         double td;
         Asteroid *as = &f->asteroids[k];

         /* Skip non-interactive asteroids. */
         if (as->state != ASTEROID_FG)
            continue;

         /* Skip out of range asteroids */
         if (!pilot_inRangeAsteroid( player.p, k, i ))
            continue;

         td = pow2(x-as->pos.x) + pow2(y-as->pos.y);
         if (td < d) {
            *pnt  = -1; /* We must clear spob target as asteroid is closer. */
            *ast  = k;
            *fie  = i;
            d     = td;
         }
      }
   }

   /* Jump points. */
   for (int i=0; i<array_size(sys->jumps); i++) {
      double td;
      JumpPoint *j  = &sys->jumps[i];
      if (!jp_isUsable(j))
         continue;
      td = pow2(x-j->pos.x) + pow2(y-j->pos.y);
      if (td < d) {
         *pnt  = -1; /* We must clear spob target as jump point is closer. */
         *ast  = -1;
         *fie  = -1;
         *jp   = i;
         d     = td;
      }
   }
   return d;
}

/**
 * @brief Gets the feature nearest to directly ahead of a position in the system.
 *
 *    @param sys System to get closest feature from a position.
 *    @param[out] pnt ID of closest spob or -1 if something else is closer (or none is close).
 *    @param[out] jp ID of closest jump point or -1 if something else is closer (or none is close).
 *    @param[out] ast ID of closest asteroid or -1 if something else is closer (or none is close).
 *    @param[out] fie ID of the asteroid anchor the asteroid belongs to.
 *    @param x X position to get closest from.
 *    @param y Y position to get closest from.
 *    @param ang Reference angle.
 *    @return The nearest angle to \p ang which is the direction from (\p x, \p y) to a feature.
 */
double system_getClosestAng( const StarSystem *sys, int *pnt, int *jp, int *ast, int *fie, double x, double y, double ang )
{
   double a;

   /* Default output. */
   *pnt = -1;
   *jp  = -1;
   a    = ang + M_PI;

   /* Spobs. */
   for (int i=0; i<array_size(sys->spobs); i++) {
      Spob *p = sys->spobs[i];
      double ta = atan2( y - p->pos.y, x - p->pos.x);
      if ( ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
         *pnt  = i;
         a     = ta;
      }
   }

   /* Asteroids. */
   for (int i=0; i<array_size(sys->asteroids); i++) {
      AsteroidAnchor *f = &sys->asteroids[i];
      for (int k=0; k<f->nb; k++) {
         double ta;
         Asteroid *as = &f->asteroids[k];

         /* Skip non-interactive asteroids. */
         if (as->state != ASTEROID_FG)
            continue;

         ta = atan2( y - as->pos.y, x - as->pos.x);
         if ( ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
            *pnt  = -1; /* We must clear spob target as asteroid is closer. */
            *ast  = k;
            *fie  = i;
            a     = ta;
         }
      }
   }

   /* Jump points. */
   for (int i=0; i<array_size(sys->jumps); i++) {
      JumpPoint *j = &sys->jumps[i];
      double ta = atan2( y - j->pos.y, x - j->pos.x);
      if ( ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
         *ast  = -1;
         *fie  = -1;
         *pnt  = -1; /* We must clear the rest as jump point is closer. */
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
int space_sysReachable( const StarSystem *sys )
{
   if (sys_isKnown(sys))
      return 1; /* it is known */

   /* check to see if it is adjacent to known */
   for (int i=0; i<array_size(sys->jumps); i++) {
      JumpPoint *jp = sys->jumps[i].returnJump;
      if (jp && jp_isUsable( jp ))
         return 1;
   }

   return 0;
}

/**
 * @brief Sees if a system can be reached via jumping.
 *
 *    @return 1 if target system is reachable, 0 if it isn't.
 */
int space_sysReallyReachable( const char* sysname )
{
   StarSystem** path;

   if (strcmp(sysname,cur_system->name)==0)
      return 1;
   path = map_getJumpPath( cur_system->name, sysname, 1, 1, NULL );
   if (path != NULL) {
      array_free(path);
      return 1;
   }
   return 0;
}

/**
 * @brief Sees if a system is reachable from another system.
 *
 *    @return 1 if target system is reachable, 0 if it isn't.
 */
int space_sysReachableFromSys( const StarSystem *target, const StarSystem *sys )
{
   /* check to see if sys contains a known jump point to target */
   JumpPoint *jp = jump_getTarget( target, sys );
   if (jp == NULL)
      return 0;
   else if (jp_isUsable( jp ))
      return 1;
   return 0;
}

/**
 * @brief Gets an array (array.h) of all star systems.
 */
StarSystem* system_getAll (void)
{
   return systems_stack;
}

/**
 * @brief Checks to see if a system exists case insensitively.
 *
 *    @param sysname Name of the system to match (case insensitive).
 *    @return The actual name of the system of NULL if not found.
 */
const char *system_existsCase( const char* sysname )
{
   for (int i=0; i<array_size(systems_stack); i++)
      if (strcasecmp(sysname, systems_stack[i].name)==0)
         return systems_stack[i].name;
   return NULL;
}

/**
 * @brief Does a fuzzy case matching. Searches translated names but returns internal names.
 */
char **system_searchFuzzyCase( const char* sysname, int *n )
{
   int len;
   char **names;

   /* Overallocate to maximum. */
   names = malloc( sizeof(char*) * array_size(systems_stack) );

   /* Do fuzzy search. */
   len = 0;
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      if (SDL_strcasestr( _(sys->name), sysname ) != NULL) {
         names[len] = sys->name;
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
 * @brief Comparison function for qsort'ing StarSystem by name.
 */
static int system_cmp( const void *p1, const void *p2 )
{
   const StarSystem *s1, *s2;
   s1 = (const StarSystem*) p1;
   s2 = (const StarSystem*) p2;
   return strcmp(s1->name,s2->name);
}

/**
 * @brief Get the system from its name.
 *
 *    @param sysname Name to match.
 *    @return System matching sysname.
 */
StarSystem* system_get( const char* sysname )
{
   if (sysname == NULL)
      return NULL;

#ifdef DEBUGGING
   if (systemstack_changed) {
      for (int i=0; i<array_size(systems_stack); i++)
         if (strcmp(systems_stack[i].name, sysname)==0)
            return &systems_stack[i];
      WARN(_("System '%s' not found in stack"), sysname);
      return NULL;
   }
#endif /* DEBUGGING */

   const StarSystem s = {.name = (char*)sysname};
   StarSystem *found = bsearch( &s, systems_stack, array_size(systems_stack), sizeof(StarSystem), system_cmp );
   if (found != NULL)
      return found;

   WARN(_("System '%s' not found in stack"), sysname);
   return NULL;
}

/**
 * @brief Get the system by its index.
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
int system_index( const StarSystem *sys )
{
   return sys->id;
}

/**
 * @brief Get whether or not a spob has a system (i.e. is on the map).
 *
 *    @param spb Spob to search for.
 *    @return 1 if the spob has a system, 0 otherwise.
 */
int spob_hasSystem( const Spob *spb )
{
   for (int i=0; i<array_size(spobname_stack); i++)
      if (strcmp(spobname_stack[i],spb->name)==0)
         return 1;
   return 0;
}

/**
 * @brief Get the name of a system from a spobname.
 *
 *    @param spobname Spob name to match.
 *    @return Name of the system spob belongs to.
 */
const char* spob_getSystem( const char* spobname )
{
   for (int i=0; i<array_size(spobname_stack); i++)
      if (strcmp(spobname_stack[i],spobname)==0)
         return systemname_stack[i];
   DEBUG(_("Spob '%s' is not placed in a system"), spobname);
   return NULL;
}

/**
 * @brief Comparison function for qsort'ing Spob by name.
 */
static int spob_cmp( const void *p1, const void *p2 )
{
   const Spob *pnt1, *pnt2;
   pnt1 = (const Spob*) p1;
   pnt2 = (const Spob*) p2;
   return strcmp(pnt1->name,pnt2->name);
}

/**
 * @brief Gets a spob based on its name.
 *
 *    @param spobname Name to match.
 *    @return Spob matching spobname.
 */
Spob* spob_get( const char* spobname )
{
   if (spobname==NULL) {
      WARN(_("Trying to find NULL spob…"));
      return NULL;
   }

#ifdef DEBUGGING
   if (spobstack_changed) {
      for (int i=0; i<array_size(spob_stack); i++)
         if (strcmp(spob_stack[i].name, spobname)==0)
            return &spob_stack[i];
      WARN(_("Spob '%s' not found in the universe"), spobname);
      return NULL;
   }
#endif /* DEBUGGING */

   const Spob p = {.name = (char*)spobname};
   Spob *found = bsearch( &p, spob_stack, array_size(spob_stack), sizeof(Spob), spob_cmp );
   if (found != NULL)
      return found;

   WARN(_("Spob '%s' not found in the universe"), spobname);
   return NULL;
}

/**
 * @brief Gets spob by index.
 *
 *    @param ind Index of the spob to get.
 *    @return The spob gotten.
 */
Spob* spob_getIndex( int ind )
{
   /* Validity check. */
   if ((ind < 0) || (ind >= array_size(spob_stack))) {
      WARN(_("Spob index '%d' out of range (max %d)"), ind, array_size(spob_stack));
      return NULL;
   }

   return &spob_stack[ ind ];
}

/**
 * @brief Gets the ID of a spob.
 *
 *    @param p Spob to get ID of.
 *    @return The ID of the spob.
 */
int spob_index( const Spob *p )
{
   return p->id;
}

/**
 * @brief Gets an array (array.h) of all spobs.
 */
Spob* spob_getAll (void)
{
   return spob_stack;
}

/**
 * @brief Sets a spob's known status, if it's real.
 */
void spob_setKnown( Spob *p )
{
   spob_setFlag(p, SPOB_KNOWN);
}

/**
 * @brief Check to see if a spob exists.
 *
 *    @param spobname Name of the spob to see if it exists.
 *    @return 1 if spob exists.
 */
int spob_exists( const char* spobname )
{
   for (int i=0; i<array_size(spob_stack); i++)
      if (strcmp(spob_stack[i].name,spobname)==0)
         return 1;
   return 0;
}

/**
 * @brief Check to see if a spob exists (case insensitive).
 *
 *    @param spobname Name of the spob to see if it exists.
 *    @return The actual name of the spob or NULL if not found.
 */
const char* spob_existsCase( const char* spobname )
{
   for (int i=0; i<array_size(spob_stack); i++)
      if (strcasecmp(spob_stack[i].name,spobname)==0)
         return spob_stack[i].name;
   return NULL;
}

/**
 * @brief Does a fuzzy case matching. Searches spob_name() but returns internal names.
 */
char **spob_searchFuzzyCase( const char* spobname, int *n )
{
   int len;
   char **names;

   /* Overallocate to maximum. */
   names = malloc( sizeof(char*) * array_size(spob_stack) );

   /* Do fuzzy search. */
   len = 0;
   for (int i=0; i<array_size(spob_stack); i++) {
      Spob *spob = &spob_stack[i];
      if (SDL_strcasestr( spob_name(spob), spobname ) != NULL) {
         names[len] = spob->name;
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
 * @brief Gets all the virtual spobs.
 */
VirtualSpob* virtualspob_getAll (void)
{
   return vspob_stack;
}

/**
 * @brief Comparison function for qsort'ing VirtuaSpob by name.
 */
static int virtualspob_cmp( const void *p1, const void *p2 )
{
   const VirtualSpob *v1, *v2;
   v1 = (const VirtualSpob*) p1;
   v2 = (const VirtualSpob*) p2;
   return strcmp(v1->name,v2->name);
}

/**
 * @brief Gets a virtual spob by matching name.
 */
VirtualSpob* virtualspob_get( const char *name )
{
   const VirtualSpob va = {.name = (char*)name};
   VirtualSpob *found = bsearch( &va, vspob_stack, array_size(vspob_stack), sizeof(VirtualSpob), virtualspob_cmp );
   if (found != NULL)
      return found;
   WARN(_("Virtual Spob '%s' not found in the universe"), name);
   return NULL;
}

/**
 * @brief Gets a jump point based on its target and system.
 *
 *    @param jumpname Name to match.
 *    @param sys System jump is in.
 *    @return Jump point matich jumpname in sys or NULL if not found.
 */
JumpPoint* jump_get( const char* jumpname, const StarSystem* sys )
{
   if (jumpname==NULL) {
      WARN(_("Trying to find NULL jump point..."));
      return NULL;
   }

   for (int i=0; i<array_size(sys->jumps); i++) {
      JumpPoint *jp = &sys->jumps[i];
      if (strcmp(jp->target->name,jumpname)==0)
         return jp;
   }

   WARN(_("Jump point '%s' not found in %s"), jumpname, sys->name);
   return NULL;
}

/**
 * @brief Less safe version of jump_get that works with pointers.
 *
 *    @param target Target system jump leads to.
 *    @param sys System to look in.
 *    @return Jump point in sys to target or NULL if not found.
 */
JumpPoint* jump_getTarget( const StarSystem* target, const StarSystem* sys )
{
   for (int i=0; i<array_size(sys->jumps); i++) {
      JumpPoint *jp = &sys->jumps[i];
      if (jp->target == target)
         return jp;
   }
   WARN(_("Jump point to '%s' not found in %s"), target->name, sys->name);
   return NULL;
}

/**
 * @brief Gets the jump point symbol.
 */
const char *jump_getSymbol( const JumpPoint *jp )
{
   if (jp_isFlag(jp, JP_HIDDEN))
      return "* ";
   return "";
}

/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 *    @param init Should be 1 to initialize the scheduler.
 */
static void system_scheduler( double dt, int init )
{
   /* Go through all the factions and reduce the timer. */
   for (int i=0; i < array_size(cur_system->presence); i++) {
      int n;
      nlua_env env;
      SystemPresence *p = &cur_system->presence[i];
      if (p->value <= 0.)
         continue;

      env = faction_getScheduler( p->faction );

      /* Must have a valid scheduler. */
      if (env==LUA_NOREF)
         continue;

      /* Spawning is disabled for this faction. */
      if (p->disabled)
         continue;

      /* Run the appropriate function. */
      if (init) {
         nlua_getenv( naevL, env, "create" ); /* f */
         if (lua_isnil(naevL,-1)) {
            WARN(_("Lua Spawn script for faction '%s' missing obligatory entry point 'create'."),
                  faction_name( p->faction ) );
            lua_pop(naevL,1);
            continue;
         }
         n = 0;
      }
      else {
         /* Decrement dt, only continue  */
         p->timer -= dt;
         if (p->timer >= 0.)
            continue;

         nlua_getenv( naevL, env, "spawn" ); /* f */
         if (lua_isnil(naevL,-1)) {
            WARN(_("Lua Spawn script for faction '%s' missing obligatory entry point 'spawn'."),
                  faction_name( p->faction ) );
            lua_pop(naevL,1);
            continue;
         }
         lua_pushnumber( naevL, p->curUsed ); /* f, presence */
         n = 1;
      }
      lua_pushnumber( naevL, p->value ); /* f, [arg,], max */

      /* Actually run the function. */
      if (nlua_pcall(env, n+1, 2)) { /* error has occurred */
         WARN(_("Lua Spawn script for faction '%s' : %s"),
               faction_name( p->faction ), lua_tostring(naevL,-1));
         lua_pop(naevL,1);
         continue;
      }

      /* Output is handled the same way. */
      if (!lua_isnumber(naevL,-2)) {
         WARN(_("Lua spawn script for faction '%s' failed to return timer value."),
               faction_name( p->faction ) );
         lua_pop(naevL,2);
         continue;
      }
      p->timer    += lua_tonumber(naevL,-2);
      /* Handle table if it exists. */
      if (lua_istable(naevL,-1)) {
         lua_pushnil(naevL); /* tk, k */
         while (lua_next(naevL,-2) != 0) { /* tk, k, v */
            Pilot *pilot;

            /* Must be table. */
            if (!lua_istable(naevL,-1)) {
               WARN(_("Lua spawn script for faction '%s' returns invalid data (not a table)."),
                     faction_name( p->faction ) );
               lua_pop(naevL,2); /* tk, k */
               continue;
            }

            lua_getfield( naevL, -1, "pilot" ); /* tk, k, v, p */
            if (!lua_ispilot(naevL,-1)) {
               WARN(_("Lua spawn script for faction '%s' returns invalid data (not a pilot)."),
                     faction_name( p->faction ) );
               lua_pop(naevL,2); /* tk, k */
               continue;
            }
            pilot = pilot_get( lua_topilot(naevL,-1) );
            if (pilot == NULL) {
               lua_pop(naevL,2); /* tk, k */
               continue;
            }
            lua_pop(naevL,1); /* tk, k, v */
            lua_getfield( naevL, -1, "presence" ); /* tk, k, v, p */
            if (!lua_isnumber(naevL,-1)) {
               WARN(_("Lua spawn script for faction '%s' returns invalid data (not a number)."),
                     faction_name( p->faction ) );
               lua_pop(naevL,2); /* tk, k */
               continue;
            }
            pilot->presence = lua_tonumber(naevL,-1);
            if (pilot->faction != p->faction) {
               int pi;
               WARN( _("Lua spawn script for faction '%s' actually spawned a '%s' pilot."),
                     faction_name( p->faction ),
                     faction_name( pilot->faction ) );
               pi = getPresenceIndex( cur_system, pilot->faction );
               p = &cur_system->presence[pi];
            }
            p->curUsed     += pilot->presence;
            lua_pop(naevL,2); /* tk, k */
         }
      }
      lua_pop(naevL,2);
   }
}

/**
 * @brief Mark when a faction changes.
 */
void space_factionChange (void)
{
   space_fchg = 1;
}

/**
 * @brief Handles landing if necessary.
 */
void space_checkLand (void)
{
   if (space_landQueueSpob != NULL) {
      land( space_landQueueSpob, 0 );
      space_landQueueSpob = NULL;
   }
}

/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 *    @param real_dt Real time incrcement (in real world seconds).
 */
void space_update( double dt, double real_dt )
{
   /* Needs a current system. */
   if (cur_system == NULL)
      return;

   /* If spawning is enabled, call the scheduler. */
   if (space_spawn)
      system_scheduler( dt, 0 );

   /*
    * Nebula.
    */
   nebu_update( dt );
   if (cur_system->nebu_volatility > 0.) {
      Pilot *const* pilot_stack;
      Damage dmg;
      dmg.type          = dtype_get("nebula");
      dmg.damage        = cur_system->nebu_volatility * dt;
      dmg.penetration   = 1.; /* Full penetration. */
      dmg.disable       = 0.;

      /* Damage pilots in volatile systems. */
      pilot_stack = pilot_getAll();
      for (int i=0; i<array_size(pilot_stack); i++)
         pilot_hit( pilot_stack[i], NULL, NULL, &dmg, NULL, LUA_NOREF, 0 );
   }

   /* Faction updates. */
   if (space_fchg) {
      for (int i=0; i<array_size(cur_system->spobs); i++)
         spob_updateLand( cur_system->spobs[i] );

      /* Verify land authorization is still valid. */
      if ((player.p != NULL) && (player.p->nav_spob >= 0) && player_isFlag(PLAYER_LANDACK))
         player_checkLandAck();

      gui_updateFaction();
      space_fchg = 0;
   }

   if (!space_simulating) {
      int found_something = 0;
      /* Spob updates */
      for (int i=0; i<array_size(cur_system->spobs); i++) {
         HookParam hparam[3];
         Spob *pnt = cur_system->spobs[i];

         /* Must update in some cases. */
         space_updateSpob( pnt, dt, real_dt );

         /* Discovering is disabled. */
         if (player.discover_off)
            continue;

         /* Handle discoveries. */
         if (spob_isKnown( pnt ) || !pilot_inRangeSpob( player.p, i ))
            continue;

         spob_setKnown( pnt );
         player_message( _("You discovered #%c%s#0."),
               spob_getColourChar( pnt ),
               spob_name( pnt ) );
         hparam[0].type  = HOOK_PARAM_STRING;
         hparam[0].u.str = "spob";
         hparam[1].type  = HOOK_PARAM_SPOB;
         hparam[1].u.la  = pnt->id;
         hparam[2].type  = HOOK_PARAM_SENTINEL;
         hooks_runParam( "discover", hparam );
         found_something = 1;
         pnt->map_alpha = 0.;
      }

      /* Jump point updates */
      for (int i=0; i<array_size(cur_system->jumps); i++) {
         HookParam hparam[3];
         JumpPoint *jp = &cur_system->jumps[i];

         /* Discovering is disabled. */
         if (player.discover_off)
            continue;

         if (jp_isKnown(jp))
            continue;
         if (jp_isFlag(jp,JP_EXITONLY))
            continue;
         if (!(pilot_inRangeJump( player.p, i )))
            continue;

         jp_setFlag( jp, JP_KNOWN );
         player_message( _("You discovered a Jump Point.") );
         hparam[0].type  = HOOK_PARAM_STRING;
         hparam[0].u.str = "jump";
         hparam[1].type  = HOOK_PARAM_JUMP;
         hparam[1].u.lj.srcid = cur_system->id;
         hparam[1].u.lj.destid = jp->target->id;
         hparam[2].type  = HOOK_PARAM_SENTINEL;
         hooks_runParam( "discover", hparam );
         found_something = 1;
         jp->map_alpha = 0.;
      }

      if (found_something)
         ovr_refresh();
   }

   /* Update the gatherable objects. */
   gatherable_update( dt );

   /* Asteroids/Debris update */
   asteroids_update( dt );
}

/**
 * @brief returns whether we're just simulating.
 */
int space_isSimulation( void )
{
   return space_simulating;
}

/**
 * @brief returns whether or not we're simulating with effects.
 */
int space_needsEffects (void)
{
   return space_simulating_effects;
}

/**
 * @brief Initializes the system.
 *
 *    @param sysname Name of the system to initialize.
 *    @param do_simulate Whether or not perform the initial simulation.
 */
void space_init( const char* sysname, int do_simulate )
{
   int n, s;
   const double fps_min_simulation = fps_min * 2.;
   StarSystem *oldsys = cur_system;

   /* cleanup some stuff */
   player_clear(); /* clears targets */
   ovr_mrkClear(); /* Clear markers when jumping. */
   pilots_clean(1); /* destroy non-persistent pilots */
   weapon_clear(); /* get rid of all the weapons */
   spfx_clear(); /* get rid of the explosions */
   gatherable_free(); /* get rid of gatherable stuff. */
   background_clear(); /* Get rid of the background. */
   factions_clearDynamic(); /* get rid of dynamic factions. */
   space_spawn = 1; /* spawn is enabled by default. */

   /* Clear persistent pilot stuff. */
   if (player.p != NULL) {
      Pilot *const* pilot_stack = pilot_getAll();
      for (int i=0; i<array_size(pilot_stack); i++) {
         Pilot *p = pilot_stack[i];
         pilot_lockClear( p );
         pilot_clearTimers( p ); /* Clear timers. */
      }
   }

   if ((sysname==NULL) && (cur_system==NULL))
      ERR(_("Cannot reinit system if there is no system previously loaded"));
   else if (sysname!=NULL) {
      cur_system = system_get( sysname );
      if (cur_system == NULL) {
         WARN(_("System '%s' not found, trying random system!"),sysname);
         cur_system = &systems_stack[ RNG(0,array_size(systems_stack)-1) ];
      }
      char *nt = ntime_pretty(0, 2);

      player_message(_("#oEntering System %s on %s."), _(sysname), nt);
      if (cur_system->nebu_volatility > 0.)
         player_message(_("#rWARNING - Volatile nebula detected in %s! Taking %.1f MW damage!"), _(sysname), cur_system->nebu_volatility);
      free(nt);

      /* Handle background */
      if (cur_system->nebu_density > 0.) {
         /* Background is Nebula */
         nebu_prep( cur_system->nebu_density, cur_system->nebu_volatility, cur_system->nebu_hue );

         /* Set up sound. */
         sound_env( SOUND_ENV_NEBULA, cur_system->nebu_density );
      }
      else {
         /* Background is starry */
         background_initDust( cur_system->spacedust );

         /* Set up sound. */
         sound_env( SOUND_ENV_NORMAL, 0. );
      }
   }

   /* Update after setting cur_system. */
   if ((oldsys != NULL && oldsys->stats != NULL) || cur_system->stats != NULL) {
      Pilot *const* pilot_stack = pilot_getAll();
      for (int i=0; i<array_size(pilot_stack); i++) {
         Pilot *p = pilot_stack[i];
         pilot_calcStats( p );
         if (pilot_isWithPlayer(p))
            pilot_setFlag( p, PILOT_HIDE );
      }
   }

   /* Set up spobs. */
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *pnt = cur_system->spobs[i];
      pnt->land_override = 0;
      spob_updateLand( pnt );
   }

   /* See if we should get a new music song. */
   if ((player.p != NULL) && do_simulate)
      music_choose(NULL);

   /* Reset new trails. */
   pilots_newSystem();

   /* Reset any schedules and used presence. */
   for (int i=0; i<array_size(cur_system->presence); i++) {
      cur_system->presence[i].curUsed  = 0;
      cur_system->presence[i].timer    = 0.;
      cur_system->presence[i].disabled = 0;
   }

   /* Load graphics. */
   space_gfxLoad( cur_system );

   /* Call the scheduler. */
   system_scheduler( 0., 1 );

   /* we now know this system */
   sys_setFlag(cur_system,SYSTEM_KNOWN);

   /* Simulate system. */
   space_simulating = 1;
   space_simulating_effects = 0;
   asteroids_init(); /* Set up asteroids. */
   if (player.p != NULL) {
      Pilot *const* pilot_stack = pilot_getAll();
      pilot_setFlag( player.p, PILOT_HIDE );
      for (int i=0; i<array_size(pilot_stack); i++) {
         Pilot *p = pilot_stack[i];
         if (pilot_isWithPlayer(p))
            pilot_setFlag( p, PILOT_HIDE );
      }
   }
   player_messageToggle( 0 );
   if (do_simulate) {
      /* Uint32 time = SDL_GetTicks(); */
      s = sound_disabled;
      sound_disabled = 1;
      ntime_allowUpdate( 0 );
      n = SYSTEM_SIMULATE_TIME_PRE / fps_min_simulation;
      for (int i=0; i<n; i++)
         update_routine( fps_min_simulation, 1 );
      space_simulating_effects = 1;
      n = SYSTEM_SIMULATE_TIME_POST / fps_min_simulation;
      for (int i=0; i<n; i++)
         update_routine( fps_min_simulation, 1 );
      ntime_allowUpdate( 1 );
      sound_disabled = s;
      /*
      if (conf.devmode)
         DEBUG(_("System simulated in %.3f s"), (SDL_GetTicks()-time)/1000.);
      */
   }
   player_messageToggle( 1 );
   if (player.p != NULL) {
      Pilot *const* pilot_stack = pilot_getAll();
      pilot_rmFlag( player.p, PILOT_HIDE );
      for (int i=0; i<array_size(pilot_stack); i++) {
         Pilot *p = pilot_stack[i];
         if (pilot_isWithPlayer(p))
            pilot_rmFlag( p, PILOT_HIDE );
      }
   }
   space_simulating_effects = 1;
   space_simulating = 0;

   /* Refresh overlay if necessary (player kept it open). */
   ovr_refresh();

   /* Update gui. */
   gui_setSystem();

   /* Start background. */
   background_load( cur_system->background );
}

/**
 * @brief Creates a new spob.
 */
Spob *spob_new (void)
{
   Spob *p, *old_stack;
   int realloced;

#if DEBUGGING
   if (!systems_loading)
      spobstack_changed = 1;
#else /* DEBUGGING */
   if (!systems_loading)
      WARN(_("Creating new spob in non-debugging mode. Things are probably going to break horribly."));
#endif /* DEBUGGING */

   /* Grow and initialize memory. */
   old_stack   = spob_stack;
   p           = &array_grow( &spob_stack );
   realloced   = (old_stack!=spob_stack);
   memset( p, 0, sizeof(Spob) );
   p->id       = array_size(spob_stack)-1;
   p->presence.faction = -1;

   /* Lua doesn't default to 0 as a safe value... */
   p->lua_env     = LUA_NOREF;
   p->lua_mem     = LUA_NOREF;
   p->lua_init    = LUA_NOREF;
   p->lua_load    = LUA_NOREF;
   p->lua_unload  = LUA_NOREF;
   p->lua_can_land= LUA_NOREF;
   p->lua_land    = LUA_NOREF;
   p->lua_render  = LUA_NOREF;
   p->lua_update  = LUA_NOREF;
   p->lua_comm    = LUA_NOREF;

   /* Reconstruct the jumps. */
   if (!systems_loading && realloced)
      systems_reconstructSpobs();

   return p;
}

/**
 * @brief Gets the translated name of a spob.
 *
 *    @param p Spob to get translated name of.
 *    @return Translated name of the spob.
 */
const char *spob_name( const Spob *p )
{
   if (p->display)
      return _(p->display);
   return _(p->name);
}

/**
 * @brief Loads all the spobs in the game.
 *
 *    @return 0 on success.
 */
static int spobs_load (void)
{
   char **spob_files;
   Commodity **stdList;

   /* Initialize stack if needed. */
   if (spob_stack == NULL)
      spob_stack = array_create_size(Spob, 256);

   /* Extract the list of standard commodities. */
   stdList = standard_commodities();

   /* Load XML stuff. */
   spob_files = ndata_listRecursive( SPOB_DATA_PATH );
   for (int i=0; i<array_size(spob_files); i++) {
      if (ndata_matchExt( spob_files[i], "xml" )) {
         Spob s;
         int ret = spob_parse( &s, spob_files[i], stdList );
         if (ret == 0) {
            s.id = array_size( spob_stack );
            array_push_back( &spob_stack, s );
         }

         /* Render if necessary. */
         naev_renderLoadscreen();
      }

      /* Clean up. */
      free( spob_files[i] );
   }
   qsort( spob_stack, array_size(spob_stack), sizeof(Spob), spob_cmp );
   for (int j=0; j<array_size(spob_stack); j++)
      spob_stack[j].id = j;

   /* Clean up. */
   array_free( spob_files );
   array_free( stdList );

   return 0;
}

/**
 * @brief Loads all the virtual spobs.
 *
 *    @return 0 on success.
 */
static int virtualspobs_load (void)
{
   char **spob_files;

   /* Initialize stack if needed. */
   if (vspob_stack == NULL)
      vspob_stack = array_create_size(VirtualSpob, 64);

   /* Load XML stuff. */
   spob_files = ndata_listRecursive( VIRTUALSPOB_DATA_PATH );
   for (int i=0; i<array_size(spob_files); i++) {
      xmlDocPtr doc;
      xmlNodePtr node;

      if (!ndata_matchExt( spob_files[i], "xml" )) {
         free( spob_files[i] );
         continue;
      }

      doc = xml_parsePhysFS( spob_files[i] );
      if (doc == NULL) {
         free( spob_files[i] );
         continue;
      }

      node = doc->xmlChildrenNode; /* first spob node */
      if (node == NULL) {
         WARN(_("Malformed %s file: does not contain elements"), spob_files[i]);
         free( spob_files[i] );
         xmlFreeDoc(doc);
         continue;
      }

      if (xml_isNode(node,XML_SPOB_TAG)) {
         xmlNodePtr cur;
         VirtualSpob va;
         memset( &va, 0, sizeof(va) );
         xmlr_attr_strd( node, "name", va.name );
         va.presences = array_create( SpobPresence );

         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"presence")) {
               SpobPresence ap;
               spob_parsePresence( cur, &ap );
               array_push_back( &va.presences, ap );
               continue;
            }

            WARN(_("Unknown node '%s' in virtual spob '%s'"),cur->name,va.name);
         } while (xml_nextNode(cur));

         array_push_back( &vspob_stack, va );
      }

      /* Clean up. */
      free( spob_files[i] );
      xmlFreeDoc(doc);
   }
   qsort( vspob_stack, array_size(vspob_stack), sizeof(VirtualSpob), virtualspob_cmp );

   /* Clean up. */
   array_free( spob_files );

   return 0;
}

/**
 * @brief Gets the spob colour char.
 */
char spob_getColourChar( const Spob *p )
{
   if (!spob_hasService( p, SPOB_SERVICE_INHABITED ))
      return 'I';

   if (p->can_land) {
      if (areAllies(FACTION_PLAYER,p->presence.faction))
         return 'F';
      return 'N';
   }

   if (areEnemies(FACTION_PLAYER,p->presence.faction))
      return 'H';
   return 'R';
}

/**
 * @brief Gets the spob symbol.
 */
const char *spob_getSymbol( const Spob *p )
{
   if (!spob_hasService( p, SPOB_SERVICE_INHABITED )) {
      if (spob_hasService( p, SPOB_SERVICE_LAND ))
         return "= ";
      return "";
   }

   if (p->can_land) {
      if (areAllies(FACTION_PLAYER,p->presence.faction))
         return "+ ";
      return "~ ";
   }

   if (areEnemies(FACTION_PLAYER,p->presence.faction))
      return "!! ";
   return "* ";
}

/**
 * @brief Gets the spob colour.
 */
const glColour* spob_getColour( const Spob *p )
{
   if (!spob_hasService( p, SPOB_SERVICE_INHABITED ))
      return &cInert;

   if (p->can_land) {
      if (areAllies(FACTION_PLAYER,p->presence.faction))
         return &cFriend;
      return &cNeutral;
   }

   if (areEnemies(FACTION_PLAYER,p->presence.faction))
      return &cHostile;
   return &cRestricted;
}

/**
 * @brief Updates the land possibilities of a spob.
 *
 *    @param p Spob to update land possibilities of.
 */
void spob_updateLand( Spob *p )
{
   /* Clean up old stuff. */
   free( p->land_msg );
   p->can_land    = 0;
   p->land_msg    = NULL;

   /* Run custom Lua. */
   if (p->lua_can_land != LUA_NOREF) {
      spob_luaInitMem( p );
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, p->lua_can_land); /* f */
      if (nlua_pcall( p->lua_env, 0, 2 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), p->name, "can_land", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
         return;
      }

      p->can_land = lua_toboolean(naevL,-2);
      if (lua_isstring(naevL,-1))
         p->land_msg = strdup( lua_tostring(naevL,-1) );
      lua_pop(naevL,2);

      return;
   }

   /* Some defaults. */
   if (spob_hasService( p, SPOB_SERVICE_LAND )) {
      p->can_land = 1;
      p->land_msg = strdup(_("Landing permission granted."));
   }
}

/**
 * @brief Initializes the memory fo a spob.
 */
void spob_luaInitMem( const Spob *spob )
{
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, spob->lua_mem );
   nlua_setenv( naevL, spob->lua_env, "mem" );
}

/**
 * @brief Updatse the spob's internal Lua stuff.
 *
 *    @param spob Spob to update.
 */
int spob_luaInit( Spob *spob )
{
   int mem;

   /* Just clear everything. */
#define UNREF( x ) \
   do { if ((x) != LUA_NOREF) { \
      luaL_unref( naevL, LUA_REGISTRYINDEX, (x) ); \
      (x) = LUA_NOREF; \
   } } while (0)
   spob->lua_env = LUA_NOREF; /* Just a pointer to some Lua index. */
   UNREF( spob->lua_init );
   UNREF( spob->lua_load );
   UNREF( spob->lua_unload );
   UNREF( spob->lua_land );
   UNREF( spob->lua_can_land );
   UNREF( spob->lua_render );
   UNREF( spob->lua_update );
   UNREF( spob->lua_comm );
   UNREF( spob->lua_mem );
#undef UNREF

   /* Initialize. */
   if (spob->lua_file == NULL)
      return 0;

   /* Try to get the environment, will create a new one as necessary. */
   nlua_env env = spob_lua_get( &mem, spob->lua_file );
   if (env==LUA_NOREF)
      return -1;

   spob->lua_env = env;

   /* Grab functions as applicable. */
   spob->lua_init     = nlua_refenvtype( env, "init",     LUA_TFUNCTION );
   spob->lua_load     = nlua_refenvtype( env, "load",     LUA_TFUNCTION );
   spob->lua_unload   = nlua_refenvtype( env, "unload",   LUA_TFUNCTION );
   spob->lua_can_land = nlua_refenvtype( env, "can_land", LUA_TFUNCTION );
   spob->lua_land     = nlua_refenvtype( env, "land",     LUA_TFUNCTION );
   spob->lua_render   = nlua_refenvtype( env, "render",   LUA_TFUNCTION );
   spob->lua_update   = nlua_refenvtype( env, "update",   LUA_TFUNCTION );
   spob->lua_comm     = nlua_refenvtype( env, "comm",     LUA_TFUNCTION );

   /* Set up local memory. */
   lua_newtable( naevL );        /* m */
   lua_pushvalue( naevL, -1 );   /* m, m */
   spob->lua_mem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* m */

   /* Copy over global memory. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, mem ); /* m, d */
   lua_pushnil( naevL );         /* m, d, nil */
   while (lua_next(naevL,-2) != 0) { /* m, d, k, v */
      lua_pushvalue( naevL, -2 );/* m, d, k, v, k */
      lua_pushvalue( naevL, -2 );/* m, d, k, v, k, v */
      lua_remove( naevL, -3 );   /* m, d, k, k, v */
      lua_settable( naevL, -5 ); /* m, d, k */
   }                             /* m, d */
   lua_pop( naevL, 2 );          /* */

   /* Run init if applicable. */
   if (spob->lua_init != LUA_NOREF) {
      spob_luaInitMem( spob );
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, spob->lua_init); /* f */
      lua_pushspob(naevL, spob_index(spob));
      if (nlua_pcall( spob->lua_env, 1, 0 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), spob->name, "init", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
         return -1;
      }
   }

   return 0;
}

/**
 * @brief Loads a spob's graphics (and radius).
 */
void spob_gfxLoad( Spob *spob )
{
   if (spob->lua_load != LUA_NOREF) {
      spob_luaInitMem( spob );
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, spob->lua_load); /* f */
      if (nlua_pcall( spob->lua_env, 0, 2 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), spob->name, "load", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
         return;
      }
      if (lua_istex(naevL,-2)) {
         if (spob->gfx_space)
            gl_freeTexture( spob->gfx_space );
         spob->gfx_space = gl_dupTexture( lua_totex(naevL,-2) );
      }
      else if (lua_isnil(naevL,-2)) {
         /* Have the engine handle it if nil. */
      }
      else
         WARN(_("Spob '%s' ran '%s' but got non-texture or nil return value!"), spob->name, "load" );
      spob->radius = luaL_optnumber(naevL,-1,-1.);
      lua_pop(naevL,2);
   }

   if (spob->gfx_space==NULL) {
      if (spob->gfx_spaceName != NULL)
         spob->gfx_space = gl_newImage( spob->gfx_spaceName, OPENGL_TEX_MIPMAPS );
   }
   /* Set default size if applicable. */
   if ((spob->gfx_space!=NULL) && (spob->radius < 0.))
      spob->radius = (spob->gfx_space->w + spob->gfx_space->h)/4.;
}

/**
 * @brief Loads all the graphics for a star system.
 *
 *    @param sys System to load graphics for.
 */
void space_gfxLoad( StarSystem *sys )
{
   for (int i=0; i<array_size(sys->spobs); i++)
      spob_gfxLoad( sys->spobs[i] );
}

/**
 * @brief Unloads all the graphics for a star system.
 *
 *    @param sys System to unload graphics for.
 */
void space_gfxUnload( StarSystem *sys )
{
   for (int i=0; i<array_size(sys->spobs); i++) {
      Spob *spob = sys->spobs[i];

      if (spob->lua_unload != LUA_NOREF) {
         spob_luaInitMem( spob );
         lua_rawgeti(naevL, LUA_REGISTRYINDEX, spob->lua_unload); /* f */
         if (nlua_pcall( spob->lua_env, 0, 0 )) {
            WARN(_("Spob '%s' failed to run '%s':\n%s"), spob->name, "unload", lua_tostring(naevL,-1));
            lua_pop(naevL,1);
         }
      }

      gl_freeTexture( spob->gfx_space );
      spob->gfx_space = NULL;
   }
}

/**
 * @brief Parsess an spob presence from xml.
 *
 *    @param node Node to process.
 *    @param[out] ap Spob presence to save to.
 */
static int spob_parsePresence( xmlNodePtr node, SpobPresence *ap )
{
   xmlNodePtr cur = node->children;
   memset( ap, 0, sizeof(SpobPresence) );
   ap->faction = -1;
   do {
      xml_onlyNodes(cur);
      xmlr_float(cur, "base", ap->base);
      xmlr_float(cur, "bonus", ap->bonus);
      xmlr_int(cur, "range", ap->range);
      if (xml_isNode(cur,"faction")) {
         ap->faction = faction_get( xml_get(cur) );
         continue;
      }
   } while (xml_nextNode(cur));
   return 0;
}

/**
 * @brief Parses a spob from an xml node.
 *
 *    @param spob Spob to fill up.
 *    @param filename Name of the file to parse.
 *    @param[in] stdList The array of standard commodities.
 *    @return 0 on success.
 */
static int spob_parse( Spob *spob, const char *filename, Commodity **stdList )
{
   xmlDocPtr doc;
   xmlNodePtr node, parent;
   unsigned int flags;
   Commodity **comms;

   doc = xml_parsePhysFS( filename );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first spob node */
   if (parent == NULL) {
      WARN(_("Malformed %s file: does not contain elements"), filename);
      xmlFreeDoc(doc);
      return -1;
   }

   /* Clear up memory for safe defaults. */
   memset( spob, 0, sizeof(Spob) );
   flags             = 0;
   spob->hide        = 0.01;
   spob->radius      = -1.;
   spob->presence.faction = -1;
   spob->marker_scale = 1.; /* Default scale. */
   comms             = array_create( Commodity* );
   /* Lua stuff. */
   spob->lua_env     = LUA_NOREF;
   spob->lua_init    = LUA_NOREF;
   spob->lua_load    = LUA_NOREF;
   spob->lua_unload  = LUA_NOREF;
   spob->lua_land    = LUA_NOREF;
   spob->lua_can_land= LUA_NOREF;
   spob->lua_render  = LUA_NOREF;
   spob->lua_update  = LUA_NOREF;
   spob->lua_comm    = LUA_NOREF;

   /* Get the name. */
   xmlr_attr_strd( parent, "name", spob->name );

   node = parent->xmlChildrenNode;
   do {
      /* Only handle nodes. */
      xml_onlyNodes(node);

      xmlr_strd(node, "display", spob->display);
      xmlr_strd(node, "feature", spob->feature);
      xmlr_strd(node, "lua", spob->lua_file);
      xmlr_float(node, "radius", spob->radius);
      if (xml_isNode(node, "marker")) {
         const char *s = xml_get(node);
         spob->marker = shaders_getSimple( s );
         xmlr_attr_float_def(node,"scale",spob->marker_scale,1.);
         if (spob->marker == NULL)
            WARN(_("Spob '%s' has unknown marker shader '%s'!"), spob->name, s );
         continue;
      }

      if (xml_isNode(node,"GFX")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"space")) { /* load space gfx */
               char str[PATH_MAX];
               snprintf( str, sizeof(str), SPOB_GFX_SPACE_PATH"%s", xml_get(cur));
               spob->gfx_spaceName = strdup(str);
               spob->gfx_spacePath = xml_getStrd(cur);
               continue;
            }
            if (xml_isNode(cur,"exterior")) { /* load land gfx */
               char str[PATH_MAX];
               snprintf( str, sizeof(str), SPOB_GFX_EXTERIOR_PATH"%s", xml_get(cur));
               spob->gfx_exterior = strdup(str);
               spob->gfx_exteriorPath = xml_getStrd(cur);
               continue;
            }
            if (xml_isNode(cur,"comm")) { /* communication gfx */
               char str[PATH_MAX];
               snprintf( str, sizeof(str), SPOB_GFX_COMM_PATH"%s", xml_get(cur));
               spob->gfx_comm = strdup(str);
               spob->gfx_commPath = xml_getStrd(cur);
               continue;
            }
            WARN(_("Unknown node '%s' in spob '%s'"),node->name,spob->name);
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"pos")) {
         xmlr_attr_float( node, "x", spob->pos.x );
         xmlr_attr_float( node, "y", spob->pos.y );
         flags |= FLAG_POSSET;
         continue;
      }
      else if (xml_isNode(node, "presence")) {
         spob_parsePresence( node, &spob->presence );
         if (spob->presence.faction>=0)
            flags |= FLAG_FACTIONSET;
         continue;
      }
      else if (xml_isNode(node,"general")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            /* Direct reads. */
            xmlr_strd(cur, "class", spob->class);
            xmlr_strd(cur, "bar", spob->bar_description);
            xmlr_strd(cur, "description", spob->description );
            xmlr_float(cur, "population", spob->population );
            xmlr_float(cur, "hide", spob->hide );

            if (xml_isNode(cur, "services")) {
               xmlNodePtr ccur = cur->children;
               flags |= FLAG_SERVICESSET;
               spob->services = 0;
               do {
                  xml_onlyNodes(ccur);

                  if (xml_isNode(ccur, "land"))
                     spob->services |= SPOB_SERVICE_LAND;
                  else if (xml_isNode(ccur, "refuel"))
                     spob->services |= SPOB_SERVICE_REFUEL | SPOB_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "bar"))
                     spob->services |= SPOB_SERVICE_BAR | SPOB_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "missions"))
                     spob->services |= SPOB_SERVICE_MISSIONS | SPOB_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "commodity"))
                     spob->services |= SPOB_SERVICE_COMMODITY | SPOB_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "outfits"))
                     spob->services |= SPOB_SERVICE_OUTFITS | SPOB_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "shipyard"))
                     spob->services |= SPOB_SERVICE_SHIPYARD | SPOB_SERVICE_INHABITED;
                  else if (xml_isNode(ccur, "nomissionspawn"))
                     spob->flags |= SPOB_NOMISNSPAWN;
                  else if (xml_isNode(ccur, "uninhabited"))
                     spob->flags |= SPOB_UNINHABITED;
                  else if (xml_isNode(ccur, "blackmarket"))
                     spob->services |= SPOB_SERVICE_BLACKMARKET;
                  else if (xml_isNode(ccur, "nolanes"))
                     spob->flags |= SPOB_NOLANES;
                  else
                     WARN(_("Spob '%s' has unknown services tag '%s'"), spob->name, ccur->name);
               } while (xml_nextNode(ccur));
            }

            else if (xml_isNode(cur, "commodities")) {
               xmlNodePtr ccur = cur->children;
               do {
                  if (xml_isNode(ccur,"commodity")) {
                     /* If the commodity is standard, don't re-add it. */
                     Commodity *com = commodity_get( xml_get(ccur) );
                     if (commodity_isFlag(com, COMMODITY_FLAG_STANDARD))
                        continue;

                     array_push_back( &comms, com );
                  }
               } while (xml_nextNode(ccur));
            }
            else if (xml_isNode(cur, "blackmarket")) {
               spob_addService(spob, SPOB_SERVICE_BLACKMARKET);
               continue;
            }
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node, "tech")) {
         spob->tech = tech_groupCreateXML( node );
         continue;
      }
      else if (xml_isNode(node, "tags")) {
         xmlNodePtr cur = node->children;
         if (spob->tags != NULL)
            WARN(_("Spob '%s' has duplicate '%s' node!"), spob->name, "tags");
         else
            spob->tags = array_create( char* );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "tag")) {
               char *tmp = xml_get(cur);
               if (tmp != NULL)
                  array_push_back( &spob->tags, strdup(tmp) );
               continue;
            }
            WARN(_("Spob '%s' has unknown node in tags '%s'."), spob->name, cur->name );
         } while (xml_nextNode(cur));
         continue;
      }
      WARN(_("Unknown node '%s' in spob '%s'"),node->name,spob->name);
   } while (xml_nextNode(node));

   /* Allow forcing to be uninhabited. */
   if (spob_isFlag(spob, SPOB_UNINHABITED))
      spob->services &= ~SPOB_SERVICE_INHABITED;

   if (spob->radius > 0.)
      spob_setFlag(spob, SPOB_RADIUS);

   /* Set defaults if not set. */
   if (spob->lua_file == NULL) {
      const char *str = start_spob_lua_default();
      if (str != NULL)
         spob->lua_file = strdup( str );
   }

/*
 * Verification
 */
#define MELEMENT(o,s)   if (o) WARN(_("Spob '%s' missing '%s' element"), spob->name, s)
   //MELEMENT(spob->gfx_spaceName==NULL,"GFX space");
   MELEMENT( spob_hasService(spob,SPOB_SERVICE_LAND) &&
         spob->gfx_exterior==NULL,"GFX exterior");
   MELEMENT( spob_hasService(spob,SPOB_SERVICE_INHABITED) &&
         (spob->population==0), "population");
   MELEMENT((flags&FLAG_POSSET)==0,"pos");
   MELEMENT(spob->class==NULL,"class");
   MELEMENT( spob_hasService(spob,SPOB_SERVICE_LAND) &&
         spob->description==NULL,"description");
   MELEMENT( spob_hasService(spob,SPOB_SERVICE_BAR) &&
         spob->bar_description==NULL,"bar");
   MELEMENT( spob_hasService(spob,SPOB_SERVICE_INHABITED) &&
         (flags&FLAG_FACTIONSET)==0,"faction");
   MELEMENT((flags&FLAG_SERVICESSET)==0,"services");
   MELEMENT( spob_hasService(spob,SPOB_SERVICE_INHABITED) && (spob_hasService(spob,SPOB_SERVICE_OUTFITS) ||
            spob_hasService(spob,SPOB_SERVICE_SHIPYARD)) &&
         (spob->tech==NULL), "tech" );
   /*MELEMENT( spob_hasService(spob,SPOB_SERVICE_COMMODITY) &&
         (array_size(spob->commodities)==0),"commodity" );*/
   /*MELEMENT( (flags&FLAG_FACTIONSET) && (spob->presenceAmount == 0.),
         "presence" );*/
#undef MELEMENT

   /* Build commodities list */
   if (spob_hasService(spob, SPOB_SERVICE_COMMODITY)) {
      spob->commodityPrice = array_create( CommodityPrice );
      spob->commodities = array_create( Commodity* );

      /* First, store all the standard commodities and prices. */
      if (array_size(stdList) > 0) {
         for (int i=0; i<array_size(stdList); i++)
            spob_addCommodity( spob, stdList[i] );
      }

      /* Now add extra commodities */
      for (int i=0; i<array_size(comms); i++)
         spob_addCommodity( spob, comms[i] );

      /* Shrink to minimum size. */
      array_shrink( &spob->commodities );
      array_shrink( &spob->commodityPrice );
   }
   /* Free temporary comms list. */
   array_free(comms);

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Adds a spob to a star system.
 *
 *    @param sys Star System to add spob to. (Assumed to belong to systems_stack.)
 *    @param spobname Name of the spob to add.
 *    @return 0 on success.
 */
int system_addSpob( StarSystem *sys, const char *spobname )
{
   Spob *spob;

   if (sys == NULL)
      return -1;

   spob = spob_get( spobname );
   if (spob == NULL)
      return -1;
   array_push_back( &sys->spobs, spob );
   array_push_back( &sys->spobsid, spob->id );

   /* add spob <-> star system to name stack */
   array_push_back( &spobname_stack, spob->name );
   array_push_back( &systemname_stack, sys->name );

   economy_addQueuedUpdate();
   /* This is required to clear the player statistics for this spob */
   economy_clearSingleSpob(spob);

   /* Reload graphics if necessary. */
   if (cur_system != NULL)
      space_gfxLoad( cur_system );

   /* Initialize economy if applicable. */
   if (spob_hasService( spob, SPOB_SERVICE_COMMODITY ))
      economy_initialiseSingleSystem( sys, spob );

   return 0;
}

/**
 * @brief Removes a spob from a star system.
 *
 *    @param sys Star System to remove spob from.
 *    @param spobname Name of the spob to remove.
 *    @return 0 on success.
 */
int system_rmSpob( StarSystem *sys, const char *spobname )
{
   int i, found;
   Spob *spob;

   if (sys == NULL) {
      WARN(_("Unable to remove spob '%s' from NULL system."), spobname);
      return -1;
   }

   /* Try to find spob. */
   spob = spob_get( spobname );
   for (i=0; i<array_size(sys->spobs); i++)
      if (sys->spobs[i] == spob)
         break;

   /* Spob not found. */
   if (i>=array_size(sys->spobs)) {
      WARN(_("Spob '%s' not found in system '%s' for removal."), spobname, sys->name);
      return -1;
   }

   /* Remove spob from system. */
   array_erase( &sys->spobs, &sys->spobs[i], &sys->spobs[i+1] );
   array_erase( &sys->spobsid, &sys->spobsid[i], &sys->spobsid[i+1] );

   /* Remove the presence. */
   space_reconstructPresences(); /* TODO defer this if removing multiple spobs at once. */

   /* Remove from the name stack thingy. */
   found = 0;
   for (i=0; i<array_size(spobname_stack); i++)
      if (strcmp(spobname, spobname_stack[i])==0) {
         array_erase( &spobname_stack, &spobname_stack[i], &spobname_stack[i+1] );
         array_erase( &systemname_stack, &systemname_stack[i], &systemname_stack[i+1] );
         found = 1;
         break;
      }
   if (found == 0)
      WARN(_("Unable to find spob '%s' and system '%s' in spob<->system stack."),
            spobname, sys->name );

   system_setFaction(sys);

   economy_addQueuedUpdate();

   return 0;
}

/**
 * @brief Adds a virtual spob to a system.
 *
 *    @param sys System to add virtual spob to.
 *    @param spobname Name of the virtual spob being added.
 */
int system_addVirtualSpob( StarSystem *sys, const char *spobname )
{
   VirtualSpob *va;

   if (sys == NULL)
      return -1;

   va = virtualspob_get( spobname );
   if (va == NULL)
      return -1;
   array_push_back( &sys->spobs_virtual, va );

   /* Economy is affected by presence. */
   economy_addQueuedUpdate();

   return 0;
}

/**
 * @brief Removes a virtual spob from a system.
 *
 *    @param sys System to remove virtual spob from.
 *    @param spobname Name of the virtual spob being removed.
 */
int system_rmVirtualSpob( StarSystem *sys, const char *spobname )
{
   int i;

   if (sys == NULL) {
      WARN(_("Unable to remove virtual spob '%s' from NULL system."), spobname);
      return -1;
   }

   /* Try to find virtual spob. */
   for (i=0; i<array_size(sys->spobs_virtual); i++)
      if (strcmp(sys->spobs_virtual[i]->name, spobname)==0)
         break;

   /* Virtual spob not found. */
   if (i>=array_size(sys->spobs_virtual)) {
      WARN(_("Virtual spob '%s' not found in system '%s' for removal."), spobname, sys->name);
      return -1;
   }

   /* Remove virtual spob. */
   array_erase( &sys->spobs_virtual, &sys->spobs_virtual[i], &sys->spobs_virtual[i+1] );

   /* Remove the presence. */
   space_reconstructPresences(); /* TODO defer this if removing multiple spobs at once. */
   system_setFaction(sys);

   economy_addQueuedUpdate();

   return 0;
}

/**
 * @brief Adds a jump point to a star system from a diff.
 *
 * Note that economy_execQueued should always be run after this.
 *
 *    @param sys Star System to add jump point to.
 *    @param node Parent node containing jump point information.
 *    @return 0 on success.
 */
int system_addJumpDiff( StarSystem *sys, xmlNodePtr node )
{
   if (system_parseJumpPointDiff(node, sys) <= -1)
      return 0;
   systems_reconstructJumps();
   economy_addQueuedUpdate();

   return 1;
}

/**
 * @brief Removes a jump point from a star system.
 *
 * Note that economy_execQueued should always be run after this.
 *
 *    @param sys Star System to remove jump point from.
 *    @param jumpname Name of the jump point to remove.
 *    @return 0 on success.
 */
int system_rmJump( StarSystem *sys, const char *jumpname )
{
   int i;
   JumpPoint *jump;

   if (sys == NULL) {
      WARN(_("Unable to remove jump point '%s' from NULL system."), jumpname);
      return -1;
   }

   /* Try to find spob. */
   jump = jump_get( jumpname, sys );
   for (i=0; i<array_size(sys->jumps); i++)
      if (&sys->jumps[i] == jump)
         break;

   /* Spob not found. */
   if (i>=array_size(sys->jumps)) {
      WARN(_("Jump point '%s' not found in system '%s' for removal."), jumpname, sys->name);
      return -1;
   }

   /* Remove jump from system. */
   array_erase( &sys->jumps, &sys->jumps[i], &sys->jumps[i+1] );

   /* Refresh presence */
   system_setFaction(sys);

   economy_addQueuedUpdate();

   return 0;
}

/**
 * @brief Initializes a new star system with null memory.
 */
static void system_init( StarSystem *sys )
{
   memset( sys, 0, sizeof(StarSystem) );
   sys->spobs     = array_create( Spob* );
   sys->spobs_virtual = array_create( VirtualSpob* );
   sys->spobsid   = array_create( int );
   sys->jumps     = array_create( JumpPoint );
   sys->asteroids = array_create( AsteroidAnchor );
   sys->astexclude= array_create( AsteroidExclusion );
   sys->faction   = -1;
}

/**
 * @brief Creates a new star system.
 */
StarSystem *system_new (void)
{
   StarSystem *sys;
   int id;

#if DEBUGGING
   if (!systems_loading)
      systemstack_changed = 1;
#else /* DEBUGGING */
   if (!systems_loading)
      WARN(_("Creating new system in non-debugging mode. Things are probably going to break horribly."));
#endif /* DEBUGGING */

   /* Protect current system in case of realloc. */
   id = -1;
   if (cur_system != NULL)
      id = system_index( cur_system );

   /* Grow array. */
   sys = &array_grow( &systems_stack );

   /* Reset cur_system. */
   if (id >= 0)
      cur_system = system_getIndex( id );

   /* Initialize system and id. */
   system_init( sys );
   sys->id = array_size(systems_stack)-1;

   /* Reconstruct the jumps, only truely necessary if the systems realloced. */
   if (!systems_loading)
      systems_reconstructJumps();

   return sys;
}

/**
 * @brief Reconstructs the jumps for a single system.
 */
void system_reconstructJumps (StarSystem *sys)
{
   for (int j=0; j<array_size(sys->jumps); j++) {
      double dx, dy, a;
      JumpPoint *jp  = &sys->jumps[j];
      jp->from       = sys;
      jp->target     = system_getIndex( jp->targetid );
      jp->returnJump = jump_getTarget( sys, jp->target );

      /* Get heading. */
      dx = jp->target->pos.x - sys->pos.x;
      dy = jp->target->pos.y - sys->pos.y;
      a = atan2( dy, dx );
      if (a < 0.)
         a += 2.*M_PI;

      /* Update position if needed.. */
      if (jp->flags & JP_AUTOPOS)
         vec2_pset( &jp->pos, sys->radius, a );

      /* Update jump specific data. */
      gl_getSpriteFromDir( &jp->sx, &jp->sy, jumppoint_gfx, a );
      jp->angle = 2.*M_PI-a;
      jp->cosa  = cos(jp->angle);
      jp->sina  = sin(jp->angle);
   }
}

/**
 * @brief Reconstructs the jumps.
 */
void systems_reconstructJumps (void)
{
   /* So we need to calculate the shortest jump. */
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      system_reconstructJumps(sys);
   }
}

/**
 * @brief Updates the system spob pointers.
 */
void systems_reconstructSpobs (void)
{
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      for (int j=0; j<array_size(sys->spobsid); j++)
         sys->spobs[j] = &spob_stack[ sys->spobsid[j] ];
   }
}

/**
 * @brief Parses a single asteroid field for a system.
 *
 *    @param node Parent node containing asteroid field information.
 *    @param sys System.
 *    @return 0 on success.
 */
static int system_parseAsteroidField( const xmlNodePtr node, StarSystem *sys )
{
   AsteroidAnchor *a;
   xmlNodePtr cur;
   int pos;

   /* Allocate more space. */
   a = &array_grow( &sys->asteroids );
   memset( a, 0, sizeof(AsteroidAnchor) );

   /* Initialize stuff. */
   pos         = 1;
   a->density  = ASTEROID_DEFAULT_DENSITY;
   a->groups   = array_create( AsteroidTypeGroup* );
   a->groupsw  = array_create( double );
   a->radius   = 0.;
   a->maxspeed = ASTEROID_DEFAULT_MAXSPEED;
   a->maxspin  = ASTEROID_DEFAULT_MAXSPIN;
   a->thrust   = ASTEROID_DEFAULT_THRUST;

   /* Parse label if available. */
   xmlr_attr_strd( node, "label", a->label );

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes(cur);

      xmlr_float( cur, "density", a->density );
      xmlr_float( cur, "radius", a->radius );
      xmlr_float( cur, "maxspeed", a->maxspeed );
      xmlr_float( cur, "thrust", a->thrust );

      /* Handle types of asteroids. */
      if (xml_isNode(cur,"group")) {
         double w;
         const char *name = xml_get(cur);
         xmlr_attr_float_def(cur,"weight",w,1.);
         array_push_back( &a->groups, astgroup_getName(name) );
         array_push_back( &a->groupsw, w );
         continue;
      }

      /* Handle position. */
      if (xml_isNode(cur,"pos")) {
         double x, y;
         pos = 1;
         xmlr_attr_float( cur, "x", x );
         xmlr_attr_float( cur, "y", y );

         /* Set position. */
         vec2_cset( &a->pos, x, y );
         continue;
      }

      WARN(_("Asteroid Field in Star System '%s' has unknown node '%s'"), sys->name, node->name);
   } while (xml_nextNode(cur));

   /* Update internals. */
   asteroids_computeInternals( a );

#define MELEMENT(o,s) \
if (o) WARN(_("Asteroid Field in Star System '%s' has missing/invalid '%s' element"), sys->name, s) /**< Define to help check for data errors. */
   MELEMENT(!pos,"pos");
   MELEMENT(a->radius<=0.,"radius");
   MELEMENT(array_size(a->groups)==0,"groups");
#undef MELEMENT

   return 0;
}

/**
 * @brief Parses a single asteroid exclusion zone for a system.
 *
 *    @param node Parent node containing asteroid exclusion information.
 *    @param sys System.
 *    @return 0 on success.
 */
static int system_parseAsteroidExclusion( const xmlNodePtr node, StarSystem *sys )
{
   AsteroidExclusion *a;
   xmlNodePtr cur;
   double x, y;
   int pos;

   /* Allocate more space. */
   a = &array_grow( &sys->astexclude );
   memset( a, 0, sizeof(*a) );

   /* Initialize stuff. */
   pos = 0;

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      xmlr_float( cur, "radius", a->radius );

      /* Handle position. */
      if (xml_isNode(cur,"pos")) {
         pos = 1;
         xmlr_attr_float( cur, "x", x );
         xmlr_attr_float( cur, "y", y );

         /* Set position. */
         vec2_cset( &a->pos, x, y );
         continue;
      }
      WARN(_("Asteroid Exclusion Zone in Star System '%s' has unknown node '%s'"), sys->name, node->name);
   } while (xml_nextNode(cur));

#define MELEMENT(o,s) \
if (o) WARN(_("Asteroid Exclusion Zone in Star System '%s' has missing/invalid '%s' element"), sys->name, s) /**< Define to help check for data errors. */
   MELEMENT(!pos,"pos");
   MELEMENT(a->radius<=0.,"radius");
#undef MELEMENT

   return 0;
}

/**
 * @brief Creates a system from an XML node.
 *
 *    @param sys System to set up.
 *    @param filename Name of the file to parse.
 *    @return 0 on success.
 */
static int system_parse( StarSystem *sys, const char *filename )
{
   xmlNodePtr node, parent;
   xmlDocPtr doc;
   uint32_t flags;

   /* Load the file. */
   doc = xml_parsePhysFS( filename );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first spob node */
   if (parent == NULL) {
      WARN(_("Malformed %s file: does not contain elements"), filename);
      xmlFreeDoc(doc);
      return -1;
   }

   /* Clear memory for safe defaults. */
   system_init( sys );
   flags          = 0;
   sys->presence  = array_create( SystemPresence );
   sys->ownerpresence = 0.;
   sys->nebu_hue  = NEBULA_DEFAULT_HUE;
   sys->spacedust = -1;

   xmlr_attr_strd( parent, "name", sys->name );

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"pos")) {
         flags |= FLAG_POSSET;
         xmlr_attr_float( node, "x", sys->pos.x );
         xmlr_attr_float( node, "y", sys->pos.y );
         continue;
      }
      else if (xml_isNode(node,"general")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            xmlr_strd( cur, "background", sys->background );
            xmlr_strd( cur, "map_shader", sys->map_shader );
            xmlr_strd( cur, "features", sys->features );
            xmlr_int( cur, "spacedust", sys->spacedust );
            if (xml_isNode( cur, "stars" )) { /* Rename to "spacedust" in 0.11.0. TODO remove sometime around 0.13.0. */
               sys->spacedust = xml_getInt(cur);
               WARN(_("System '%s' is using deprecated field 'stars'. Use 'spacedust' instead!"), sys->name);
            }
            xmlr_float( cur, "radius", sys->radius );
            if (xml_isNode(cur,"interference")) {
               flags |= FLAG_INTERFERENCESET;
               sys->interference = xml_getFloat(cur);
               continue;
            }
            if (xml_isNode(cur,"nebula")) {
               xmlr_attr_float( cur, "volatility", sys->nebu_volatility );
               xmlr_attr_float_def( cur, "hue", sys->nebu_hue, NEBULA_DEFAULT_HUE );
               sys->nebu_density = xml_getFloat(cur);
               continue;
            }
            if (xml_isNode(cur,"nolanes")) {
               sys_setFlag( sys, SYSTEM_NOLANES );
               continue;
            }
            DEBUG(_("Unknown node '%s' in star system '%s'"),node->name,sys->name);
         } while (xml_nextNode(cur));
         continue;
      }
      /* Loads all the spobs. */
      else if (xml_isNode(node,"spobs")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"spob")) {
               system_addSpob( sys, xml_get(cur) );
               continue;
            }
            if (xml_isNode(cur,"spob_virtual")) {
               system_addVirtualSpob( sys, xml_get(cur) );
               continue;
            }
            DEBUG(_("Unknown node '%s' in star system '%s'"),node->name,sys->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"asteroids")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"asteroid"))
               system_parseAsteroidField( cur, sys );
            else if (xml_isNode(cur,"exclusion"))
               system_parseAsteroidExclusion( cur, sys );
         } while (xml_nextNode(cur));
      }

      if (xml_isNode(node, "stats")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            ShipStatList *ll = ss_listFromXML( cur );
            if (ll != NULL) {
               ll->next   = sys->stats;
               sys->stats = ll;
               continue;
            }
            WARN(_("System '%s' has unknown stat '%s'."), sys->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node, "tags")) {
         xmlNodePtr cur = node->children;
         sys->tags = array_create( char* );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "tag")) {
               char *tmp = xml_get(cur);
               if (tmp != NULL)
                  array_push_back( &sys->tags, strdup(tmp) );
               continue;
            }
            WARN(_("System '%s' has unknown node in tags '%s'."), sys->name, cur->name );
         } while (xml_nextNode(cur));
         continue;
      }

      /* Avoid warnings. */
      if (xml_isNode(node,"jumps") || xml_isNode(node,"asteroids"))
         continue;

      DEBUG(_("Unknown node '%s' in star system '%s'"),node->name,sys->name);
   } while (xml_nextNode(node));

   ss_sort( &sys->stats );
   array_shrink( &sys->spobs );
   array_shrink( &sys->spobsid );
   array_shrink( &sys->asteroids );
   array_shrink( &sys->astexclude );

   /* Convert hue from 0 to 359 value to 0 to 1 value. */
   sys->nebu_hue /= 360.;

   /* Load the shader. */
   if (sys->map_shader != NULL)
      sys->ms = mapshader_get( sys->map_shader );

#define MELEMENT(o,s)      if (o) WARN(_("Star System '%s' missing '%s' element"), sys->name, s)
   if (sys->name == NULL) WARN(_("Star System '%s' missing 'name' tag"), sys->name);
   MELEMENT((flags&FLAG_POSSET)==0,"pos");
   MELEMENT(sys->spacedust<0,"spacedust");
   MELEMENT(sys->radius==0.,"radius");
   MELEMENT((flags&FLAG_INTERFERENCESET)==0,"inteference");
#undef MELEMENT

   xmlFreeDoc( doc );

   return 0;
}

/**
 * @brief Compares two system presences.
 */
static int sys_cmpSysFaction( const void *a, const void *b )
{
   SystemPresence *spa, *spb;

   spa = (SystemPresence*) a;
   spb = (SystemPresence*) b;

   /* Compare value. */
   if (spa->value < spb->value)
      return +1;
   else if (spa->value > spb->value)
      return -1;

   /* Compare faction id. */
   if (spa->faction < spb->faction)
      return +1;
   else if (spa->faction > spb->faction)
      return -1;

   return 0;
}

/**
 * @brief Sets the system faction based on the spobs it has.
 *
 *    @param sys System to set the faction of.
 */
void system_setFaction( StarSystem *sys )
{
   /* Sort presences in descending order. */
   if (array_size(sys->presence) != 0)
      qsort( sys->presence, array_size(sys->presence), sizeof(SystemPresence), sys_cmpSysFaction );

   sys->faction = -1;
   for (int i=0; i<array_size(sys->presence); i++) {
      for (int j=0; j<array_size(sys->spobs); j++) { /** @todo Handle multiple different factions. */
         Spob *pnt = sys->spobs[j];

         if (pnt->presence.faction != sys->presence[i].faction)
            continue;

         sys->faction = pnt->presence.faction;
         return;
      }
   }
}

/**
 * @brief Parses a single jump point for a system, from unidiff.
 *
 *    @param node Parent node containing jump point information.
 *    @param sys System to which the jump point belongs.
 *    @return 0 on success.
 */
static int system_parseJumpPointDiff( const xmlNodePtr node, StarSystem *sys )
{
   JumpPoint *j;
   char *buf;
   double x, y;
   StarSystem *target;

   /* Get target. */
   xmlr_attr_strd( node, "target", buf );
   if (buf == NULL) {
      WARN(_("JumpPoint node for system '%s' has no target attribute."), sys->name);
      return -1;
   }
   target = system_get(buf);
   if (target == NULL) {
      WARN(_("JumpPoint node for system '%s' has invalid target '%s'."), sys->name, buf );
      free(buf);
      return -1;
   }
   free(buf);

#ifdef DEBUGGING
   for (int i=0; i<array_size(sys->jumps); i++) {
      JumpPoint *jp = &sys->jumps[i];
      if (jp->targetid != target->id)
         continue;

      WARN(_("Star System '%s' has duplicate jump point to '%s'."),
            sys->name, target->name );
      break;
   }
#endif /* DEBUGGING */

   /* Allocate more space. */
   j = &array_grow( &sys->jumps );
   memset( j, 0, sizeof(JumpPoint) );

   /* Handle jump point position. We want both x and y, or we autoposition the jump point. */
   xmlr_attr_float_def( node, "x", x, HUGE_VAL );
   xmlr_attr_float_def( node, "y", y, HUGE_VAL );

   /* Handle jump point type. */
   xmlr_attr_strd( node, "type", buf );
   if (buf == NULL);
   else if (strcmp(buf, "hidden") == 0)
      jp_setFlag(j,JP_HIDDEN);
   else if (strcmp(buf, "exitonly") == 0)
      jp_setFlag(j,JP_EXITONLY);
   free( buf );

   xmlr_attr_float_def( node, "hide", j->hide, HIDE_DEFAULT_JUMP);

   /* Set some stuff. */
   j->target = target;
   j->targetid = j->target->id;
   j->radius = 200.;

   if (x < HUGE_VAL && y < HUGE_VAL)
      vec2_cset( &j->pos, x, y );
   else
      jp_setFlag(j,JP_AUTOPOS);

   return 0;
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
   xmlNodePtr cur;
   double x, y;
   StarSystem *target;
   int pos;

   /* Get target. */
   xmlr_attr_strd( node, "target", buf );
   if (buf == NULL) {
      WARN(_("JumpPoint node for system '%s' has no target attribute."), sys->name);
      return -1;
   }
   target = system_get(buf);
   if (target == NULL) {
      WARN(_("JumpPoint node for system '%s' has invalid target '%s'."), sys->name, buf );
      free(buf);
      return -1;
   }
   free(buf);

#ifdef DEBUGGING
   for (int i=0; i<array_size(sys->jumps); i++) {
      JumpPoint *jp = &sys->jumps[i];
      if (jp->targetid != target->id)
         continue;

      WARN(_("Star System '%s' has duplicate jump point to '%s'."),
            sys->name, target->name );
      break;
   }
#endif /* DEBUGGING */

   /* Allocate more space. */
   j = &array_grow( &sys->jumps );
   memset( j, 0, sizeof(JumpPoint) );

   /* Set some stuff. */
   j->from = sys;
   j->target = target;
   j->targetid = j->target->id;
   j->radius = 200.;

   pos = 0;

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xmlr_float( cur, "radius", j->radius );

      /* Handle position. */
      if (xml_isNode(cur,"pos")) {
         pos = 1;
         xmlr_attr_float( cur, "x", x );
         xmlr_attr_float( cur, "y", y );

         /* Set position. */
         vec2_cset( &j->pos, x, y );
      }
      else if (xml_isNode(cur,"autopos"))
         jp_setFlag(j,JP_AUTOPOS);
      else if (xml_isNode(cur,"hidden"))
         jp_setFlag(j,JP_HIDDEN);
      else if (xml_isNode(cur,"exitonly"))
         jp_setFlag(j,JP_EXITONLY);
      else if (xml_isNode(cur,"nolanes"))
         jp_setFlag(j,JP_NOLANES);
      else if (xml_isNode(cur,"hide")) {
         xmlr_float( cur,"hide", j->hide );
      }
   } while (xml_nextNode(cur));

   if (!jp_isFlag(j,JP_AUTOPOS) && !pos)
      WARN(_("JumpPoint in system '%s' is missing pos element but does not have autopos flag."), sys->name);

   return 0;
}

/**
 * @brief Loads the jumps into a system.
 *
 *    @param sys Star system to load jumps of.
 *    @return 0 on success.
 */
static int system_parseJumps( StarSystem *sys )
{
   xmlNodePtr parent, node;
   xmlDocPtr doc;

   doc = xml_parsePhysFS( sys->filename );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first spob node */
   if (parent == NULL) {
      xmlFreeDoc(doc);
      return -1;
   }

   node  = parent->xmlChildrenNode;
   do { /* load all the data */
      if (xml_isNode(node,"jumps")) {
         xmlNodePtr cur = node->children;
         do {
            if (xml_isNode(cur,"jump"))
               system_parseJumpPoint( cur, sys );
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   array_shrink( &sys->jumps );

   xmlFreeDoc(doc);
   return 0;
}

/**
 * @brief Loads the entire universe into ram - pretty big feat eh?
 *
 *    @return 0 on success.
 */
int space_load (void)
{
   int ret;

   /* Loading. */
   systems_loading = 1;

   /* Create some arrays. */
   spobname_stack = array_create( char* );
   systemname_stack = array_create( char* );

   /* Load jump point graphic - must be before systems_load(). */
   jumppoint_gfx = gl_newSprite(  SPOB_GFX_SPACE_PATH"jumppoint.webp", 4, 4, OPENGL_TEX_MIPMAPS );
   jumpbuoy_gfx = gl_newImage(  SPOB_GFX_SPACE_PATH"jumpbuoy.webp", 0 );

   /* Load spobs. */
   ret = spobs_load();
   if (ret < 0)
      return ret;

   /* Load virtual spobs. */
   ret = virtualspobs_load();
   if (ret < 0)
      return ret;

   /* Load asteroid stuff. */
   ret = asteroids_load ();
   if (ret < 0)
      return ret;

   /* Load systems. */
   ret = systems_load();
   if (ret < 0)
      return ret;

   /* Done loading. */
   systems_loading = 0;

   /* Apply all the presences. */
   for (int i=0; i<array_size(systems_stack); i++)
      system_addAllSpobsPresence(&systems_stack[i]);

   /* Determine dominant faction. */
   for (int i=0; i<array_size(systems_stack); i++)
      system_setFaction( &systems_stack[i] );

   /* Reconstruction. */
   systems_reconstructJumps();
   systems_reconstructSpobs();

   /* Fine tuning. */
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];

      /* Save jump indexes. */
      for (int j=0; j<array_size(sys->jumps); j++)
         sys->jumps[j].targetid = sys->jumps[j].target->id;
      sys->ownerpresence = system_getPresence( sys, sys->faction );
   }

   /* Calculate commodity prices (sinusoidal model). */
   economy_initialiseCommodityPrices();

   return 0;
}

/**
 * @brief initializes the Lua for all the spobs.
 */
int space_loadLua (void)
{
   int ret = 0;
   for (int i=0; i<array_size(spob_stack); i++)
      ret |= spob_luaInit( &spob_stack[i] );
   return ret;
}

/**
 * @brief Loads the entire systems, needs to be called after spobs_load.
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
   char **system_files;
   Uint32 time = SDL_GetTicks();

   /* Allocate if needed. */
   if (systems_stack == NULL)
      systems_stack = array_create( StarSystem );

   system_files = ndata_listRecursive( SYSTEM_DATA_PATH );

   /*
    * First pass - loads all the star systems_stack.
    */
   for (int i=0; i<array_size(system_files); i++) {
      StarSystem sys;

      if (!ndata_matchExt( system_files[i], "xml" ))
         continue;

      int ret = system_parse( &sys, system_files[i] );
      if (ret == 0) {
         sys.filename = system_files[i];
         sys.id = array_size(systems_stack);

         /* Update asteroid info. */
         system_updateAsteroids( &sys );

         array_push_back( &systems_stack, sys );

         /* Render if necessary. */
         naev_renderLoadscreen();
      }
   }
   qsort( systems_stack, array_size(systems_stack), sizeof(StarSystem), system_cmp );
   for (int j=0; j<array_size(systems_stack); j++) {
      systems_stack[j].id = j;
      systems_stack[j].note = NULL; /* just to be sure */
   }

   /*
    * Second pass - loads all the jump routes.
    */
   for (int i=0; i<array_size(systems_stack); i++)
      system_parseJumps( &systems_stack[i] );

   /* Clean up. */
   array_free( system_files );

   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_( "Loaded %d Star System",
                 "Loaded %d Star Systems", array_size(systems_stack) ), array_size(systems_stack) );
      DEBUG( n_( "       with %d Space Object in %.3f s",
                 "       with %d Space Objects in %.3f s", array_size(spob_stack) ), array_size(spob_stack), time/1000. );
   }
   else {
      DEBUG( n_( "Loaded %d Star System",
                 "Loaded %d Star Systems", array_size(systems_stack) ), array_size(systems_stack) );
      DEBUG( n_( "       with %d Space Object",
                 "       with %d Space Objects", array_size(spob_stack) ), array_size(spob_stack) );
   }

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

   /* Render the debris. */
   asteroids_renderOverlay();

   /* Render overlay if necessary. */
   background_renderOverlay( dt );

   if ((cur_system->nebu_density > 0.) &&
         !menu_isOpen( MENU_MAIN ) && !menu_isOpen( MENU_EDITORS ))
      nebu_renderOverlay(dt);
}

/**
 * @brief Renders the current systems' spobs.
 */
void spobs_render (void)
{
   /* Must be a system. */
   if (cur_system==NULL)
      return;

   /* Render the jumps. */
   for (int i=0; i < array_size(cur_system->jumps); i++)
      space_renderJumpPoint( &cur_system->jumps[i], i );

   /* Render the spobs. */
   for (int i=0; i < array_size(cur_system->spobs); i++)
      space_renderSpob( cur_system->spobs[i] );

   /* Render the asteroids & debris. */
   asteroids_render();

   /* Render gatherable stuff. */
   gatherable_render();

}

/**
 * @brief Renders a jump point.
 */
static void space_renderJumpPoint( const JumpPoint *jp, int i )
{
   const glColour *c;

   if (!jp_isUsable(jp))
      return;

   if ((player.p != NULL) && (i==player.p->nav_hyperspace) &&
         (pilot_isFlag(player.p, PILOT_HYPERSPACE) || space_canHyperspace(player.p)))
      c = &cGreen;
   else if (jp_isFlag(jp, JP_HIDDEN))
      c = &cRed;
   else
      c = NULL;

   gl_renderSprite( jumppoint_gfx, jp->pos.x, jp->pos.y, jp->sx, jp->sy, c );

   /* Draw buoys next to "highway" jump points. */
   if (jp->hide == 0.) {
      gl_renderSprite( jumpbuoy_gfx, jp->pos.x + 200 * jp->sina, jp->pos.y + 200 * jp->cosa, 0, 0, NULL ); /* Left */
      gl_renderSprite( jumpbuoy_gfx, jp->pos.x + -200 * jp->sina, jp->pos.y + -200 * jp->cosa, 0, 0, NULL ); /* Right */
   }
}

/**
 * @brief Renders a spob.
 */
static void space_renderSpob( const Spob *p )
{
   if (p->lua_render != LUA_NOREF) {
      spob_luaInitMem( p );
      /* TODO do a clip test first. */
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, p->lua_render); /* f */
      if (nlua_pcall( p->lua_env, 0, 0 )) {
         WARN(_("Spob '%s' failed to run '%s':\n%s"), p->name, "render", lua_tostring(naevL,-1));
         lua_pop(naevL,1);
      }
   }
   else if (p->gfx_space)
      gl_renderSprite( p->gfx_space, p->pos.x, p->pos.y, 0, 0, NULL );
}

/**
 * @brief Renders a spob.
 */
static void space_updateSpob( const Spob *p, double dt, double real_dt )
{
   if (p->lua_update == LUA_NOREF)
      return;
   /* TODO do a clip test first. */
   spob_luaInitMem( p );
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, p->lua_update); /* f */
   lua_pushnumber(naevL, dt); /* f, dt */
   lua_pushnumber(naevL, real_dt); /* f, real_dt */
   if (nlua_pcall( p->lua_env, 2, 0 )) {
      WARN(_("Spob '%s' failed to run '%s':\n%s"), p->name, "update", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }
}

/**
 * @brief Cleans up the system.
 */
void space_exit (void)
{
   /* Free standalone graphic textures */
   gl_freeTexture(jumppoint_gfx);
   jumppoint_gfx = NULL;
   gl_freeTexture(jumpbuoy_gfx);
   jumpbuoy_gfx = NULL;

   /* Free the names. */
   array_free(spobname_stack);
   array_free(systemname_stack);

   /* Free the spobs. */
   for (int i=0; i < array_size(spob_stack); i++) {
      Spob *spb = &spob_stack[i];

      free(spb->name);
      free(spb->display);
      free(spb->feature);
      free(spb->lua_file);
      free(spb->class);
      free(spb->description);
      free(spb->bar_description);
      for (int j=0; j<array_size(spb->tags); j++)
         free( spb->tags[j] );
      array_free(spb->tags);

      /* graphics */
      gl_freeTexture( spb->gfx_space );
      free(spb->gfx_spaceName);
      free(spb->gfx_spacePath);
      free(spb->gfx_exterior);
      free(spb->gfx_exteriorPath);
      free(spb->gfx_comm);
      free(spb->gfx_commPath);

      /* Landing. */
      free(spb->land_msg);

      /* tech */
      if (spb->tech != NULL)
         tech_groupDestroy( spb->tech );

      /* commodities */
      array_free(spb->commodities);
      array_free(spb->commodityPrice);

      /* Lua. */
      nlua_freeEnv( spb->lua_env );
   }
   array_free(spob_stack);

   for (int i=0; i<array_size(spob_lua_stack); i++)
      spob_lua_free( &spob_lua_stack[i] );
   array_free(spob_lua_stack);

   for (int i=0; i<array_size(vspob_stack); i++) {
      VirtualSpob *va = &vspob_stack[i];
      free( va->name );
      array_free( va->presences );
   }
   array_free( vspob_stack );

   /* Free the systems. */
   for (int i=0; i < array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];

      free(sys->filename);
      free(sys->name);
      free(sys->background);
      free(sys->map_shader);
      free(sys->features);
      free(sys->note);
      array_free(sys->jumps);
      array_free(sys->presence);
      array_free(sys->spobs);
      array_free(sys->spobsid);
      array_free(sys->spobs_virtual);

      for (int j=0; j<array_size(sys->tags); j++)
         free( sys->tags[j] );
      array_free(sys->tags);

      /* Free the asteroids. */
      for (int j=0; j < array_size(sys->asteroids); j++)
         asteroid_free( &sys->asteroids[j] );
      array_free(sys->asteroids);
      array_free(sys->astexclude);

      ss_free( sys->stats );
   }
   array_free(systems_stack);
   systems_stack = NULL;

   /* Free asteroids stuff. */
   asteroids_free();

   /* Free the gatherable stack. */
   gatherable_free();

   /* Free the map shaders. */
   for (int i=0; i<array_size(mapshaders); i++) {
      MapShader *ms = mapshaders[i];
      free( ms->name );
      glDeleteProgram( ms->program );
      free(ms);
   }
   array_free( mapshaders );
}

/**
 * @brief Clears all system knowledge.
 */
void space_clearKnown (void)
{
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      sys_rmFlag(sys,SYSTEM_KNOWN);
      sys_rmFlag(sys,SYSTEM_HIDDEN);
      sys_rmFlag(sys,SYSTEM_PMARKED);
      for (int j=0; j<array_size(sys->jumps); j++)
         jp_rmFlag(&sys->jumps[j],JP_KNOWN);
      free(sys->note);
      sys->note=NULL;
   }
   for (int j=0; j<array_size(spob_stack); j++)
      spob_rmFlag(&spob_stack[j],SPOB_KNOWN);
}

/**
 * @brief Clears all system markers.
 */
void space_clearMarkers (void)
{
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      sys_rmFlag( sys, SYSTEM_MARKED );
      sys->markers_computer = 0;
      sys->markers_plot  = 0;
      sys->markers_high  = 0;
      sys->markers_low   = 0;
   }
   for (int i=0; i<array_size(spob_stack); i++) {
      Spob *pnt = &spob_stack[i];
      spob_rmFlag( pnt, SPOB_MARKED );
      pnt->markers = 0;
   }
}

/**
 * @brief Clears all the system computer markers.
 */
void space_clearComputerMarkers (void)
{
   for (int i=0; i<array_size(systems_stack); i++)
      sys_rmFlag(&systems_stack[i],SYSTEM_CMARKED);
}

static int space_addMarkerSystem( int sysid, MissionMarkerType type )
{
   StarSystem *ssys;
   int *markers;

   /* Get the system. */
   ssys = system_getIndex(sysid);
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
         WARN(_("Unknown marker type."));
         return -1;
   }

   /* Decrement markers. */
   (*markers)++;
   sys_setFlag(ssys, SYSTEM_MARKED);

   return 0;
}

static int space_addMarkerSpob( int pntid, MissionMarkerType type )
{
   const char *sys;
   MissionMarkerType stype;
   Spob *pnt = spob_getIndex( pntid );
   if (pnt==NULL)
      return -1;

   /* Mark spob. */
   pnt->markers++;
   spob_setFlag( pnt, SPOB_MARKED );

   /* Now try to mark system. */
   sys = spob_getSystem( pnt->name );
   if (sys == NULL) {
      WARN(_("Marking spob '%s' that is not in any system!"), pnt->name);
      return 0;
   }
   stype = mission_markerTypeSpobToSystem( type );
   return space_addMarkerSystem( system_index( system_get(sys) ), stype );
}

/**
 * @brief Adds a marker to a system.
 *
 *    @param objid ID of the object to add marker to.
 *    @param type Type of the marker to add.
 *    @return 0 on success.
 */
int space_addMarker( int objid, MissionMarkerType type )
{
   switch (type) {
      case SYSMARKER_COMPUTER:
      case SYSMARKER_LOW:
      case SYSMARKER_HIGH:
      case SYSMARKER_PLOT:
         return space_addMarkerSystem( objid, type );
      case SPOBMARKER_COMPUTER:
      case SPOBMARKER_LOW:
      case SPOBMARKER_HIGH:
      case SPOBMARKER_PLOT:
         return space_addMarkerSpob( objid, type );
      default:
         WARN(_("Unknown marker type."));
         return -1;
   }
   return 0;
}

static int space_rmMarkerSystem( int sys, MissionMarkerType type )
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
         WARN(_("Unknown marker type."));
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

static int space_rmMarkerSpob( int pntid, MissionMarkerType type )
{
   (void) type;
   const char *sys;
   MissionMarkerType stype;
   Spob *pnt = spob_getIndex( pntid );

   /* Remove spob marker. */
   pnt->markers--;
   if (pnt->markers <= 0)
      spob_rmFlag( pnt, SPOB_MARKED );

   /* Now try to remove system. */
   sys = spob_getSystem( pnt->name );
   if (sys == NULL)
      return 0;
   stype = mission_markerTypeSpobToSystem( type );
   return space_rmMarkerSystem( system_index( system_get(sys) ), stype );
}

/**
 * @brief Removes a marker from a system.
 *
 *    @param objid ID of the object to remove marker from.
 *    @param type Type of the marker to remove.
 *    @return 0 on success.
 */
int space_rmMarker( int objid, MissionMarkerType type )
{
   switch (type) {
      case SYSMARKER_COMPUTER:
      case SYSMARKER_LOW:
      case SYSMARKER_HIGH:
      case SYSMARKER_PLOT:
         return space_rmMarkerSystem( objid, type );
      case SPOBMARKER_COMPUTER:
      case SPOBMARKER_LOW:
      case SPOBMARKER_HIGH:
      case SPOBMARKER_PLOT:
         return space_rmMarkerSpob( objid, type );
      default:
         WARN(_("Unknown marker type."));
         return -1;
   }
}

/**
 * @brief Saves what is needed to be saved for space.
 *
 *    @param writer XML writer to use.
 *    @return 0 on success.
 */
int space_sysSave( xmlTextWriterPtr writer )
{
   xmlw_startElem(writer,"space");
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];

      if (!sys_isKnown(sys))
         continue; /* not known */

      xmlw_startElem(writer,"known");
      xmlw_attr(writer,"sys","%s",sys->name);
      if (sys_isFlag(sys, SYSTEM_PMARKED))
         xmlw_attr(writer,"pmarked","%s","true");
      if (sys->note != NULL)
         xmlw_attr(writer,"note","%s",sys->note);
      for (int j=0; j<array_size(sys->spobs); j++) {
         if (!spob_isKnown(sys->spobs[j]))
            continue; /* not known */
         xmlw_elem(writer, "spob", "%s", sys->spobs[j]->name);
      }

      for (int j=0; j<array_size(sys->jumps); j++) {
         if (!jp_isKnown(&sys->jumps[j]))
            continue; /* not known */
         xmlw_elem(writer,"jump","%s",(&sys->jumps[j])->target->name);
      }

      xmlw_endElem(writer);
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
   xmlNodePtr node;

   space_clearKnown();

   node = parent->xmlChildrenNode;
   do {
      xmlNodePtr cur;

      xml_onlyNodes(node);
      if (!xml_isNode(node,"space"))
         continue;

      cur = node->xmlChildrenNode;
      do {
         char *str;
         StarSystem *sys;

         xml_onlyNodes( cur );
         if (!xml_isNode(cur,"known"))
            continue;

         xmlr_attr_strd(cur,"sys",str);
         if (str != NULL) { /* check for 0.5.0 saves */
            sys = system_get(str);
            free(str);
         }
         else /* load from 0.5.0 saves */
            sys = system_get(xml_get(cur));

         if (sys != NULL) { /* Must exist */
            sys_setFlag(sys,SYSTEM_KNOWN);

            xmlr_attr_strd(cur,"pmarked",str);
            if (str != NULL) {
               sys_setFlag(sys,SYSTEM_PMARKED);
               free(str);
            }

            xmlr_attr_strd(cur,"note",str);
            if (str != NULL) {
               xmlr_attr_strd(cur,"note",sys->note);
               free(str);
            }
            space_parseSpobs(cur, sys);
         }
      } while (xml_nextNode(cur));
   } while (xml_nextNode(node));

   return 0;
}

/**
 * @brief Parses spobs in a system.
 *
 *    @param parent Node of the system.
 *    @param sys System to populate.
 *    @return 0 on success.
 */
static int space_parseSpobs( xmlNodePtr parent, StarSystem* sys )
{
   xmlNodePtr node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"spob")) {
         Spob *spob = spob_get(xml_get(node));
         if (spob != NULL) /* Must exist */
            spob_setKnown(spob);
      }
      else if (xml_isNode(node,"jump")) {
         JumpPoint *jp = jump_get(xml_get(node), sys);
         if (jp != NULL) /* Must exist */
            jp_setFlag(jp,JP_KNOWN);
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
   int n;

   /* Check for NULL and display a warning. */
   if (sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }

   /* Go through the array (if created), looking for the faction. */
   for (int i=0; i < array_size(sys->presence); i++)
      if (sys->presence[i].faction == faction)
         return i;

   /* Grow the array. */
   n = array_size(sys->presence);
   memset(&array_grow(&sys->presence), 0, sizeof(SystemPresence));
   sys->presence[n].faction = faction;

   return n;
}

/**
 * @brief Adds (or removes) some presence to a system.
 *
 *    @param sys Pointer to the system to add to or remove from.
 *    @param ap Spob presence to add.
 */
void system_presenceAddSpob( StarSystem *sys, const SpobPresence *ap )
{
   int id, curSpill;
   Queue q, qn;
   double spillfactor;
   int faction = ap->faction;
   double base = ap->base;
   double bonus = ap->bonus;
   double range = ap->range;
   int usehidden = faction_usesHiddenJumps( faction );
   const FactionGenerator *fgens;

   /* Check for NULL and display a warning. */
   if (sys == NULL) {
      WARN("sys == NULL");
      return;
   }

   /* Check that we have a valid faction. */
   if (faction_isFaction(faction) == 0)
      return;

   /* Check that we're actually adding any. */
   if ((base == 0.) && (bonus == 0.))
      return;

   /* Get secondary if applicable. */
   fgens = faction_generators( faction );

   /* Add the presence to the current system. */
   id = getPresenceIndex(sys, faction);
   sys->presence[id].base   = MAX( sys->presence[id].base, base );
   sys->presence[id].bonus += bonus;
   sys->presence[id].value  = sys->presence[id].base + sys->presence[id].bonus;
   for (int i=0; i<array_size(fgens); i++) {
      int x = getPresenceIndex(sys, fgens[i].id);
      sys->presence[x].base   = MAX( sys->presence[x].base, MAX(0., base*fgens[i].weight) );
      sys->presence[x].bonus += MAX(0., bonus*fgens[i].weight);
      sys->presence[x].value  = sys->presence[x].base + sys->presence[x].bonus;
   }

   /* If there's no range, we're done here. */
   if (range < 1)
      return;

   /* Add the spill. */
   sys->spilled   = 1;
   curSpill       = 0;
   q              = q_create();
   qn             = q_create();

   /* Create the initial queue consisting of sys adjacencies. */
   for (int i=0; i < array_size(sys->jumps); i++) {
      if (sys->jumps[i].target->spilled == 0 && (usehidden || !jp_isFlag( &sys->jumps[i], JP_HIDDEN )) && !jp_isFlag( &sys->jumps[i], JP_EXITONLY )) {
         q_enqueue( q, sys->jumps[i].target );
         sys->jumps[i].target->spilled = 1;
      }
   }

   /* If it's empty, something's wrong. */
   if (q_isEmpty(q)) {
      /* Means system isn't connected. */
      /*WARN(_("q is empty after getting adjacencies of %s."), sys->name);*/
      q_destroy(q);
      q_destroy(qn);
      goto sys_cleanup;
      return;
   }

   while (curSpill < range) {
      int x;

      /* Pull one off the current range queue. */
      StarSystem *cur = q_dequeue(q);

      /* Ran out of candidates before running out of spill range! */
      if (cur == NULL)
         break;

      /* Enqueue all its adjacencies to the next range queue. */
      for (int i=0; i<array_size(cur->jumps); i++) {
         if (cur->jumps[i].target->spilled == 0 && (usehidden || !jp_isFlag( &cur->jumps[i], JP_HIDDEN )) && !jp_isFlag( &cur->jumps[i], JP_EXITONLY )) {
            q_enqueue( qn, cur->jumps[i].target );
            cur->jumps[i].target->spilled = 1;
         }
      }

      /* Spill some presence. */
      x = getPresenceIndex(cur, faction);
      spillfactor = 1. / (2. + (double)curSpill);
      cur->presence[x].base   = MAX( cur->presence[x].base, base * spillfactor );
      cur->presence[x].bonus += bonus * spillfactor;
      cur->presence[x].value  = cur->presence[x].base + cur->presence[x].bonus;

      for (int i=0; i<array_size(fgens); i++) {
         int y = getPresenceIndex(cur, fgens[i].id);
         cur->presence[y].base   = MAX( cur->presence[y].base, MAX(0., base*spillfactor*fgens[i].weight) );
         cur->presence[y].bonus += MAX(0., bonus*spillfactor*fgens[i].weight );
         cur->presence[y].value  = cur->presence[y].base + cur->presence[y].bonus;
      }

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

sys_cleanup:
   /* Clean up our mess. */
   for (int i=0; i < array_size(systems_stack); i++)
      systems_stack[i].spilled = 0;
   return;
}

/**
 * @brief Get the presence of a faction in a system.
 *
 *    @param sys Pointer to the system to process.
 *    @param faction The faction to get the presence for.
 *    @return The amount of presence the faction has in the system.
 */
double system_getPresence( const StarSystem *sys, int faction )
{
   /* Check for NULL and display a warning. */
#if DEBUGGING
   if (sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }
#endif /* DEBUGGING */

   /* Go through the array, looking for the faction. */
   for (int i=0; i < array_size(sys->presence); i++) {
      if (sys->presence[i].faction == faction)
         return MAX(sys->presence[i].value, 0);
   }

   /* If it's not in there, it's zero. */
   return 0.;
}

/**
 * @brief Get the presence of a faction in a system.
 *
 *    @param sys Pointer to the system to process.
 *    @param faction The faction to get the presence for.
 *    @param[out] base Base value of the presence.
 *    @param[out] bonus Bonus value of the presence.
 *    @return The amount of presence the faction has in the system.
 */
double system_getPresenceFull( const StarSystem *sys, int faction, double *base, double *bonus )
{
   /* Check for NULL and display a warning. */
#if DEBUGGING
   if (sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }
#endif /* DEBUGGING */

   /* Go through the array, looking for the faction. */
   for (int i=0; i < array_size(sys->presence); i++) {
      if (sys->presence[i].faction == faction) {
         *base = sys->presence[i].base;
         *bonus = sys->presence[i].bonus;
         return MAX(sys->presence[i].value, 0);
      }
   }

   /* If it's not in there, it's zero. */
   *base = 0.;
   *bonus = 0.;
   return 0.;
}

/**
 * @brief Go through all the spobs and call system_addPresence().
 *
 *    @param sys Pointer to the system to process.
 */
void system_addAllSpobsPresence( StarSystem *sys )
{
   /* Check for NULL and display a warning. */
#if DEBUGGING
   if (sys == NULL) {
      WARN("sys == NULL");
      return;
   }
#endif /* DEBUGGING */

   /* Real spobs. */
   for (int i=0; i<array_size(sys->spobs); i++)
      system_presenceAddSpob(sys, &sys->spobs[i]->presence );

   /* Virtual spobs. */
   for (int i=0; i<array_size(sys->spobs_virtual); i++)
      for (int j=0; j<array_size(sys->spobs_virtual[i]->presences); j++)
         system_presenceAddSpob(sys, &sys->spobs_virtual[i]->presences[j] );
}

/**
 * @brief Reset the presence of all systems.
 */
void space_reconstructPresences( void )
{
   /* Reset the presence in each system. */
   for (int i=0; i<array_size(systems_stack); i++) {
      array_free(systems_stack[i].presence);
      systems_stack[i].presence  = array_create( SystemPresence );
      systems_stack[i].ownerpresence = 0.;
   }

   /* Re-add presence to each system. */
   for (int i=0; i<array_size(systems_stack); i++)
      system_addAllSpobsPresence(&systems_stack[i]);

   /* Determine dominant faction. */
   for (int i=0; i<array_size(systems_stack); i++) {
      system_setFaction( &systems_stack[i] );
      systems_stack[i].ownerpresence = system_getPresence( &systems_stack[i], systems_stack[i].faction );
   }

   /* Have to redo the scheduler because everything changed. */
   /* TODO this actually ignores existing presence and will temporarily increase system presence more than normal... */
   if (cur_system != NULL)
      system_scheduler( 0., 1 );
}

/**
 * @brief See if the system has a spob.
 *
 *    @param sys Pointer to the system to process.
 *    @return 0 If empty; otherwise 1.
 */
int system_hasSpob( const StarSystem *sys )
{
   /* Check for NULL and display a warning. */
   if (sys == NULL) {
      WARN("sys == NULL");
      return 0;
   }

   /* Go through all the spobs and look for a real one. */
   for (int i=0; i < array_size(sys->spobs); i++)
      return 1;

   return 0;
}

/**
 * @brief Removes active presence.
 */
void system_rmCurrentPresence( StarSystem *sys, int faction, double amount )
{
   int id;
   nlua_env env;
   SystemPresence *presence;

   /* Remove the presence. */
   id = getPresenceIndex( cur_system, faction );
   sys->presence[id].curUsed -= amount;

   /* Safety. */
   presence = &sys->presence[id];
   presence->curUsed = MAX( 0, sys->presence[id].curUsed );

   /* Run lower hook. */
   env = faction_getScheduler( faction );

   /* Run decrease function if applicable. */
   nlua_getenv( naevL, env, "decrease" ); /* f */
   if (lua_isnil(naevL,-1)) {
      lua_pop(naevL,1);
      return;
   }
   lua_pushnumber( naevL, presence->curUsed ); /* f, cur */
   lua_pushnumber( naevL, presence->value );   /* f, cur, max */
   lua_pushnumber( naevL, presence->timer );   /* f, cur, max, timer */

   /* Actually run the function. */
   if (nlua_pcall(env, 3, 1)) { /* error has occurred */
      WARN(_("Lua decrease script for faction '%s' : %s"),
            faction_name( faction ), lua_tostring(naevL,-1));
      lua_pop(naevL,1);
      return;
   }

   /* Output is handled the same way. */
   if (!lua_isnumber(naevL,-1)) {
      WARN(_("Lua spawn script for faction '%s' failed to return timer value."),
            faction_name( presence->faction ) );
      lua_pop(naevL,1);
      return;
   }
   presence->timer = lua_tonumber(naevL,-1);
   lua_pop(naevL,1);
}

/**
 * @brief Cues a spob to be landed on. This is not done immediately, but when
 * the engine thinks it is ok to do.
 *
 *    @param pnt Spob to land on.
 */
void space_queueLand( Spob *pnt )
{
   space_landQueueSpob = pnt;
}

/**
 * @brief Gets the population in an approximated string. Note this function changes the string value each call, so be careful!
 *
 *    @param population Population to get string of.
 *    @return String corresponding to the population.
 */
const char *space_populationStr( uint64_t population )
{
   static char pop[STRMAX_SHORT];
   double p = (double)population;

   /* Out of respect for the first version of this, do something fancy and human-oriented.
    * However, specifying a thousand/million/billion system failed in a few ways: needing 2x as many cases as
    * intended to avoid silliness (1.0e10 -> 10000 million), and not being gettext-translatable to other number
    * systems like the Japanese one. */

   if (p < 1.0e3)
      snprintf( pop, sizeof(pop), "%.0f", p );
   else {
      char scratch[STRMAX_SHORT];
      const char *digits[] = {"\xe2\x81\xb0", "\xc2\xb9", "\xc2\xb2", "\xc2\xb3", "\xe2\x81\xb4", "\xe2\x81\xb5", "\xe2\x81\xb6", "\xe2\x81\xb7", "\xe2\x81\xb8", "\xe2\x81\xb9"};
      int state = 0,  COEF = 0, E = 1, EXP = 4;
      size_t l = 0;
      snprintf( scratch, sizeof(scratch), "%.1e", p );
      for (const char *c = scratch; *c; c++) {
         if (state == COEF && *c != 'e')
            l += scnprintf( &pop[l], sizeof(pop)-l, "%c", *c );
         else if (state == COEF ) {
            l += scnprintf( &pop[l], sizeof(pop)-l, "%s", "\xc2\xb7" "10" );
            state = E;
         }
         else if (state == E && (*c == '+' || *c == '0'))
            state = E;
         else {
            state = EXP;
            l += scnprintf( &pop[l], sizeof(pop)-l, "%s", digits[*c-'0'] );
         }
      }
   }

   return pop;
}

/**
 * @brief Gets the map shader by name.
 *
 *    @param name NAme of the map shader.
 *    @return The map shader.
 */
static const MapShader *mapshader_get( const char *name )
{
   MapShader *ms;

   if (mapshaders==NULL)
      mapshaders = array_create( MapShader* );

   for (int i=0; i<array_size(mapshaders); i++) {
      MapShader *t = mapshaders[i];
      if (strcmp(t->name,name)==0)
         return t;
   }

   /* Allocate and set up. */
   ms = malloc( sizeof(MapShader) );
   array_push_back( &mapshaders, ms );

   ms->name      = strdup( name );
   ms->program   = gl_program_vert_frag( "system_map.vert", name, NULL );
   ms->vertex    = glGetAttribLocation(  ms->program,  "vertex" );
   ms->projection= glGetUniformLocation( ms->program, "projection" );
   ms->time      = glGetUniformLocation( ms->program, "time" );
   ms->globalpos = glGetUniformLocation( ms->program, "globalpos" );
   ms->alpha     = glGetUniformLocation( ms->program, "alpha" );

   return ms;
}

static int spob_lua_cmp( const void *a, const void *b )
{
   const spob_lua_file *la = (const spob_lua_file*) a;
   const spob_lua_file *lb = (const spob_lua_file*) b;
   return strcmp( la->filename, lb->filename );
}

static nlua_env spob_lua_get( int *mem, const char *filename )
{
   size_t sz;
   char *dat;
   spob_lua_file *lf;
   const spob_lua_file key = { .filename=filename };

   if (spob_lua_stack == NULL)
      spob_lua_stack = array_create( spob_lua_file );

   lf = bsearch( &key, spob_lua_stack, array_size(spob_lua_stack), sizeof(spob_lua_file), spob_lua_cmp );
   if (lf != NULL) {
      *mem = lf->lua_mem;
      return lf->env;
   }

   dat = ndata_read( filename, &sz );
   if (dat==NULL) {
      WARN(_("Failed to read spob Lua '%s'!"), filename );
      return LUA_NOREF;
   }

   nlua_env env = nlua_newEnv();
   nlua_loadStandard( env );
   nlua_loadGFX( env );
   nlua_loadCamera( env );

   /* Add new entry and sort. */
   lf = &array_grow( &spob_lua_stack );
   lf->filename = strdup( filename );
   lf->env = env;

   /* Add the spob memory table. */
   lua_newtable(naevL);              /* m */
   lua_pushvalue(naevL, -1);         /* m, m */
   lf->lua_mem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* m */
   nlua_setenv(naevL, env, "mem");   /* */
   *mem = lf->lua_mem;

   if (nlua_dobufenv(env, dat, sz, filename) != 0) {
      int n;
      WARN(_("Lua Spob '%s' error:\n%s"), filename, lua_tostring(naevL,-1));
      lua_pop(naevL,1);
      spob_lua_free( lf );
      free( dat );
      n = array_size( spob_lua_stack );
      array_erase( &spob_lua_stack, &spob_lua_stack[n-1], &spob_lua_stack[n] );
      return LUA_NOREF;
   }
   free(dat);

   qsort( spob_lua_stack, array_size(spob_lua_stack), sizeof(spob_lua_file), spob_lua_cmp );
   return env;
}

static void spob_lua_free( spob_lua_file *lf )
{
   free( (char *) lf->filename );
   nlua_freeEnv( lf->env );
   luaL_unref( naevL, LUA_REGISTRYINDEX, lf->lua_mem );
}
