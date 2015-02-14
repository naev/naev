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


#ifndef LAND_H
#  define LAND_H


#include "space.h"


/*
 * The window interfaces.
 */
enum {
   LAND_WINDOW_MAIN,         /**< Main window. */
   LAND_WINDOW_BAR,          /**< Bar window. */
   LAND_WINDOW_MISSION,      /**< Mission computer window. */
   LAND_WINDOW_OUTFITS,      /**< Outfits window. */
   LAND_WINDOW_SHIPYARD,     /**< Shipyard window. */
   LAND_WINDOW_EQUIPMENT,    /**< Equipment window. */
   LAND_WINDOW_COMMODITY,    /**< Commodity window. */
   LAND_NUMWINDOWS           /**< Number of land windows. */
};


/*
 * Default button sizes.
 */
#define LAND_BUTTON_WIDTH 200 /**< Default button width. */
#define LAND_BUTTON_HEIGHT 40 /**< Default button height. */


/*
 * Landed at.
 */
extern int landed;
extern Planet* land_planet;


/*
 * Main interface.
 */
void land_queueTakeoff (void);
int land_doneLoading (void);
void land( Planet* p, int load );
void land_genWindows( int load, int changetab );
void takeoff( int delay );
void land_cleanup (void);
void land_exit (void);
int land_setWindow( int window );


/*
 * Internal usage.
 */
void land_refuel (void);
void land_buttonTakeoff( unsigned int wid, char *unused );
unsigned int land_getWid( int window );
void bar_regen (void);

/*
 * Error dialogue generation and associated sanity checks.
 */
int can_swap( char* shipname );
int can_swapEquipment( char* shipname );
int can_sell( char* shipname );
int land_errDialogue( char* name, char* type );
void land_errDialogueBuild( const char *fmt, ... );


#endif /* LAND_H */
