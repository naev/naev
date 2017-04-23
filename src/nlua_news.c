/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_news.c
 *
 * @brief Lua news module.
 */

#include "nlua_news.h"

#include "naev.h"

#include <lauxlib.h>

#include "land.h"
#include "nluadef.h"
#include "nstring.h"
#include "ntime.h"
#include "nlua_time.h"


extern news_t *news_list; /**< Linked list containing all articles */
extern int land_loaded;


int newsL_add( lua_State *L );
int newsL_rm( lua_State *L );
int newsL_get( lua_State *L );
LuaArticle* luaL_validarticle( lua_State *L, int ind );
int lua_isarticle( lua_State *L, int ind );
LuaArticle* lua_pusharticle( lua_State *L, LuaArticle article );
int newsL_eq( lua_State *L );
int newsL_title( lua_State *L );
int newsL_desc( lua_State *L );
int newsL_faction( lua_State *L );
int newsL_date( lua_State *L );
int newsL_bind( lua_State *L );
static const luaL_reg news_methods[] = {
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
   nlua_register(env, ARTICLE_METATABLE, news_methods, 1);
   return 0; /* No error */
}


/**
 * @brief Pushes an article on the stack.
 *
 *    @param L Lua state to push article into.
 *    @param article article to push.
 *    @return Newly pushed article.
 */
LuaArticle* lua_pusharticle( lua_State *L, LuaArticle article )
{
   LuaArticle *o;
   o = (LuaArticle *)lua_newuserdata(L, sizeof(LuaArticle));
   *o = article;
   luaL_getmetatable(L, ARTICLE_METATABLE);
   lua_setmetatable(L, -2);
   return o;
}


/**
 * @brief Makes sure the article is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the article to validate.
 *    @return The article (doesn't return if fails - raises Lua error ).
 */
LuaArticle* luaL_validarticle( lua_State *L, int ind )
{
   LuaArticle *Larticle;

   if (lua_isarticle(L, ind)) {
      Larticle = (LuaArticle *)lua_touserdata(L, ind);
      if (news_get(*Larticle))
         return Larticle;
      else
         NLUA_ERROR(L, "article is old");
   }
   else {
      luaL_typerror(L, ind, ARTICLE_METATABLE);
      return NULL;
   }

   NLUA_ERROR(L, "article is invalid.");

   return NULL;
}


/**
 * @brief Checks to see if ind is an article.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a article.
 */
int lua_isarticle( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L, ind) == 0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, ARTICLE_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2)) /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2); /* remove both metatables */
   return ret;
}


/**
 * @brief Lua bindings to interact with the news.
 *
 * This will allow you to interact and manipulate the in-game news.
 *
 * @luamod news
 */
/**
 * @brief Adds an article.
 * @usage news.add(faction,title,body,[date_to_rm, [date]])
 *
 * @usage s = news.add( "Empire", "Hello world!", "The Empire wishes to say hello!", 0 ) -- Adds an Empire specific article, with date 0.
 *
 *    @luatparam Faction|string faction of the article, "Generic" for non-factional
 *    @luatparam string title Title of the article
 *    @luatparam string body What's in the article
 *    @luatparam[opt] number|Time date_to_rm date to remove the article
 *    @luatparam[opt] number|Time date What time to put, defaults to current date, use 0 to not use a date
 *    @luatreturn Article The article matching name or nil if error.
 * @luafunc add( faction, title, body, date_to_rm, date )
 */
int newsL_add( lua_State *L )
{
   news_t *n_article;
   char *title, *content, *faction;
   ntime_t date, date_to_rm;

   NLUA_CHECKRW(L);

   n_article = NULL;
   title   = NULL;
   content = NULL;
   faction = NULL;

   date = ntime_get();
   date_to_rm = 50000000000000;

   /* If a table is passed in. ugly hack */
   if (lua_istable(L, 1)) {
      lua_pushnil(L);

      /* traverse table */
      while (lua_next(L, -2)) {
         /* traverse sub table */
         if (lua_istable(L, -1)) {
            lua_pushnil(L);
            while (lua_next(L, -2)) {
               if (lua_isnumber(L, -1)) {
                  if (date_to_rm)
                     date_to_rm = lua_tonumber(L, -1);
                  else
                     date = lua_tonumber(L, -1);
               }
               else if (lua_istime(L, -1)) {
                  if (date_to_rm)
                     date_to_rm = luaL_validtime(L, -1);
                  else
                     date = luaL_validtime(L, -1);
               }
               else if (lua_isstring(L, -1)) {
                  if (!faction)
                     faction = strdup(lua_tostring(L, -1));
                  else if (!title)
                     title = strdup(lua_tostring(L, -1));
                  else if (!content)
                     content = strdup(lua_tostring(L, -1));
               }

               lua_pop(L, 1);
            }

            if (title && content && faction)
               new_article(title, content, faction, date, date_to_rm);
            else
               WARN("Bad arguments");

            free(faction);
            free(title);
            free(content);
            faction = NULL;
            title = NULL;
            content = NULL;

            date = ntime_get();
            date_to_rm = 50000000000000;
         }
         lua_pop(L, 1);
      }
      lua_pop(L, 1);

      /* If we're landed, we should regenerate the news buffer. */
      if (landed) {
         generate_news(faction_name(land_planet->faction));
         if (land_loaded)
            bar_regen();
      }

      return 0;
   }

   if (!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3))) {
      WARN("\nBad arguments, use "
           "addArticle(\"Faction\",\"Title\",\"Content\",[date,[date_to_rm]])");
      return 0;
   }

   faction = strdup(lua_tostring(L, 1));
   title   = strdup(lua_tostring(L, 2));
   content = strdup(lua_tostring(L, 3));

   /* get date and date to remove, or leave at defaults*/
   if (lua_isnumber(L, 4) || lua_istime(L, 4)) {
      if (lua_istime(L, 4))
         date_to_rm = luaL_validtime(L, 4);
      else
         date_to_rm = lua_tonumber(L, 4);
   }

   if (lua_isnumber(L, 5) || lua_istime(L, 5)) {
      if (lua_istime(L, 5))
         date = luaL_validtime(L, 5);
      else
         date = lua_tonumber(L, 5);
   }

   if (title && content && faction)
      n_article = new_article(title, content, faction, date, date_to_rm);
   else
      NLUA_ERROR(L,"Bad arguments");

   lua_pusharticle(L, n_article->id);

   free(title);
   free(content);
   free(faction);

   /* If we're landed, we should regenerate the news buffer. */
   if (landed) {
      generate_news(faction_name(land_planet->faction));
      if (land_loaded)
         bar_regen();
   }

   return 1;
}


/**
 * @brief Frees an article or a table of articles.
 *    @luatparam Article article article to free
 * @luafunc rm( article )
 */
int newsL_rm( lua_State *L )
{
   LuaArticle *Larticle;

   NLUA_CHECKRW(L);

   if (lua_istable(L, 1)) {
      lua_pushnil(L);
      while (lua_next(L, -2)) {
         Larticle = luaL_validarticle(L, -1);

         free_article(*Larticle);

         lua_pop(L, 1);
      }
   }
   else {
      Larticle = luaL_validarticle(L, 1);

      free_article(*Larticle);
   }

   /* If we're landed, we should regenerate the news buffer. */
   if (landed) {
      generate_news(faction_name(land_planet->faction));
      if (land_loaded) {
         bar_regen();
      }
   }

   return 1;
}


/**
 * @brief Gets all matching articles in a table.
 *    @luatparam[opt] number|String characteristic characteristic to match, or no parameter for all articles
 *    @luatreturn {Article,...} a table with matching articles
 * @luafunc get(characteristic)
 */
int newsL_get( lua_State *L )
{
   LuaArticle Larticle;
   ntime_t date;
   news_t *article_ptr;
   char *characteristic;
   int k, print_all;

   date = -1;
   article_ptr = news_list;
   characteristic = NULL;
   print_all = 0;

   if (lua_isnil(L, 1) || lua_gettop(L) == 0) /* Case no argument */
      print_all = 1;
   else if (lua_isnumber(L, 1))
      date = (ntime_t)lua_tonumber(L, 1);
   else if (lua_isstring(L, 1))
      characteristic = strdup(lua_tostring(L, 1));
   else
      NLUA_INVALID_PARAMETER(L); /* Bad Parameter */

   /* Now put all the matching articles in a table. */
   lua_newtable(L);
   k = 1;
   do {

      if (article_ptr->title == NULL || article_ptr->desc == NULL ||
          article_ptr->faction == NULL)
         continue;

      if (print_all || date == article_ptr->date ||
          (characteristic && (!strcmp(article_ptr->title, characteristic) ||
                              !strcmp(article_ptr->desc, characteristic) ||
                              !strcmp(article_ptr->faction, characteristic))) ||
          (article_ptr->tag != NULL &&
           !strcmp(article_ptr->tag, characteristic))) {
         lua_pushnumber(L, k++); /* key */
         Larticle = article_ptr->id;
         lua_pusharticle(L, Larticle); /* value */
         lua_rawset(L, -3);            /* table[key] = value */
      }

   } while ((article_ptr = article_ptr->next) != NULL);

   free(characteristic);

   return 1;
}


/**
 * @brief Check articles for equality.
 *
 * Allows you to use the '=' operator in Lua with articles.
 *
 *    @luatparam Article a1 article 1
 *    @luatparam Article a2 article 2
 *    @luatreturn boolean true if both systems are the same.
 * @luafunc __eq( a1, a2 )
 */
int newsL_eq( lua_State *L )
{
   LuaArticle *a1;
   LuaArticle *a2;
   a1 = luaL_validarticle(L, 1);
   a2 = luaL_validarticle(L, 2);
   if (*a1 == *a2)
      lua_pushboolean(L, 1);
   else
      lua_pushboolean(L, 0);
   return 1;
}


/**
 * @brief Gets the article title.
 *    @luatparam Article a article to get the title of
 *    @luatreturn string title
 * @luafunc title(a)
 */
int newsL_title( lua_State *L )
{
   LuaArticle *a;
   news_t *article_ptr;
   if (!(a = luaL_validarticle(L, 1))) {
      WARN("Bad argument to news.date(), must be article");
      return 0;
   }
   if ((article_ptr = news_get(*a)) == NULL) {
      WARN("\nArticle not valid");
      return 0;
   }
   lua_pushstring(L, article_ptr->title);
   return 1;
}


/**
 * @brief Gets the article description.
 *    @luatparam Article a article to get the desc of
 *    @luatreturn string desc
 * @luafunc desc(a)
 */
int newsL_desc( lua_State *L )
{
   LuaArticle *a;
   news_t *article_ptr;
   if (!(a = luaL_validarticle(L, 1))) {
      WARN("Bad argument to news.date(), must be article");
      return 0;
   }
   if ((article_ptr = news_get(*a)) == NULL) {
      WARN("\nArticle not valid");
      return 0;
   }
   lua_pushstring(L, article_ptr->desc);
   return 1;
}


/**
 * @brief Gets the article faction.
 *    @luatparam Article a article to get the faction of
 *    @luatreturn Faction faction
 * @luafunc faction(a)
 */
int newsL_faction( lua_State *L )
{
   LuaArticle *a;
   news_t *article_ptr;
   if (!(a = luaL_validarticle(L, 1))) {
      WARN("Bad argument to news.date(), must be article");
      return 0;
   }
   if ((article_ptr = news_get(*a)) == NULL) {
      WARN("\nArticle not valid");
      return 0;
   }
   lua_pushstring(L, article_ptr->faction);
   return 1;
}


/**
 * @brief Gets the article date.
 *    @luatparam Article a article to get the date of
 *    @luatreturn number date
 * @luafunc date(a)
 */
int newsL_date( lua_State *L )
{
   LuaArticle *a;
   news_t *article_ptr;
   if (!(a = luaL_validarticle(L, 1))) {
      WARN("Bad argument to news.date(), must be article");
      return 0;
   }
   if ((article_ptr = news_get(*a)) == NULL) {
      WARN("\nArticle not valid");
      return 0;
   }
   lua_pushinteger(L, (lua_Integer)article_ptr->date);
   return 1;
}


/**
 * @brief Tags an article or a table of articles with a string.
 *    @luatparam Article a article to get the faction of
 *    @luatparam string tag
 * @luafunc bind(a, tag)
 */
int newsL_bind( lua_State *L )
{
   LuaArticle *a;
   news_t *article_ptr;
   char *tag;

   NLUA_CHECKRW(L);

   a = NULL;
   tag = NULL;

   if (lua_istable(L, 1)) {
      if (!lua_isstring(L, 2)) {
         WARN("\n2nd argument is invalid, use a string");
         return 1;
      }

      tag = strdup(lua_tostring(L, 2));

      lua_pop(L, 1);

      lua_pushnil(L);

      /* traverse table */
      while (lua_next(L, -2)) {
         if (!(a = luaL_validarticle(L, -1))) {
            WARN("Bad argument to news.date(), must be article or a table of "
                 "articles");
            return 0;
         }
         if (article_ptr == NULL) {
            WARN("\nArticle not valid");
            return 0;
         }
         article_ptr = news_get(*a);
         article_ptr->tag = strdup(tag);
         lua_pop(L, 1);
      }
   }
   else {
      if (!(a = luaL_validarticle(L, 1))) {
         WARN("Bad argument to news.date(), must be article or a table of "
              "articles");
         return 0;
      }
      article_ptr = news_get(*a);
      if (article_ptr == NULL) {
         WARN("\nArticle not valid");
         return 0;
      }
      if (!lua_isstring(L, 2)) {
         WARN("\n2nd argument is invalid, use a string");
         return 1;
      }

      tag = strdup(lua_tostring(L, 2));

      article_ptr->tag = tag;
   }

   return 1;
}
