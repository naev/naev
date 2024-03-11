/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nxml_lua.c
 *
 * @brief Handles the saving and writing of a nlua state to XML.
 */
#include "nxml_lua.h"

#include "base64.h"
#include "log.h"
#include "nlua.h"
#include "nlua_commodity.h"
#include "nlua_faction.h"
#include "nlua_jump.h"
#include "nlua_outfit.h"
#include "nlua_spob.h"
#include "nlua_ship.h"
#include "nlua_system.h"
#include "nlua_time.h"
#include "nlua_vec2.h"
#include "mission.h"
#include "utf8.h"

/*
 * Prototypes.
 */
static int nxml_persistDataNode( lua_State *L, xmlTextWriterPtr writer );
static int nxml_unpersistDataNode( lua_State *L, xmlNodePtr parent );
static int nxml_canWriteString( const char *buf, size_t len );

/**
 * @brief Persists the key of a key/value pair.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param name Name of the data to save.
 *    @param name_len Data size of name (which is an arbitrary Lua string).
 *    @param keynum Whether the "name" is a numeric key, to be marked with keynum="1" in the XML.
 *    @return 0 on success.
 */
static int nxml_saveNameAttribute( xmlTextWriterPtr writer, const char *name, size_t name_len, int keynum )
{
   if (nxml_canWriteString( name, name_len ))
      xmlw_attr( writer, "name", "%s", name );
   else {
      char *encoded = base64_encode_to_cstr( name, name_len );
      xmlw_attr( writer, "name_base64", "%s", encoded );
      free( encoded );
   }
   if (keynum)
      xmlw_attr(writer,"keynum","1");
   return 0;
}

/**
 * @brief Persists Lua data.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param type Type of the data to save.
 *    @param name Name of the data to save.
 *    @param name_len Data size of name (which is an arbitrary Lua string).
 *    @param value Value of the data to save.
 *    @param keynum Whether the key is a number (not a string) and should be read back as such.
 *    @return 0 on success.
 */
static int nxml_saveData( xmlTextWriterPtr writer, const char *type, const char *name, size_t name_len,
                          const char *value, int keynum )
{
   xmlw_startElem(writer,"data");

   xmlw_attr(writer,"type","%s",type);
   nxml_saveNameAttribute( writer, name, name_len, keynum );
   xmlw_str(writer,"%s",value);

   xmlw_endElem(writer); /* "data" */

   return 0;
}

/**
 * @brief Commodity-specific nxml_saveData derivative.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param name Name of the data to save.
 *    @param name_len Data size of name (which is an arbitrary Lua string).
 *    @param c Commodity to save.
 *    @param keynum Whether the "name" is a numeric key, to be marked with keynum="1" in the XML.
 *    @return 0 on success.
 */
static int nxml_saveCommodity( xmlTextWriterPtr writer, const char *name, size_t name_len, const Commodity* c, int keynum )
{
   int status = 0;
   if (c->name == NULL)
      return 1;

   xmlw_startElem( writer, "data" );

   xmlw_attr( writer, "type", COMMODITY_METATABLE );
   nxml_saveNameAttribute( writer, name, name_len, keynum );
   if (c->istemp) {
      xmlw_attr( writer, "temp", "%d", c->istemp );
      xmlw_startElem( writer, "commodity" );
      status = missions_saveTempCommodity( writer, c );
      xmlw_endElem( writer ); /* "commodity" */
   }
   else
      xmlw_str( writer, "%s", c->name );
   xmlw_endElem( writer ); /* "data" */
   return status;
}

/**
 * @brief Reverse of nxml_saveCommodity.
 */
static Commodity* nxml_loadCommodity( xmlNodePtr node )
{
   Commodity *c;
   int istemp;

   xmlr_attr_int_def( node, "temp", istemp, 0);
   if (!istemp)
      c = commodity_get( xml_get( node ) );
   else {
      xmlNodePtr cur = node->xmlChildrenNode;
      c = NULL;
      do {
         xml_onlyNodes(cur);
         if ( xml_isNode( cur, "commodity" ) )
            c = missions_loadTempCommodity( cur );
      } while ( xml_nextNode( cur ) );
   }
   return c;
}

/**
 * @brief Jump-specific nxml_saveData derivative.
 *
 *    @param writer XML Writer to use to persist stuff.
 *    @param name Name of the data to save.
 *    @param name_len Data size of name (which is an arbitrary Lua string).
 *    @param start System in which the jump is.
 *    @param dest Jump's destination system.
 *    @param keynum Whether the "name" is a numeric key, to be marked with keynum="1" in the XML.
 *    @return 0 on success.
 */
static int nxml_saveJump( xmlTextWriterPtr writer, const char *name, size_t name_len, const char *start,
                          const char *dest, int keynum )
{
   xmlw_startElem(writer,"data");

   xmlw_attr(writer,"type",JUMP_METATABLE);
   nxml_saveNameAttribute( writer, name, name_len, keynum );
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
 *    @return 0 on success.
 */
static int nxml_persistDataNode( lua_State *L, xmlTextWriterPtr writer )
{
   int ret;
   char buf[32]; /* Buffer large enough for a formatted i64 (base 10). */
   const char *name, *str, *data;
   int keynum;
   size_t len, name_len;

   /* Default values. */
   ret   = 0;

   /* We receive data in the format of: key, value */

   /* key, value */
   /* Handle different types of keys, we must not touch the stack after this operation. */
   switch (lua_type(L, -2)) {
      case LUA_TSTRING:
         /* Can just tostring directly. */
         name = lua_tolstring( L, -2, &name_len );
         /* Isn't a number key. */
         keynum   = 0;
         break;
      case LUA_TNUMBER:
         /* Can't tostring directly. */
         lua_pushvalue(L,-2);
         name = lua_tolstring( L, -1, &name_len );
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
         /* Start the table. */
         xmlw_startElem(writer,"data");
         xmlw_attr(writer,"type","table");
         nxml_saveNameAttribute( writer, name, name_len, keynum );
         lua_pushnil(L); /* key, value, nil */
         while (lua_next(L, -2) != 0) {
            /* key, value, key, value */
            ret |= nxml_persistDataNode( L, writer ); /* pops the value. */
            /* key, value, key */
         }
         /* key, value */
         xmlw_endElem(writer); /* "table" */
         break;

      /* Normal number. */
      case LUA_TNUMBER:
         nxml_saveData( writer, "number", name, name_len, lua_tostring( L, -1 ), keynum );
         /* key, value */
         break;

      /* Boolean is either 1 or 0. */
      case LUA_TBOOLEAN:
         /* lua_tostring doesn't work on booleans. */
         if (lua_toboolean(L,-1)) buf[0] = '1';
         else buf[0] = '0';
         buf[1] = '\0';
         nxml_saveData( writer, "bool", name, name_len, buf, keynum );
         /* key, value */
         break;

      /* String is saved normally. */
      case LUA_TSTRING:
         data = lua_tolstring( L, -1, &len );
         if ( nxml_canWriteString( data, len ) )
            nxml_saveData( writer, "string", name, name_len, lua_tostring( L, -1 ), keynum );
         else {
            char *encoded = base64_encode_to_cstr( data, len );
            nxml_saveData( writer, "string_base64", name, name_len, encoded, keynum );
            free( encoded );
         }
         /* key, value */
         break;

      /* User data must be handled here. */
      case LUA_TUSERDATA:
         if (lua_isspob(L,-1)) {
            Spob *pnt = spob_getIndex( lua_tospob(L,-1) );
            if (pnt != NULL)
               nxml_saveData( writer, SPOB_METATABLE, name, name_len, pnt->name, keynum );
            else
               WARN(_("Failed to save invalid spob."));
            /* key, value */
            break;
         }
         else if (lua_issystem(L,-1)) {
            StarSystem *ss = system_getIndex( lua_tosystem(L,-1) );
            if (ss != NULL)
               nxml_saveData( writer, SYSTEM_METATABLE, name, name_len, ss->name, keynum );
            else
               WARN(_("Failed to save invalid system."));
            /* key, value */
            break;
         }
         else if (lua_isfaction(L,-1)) {
            LuaFaction lf = lua_tofaction(L,-1);
            if (!faction_isFaction(lf)) /* Dynamic factions may become invalid for saving. */
               break;
            str = faction_name( lua_tofaction(L,-1) );
            if (str == NULL)
               break;
            nxml_saveData( writer, FACTION_METATABLE, name, name_len, str, keynum );
            /* key, value */
            break;
         }
         else if (lua_isship(L,-1)) {
            const Ship *sh = lua_toship(L,-1);
            str = sh->name;
            if (str == NULL)
               break;
            nxml_saveData( writer, SHIP_METATABLE, name, name_len, str, keynum );
            /* key, value */
            break;
         }
         else if (lua_istime(L,-1)) {
            ntime_t t = *lua_totime(L,-1);
            snprintf( buf, sizeof(buf), "%"PRId64, t );
            nxml_saveData( writer, TIME_METATABLE, name, name_len, buf, keynum );
            /* key, value */
            break;
         }
         else if (lua_isjump(L,-1)) {
            LuaJump *lj = lua_tojump(L,-1);
            StarSystem *ss = system_getIndex( lj->srcid );
            StarSystem *dest = system_getIndex( lj->destid );
            if ((ss == NULL) || (dest == NULL))
               WARN(_("Failed to save invalid jump."));
            else
               nxml_saveJump( writer, name, name_len, ss->name, dest->name, keynum );
         }
         else if (lua_iscommodity(L,-1)) {
            Commodity *com = lua_tocommodity(L,-1);
            if( nxml_saveCommodity( writer, name, name_len, com, keynum ) != 0)
               WARN( _("Failed to save invalid commodity.") );
            /* key, value */
            break;
         }
         else if (lua_isoutfit(L,-1)) {
            const Outfit *o = lua_tooutfit(L,-1);
            str = o->name;
            if (str == NULL)
               break;
            nxml_saveData( writer, OUTFIT_METATABLE, name, name_len, str, keynum );
            /* key, value */
            break;
         }
         else if (lua_isvector(L,-1)) {
            vec2 *vec = lua_tovector( L, -1 );
            xmlw_startElem( writer, "data" );
            xmlw_attr( writer, "type", VECTOR_METATABLE );
            nxml_saveNameAttribute( writer, name, name_len, keynum );
            xmlw_attr( writer, "x", "%.16e", vec->x );
            xmlw_attr( writer, "y", "%.16e", vec->y );
            xmlw_attr( writer, "mod", "%.16e", vec->mod );
            xmlw_attr( writer, "angle", "%.16e", vec->angle );
            xmlw_endElem( writer );
            /* key, value */
            break;
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
 * All supported contents of the table named "mem" are saved. Functions aren't supported.
 *
 *    @param env Lua environment to save.
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int nxml_persistLua( nlua_env env, xmlTextWriterPtr writer )
{
   int ret = 0;

   nlua_getenv(naevL, env, "mem");

   lua_pushnil(naevL);         /* nil */
   /* str, nil */
   while (lua_next(naevL, -2) != 0) {
      /* key, value */
      ret |= nxml_persistDataNode( naevL, writer );
      /* key */
   }

   lua_pop(naevL, 1);

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
   xmlNodePtr node;
   char *name, *type, *buf, *num, *data;
   size_t len;
   int ret = 0;

   node = parent->xmlChildrenNode;
   do {
      int failed = 0;
      if (xml_isNode(node,"data")) {
         /* Get general info. */
         xmlr_attr_strd(node,"name",name);
         xmlr_attr_strd(node,"type",type);
         /* Check to see if key is a number. */
         xmlr_attr_strd(node,"keynum",num);
         if (num != NULL) {
            lua_pushnumber(L, strtod( name, NULL ));
            free(num);
         }
         else if ( name != NULL )
            lua_pushstring(L, name);
         else {
            xmlr_attr_strd( node, "name_base64", name );
            data = base64_decode_cstr( &len, name );
            lua_pushlstring( L, data, len );
            free( data );
         }

         /* handle data types */
         /* Recursive tables. */
         if (strcmp(type,"table")==0) {
            xmlr_attr_strd(node,"name",buf);
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
         else if (strcmp( type, "string_base64" ) == 0) {
            data = base64_decode_cstr( &len, xml_get( node ) );
            lua_pushlstring( L, data, len );
            free( data );
         }
         else if (strcmp(type,SPOB_METATABLE)==0) {
            Spob *pnt = spob_get(xml_get(node));
            if (pnt != NULL) {
               lua_pushspob(L,spob_index(pnt));
            }
            else {
               WARN(_("Failed to load nonexistent spob '%s'"), xml_get(node));
               failed = 1;
            }
         }
         else if (strcmp(type,SYSTEM_METATABLE)==0) {
            StarSystem *ss = system_get(xml_get(node));
            if (ss != NULL)
               lua_pushsystem(L,system_index( ss ));
            else {
               WARN(_("Failed to load nonexistent system '%s'"), xml_get(node));
               failed = 1;
            }
         }
         else if (strcmp(type,FACTION_METATABLE)==0) {
            lua_pushfaction(L,faction_get(xml_get(node)));
         }
         else if (strcmp(type,SHIP_METATABLE)==0)
            lua_pushship(L,ship_get(xml_get(node)));
         else if (strcmp(type,TIME_METATABLE)==0) {
            lua_pushtime(L,xml_getLong(node));
         }
         else if (strcmp(type,JUMP_METATABLE)==0) {
            StarSystem *ss = system_get(xml_get(node));
            xmlr_attr_strd(node,"dest",buf);
            StarSystem *dest = system_get( buf );
            if ((ss != NULL) && (dest != NULL)) {
               LuaJump lj = {.srcid = ss->id, .destid = dest->id};
               lua_pushjump(L,lj);
            }
            else {
               WARN(_("Failed to load nonexistent jump from '%s' to '%s'"), xml_get(node), buf);
               failed = 1;
            }
            free(buf);
         }
         else if (strcmp(type,COMMODITY_METATABLE)==0)
            lua_pushcommodity(L, nxml_loadCommodity( node ) );
         else if (strcmp(type,OUTFIT_METATABLE)==0)
            lua_pushoutfit(L,outfit_get(xml_get(node)));
         else if (strcmp(type, VECTOR_METATABLE)==0) {
            vec2 vec;
            xmlr_attr_float( node, "x", vec.x );
            xmlr_attr_float( node, "y", vec.y );
            xmlr_attr_float( node, "mod", vec.mod );
            xmlr_attr_float( node, "angle", vec.angle );
            lua_pushvector( L, vec );
         }
         else {
            /* There are a few types knowingly left out above. Quoting the lua_{to,push} methods, as of 2021-11-13, they are:
             * article, audio, canvas, colour, data, file, font, linopt, pilot, pilotoutfit, shader, tex, transform.
             * */
            WARN(_("Unknown Lua data type!"));
            failed = 1;
         }

         /* Set field. */
         if (!failed)
            lua_settable(L, -3);
         else
            lua_pop(L,1);

         /* cleanup */
         free(type);
         free(name);

         ret |= failed;
      }
   } while (xml_nextNode(node));

   return ret;
}

/**
 * @brief Unpersists Lua data into a table named "mem".
 *
 *    @param env Environment to unpersist data into.
 *    @param parent Node containing all the Lua persisted data.
 *    @return 0 on success.
 */
int nxml_unpersistLua( nlua_env env, xmlNodePtr parent )
{
   int ret;

   nlua_getenv(naevL, env, "mem");
   ret = nxml_unpersistDataNode(naevL,parent);
   lua_pop(naevL,1);

   return ret;
}

/**
 * @brief Checks whether saving the given string (from lua_tolstring)
 *        can be saved into an XML document without blowing up.
 *
 *    @param buf Contents of a valid Lua string.
 *    @param len Corresponding length.
 *    @return 1 if OK, 0 if it won't work.
 */
static int nxml_canWriteString( const char *buf, size_t len )
{
   for (size_t i = 0; i < len; i++) {
      if ( buf[ i ] == '\0'
           || ( buf[ i ] < 0x20 && buf[ i ] != '\t' && buf[ i ] != '\n' && buf[ i ] != '\r' ) )
         return 0;
   }
   return u8_isvalid( buf, len );
}
