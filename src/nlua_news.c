/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_economy.c
 *
 * @brief Lua economy module.
 */

#include "nlua_news.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include <string.h>

extern news_t* news_list;   /**< Linked list containing all articles */

int newsL_addArticle( lua_State *L );
int newsL_freeArticle( lua_State *L );
int newsL_getFaction( lua_State *L );
int newsL_getTitle( lua_State *L );
int newsL_getDate( lua_State *L );
Lua_article* luaL_validarticle( lua_State *L, int ind );
int lua_isarticle( lua_State *L, int ind );
Lua_article* lua_pusharticle( lua_State *L, Lua_article article );

int newL_DummyPrint( lua_State *L);
int newsL_getTitleOA(lua_State *L);
int newsL_eq( lua_State *L );

static const luaL_reg economy_methods[] = {
   {"addArticle",newsL_addArticle},
   {"freeArticle",newsL_freeArticle},
   {"getFaction",newsL_getFaction},
   {"getTitle",newsL_getTitle},
   {"getDate",newsL_getDate},
   {"getTitleOA",newsL_getTitleOA},
   {"__eq",newsL_eq},
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   {"getFaction",newsL_getFaction},
   {"getTitle",newsL_getTitle},
   {"getDate",newsL_getDate},
   {"getTitleOA",newsL_getTitleOA},
   {"__eq",newsL_eq},
   {0,0}
}; /**< Read only economy metatable methods. */

/**
 * @brief Loads the economy library.
 *
 *    @param L State to load economy library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadNews( lua_State *L, int readonly )
{

   /* Create the metatable */
   luaL_newmetatable(L, ARTICLE_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, economy_cond_methods);
   else
      luaL_register(L, NULL, economy_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, ARTICLE_METATABLE);

   return 0; /* No error */
}

int newL_DummyPrint( lua_State *L)
{
   (void) L;

   new_article("Generic Title II","The reader of this article hereafter forfeits any previous or current right to the throne of generality"
      ,"Generic",300000000);

   return 1;
}


/**
 * @brief Adds an article
 *
 * @usage s = news.addArticle( "Hyena" ) -- Gets the hyena
 *
 *    @luaparam title Title of the article
 *    @luaparam content What's in the article
 *    @luaparam faction faction of the article, "Generic" for non-factional
 *    @luaparam date What time to put, use 0 to not use a date
 *    @luareturn The article matching name or nil if error.
 * @luafunc get( s )
 */
int newsL_addArticle( lua_State *L ){

   printf("\nAdding new Lua article");

   news_t* n_article;
   Lua_article Larticle;

   char* title=strdup(lua_tostring(L,1));
   char* content=strdup(lua_tostring(L,2));
   char* faction=strdup(lua_tostring(L,3));
   ntime_t date=lua_tointeger(L,4);

   if (title && content && faction)
      n_article = new_article(title, content, faction, date);


   Larticle.id = n_article->id;
   lua_pusharticle(L, Larticle);

   free(title);
   free(content);
   free(faction);


   return 1;
}


/**
 * @brief frees an article
 *    @luaparam Lua_article article to free
 */
int newsL_freeArticle( lua_State *L ){

   Lua_article* Larticle;

   Larticle = luaL_validarticle(L,1);

   free_article(Larticle->id);

   return 1;
}

/**
 * @brief gets all matching articles
 *    @luaparam faction name of faction
 *    @luareturn a table with matching factions
 */
int newsL_getFaction( lua_State *L ){

   printf("\nGetting factions...");

   news_t* article_ptr = news_list;
   Lua_article Larticle;
   int k;

   char* getFaction = strdup(lua_tostring(L,1));

         /* Now put all the matching articles in a table. */
   lua_newtable(L);
   k = 1;
   do {

      if (article_ptr->faction==NULL)
         continue;

      if (!strcmp(article_ptr->faction,getFaction)) {
         lua_pushnumber(L, k++); /* key */
         Larticle.id = article_ptr->id;
         lua_pusharticle(L, Larticle); /* value */
         lua_rawset(L,-3); /* table[key] = value */
      }

   }while( (article_ptr = article_ptr->next) != NULL );

   free(getFaction);

   return 1;

}

/**
 * @brief gets all
 *    @luaparam faction name of faction
 *    @luareturn a table with matching factions
 */
int newsL_getTitle( lua_State *L ){

   news_t* article_ptr=news_list;
   Lua_article Larticle;
   int k;

   char* getTitle = strdup(lua_tostring(L,1));

         /* Now put all the matching articles in a table. */
   lua_newtable(L);
   k = 1;
   do {

      if (article_ptr->title==NULL)
         continue;

      if (!strcmp(article_ptr->title,getTitle)) {
         lua_pushnumber(L, k++); /* key */
         Larticle.id=article_ptr->id;
         lua_pusharticle(L, Larticle); /* value */
         lua_rawset(L,-3); /* table[key] = value */
      }

   }while( (article_ptr = article_ptr->next) != NULL );

   free(getTitle);

   return 1;
}

/**
 * @brief gets all matching articles in a range of dates
 *    @luaparam date1 lower limit date of the article(s)
 *    @luaparam date2 upper limit date of the article(s)
 *    @luareturn a table with matching factions
 */
int newsL_getDate( lua_State *L )
{

   news_t* article_ptr=news_list;
   Lua_article Larticle;
   int k;

   ntime_t time1 = lua_tonumber(L,1);
   ntime_t time2 = lua_tonumber(L,2);

         /* Now put all the matching articles in a table. */
   lua_newtable(L);
   k = 1;
   do {

      if ( time1<=article_ptr->date && time2>=article_ptr->date ) {
         lua_pushnumber(L, k++); /* key */
         Larticle.id=article_ptr->id;
         lua_pusharticle(L, Larticle); /* value */
         lua_rawset(L,-3); /* table[key] = value */
      }

   }while( (article_ptr = article_ptr->next) != NULL );

   return 1;
}

/**
 * @brief Check articles for equality.
 *
 * Allows you to use the '=' operator in Lua with articles.
 *
 *    @luaparam a1 article 1
 *    @luaparam a2 article 2
 *    @luareturn true if both systems are the same.
 * @luafunc __eq( s, comp )
 */
int newsL_eq( lua_State *L )
{
   Lua_article *a1;
   Lua_article *a2;
   a1 = luaL_validarticle( L,1 );
   a2 = luaL_validarticle( L,2 );
   if (a1->id == a2->id)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}


/**
 * @brief returns the title of a selected article
 *    @luaparam article an article
 *    @return string with the title of the article in it
 */
int newsL_getTitleOA(lua_State *L)
{
   printf("\nGetting title");

   news_t* article_ptr;

   article_ptr = get_article(luaL_validarticle(L,1)->id);

   lua_pushstring(L,article_ptr->title);

   return 1;

}



/**
 * @brief Makes sure the article is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the article to validate.
 *    @return The article (doesn't return if fails - raises Lua error ).
 */
Lua_article* luaL_validarticle( lua_State *L, int ind )
{

   if (lua_isarticle(L, ind)) {
      return (Lua_article*) lua_touserdata(L,ind);
   }
   else {
      luaL_typerror(L, ind, ARTICLE_METATABLE);
      return NULL;
   }

   NLUA_ERROR(L, "article is invalid.");

   return NULL;
}

/**
 * @brief Checks to see if ind is an article
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a article.
 */
int lua_isarticle( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, ARTICLE_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Pushes an article on the stack.
 *
 *    @param L Lua state to push article into.
 *    @param article article to push.
 *    @return Newly pushed article.
 */
Lua_article* lua_pusharticle( lua_State *L, Lua_article article )
{
   Lua_article *o;
   o = (Lua_article*) lua_newuserdata(L, sizeof(Lua_article));
   *o = article;
   luaL_getmetatable(L, ARTICLE_METATABLE);
   lua_setmetatable(L, -2);
   return o;
}