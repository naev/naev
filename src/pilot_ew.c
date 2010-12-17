/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_ew.c
 *
 * @brief Pilot electronic warfare information.
 */


#include "pilot.h"

#include "naev.h"

#include "log.h"



/**
 * @brief Updates the pilot's sattic electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateStatic( Pilot *p )
{
   p->ew_mass = pilot_ewMass( p->solid->mass );
   p->ew_hide = p->ew_mass * p->ew_base_hide;
}


/**
 * @brief Updates the pilot's dynamic electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateDynamic( Pilot *p )
{
   p->ew_movement = pilot_ewMovement( VMOD(p->solid->vel) );
   p->ew_evasion  = p->ew_hide * p->ew_movement;
}


/**
 * @brief Gets the electronic warfare movement modifier for a given velocity.
 *
 *    @param vmod Velocity to get electronic warfare movement modifier of.
 *    @return The electronic warfare movement modifier.
 */
double pilot_ewMovement( double vmod )
{
   return 1. + sqrt( vmod ) / 15.;
}


/**
 * @brief Gets the electronic warfare mass modifier for a given velocity.
 *
 *    @param mass Mass to get the electronic warfare mass modifier of.
 *    @return The electronic warfare mass modifier.
 */
double pilot_ewMass( double mass )
{
   return 1. / (1. + sqrt( mass ) / 25.);
}


