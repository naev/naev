/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file news.c
 *
 * @brief Handles news generation.
 */


#include "news.h"

#include <stdint.h>
#include <stdlib.h>

#include "naev.h"
#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_misn.h"
#include "nlua_faction.h"
#include "ndata.h"


#define LUA_NEWS     "dat/news.lua"


static lua_State *news_state = NULL; /**< Lua news state. */


/**
 * @brief Initializes the news.
 *
 *    @return 0 on success.
 */
int news_init (void)
{
   lua_State *L;
   char *buf;
   uint32_t bufsize;

   /* Already initialized. */
   if (news_state != NULL)
      return 0;

   /* Create the state. */
   news_state = nlua_newState();
   L = news_state;

   /* Load the libraries. */
   nlua_loadBasic(L);
   nlua_load(L,luaopen_string);
   lua_loadNaev(L);
   lua_loadVar(L,1);
   lua_loadSpace(L,1);
   lua_loadTime(L,1);
   lua_loadPlayer(L,1);
   lua_loadRnd(L);
   lua_loadDiff(L,1);
   lua_loadFaction(L,1);

   /* Load the news file. */
   buf = ndata_read( LUA_NEWS, &bufsize );
   if (luaL_dobuffer(news_state, buf, bufsize, LUA_NEWS) != 0) {
      WARN("Failed to load news file: %s\n"
           "%s\n"
           "Most likely Lua file has improper syntax, please check",
            LUA_NEWS, lua_tostring(L,-1));
      return -1;
   }
   free(buf);

   return 0;
}


/**
 * @brief Cleans up the news stuff.
 */
void news_exit (void)
{
   /* Already freed. */
   if (news_state == NULL)
      return;

   /* Clean up. */
   lua_close(news_state);
   news_state = NULL;
}


/**
 * @brief Gets a news sentence.
 */
char *news_get (void)
{
   char *s;
   lua_State *L;

   /* Lazy allocation. */
   if (news_state == NULL)
      news_init();
   L = news_state;

   /* Run the function. */
   lua_getglobal(L, "news");
   if (lua_pcall(L, 0, 1, 0)) { /* error has occured */
      WARN("News: '%s' : %s", "news", lua_tostring(L,-1));
      lua_pop(L,1);
   }

   /* Get the output. */
   s = strdup( luaL_checkstring(L, -1) );
   lua_pop(L,1);
   return s;
}


