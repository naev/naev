/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space.h"

int  comm_isOpen( void );
void comm_queueClose( void );
int  comm_openPilot( unsigned int pilot );
int  comm_openSpob( Spob *spob );
