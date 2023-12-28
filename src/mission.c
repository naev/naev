/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file mission.c
 *
 * @brief Handles missions.
 */
/** @cond */
#include <stdint.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "mission.h"

#include "array.h"
#include "cond.h"
#include "faction.h"
#include "gui_osd.h"
#include "hook.h"
#include "land.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_faction.h"
#include "nlua_misn.h"
#include "nlua_ship.h"
#include "nlua_shiplog.h"
#include "nluadef.h"
#include "npc.h"
#include "nstring.h"
#include "nxml.h"
#include "nxml_lua.h"
#include "player.h"
#include "player_fleet.h"
#include "rng.h"
#include "space.h"

#define XML_MISSION_TAG       "mission" /**< XML mission tag. */

/*
 * current player missions
 */
static unsigned int mission_id = 0; /**< Mission ID generator. */
Mission **player_missions = NULL; /**< Player's active missions. */
static char **player_missions_failed = NULL; /**< Name of missions that failed to load. */

/*
 * mission stack
 */
static MissionData *mission_stack = NULL; /**< Unmutable after creation */

/*
 * prototypes
 */
/* static */
/* Generation. */
static unsigned int mission_genID (void);
static int mission_init( Mission* mission, const MissionData* misn, int genid, int create, unsigned int *id );
static void mission_freeData( MissionData* mission );
/* Matching. */
static int mission_compare( const void* arg1, const void* arg2 );
static int mission_meetConditionals( const MissionData *misn );
static int mission_meetReq( const MissionData *misn, int faction,
      const Spob *pnt, const StarSystem *sys );
static int mission_matchFaction( const MissionData* misn, int faction );
static int mission_location( const char *loc );
/* Loading. */
static int missions_cmp( const void *a, const void *b );
static int mission_parseFile( const char* file, MissionData *temp );
static int mission_parseXML( MissionData *temp, const xmlNodePtr parent );
static int missions_parseActive( xmlNodePtr parent );
/* Misc. */
static const char* mission_markerTarget( const MissionMarker *m );
static int mission_markerLoad( Mission *misn, xmlNodePtr node );

/**
 * @brief Generates a new id for the mission.
 *
 *    @return New id for the mission.
 */
static unsigned int mission_genID (void)
{
   unsigned int id = ++mission_id; /* default id, not safe if loading */
   /* we save mission ids, so check for collisions with player's missions */
   for (int i=0; i<array_size(player_missions); i++)
      if (id == player_missions[i]->id) /* mission id was loaded from save */
         return mission_genID(); /* recursively try again */
   return id;
}

/**
 * @brief Gets id from mission name.
 *
 *    @param name Name to match.
 *    @return id of the matching mission.
 */
int mission_getID( const char* name )
{
   for (int i=0; i<array_size(mission_stack); i++)
      if (strcmp(name,mission_stack[i].name)==0)
         return i;

   WARN(_("Mission '%s' not found in stack"), name);
   return -1;
}

/**
 * @brief Gets a MissionData based on ID.
 *
 *    @param id ID to match.
 *    @return MissonData matching ID.
 */
const MissionData* mission_get( int id )
{
   if ((id < 0) || (id >= array_size(mission_stack))) return NULL;
   return &mission_stack[id];
}

/**
 * @brief Gets mission data from a name.
 */
const MissionData* mission_getFromName( const char* name )
{
   int id = mission_getID( name );
   if (id < 0)
      return NULL;

   return mission_get( id );
}

/**
 * @brief Initializes a mission.
 *
 *    @param mission Mission to initialize.
 *    @param misn Data to use.
 *    @param genid 1 if should generate id, 0 otherwise.
 *    @param create 1 if should run create function, 0 otherwise.
 *    @param[out] id ID of the newly created mission.
 *    @return 0 on success.
 */
static int mission_init( Mission* mission, const MissionData* misn, int genid, int create, unsigned int *id )
{
   if (misn->chunk == LUA_NOREF) {
      WARN(_("Trying to initialize mission '%s' that has no loaded Lua chunk!"), misn->name);
      return -1;
   }

   /* clear the mission */
   memset( mission, 0, sizeof(Mission) );
   mission->env = LUA_NOREF;

   /* Create id if needed. */
   mission->id    = (genid) ? mission_genID() : 0;

   if (id != NULL)
      *id         = mission->id;
   mission->data  = misn;
   if (create) {
      mission->title = strdup(_(misn->name));
      mission->desc  = strdup(_("No description."));
   }

   /* init Lua */
   mission->env = nlua_newEnv();

   misn_loadLibs( mission->env ); /* load our custom libraries */

   /* Create the "mem" table for persistence. */
   lua_newtable(naevL);
   nlua_setenv(naevL, mission->env, "mem");

   /* load the file */
   if (nlua_dochunkenv(mission->env, misn->chunk, misn->sourcefile) != 0) {
      WARN(_("Error loading mission file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check"),
           misn->sourcefile, lua_tostring(naevL, -1));
      return -1;
   }

   /* run create function */
   if (create) {
      /* Failed to create. */
      int ret = misn_run( mission, "create");
      if (ret) {
         mission_cleanup(mission);
         return ret;
      }
   }

   return 0;
}

/**
 * @brief Small wrapper for misn_run.
 *
 *    @param mission Mission to accept.
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted,
 *          3 if the mission got accepted, and 0 normally.
 *
 * @sa misn_run
 */
int mission_accept( Mission* mission )
{
   return misn_run( mission, "accept" );
}

/**
 * @brief Returns all the missions.
 */
const MissionData *mission_list (void)
{
   return mission_stack;
}

/**
 * @brief Checks to see if mission is already running.
 *
 *    @param misn Mission to check if is already running.
 *    @return Number of instances if already running, 0 if isn't.
 */
int mission_alreadyRunning( const MissionData* misn )
{
   int n = 0;
   for (int i=0; i<array_size(player_missions); i++)
      if (player_missions[i]->data == misn)
         n++;
   return n;
}

static int mission_meetConditionals( const MissionData *misn )
{
   /* If chapter, must match chapter. */
   if (misn->avail.chapter_re != NULL) {
      pcre2_match_data *match_data = pcre2_match_data_create_from_pattern( misn->avail.chapter_re, NULL );
      int rc = pcre2_match( misn->avail.chapter_re, (PCRE2_SPTR)player.chapter, strlen(player.chapter), 0, 0, match_data, NULL );
      pcre2_match_data_free( match_data );
      if (rc < 0) {
         switch (rc) {
            case PCRE2_ERROR_NOMATCH:
               return -1;
            default:
               WARN(_("Matching error %d"), rc );
               break;
         }
      }
      else if (rc == 0)
         return 1;
   }

   /* Must not be already done or running if unique. */
   if (mis_isFlag(misn,MISSION_UNIQUE) &&
         (player_missionAlreadyDone( mission_getID(misn->name) ) ||
          mission_alreadyRunning(misn)))
      return 1;

   /* Must meet Lua condition. */
   if (misn->avail.cond != NULL) {
      int c = cond_checkChunk( misn->avail.cond_chunk, misn->avail.cond );
      if (c < 0) {
         WARN(_("Conditional for mission '%s' failed to run"), misn->name);
         return 1;
      }
      else if (!c)
         return 1;
   }

   /* Must meet previous mission requirements. */
   if ((misn->avail.done != NULL) &&
         (player_missionAlreadyDone( mission_getID(misn->avail.done) ) == 0))
      return 1;

  return 0;
}

/**
 * @brief Checks to see if a mission meets the requirements.
 *
 *    @param misn Mission to check.
 *    @param faction Faction of the current spob.
 *    @param pnt Spob to run on.
 *    @param sys System to run on.
 *    @return 1 if requirements are met, 0 if they aren't.
 */
static int mission_meetReq( const MissionData *misn, int faction,
      const Spob *pnt, const StarSystem *sys )
{
   if (misn == NULL) /* In case it doesn't exist */
      return 0;

   /* If spob, must match spob. */
   if (misn->avail.spob != NULL) {
      if ((pnt==NULL) || strcmp(misn->avail.spob,pnt->name)!=0)
         return 0;
   }
   else if (spob_isFlag(pnt, SPOB_NOMISNSPAWN))
      return 0;

   /* If system, must match system. */
   if ((misn->avail.system != NULL) && (sys==NULL || (strcmp(misn->avail.system,sys->name)!=0)))
      return 0;

   /* Match faction. */
   if ((faction >= 0) && !mission_matchFaction(misn,faction))
      return 0;

   return !mission_meetConditionals( misn );
}

/**
 * @brief Runs missions matching location, all Lua side and one-shot.
 *
 *    @param loc Location to match.
 *    @param faction Faction of the spob.
 *    @param pnt Spob to run on.
 *    @param sys System to run on.
 */
void missions_run( MissionAvailability loc, int faction, const Spob *pnt, const StarSystem *sys )
{
   for (int i=0; i<array_size(mission_stack); i++) {
      Mission mission;
      double chance;
      MissionData *misn = &mission_stack[i];

      if (naev_isQuit())
         return;

      if (misn->avail.loc != loc)
         continue;

      if (!mission_meetReq( misn, faction, pnt, sys ))
         continue;

      chance = (double)(misn->avail.chance % 100)/100.;
      if (chance == 0.) /* We want to consider 100 -> 100% not 0% */
         chance = 1.;

      if (RNGF() < chance) {
         mission_init( &mission, misn, 1, 1, NULL );
         mission_cleanup(&mission); /* it better clean up for itself or we do it */
      }
   }
}

/**
 * @brief Starts a mission.
 *
 *  Mission must still call misn.accept() to actually get added to the player's
 * active missions.
 *
 *    @param name Name of the mission to start.
 *    @param[out] id ID of the newly created mission.
 *    @return 0 on success, >0 on forced exit (misn.finish), <0 on error.
 */
int mission_start( const char *name, unsigned int *id )
{
   Mission mission;
   const MissionData *mdat;
   int ret;

   /* Try to get the mission. */
   mdat = mission_get( mission_getID(name) );
   if (mdat == NULL)
      return -1;

   /* Try to run the mission. */
   ret = mission_init( &mission, mdat, 1, 1, id );
   /* Add to mission giver if necessary. */
   if (landed && (ret==0) && (mdat->avail.loc==MIS_AVAIL_BAR))
      npc_patchMission( &mission );
   else
      mission_cleanup( &mission ); /* Clean up in case not accepted. */

   return ret;
}

/**
 * @brief Tests the conditionals of a mission.
 *
 *    @param name Name of the mission to test.
 *    @return -1 on error, 0 on mission conditionals passing, >0 otherwise.
 */
int mission_test( const char *name )
{
   const MissionData *mdat;

   /* Try to get the mission. */
   mdat = mission_get( mission_getID(name) );
   if (mdat == NULL)
      return -1;

   return mission_meetConditionals( mdat );
}

const char *mission_availabilityStr( MissionAvailability loc )
{
   switch (loc) {
      case MIS_AVAIL_UNSET:
         return "unset";
      case MIS_AVAIL_NONE:
         return "none";
      case MIS_AVAIL_COMPUTER:
         return "computer";
      case MIS_AVAIL_BAR:
         return "bar";
      case MIS_AVAIL_LAND:
         return "land";
      case MIS_AVAIL_ENTER:
         return "enter";
   }
   return NULL;
}

/**
 * @brief Gets the name of the mission marker target.
 */
static const char* mission_markerTarget( const MissionMarker *m )
{
   switch (m->type) {
      case SYSMARKER_COMPUTER:
      case SYSMARKER_LOW:
      case SYSMARKER_HIGH:
      case SYSMARKER_PLOT:
         return system_getIndex( m->objid )->name;
      case SPOBMARKER_COMPUTER:
      case SPOBMARKER_LOW:
      case SPOBMARKER_HIGH:
      case SPOBMARKER_PLOT:
         return spob_getIndex( m->objid )->name;
      default:
         WARN(_("Unknown marker type."));
         return NULL;
   }
}

MissionMarkerType mission_markerTypeSpobToSystem( MissionMarkerType t )
{
   switch (t) {
      case SYSMARKER_COMPUTER:
      case SYSMARKER_LOW:
      case SYSMARKER_HIGH:
      case SYSMARKER_PLOT:
         return t;
      case SPOBMARKER_COMPUTER:
         return SYSMARKER_COMPUTER;
      case SPOBMARKER_LOW:
         return SYSMARKER_LOW;
      case SPOBMARKER_HIGH:
         return SYSMARKER_HIGH;
      case SPOBMARKER_PLOT:
         return SYSMARKER_PLOT;
      default:
         WARN(_("Unknown marker type."));
         return -1;
   }
}

MissionMarkerType mission_markerTypeSystemToSpob( MissionMarkerType t )
{
   switch (t) {
      case SYSMARKER_COMPUTER:
         return SPOBMARKER_COMPUTER;
      case SYSMARKER_LOW:
         return SPOBMARKER_LOW;
      case SYSMARKER_HIGH:
         return SPOBMARKER_HIGH;
      case SYSMARKER_PLOT:
         return SPOBMARKER_PLOT;
      case SPOBMARKER_COMPUTER:
      case SPOBMARKER_LOW:
      case SPOBMARKER_HIGH:
      case SPOBMARKER_PLOT:
         return t;
      default:
         WARN(_("Unknown marker type."));
         return -1;
   }
}

void mission_toLuaTable( lua_State *L , const MissionData *m )
{
   lua_newtable(L);

   lua_pushstring(L, m->name);
   lua_setfield(L,-2,"name");

   lua_pushboolean(L,mis_isFlag(m,MISSION_UNIQUE));
   lua_setfield(L,-2,"unique");

   lua_newtable(L);
   for (int j=0; j<array_size(m->tags); j++) {
      lua_pushboolean(L,1);
      lua_setfield(L,-2,m->tags[j]);
   }
   lua_setfield(L,-2,"tags");
}

const char **mission_loadFailed (void)
{
   return (const char**) player_missions_failed;
}

/**
 * @brief Loads a mission marker from xml.
 */
static int mission_markerLoad( Mission *misn, xmlNodePtr node )
{
   int id;
   MissionMarkerType type;
   StarSystem *ssys;
   Spob *pnt;

   xmlr_attr_int_def( node, "id", id, -1 );
   xmlr_attr_int_def( node, "type", type, -1 );

   switch (type) {
      case SYSMARKER_COMPUTER:
      case SYSMARKER_LOW:
      case SYSMARKER_HIGH:
      case SYSMARKER_PLOT:
         ssys = system_get( xml_get( node ));
         if (ssys == NULL) {
            WARN( _("Mission Marker to system '%s' does not exist"), xml_get( node ) );
            return -1;
         }
         return mission_addMarker( misn, id, system_index(ssys), type );
      case SPOBMARKER_COMPUTER:
      case SPOBMARKER_LOW:
      case SPOBMARKER_HIGH:
      case SPOBMARKER_PLOT:
         pnt = spob_get( xml_get( node ));
         if (pnt == NULL) {
            WARN( _("Mission Marker to spob '%s' does not exist"), xml_get( node ) );
            return -1;
         }
         return mission_addMarker( misn, id, spob_index(pnt), type );
      default:
         WARN(_("Unknown marker type."));
         return -1;
   }
}

/**
 * @brief Adds a system marker to a mission.
 */
int mission_addMarker( Mission *misn, int id, int objid, MissionMarkerType type )
{
   MissionMarker *marker;

   /* Create array. */
   if (misn->markers == NULL)
      misn->markers = array_create( MissionMarker );

   /* Avoid ID collisions. */
   if (id < 0) {
      int m = -1;
      for (int i=0; i<array_size(misn->markers); i++)
         if (misn->markers[i].id > m)
            m = misn->markers[i].id;
      id = m+1;
   }

   /* Create the marker. */
   marker      = &array_grow( &misn->markers );
   marker->id  = id;
   marker->objid = objid;
   marker->type = type;

   return marker->id;
}

/**
 * @brief Marks all active systems that need marking.
 */
void mission_sysMark (void)
{
   /* Clear markers. */
   space_clearMarkers();
   for (int i=0; i<array_size(player_missions); i++) {
      /* Must be a valid player mission. */
      if (player_missions[i]->id == 0)
         continue;

      for (int j=0; j<array_size(player_missions[i]->markers); j++) {
         const MissionMarker *m = &player_missions[i]->markers[j];

         /* Add the individual markers. */
         space_addMarker( m->objid, m->type );
      }
   }
}

/**
 * @brief Marks the system of the computer mission to reflect where it will head to.
 *
 * Does not modify other markers.
 *
 *    @param misn Mission to mark.
 */
const StarSystem* mission_sysComputerMark( const Mission* misn )
{
   StarSystem *firstsys = NULL;

   /* Clear markers. */
   space_clearComputerMarkers();

   /* Set all the markers. */
   for (int i=0; i<array_size(misn->markers); i++) {
      StarSystem *sys;
      Spob *pnt;
      const char *sysname;
      const MissionMarker *m = &misn->markers[i];

      switch (m->type) {
         case SYSMARKER_COMPUTER:
         case SYSMARKER_LOW:
         case SYSMARKER_HIGH:
         case SYSMARKER_PLOT:
            sys = system_getIndex( m->objid );
            break;
         case SPOBMARKER_COMPUTER:
         case SPOBMARKER_LOW:
         case SPOBMARKER_HIGH:
         case SPOBMARKER_PLOT:
            pnt = spob_getIndex( m->objid );
            sysname = spob_getSystem( pnt->name );
            if (sysname==NULL) {
               WARN(_("Marked spob '%s' is not in any system!"), pnt->name);
               continue;
            }
            sys = system_get( sysname );
            break;
         default:
            WARN(_("Unknown marker type."));
            continue;
      }

      if (sys != NULL)
         sys_setFlag( sys, SYSTEM_CMARKED );

      if (firstsys==NULL)
         firstsys = sys;
   }
   return firstsys;
}

/**
 * @brief Gets the first system that has been marked by a mission.
 *
 *    @param misn Mission to get marked system of.
 *    @return First marked system.
 */
const StarSystem* mission_getSystemMarker( const Mission* misn )
{
   /* Set all the markers. */
   for (int i=0; i<array_size(misn->markers); i++) {
      StarSystem *sys;
      Spob *pnt;
      const char *sysname;
      const MissionMarker *m = &misn->markers[i];;

      switch (m->type) {
         case SYSMARKER_COMPUTER:
         case SYSMARKER_LOW:
         case SYSMARKER_HIGH:
         case SYSMARKER_PLOT:
            sys = system_getIndex( m->objid );
            break;
         case SPOBMARKER_COMPUTER:
         case SPOBMARKER_LOW:
         case SPOBMARKER_HIGH:
         case SPOBMARKER_PLOT:
            pnt = spob_getIndex( m->objid );
            sysname = spob_getSystem( pnt->name );
            if (sysname==NULL) {
               WARN(_("Marked spob '%s' is not in any system!"), pnt->name);
               continue;
            }
            sys = system_get( sysname );
            break;
         default:
            WARN(_("Unknown marker type."));
            continue;
      }

      return sys;
   }
   return NULL;
}

/**
 * @brief Links cargo to the mission for posterior cleanup.
 *
 *    @param misn Mission to link cargo to.
 *    @param cargo_id ID of cargo to link.
 *    @return 0 on success.
 */
int mission_linkCargo( Mission* misn, unsigned int cargo_id )
{
   if (misn->cargo == NULL)
      misn->cargo = array_create( unsigned int );
   array_push_back( &misn->cargo, cargo_id );
   return 0;
}

/**
 * @brief Unlinks cargo from the mission, removes it from the player.
 *
 *    @param misn Mission to unlink cargo from.
 *    @param cargo_id ID of cargo to unlink.
 *    @return returns 0 on success.
 */
int mission_unlinkCargo( Mission* misn, unsigned int cargo_id )
{
   int i;
   for (i=0; i<array_size(misn->cargo); i++)
      if (misn->cargo[i] == cargo_id)
         break;

   if (i>=array_size(misn->cargo)) { /* not found */
      DEBUG(_("Mission '%s' attempting to unlink nonexistent cargo %d."),
            misn->title, cargo_id);
      return 1;
   }

   /* shrink cargo size. */
   array_erase( &misn->cargo, &misn->cargo[i], &misn->cargo[i+1] );
   return 0;
}

/**
 * @brief Cleans up a mission.
 *
 *    @param misn Mission to clean up.
 */
void mission_cleanup( Mission* misn )
{
   /* Hooks and missions. */
   if (misn->id != 0) {
      hook_rmMisnParent( misn->id ); /* remove existing hooks */
      npc_rm_parentMission( misn->id ); /* remove existing npc */
   }

   /* Cargo. */
   if ((player.p != NULL) && !pilot_isFlag(player.p, PILOT_DEAD)) { /* Only remove if player exists. */
      for (int i=0; i<array_size(misn->cargo); i++) { /* must unlink all the cargo */
         int ret = pilot_rmMissionCargo( player.p, misn->cargo[i], 0 );
         if (ret)
            WARN(_("Failed to remove mission cargo '%d' for mission '%s'."), misn->cargo[i], misn->title);
      }
   }
   array_free(misn->cargo);
   if (misn->osd > 0)
      osd_destroy(misn->osd);
   /*
    * XXX With the way the mission code works, this function is called on a
    * Mission struct of all zeros. Looking at the implementation, luaL_ref()
    * never returns 0, but this is probably undefined behavior.
    */
   if (misn->env != LUA_NOREF)
      nlua_freeEnv(misn->env);

   /* Data. */
   free(misn->title);
   free(misn->desc);
   free(misn->reward);
   gl_freeTexture(misn->portrait);
   free(misn->npc);
   free(misn->npc_desc);

   /* Markers. */
   array_free( misn->markers );

   /* Claims. */
   if (misn->claims != NULL)
      claim_destroy( misn->claims );

   /* Clear the memory. */
   memset( misn, 0, sizeof(Mission) );
   misn->env = LUA_NOREF;
}

/**
 * @brief Puts the specified mission at the end of the player_missions array.
 *
 *    @param pos Mission's position within player_missions
 */
void mission_shift( int pos )
{
   Mission *misn;

   if (pos >= (array_size(player_missions)-1))
      return;

   /* Store specified mission. */
   misn = player_missions[pos];

   /* Move other missions down. */
   memmove( &player_missions[pos], &player_missions[pos+1],
      sizeof(Mission*) * (array_size(player_missions) - pos - 1) );

   /* Put the specified mission at the end of the array. */
   player_missions[array_size(player_missions) - 1] = misn;
}

/**
 * @brief Frees MissionData.
 *
 *    @param mission MissionData to free.
 */
static void mission_freeData( MissionData* mission )
{
   free(mission->name);
   free(mission->lua);
   free(mission->sourcefile);
   free(mission->avail.spob);
   free(mission->avail.system);
   free(mission->avail.chapter);
   pcre2_code_free( mission->avail.chapter_re );
   array_free(mission->avail.factions);
   free(mission->avail.cond);
   free(mission->avail.done);

   if (mission->chunk != LUA_NOREF)
      luaL_unref( naevL, LUA_REGISTRYINDEX, mission->chunk );

   if (mission->avail.cond_chunk != LUA_NOREF)
      luaL_unref( naevL, LUA_REGISTRYINDEX, mission->avail.cond_chunk );

   for (int i=0; i<array_size(mission->tags); i++)
      free(mission->tags[i]);
   array_free(mission->tags);

   /* Clear the memory. */
#ifdef DEBUGGING
   memset( mission, 0, sizeof(MissionData) );
#endif /* DEBUGGING */
}

/**
 * @brief Checks to see if a mission matches the faction requirements.
 *
 *    @param misn Mission to check.
 *    @param faction Faction to check against.
 *    @return 1 if it meets the faction requirement, 0 if it doesn't.
 */
static int mission_matchFaction( const MissionData* misn, int faction )
{
   /* No faction always accepted. */
   if (array_size(misn->avail.factions) == 0)
      return 1;

   /* Check factions. */
   for (int i=0; i<array_size(misn->avail.factions); i++)
      if (faction == misn->avail.factions[i])
         return 1;

   return 0;
}

/**
 * @brief Activates mission claims.
 */
void missions_activateClaims (void)
{
   for (int i=0; i<array_size(player_missions); i++)
      if (player_missions[i]->claims != NULL)
         claim_activate( player_missions[i]->claims );
}

/**
 * @brief Compares to missions to see which has more priority.
 */
static int mission_compare( const void* arg1, const void* arg2 )
{
   Mission *m1, *m2;

   /* Get arguments. */
   m1 = (Mission*) arg1;
   m2 = (Mission*) arg2;

   /* Check priority - lower is more important. */
   if (m1->data->avail.priority < m2->data->avail.priority)
      return -1;
   else if (m1->data->avail.priority > m2->data->avail.priority)
      return +1;

   /* Compare NPC. */
   if ((m1->npc != NULL) && (m2->npc != NULL))
      return strcmp( m1->npc, m2->npc );

   /* Compare title. */
   if ((m1->title != NULL) && (m2->title != NULL))
      return strcmp( m1->title, m2->title );

   /* Tied. */
   return strcmp(m1->data->name, m2->data->name);
}

/**
 * @brief Generates a mission list. This runs create() so won't work with all
 *        missions.
 *
 *    @param[out] n Missions created.
 *    @param faction Faction of the spob.
 *    @param pnt Spob to run on.
 *    @param sys System to run on.
 *    @param loc Location
 *    @return The stack of Missions created with n members.
 */
Mission* missions_genList( int *n, int faction,
      const Spob *pnt, const StarSystem *sys, MissionAvailability loc )
{
   int m, alloced;
   int rep;
   Mission* tmp;

   /* Find available missions. */
   tmp      = NULL;
   m        = 0;
   alloced  = 0;
   for (int i=0; i<array_size(mission_stack); i++) {
      double chance;
      MissionData *misn = &mission_stack[i];
      if (misn->avail.loc != loc)
         continue;

      /* Must hit chance. */
      chance = (double)(misn->avail.chance % 100)/100.;
      if (chance == 0.) /* We want to consider 100 -> 100% not 0% */
         chance = 1.;
      rep = MAX(1, misn->avail.chance / 100);

      /* random chance of rep appearances */
      for (int j=0; j<rep; j++) {
         if (RNGF() > chance)
            continue;

         /* Must meet requirements. */
         if (!mission_meetReq( misn, faction, pnt, sys ))
            continue;

         m++;
         /* Extra allocation. */
         if (m > alloced) {
            if (alloced == 0)
               alloced = 32;
            else
               alloced *= 2;
            tmp      = realloc( tmp, sizeof(Mission) * alloced );
         }
         /* Initialize the mission. */
         if (mission_init( &tmp[m-1], misn, 1, 1, NULL ))
            m--;
      }
   }

   /* Sort. */
   if (tmp != NULL) {
      qsort( tmp, m, sizeof(Mission), mission_compare );
      (*n) = m;
   }
   else
      (*n) = 0;

   return tmp;
}

/**
 * @brief Gets location based on a human readable string.
 *
 *    @param loc String to get the location of.
 *    @return Location matching loc.
 */
static int mission_location( const char *loc )
{
   if (loc != NULL) {
      if (strcasecmp( loc, "None" ) == 0)
         return MIS_AVAIL_NONE;
      else if (strcasecmp( loc, "Computer" ) == 0)
         return MIS_AVAIL_COMPUTER;
      else if (strcasecmp( loc, "Bar" ) == 0)
         return MIS_AVAIL_BAR;
      else if (strcasecmp( loc, "Land" ) == 0)
         return MIS_AVAIL_LAND;
      else if (strcasecmp( loc, "Enter" ) == 0)
         return MIS_AVAIL_ENTER;
   }
   return -1;
}

/**
 * @brief Parses a node of a mission.
 *
 *    @param temp Data to load into.
 *    @param parent Node containing the mission.
 *    @return 0 on success.
 */
static int mission_parseXML( MissionData *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;

   /* Clear memory. */
   memset( temp, 0, sizeof(MissionData) );

   /* Defaults. */
   temp->chunk = LUA_NOREF;
   temp->avail.priority = 5;
   temp->avail.loc = MIS_AVAIL_UNSET;
   temp->avail.cond_chunk = LUA_NOREF;

   /* get the name */
   xmlr_attr_strd(parent,"name",temp->name);
   if (temp->name == NULL)
      WARN( _("Mission in %s has invalid or no name"), MISSION_DATA_PATH );

   node = parent->xmlChildrenNode;

   do { /* load all the data */
      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"unique")) { /* Unique mission. */
         mis_setFlag(temp,MISSION_UNIQUE);
         continue;
      }
      if (xml_isNode(node,"location")) {
         temp->avail.loc = mission_location( xml_get(node) );
         if (temp->avail.loc < 0)
            WARN(_("Mission '%s' has unknown location '%s'!"), temp->name, xml_get(node) );
         continue;
      }
      xmlr_int(node,"chance",temp->avail.chance);
      xmlr_strd(node,"spob",temp->avail.spob);
      xmlr_strd(node,"system",temp->avail.system);
      xmlr_strd(node,"chapter",temp->avail.chapter);
      if (xml_isNode(node,"faction")) {
         if (temp->avail.factions == NULL)
            temp->avail.factions = array_create( int );
         array_push_back( &temp->avail.factions, faction_get( xml_get(node) ) );
         continue;
      }
      xmlr_strd(node,"cond",temp->avail.cond);
      xmlr_strd(node,"done",temp->avail.done);
      xmlr_int(node,"priority",temp->avail.priority);

      if (xml_isNode(node,"tags")) {
         xmlNodePtr cur = node->children;
         temp->tags = array_create( char* );
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur, "tag")) {
               const char *tmp = xml_get(cur);
               if (tmp != NULL)
                  array_push_back( &temp->tags, strdup(tmp) );
               continue;
            }
            WARN(_("Mission '%s' has unknown node in tags '%s'."), temp->name, cur->name );
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"notes")) continue; /* Notes for the python mission mapping script */

      WARN(_("Unknown node '%s' in mission '%s'"),node->name,temp->name);
   } while (xml_nextNode(node));

   /* Compile conditional chunk. */
   if (temp->avail.cond != NULL) {
      temp->avail.cond_chunk = cond_compile( temp->avail.cond );
      if (temp->avail.cond_chunk == LUA_NOREF || temp->avail.cond_chunk == LUA_REFNIL)
         WARN(_("Mission '%s' failed to compile Lua conditional!"), temp->name);
   }

   /* Compile regex for chapter matching. */
   if (temp->avail.chapter != NULL) {
      int errornumber;
      PCRE2_SIZE erroroffset;
      temp->avail.chapter_re = pcre2_compile( (PCRE2_SPTR)temp->avail.chapter, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL );
      if (temp->avail.chapter_re == NULL) {
         PCRE2_UCHAR buffer[256];
         pcre2_get_error_message( errornumber, buffer, sizeof(buffer) );
         WARN(_("Mission '%s' chapter PCRE2 compilation failed at offset %d: %s"), temp->name, (int)erroroffset, buffer );
      }
   }

#define MELEMENT(o,s) \
   if (o) WARN( _("Mission '%s' missing/invalid '%s' element"), temp->name, s)
   MELEMENT(temp->avail.loc==MIS_AVAIL_UNSET,"location");
   MELEMENT((temp->avail.loc!=MIS_AVAIL_NONE) && (temp->avail.chance==0),"chance");
   MELEMENT( ((temp->avail.spob!=NULL) && spob_get(temp->avail.spob)==NULL), "spob" );
   MELEMENT( ((temp->avail.system!=NULL) && system_get(temp->avail.system)==NULL), "system" );
#undef MELEMENT

   return 0;
}

/**
 * @brief Ordering function for missions.
 */
static int missions_cmp( const void *a, const void *b )
{
   const MissionData *ma, *mb;
   ma = (const MissionData*) a;
   mb = (const MissionData*) b;
   if (ma->avail.priority < mb->avail.priority)
      return -1;
   else if (ma->avail.priority > mb->avail.priority)
      return +1;
   return strcmp( ma->name, mb->name );
}

/**
 * @brief Loads all the mission data.
 *
 *    @return 0 on success.
 */
int missions_load (void)
{
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */
   char **mission_files;

   /* Run over missions. */
   mission_files = ndata_listRecursive( MISSION_DATA_PATH );
   mission_stack = array_create_size( MissionData, array_size( mission_files ) );
   for (int i=0; i < array_size( mission_files ); i++) {
      mission_parseFile( mission_files[i], NULL );
      free( mission_files[i] );
   }
   array_free( mission_files );
   array_shrink(&mission_stack);

#ifdef DEBUGGING
   for (int i=0; i<array_size(mission_stack); i++) {
      MissionData *md = &mission_stack[i];
      for (int j=i+1; j<array_size(mission_stack); j++)
         if (strcmp( md->name, mission_stack[j].name )==0)
            WARN(_("Duplicate event '%s'!"), md->name);
   }
#endif /* DEBUGGING */

   /* Sort based on priority so higher priority missions can establish claims first. */
   qsort( mission_stack, array_size(mission_stack), sizeof(MissionData), missions_cmp );

#if DEBUGGING
   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_("Loaded %d Mission in %.3f s", "Loaded %d Missions in %.3f s", array_size(mission_stack) ), array_size(mission_stack), time/1000. );
   }
   else
      DEBUG( n_("Loaded %d Mission", "Loaded %d Missions", array_size(mission_stack) ), array_size(mission_stack) );
#endif /* DEBUGGING */

   return 0;
}

/**
 * @brief Parses a single mission.
 *
 *    @param file Source file path.
 *    @param temp Data to load into, or NULL for initial load.
 */
static int mission_parseFile( const char* file, MissionData *temp )
{
   xmlDocPtr doc;
   xmlNodePtr node;
   size_t bufsize;
   char *filebuf;
   const char *pos, *start_pos;

   /* Load string. */
   filebuf = ndata_read( file, &bufsize );
   if (filebuf == NULL) {
      WARN(_("Unable to read data from '%s'"), file);
      return -1;
   }
   if (bufsize==0) {
      free( filebuf );
      return -1;
   }

   /* Skip if no XML. */
   pos = strnstr( filebuf, "</mission>", bufsize );
   if (pos==NULL) {
      pos = strnstr( filebuf, "function create", bufsize );
      if ((pos != NULL) && !strncmp(pos,"--common",bufsize))
         WARN(_("Mission '%s' has create function but no XML header!"), file);
      free(filebuf);
      return -1;
   }

   /* Separate XML header and Lua. */
   start_pos = strnstr( filebuf, "<?xml ", bufsize );
   pos = strnstr( filebuf, "--]]", bufsize );
   if (pos == NULL || start_pos == NULL) {
      WARN(_("Mission file '%s' has missing XML header!"), file);
      return -1;
   }

   /* Parse the header. */
   doc = xmlParseMemory( start_pos, pos-start_pos);
   if (doc == NULL) {
      WARN(_("Unable to parse document XML header for Mission '%s'"), file);
      return -1;
   }

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_MISSION_TAG)) {
      ERR( _("Malformed XML header for '%s' mission: missing root element '%s'"), file, XML_MISSION_TAG );
      return -1;
   }

   if (temp == NULL)
      temp = &array_grow(&mission_stack);
   mission_parseXML( temp, node );
   temp->lua = filebuf;
   temp->sourcefile = strdup(file);

   /* Clear chunk if already loaded. */
   if (temp->chunk != LUA_NOREF) {
      luaL_unref( naevL, LUA_REGISTRYINDEX, temp->chunk );
      temp->chunk = LUA_NOREF;
   }

   /* Load the chunk. */
   int ret = luaL_loadbuffer(naevL, temp->lua, strlen(temp->lua), temp->name );
   if (ret == LUA_ERRSYNTAX)
      WARN(_("Mission Lua '%s' syntax error: %s"), file, lua_tostring(naevL,-1) );
   else
      temp->chunk = luaL_ref( naevL, LUA_REGISTRYINDEX );

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Frees all the mission data.
 */
void missions_free (void)
{
   /* Free all the player missions. */
   missions_cleanup();

   /* Free the mission data. */
   for (int i=0; i<array_size(mission_stack); i++)
      mission_freeData( &mission_stack[i] );
   array_free( mission_stack );
   mission_stack = NULL;

   /* Free the player mission stack. */
   array_free( player_missions );
   player_missions = NULL;

   /* Frees failed missions. */
   array_free( player_missions_failed );
   player_missions_failed = NULL;
}

/**
 * @brief Cleans up all the player's active missions.
 */
void missions_cleanup (void)
{
   for (int i=0; i<array_size(player_missions); i++) {
      mission_cleanup( player_missions[i] );
      free( player_missions[i] );
   }
   array_erase( &player_missions, array_begin(player_missions), array_end(player_missions) );

   for (int i=0; i<array_size(player_missions_failed); i++)
      free( player_missions_failed[i] );
   array_erase( &player_missions_failed, array_begin(player_missions_failed), array_end(player_missions_failed) );
}

/**
 * @brief Saves the player's active missions.
 *
 *    @param writer XML Write to use to save missions.
 *    @return 0 on success.
 */
int missions_saveActive( xmlTextWriterPtr writer )
{
   /* We also save specially created cargos here. */
   PilotCommodity *pcom = pfleet_cargoList();
   xmlw_startElem(writer,"temporary_cargo");
   for (int i=0; i<array_size(pcom); i++) {
      const Commodity *c = pcom[i].commodity;
      if (!c->istemp)
         continue;
      xmlw_startElem(writer,"cargo");
      missions_saveTempCommodity( writer, c );
      xmlw_endElem(writer); /* "cargo" */
   }
   xmlw_endElem(writer); /* "missions_cargo */
   array_free(pcom);

   xmlw_startElem(writer,"missions");
   for (int i=0; i<array_size(player_missions); i++) {
      Mission *misn = player_missions[i];
      if (misn->id == 0)
         continue;

      xmlw_startElem(writer,"mission");

      /* data and id are attributes because they must be loaded first */
      xmlw_attr(writer,"data","%s",misn->data->name);
      xmlw_attr(writer,"id","%u",misn->id);

      xmlw_elem(writer,"title","%s",misn->title);
      xmlw_elem(writer,"desc","%s",misn->desc);
      xmlw_elem(writer,"reward","%s",misn->reward);
      xmlw_elem(writer,"reward_value","%"CREDITS_PRI,misn->reward_value);

      /* Markers. */
      xmlw_startElem( writer, "markers" );
      for (int j=0; j<array_size( misn->markers ); j++) {
         MissionMarker *m = &misn->markers[j];
         xmlw_startElem(writer,"marker");
         xmlw_attr(writer,"id","%d",m->id);
         xmlw_attr(writer,"type","%d",m->type);
         xmlw_str(writer,"%s", mission_markerTarget( m ));
         xmlw_endElem(writer); /* "marker" */
      }
      xmlw_endElem( writer ); /* "markers" */

      /* Cargo */
      xmlw_startElem(writer,"cargos");
      for (int j=0; j<array_size(misn->cargo); j++)
         xmlw_elem(writer,"cargo","%u", misn->cargo[j]);
      xmlw_endElem(writer); /* "cargos" */

      /* OSD. */
      if (misn->osd > 0) {
         int priority;
         char **items = osd_getItems(misn->osd);

         xmlw_startElem(writer,"osd");

         /* Save attributes. */
         xmlw_attr(writer,"title","%s",osd_getTitle(misn->osd));
         xmlw_attr(writer,"nitems","%d",array_size(items));
         xmlw_attr(writer,"active","%d",osd_getActive(misn->osd));
         xmlw_attr(writer,"hidden","%d",osd_getHide(misn->osd));
         priority = osd_getPriority(misn->osd);
         if (priority != misn->data->avail.priority)
            xmlw_attr(writer,"priority","%d",osd_getPriority(misn->osd));

         /* Save messages. */
         for (int j=0; j<array_size(items); j++)
            xmlw_elem(writer,"msg","%s",items[j]);

         xmlw_endElem(writer); /* "osd" */
      }

      /* Claims. */
      xmlw_startElem(writer,"claims");
      claim_xmlSave( writer, misn->claims );
      xmlw_endElem(writer); /* "claims" */

      /* Write Lua magic */
      xmlw_startElem(writer,"lua");
      nxml_persistLua( misn->env, writer );
      xmlw_endElem(writer); /* "lua" */

      xmlw_endElem(writer); /* "mission" */
   }
   xmlw_endElem(writer); /* "missions" */

   return 0;
}

/**
 * @brief Saves a temporary commodity's defintion into the current node.
 *
 *    @param writer XML Write to use to save missions.
 *    @param c Commodity to save.
 *    @return 0 on success.
 */
int missions_saveTempCommodity( xmlTextWriterPtr writer, const Commodity *c )
{
   xmlw_attr( writer, "name", "%s", c->name );
   xmlw_attr( writer, "description", "%s", c->description );
   for (int j=0; j < array_size( c->illegalto ); j++)
      xmlw_elem( writer, "illegalto", "%s", faction_name( c->illegalto[ j ] ) );
   return 0;
}

/**
 * @brief Loads the player's special mission commodities.
 *
 *    @param parent Node containing the player's special mission cargo.
 *    @return 0 on success.
 */
int missions_loadCommodity( xmlNodePtr parent )
{
   xmlNodePtr node;

   /* We have to ensure the temporary_cargo stuff is loaded first. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      if (xml_isNode(node,"temporary_cargo")) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode( cur, "cargo" ))
               (void)missions_loadTempCommodity( cur );
         } while (xml_nextNode( cur ));
         continue;
      }
   } while (xml_nextNode( node ));

   return 0;
}

/**
 * @brief Loads a temporary commodity.
 *
 *    @param cur Node defining the commodity.
 *    @return The temporary commodity, or NULL on failure.
 */
Commodity *missions_loadTempCommodity( xmlNodePtr cur )
{
   xmlNodePtr ccur;
   char *     name, *desc;
   Commodity *c;

   xmlr_attr_strd( cur, "name", name );
   if ( name == NULL ) {
      WARN( _( "Mission cargo without name!" ) );
      return NULL;
   }

   c = commodity_getW( name );
   if ( c != NULL ) {
      free( name );
      return c;
   }

   xmlr_attr_strd( cur, "description", desc );
   if ( desc == NULL ) {
      WARN( _( "Mission temporary cargo '%s' missing description!" ), name );
      free( name );
      return NULL;
   }

   c = commodity_newTemp( name, desc );

   ccur = cur->xmlChildrenNode;
   do {
      xml_onlyNodes( ccur );
      if ( xml_isNode( ccur, "illegalto" ) ) {
         int f = faction_get( xml_get( ccur ) );
         commodity_tempIllegalto( c, f );
      }
   } while ( xml_nextNode( ccur ) );

   free( name );
   free( desc );
   return c;
}

/**
 * @brief Loads the player's active missions from a save.
 *
 *    @param parent Node containing the player's active missions.
 *    @return 0 on success.
 */
int missions_loadActive( xmlNodePtr parent )
{
   xmlNodePtr node;

   /* cleanup old missions */
   missions_cleanup();

   /* After load the normal missions. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,"missions")) {
         if (missions_parseActive( node ) < 0)
            return -1;
         continue;
      }
   } while (xml_nextNode(node));

   return 0;
}

/**
 * @brief Parses the actual individual mission nodes.
 *
 *    @param parent Parent node to parse.
 *    @return 0 on success.
 */
static int missions_parseActive( xmlNodePtr parent )
{
   char *buf;
   char *title;
   const char **items;
   int nitems, active;
   xmlNodePtr node;
   int failed = 0;

   if (player_missions == NULL)
      player_missions = array_create( Mission* );

   if (player_missions_failed == NULL)
      player_missions_failed = array_create( char* );

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node, "mission")) {
         xmlNodePtr cur;
         const MissionData *data;
         int misn_failed = 0;
         Mission *misn = calloc( 1, sizeof(Mission) );

         /* process the attributes to create the mission */
         xmlr_attr_strd(node, "data", buf);
         data = mission_get( mission_getID(buf) );
         if (data == NULL) {
            WARN(_("Mission '%s' from saved game not found in game - ignoring."), buf);
            array_push_back( &player_missions_failed, strdup(buf) );
            free(buf);
            continue;
         }
         else {
            if (mission_init( misn, data, 0, 0, NULL )) {
               WARN(_("Mission '%s' from saved game failed to load properly - ignoring."), buf);
               array_push_back( &player_missions_failed, strdup(buf) );
               free(buf);
               continue;
            }
            misn->accepted = 1;
         }
         free(buf);

         /* this will orphan an identifier */
         xmlr_attr_int(node, "id", misn->id);

         cur = node->xmlChildrenNode;
         do {
            xmlr_strd(cur,"title",misn->title);
            xmlr_strd(cur,"desc",misn->desc);
            xmlr_strd(cur,"reward",misn->reward);
            xmlr_ulong(cur,"reward_value",misn->reward_value);

            /* Get the markers. */
            if (xml_isNode(cur,"markers")) {
               xmlNodePtr nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"marker"))
                     mission_markerLoad( misn, nest );
               } while (xml_nextNode(nest));
            }

            /* Cargo. */
            if (xml_isNode(cur,"cargos")) {
               xmlNodePtr nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"cargo"))
                     mission_linkCargo( misn, xml_getLong(nest) );
               } while (xml_nextNode(nest));
            }

            /* OSD. */
            if (xml_isNode(cur,"osd")) {
               int hidden, priority;
               xmlNodePtr nest;
               int i = 0;
               xmlr_attr_int_def( cur, "nitems", nitems, -1 );
               if (nitems == -1)
                  continue;
               xmlr_attr_strd(cur,"title",title);
               items = malloc( nitems * sizeof(char*) );
               nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"msg")) {
                     if (i > nitems) {
                        WARN(_("Inconsistency with 'nitems' in save file."));
                        break;
                     }
                     items[i] = xml_get(nest);
                     i++;
                  }
               } while (xml_nextNode(nest));

               /* Get priority if set differently. */
               xmlr_attr_int_def( cur, "priority", priority, data->avail.priority );

               /* Create the OSD. */
               misn->osd = osd_create( title, nitems, items, priority );
               free(items);
               free(title);

               /* Set active. */
               xmlr_attr_int_def( cur, "active", active, -1 );
               if (active != -1)
                  osd_active( misn->osd, active );

               xmlr_attr_int_def( cur, "hidden", hidden, 0 );
               osd_setHide( misn->osd, hidden );
            }

            /* Claims. */
            if (xml_isNode(cur,"claims"))
               misn->claims = claim_xmlLoad( cur );

            if (xml_isNode(cur,"lua")) {
               /* start the unpersist routine */
               int ret = nxml_unpersistLua( misn->env, cur );
               if (ret) {
                  WARN(_("Mission '%s' from saved game failed to unpersist Lua properly - ignoring."), data->name);
                  misn_failed = -1;
               }
            }

         } while (xml_nextNode(cur));

         if (misn_failed) {
            array_push_back( &player_missions_failed, strdup(data->name) );
            failed = -1;
            mission_cleanup( misn );
         }
         else
            array_push_back( &player_missions, misn );
      }
   } while (xml_nextNode(node));

   return failed;
}

int mission_reload( const char *name )
{
   int res, id;
   MissionData save, *temp;

   /* Can't use mission_getFromName here. */
   id = mission_getID( name );
   if (id < 0)
      return -1;
   temp = &mission_stack[id];

   if (temp == NULL)
      return -1;
   save = *temp;
   res = mission_parseFile( save.sourcefile, temp );
   if (res == 0)
      mission_freeData( &save );
   else
      *temp = save;
   return res;
}
