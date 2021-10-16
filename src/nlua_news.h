/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "news.h"
#include "nlua.h"

#define ARTICLE_METATABLE   "news" /**< System metatable identifier. */

/**
 * @brief Lua article Wrapper.
 */
typedef int LuaArticle;

/*
 * Load the system library.
 */
int nlua_loadNews( nlua_env env );
