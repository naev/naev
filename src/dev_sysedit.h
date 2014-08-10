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


#ifndef DEV_SYSEDIT_H
#  define DEV_SYSEDIT_H


#include "space.h"


#define HIDE_DEFAULT_PLANET      0.25 /**< Default hide value for new planets. */

void sysedit_open( StarSystem *sys );
void sysedit_sysScale( StarSystem *sys, double factor );

#endif /* DEV_SYSEDIT_H */
