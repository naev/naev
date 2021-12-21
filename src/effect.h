/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "opengl.h"
#include "shipstats.h"
#include "rng.h"

/**
 * @brief Pilot ship effect data.
 */
typedef struct EffectData_ {
   char *name;          /**< Name of the effect. */
   char *desc;          /**< Description of the effect. */
   char *overwrite;     /**< Common string to overwrite when adding. */
   int priority;        /**< Priority of the effect when overwriting. Lower is more important. */
   double duration;     /**< Max duration of the effect. */
   ShipStatList *stats; /**< Actual effect. */
   /* Visuals. */
   glTexture *icon;     /**< Effect icon texture. */
   GLuint program;
   GLuint vertex;
   GLuint projection;
   GLuint dimensions;
   GLuint u_r;
   GLuint u_tex;
   GLuint u_duration;
   GLuint u_timer;
} EffectData;

/**
 * @brief Pilot ship effect.
 */
typedef struct Effect_ {
   const EffectData *data;/**< Base data of the effect. */
   double timer;        /**< Time left on the effect. */
   double scale;        /**< Scales the effect. */
   double r;            /**< Random number. */
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
int effect_add( Effect **efxlist, const EffectData *efx, double scale );
void effect_clear( Effect **efxlist );
void effect_compute( ShipStats *s, const Effect *efxlist );
void effect_cleanup( Effect *efxlist );
