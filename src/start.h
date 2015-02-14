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


#ifndef START_H
#  define START_H


#include "ntime.h"


/*
 * Initialization/clean up.
 */
int start_load (void);
void start_cleanup (void);

/*
 * Getting data.
 */
const char* start_name (void);
const char* start_ship (void);
const char* start_shipname (void);
unsigned int start_credits (void);
ntime_t start_date (void);
const char* start_system (void);
void start_position( double *x, double *y );
const char* start_mission (void);
const char* start_event (void);
const char* start_tutMission (void);
const char* start_tutEvent (void);
const char* start_tutSystem (void);
void start_tutPosition( double *x, double *y );


#endif /* START_H */

