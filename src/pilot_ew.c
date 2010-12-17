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


double pilot_ewMovement( double vmod )
{
   return 1. + sqrt( vmod ) / 15.;
}


double pilot_ewMass( double mass )
{
   return 1. / (1. + sqrt( mass ) / 25.);
}


