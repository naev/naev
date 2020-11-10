/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef ENV_H
#define ENV_H

#include <stdlib.h>


typedef struct _env_t {
   short isAppImage;
   char *appimage;
   char *appdir;
   char *argv0;
} env_t;
extern env_t env;

void env_detect( int argc, char **argv );

#endif
