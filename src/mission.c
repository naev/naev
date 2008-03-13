/*
 * See Licensing and Copyright notice in naev.h
 */


#include "mission.h"

#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "pluto.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "rng.h"
#include "naev.h"
#include "log.h"
#include "hook.h"
#include "pack.h"
#include "xml.h"
#include "faction.h"
#include "player.h"
#include "base64.h"


#define XML_MISSION_ID        "Missions"   /* XML section identifier */
#define XML_MISSION_TAG       "mission"

#define MISSION_DATA          "dat/mission.xml"
#define MISSION_LUA_PATH      "dat/missions/"


/* L state, void* buf, int n size, char* s identifier */
#define luaL_dobuffer(L, b, n, s) \
   (luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))


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
 * prototypes
 */
/* extern */
extern int misn_run( Mission *misn, char *func );
/* static */
static unsigned int mission_genID (void);
static int mission_init( Mission* mission, MissionData* misn );
static void mission_freeData( MissionData* mission );
static int mission_alreadyRunning( MissionData* misn );
static int mission_meetReq( int mission, int faction, char* planet, char* system );
static int mission_matchFaction( MissionData* misn, int faction );
static int mission_location( char* loc );
static MissionData* mission_parse( const xmlNodePtr parent );
static int missions_parseActive( xmlNodePtr parent );
static int mission_persistTable( lua_State *L );
static int mission_unpersistTable( lua_State *L );


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
static int mission_init( Mission* mission, MissionData* misn )
{
   char *buf;
   uint32_t bufsize;

   if (misn != NULL) {  /* don't set the data either */
      mission->id = mission_genID();
      mission->data = misn;
   }

   /* sane defaults */
   mission->title = NULL;
   mission->desc = NULL;
   mission->reward = NULL;
   mission->cargo = NULL;
   mission->ncargo = 0;

   /* init lua */
   mission->L = luaL_newstate();
   if (mission->L == NULL) {
      ERR("Unable to create a new lua state.");
      return -1;
   }
   /* luaopen_base( mission->L ); *//* can be useful */
   luaopen_string( mission->L ); /* string.format can be very useful */
   misn_loadLibs( mission->L ); /* load our custom libraries */

   if (misn != NULL) { /* don't load it with data */
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
      misn_run( mission, "create");
   }

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
 * does the mission meet the minimum requirements?
 */
static int mission_meetReq( int mission, int faction, char* planet, char* system )
{
   MissionData* misn;

   misn = &mission_stack[mission];

   /* must match planet, system or faction */
   if (!(((misn->avail.planet && strcmp(misn->avail.planet,planet)==0)) ||
         (misn->avail.system && (strcmp(misn->avail.system,system)==0)) ||
         mission_matchFaction(misn,faction)))
      return 0;

   if (mis_isFlag(misn,MISSION_UNIQUE) && /* mission done or running */
         (player_missionAlreadyDone(mission) ||
          mission_alreadyRunning(misn)))
      return 0;

   if ((misn->avail.req != NULL) && /* mission doesn't meet requirement */
         !var_checkflag(misn->avail.req))
      return 0;

  return 1;
}


/*
 * runs bar missions, all lua side and one-shot
 */
void missions_bar( int faction, char* planet, char* system )
{
   MissionData* misn;
   Mission mission;
   int i;
   double chance;

   for (i=0; i<mission_nstack; i++) {
      misn = &mission_stack[i];
      if (misn->avail.loc==MIS_AVAIL_BAR) {

         if (!mission_meetReq(i, faction, planet, system))
            continue;

         chance = (double)(misn->avail.chance % 100)/100.;

         if (RNGF() < chance) {
            mission_init( &mission, misn );
            mission_cleanup(&mission); /* it better clean up for itself or we do it */
         }
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
   if (misn->reward) {
      free(misn->reward);
      misn->reward = NULL;
   }
   if (misn->cargo) {
      for (i=0; i<misn->ncargo; i++)
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
Mission* missions_computer( int *n, int faction, char* planet, char* system )
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

         if (!mission_meetReq(i, faction, planet, system))
            continue;

         chance = (double)(misn->avail.chance % 100)/100.;
         rep = misn->avail.chance / 100;

         for (j=0; j<rep; j++) /* random chance of rep appearances */
            if (RNGF() < chance) {
               tmp = realloc( tmp, sizeof(Mission) * ++m);
               mission_init( &tmp[m-1], misn );
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
            else if (xml_isNode(cur,"req"))
               temp->avail.req = strdup( xml_get(cur) );
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
}
void missions_cleanup (void)
{
   int i;

   for (i=0; i<MISSION_MAX; i++)
      mission_cleanup( &player_missions[i] );
}


/*
 * memory buffer structure to handle lua writers/reades
 */
typedef struct MBuf_ {
   char *data;
   ssize_t len, alloc; /* size of each data chunk, chunks to alloc when growing */
   size_t ndata, mdata; /* buffer real length, memory length */
} MBuf;
MBuf* mbuf_create( int len, int alloc )
{
   MBuf* buf;

   buf = malloc(sizeof(MBuf));
   if (buf == NULL) {
      WARN("Out of memory");
      return NULL;
   }

   buf->data = 0;
   buf->ndata = buf->mdata = 0;

   buf->len = len;
   buf->alloc = alloc;

   return buf;
}
void mbuf_free( MBuf *buf )
{
   if (buf->data != NULL) free( buf->data );
   buf->ndata = buf->mdata = 0;
   free(buf);
}
static int mission_writeLua( lua_State *L , const void *p, size_t sz, void* ud )
{
   int i;
   MBuf *buf;
   (void)L;

   buf = (MBuf*)ud;

   i = buf->ndata*buf->len + sz - buf->mdata*buf->len;
   if (i > 0) { /* need more memory */
      buf->mdata += (i / (buf->len*buf->alloc) + 1) * buf->len*buf->alloc;
      buf->data = realloc( buf->data, buf->mdata*buf->len );
   }

   memcpy( &buf->data[buf->ndata*buf->len], p, sz );
   buf->ndata += sz;

   return 0;
}
static int mission_persistTable( lua_State *L )
{
   lua_newtable(L);
   /* table */
   lua_pushnil(L);
   /* table, nil */
   while (lua_next(L, LUA_GLOBALSINDEX) != 0) {
      /* table, key, value */
      switch (lua_type(L, -1)) {
         case LUA_TNUMBER:
         case LUA_TBOOLEAN:
         case LUA_TSTRING:
            lua_pushvalue(L, -2); /* copy key */
            /* table, key, value, key */
            lua_insert(L, -3); /* key << 2 */
            /* table, key, key, value */
            lua_settable(L, -4); /* table[key] = value */
            /* table, key */
            break;

         default:
            lua_pop(L,1);
            /* table, key */
            continue;
      }
   }
   /* table */

   return 0;
}
static int mission_unpersistTable( lua_State *L )
{
   /* table */
   lua_pushnil(L);
   /* table, nil */
   while (lua_next(L, -2) != 0) {
      /* table, key, value */
      lua_pushvalue(L,-2); /* copy key */
      /* table, key, value, key */
      lua_insert(L, -3); /* key << 2 */
      /* table, key, key, value */
      lua_settable(L, LUA_GLOBALSINDEX);
      /* table, key */
   }
   /* table */
   lua_pop(L,1);
   /* */

   return 0;
}
int missions_saveActive( xmlTextWriterPtr writer )
{
   MBuf *buf;
   char *data;
   int i,j;
   size_t sz;

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

         xmlw_startElem(writer,"cargos");
         for (j=0; j<player_missions[i].ncargo; j++)
            xmlw_elem(writer,"cargo","%u", player_missions[i].cargo[j]);
         xmlw_endElem(writer); /* "cargos" */

         /* write lua magic */
         xmlw_startElem(writer,"lua");

         /* we need to use a special data struct */
         buf = mbuf_create(1,128);

         /* prepare the data */
         lua_pushnil(player_missions[i].L);
         mission_persistTable(player_missions[i].L); /* we don't save it all */
         pluto_persist( player_missions[i].L, mission_writeLua, buf );

         /* now process it to save it */
         data = base64_encode( &sz, buf->data, buf->ndata );
         xmlw_raw(writer,data,sz);

         /* cleanup */
         mbuf_free(buf);
         free(data);

         xmlw_endElem(writer); /* "lua" */

         xmlw_endElem(writer); /* "mission" */
      }
   }

   xmlw_endElem(writer); /* "missions" */

   return 0;
}
const char* mission_readLua( lua_State *L, void *data, size_t *size )
{
   (void)L;
   MBuf *dat;
   char *pos;
   char *buf;
   size_t len;

   dat = (MBuf*)data;
   len = dat->alloc * dat->len;
   pos = dat->data;

   buf = &pos[dat->ndata];
   if (dat->ndata >= dat->mdata) { /* end of stream */
      (*size) = 0;
      return NULL;
   }
   if (dat->mdata < (dat->ndata + len)) { /* last chunk */
      (*size) = dat->mdata - dat->ndata;
      dat->ndata = dat->mdata;
   }
   else {
      (*size) = len;
      dat->ndata += len;
   }
   return buf;
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
   char *buf, *str;
   MBuf dat;

   xmlNodePtr node, cur, nest;

   m = 0; /* start with mission 0 */
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"mission")) {
         misn = &player_missions[m];

         /* process the attributes to create the mission */
         xmlr_attr(node,"data",buf);
         mission_init( misn, mission_get(mission_getID(buf)) );
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

            if (xml_isNode(cur,"cargos")) {
               nest = cur->xmlChildrenNode;
               do {
                  if (xml_isNode(nest,"cargo"))
                     mission_linkCargo( misn, xml_getLong(nest) );
               } while (xml_nextNode(nest));
            }

            if (xml_isNode(cur,"lua")) {

               /* prepare the data */
               str = xml_get(cur);
               dat.ndata = 0;
               dat.len = 1024;
               dat.alloc = 128;
               dat.data = base64_decode( &dat.mdata, str, strlen(str) );

               /* start the unpersist routine */
               lua_pushnil(misn->L);
               pluto_unpersist( misn->L, mission_readLua, &dat );
               mission_unpersistTable(misn->L);

               /* cleanup */
               free(dat.data);
            }
         } while (xml_nextNode(cur));



         m++; /* next mission */
         if (m >= MISSION_MAX) break; /* full of missions, must be an error */
      }
   } while (xml_nextNode(node));

   return 0;
}


