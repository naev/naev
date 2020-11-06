/*
 * Copyright 2006-2020 Naev DevTeam
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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


#include "ncompat.h"

#include <limits.h>
#include <inttypes.h>
#include <math.h>


#define APPNAME            "Naev" /**< Application name. */

#define ABS(x)             (((x)<0)?-(x):(x)) /**< Returns absolute value. */
#define FABS(x)            (((x)<0.)?-(x):(x)) /**< Returns float absolute value. */

#define MAX(x,y)           (((x)>(y))?(x):(y)) /**< Returns maximum. */
#define MIN(x,y)           (((x)>(y))?(y):(x)) /**< Returns minimum. */
#define CLAMP(a, b, x)     ((x)<(a)?(a):((x)>(b)?(b):(x))) /**< Clamps x between a and b: a <= x <= b. */

#define pow2(x)            ((x)*(x)) /**< ^2 */

/* maximum filename path */
#ifndef PATH_MAX
#  define PATH_MAX         256 /**< If not already defined. */
#endif /* PATH_MAX */



/* For inferior OS. */
#ifndef M_PI
#  define M_PI          3.14159265358979323846
#endif /* M_PI */
#ifndef M_SQRT1_2
#  define M_SQRT1_2     0.70710678118654752440
#endif
#ifndef M_SQRT2
#  define M_SQRT2       1.41421356237309504880
#endif


/* I've heard talk of PRIuN being evil, so there's this sad panda here.
 *
 *             :OOkxddkkdc
 *           cl;.......',d0kl'
 *          l;...........'c0NWO;
 *        ,xo'............',lkKdx:
 *        0x,''...........',;;::oO0,
 *       :0:,'''.......;dl,',',;xXNNx.
 *       00c;,,,,.....'KXXKOd:,;oXWWNNd
 *     ,KXx:;,;:xd'....ck0XNXo;:l0NNNNN0.
 *    oXNNOc;,,lNNk',,,...,cl::cd0NNNXXXK,
 *    XXXNNKo;;kNNd',;;,..',;cloONNNNNNXXk
 *  .dNNNNNNXdckNK:'',::::c;:ldkXNWWNNNNNX.
 * cXNNWWWWNNKoloc:,,:cldOoodkKxcKWWWWWWNN;
 * 0NWWWWWWNXl''';ooodkxOxk00o.  xNWWWWWWNd
 * ;odkOOkxo:       .',,,,'      ;K0NKNKNNk
 *                                 .cxxkk,.
 */
#ifndef PRIu64
#   define PRIu64    "%ju" /**< Illegal, evil and probably eats babies too. */
#endif
#ifndef PRIi64
#   define PRIi64    "%jd" /**< Illegal, evil and probably eats babies too. */
#endif


/*
 * Misc stuff.
 */
extern const double fps_min;
void fps_setPos( double x, double y );
void naev_resize (void);
void naev_toggleFullscreen (void);
void update_routine( double dt, int enter_sys );
char *naev_version( int long_version );
int naev_versionCompare( char *version );
char *naev_binary (void);
void naev_quit (void);


#endif /* NAEV_H */
