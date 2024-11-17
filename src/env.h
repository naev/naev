/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct _env_t {
   short isAppImage;
   char *appimage;
   char *appdir;
   char *argv0;
} env_t;
extern env_t env;

void env_detect( int argc, char **argv );
