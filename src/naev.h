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


#ifndef NAEV_H
#  define NAEV_H


#define APPNAME            "NAEV"

#define MALLOC_ONE(type)   (malloc(sizeof(type)))
#define CALLOC_ONE(type)   (calloc(1,sizeof(type)))

#define ABS(x)             (((x)<0)?-(x):(x))

#define MAX(x,y)           (((x)>(y))?(x):(y))
#define MIN(x,y)           (((x)>(y))?(y):(x))

#define pow2(x)            ((x)*(x))

#define DATA_DEF           "data" /* default data packfile */
extern char* data; /* modifiable datafile */
#define DATA               data /* data file */

/* maximum filename path */
#ifndef PATH_MAX
#  define PATH_MAX         100
#endif /* PATH_MAX */


#endif /* NAEV_H */

