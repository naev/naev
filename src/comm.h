/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef COMM_H
#  define COMM_H


#include "space.h"


int comm_isOpen (void);
void comm_queueClose (void);
int comm_openPilot( unsigned int pilot );
int comm_openPlanet( Planet *planet );


#endif /* COMM_H */

