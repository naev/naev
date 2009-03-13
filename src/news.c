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


/*
 * News state.
 */
static lua_State *news_state  = NULL; /**< Lua news state. */


/*
 * News buffer.
 */
static news_t *news_buf       = NULL; /**< Buffer of news. */
static int news_nbuf          = 0; /**< Size of news buffer. */


/*
 * Prototypes.
 */
static void news_cleanBuffer (void);


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
 * @brief Cleans the news buffer.
 */
static void news_cleanBuffer (void)
{
   int i;

   if (news_buf != NULL) {
      for (i=0; i<news_nbuf; i++) {
         free(news_buf[i].title);
         free(news_buf[i].desc);
      }
      free(news_buf);
      news_buf    = NULL;
      news_nbuf   = 0;
   }
}


/**
 * @brief Cleans up the news stuff.
 */
void news_exit (void)
{
   /* Already freed. */
   if (news_state == NULL)
      return;

   news_cleanBuffer();

   /* Clean up. */
   lua_close(news_state);
   news_state = NULL;
}


/**
 * @brief Gets a news sentence.
 */
const news_t *news_generate( int *ngen, int n )
{
   int i;
   lua_State *L;

   /* Lazy allocation. */
   if (news_state == NULL)
      news_init();
   L = news_state;

   /* Clean up the old buffer. */
   news_cleanBuffer();

   /* Allocate news. */
   news_buf = calloc( sizeof(news_t), n );
   (*ngen)  = 0;

   /* Run the function. */
   lua_getglobal(L, "news");
   lua_pushnumber(L, n);
   if (lua_pcall(L, 1, 2, 0)) { /* error has occured */
      WARN("News: '%s' : %s", "news", lua_tostring(L,-1));
      lua_pop(L,1);
      return NULL;
   }

   /* Check to see if it's valid. */
   if (!lua_isstring(L, -2) || !lua_istable(L, -1)) { 
      WARN("News generated invalid output!");
      lua_pop(L,1);
      return NULL;
   }

   /* Create the title header. */
   news_buf[0].title = strdup("TITLE");
   news_buf[0].desc  = strdup( lua_tostring(L, -2) );

   /* Pull it out of the table. */
   i = 1;
   lua_pushnil(L);
   while ((lua_next(L,-2) != 0) && (i<n)) {
      /* Pull out of the internal table the data. */
      lua_getfield(L, -1, "title");
      news_buf[i].title = strdup( luaL_checkstring(L, -1) );
      lua_pop(L,1); /* pop the title string. */
      lua_getfield(L, -1, "desc");
      news_buf[i].desc = strdup( luaL_checkstring(L, -1) );
      lua_pop(L,1); /* pop the desc string. */
      /* Go to next element. */
      lua_pop(L,1); /* pop the table. */
      i++;
   }

   /* Clen up results. */
   lua_pop(L,2);

   /* Save news found. */
   news_nbuf   = i;
   (*ngen)     = news_nbuf;
   
   return news_buf;
}


