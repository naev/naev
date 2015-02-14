/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NLUA_NEWS_H
#  define NLUA_NEWS_H


#include <lua.h>
#include "news.h"

#define ARTICLE_METATABLE   "news" /**< System metatable identifier. */


/**
 * @brief Lua article Wrapper.
 */
typedef struct _article {

   int id; /**< pointer to article */

} Lua_article;


/*
 * Load the system library.
 */
int nlua_loadNews( lua_State *L, int readonly );

#endif /* NLUA_NEWS_H */
