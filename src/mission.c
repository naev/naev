/*
 * See Licensing and Copyright notice in naev.h
 */


#include "mission.h"

#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "nlua.h"
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


#define XML_MISSION_ID        "Missions"   /* XML section identifier */
#define XML_MISSION_TAG       "mission"

#define MISSION_DATA          "dat/mission.xml"
#define MISSION_LUA_PATH      "dat/missions/"


/*
 * current player missions
 */
static unsigned int mission_id = 0;
Mission player_missions[MISSION_MAX];


/*
 * mission stack
 */
static MissionData *mission_stack = NULL; /* unmuteable after creation */
static int mission_nstack = 0;


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


/*
 * generates a new id for the mission
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

/*
 * gets id from mission name
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


/*
 * gets a MissionData based on ID
 */
MissionData* mission_get( int id )
{
   if ((id <= 0) || (mission_nstack < id)) return NULL;
   return &mission_stack[id];
}


/*
 * initializes a mission
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
   nlua_loadBase( mission->L ); /* pairs and such */
   luaopen_string( mission->L ); /* string.format can be very useful */
   misn_loadLibs( mission->L ); /* load our custom libraries */

   /* load the file */
   buf = pack_readfile( DATA, misn->lua, &bufsize );
   if (luaL_dobuffer(mission->L, buf, bufsize, misn->lua) != 0) {
      ERR("Error loading mission file: %s",misn->lua);
      ERR("%s",lua_tostring(mission->L,-1));
      WARN("Most likely Lua file has improper syntax, please check");
      return -1;
   }
   free(buf);

   /* run create function */
   if (load == 0) /* never run when loading */
      misn_run( mission, "create");

   return mission->id;
}


/*
 * small wrapper for misn_run
 */
int mission_accept( Mission* mission )
{
   int ret;
   ret = misn_run( mission, "accept" );
   if (ret==0) return 1;
   return 0;
}


/*
 * checks to see if mission is already running
 */
static int mission_alreadyRunning( MissionData* misn )
{
   int i;
   for (i=0; i<MISSION_MAX; i++)
      if (player_missions[i].data==misn)
         return 1;
   return 0;
}


/*
 * is the lua condition for misn met?
 */
static lua_State* mission_cond_L = NULL;
static int mission_meetCond( MissionData* misn )
{
   int ret;
   char buf[256];

   if (mission_cond_L == NULL) { /* must create the conditional environment */
      mission_cond_L = nlua_newState();
      misn_loadCondLibs( mission_cond_L );
   }

   snprintf( buf, 256, "return %s", misn->avail.cond );
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


   if (lua_isboolean(mission_cond_L, -1)) {
      if (lua_toboolean(mission_cond_L, -1))
         return 1;
      else
         return 0;
   }
   WARN("Mission '%s' Conditional Lua didn't return a boolean", misn->name);
   return 0;
}

/*
 * does the mission meet the minimum requirements?
 */
static int mission_meetReq( int mission, int faction, char* planet, char* sysname )
{
   MissionData* misn;

   misn = &mission_stack[mission];

   /* must match planet, system or faction */
   if (!(((misn->avail.planet && strcmp(misn->avail.planet,planet)==0)) ||
         (misn->avail.system && (strcmp(misn->avail.system,sysname)==0)) ||
         mission_matchFaction(misn,faction)))
      return 0;

   if (mis_isFlag(misn,MISSION_UNIQUE) && /* mission done or running */
         (player_missionAlreadyDone(mission) ||
          mission_alreadyRunning(misn)))
      return 0;

   if ((misn->avail.cond != NULL) && /* mission doesn't meet the lua conditional */
         !mission_meetCond(misn))
      return 0;

  return 1;
}


/*
 * runs bar missions, all lua side and one-shot
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

         chance = (double)(misn->avail.chance % 100)/100.;

         if (RNGF() < chance) {
            mission_init( &mission, misn, 0 );
            mission_cleanup(&mission); /* it better clean up for itself or we do it */
         }
      }
   }
}


/*
 * marks all active systems that need marking
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


/*
 * links cargo to the mission for posterior cleanup
 */
void mission_linkCargo( Mission* misn, unsigned int cargo_id )
{
   misn->ncargo++;
   misn->cargo = realloc( misn->cargo, sizeof(unsigned int) * misn->ncargo);
   misn->cargo[ misn->ncargo-1 ] = cargo_id;
}


/*
 * unlinks cargo from the mission, removes it from the player
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


/*
 * cleans up a mission
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


/*
 * frees a mission
 */
static void mission_freeData( MissionData* mission )
{
   if (mission->name) free(mission->name);
   if (mission->lua) free(mission->lua);
   if (mission->avail.planet) free(mission->avail.planet);
   if (mission->avail.system) free(mission->avail.system);
   if (mission->avail.factions) free(mission->avail.factions);
   memset( mission, 0, sizeof(MissionData) );
}


/*
 * does mission match faction requirement?
 */
static int mission_matchFaction( MissionData* misn, int faction )
{
   int i;

   for (i=0; i<misn->avail.nfactions; i++) {
      if (faction_isFaction(misn->avail.factions[i]) &&
            (faction == misn->avail.factions[i]))
         return 1;
      else if (faction_ofAlliance( faction, misn->avail.factions[i]))
         return 1;
   }

   return 0;
}


/*
 * generates misisons for the computer - special case
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


/*
 * returns location based on string
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


/*
 * parses a node of a mission
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
            if (xml_isNode(cur,"location"))
               temp->avail.loc = mission_location( xml_get(cur) );
            else if (xml_isNode(cur,"chance"))
               temp->avail.chance = xml_getInt(cur);
            else if (xml_isNode(cur,"planet"))
               temp->avail.planet = strdup( xml_get(cur) );
            else if (xml_isNode(cur,"system"))
               temp->avail.system = strdup( xml_get(cur) );
            else if (xml_isNode(cur,"alliance")) {
               temp->avail.factions = realloc( temp->avail.factions,
                     sizeof(int) * ++temp->avail.nfactions );
               temp->avail.factions[temp->avail.nfactions-1] = 
                     faction_getAlliance( xml_get(cur) );
            }
            else if (xml_isNode(cur,"faction")) {
               temp->avail.factions = realloc( temp->avail.factions, 
                     sizeof(int) * ++temp->avail.nfactions );
               temp->avail.factions[temp->avail.nfactions-1] =
                     faction_get( xml_get(cur) );
            }
            else if (xml_isNode(cur,"cond"))
               temp->avail.cond = strdup( xml_get(cur) );
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


/*
 * load/free
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
void missions_cleanup (void)
{
   int i;

   for (i=0; i<MISSION_MAX; i++)
      mission_cleanup( &player_missions[i] );
}



/*
 * persists partial lua data
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
static int mission_persistData( lua_State *L, xmlTextWriterPtr writer )
{
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

         default:
            break;
      }
      lua_pop(L,1);
      /* key */
   }

   return 0;
}


/* 
 * unpersists lua data
 */
static int mission_unpersistData( lua_State *L, xmlNodePtr parent )
{
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
         else {
            WARN("Unknown lua data type!");
            return -1;
         }

         lua_setglobal(L,name);
         
         /* cleanup */
         free(type);
         free(name);
      }
   } while (xml_nextNode(node));

   return 0;
}
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


