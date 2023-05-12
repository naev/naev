/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_news.c
 *
 * @brief Lua news module.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_news.h"

#include "array.h"
#include "land.h"
#include "nlua_time.h"
#include "nluadef.h"
#include "nstring.h"
#include "ntime.h"

extern news_t *news_list; /**< Linked list containing all newss */
extern int land_loaded;

news_t* luaL_validnews( lua_State *L, int ind );
int lua_isnews( lua_State *L, int ind );
LuaNews_t* lua_pushnews( lua_State *L, LuaNews_t news );

static int newsL_add( lua_State *L );
static int newsL_rm( lua_State *L );
static int newsL_get( lua_State *L );
static int newsL_eq( lua_State *L );
static int newsL_title( lua_State *L );
static int newsL_desc( lua_State *L );
static int newsL_faction( lua_State *L );
static int newsL_date( lua_State *L );
static int newsL_bind( lua_State *L );
static const luaL_Reg news_methods[] = {
   {"add", newsL_add},
   {"rm", newsL_rm},
   {"get", newsL_get},
   {"title", newsL_title},
   {"desc", newsL_desc},
   {"faction", newsL_faction},
   {"date", newsL_date},
   {"bind", newsL_bind},
   {"__eq", newsL_eq},
   {0, 0}
}; /**< News metatable methods. */

/**
 * @brief Loads the news library.
 *
 *    @param env Environment to load news library into.
 *    @return 0 on success.
 */
int nlua_loadNews( nlua_env env )
{
   nlua_register(env, NEWS_METATABLE, news_methods, 1);
   return 0; /* No error */
}

/**
 * @brief Gets news at index.
 *
 *    @param L Lua state to get news from.
 *    @param ind Index position to find the news.
 *    @return News found at the index in the state.
 */
LuaNews_t* lua_tonews( lua_State *L, int ind )
{
   return (LuaNews_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets news at index or raises error if there is no news at index.
 *
 *    @param L Lua state to get news from.
 *    @param ind Index position to find news.
 *    @return News found at the index in the state.
 */
LuaNews_t* luaL_checknews( lua_State *L, int ind )
{
   if (lua_isnews(L,ind))
      return lua_tonews(L,ind);
   luaL_typerror(L, ind, NEWS_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a news on the stack.
 *
 *    @param L Lua state to push news into.
 *    @param news News to push.
 *    @return Newly pushed news.
 */
LuaNews_t* lua_pushnews( lua_State *L, LuaNews_t news )
{
   LuaNews_t *la = (LuaNews_t*) lua_newuserdata(L, sizeof(LuaNews_t));
   *la = news;
   luaL_getmetatable(L, NEWS_METATABLE);
   lua_setmetatable(L, -2);
   return la;
}
/**
 * @brief Checks to see if ind is a news.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a news.
 */
int lua_isnews( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, NEWS_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Makes sure the news is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the news to validate.
 *    @return The news (doesn't return if fails - raises Lua error ).
 */
news_t* luaL_validnews( lua_State *L, int ind )
{
   LuaNews_t *ln = luaL_checknews( L, ind );
   news_t *n = news_get( *ln );
   if (n==NULL)
      NLUA_ERROR(L, _("Article is invalid."));
   return n;
}

/**
 * @brief Lua bindings to interact with the news.
 *
 * This will allow you to interact and manipulate the in-game news.
 *
 * @luamod news
 */
/**
 * @brief Adds a news article.
 * @usage news.add(faction,title,body,[date_to_rm, [date]])
 *
 * @usage s = news.add( "Empire", "Hello world!", "The Empire wishes to say hello!", 0 ) -- Adds an Empire specific article, with date 0.
 * @usage s = news.add( { { faction = "Empire", title = "Hello World!", body = "The Empire wishes to say hello!" } } ) -- Can also be passed as tables
 *
 *    @luatparam Faction|string faction of the article, "Generic" for non-factional
 *    @luatparam string title Title of the article
 *    @luatparam string body What's in the article
 *    @luatparam[opt] number|Time date_to_rm date to remove the article
 *    @luatparam[opt] number|Time date What time to put, defaults to current date, use 0 to not use a date
 *    @luatparam[opt=5] number Priority to use. Lower is more important and will appear first.
 *    @luatreturn Article The article matching name or nil if error.
 * @luafunc add
 */
int newsL_add( lua_State *L )
{
   const char *title, *body, *faction;
   ntime_t date, date_to_rm;
   int priority;

   title    = NULL;
   body     = NULL;
   faction  = NULL;
   priority = 5;

   date = ntime_get();
   date_to_rm = NEWS_FOREVER;

   /* If a table is passed in. ugly hack */
   if (lua_istable(L, 1)) {
      lua_pushnil(L);

      /* traverse table */
      while (lua_next(L, -2)) {
         /* traverse sub table */
         if (lua_istable(L, -1)) {
            faction  = title = body = NULL;
            priority    = 5;
            date        = ntime_get();
            date_to_rm  = NEWS_FOREVER;

            lua_getfield( L, -1, "faction" );
            if (!lua_isnil(L,-1))
               faction = luaL_checkstring(L,-1);
            lua_pop(L,1);

            lua_getfield( L, -1, "head" );
            if (!lua_isnil(L,-1))
               title = luaL_checkstring(L,-1);
            lua_pop(L,1);

            lua_getfield( L, -1, "body" );
            if (!lua_isnil(L,-1))
               body = luaL_checkstring(L,-1);
            lua_pop(L,1);

            lua_getfield( L, -1, "date" );
            if (!lua_isnil(L,-1)) {
               if (lua_isnumber(L,-1))
                  date = lua_tonumber(L,-1);
               else
                  date = luaL_validtime(L,-1);
            }
            lua_pop(L,1);

            lua_getfield( L, -1, "date_to_rm" );
            if (!lua_isnil(L,-1)) {
               if (lua_isnumber(L,-1))
                  date_to_rm = lua_tonumber(L,-1);
               else
                  date_to_rm = luaL_validtime(L,-1);
            }
            lua_pop(L,1);

            lua_getfield( L, -1, "priority" );
            if (!lua_isnil(L,-1))
               priority = luaL_checkinteger(L,-1);
            lua_pop(L,1);

            if (title && body && faction)
               news_add( title, body, faction, NULL, date, date_to_rm, priority );
            else
               WARN(_("Bad arguments"));
         }
         lua_pop(L, 1);
      }
      lua_pop(L, 1);

      /* If we're landed, we should regenerate the news buffer. */
      if (landed) {
         generate_news( land_spob->presence.faction );
         if (land_loaded)
            bar_regen();
      }

      return 0;
   }

   if (!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3))) {
      WARN(_("\nBad arguments, use "
           "addArticle(\"Faction\",\"Title\",\"Content\",[date,[date_to_rm]])"));
      return 0;
   }

   faction = luaL_checkstring(L, 1);
   title   = luaL_checkstring(L, 2);
   body = luaL_checkstring(L, 3);
   priority = luaL_optinteger(L, 6, 5);

   /* Get date and date to remove, or leave at defaults. */
   if (!lua_isnoneornil(L,4)) {
      if (lua_istime(L, 4))
         date_to_rm = luaL_validtime(L, 4);
      else
         date_to_rm = luaL_checklong(L, 4);
   }
   if (!lua_isnoneornil(L,5)) {
      if (lua_istime(L, 5))
         date = luaL_validtime(L, 5);
      else
         date = luaL_checklong(L, 5);
   }

   if (title && body && faction) {
      LuaNews_t n_article = news_add( title, body, faction, NULL, date, date_to_rm, priority );
      lua_pushnews( L, n_article );
   }
   else
      NLUA_ERROR(L,_("Bad arguments"));

   /* If we're landed, we should regenerate the news buffer. */
   if (landed) {
      generate_news(land_spob->presence.faction);
      if (land_loaded)
         bar_regen();
   }

   return 1;
}

/**
 * @brief Frees a news article or a table of articles.
 *    @luatparam News n News article to free.
 * @luafunc rm
 */
int newsL_rm( lua_State *L )
{
   if (lua_istable(L, 1)) {
      lua_pushnil(L);
      while (lua_next(L, -2)) {
         LuaNews_t *Larticle = luaL_checknews(L, -1);
         news_rm( *Larticle );
         lua_pop(L, 1);
      }
   }
   else {
      LuaNews_t *Larticle = luaL_checknews(L, 1);
      news_rm( *Larticle );
   }

   /* If we're landed, we should regenerate the news buffer. */
   if (landed) {
      generate_news(land_spob->presence.faction);
      if (land_loaded)
         bar_regen();
   }

   return 1;
}

/**
 * @brief Gets all matching news articles in a table.
 *
 * characteristic can be any of the following:<br />
 * <ul>
 *    <li>Title of the news articles</li>
 *    <li>Body (text) of the articles</li>
 *    <li>Faction name of the articles ("Generic" for generic)</li>
 *    <li>Tag of the articles (applied with news.bind())</li>
 *    <li>Date of the articles in number form</li>
 * </ul>
 * <br />
 * The returned table is populated with all articles matching the
 *  specified characteristic.
 *
 *    @luatparam[opt] number|String characteristic characteristic to match, or no parameter for all articles
 *    @luatreturn {Article,...} a table with matching articles
 * @luafunc get
 */
int newsL_get( lua_State *L )
{
   ntime_t date  = -1;
   const char *characteristic = NULL;
   int print_all = 0;
   int narticle = 1;

   if (lua_isnoneornil(L, 1)) /* Case no argument */
      print_all = 1;
   else if (lua_isnumber(L, 1))
      date = (ntime_t)lua_tonumber(L, 1);
   else if (lua_isstring(L, 1))
      characteristic = lua_tostring(L, 1);
   else
      NLUA_INVALID_PARAMETER(L); /* Bad Parameter */

   /* Now put all the matching articles in a table. */
   lua_newtable(L);
   for (int i=0; i<array_size(news_list); i++) {
      news_t *n = &news_list[i];

      if ((n->title == NULL) || (n->desc == NULL) || (n->faction == NULL))
         continue;

      if (print_all || date == n->date
           || (characteristic
                && ((strcmp( n->title, characteristic ) == 0)
                     || (strcmp( n->desc, characteristic ) == 0)
                     || (strcmp( n->faction, characteristic ) == 0)
                     || (n->tag != NULL && strcmp( n->tag, characteristic ) == 0 )))) {
         lua_pushnews(L, n->id); /* value */
         lua_rawseti(L, -2, narticle++);
      }
   }

   return 1;
}

/**
 * @brief Check news articles for equality.
 *
 * Allows you to use the '==' operator in Lua with articles.
 *
 *    @luatparam Article a1 article 1
 *    @luatparam Article a2 article 2
 *    @luatreturn boolean true if both systems are the same.
 * @luafunc __eq
 */
int newsL_eq( lua_State *L )
{
   const LuaNews_t *a1, *a2;
   a1 = luaL_checknews(L, 1);
   a2 = luaL_checknews(L, 2);
   lua_pushboolean(L, *a1 == *a2);
   return 1;
}

/**
 * @brief Gets the news article title.
 *    @luatparam Article a article to get the title of
 *    @luatreturn string title
 * @luafunc title
 */
int newsL_title( lua_State *L )
{
   news_t *article_ptr = luaL_validnews(L, 1);
   lua_pushstring(L, article_ptr->title);
   return 1;
}

/**
 * @brief Gets the news article description.
 *    @luatparam Article a article to get the desc of
 *    @luatreturn string desc
 * @luafunc desc
 */
int newsL_desc( lua_State *L )
{
   news_t *article_ptr = luaL_validnews(L, 1);
   lua_pushstring(L, article_ptr->desc);
   return 1;
}

/**
 * @brief Gets the news article faction.
 *    @luatparam Article a article to get the faction of
 *    @luatreturn Faction faction
 * @luafunc faction
 */
int newsL_faction( lua_State *L )
{
   news_t *article_ptr = luaL_validnews(L, 1);
   lua_pushstring(L, article_ptr->faction);
   return 1;
}

/**
 * @brief Gets the news article date.
 *    @luatparam Article a article to get the date of
 *    @luatreturn number date
 * @luafunc date
 */
int newsL_date( lua_State *L )
{
   news_t *article_ptr = luaL_validnews(L, 1);
   lua_pushinteger(L, (lua_Integer)article_ptr->date);
   return 1;
}

/**
 * @brief Tags a news article or a table of articles with a string.
 *    @luatparam Article a Article to bind
 *    @luatparam string tag Tag to bind to the article
 * @luafunc bind
 */
int newsL_bind( lua_State *L )
{
   news_t *article_ptr;
   LuaNews_t *a = NULL;

   if (lua_istable(L, 1)) {
      const char *tag = luaL_checkstring(L, 2);
      lua_pop(L, 1);
      lua_pushnil(L);

      /* traverse table */
      while (lua_next(L, -2)) {
         if (!(a = luaL_checknews(L, -1))) {
            WARN(_("Bad argument to news.date(), must be article or a table of articles"));
            return 0;
         }
         article_ptr = news_get( *a );
         if (article_ptr == NULL) {
            WARN(_("Article not valid"));
            return 0;
         }
         article_ptr->tag = strdup(tag);
         lua_pop(L, 1);
      }
   }
   else {
      const char *tag;
      if (!(a = luaL_checknews(L, 1))) {
         WARN(_("Bad argument to news.date(), must be article or a table of articles"));
         return 0;
      }
      article_ptr = news_get(*a);
      if (article_ptr == NULL) {
         WARN(_("Article not valid"));
         return 0;
      }

      tag = luaL_checkstring(L, 2);
      article_ptr->tag = strdup( tag );
   }
   return 1;
}
