/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "opengl.h"
#include "shipstats.h"

/**
 * @brief Pilot ship effect data.
 */
typedef struct EffectData_s {
   char *name;          /**< Name of the effect. */
   char *desc;          /**< Description of the effect. */
   double duration;     /**< Max duration of the effect. */
   glTexture *tex;      /**< Effect icon texture. */
   ShipStatList *stats; /**< Actual effect. */
} EffectData;

/**
 * @brief Pilot ship effect.
 */
typedef struct Effect_s {
   const EffectData *data;/**< Base data of the effect. */
   double timer;        /**< Time left on the effect. */
} Effect;

/*
 * Effect stuff.
 */
int effect_load (void);
void effect_exit (void);
const EffectData *effect_get( const char *name );

/*
 * Effect list stuff.
 */
Effect *effect_init (void);
int effect_update( Effect **efxlist, double dt );
int effect_add( Effect **efxlist, const EffectData *efx );
void effect_clear( Effect **efxlist );
void effect_compute( ShipStats *s, const Effect *efxlist );
void effect_cleanup( Effect *efxlist );
