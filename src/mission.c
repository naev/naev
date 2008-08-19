/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file mission.c
 *
 * @brief Handles missions.
 */


#include "mission.h"

#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "nlua.h"
#include "nlua_space.h"
#include "nluadef.h"

#include "rng.h"
#include "naev.h"
#include "log.h"
#include "hook.h"
#include "pack.h"
#include "xml.h"
#include "faction.h"
#include "player.h"
#include "base64.h"
#include "space.h"


#define XML_MISSION_ID        "Missions" /**< XML document identifier */
#define XML_MISSION_TAG       "mission" /**< XML mission tag. */

#define MISSION_DATA          "dat/mission.xml" /**< Path to missions XML. */
#define MISSION_LUA_PATH      "dat/missions/" /**< Path to Lua files. */


/*
 * current player missions
 */
static unsigned int mission_id = 0; /**< Mission ID generator. */
Mission player_missions[MISSION_MAX]; /**< Player's active missions. */


/*
 * mission stack
 */
static MissionData *mission_stack = NULL; /**< Unmuteable after creation */
static int mission_nstack = 0; /**< Mssions in stack. */


/*
 * external space stack
 */
extern StarSystem *systems_stack;
extern int systems_nstack;


/*
 * prototypes
 */
/* extern */
extern int misn_run( Mission *misn, char *func );
/* static */
static unsigned int mission_genID (void);
static int mission_init( Mission* mission, MissionData* misn, int load );
static void mission_freeData( MissionData* mission );
static int mission_alreadyRunning( MissionData* misn );
static int mission_meetCond( MissionData* misn );
static int mission_meetReq( int mission, int faction, char* planet, char* sysname );
static int mission_matchFaction( MissionData* misn, int faction );
static int mission_location( char* loc );
static MissionData* mission_parse( const xmlNodePtr parent );
static int missions_parseActive( xmlNodePtr parent );
static int mission_persistData( lua_State *L, xmlTextWriterPtr writer );
static int mission_unpersistData( lua_State *L, xmlNodePtr parent );
/* externed */
int missions_saveActive( xmlTextWriterPtr writer );
int missions_loadActive( xmlNodePtr parent );


/**
 * @fn static unsigned int mission_genID (void)
 *
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
      if (id == player_missions[i].id) /* mission id was loaded from save */
         return mission_genID(); /* recursively try again */
   return id;
}

/**
 * @fn int mission_getID( char* name )
 *
 * @brief Gets id from mission name.
 *
 *    @param name Name to match.
 *    @return id of the matching mission.
 */
int mission_getID( char* name )
{
   int i;

   for (i=0; i<mission_nstack; i++)
      if (strcmp(name,mission_stack[i].name)==0)
         return i;

   DEBUG("Mission '%s' not found in stack", name);
   return -1;
}


/**
 * @fn MissionData* mission_get( int id )
 *
 * @brief Gets a MissionData based on ID.
 *
 *    @param id ID to match.
 *    @return MissonData matching ID.
 */
MissionData* mission_get( int id )
{
   if ((id <= 0) || (mission_nstack < id)) return NULL;
   return &mission_stack[id];
}


/**
 * @fn static int mission_init( Mission* mission, MissionData* misn, int load )
 *
 * @brief Initializes a mission.
 *
 *    @param mission Mission to initialize.
 *    @param misn Data to use.
 *    @param load 1 if loading mision from save file, otherwise 0.
 *    @return ID of the newly created mission.
 */
static int mission_init( Mission* mission, MissionData* misn, int load )
{
   char *buf;
   uint32_t bufsize;

   /* clear the mission */
   memset(mission,0,sizeof(Mission));

   /* we only need an id if not loading */
   if (load != 0)
      mission->id = 0;
   else
      mission->id = mission_genID();
   mission->data = misn;

   /* init lua */
   mission->L = nlua_newState();
   if (mission->L == NULL) {
      ERR("Unable to create a new lua state.");
      return -1;
   }
   nlua_loadBasic( mission->L ); /* pairs and such */
   nlua_load( mission->L, luaopen_string ); /* string.format can be very useful */
   misn_loadLibs( mission->L ); /* load our custom libraries */

   /* load the file */
   buf = pack_readfile( DATA, misn->lua, &bufsize );
   if (luaL_dobuffer(mission->L, buf, bufsize, misn->lua) != 0) {
      ERR("Error loading mission file: %s\n"
          "%s\n"
          "Most likely Lua file has improper syntax, please check",
            misn->lua, lua_tostring(mission->L,-1));
      return -1;
   }
   free(buf);

   /* run create function */
   if (load == 0) /* never run when loading */
      misn_run( mission, "create");

   return mission->id;
}


/**
 * @fn int mission_accept( Mission* mission )
 *
 * @brief Small wrapper for misn_run.
 *
 *    @param mission Mission to accept.
 *    @return 0 on success.
 *
 * @sa misn_run
 */
int mission_accept( Mission* mission )
{
   int ret;
   ret = misn_run( mission, "accept" );
   if (ret==0) return 1;
   return 0;
}


/**
 * @fn static int mission_alreadyRunning( MissionData* misn )
 *
 * @brief Checks to see if mission is already running.
 *
 *    @param misn Mission to check if is already running.
 *    @return 1 if already running, 0 if isn't.
 */
static int mission_alreadyRunning( MissionData* misn )
{
   int i;
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data==misn)
         return 1;
   return 0;
}


static lua_State* mission_cond_L = NULL; /**< Mission conditional Lua state. */
/**
 * @fn static int mission_meetCond( MissionData* misn )
 *
 * @brief Checks to see if the mission conditional data is met.
 *
 *    @param misn Mission to check.
 *    @return 1 if is met, 0 if it isn't.
 */
static int mission_meetCond( MissionData* misn )
{
   int ret;
   char buf[256];

   /* Create environment if needed. */
   if (mission_cond_L == NULL) { /* must create the conditional environment */
      mission_cond_L = nlua_newState();
      misn_loadCondLibs( mission_cond_L );
   }

   /* Load the string. */ 
   snprintf( buf, 256, "return %s", misn->avail.cond ); /* Must convert to Lua syntax */
   ret = luaL_loadstring( mission_cond_L, buf );
   switch (ret) {
      case  LUA_ERRSYNTAX:
         WARN("Mission '%s' Lua conditional syntax error", misn->name );
         return 0;
      case LUA_ERRMEM:
         WARN("Mission '%s' Lua Conditional ran out of memory", misn->name );
         return 0;
      default:
         break;
   }

   /* Run the string. */
   ret = lua_pcall( mission_cond_L, 0, 1, 0 );
   switch (ret) {
      case LUA_ERRRUN:
         WARN("Mission '%s' Lua Conditional had a runtime error: %s",
               misn->name, lua_tostring(mission_cond_L, -1));
         return 0;
      case LUA_ERRMEM:
         WARN("Mission '%s' Lua Conditional ran out of memory", misn->name);
         return 0;
      case LUA_ERRERR:
         WARN("Mission '%s' Lua Conditional had an error while handling error function",
               misn->name);
         return 0;
      default:
         break;
   }

   /* Check the result. */
   if (lua_isboolean(mission_cond_L, -1)) {
      if (lua_toboolean(mission_cond_L, -1))
         return 1;
      else
         return 0;
   }
   WARN("Mission '%s' Conditional Lua didn't return a boolean", misn->name);
   return 0;
}


/**
 * @fn static int mission_meetReq( int mission, int faction, char* planet, char* sysname )
 *
 * @brief Checks to see if a mission meets the requirements.
 *
 *    @param mission ID of the mission to check.
 *    @param faction Faction of the current planet.
 *    @param planet Name of the current planet.
 *    @param sysname Name of the current system.
 *    @return 1 if requirements are met, 0 if they aren't.
 */
static int mission_meetReq( int mission, int faction, char* planet, char* sysname )
{
   MissionData* misn;

   misn = mission_get( mission );
   if (misn == NULL) /* In case it doesn't exist */
      return 0;

   /* Must match planet, system or faction. */
   if (!((misn->avail.planet && (strcmp(misn->avail.planet,planet)==0)) ||
         (misn->avail.system && (strcmp(misn->avail.system,sysname)==0)) ||
         mission_matchFaction(misn,faction)))
      return 0;

   /* Must not be already done or running if unique. */
   if (mis_isFlag(misn,MISSION_UNIQUE) &&
         (player_missionAlreadyDone(mission) ||
          mission_alreadyRunning(misn)))
      return 0;

   /* Must meet Lua condition. */
   if ((misn->avail.cond != NULL) &&
         !mission_meetCond(misn))
      return 0;

   /* Must meet previous mission requirements. */
   if ((misn->avail.done != NULL) &&
         (player_missionAlreadyDone( mission_getID(misn->avail.done) ) == 0))
      return 0;

  return 1;
}


/**
 * @fn void missions_bar( int faction, char* planet, char* sysname )
 *
 * @brief Runs bar missions, all Lua side and one-shot.
 *
 *    @param faction Faction of the planet.
 *    @param planet Name of the current planet.
 *    @param sysname Name of the current system.
 */
void missions_bar( int faction, char* planet, char* sysname )
{
   MissionData* misn;
   Mission mission;
   int i;
   double chance;

   for (i=0; i<mission_nstack; i++) {
      misn = &mission_stack[i];
      if (misn->avail.loc==MIS_AVAIL_BAR) {

         if (!mission_meetReq(i, faction, planet, sysname))
            continue;

         chance = (double)(misn->avail.chance % 101)/100.;

         if (RNGF() < chance) {
            mission_init( &mission, misn, 0 );
            mission_cleanup(&mission); /* it better clean up for itself or we do it */
         }
      }
   }
}


/**
 * @fn void mission_sysMark (void)
 *
 * @brief Marks all active systems that need marking.
 */
void mission_sysMark (void)
{
   int i;
   StarSystem *sys;

   space_clearMarkers();

   for (i=0; i<MISSION_MAX; i++) {
      if (player_missions[i].sys_marker != NULL) {
         sys = system_get(player_missions[i].sys_marker);
         sys_setFlag(sys,SYSTEM_MARKED);
      }
   }
}


/**
 * @fn void mission_linkCargo( Mission* misn, unsigned int cargo_id )
 *
 * @brief Links cargo to the mission for posterior cleanup.
 *
 *    @param misn Mission to link cargo to.
 *    @param cargo_id ID of cargo to link.
 */
void mission_linkCargo( Mission* misn, unsigned int cargo_id )
{
   misn->ncargo++;
   misn->cargo = realloc( misn->cargo, sizeof(unsigned int) * misn->ncargo);
   misn->cargo[ misn->ncargo-1 ] = cargo_id;
}


/**
 * @fn void mission_unlinkCargo( Mission* misn, unsigned int cargo_id )
 *
 * @brief Unlinks cargo from the mission, removes it from the player.
 *
 *    @param misn Mission to unlink cargo from.
 *    @param cargo_id ID of cargo to unlink.
 */
void mission_unlinkCargo( Mission* misn, unsigned int cargo_id )
{
   int i;
   for (i=0; i<misn->ncargo; i++)
      if (misn->cargo[i] == cargo_id)
         break;

   if (i>=misn->ncargo) { /* not found */
      DEBUG("Mission '%s' attempting to unlink inexistant cargo %d.",
            misn->title, cargo_id);
      return;
   }

   /* shrink cargo size - no need to realloc */
   memmove( &misn->cargo[i], &misn->cargo[i+1],
         sizeof(unsigned int) * (misn->ncargo-i-1) );
   misn->ncargo--;
   player_rmMissionCargo( cargo_id );
}


/**
 * @fn void mission_cleanup( Mission* misn )
 *
 * @brief Cleans up a mission.
 *
 *    @param misn Mission to clean up.
 */
void mission_cleanup( Mission* misn )
{
   int i;
   if (misn->id != 0) {
      hook_rmParent( misn->id ); /* remove existing hooks */
      misn->id = 0;
   }
   if (misn->title != NULL) {
      free(misn->title);
      misn->title = NULL;
   }
   if (misn->desc != NULL) {
      free(misn->desc);
      misn->desc = NULL;
   }
   if (misn->reward != NULL) {
      free(misn->reward);
      misn->reward = NULL;
   }
   if (misn->sys_marker != NULL) {
      free(misn->sys_marker);
      misn->sys_marker = NULL;
   }
   if (misn->cargo != NULL) {
      for (i=0; i<misn->ncargo; i++) /* must unlink all the cargo */
         mission_unlinkCargo( misn, misn->cargo[i] );
      free(misn->cargo);
      misn->cargo = NULL;
      misn->ncargo = 0;
   }
   if (misn->L) {
      lua_close(misn->L);
      misn->L = NULL;
   }
}


/**
 * @fn static void mission_freeData( MissionData* mission )
 *
 * @brief Frees MissionData.
 *
 *    @param mission MissionData to free.
 */
static void mission_freeData( MissionData* mission )
{
   if (mission->name) free(mission->name);
   if (mission->lua) free(mission->lua);
   if (mission->avail.planet) free(mission->avail.planet);
   if (mission->avail.system) free(mission->avail.system);
   if (mission->avail.factions) free(mission->avail.factions);
   if (mission->avail.cond) free(mission->avail.cond);
   if (mission->avail.done) free(mission->avail.done);
   memset( mission, 0, sizeof(MissionData) );
}


/**
 * @fn static int mission_matchFaction( MissionData* misn, int faction )
 *
 * @brief Checks to see if a mission matches the faction requirements.
 *
 *    @param misn Mission to check.
 *    @param faction Faction to check against.
 *    @return 1 if it meets the faction requirement, 0 if it doesn't.
 */
static int mission_matchFaction( MissionData* misn, int faction )
{
   int i;

   for (i=0; i<misn->avail.nfactions; i++)
      if (faction == misn->avail.factions[i])
         return 1;

   return 0;
}


/**
 * @fn Mission* missions_computer( int *n, int faction, char* planet, char* sysname )
 *
 * @brief Generates misisons for the computer - special case.
 *
 *    @param[out] n Missions created.
 *    @param faction Faction of the planet.
 *    @param planet Name of the planet.
 *    @param sysname Name of the current system.
 *    @return The stack of Missions created with n members.
 */
Mission* missions_computer( int *n, int faction, char* planet, char* sysname )
{
   int i,j, m;
   double chance;
   int rep;
   Mission* tmp;
   MissionData* misn;

   tmp = NULL;
   m = 0;
   for (i=0; i<mission_nstack; i++) {
      misn = &mission_stack[i];
      if (misn->avail.loc==MIS_AVAIL_COMPUTER) {

         if (!mission_meetReq(i, faction, planet, sysname))
            continue;

         chance = (double)(misn->avail.chance % 100)/100.;
         rep = misn->avail.chance / 100;

         for (j=0; j<rep; j++) /* random chance of rep appearances */
            if (RNGF() < chance) {
               tmp = realloc( tmp, sizeof(Mission) * ++m);
               mission_init( &tmp[m-1], misn, 0 );
            }
      }
   }

   (*n) = m;
   return tmp;
}


/**
 * @fn static int mission_location( char* loc )
 *
 * @brief Gets location based on a human readable string.
 *
 *    @param loc String to get the location of.
 *    @return Location matching loc.
 */
static int mission_location( char* loc )
{
   if (strcmp(loc,"None")==0) return MIS_AVAIL_NONE;
   else if (strcmp(loc,"Computer")==0) return MIS_AVAIL_COMPUTER;
   else if (strcmp(loc,"Bar")==0) return MIS_AVAIL_BAR;
   else if (strcmp(loc,"Outfit")==0) return MIS_AVAIL_OUTFIT;
   else if (strcmp(loc,"Shipyard")==0) return MIS_AVAIL_SHIPYARD;
   else if (strcmp(loc,"Land")==0) return MIS_AVAIL_LAND;
   return -1;
}


/**
 * @fn static MissionData* mission_parse( const xmlNodePtr parent )
 *
 * @brief Parses a node of a mission.
 *
 *    @param parent Node containing the mission.
 *    @return The MissionData extracted from parent.
 */
static MissionData* mission_parse( const xmlNodePtr parent )
{
   MissionData *temp;
   xmlNodePtr cur, node;

   temp = malloc(sizeof(MissionData));
   memset( temp, 0, sizeof(MissionData) );

   /* get the name */
   temp->name = xml_nodeProp(parent,"name");
   if (temp->name == NULL) WARN("Mission in "MISSION_DATA" has invalid or no name");

   node = parent->xmlChildrenNode;

   char str[PATH_MAX] = "\0";

   do { /* load all the data */
      if (xml_isNode(node,"lua")) {
         snprintf( str, PATH_MAX, MISSION_LUA_PATH"%s.lua", xml_get(node) );
         temp->lua = strdup( str );
         str[0] = '\0';
      }
      else if (xml_isNode(node,"flags")) { /* set the various flags */
         cur = node->children;
         do {
            if (xml_isNode(cur,"unique"))
               mis_setFlag(temp,MISSION_UNIQUE);
         } while (xml_nextNode(cur));
      }
      else if (xml_isNode(node,"avail")) { /* mission availability */
         cur = node->children;
         do {
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
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
   if (o) WARN("Mission '%s' missing/invalid '"s"' element", temp->name)
   MELEMENT(temp->lua==NULL,"lua");
   MELEMENT(temp->avail.loc==-1,"location");
#undef MELEMENT

   return temp;
}


/**
 * @fn int missions_load (void)
 *
 * @brief Loads all the mission data.
 *
 *    @return 0 on success.
 */
int missions_load (void)
{
   uint32_t bufsize;
   char *buf = pack_readfile( DATA, MISSION_DATA, &bufsize );

   MissionData *temp;

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_MISSION_ID)) {
      ERR("Malformed '"MISSION_DATA"' file: missing root element '"XML_MISSION_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first mission node */
   if (node == NULL) {
      ERR("Malformed '"MISSION_DATA"' file: does not contain elements");
      return -1;
   }

   do {
      if (xml_isNode(node,XML_MISSION_TAG)) {

         temp = mission_parse(node);
         mission_stack = realloc(mission_stack, sizeof(MissionData)*(++mission_nstack));
         memcpy(mission_stack+mission_nstack-1, temp, sizeof(MissionData));
         free(temp);
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Mission%s", mission_nstack, (mission_nstack==1) ? "" : "s" );

   return 0;
}


/**
 * @fn void missions_free (void)
 *
 * @brief Frees all the mission data.
 */
void missions_free (void)
{
   int i;

   /* free the mission data */
   for (i=0; i<mission_nstack; i++)
      mission_freeData( &mission_stack[i] );
   free( mission_stack );
   mission_stack = NULL;
   mission_nstack = 0;

   /* frees the lua stack */
   if (mission_cond_L != NULL) {
      lua_close( mission_cond_L );
      mission_cond_L = NULL;
   }
}


/**
 * @fn void missions_cleanup (void)
 *
 * @brief Cleans up all the player's active missions.
 */
void missions_cleanup (void)
{
   int i;

   for (i=0; i<MISSION_MAX; i++)
      mission_cleanup( &player_missions[i] );
}



/**
 * @fn static int mission_saveData( xmlTextWriterPtr writer,
 *       char *type, char *name, char *value )
 *
 * @brief Persists Lua data.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param type Type of the data to save.
 *    @param name Name of the data to save.
 *    @param value Value of the data to save.
 *    @return 0 on success.
 */
static int mission_saveData( xmlTextWriterPtr writer,
      char *type, char *name, char *value )
{
   xmlw_startElem(writer,"data");

   xmlw_attr(writer,"type",type);
   xmlw_attr(writer,"name",name);
   xmlw_str(writer,"%s",value);

   xmlw_endElem(writer); /* "data" */

   return 0;
}


/**
 * @fn static int mission_persistData( lua_State *L, xmlTextWriterPtr writer )
 *
 * @brief Persists all the mission Lua data.
 *
 * Does not save anything in tables nor functions of any type.
 *
 *    @param L Lua state to save.
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
static int mission_persistData( lua_State *L, xmlTextWriterPtr writer )
{
   LuaPlanet *p;
   LuaSystem *s;

   lua_pushnil(L);
   /* nil */
   while (lua_next(L, LUA_GLOBALSINDEX) != 0) {
      /* key, value */
      switch (lua_type(L, -1)) {
         case LUA_TNUMBER:
            mission_saveData( writer, "number",
                  (char*)lua_tostring(L,-2), (char*)lua_tostring(L,-1) );
            break;
         case LUA_TBOOLEAN:
            mission_saveData( writer, "bool",
                  (char*)lua_tostring(L,-2), (char*)lua_tostring(L,-1) );
            break;
         case LUA_TSTRING:
            mission_saveData( writer, "string",
                  (char*)lua_tostring(L,-2), (char*)lua_tostring(L,-1) );
            break;

         /* User data must be hardcoded here. */
         case LUA_TUSERDATA:
            if (lua_isplanet(L,-1)) {
               p = lua_toplanet(L,-1);
               mission_saveData( writer, "planet",
                     (char*)lua_tostring(L,-2), p->p->name );
               break;
            }
            else if (lua_issystem(L,-1)) {
               s = lua_tosystem(L,-1);
               mission_saveData( writer, "system",
                     (char*)lua_tostring(L,-2), s->s->name );
               break;
            }

         default:
            break;
      }
      lua_pop(L,1);
      /* key */
   }

   return 0;
}


/**
 * @fn static int mission_unpersistData( lua_State *L, xmlNodePtr parent )
 *
 * @brief Unpersists Lua data.
 *
 *    @param L State to unperisist data into.
 *    @param parent Node containing all the Lua persisted data.
 *    @return 0 on success.
 */
static int mission_unpersistData( lua_State *L, xmlNodePtr parent )
{
   LuaPlanet p;
   LuaSystem s;
   xmlNodePtr node;
   char *name, *type;

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"data")) {
         xmlr_attr(node,"name",name);
         xmlr_attr(node,"type",type);

         /* handle data types */
         if (strcmp(type,"number")==0)
            lua_pushnumber(L,xml_getFloat(node));
         else if (strcmp(type,"bool")==0)
            lua_pushboolean(L,xml_getInt(node));
         else if (strcmp(type,"string")==0)
            lua_pushstring(L,xml_get(node));
         else if (strcmp(type,"planet")==0) {
            p.p = planet_get(xml_get(node));
            lua_pushplanet(L,p);
         }
         else if (strcmp(type,"system")==0) {
            s.s = system_get(xml_get(node));
            lua_pushsystem(L,s);
         }
         else {
            WARN("Unknown lua data type!");
            return -1;
         }

         /* Set as global */
         lua_setglobal(L,name);
         
         /* cleanup */
         free(type);
         free(name);
      }
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @fn int missions_saveActive( xmlTextWriterPtr writer )
 *
 * @brief Saves the player's active missions.
 *
 *    @param writer XML Write to use to save missions.
 *    @return 0 on success.
 */
int missions_saveActive( xmlTextWriterPtr writer )
{
   int i,j;

   xmlw_startElem(writer,"missions");

   for (i=0; i<MISSION_MAX; i++) {
      if (player_missions[i].id != 0) {
         xmlw_startElem(writer,"mission");

         /* data and id are attributes becaues they must be loaded first */
         xmlw_attr(writer,"data",player_missions[i].data->name);
         xmlw_attr(writer,"id","%u",player_missions[i].id);

         xmlw_elem(writer,"title",player_missions[i].title);
         xmlw_elem(writer,"desc",player_missions[i].desc);
         xmlw_elem(writer,"reward",player_missions[i].reward);
         if (player_missions[i].sys_marker != NULL)
            xmlw_elem(writer,"marker",player_missions[i].sys_marker);

         xmlw_startElem(writer,"cargos");
         for (j=0; j<player_missions[i].ncargo; j++)
            xmlw_elem(writer,"cargo","%u", player_missions[i].cargo[j]);
         xmlw_endElem(writer); /* "cargos" */

         /* write lua magic */
         xmlw_startElem(writer,"lua");

         /* prepare the data */
         mission_persistData(player_missions[i].L, writer);

         xmlw_endElem(writer); /* "lua" */

         xmlw_endElem(writer); /* "mission" */
      }
   }

   xmlw_endElem(writer); /* "missions" */

   return 0;
}


/**
 * @fn int missions_loadActive( xmlNodePtr parent )
 *
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
 * @fn static int missions_parseActive( xmlNodePtr parent )
 *
 * @brief Parses the actual individual mission nodes.
 *
 *    @param parent Parent node to parse.
 *    @return 0 on success.
 */
static int missions_parseActive( xmlNodePtr parent )
{
   Mission *misn;
   int m;
   char *buf;

   xmlNodePtr node, cur, nest;

   m = 0; /* start with mission 0 */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"mission")) {
         misn = &player_missions[m];

         /* process the attributes to create the mission */
         xmlr_attr(node,"data",buf);
         mission_init( misn, mission_get(mission_getID(buf)), 1 );
         free(buf);

         /* this will orphan an identifier */
         xmlr_attr(node,"id",buf);
         misn->id = atol(buf);
         free(buf);

         cur = node->xmlChildrenNode;
         do {

            xmlr_strd(cur,"title",misn->title);
            xmlr_strd(cur,"desc",misn->desc);
            xmlr_strd(cur,"reward",misn->reward);
            xmlr_strd(cur,"marker",misn->sys_marker);

            if (xml_isNode(cur,"cargos")) {
               nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"cargo"))
                     mission_linkCargo( misn, xml_getLong(nest) );
               } while (xml_nextNode(nest));
            }

            if (xml_isNode(cur,"lua"))
               /* start the unpersist routine */
               mission_unpersistData(misn->L, cur);

         } while (xml_nextNode(cur));



         m++; /* next mission */
         if (m >= MISSION_MAX) break; /* full of missions, must be an error */
      }
   } while (xml_nextNode(node));

   return 0;
}


