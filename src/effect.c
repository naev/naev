/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file effect.c
 *
 * @brief Handles pilot effects.
 */
#include "effect.h"

#include "array.h"
#include "nxml.h"

static EffectData *effect_list = NULL; /* List of all available effects. */

static int effect_cmp( const void *p1, const void *p2 )
{
   const EffectData *e1 = p1;
   const EffectData *e2 = p2;
   return strcmp( e1->name, e2->name );
}

static int effect_cmpTimer( const void *p1, const void *p2 )
{
   const Effect *e1 = p1;
   const Effect *e2 = p2;
   return e1->timer - e2->timer;
}

/**
 * @brief Loads all the effects.
 *
 *    @return 0 on success
 */
int effect_load (void)
{
   effect_list = array_create( EffectData );
   qsort( effect_list, array_size(effect_list), sizeof(EffectData), effect_cmp );
   return 0;
}

/**
 * @brief Gets rid of all the effects.
 */
void effect_exit (void)
{
   for (int i=0; i<array_size(effect_list); i++) {
      EffectData *e = &effect_list[i];
      free( e->name );
      free( e->desc );
      gl_freeTexture( e->tex );
      ss_free( e->stats );
   }
   array_free( effect_list );
}

/**
 * @brief Gets an effect by name.
 *
 *    @param name Name of the base effect to get.
 *    @return The base effect or NULL if not applicable.
 */
const EffectData *effect_get( const char *name )
{
   const EffectData k = { .name = (char*)name };
   EffectData *e = bsearch( &k, effect_list, array_size(effect_list), sizeof(EffectData), effect_cmp );
   if (e==NULL)
      WARN(_("Trying to get unknown effect data '%s'!"), name);
   return e;
}

/**
 * @brief Initializes an effect list.
 */
Effect *effect_init (void)
{
   return array_create( Effect );
}

/**
 * @brief Updates an effect list.
 *
 *    @param efxlist The effect list.
 *    @param dt The time update.
 *    @return The number of effects that ended or changed.
 */
int effect_update( Effect **efxlist, double dt )
{
   int n = 0;
   for (int i=array_size(*efxlist)-1; i>=0; i--) {
      Effect *e = &(*efxlist)[i];
      e->timer -= dt;
      if (e->timer > 0.)
         continue;

      /* Get rid of it. */
      array_erase( efxlist, e, e+1 );
      n++;
   }
   return n;
}

/**
 * @brief Adds an effect to an effect list.
 *
 *    @param efxlist List of effects.
 *    @param efx Effect to add.
 *    @return 0 on success.
 */
int effect_add( Effect **efxlist, const EffectData *efx )
{
   Effect *e = &array_grow( efxlist );
   e->data = efx;
   e->timer = efx->duration;
   qsort( efxlist, array_size(efxlist), sizeof(Effect), effect_cmpTimer );
   return 0;
}

/**
 * @brief Clears an effect list, removing all active effects.
 *
 *    @param efxlist List of effects.
 */
void effect_clear( Effect **efxlist )
{
   array_erase( efxlist, array_begin(*efxlist), array_end(*efxlist) );
}

/**
 * @brief Updates shipstats from effect list.
 *
 *    @param s Stats to update.
 *    @param efxlist List of effects.
 */
void effect_compute( ShipStats *s, const Effect *efxlist )
{
   for (int i=0; i<array_size(efxlist); i++)
      ss_statsModFromList( s, efxlist[i].data->stats );
}

/**
 * @brief Cleans up an effect list freeing it.
 *
 *    @param efxlist List to free.
 */
void effect_cleanup( Effect *efxlist )
{
   array_free( efxlist );
}
