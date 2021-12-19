/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "opengl.h"
#include "shipstats.h"

/**
 * @brief Pilot ship effect.
 */
typedef struct Effect_s {
   char *name;          /**< Name of the effect. */
   char *desc;          /**< Description of the effect. */
   glTexture *tex;      /**< Effect icon texture. */
   double duration;     /**< Max duration of the effect. */
   double timer;        /**< Time left on the effect. */
   ShipStatList *stats; /**< Actual effect. */
} Effect;

/*
 * Effect stuff.
 */
int effect_load (void);
void effect_exit (void);
const Effect *effect_get( const char *name );

/*
 * Effect list stuff.
 */
Effect *effect_init (void);
int effect_update( Effect **efxlist, double dt );
int effect_add( Effect **efxlist, const Effect *efx );
void effect_clear( Effect **efxlist );
void effect_compute( ShipStats *s, const Effect *efxlist );
void effect_cleanup( Effect *efxlist );
