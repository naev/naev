/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file effect.c
 *
 * @brief Handles pilot effects.
 */
/** @cond */
#include <SDL3/SDL_timer.h>

#include "naev.h"
/** @endcond */

#include "effect.h"

#include "array.h"
#include "conf.h"
#include "gui.h"
#include "log.h"
#include "ndata.h"
#include "nlua_pilot.h"
#include "nxml.h"
#include "rng.h"

static EffectData *effect_list = NULL; /* List of all available effects. */

/**
 * @brief Compares effects based on name.
 */
static int effect_cmp( const void *p1, const void *p2 )
{
   const EffectData *e1 = p1;
   const EffectData *e2 = p2;
   return strcmp( e1->name, e2->name );
}

/**
 * @brief Compares effects based on priority, then timer.
 */
static int effect_cmpTimer( const void *p1, const void *p2 )
{
   const Effect *e1 = p1;
   const Effect *e2 = p2;
   if ( e1->data->priority < e2->data->priority )
      return -1;
   else if ( e1->data->priority > e2->data->priority )
      return +1;
   return e1->timer - e2->timer;
}

/**
 * @brief Parses an effect.
 */
static int effect_parse( EffectData *efx, const char *file )
{
   xmlNodePtr node, parent;
   xmlDocPtr  doc = xml_parsePhysFS( file );
   if ( doc == NULL )
      return -1;

   parent = doc->xmlChildrenNode; /* first system node */
   if ( parent == NULL ) {
      WARN( _( "Malformed '%s' file: does not contain elements" ), file );
      return -1;
   }

   /* Clear data. */
   memset( efx, 0, sizeof( EffectData ) );
   efx->duration   = -1.;
   efx->priority   = 5;
   efx->lua_env    = NULL;
   efx->lua_add    = LUA_NOREF;
   efx->lua_extend = LUA_NOREF;
   efx->lua_remove = LUA_NOREF;

   xmlr_attr_strd( parent, "name", efx->name );
   if ( efx->name == NULL )
      WARN( _( "Effect '%s' has invalid or no name" ), file );

   node = parent->xmlChildrenNode;
   do { /* load all the data */
      /* Only handle nodes. */
      xml_onlyNodes( node );

      xmlr_strd( node, "description", efx->desc );
      xmlr_strd( node, "overwrite", efx->overwrite );
      xmlr_int( node, "priority", efx->priority );
      xmlr_float( node, "duration", efx->duration );
      if ( xml_isNode( node, "buff" ) ) {
         efx->flags |= EFFECT_BUFF;
         continue;
      }
      if ( xml_isNode( node, "debuff" ) ) {
         efx->flags |= EFFECT_DEBUFF;
         continue;
      }
      if ( xml_isNode( node, "icon" ) ) {
         efx->icon = xml_parseTexture( node, "%s", 1, 1, OPENGL_TEX_MIPMAPS );
         continue;
      }
      if ( xml_isNode( node, "shader" ) ) {
         char *vertex, *img;
         xmlr_attr_strd( node, "vertex", vertex );
         if ( vertex == NULL )
            vertex = strdup( "effect.vert" );
         else
            efx->flags |= EFFECT_VERTEX;
         xmlr_attr_strd( node, "img", img );
         if ( img != NULL ) {
            efx->img =
               gl_newImage( img, OPENGL_TEX_MIPMAPS | OPENGL_TEX_CLAMP_ALPHA );
            free( img );
         }
         efx->program = gl_program_vert_frag( vertex, xml_get( node ) );
         free( vertex );
         efx->vertex     = glGetAttribLocation( efx->program, "vertex" );
         efx->projection = glGetUniformLocation( efx->program, "projection" );
         efx->tex_mat    = glGetUniformLocation( efx->program, "tex_mat" );
         efx->dimensions = glGetUniformLocation( efx->program, "dimensions" );
         efx->u_r        = glGetUniformLocation( efx->program, "u_r" );
         efx->u_tex      = glGetUniformLocation( efx->program, "u_tex" );
         efx->u_timer    = glGetUniformLocation( efx->program, "u_timer" );
         efx->u_elapsed  = glGetUniformLocation( efx->program, "u_elapsed" );
         efx->u_dir      = glGetUniformLocation( efx->program, "u_dir" );
         efx->u_img      = glGetUniformLocation( efx->program, "u_img" );
         continue;
      }
      if ( xml_isNode( node, "lua" ) ) {
         nlua_env   *env;
         size_t      sz;
         const char *filename = xml_get( node );
         char       *dat      = ndata_read( filename, &sz );
         if ( dat == NULL ) {
            WARN( _( "Effect '%s' failed to read Lua '%s'!" ), efx->name,
                  filename );
            continue;
         }

         env = nlua_newEnv( filename );
         nlua_loadStandard( env );
         if ( nlua_dobufenv( env, dat, sz, filename ) != 0 ) {
            WARN( _( "Effect '%s' Lua error:\n%s" ), efx->name,
                  lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
            nlua_freeEnv( env );
            free( dat );
            continue;
         }
         efx->lua_env    = env;
         efx->lua_add    = nlua_refenvtype( env, "add", LUA_TFUNCTION );
         efx->lua_extend = nlua_refenvtype( env, "extend", LUA_TFUNCTION );
         efx->lua_remove = nlua_refenvtype( env, "remove", LUA_TFUNCTION );
         free( dat );
         continue;
      }

      if ( xml_isNode( node, "stats" ) ) {
         efx->stats = ss_listFromXML( node );
         continue;
      }
      // cppcheck-suppress nullPointerRedundantCheck
      WARN( _( "Effect '%s' has unknown node '%s'" ), efx->name, node->name );
   } while ( xml_nextNode( node ) );

   /* See if it is damaging. */
   efx->damaging = 0;
   if ( ( efx->stats != NULL ) && ( efx->flags & EFFECT_DEBUFF ) ) {
      ShipStats stats;
      ss_statsInit( &stats );
      ss_statsMergeFromList( &stats, efx->stats, 1 );
      efx->damaging = ( stats.damage > 0. ) || ( stats.disable > 0. );
   }

#define MELEMENT( o, s )                                                       \
   if ( o )                                                                    \
   WARN( _( "Effect '%s' missing/invalid '%s' element" ), efx->name,           \
         s ) /**< Define to help check for data errors. */
   MELEMENT( efx->name == NULL, "name" );
   MELEMENT( efx->desc == NULL, "description" );
   MELEMENT( efx->duration < 0., "duration" );
   MELEMENT( efx->icon == NULL, "icon" );
#undef MELEMENT

   xmlFreeDoc( doc );

   return 0;
}

/**
 * @brief Loads all the effects.
 *
 *    @return 0 on success
 */
int effect_load( void )
{
   int ne;
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */
   char **effect_files = ndata_listRecursive( EFFECT_DATA_PATH );
   effect_list         = array_create( EffectData );

   for ( int i = 0; i < array_size( effect_files ); i++ ) {
      if ( ndata_matchExt( effect_files[i], "xml" ) ) {
         EffectData ed;
         int        ret = effect_parse( &ed, effect_files[i] );
         if ( ret == 0 )
            array_push_back( &effect_list, ed );
      }
      free( effect_files[i] );
   }
   array_free( effect_files );

   ne = array_size( effect_list );
   qsort( effect_list, ne, sizeof( EffectData ), effect_cmp );

#if DEBUGGING
   /* Check to see if there are name collisions. */
   for ( int i = 1; i < array_size( effect_list ); i++ )
      if ( strcmp( effect_list[i - 1].name, effect_list[i].name ) == 0 )
         WARN( _( "Duplicated effect name '%s' detected!" ),
               effect_list[i].name );

   if ( conf.devmode ) {
      time = SDL_GetTicks() - time;
      DEBUG(
         n_( "Loaded %d Effect in %.3f s", "Loaded %d Effects in %.3f s", ne ),
         ne, time / 1000. );
   } else
      DEBUG( n_( "Loaded %d Effect", "Loaded %d Effects", ne ), ne );
#endif /* DEBUGGING */

   return 0;
}

/**
 * @brief Gets rid of all the effects.
 */
void effect_exit( void )
{
   for ( int i = 0; i < array_size( effect_list ); i++ ) {
      EffectData *e = &effect_list[i];
      nlua_freeEnv( e->lua_env );
      free( e->name );
      free( e->desc );
      free( e->overwrite );
      gl_freeTexture( e->icon );
      gl_freeTexture( e->img );
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
   const EffectData k = { .name = (char *)name };
   EffectData      *e = bsearch( &k, effect_list, array_size( effect_list ),
                                 sizeof( EffectData ), effect_cmp );
   if ( e == NULL )
      WARN( _( "Trying to get unknown effect data '%s'!" ), name );
   return e;
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
   for ( int i = array_size( *efxlist ) - 1; i >= 0; i-- ) {
      Effect *e = &( *efxlist )[i];
      e->timer -= dt;
      e->elapsed += dt;
      if ( e->timer > 0. )
         continue;

      /* Run Lua if necessary. */
      if ( e->data->lua_remove != LUA_NOREF ) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_remove ); /* f */
         lua_pushpilot( naevL, e->parent );
         if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
            WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
                  "remove", lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }

      /* Get rid of it. */
      array_erase( efxlist, e, e + 1 );
      n++;
   }
   if ( n > 0 )
      gui_updateEffects();
   return n;
}

/**
 * @brief Adds an effect to an effect list.
 *
 *    @param efxlist List of effects.
 *    @param efx Effect to add.
 *    @param duration Duration of the effect or set to negative for default.
 *    @param strength Scaling strength of the effect.
 *    @param parent Pilot the effect is being added to.
 *    @param applicator Pilot applying the effect or 0 if none.
 *    @return 0 on success.
 */
int effect_add( Effect **efxlist, const EffectData *efx, double duration,
                double strength, unsigned int parent, unsigned int applicator )
{
   Effect *e         = NULL;
   int     overwrite = 0;
   duration          = ( duration > 0. ) ? duration : efx->duration;

   if ( *efxlist == NULL )
      *efxlist = array_create( Effect );

   /* See if we should overwrite. */
   if ( efx->overwrite != NULL ) {
      for ( int i = 0; i < array_size( *efxlist ); i++ ) {
         Effect *el = &( *efxlist )[i];
         if ( ( el->data->overwrite != NULL ) &&
              ( efx->priority <= el->data->priority ) &&
              ( strcmp( efx->overwrite, el->data->overwrite ) == 0 ) ) {
            e = el;
            if ( e->data == efx ) {
               /* Case the effect is weaker when both are the same, we just
                * ignore. */
               if ( el->strength > strength )
                  return 0;
               /* Case the base effect has a longer timer with same strength we
                * ignore. */
               if ( ( fabs( el->strength - strength ) < DOUBLE_TOL ) &&
                    ( el->timer > duration ) )
                  return 0;
               /* Procede to overwrite. */
               overwrite = 1;
            } else {
               /* Here we remove the effect and replace it as they overlap while
                * not being exactly the same. Can't do a strength check because
                * they may not be comparable. */
               if ( e->data->lua_remove != LUA_NOREF ) {
                  lua_rawgeti( naevL, LUA_REGISTRYINDEX,
                               e->data->lua_remove ); /* f */
                  lua_pushpilot( naevL, e->parent );
                  if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
                     WARN( _( "Effect '%s' failed to run '%s':\n%s" ),
                           e->data->name, "remove", lua_tostring( naevL, -1 ) );
                     lua_pop( naevL, 1 );
                  }
               }
            }
            break;
         }
      }
   }

   /* Add new effect if necessary. */
   if ( e == NULL ) {
      e          = &array_grow( efxlist );
      e->elapsed = 0.; /* Don't 0. when overwriting. */
      e->r       = RNGF();
   }
   e->data       = efx;
   e->duration   = duration;
   e->timer      = e->duration;
   e->strength   = strength;
   e->parent     = parent;
   e->applicator = applicator;

   /* Run Lua if necessary. */
   if ( overwrite ) {
      if ( efx->lua_extend != LUA_NOREF ) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_extend ); /* f */
         lua_pushpilot( naevL, e->parent );
         if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
            WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
                  "extend", lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }
   } else {
      if ( efx->lua_add != LUA_NOREF ) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_add ); /* f */
         lua_pushpilot( naevL, e->parent );
         if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
            WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
                  "add", lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }
   }

   /* Sort and update. */
   qsort( *efxlist, array_size( *efxlist ), sizeof( Effect ), effect_cmpTimer );
   gui_updateEffects();
   return 0;
}

/**
 * @brief Removes an effect from an effect list by index.
 *
 *    @param efxlist List of effects.
 *    @param idx Index to remove.
 *    @return Number of instances removed.
 */
int effect_rm( Effect **efxlist, int idx )
{
   const Effect *e;

   if ( ( idx < 0 ) || ( idx > array_size( efxlist ) ) ) {
      WARN( _( "Trying to remove invalid effect ID!" ) );
      return -1;
   }
   e = &( *efxlist )[idx];

   /* Run Lua if necessary. */
   if ( e->data->lua_remove != LUA_NOREF ) {
      lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_remove ); /* f */
      lua_pushpilot( naevL, e->parent );
      if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
         WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
               "remove", lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
      }
   }
   array_erase( efxlist, &e[0], &e[1] );
   gui_updateEffects();
   return 1;
}

/**
 * @brief Removes an effect type from an effect list.
 *
 *    @param efxlist List of effects.
 *    @param efx Effect type to remove.
 *    @param all Whether or not to remove all instances.
 *    @return Number of instances removed.
 */
int effect_rmType( Effect **efxlist, const EffectData *efx, int all )
{
   int ret = 0;
   for ( int i = array_size( *efxlist ) - 1; i >= 0; i-- ) {
      const Effect *e = &( *efxlist )[i];
      if ( e->data != efx )
         continue;
      /* Run Lua if necessary. */
      if ( e->data->lua_remove != LUA_NOREF ) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_remove ); /* f */
         lua_pushpilot( naevL, e->parent );
         if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
            WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
                  "remove", lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }
      array_erase( efxlist, &e[0], &e[1] );
      if ( !all ) {
         gui_updateEffects();
         return 1;
      }
      ret++;
   }
   if ( ret > 0 )
      gui_updateEffects();
   return ret;
}

/**
 * @brief Clears specific types of effects.
 *
 *    @param efxlist List of effects.
 *    @param debuffs Whether or not to clear debuffs.
 *    @param buffs Whether or not to clear buffs.
 *    @param others Whether or not to clear other effects.
 */
void effect_clearSpecific( Effect **efxlist, int debuffs, int buffs,
                           int others )
{
   int n = 0;
   for ( int i = array_size( *efxlist ) - 1; i >= 0; i-- ) {
      const Effect *e = &( *efxlist )[i];

      /* See if should be eliminated. */
      if ( e->data->flags & EFFECT_BUFF ) {
         if ( !buffs )
            continue;
      } else if ( e->data->flags & EFFECT_DEBUFF ) {
         if ( !debuffs )
            continue;
      } else {
         if ( !others )
            continue;
      }

      /* Run Lua if necessary. */
      if ( e->data->lua_remove != LUA_NOREF ) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_remove ); /* f */
         lua_pushpilot( naevL, e->parent );
         if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
            WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
                  "remove", lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }

      /* Clear effect. */
      array_erase( efxlist, &e[0], &e[1] );
      n++;
   }
   if ( n > 0 )
      gui_updateEffects();
}

/**
 * @brief Clears an effect list, removing all active effects.
 *
 *    @param efxlist List of effects.
 */
void effect_clear( Effect **efxlist )
{
   for ( int i = 0; i < array_size( *efxlist ); i++ ) {
      const Effect *e = &( *efxlist )[i];
      /* Run Lua if necessary. */
      if ( e->data->lua_remove != LUA_NOREF ) {
         lua_rawgeti( naevL, LUA_REGISTRYINDEX, e->data->lua_remove ); /* f */
         lua_pushpilot( naevL, e->parent );
         if ( nlua_pcall( e->data->lua_env, 1, 0 ) ) {
            WARN( _( "Effect '%s' failed to run '%s':\n%s" ), e->data->name,
                  "remove", lua_tostring( naevL, -1 ) );
            lua_pop( naevL, 1 );
         }
      }
   }
   array_erase( efxlist, array_begin( *efxlist ), array_end( *efxlist ) );
   gui_updateEffects();
}

/**
 * @brief Updates shipstats from effect list.
 *
 *    @param s Stats to update.
 *    @param efxlist List of effects.
 */
void effect_compute( ShipStats *s, const Effect *efxlist )
{
   ShipStats stats;
   ss_statsInit( &stats );
   for ( int i = 0; i < array_size( efxlist ); i++ ) {
      const Effect *e = &efxlist[i];
      ss_statsMergeFromListScale( &stats, e->data->stats, e->strength, 0 );
   }
   ss_statsMerge( s, &stats, 1 );
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
