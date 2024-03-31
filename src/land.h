/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "conf.h"
#include "mission.h"
#include "nstring.h"
#include "space.h"
#include "start.h" // IWYU pragma: keep

/*
 * The window interfaces.
 */
enum {
   LAND_WINDOW_MAIN,      /**< Main window. */
   LAND_WINDOW_BAR,       /**< Bar window. */
   LAND_WINDOW_MISSION,   /**< Mission computer window. */
   LAND_WINDOW_OUTFITS,   /**< Outfits window. */
   LAND_WINDOW_SHIPYARD,  /**< Shipyard window. */
   LAND_WINDOW_EQUIPMENT, /**< Equipment window. */
   LAND_WINDOW_COMMODITY, /**< Commodity window. */
   LAND_NUMWINDOWS        /**< Number of land windows. */
};

typedef enum {
   MISNCOMPUTER_SORT_PRIORITY,
   MISNCOMPUTER_SORT_REWARD,
   MISNCOMPUTER_SORT_DISTANCE,
   MISNCOMPUTER_SORT_SETTINGS,
} MissionComputerSort;
typedef struct MissionComputerOptions_ {
   MissionComputerSort sortby;
} MissionComputerOptions;

/* global/main window */
#define LAND_WIDTH RESOLUTION_W_MIN  /**< Land window width. */
#define LAND_HEIGHT RESOLUTION_H_MIN /**< Land window height. */
#define PORTRAIT_WIDTH 200
#define PORTRAIT_HEIGHT 150

/*
 * Default button sizes.
 */
#define LAND_BUTTON_WIDTH 200 /**< Default button width. */
#define LAND_BUTTON_HEIGHT 40 /**< Default button height. */

/*
 * For the 'buy map' button
 */
#define LOCAL_MAP_NAME ( start_local_map_default() )

/*
 * Landed at.
 */
extern int   landed;
extern Spob *land_spob;

/* Tracking for which tabs have been generated. */
#define land_tabGenerate( w )                                                  \
   ( land_generated |= ( 1 << w ) ) /**< Mark tab generated. */
#define land_tabGenerated( w )                                                 \
   ( land_generated & ( 1 << w ) ) /**< Check if tab has been generated. */
extern unsigned int land_generated;

/*
 * Main interface.
 */
void land_queueTakeoff( void );
void land_needsTakeoff( int delay );
int  land_canSave( void );
int  land_doneLoading( void );
void land( Spob *p, int load );
void land_genWindows( int load );
void takeoff( int delay, int nosave );
void land_cleanup( void );
void land_exit( void );
int  land_setWindow( int window );

/*
 * Internal usage.
 */
void         land_refuel( void );
void         land_updateMainTab( void );
void         land_buttonTakeoff( unsigned int wid, const char *unused );
unsigned int land_getWid( int window );
void         bar_regen( void );
void         misn_regen( void );
void         misn_patchMission( const Mission *misn );

/*
 * Error dialogue generation and associated checks.
 */
void land_errClear( void );
PRINTF_FORMAT( 1, 2 ) void land_errDialogueBuild( const char *fmt, ... );
int land_errDisplay( void );
