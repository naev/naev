/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#define NEBULA_DEFAULT_HUE                                                     \
   260.0 /**< Default hue (in degrees) for the nebula. */

/*
 * Init/Exit
 */
int  nebu_init( void );
int  nebu_resize( void );
void nebu_exit( void );

/*
 * Render
 */
void nebu_render( const double dt );
void nebu_renderOverlay( const double dt );

/*
 * Update.
 */
void nebu_update( double dt );

/*
 * Misc
 */
double nebu_getSightRadius( void );
void   nebu_prep( double density, double volatility, double hue );
