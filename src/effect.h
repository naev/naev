/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "opengl.h"
#include "shipstats.h"
#include "rng.h"

#define EFFECT_BUFF     (1<<0)   /**< Effect is a buff. */
#define EFFECT_DEBUFF   (1<<1)   /**< Effect is a debuff. */

/**
 * @brief Pilot ship effect data.
 */
typedef struct EffectData_ {
   char *name;          /**< Name of the effect. */
   char *desc;          /**< Description of the effect. */
   char *overwrite;     /**< Common string to overwrite when adding. */
   int priority;        /**< Priority of the effect when overwriting. Lower is more important. */
   double duration;     /**< Max duration of the effect. */
   unsigned int flags;  /**< Flags. */
   ShipStatList *stats; /**< Actual effect. */
   /* Visuals. */
   glTexture *icon;     /**< Effect icon texture. */
   GLuint program;
   GLuint vertex;
   GLuint projection;
   GLuint tex_mat;
   GLuint dimensions;
   GLuint u_r;
   GLuint u_tex;
   GLuint u_timer;
   GLuint u_elapsed;
   GLuint u_dir;
   /* Lua. */
   nlua_env lua_env;    /**< Lua environment. */
   int lua_add;         /**< Effect has been added to a pilot. */
   int lua_extend;      /**< Effect has been extended. */
   int lua_remove;      /**< Effect has been removed from a pilot. */
} EffectData;

/**
 * @brief Pilot ship effect.
 */
typedef struct Effect_ {
   const EffectData *data;/**< Base data of the effect. */
   unsigned int parent; /**< Pilot it is being applied to. */
   double timer;        /**< Time left on the effect. */
   double duration;     /**< Duration of this effect. */
   double strength;        /**< Scales the effect. */
   double r;            /**< Random number. */
   double elapsed;      /**< Total elapsed time. */
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
int effect_update( Effect **efxlist, double dt );
int effect_add( Effect **efxlist, const EffectData *efx, double duration, double strength, unsigned int parent );
int effect_rm( Effect **efxlist, int idx );
int effect_rmType( Effect **efxlist, const EffectData *efx, int all );
void effect_clearSpecific( Effect **efxlist, int debuffs, int buffs, int others );
void effect_clear( Effect **efxlist );
void effect_compute( ShipStats *s, const Effect *efxlist );
void effect_cleanup( Effect *efxlist );
