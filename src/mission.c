/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file mission.c
 *
 * @brief Handles missions.
 */


#include "mission.h"

#include "naev.h"

#include <stdint.h>
#include "nstring.h"
#include <stdlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_faction.h"
#include "nlua_ship.h"
#include "nlua_misn.h"
#include "nlua_shiplog.h"
#include "rng.h"
#include "log.h"
#include "hook.h"
#include "ndata.h"
#include "nxml.h"
#include "nxml_lua.h"
#include "faction.h"
#include "player.h"
#include "space.h"
#include "cond.h"
#include "gui_osd.h"
#include "npc.h"
#include "array.h"
#include "land.h"


#define XML_MISSION_TAG       "mission" /**< XML mission tag. */

#define MISSION_CHUNK         32 /**< Chunk allocation. */


/*
 * current player missions
 */
static unsigned int mission_id = 0; /**< Mission ID generator. */
Mission *player_missions[MISSION_MAX]; /**< Player's active missions. */


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
static int mission_init( Mission* mission, MissionData* misn, int genid, int create, unsigned int *id );
static void mission_freeData( MissionData* mission );
/* Matching. */
static int mission_compare( const void* arg1, const void* arg2 );
static int mission_meetReq( int mission, int faction,
      const char* planet, const char* sysname );
static int mission_matchFaction( MissionData* misn, int faction );
static int mission_location( const char *loc );
/* Loading. */
static int missions_cmp( const void *a, const void *b );
static int mission_parseFile( const char* file );
static int mission_parseXML( MissionData *temp, const xmlNodePtr parent );
static int missions_parseActive( xmlNodePtr parent );
/* externed */
int missions_saveActive( xmlTextWriterPtr writer );
int missions_loadActive( xmlNodePtr parent );


/**
 * @brief Generates a new id for the mission.
 *
 *    @return New id for the mission.
 */
static unsigned int mission_genID (void)
{
   unsigned int id;
   int i;
   id = ++mission_id; /* default id, not safe if loading */

   /* we save mission ids, so check for collisions with player's missions */
   for (i=0; i<MISSION_MAX; i++)
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
   int i;

   for (i=0; i<array_size(mission_stack); i++)
      if (strcmp(name,mission_stack[i].name)==0)
         return i;

   DEBUG(_("Mission '%s' not found in stack"), name);
   return -1;
}


/**
 * @brief Gets a MissionData based on ID.
 *
 *    @param id ID to match.
 *    @return MissonData matching ID.
 */
MissionData* mission_get( int id )
{
   if ((id < 0) || (id >= array_size(mission_stack))) return NULL;
   return &mission_stack[id];
}


/**
 * @brief Gets mission data from a name.
 */
MissionData* mission_getFromName( const char* name )
{
   int id;

   id = mission_getID( name );
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
static int mission_init( Mission* mission, MissionData* misn, int genid, int create, unsigned int *id )
{
   int ret;

   /* clear the mission */
   memset( mission, 0, sizeof(Mission) );

   /* Create id if needed. */
   mission->id    = (genid) ? mission_genID() : 0;

   if (id != NULL)
      *id         = mission->id;
   mission->data  = misn;
   if (create) {
      mission->title = strdup(misn->name);
      mission->desc  = strdup(_("No description."));
   }

   /* init Lua */
   mission->env = nlua_newEnv(1);

   misn_loadLibs( mission->env ); /* load our custom libraries */

   /* load the file */
   if (nlua_dobufenv(mission->env, misn->lua, strlen(misn->lua), misn->sourcefile) != 0) {
      WARN(_("Error loading mission file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check"),
           misn->sourcefile, lua_tostring(naevL, -1));
      return -1;
   }

   /* run create function */
   if (create) {
      /* Failed to create. */
      ret = misn_run( mission, "create");
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
 *    @return -1 on error, 1 on misn.finish() call, 2 if mission got deleted
 *            and 0 normally.
 *
 * @sa misn_run
 */
int mission_accept( Mission* mission )
{
   return misn_run( mission, "accept" );
}


/**
 * @brief Checks to see if mission is already running.
 *
 *    @param misn Mission to check if is already running.
 *    @return 1 if already running, 0 if isn't.
 */
int mission_alreadyRunning( MissionData* misn )
{
   int i;
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i]->data == misn)
         return 1;
   return 0;
}


/**
 * @brief Checks to see if a mission meets the requirements.
 *
 *    @param mission ID of the mission to check.
 *    @param faction Faction of the current planet.
 *    @param planet Name of the current planet.
 *    @param sysname Name of the current system.
 *    @return 1 if requirements are met, 0 if they aren't.
 */
static int mission_meetReq( int mission, int faction,
      const char* planet, const char* sysname )
{
   MissionData* misn;
   int c;

   misn = mission_get( mission );
   if (misn == NULL) /* In case it doesn't exist */
      return 0;

   /* If planet, must match planet. */
   if ((misn->avail.planet != NULL) && (strcmp(misn->avail.planet,planet)!=0))
      return 0;

   /* If system, must match system. */
   if ((misn->avail.system != NULL) && (strcmp(misn->avail.system,sysname)!=0))
      return 0;

   /* Match faction. */
   if ((faction >= 0) && !mission_matchFaction(misn,faction))
      return 0;

   /* Must not be already done or running if unique. */
   if (mis_isFlag(misn,MISSION_UNIQUE) &&
         (player_missionAlreadyDone(mission) ||
          mission_alreadyRunning(misn)))
      return 0;

   /* Must meet Lua condition. */
   if (misn->avail.cond != NULL) {
      c = cond_check(misn->avail.cond);
      if (c < 0) {
         WARN(_("Conditional for mission '%s' failed to run"), misn->name);
         return 0;
      }
      else if (!c)
         return 0;
   }

   /* Must meet previous mission requirements. */
   if ((misn->avail.done != NULL) &&
         (player_missionAlreadyDone( mission_getID(misn->avail.done) ) == 0))
      return 0;

  return 1;
}


/**
 * @brief Runs missions matching location, all Lua side and one-shot.
 *
 *    @param loc Location to match.
 *    @param faction Faction of the planet.
 *    @param planet Name of the current planet.
 *    @param sysname Name of the current system.
 */
void missions_run( int loc, int faction, const char* planet, const char* sysname )
{
   MissionData* misn;
   Mission mission;
   int i;
   double chance;

   for (i=0; i<array_size(mission_stack); i++) {
      misn = &mission_stack[i];
      if (misn->avail.loc != loc)
         continue;

      if (!mission_meetReq(i, faction, planet, sysname))
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
   MissionData *mdat;
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
 * @brief Adds a system marker to a mission.
 */
int mission_addMarker( Mission *misn, int id, int sys, SysMarker type )
{
   MissionMarker *marker;
   int i, n, m;

   /* Create array. */
   if (misn->markers == NULL)
      misn->markers = array_create( MissionMarker );

   /* Avoid ID collisions. */
   if (id < 0) {
      m = -1;
      n = array_size( misn->markers );
      for (i=0; i<n; i++)
         if (misn->markers[i].id > m)
            m = misn->markers[i].id;
      id = m+1;
   }

   /* Create the marker. */
   marker      = &array_grow( &misn->markers );
   marker->id  = id;
   marker->sys = sys;
   marker->type = type;

   return marker->id;
}


/**
 * @brief Marks all active systems that need marking.
 */
void mission_sysMark (void)
{
   int i, j, n;
   MissionMarker *m;

   /* Clear markers. */
   space_clearMarkers();

   for (i=0; i<MISSION_MAX; i++) {
      /* Must be a valid player mission. */
      if (player_missions[i]->id == 0)
         continue;
      /* Must have markers. */
      if (player_missions[i]->markers == NULL)
         continue;

      n = array_size( player_missions[i]->markers );
      for (j=0; j<n; j++) {
         m = &player_missions[i]->markers[j];

         /* Add the individual markers. */
         space_addMarker( m->sys, m->type );
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
void mission_sysComputerMark( Mission* misn )
{
   StarSystem *sys;
   int i, n;

   /* Clear markers. */
   space_clearComputerMarkers();

   /* Set all the markers. */
   if (misn->markers != NULL) {
      n = array_size(misn->markers);
      for (i=0; i<n; i++) {
         sys = system_getIndex( misn->markers[i].sys );
         sys_setFlag( sys,SYSTEM_CMARKED );
      }
   }
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
   misn->ncargo++;
   misn->cargo = realloc( misn->cargo, sizeof(unsigned int) * misn->ncargo);
   misn->cargo[ misn->ncargo-1 ] = cargo_id;

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
   for (i=0; i<misn->ncargo; i++)
      if (misn->cargo[i] == cargo_id)
         break;

   if (i>=misn->ncargo) { /* not found */
      DEBUG(_("Mission '%s' attempting to unlink nonexistent cargo %d."),
            misn->title, cargo_id);
      return 1;
   }

   /* shrink cargo size - no need to realloc */
   memmove( &misn->cargo[i], &misn->cargo[i+1],
         sizeof(unsigned int) * (misn->ncargo-i-1) );
   misn->ncargo--;

   return 0;
}


/**
 * @brief Cleans up a mission.
 *
 *    @param misn Mission to clean up.
 */
void mission_cleanup( Mission* misn )
{
   int i, ret;

   /* Hooks and missions. */
   if (misn->id != 0) {
      hook_rmMisnParent( misn->id ); /* remove existing hooks */
      npc_rm_parentMission( misn ); /* remove existing npc */
   }

   /* Cargo. */
   if (misn->cargo != NULL) {
      for (i=0; i<misn->ncargo; i++) { /* must unlink all the cargo */
         if (player.p != NULL) { /* Only remove if player exists. */
            ret = pilot_rmMissionCargo( player.p, misn->cargo[i], 0 );
            if (ret)
               WARN(_("Failed to remove mission cargo '%d' for mission '%s'."), misn->cargo[i], misn->title);
         }
      }
      free(misn->cargo);
   }
   if (misn->osd > 0)
      osd_destroy(misn->osd);
   /*
    * XXX With the way the mission code works, this function is called on a
    * Mission struct of all zeros. Looking at the implementation, luaL_ref()
    * never returns 0, but this is probably undefined behavior.
    */
   if (misn->env != LUA_NOREF && misn->env != 0)
      nlua_freeEnv(misn->env);

   /* Data. */
   if (misn->title != NULL)
      free(misn->title);
   if (misn->desc != NULL)
      free(misn->desc);
   if (misn->reward != NULL)
      free(misn->reward);
   if (misn->portrait != NULL)
      gl_freeTexture(misn->portrait);
   if (misn->npc != NULL)
      free(misn->npc);

   /* Markers. */
   if (misn->markers != NULL)
      array_free( misn->markers );

   /* Claims. */
   if (misn->claims != NULL)
      claim_destroy( misn->claims );

   /* Clear the memory. */
   memset( misn, 0, sizeof(Mission) );
}


/**
 * @brief Puts the specified mission at the end of the player_missions array.
 *
 *    @param pos Mission's position within player_missions
 */
void mission_shift( int pos )
{
   Mission *misn;

   if (pos >= (MISSION_MAX-1))
      return;

   /* Store specified mission. */
   misn = player_missions[pos];

   /* Move other missions down. */
   memmove( &player_missions[pos], &player_missions[pos+1],
      sizeof(Mission*) * (MISSION_MAX - pos - 1) );

   /* Put the specified mission at the end of the array. */
   player_missions[MISSION_MAX - 1] = misn;
}


/**
 * @brief Frees MissionData.
 *
 *    @param mission MissionData to free.
 */
static void mission_freeData( MissionData* mission )
{
   if (mission->name)
      free(mission->name);
   if (mission->lua)
      free(mission->lua);
   if (mission->sourcefile)
      free(mission->sourcefile);
   if (mission->avail.planet)
      free(mission->avail.planet);
   if (mission->avail.system)
      free(mission->avail.system);
   if (mission->avail.factions)
      free(mission->avail.factions);
   if (mission->avail.cond)
      free(mission->avail.cond);
   if (mission->avail.done)
      free(mission->avail.done);

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
static int mission_matchFaction( MissionData* misn, int faction )
{
   int i;

   /* No faction always accepted. */
   if (misn->avail.nfactions <= 0)
      return 1;

   /* Check factions. */
   for (i=0; i<misn->avail.nfactions; i++)
      if (faction == misn->avail.factions[i])
         return 1;

   return 0;
}


/**
 * @brief Activates mission claims.
 */
void missions_activateClaims (void)
{
   int i;

   for (i=0; i<MISSION_MAX; i++)
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
 *    @param faction Faction of the planet.
 *    @param planet Name of the planet.
 *    @param sysname Name of the current system.
 *    @param loc Location
 *    @return The stack of Missions created with n members.
 */
Mission* missions_genList( int *n, int faction,
      const char* planet, const char* sysname, int loc )
{
   int i,j, m, alloced;
   double chance;
   int rep;
   Mission* tmp;
   MissionData* misn;

   /* Find available missions. */
   tmp      = NULL;
   m        = 0;
   alloced  = 0;
   for (i=0; i<array_size(mission_stack); i++) {
      misn = &mission_stack[i];
      if (misn->avail.loc == loc) {

         /* Must meet requirements. */
         if (!mission_meetReq(i, faction, planet, sysname))
            continue;

         /* Must hit chance. */
         chance = (double)(misn->avail.chance % 100)/100.;
         if (chance == 0.) /* We want to consider 100 -> 100% not 0% */
            chance = 1.;
         rep = MAX(1, misn->avail.chance / 100);

         for (j=0; j<rep; j++) /* random chance of rep appearances */
            if (RNGF() < chance) {
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
   if ( loc != NULL ) {
      if ( strcmp( loc, "None" ) == 0 )
         return MIS_AVAIL_NONE;
      else if ( strcmp( loc, "Computer" ) == 0 )
         return MIS_AVAIL_COMPUTER;
      else if ( strcmp( loc, "Bar" ) == 0 )
         return MIS_AVAIL_BAR;
      else if ( strcmp( loc, "Outfit" ) == 0 )
         return MIS_AVAIL_OUTFIT;
      else if ( strcmp( loc, "Shipyard" ) == 0 )
         return MIS_AVAIL_SHIPYARD;
      else if ( strcmp( loc, "Land" ) == 0 )
         return MIS_AVAIL_LAND;
      else if ( strcmp( loc, "Commodity" ) == 0 )
         return MIS_AVAIL_COMMODITY;
      else if ( strcmp( loc, "Space" ) == 0 )
         return MIS_AVAIL_SPACE;
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
   xmlNodePtr cur, node;

   /* Clear memory. */
   memset( temp, 0, sizeof(MissionData) );

   /* Defaults. */
   temp->avail.priority = 5;

   /* get the name */
   temp->name = xml_nodeProp(parent,"name");
   if (temp->name == NULL)
      WARN( _("Mission in %s has invalid or no name"), MISSION_DATA_PATH );

   node = parent->xmlChildrenNode;

   do { /* load all the data */

      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"flags")) { /* set the various flags */
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"unique")) {
               mis_setFlag(temp,MISSION_UNIQUE);
               continue;
            }
            WARN(_("Mission '%s' has unknown flag node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"avail")) { /* mission availability */
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"location")) {
               temp->avail.loc = mission_location( xml_get(cur) );
               continue;
            }
            xmlr_int(cur,"chance",temp->avail.chance);
            xmlr_strd(cur,"planet",temp->avail.planet);
            xmlr_strd(cur,"system",temp->avail.system);
            if (xml_isNode(cur,"faction")) {
               temp->avail.factions = realloc( temp->avail.factions,
                     sizeof(int) * ++temp->avail.nfactions );
               temp->avail.factions[temp->avail.nfactions-1] =
                     faction_get( xml_get(cur) );
               continue;
            }
            xmlr_strd(cur,"cond",temp->avail.cond);
            xmlr_strd(cur,"done",temp->avail.done);
            xmlr_int(cur,"priority",temp->avail.priority);
            WARN(_("Mission '%s' has unknown avail node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      else if (xml_isNode(node,"notes")) continue; /* Notes for the python mission mapping script */

      DEBUG(_("Unknown node '%s' in mission '%s'"),node->name,temp->name);
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
   if (o) WARN( _("Mission '%s' missing/invalid '%s' element"), temp->name, s)
   MELEMENT(temp->avail.loc==-1,"location");
   MELEMENT((temp->avail.loc!=MIS_AVAIL_NONE) && (temp->avail.chance==0),"chance");
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
   int    i;
   char **mission_files;

   /* Allocate player missions. */
   for (i=0; i<MISSION_MAX; i++)
      player_missions[i] = calloc(1, sizeof(Mission));

   /* Run over missions. */
   mission_files = ndata_listRecursive( MISSION_DATA_PATH );
   mission_stack = array_create_size( MissionData, array_size( mission_files ) );
   for ( i = 0; i < array_size( mission_files ); i++ ) {
      mission_parseFile( mission_files[i] );
      free( mission_files[i] );
   }
   array_free( mission_files );
   array_shrink(&mission_stack);

   /* Sort based on priority so higher priority missions can establish claims first. */
   qsort( mission_stack, array_size(mission_stack), sizeof(MissionData), missions_cmp );

   DEBUG( ngettext("Loaded %d Mission", "Loaded %d Missions", array_size(mission_stack) ), array_size(mission_stack) );

   return 0;
}


/**
 * @brief Parses a single mission.
 */
static int mission_parseFile( const char* file )
{
   xmlDocPtr doc;
   xmlNodePtr node;
   size_t bufsize;
   char *filebuf, *luabuf;
   const char *pos;
   MissionData *temp;

#ifdef DEBUGGING
   /* To check if mission is valid. */
   int ret;
#endif /* DEBUGGING */

   /* Load string. */
   filebuf = ndata_read( file, &bufsize );
   if (filebuf == NULL) {
      WARN(_("Unable to read data from '%s'"), file);
      return -1;
   }

   /* Skip if no XML. */
   pos = nstrnstr( filebuf, "</mission>", bufsize );
   if (pos==NULL) {
      pos = nstrnstr( filebuf, "function create", bufsize );
      if ((pos != NULL) && !strncmp(pos,"--common",bufsize))
         WARN(_("Mission '%s' has create function but no XML header!"), file);
      free(filebuf);
      return 0;
   }

   /* Separate XML header and Lua. */
   pos = nstrnstr( filebuf, "--]]", bufsize );
   if (pos == NULL) {
      WARN(_("Mission file '%s' has missing XML header!"), file);
      return -1;
   }
   luabuf = &filebuf[ pos-filebuf+4 ];

   /* Parse the header. */
   doc = xmlParseMemory( &filebuf[5], pos-filebuf-5 );
   if (doc == NULL) {
      WARN(_("Unable to parse document XML header for Mission '%s'"), file);
      return -1;
   }

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_MISSION_TAG)) {
      ERR( _("Malformed XML header for '%s' mission: missing root element '%s'"), file, XML_MISSION_TAG );
      return -1;
   }

   temp = &array_grow(&mission_stack);
   mission_parseXML( temp, node );
   temp->lua = calloc( 1, bufsize-(luabuf-filebuf)+1 );
   temp->sourcefile = strdup(file);
   strncpy( temp->lua, luabuf, bufsize-(luabuf-filebuf) );

#ifdef DEBUGGING
   /* Check to see if syntax is valid. */
   ret = luaL_loadbuffer(naevL, temp->lua, strlen(temp->lua), temp->name );
   if (ret == LUA_ERRSYNTAX) {
      WARN(_("Mission Lua '%s' syntax error: %s"),
            file, lua_tostring(naevL,-1) );
   } else {
      lua_pop(naevL, 1);
   }
#endif /* DEBUGGING */

   /* Clean up. */
   xmlFreeDoc(doc);
   free(filebuf);

   return 0;
}


/**
 * @brief Frees all the mission data.
 */
void missions_free (void)
{
   int i;

   /* Free all the player missions. */
   missions_cleanup();

   /* Free the mission data. */
   for (i=0; i<array_size(mission_stack); i++)
      mission_freeData( &mission_stack[i] );
   array_free( mission_stack );
   mission_stack = NULL;

   /* Free the player mission stack. */
   for (i=0; i<MISSION_MAX; i++)
      free(player_missions[i]);
}


/**
 * @brief Cleans up all the player's active missions.
 */
void missions_cleanup (void)
{
   int i;

   for (i=0; i<MISSION_MAX; i++)
      mission_cleanup( player_missions[i] );
}


/**
 * @brief Saves the player's active missions.
 *
 *    @param writer XML Write to use to save missions.
 *    @return 0 on success.
 */
int missions_saveActive( xmlTextWriterPtr writer )
{
   int i,j,n;
   int nitems;
   char **items;

   xmlw_startElem(writer,"missions");

   for (i=0; i<MISSION_MAX; i++) {
      if (player_missions[i]->id != 0) {
         xmlw_startElem(writer,"mission");

         /* data and id are attributes because they must be loaded first */
         xmlw_attr(writer,"data","%s",player_missions[i]->data->name);
         xmlw_attr(writer,"id","%u",player_missions[i]->id);

         xmlw_elem(writer,"title","%s",player_missions[i]->title);
         xmlw_elem(writer,"desc","%s",player_missions[i]->desc);
         xmlw_elem(writer,"reward","%s",player_missions[i]->reward);

         /* Markers. */
         xmlw_startElem( writer, "markers" );
         if (player_missions[i]->markers != NULL) {
            n = array_size( player_missions[i]->markers );
            for (j=0; j<n; j++) {
               xmlw_startElem(writer,"marker");
               xmlw_attr(writer,"id","%d",player_missions[i]->markers[j].id);
               xmlw_attr(writer,"type","%d",player_missions[i]->markers[j].type);
               xmlw_str(writer,"%s", system_getIndex(player_missions[i]->markers[j].sys)->name);
               xmlw_endElem(writer); /* "marker" */
            }
         }
         xmlw_endElem( writer ); /* "markers" */

         /* Cargo */
         xmlw_startElem(writer,"cargos");
         for (j=0; j<player_missions[i]->ncargo; j++)
            xmlw_elem(writer,"cargo","%u", player_missions[i]->cargo[j]);
         xmlw_endElem(writer); /* "cargos" */

         /* OSD. */
         if (player_missions[i]->osd > 0) {
            xmlw_startElem(writer,"osd");

            /* Save attributes. */
            items = osd_getItems(player_missions[i]->osd, &nitems);
            xmlw_attr(writer,"title","%s",osd_getTitle(player_missions[i]->osd));
            xmlw_attr(writer,"nitems","%d",nitems);
            xmlw_attr(writer,"active","%d",osd_getActive(player_missions[i]->osd));

            /* Save messages. */
            for (j=0; j<nitems; j++)
               xmlw_elem(writer,"msg","%s",items[j]);

            xmlw_endElem(writer); /* "osd" */
         }

         /* Claims. */
         xmlw_startElem(writer,"claims");
         claim_xmlSave( writer, player_missions[i]->claims );
         xmlw_endElem(writer); /* "claims" */

         /* Write Lua magic */
         xmlw_startElem(writer,"lua");
         nxml_persistLua( player_missions[i]->env, writer );
         xmlw_endElem(writer); /* "lua" */

         xmlw_endElem(writer); /* "mission" */
      }
   }

   xmlw_endElem(writer); /* "missions" */

   return 0;
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

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"missions"))
         if (missions_parseActive( node ) < 0) return -1;
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
   Mission *misn;
   MissionData *data;
   int m, i;
   char *buf;
   char *title;
   const char **items;
   int nitems;
   int id, sys, type;
   StarSystem *ssys;

   xmlNodePtr node, cur, nest;

   m = 0; /* start with mission 0 */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node, "mission")) {
         misn = player_missions[m];

         /* process the attributes to create the mission */
         xmlr_attr(node, "data", buf);
         data = mission_get(mission_getID(buf));
         if (data == NULL) {
            WARN(_("Mission '%s' from savegame not found in game - ignoring."), buf);
            free(buf);
            continue;
         }
         else {
            if (mission_init( misn, data, 0, 0, NULL )) {
               WARN(_("Mission '%s' from savegame failed to load properly - ignoring."), buf);
               free(buf);
               continue;
            }
            misn->accepted = 1;
         }
         free(buf);

         /* this will orphan an identifier */
         xmlr_attr(node, "id", buf);
         misn->id = atol(buf);
         free(buf);

         cur = node->xmlChildrenNode;
         do {

            xmlr_strd(cur,"title",misn->title);
            xmlr_strd(cur,"desc",misn->desc);
            xmlr_strd(cur,"reward",misn->reward);

            /* Get the markers. */
            if (xml_isNode(cur,"markers")) {
               nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"marker")) {
                     /* Get ID. */
                     xmlr_attr(nest,"id",buf);
                     id = (buf != NULL) ? atoi(buf) : -1;
                     if (buf != NULL)
                        free(buf);
                     /* Get type. */
                     xmlr_attr(nest,"type",buf);
                     type = (buf != NULL) ? atoi(buf) : -1;
                     if (buf != NULL)
                        free(buf);
                     /* Get system. */
                     ssys = system_get( xml_get( nest ));
                     if (ssys == NULL) {
                        WARN( _("System Marker to '%s' does not exist"), xml_get( nest ) );
                        continue;
                     }
                     sys = system_index( ssys );
                     mission_addMarker( misn, id, sys, type );
                  }
               } while (xml_nextNode(nest));
            }

            /* Cargo. */
            if (xml_isNode(cur,"cargos")) {
               nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"cargo"))
                     mission_linkCargo( misn, xml_getLong(nest) );
               } while (xml_nextNode(nest));
            }

            /* OSD. */
            if (xml_isNode(cur,"osd")) {
               xmlr_attr(cur,"nitems",buf);
               if (buf != NULL) {
                  nitems = atoi(buf);
                  free(buf);
               }
               else
                  continue;
               xmlr_attr(cur,"title",title);
               items = malloc( nitems * sizeof(char*) );
               i = 0;
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

               /* Create the OSD. */
               misn->osd = osd_create( title, nitems, items, data->avail.priority );
               free(items);
               free(title);

               /* Set active. */
               xmlr_attr(cur,"active",buf);
               if (buf != NULL) {
                  osd_active( misn->osd, atoi(buf) );
                  free(buf);
               }
            }

            /* Claims. */
            if (xml_isNode(cur,"claims"))
               misn->claims = claim_xmlLoad( cur );

            if (xml_isNode(cur,"lua"))
               /* start the unpersist routine */
               nxml_unpersistLua( misn->env, cur );

         } while (xml_nextNode(cur));



         m++; /* next mission */
         if (m >= MISSION_MAX) break; /* full of missions, must be an error */
      }
   } while (xml_nextNode(node));

   return 0;
}


