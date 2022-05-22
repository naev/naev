/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "news.h"
#include "nlua.h"

#define NEWS_METATABLE   "news" /**< System metatable identifier. */

/**
 * @brief Lua news Wrapper.
 */
typedef int LuaNews_t;

/*
 * Load the system library.
 */
int nlua_loadNews( nlua_env env );
