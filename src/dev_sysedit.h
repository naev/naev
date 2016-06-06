/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DEV_SYSEDIT_H
#  define DEV_SYSEDIT_H


#include "space.h"


#define HIDE_DEFAULT_PLANET      0.25 /**< Default hide value for new planets. */

void sysedit_open( StarSystem *sys );
void sysedit_sysScale( StarSystem *sys, double factor );

#endif /* DEV_SYSEDIT_H */
