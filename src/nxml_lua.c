/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nxml_lua.c
 *
 * @brief Handles the saving and writing of a nlua state to XML.
 */


#include "nxml_lua.h"

#include "naev.h"

#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_space.h"
#include "nlua_faction.h"
#include "nlua_ship.h"
#include "nlua_time.h"
#include "nstring.h"


/*
 * Prototypes.
 */
static int nxml_persistDataNode( lua_State *L, xmlTextWriterPtr writer, int intable );
static int nxml_unpersistDataNode( lua_State *L, xmlNodePtr parent );

/**
 * @brief Persists Lua data.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param type Type of the data to save.
 *    @param name Name of the data to save.
 *    @param value Value of the data to save.
 *    @return 0 on success.
 */
static int nxml_saveData( xmlTextWriterPtr writer,
      const char *type, const char *name, const char *value,
      int keynum )
{
   xmlw_startElem(writer,"data");

   xmlw_attr(writer,"type","%s",type);
   xmlw_attr(writer,"name","%s",name);
   if (keynum)
      xmlw_attr(writer,"keynum","1");
   xmlw_str(writer,"%s",value);

   xmlw_endElem(writer); /* "data" */

   return 0;
}


/**
 * @brief Jump-specific nxml_saveData derivative.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param name Name of the data to save.
 *    @param start System in which the jump is.
 *    @param dest Jump's destination system.
 *    @return 0 on success.
 */
static int nxml_saveJump( xmlTextWriterPtr writer,
      const char *name, const char *start, const char *dest )
{
   xmlw_startElem(writer,"data");

   xmlw_attr(writer,"type","jump");
   xmlw_attr(writer,"name","%s",name);
   xmlw_attr(writer,"dest","%s",dest);
   xmlw_str(writer,"%s",start);

   xmlw_endElem(writer); /* "data" */

   return 0;
}


/**
 * @brief Persists the node on the top of the stack and pops it.
 *
 *    @param L Lua state with node to persist on top of the stack.
 *    @param writer XML Writer to use.
 *    @param Are we parsing a node in a table?  Avoids checking for extra __save.
 *    @return 0 on success.
 */
static int nxml_persistDataNode( lua_State *L, xmlTextWriterPtr writer, int intable )
{
   int ret, b;
   LuaPlanet *p;
   LuaSystem *s;
   LuaFaction *f;
   LuaShip *sh;
   LuaTime *lt;
   LuaJump *lj;
   Planet *pnt;
   StarSystem *ss, *dest;
   char buf[PATH_MAX];
   const char *name, *str;
   int keynum;

   /* Default values. */
   ret   = 0;

   /* We receive data in the format of: key, value */

   /* key, value */
   /* Handle different types of keys, we must not touch the stack after this operation. */
   switch (lua_type(L, -2)) {
      case LUA_TSTRING:
         /* Can just tostring directly. */
         name     = lua_tostring(L,-2);
         /* Isn't a number key. */
         keynum   = 0;
         break;
      case LUA_TNUMBER:
         /* Can't tostring directly. */
         lua_pushvalue(L,-2);
         name     = lua_tostring(L,-1);
         lua_pop(L,1); /* Pop the new value. */
         /* Is a number key. */
         keynum   = 1;
         break;

      /* We only handle string or number keys, so ignore the rest. */
      default:
         lua_pop(L,1); /* key. */
         return 0;
   }
   /* key, value */

   /* Now handle the value. */
   switch (lua_type(L, -1)) {
      /* Recursive for tables. */
      case LUA_TTABLE:
         /* Check if should save -- only if not in table.. */
         if (!intable) {
            lua_getfield(L, -1, "__save"); /* key, value, field */
            b = lua_toboolean(L,-1);
            lua_pop(L,1); /* key, value */
            if (!b) /* No need to save. */
               break;
         }
         /* Start the table. */
         xmlw_startElem(writer,"data");
         xmlw_attr(writer,"type","table");
         xmlw_attr(writer,"name","%s",name);
         if (keynum)
            xmlw_attr(writer,"keynum","1");
         lua_pushnil(L); /* key, value, nil */
         while (lua_next(L, -2) != 0) {
            /* key, value, key, value */
            ret |= nxml_persistDataNode( L, writer, 1 ); /* pops the value. */
            /* key, value, key */
         }
         /* key, value */
         xmlw_endElem(writer); /* "table" */
         break;

      /* Normal number. */
      case LUA_TNUMBER:
         nxml_saveData( writer, "number",
               name, lua_tostring(L,-1), keynum );
         /* key, value */
         break;

      /* Boolean is either 1 or 0. */
      case LUA_TBOOLEAN:
         /* lua_tostring doesn't work on booleans. */
         if (lua_toboolean(L,-1)) buf[0] = '1';
         else buf[0] = '0';
         buf[1] = '\0';
         nxml_saveData( writer, "bool",
               name, buf, keynum );
         /* key, value */
         break;

      /* String is saved normally. */
      case LUA_TSTRING:
         nxml_saveData( writer, "string",
               name, lua_tostring(L,-1), keynum );
         /* key, value */
         break;

      /* User data must be handled here. */
      case LUA_TUSERDATA:
         if (lua_isplanet(L,-1)) {
            p = lua_toplanet(L,-1);
            pnt = planet_getIndex( p->id );
            if (pnt != NULL)
               nxml_saveData( writer, "planet",
                     name, pnt->name, keynum );
            else
               WARN("Failed to save invalid planet.");
            /* key, value */
            break;
         }
         else if (lua_issystem(L,-1)) {
            s  = lua_tosystem(L,-1);
            ss = system_getIndex( s->id );
            if (ss != NULL)
               nxml_saveData( writer, "system",
                     name, ss->name, keynum );
            else
               WARN("Failed to save invalid system.");
            /* key, value */
            break;
         }
         else if (lua_isfaction(L,-1)) {
            f = lua_tofaction(L,-1);
            str = faction_name( f->f );
            if (str == NULL)
               break;
            nxml_saveData( writer, "faction",
                  name, str, keynum );
            /* key, value */
            break;
         }
         else if (lua_isship(L,-1)) {
            sh = lua_toship(L,-1);
            str = sh->ship->name;
            if (str == NULL)
               break;
            nxml_saveData( writer, "ship",
                  name, str, keynum );
            /* key, value */
            break;
         }
         else if (lua_istime(L,-1)) {
            lt = lua_totime(L,-1);
            nsnprintf( buf, sizeof(buf), "%"PRId64, lt->t );
            nxml_saveData( writer, "time",
                  name, buf, keynum );
            /* key, value */
            break;
         }
         else if (lua_isjump(L,-1)) {
            lj = lua_tojump(L,-1);
            ss = system_getIndex( lj->srcid );
            dest = system_getIndex( lj->destid );
            if ((ss == NULL) || (dest == NULL))
               WARN("Failed to save invalid jump.");
            else
               nxml_saveJump( writer, name, ss->name, dest->name );
         }
         /* Purpose fallthrough. */

      /* Rest gets ignored, like functions, etc... */
      default:
         /* key, value */
         break;
   }
   lua_pop(L,1); /* key */

   /* We must pop the value and leave only the key so it can continue iterating. */

   return ret;
}


/**
 * @brief Persists all the nxml Lua data.
 *
 * Does not save anything in tables nor functions of any type.
 *
 *    @param L Lua state to save.
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int nxml_persistLua( lua_State *L, xmlTextWriterPtr writer )
{
   int ret = 0;

   lua_pushnil(L);         /* nil */
   /* str, nil */
   while (lua_next(L, LUA_GLOBALSINDEX) != 0) {
      /* key, value */
      ret |= nxml_persistDataNode( L, writer, 0 );
      /* key */
   }

   return ret;
}


/**
 * @brief Unpersists Lua data.
 *
 *    @param L State to unpersist data into.
 *    @param parent Node containing all the Lua persisted data.
 *    @return 0 on success.
 */
static int nxml_unpersistDataNode( lua_State *L, xmlNodePtr parent )
{
   LuaPlanet p;
   LuaSystem s;
   LuaFaction f;
   LuaShip sh;
   LuaTime lt;
   LuaJump lj;
   Planet *pnt;
   StarSystem *ss, *dest;
   xmlNodePtr node;
   char *name, *type, *buf, *num;

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"data")) {
         /* Get general info. */
         xmlr_attr(node,"name",name);
         xmlr_attr(node,"type",type);
         /* Check to see if key is a number. */
         xmlr_attr(node,"keynum",num);
         if (num != NULL) {
            lua_pushnumber(L, atof(name));
            free(num);
         }
         else
            lua_pushstring(L, name);

         /* handle data types */
         /* Recursive tables. */
         if (strcmp(type,"table")==0) {
            xmlr_attr(node,"name",buf);
            /* Create new table. */
            lua_newtable(L);
            /* Save data. */
            nxml_unpersistDataNode(L,node);
            /* Set table. */
            free(buf);
         }
         else if (strcmp(type,"number")==0)
            lua_pushnumber(L,xml_getFloat(node));
         else if (strcmp(type,"bool")==0)
            lua_pushboolean(L,xml_getInt(node));
         else if (strcmp(type,"string")==0)
            lua_pushstring(L,xml_get(node));
         else if (strcmp(type,"planet")==0) {
            pnt = planet_get(xml_get(node));
            if (pnt != NULL) {
               p.id = planet_index(pnt);
               lua_pushplanet(L,p);
            }
            else
               WARN("Failed to load unexistent planet '%s'", xml_get(node));
         }
         else if (strcmp(type,"system")==0) {
            ss = system_get(xml_get(node));
            if (ss != NULL) {
               s.id = system_index( ss );
               lua_pushsystem(L,s);
            }
            else
               WARN("Failed to load unexistent system '%s'", xml_get(node));
         }
         else if (strcmp(type,"faction")==0) {
            f.f = faction_get(xml_get(node));
            lua_pushfaction(L,f);
         }
         else if (strcmp(type,"ship")==0) {
            sh.ship = ship_get(xml_get(node));
            lua_pushship(L,sh);
         }
         else if (strcmp(type,"time")==0) {
            lt.t = xml_getLong(node);
            lua_pushtime(L,lt);
         }
         else if (strcmp(type,"jump")==0) {
            ss = system_get(xml_get(node));
            system_get(xmlr_attr(node,"dest",buf));
            dest = system_get( buf );
            if ((ss != NULL) && (dest != NULL)) {
               lj.srcid = ss->id;
               lj.destid = dest->id;
               lua_pushjump(L,lj);
            }
            else
               WARN("Failed to load unexistent jump from '%s' to '%s'", xml_get(node), buf);
         }
         else {
            WARN("Unknown Lua data type!");
            lua_pop(L,1);
            return -1;
         }

         /* Set field. */
         lua_settable(L, -3);

         /* cleanup */
         free(type);
         free(name);
      }
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Unpersists Lua data.
 *
 *    @param L State to unpersist data into.
 *    @param parent Node containing all the Lua persisted data.
 *    @return 0 on success.
 */
int nxml_unpersistLua( lua_State *L, xmlNodePtr parent )
{
   int ret;

   lua_pushvalue(L,LUA_GLOBALSINDEX);
   ret = nxml_unpersistDataNode(L,parent);
   lua_pop(L,1);

   return ret;
}
