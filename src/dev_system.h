/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DEV_SYSTEM_H
#  define DEV_SYSTEM_H

#include "space.h"

int dsys_saveSystem( StarSystem *sys );
int dsys_saveAll (void);
int dsys_saveMap (StarSystem **uniedit_sys, int uniedit_nsys);


#endif /* DEV_SYSTEM_H */
