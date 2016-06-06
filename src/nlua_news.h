/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_NEWS_H
#  define NLUA_NEWS_H


#include <lua.h>
#include "news.h"

#define ARTICLE_METATABLE   "news" /**< System metatable identifier. */


/**
 * @brief Lua article Wrapper.
 */
typedef int LuaArticle;


/*
 * Load the system library.
 */
int nlua_loadNews( lua_State *L, int readonly );

#endif /* NLUA_NEWS_H */
