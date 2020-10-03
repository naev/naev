/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef ENV_H
#define ENV_H

#include <stdlib.h>


struct {
   short isAppImage;
   char *appimage;
   char *appdir;
   char *argv0;
} env;


void env_detect( char **argv );

#endif