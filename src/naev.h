/*
 * Copyright 2006, 2007, 2008 Edgar Simo Serra
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

/**
 * @file naev.h
 *
 * @brief Header file with generic functions and naev-specifics.
 */


#ifndef NAEV_H
#  define NAEV_H


#define APPNAME            "NAEV" /**< Application name. */

#define MALLOC_ONE(type)   (malloc(sizeof(type))) /**< Deprecated. */
#define CALLOC_ONE(type)   (calloc(1,sizeof(type))) /**< Deprecated. */

#define ABS(x)             (((x)<0)?-(x):(x)) /**< Returns absolute value. */
#define FABS(x)            (((x)<0.)?-(x):(x)) /**< Returns float absolute value. */

#define MAX(x,y)           (((x)>(y))?(x):(y)) /**< Returns maximum. */
#define MIN(x,y)           (((x)>(y))?(y):(x)) /**< Returns minimum. */

#define pow2(x)            ((x)*(x)) /**< ^2 */

/* maximum filename path */
#ifndef PATH_MAX
#  define PATH_MAX         256 /**< If not already defined. */
#endif /* PATH_MAX */

#ifndef M_PI
# define M_PI     3.14159265358979323846  /**< If not already defined. */
#endif /* M_PI */


char *naev_version (void);


#endif /* NAEV_H */

