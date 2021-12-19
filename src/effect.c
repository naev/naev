/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file effect.c
 *
 * @brief Handles pilot effects.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "effect.h"

#include "array.h"
#include "conf.h"
#include "log.h"
#include "nxml.h"
#include "ndata.h"
#include "gui.h"

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

static int effect_parse( EffectData *efx, const char *file )
{
   xmlNodePtr node, parent;
   xmlDocPtr doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* first system node */
   if (parent == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), file );
      return -1;
   }

   /* Clear data. */
   memset( efx, 0, sizeof(EffectData) );
   efx->duration = -1.;
   efx->priority = 5;

   xmlr_attr_strd(parent,"name",efx->name);
   if (efx->name == NULL)
      WARN(_("Effect '%s' has invalid or no name"), file);

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      /* Only handle nodes. */
      xml_onlyNodes(node);

      xmlr_strd(node, "description", efx->desc);
      xmlr_strd(node, "overwrite", efx->overwrite);
      xmlr_int(node, "priority", efx->priority);
      xmlr_float(node, "duration", efx->duration);
      if (xml_isNode(node,"icon")) {
         efx->icon = xml_parseTexture( node, "%s", 1, 1, OPENGL_TEX_MIPMAPS );
         continue;
      }

      if (xml_isNode(node,"stats")) {
         xmlNodePtr cur = node->children;
         do {
            ShipStatList *ll;
            xml_onlyNodes(cur);
            /* Stats. */
            ll = ss_listFromXML( cur );
            if (ll != NULL) {
               ll->next    = efx->stats;
               efx->stats = ll;
               continue;
            }
            WARN(_("Effect '%s' has unknown node '%s'"), efx->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      WARN(_("Effect '%s' has unknown node '%s'"),efx->name, node->name);
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
if (o) WARN( _("Effect '%s' missing/invalid '%s' element"), efx->name, s) /**< Define to help check for data errors. */
   MELEMENT(efx->name==NULL,"name");
   MELEMENT(efx->desc==NULL,"description");
   MELEMENT(efx->duration<0.,"duration");
   MELEMENT(efx->icon==NULL,"icon");
#undef MELEMENT

   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Loads all the effects.
 *
 *    @return 0 on success
 */
int effect_load (void)
{
   int ne;
   Uint32 time = SDL_GetTicks();
   char **effect_files = ndata_listRecursive( EFFECT_DATA_PATH );
   effect_list = array_create( EffectData );

   for (int i=0; i<array_size(effect_files); i++) {
      if (ndata_matchExt( effect_files[i], "xml" )) {
         int ret = effect_parse( &array_grow(&effect_list), effect_files[i] );
         if (ret < 0) {
            int n = array_size( effect_list );
            array_erase( &effect_list, &effect_list[n-1], &effect_list[n] );
         }
      }
      free( effect_files[i] );
   }
   array_free( effect_files );

   ne = array_size(effect_list);
   qsort( effect_list, ne, sizeof(EffectData), effect_cmp );

   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_( "Loaded %d Effect in %.3f s", "Loaded %d Effects in %.3f s", ne ), ne, time/1000. );
   }
   else
      DEBUG( n_( "Loaded %d Effect", "Loaded %d Effects", ne ), ne );

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
      free( e->overwrite );
      gl_freeTexture( e->icon );
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
   if (n>0)
      gui_updateEffects();
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
   Effect *e = NULL;

   /* See if we should overwrite. */
   if (efx->overwrite != NULL) {
      for (int i=0; i<array_size(*efxlist); i++) {
         Effect *el = &(*efxlist)[i];
         if ((el->data->overwrite!=NULL) &&
               (efx->priority <= el->data->priority) &&
               (strcmp(efx->overwrite, el->data->overwrite)==0)) {
            e = el;
            break;
         }
      }
   }

   /* Add new effect if necessary. */
   if (e==NULL)
      e = &array_grow( efxlist );
   e->data  = efx;
   e->timer = efx->duration;
   qsort( efxlist, array_size(efxlist), sizeof(Effect), effect_cmpTimer );
   gui_updateEffects();
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
