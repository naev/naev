/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "opengl.h"
#include "shipstats.h"

typedef struct Effect_s {
   char *name;
   char *desc;
   glTexture *tex;
   double duration;
   double timer;
   ShipStatList *stats;
} Effect;

int effect_load (void);
void effect_exit (void);

const Effect *effect_get( const char *name );

Effect *effect_init (void);
int effect_update( Effect **efxlist, double dt );
int effect_add( Effect **efxlist, const Effect *efx );
void effect_clear( Effect **efxlist );
void effect_compute( ShipStats *s, const Effect *efxlist );
void effect_cleanup( Effect *efxlist );
