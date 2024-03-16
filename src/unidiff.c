/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file unidiff.c
 *
 * @brief Handles the application and removal of 'diffs' to the universe.
 *
 * Diffs allow changing spobs, factions, etc... in the universe.
 *  These are meant to be applied after the player triggers them, mostly
 *  through missions.
 */
/** @cond */
#include <stdlib.h>
/** @endcond */

#include "unidiff.h"

#include "array.h"
#include "conf.h"
#include "economy.h"
#include "log.h"
#include "map_overlay.h"
#include "ndata.h"
#include "nxml.h"
#include "player.h"
#include "safelanes.h"
#include "space.h"

/**
 * @brief Universe diff filepath list.
 */
typedef struct UniDiffData_ {
   char *name;     /**< Name of the diff (read from XML). */
   char *filename; /**< Filename of the diff. */
} UniDiffData_t;
static UniDiffData_t *diff_available = NULL; /**< Available diffs. */

/**
 * @enum UniHunkTargetType_t
 *
 * @brief Represents the possible hunk targets.
 */
typedef enum UniHunkTargetType_ {
   HUNK_TARGET_NONE,
   HUNK_TARGET_SYSTEM,
   HUNK_TARGET_SPOB,
   HUNK_TARGET_TECH,
   HUNK_TARGET_FACTION,
} UniHunkTargetType_t;

/**
 * @struct UniHunkTarget_t
 *
 * @brief Represents the hunk's target.
 */
typedef struct UniHunkTarget_ {
   UniHunkTargetType_t type; /**< Type of hunk target. */
   union {
      char *name; /**< Name of the target. */
   } u;           /**< Union of possible target types. */
} UniHunkTarget_t;

/**
 * @enum UniHunkType_t
 *
 * @brief Represents the different type of hunk actions.
 */
typedef enum UniHunkType_ {
   HUNK_TYPE_NONE,
   /* Target should be system. */
   HUNK_TYPE_SPOB_ADD,
   HUNK_TYPE_SPOB_REMOVE,
   HUNK_TYPE_VSPOB_ADD,
   HUNK_TYPE_VSPOB_REMOVE,
   HUNK_TYPE_JUMP_ADD,
   HUNK_TYPE_JUMP_REMOVE,
   HUNK_TYPE_SSYS_BACKGROUND,
   HUNK_TYPE_SSYS_BACKGROUND_REVERT, /* For internal usage. */
   HUNK_TYPE_SSYS_FEATURES,
   HUNK_TYPE_SSYS_FEATURES_REVERT, /* For internal usage. */
   /* Target should be tech. */
   HUNK_TYPE_TECH_ADD,
   HUNK_TYPE_TECH_REMOVE,
   /* Target should be spob. */
   HUNK_TYPE_SPOB_FACTION,
   HUNK_TYPE_SPOB_FACTION_REMOVE, /* For internal usage. */
   HUNK_TYPE_SPOB_POPULATION,
   HUNK_TYPE_SPOB_POPULATION_REMOVE, /* For internal usage. */
   HUNK_TYPE_SPOB_DISPLAYNAME,
   HUNK_TYPE_SPOB_DISPLAYNAME_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_DESCRIPTION,
   HUNK_TYPE_SPOB_DESCRIPTION_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_BAR,
   HUNK_TYPE_SPOB_BAR_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_SERVICE_ADD,
   HUNK_TYPE_SPOB_SERVICE_REMOVE,
   HUNK_TYPE_SPOB_NOMISNSPAWN_ADD,
   HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE,
   HUNK_TYPE_SPOB_TECH_ADD,
   HUNK_TYPE_SPOB_TECH_REMOVE,
   HUNK_TYPE_SPOB_TAG_ADD,
   HUNK_TYPE_SPOB_TAG_REMOVE,
   HUNK_TYPE_SPOB_SPACE,
   HUNK_TYPE_SPOB_SPACE_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_EXTERIOR,
   HUNK_TYPE_SPOB_EXTERIOR_REVERT, /* For internal usage. */
   HUNK_TYPE_SPOB_LUA,
   HUNK_TYPE_SPOB_LUA_REVERT, /* For internal usage. */
   /* Target should be faction. */
   HUNK_TYPE_FACTION_VISIBLE,
   HUNK_TYPE_FACTION_INVISIBLE,
   HUNK_TYPE_FACTION_ALLY,
   HUNK_TYPE_FACTION_ENEMY,
   HUNK_TYPE_FACTION_NEUTRAL,
   HUNK_TYPE_FACTION_REALIGN, /* For internal usage. */
} UniHunkType_t;

/**
 * @struct UniHunk_t
 *
 * @brief Represents a single hunk in the diff.
 */
typedef struct UniHunk_ {
   UniHunkTarget_t target; /**< Hunk's target. */

   UniHunkType_t type; /**< Type of hunk it is. */
   xmlNodePtr    node; /**< Parent node. */
   union {
      char *name;
      int   data;
   } u; /**< Actual data to patch. */
   union {
      const char *name; /* We just save the pointer, so keep as const. */
      int         data;
   } o; /** Old data to possibly replace. */
} UniHunk_t;

/**
 * @struct UniDiff_t
 *
 * @brief Represents each Universe Diff.
 */
typedef struct UniDiff_ {
   char      *name;    /**< Name of the diff. */
   UniHunk_t *applied; /**< Applied hunks. */
   UniHunk_t *failed;  /**< Failed hunks. */
} UniDiff_t;

/*
 * Diff stack.
 */
static UniDiff_t *diff_stack = NULL; /**< Currently applied universe diffs. */

/* Useful variables. */
static int diff_universe_changed =
   0; /**< Whether or not the universe changed. */
static int         diff_universe_defer = 0; /**< Defers changes to later. */
static const char *diff_nav_spob =
   NULL; /**< Stores the player's spob target if necessary. */
static const char *diff_nav_hyperspace =
   NULL; /**< Stores the player's hyperspace target if necessary. */

/*
 * Prototypes.
 */
static int diff_applyInternal( const char *name, int oneshot );
NONNULL( 1 ) static UniDiff_t *diff_get( const char *name );
static UniDiff_t *diff_newDiff( void );
static int        diff_removeDiff( UniDiff_t *diff );
static int        diff_patchSystem( UniDiff_t *diff, xmlNodePtr node );
static int        diff_patchTech( UniDiff_t *diff, xmlNodePtr node );
static int        diff_patch( xmlNodePtr parent );
static int        diff_patchHunk( UniHunk_t *hunk );
static void       diff_hunkFailed( UniDiff_t *diff, const UniHunk_t *hunk );
static void       diff_hunkSuccess( UniDiff_t *diff, const UniHunk_t *hunk );
static void       diff_cleanup( UniDiff_t *diff );
static void       diff_cleanupHunk( UniHunk_t *hunk );
/* Misc. */
static int diff_checkUpdateUniverse( void );
/* Externed. */
int diff_save( xmlTextWriterPtr writer ); /**< Used in save.c */
int diff_load( xmlNodePtr parent );       /**< Used in save.c */

/**
 * @brief Simple comparison for UniDiffData_t based on name.
 */
static int diff_cmp( const void *p1, const void *p2 )
{
   const UniDiffData_t *d1, *d2;
   d1 = (const UniDiffData_t *)p1;
   d2 = (const UniDiffData_t *)p2;
   return strcmp( d1->name, d2->name );
}

/**
 * @brief Loads available universe diffs.
 *
 *    @return 0 on success.
 */
int diff_loadAvailable( void )
{
#if DEBUGGING
   Uint32 time = SDL_GetTicks();
#endif /* DEBUGGING */
   char **diff_files = ndata_listRecursive( UNIDIFF_DATA_PATH );
   diff_available =
      array_create_size( UniDiffData_t, array_size( diff_files ) );
   for ( int i = 0; i < array_size( diff_files ); i++ ) {
      xmlDocPtr      doc;
      xmlNodePtr     node;
      UniDiffData_t *diff;

      /* Parse the header. */
      doc = xml_parsePhysFS( diff_files[i] );
      if ( doc == NULL ) {
         free( diff_files[i] );
         continue;
      }

      node = doc->xmlChildrenNode;
      if ( !xml_isNode( node, "unidiff" ) ) {
         WARN( _( "Malformed XML header for '%s' UniDiff: missing root element "
                  "'%s'" ),
               diff_files[i], "unidiff" );
         xmlFreeDoc( doc );
         free( diff_files[i] );
         continue;
      }

      diff           = &array_grow( &diff_available );
      diff->filename = diff_files[i];
      xmlr_attr_strd( node, "name", diff->name );
      xmlFreeDoc( doc );
   }
   array_free( diff_files );
   array_shrink( &diff_available );

   /* Sort and warn about duplicates. */
   qsort( diff_available, array_size( diff_available ), sizeof( UniDiffData_t ),
          diff_cmp );
   for ( int i = 0; i < array_size( diff_available ) - 1; i++ ) {
      UniDiffData_t       *d  = &diff_available[i];
      const UniDiffData_t *dn = &diff_available[i + 1];
      if ( strcmp( d->name, dn->name ) == 0 )
         WARN( _( "Two unidiff have the same name '%s'!" ), d->name );
   }

#if DEBUGGING
   if ( conf.devmode ) {
      DEBUG( n_( "Loaded %d UniDiff in %.3f s", "Loaded %d UniDiffs in %.3f s",
                 array_size( diff_available ) ),
             array_size( diff_available ), ( SDL_GetTicks() - time ) / 1000. );
   } else
      DEBUG( n_( "Loaded %d UniDiff", "Loaded %d UniDiffs",
                 array_size( diff_available ) ),
             array_size( diff_available ) );
#endif /* DEBUGGING */

   return 0;
}

/**
 * @brief Checks if a diff is currently applied.
 *
 *    @param name Diff to check.
 *    @return 0 if it's not applied, 1 if it is.
 */
int diff_isApplied( const char *name )
{
   if ( diff_get( name ) != NULL )
      return 1;
   return 0;
}

/**
 * @brief Gets a diff by name.
 *
 *    @param name Name of the diff to get.
 *    @return The diff if found or NULL if not found.
 */
static UniDiff_t *diff_get( const char *name )
{
   for ( int i = 0; i < array_size( diff_stack ); i++ )
      if ( strcmp( diff_stack[i].name, name ) == 0 )
         return &diff_stack[i];
   return NULL;
}

/**
 * @brief Applies a diff to the universe.
 *
 *    @param name Diff to apply.
 *    @return 0 on success.
 */
int diff_apply( const char *name )
{
   diff_nav_hyperspace = NULL;
   diff_nav_spob       = NULL;
   if ( player.p ) {
      if ( player.p->nav_hyperspace >= 0 )
         diff_nav_hyperspace =
            cur_system->jumps[player.p->nav_hyperspace].target->name;
      if ( player.p->nav_spob >= 0 )
         diff_nav_spob = cur_system->spobs[player.p->nav_spob]->name;
   }
   return diff_applyInternal( name, 1 );
}

/**
 * @brief Applies a diff to the universe.
 *
 *    @param name Diff to apply.
 *    @param oneshot Whether or not this diff should be applied as a single
 * one-shot diff.
 *    @return 0 on success.
 */
static int diff_applyInternal( const char *name, int oneshot )
{
   xmlNodePtr     node;
   xmlDocPtr      doc;
   UniDiffData_t *d;

   /* Check if already applied. */
   if ( diff_isApplied( name ) )
      return 0;

   /* Reset change variable. */
   if ( oneshot && !diff_universe_defer )
      diff_universe_changed = 0;

   const UniDiffData_t q = { .name = (char *)name };
   d = bsearch( &q, diff_available, array_size( diff_available ),
                sizeof( UniDiffData_t ), diff_cmp );
   if ( d == NULL ) {
      WARN( _( "UniDiff '%s' not found in %s!" ), name, UNIDIFF_DATA_PATH );
      return -1;
   }

   doc = xml_parsePhysFS( d->filename );

   node = doc->xmlChildrenNode;
   if ( strcmp( (char *)node->name, "unidiff" ) ) {
      ERR( _( "Malformed unidiff file: missing root element 'unidiff'" ) );
      return 0;
   }

   /* Apply it. */
   diff_patch( node );

   xmlFreeDoc( doc );

   /* Update universe. */
   if ( oneshot )
      diff_checkUpdateUniverse();

   return 0;
}

/**
 * @brief Patches a system.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the system.
 *    @return 0 on success.
 */
static int diff_patchSystem( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;
   char      *buf;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_SYSTEM;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has a system node without a 'name' tag, not "
               "applying." ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );
      if ( xml_isNode( cur, "spob" ) ) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Get the spob to modify. */
         xmlr_attr_strd( cur, "name", hunk.u.name );

         /* Get the type. */
         if ( strcmp( buf, "add" ) == 0 )
            hunk.type = HUNK_TYPE_SPOB_ADD;
         else if ( strcmp( buf, "remove" ) == 0 )
            hunk.type = HUNK_TYPE_SPOB_REMOVE;
         else
            WARN( _( "Unidiff '%s': Unknown hunk type '%s' for spob '%s'." ),
                  diff->name, buf, hunk.u.name );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "spob_virtual" ) ) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Get the spob to modify. */
         xmlr_attr_strd( cur, "name", hunk.u.name );

         /* Get the type. */
         if ( strcmp( buf, "add" ) == 0 )
            hunk.type = HUNK_TYPE_VSPOB_ADD;
         else if ( strcmp( buf, "remove" ) == 0 )
            hunk.type = HUNK_TYPE_VSPOB_REMOVE;
         else
            WARN( _( "Unidiff '%s': Unknown hunk type '%s' for virtual spob "
                     "'%s'." ),
                  diff->name, buf, hunk.u.name );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "jump" ) ) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Get the jump point to modify. */
         xmlr_attr_strd( cur, "target", hunk.u.name );

         /* Get the type. */
         if ( strcmp( buf, "add" ) == 0 )
            hunk.type = HUNK_TYPE_JUMP_ADD;
         else if ( strcmp( buf, "remove" ) == 0 )
            hunk.type = HUNK_TYPE_JUMP_REMOVE;
         else
            WARN( _( "Unidiff '%s': Unknown hunk type '%s' for jump '%s'." ),
                  diff->name, buf, hunk.u.name );

         hunk.node = cur;

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "background" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SSYS_BACKGROUND;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "features" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SSYS_FEATURES;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name,
            node->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Patches a tech.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the tech.
 *    @return 0 on success.
 */
static int diff_patchTech( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_TECH;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has an target node without a 'name' tag" ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );
      if ( xml_isNode( cur, "add" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_TECH_ADD;

         /* Get the data. */
         hunk.u.name = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "remove" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_TECH_REMOVE;

         /* Get the data. */
         hunk.u.name = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name,
            node->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Patches a spob.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the spob.
 *    @return 0 on success.
 */
static int diff_patchSpob( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_SPOB;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has an target node without a 'name' tag" ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );
      if ( xml_isNode( cur, "faction" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_FACTION;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "population" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_POPULATION;
         hunk.u.data        = xml_getUInt( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "displayname" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_DISPLAYNAME;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "description" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_DESCRIPTION;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "bar" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_BAR;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "service_add" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_SERVICE_ADD;
         hunk.u.data        = spob_getService( xml_get( cur ) );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "service_remove" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_SERVICE_REMOVE;
         hunk.u.data        = spob_getService( xml_get( cur ) );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "nomissionspawn_add" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_NOMISNSPAWN_ADD;

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "nomissionspawn_remove" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE;

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "tech_add" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_TECH_ADD;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "tech_remove" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_TECH_REMOVE;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "tag_add" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_TAG_ADD;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "tag_remove" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_TAG_REMOVE;
         hunk.u.name        = xml_getStrd( cur );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "gfx_space" ) ) {
         char str[PATH_MAX];
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_SPACE;
         snprintf( str, sizeof( str ), SPOB_GFX_SPACE_PATH "%s",
                   xml_get( cur ) );
         hunk.u.name = strdup( str );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "gfx_exterior" ) ) {
         char str[PATH_MAX];
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_EXTERIOR;
         snprintf( str, sizeof( str ), SPOB_GFX_EXTERIOR_PATH "%s",
                   xml_get( cur ) );
         hunk.u.name = strdup( str );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "lua" ) ) {
         char *str;
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );
         hunk.type          = HUNK_TYPE_SPOB_LUA;
         str                = xml_get( cur );
         if ( str != NULL )
            hunk.u.name = strdup( str );
         else
            hunk.u.name = NULL;

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name, cur->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Patches a faction.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the spob.
 *    @return 0 on success.
 */
static int diff_patchFaction( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t  base, hunk;
   xmlNodePtr cur;
   char      *buf;

   /* Set the target. */
   memset( &base, 0, sizeof( UniHunk_t ) );
   base.target.type = HUNK_TARGET_FACTION;
   xmlr_attr_strd( node, "name", base.target.u.name );
   if ( base.target.u.name == NULL ) {
      WARN( _( "Unidiff '%s' has an target node without a 'name' tag" ),
            diff->name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );
      if ( xml_isNode( cur, "visible" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Faction type is constant. */
         hunk.type = HUNK_TYPE_FACTION_VISIBLE;

         /* There is no name. */
         hunk.u.name = NULL;

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "invisible" ) ) {
         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Faction type is constant. */
         hunk.type = HUNK_TYPE_FACTION_INVISIBLE;

         /* There is no name. */
         hunk.u.name = NULL;

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      } else if ( xml_isNode( cur, "faction" ) ) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type   = base.target.type;
         hunk.target.u.name = strdup( base.target.u.name );

         /* Get the faction to set the association of. */
         xmlr_attr_strd( cur, "name", hunk.u.name );

         /* Get the type. */
         if ( strcmp( buf, "ally" ) == 0 )
            hunk.type = HUNK_TYPE_FACTION_ALLY;
         else if ( strcmp( buf, "enemy" ) == 0 )
            hunk.type = HUNK_TYPE_FACTION_ENEMY;
         else if ( strcmp( buf, "neutral" ) == 0 )
            hunk.type = HUNK_TYPE_FACTION_NEUTRAL;
         else
            WARN( _( "Unidiff '%s': Unknown hunk type '%s' for faction '%s'." ),
                  diff->name, buf, hunk.u.name );

         /* Apply diff. */
         if ( diff_patchHunk( &hunk ) < 0 )
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name,
            node->name );
   } while ( xml_nextNode( cur ) );

   /* Clean up some stuff. */
   free( base.target.u.name );
   base.target.u.name = NULL;

   return 0;
}

/**
 * @brief Actually applies a diff in XML node form.
 *
 *    @param parent Node containing the diff information.
 *    @return 0 on success.
 */
static int diff_patch( xmlNodePtr parent )
{
   UniDiff_t *diff;
   xmlNodePtr node;
   int        nfailed;

   /* Prepare it. */
   diff = diff_newDiff();
   memset( diff, 0, sizeof( UniDiff_t ) );
   xmlr_attr_strd( parent, "name", diff->name );

   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes( node );
      if ( xml_isNode( node, "system" ) ) {
         diff_universe_changed = 1;
         diff_patchSystem( diff, node );
      } else if ( xml_isNode( node, "tech" ) )
         diff_patchTech( diff, node );
      else if ( xml_isNode( node, "spob" ) ) {
         diff_universe_changed = 1;
         diff_patchSpob( diff, node );
      } else if ( xml_isNode( node, "faction" ) ) {
         diff_universe_changed = 1;
         diff_patchFaction( diff, node );
      } else
         WARN( _( "Unidiff '%s' has unknown node '%s'." ), diff->name,
               node->name );
   } while ( xml_nextNode( node ) );

   nfailed = array_size( diff->failed );
   if ( nfailed > 0 ) {
      WARN( n_( "Unidiff '%s' failed to apply %d hunk.",
                "Unidiff '%s' failed to apply %d hunks.", nfailed ),
            diff->name, nfailed );
      for ( int i = 0; i < nfailed; i++ ) {
         UniHunk_t *fail   = &diff->failed[i];
         char      *target = fail->target.u.name;
         switch ( fail->type ) {
         case HUNK_TYPE_SPOB_ADD:
            WARN( _( "   [%s] spob add: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_REMOVE:
            WARN( _( "   [%s] spob remove: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_VSPOB_ADD:
            WARN( _( "   [%s] virtual spob add: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_VSPOB_REMOVE:
            WARN( _( "   [%s] virtual spob remove: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_JUMP_ADD:
            WARN( _( "   [%s] jump add: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_JUMP_REMOVE:
            WARN( _( "   [%s] jump remove: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_TECH_ADD:
            WARN( _( "   [%s] tech add: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_TECH_REMOVE:
            WARN( _( "   [%s] tech remove: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_FACTION:
            WARN( _( "   [%s] spob faction: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_FACTION_REMOVE:
            WARN( _( "   [%s] spob faction removal: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_SPOB_POPULATION:
            WARN( _( "   [%s] spob population: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_POPULATION_REMOVE:
            WARN( _( "   [%s] spob population removal: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_SPOB_DISPLAYNAME:
            WARN( _( "   [%s] spob displayname: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_DISPLAYNAME_REVERT:
            WARN( _( "   [%s] spob displayname revert: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_SPOB_DESCRIPTION:
            WARN( _( "   [%s] spob description: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_DESCRIPTION_REVERT:
            WARN( _( "   [%s] spob description revert: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_SPOB_BAR:
            WARN( _( "   [%s] spob bar: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_BAR_REVERT:
            WARN( _( "   [%s] spob bar revert: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_SPACE:
            WARN( _( "   [%s] spob space: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_SPACE_REVERT:
            WARN( _( "   [%s] spob space revert: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_SPOB_EXTERIOR:
            WARN( _( "   [%s] spob exterior: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_EXTERIOR_REVERT:
            WARN( _( "   [%s] spob exterior revert: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_SPOB_LUA:
            WARN( _( "   [%s] spob lua: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_LUA_REVERT:
            WARN( _( "   [%s] spob lua revert: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_SERVICE_ADD:
            WARN( _( "   [%s] spob service add: '%s'" ), target,
                  spob_getServiceName( fail->u.data ) );
            break;
         case HUNK_TYPE_SPOB_SERVICE_REMOVE:
            WARN( _( "   [%s] spob service remove: '%s'" ), target,
                  spob_getServiceName( fail->u.data ) );
            break;
         case HUNK_TYPE_SPOB_NOMISNSPAWN_ADD:
            WARN( _( "   [%s] spob nomissionspawn add" ), target );
            break;
         case HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE:
            WARN( _( "   [%s] spob nomissionspawn remove" ), target );
            break;
         case HUNK_TYPE_SPOB_TECH_ADD:
            WARN( _( "   [%s] spob tech add: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_TECH_REMOVE:
            WARN( _( "   [%s] spob tech remove: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_TAG_ADD:
            WARN( _( "   [%s] spob tech add: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_SPOB_TAG_REMOVE:
            WARN( _( "   [%s] spob tech remove: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_FACTION_VISIBLE:
            WARN( _( "   [%s] faction visible: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_FACTION_INVISIBLE:
            WARN( _( "   [%s] faction invisible: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_FACTION_ALLY:
            WARN( _( "   [%s] faction set ally: '%s'" ), target, fail->u.name );
            break;
         case HUNK_TYPE_FACTION_ENEMY:
            WARN( _( "   [%s] faction set enemy: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_FACTION_NEUTRAL:
            WARN( _( "   [%s] faction set neutral: '%s'" ), target,
                  fail->u.name );
            break;
         case HUNK_TYPE_FACTION_REALIGN:
            WARN( _( "   [%s] faction alignment reset: '%s'" ), target,
                  fail->u.name );
            break;

         default:
            WARN( _( "   unknown hunk '%d'" ), fail->type );
            break;
         }
      }
   }

   /* Update overlay map just in case. */
   ovr_refresh();
   return 0;
}

/**
 * @brief Applies a hunk and adds it to the diff.
 *
 *    @param hunk Hunk to apply.
 *    @return 0 on success.
 */
static int diff_patchHunk( UniHunk_t *hunk )
{
   Spob       *p;
   StarSystem *ssys;
   int         a, b;

   switch ( hunk->type ) {

   /* Adding an spob. */
   case HUNK_TYPE_SPOB_ADD:
      spob_luaInit( spob_get( hunk->u.name ) );
      diff_universe_changed = 1;
      return system_addSpob( system_get( hunk->target.u.name ), hunk->u.name );
   /* Removing an spob. */
   case HUNK_TYPE_SPOB_REMOVE:
      diff_universe_changed = 1;
      return system_rmSpob( system_get( hunk->target.u.name ), hunk->u.name );

   /* Adding an spob. */
   case HUNK_TYPE_VSPOB_ADD:
      diff_universe_changed = 1;
      return system_addVirtualSpob( system_get( hunk->target.u.name ),
                                    hunk->u.name );
   /* Removing an spob. */
   case HUNK_TYPE_VSPOB_REMOVE:
      diff_universe_changed = 1;
      return system_rmVirtualSpob( system_get( hunk->target.u.name ),
                                   hunk->u.name );

   /* Adding a Jump. */
   case HUNK_TYPE_JUMP_ADD:
      diff_universe_changed = 1;
      return system_addJumpDiff( system_get( hunk->target.u.name ),
                                 hunk->node );
   /* Removing a jump. */
   case HUNK_TYPE_JUMP_REMOVE:
      diff_universe_changed = 1;
      return system_rmJump( system_get( hunk->target.u.name ), hunk->u.name );

   /* Changing system background. */
   case HUNK_TYPE_SSYS_BACKGROUND:
      ssys             = system_get( hunk->target.u.name );
      hunk->o.name     = ssys->background;
      ssys->background = hunk->u.name;
      return 0;
   case HUNK_TYPE_SSYS_BACKGROUND_REVERT:
      ssys             = system_get( hunk->target.u.name );
      ssys->background = (char *)hunk->o.name;
      return 0;

   /* Changing system features designation. */
   case HUNK_TYPE_SSYS_FEATURES:
      ssys           = system_get( hunk->target.u.name );
      hunk->o.name   = ssys->features;
      ssys->features = hunk->u.name;
      return 0;
   case HUNK_TYPE_SSYS_FEATURES_REVERT:
      ssys           = system_get( hunk->target.u.name );
      ssys->features = (char *)hunk->o.name;
      return 0;

   /* Adding a tech. */
   case HUNK_TYPE_TECH_ADD:
      return tech_addItem( hunk->target.u.name, hunk->u.name );
   /* Removing a tech. */
   case HUNK_TYPE_TECH_REMOVE:
      return tech_rmItem( hunk->target.u.name, hunk->u.name );

   /* Changing spob faction. */
   case HUNK_TYPE_SPOB_FACTION:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( p->presence.faction < 0 )
         hunk->o.name = NULL;
      else
         hunk->o.name = faction_name( p->presence.faction );
      diff_universe_changed = 1;
      return spob_setFaction( p, faction_get( hunk->u.name ) );
   case HUNK_TYPE_SPOB_FACTION_REMOVE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      diff_universe_changed = 1;
      if ( hunk->o.name == NULL )
         return spob_setFaction( p, -1 );
      else
         return spob_setFaction( p, faction_get( hunk->o.name ) );

   /* Changing spob population. */
   case HUNK_TYPE_SPOB_POPULATION:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.data  = p->population;
      p->population = hunk->u.data;
      return 0;
   case HUNK_TYPE_SPOB_POPULATION_REMOVE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->population = hunk->o.data;
      return 0;

   /* Changing spob displayname. */
   case HUNK_TYPE_SPOB_DISPLAYNAME:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.name = p->display;
      p->display   = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_DISPLAYNAME_REVERT:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->display = (char *)hunk->o.name;
      return 0;

   /* Changing spob description. */
   case HUNK_TYPE_SPOB_DESCRIPTION:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.name   = p->description;
      p->description = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_DESCRIPTION_REVERT:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->description = (char *)hunk->o.name;
      return 0;

   /* Changing spob bar description. */
   case HUNK_TYPE_SPOB_BAR:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.name       = p->bar_description;
      p->bar_description = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_BAR_REVERT:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->bar_description = (char *)hunk->o.name;
      return 0;

   /* Modifying spob services. */
   case HUNK_TYPE_SPOB_SERVICE_ADD:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( spob_hasService( p, hunk->u.data ) )
         return -1;
      spob_addService( p, hunk->u.data );
      diff_universe_changed = 1;
      return 0;
   case HUNK_TYPE_SPOB_SERVICE_REMOVE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( !spob_hasService( p, hunk->u.data ) )
         return -1;
      spob_rmService( p, hunk->u.data );
      diff_universe_changed = 1;
      return 0;

   /* Modifying mission spawn. */
   case HUNK_TYPE_SPOB_NOMISNSPAWN_ADD:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( spob_isFlag( p, SPOB_NOMISNSPAWN ) )
         return -1;
      spob_setFlag( p, SPOB_NOMISNSPAWN );
      return 0;
   case HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( !spob_isFlag( p, SPOB_NOMISNSPAWN ) )
         return -1;
      spob_rmFlag( p, SPOB_NOMISNSPAWN );
      return 0;

   /* Modifying tech stuff. */
   case HUNK_TYPE_SPOB_TECH_ADD:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( p->tech == NULL )
         p->tech = tech_groupCreate();
      tech_addItemTech( p->tech, hunk->u.name );
      return 0;
   case HUNK_TYPE_SPOB_TECH_REMOVE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      tech_rmItemTech( p->tech, hunk->u.name );
      return 0;

   /* Modifying tag stuff. */
   case HUNK_TYPE_SPOB_TAG_ADD:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      if ( p->tech == NULL )
         p->tech = tech_groupCreate();
      if ( p->tags == NULL )
         p->tags = array_create( char * );
      array_push_back( &p->tags, strdup( hunk->u.name ) );
      return 0;
   case HUNK_TYPE_SPOB_TAG_REMOVE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      a = -1;
      for ( int i = 0; i < array_size( p->tags ); i++ ) {
         if ( strcmp( p->tags[i], hunk->u.name ) == 0 ) {
            a = i;
            break;
         }
      }
      if ( a < 0 )
         return -1; /* Didn't find tag! */
      free( p->tags[a] );
      array_erase( &p->tags, &p->tags[a], &p->tags[a + 1] );
      return 0;

   /* Changing spob space graphics. */
   case HUNK_TYPE_SPOB_SPACE:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.name          = p->gfx_spaceName;
      p->gfx_spaceName      = hunk->u.name;
      diff_universe_changed = 1;
      return 0;
   case HUNK_TYPE_SPOB_SPACE_REVERT:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->gfx_spaceName      = (char *)hunk->o.name;
      diff_universe_changed = 1;
      return 0;

   /* Changing spob exterior graphics. */
   case HUNK_TYPE_SPOB_EXTERIOR:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.name    = p->gfx_exterior;
      p->gfx_exterior = hunk->u.name;
      return 0;
   case HUNK_TYPE_SPOB_EXTERIOR_REVERT:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->gfx_exterior = (char *)hunk->o.name;
      return 0;

   /* Change Lua stuff. */
   case HUNK_TYPE_SPOB_LUA:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      hunk->o.name = p->lua_file;
      p->lua_file  = hunk->u.name;
      spob_luaInit( p );
      diff_universe_changed = 1;
      return 0;
   case HUNK_TYPE_SPOB_LUA_REVERT:
      p = spob_get( hunk->target.u.name );
      if ( p == NULL )
         return -1;
      p->lua_file = (char *)hunk->o.name;
      spob_luaInit( p );
      diff_universe_changed = 1;
      return 0;

   /* Making a faction visible. */
   case HUNK_TYPE_FACTION_VISIBLE:
      return faction_setInvisible( faction_get( hunk->target.u.name ), 0 );
   /* Making a faction invisible. */
   case HUNK_TYPE_FACTION_INVISIBLE:
      return faction_setInvisible( faction_get( hunk->target.u.name ), 1 );
   /* Making two factions allies. */
   case HUNK_TYPE_FACTION_ALLY:
      a = faction_get( hunk->target.u.name );
      b = faction_get( hunk->u.name );
      if ( areAllies( a, b ) )
         hunk->o.data = 'A';
      else if ( areEnemies( a, b ) )
         hunk->o.data = 'E';
      else
         hunk->o.data = 0;
      faction_addAlly( a, b );
      faction_addAlly( b, a );
      return 0;
   /* Making two factions enemies. */
   case HUNK_TYPE_FACTION_ENEMY:
      a = faction_get( hunk->target.u.name );
      b = faction_get( hunk->u.name );
      if ( areAllies( a, b ) )
         hunk->o.data = 'A';
      else if ( areEnemies( a, b ) )
         hunk->o.data = 'E';
      else
         hunk->o.data = 0;
      faction_addEnemy( a, b );
      faction_addEnemy( b, a );
      return 0;
   /* Making two factions neutral (removing enemy/ally statuses). */
   case HUNK_TYPE_FACTION_NEUTRAL:
      a = faction_get( hunk->target.u.name );
      b = faction_get( hunk->u.name );
      if ( areAllies( a, b ) )
         hunk->o.data = 'A';
      else if ( areEnemies( a, b ) )
         hunk->o.data = 'E';
      else
         hunk->o.data = 0;
      faction_rmAlly( a, b );
      faction_rmAlly( b, a );
      faction_rmEnemy( a, b );
      faction_rmEnemy( b, a );
      return 0;
   /* Resetting the alignment state of two factions. */
   case HUNK_TYPE_FACTION_REALIGN:
      a = faction_get( hunk->target.u.name );
      b = faction_get( hunk->u.name );
      if ( hunk->o.data == 'A' ) {
         faction_rmEnemy( a, b );
         faction_rmEnemy( b, a );
         faction_addAlly( a, b );
         faction_addAlly( b, a );
      } else if ( hunk->o.data == 'E' ) {
         faction_rmAlly( a, b );
         faction_rmAlly( b, a );
         faction_addEnemy( a, b );
         faction_addAlly( b, a );
      } else {
         faction_rmAlly( a, b );
         faction_rmAlly( b, a );
         faction_rmEnemy( a, b );
         faction_rmEnemy( b, a );
      }
      return 0;

   default:
      WARN( _( "Unknown hunk type '%d'." ), hunk->type );
      break;
   }

   return -1;
}

/**
 * @brief Adds a hunk to the failed list.
 *
 *    @param diff Diff to add hunk to.
 *    @param hunk Hunk that failed to apply.
 */
static void diff_hunkFailed( UniDiff_t *diff, const UniHunk_t *hunk )
{
   if ( diff == NULL )
      return;
   if ( diff->failed == NULL )
      diff->failed = array_create( UniHunk_t );
   array_grow( &diff->failed ) = *hunk;
}

/**
 * @brief Adds a hunk to the applied list.
 *
 *    @param diff Diff to add hunk to.
 *    @param hunk Hunk that applied correctly.
 */
static void diff_hunkSuccess( UniDiff_t *diff, const UniHunk_t *hunk )
{
   if ( diff == NULL )
      return;
   if ( diff->applied == NULL )
      diff->applied = array_create( UniHunk_t );
   array_grow( &diff->applied ) = *hunk;
}

/**
 * @brief Removes a diff from the universe.
 *
 *    @param name Diff to remove.
 */
void diff_remove( const char *name )
{
   /* Check if already applied. */
   UniDiff_t *diff = diff_get( name );
   if ( diff == NULL )
      return;

   diff_removeDiff( diff );

   diff_checkUpdateUniverse();
}

/**
 * @brief Removes all active diffs. (Call before economy_destroy().)
 */
void diff_clear( void )
{
   while ( array_size( diff_stack ) > 0 )
      diff_removeDiff( &diff_stack[array_size( diff_stack ) - 1] );
   array_free( diff_stack );
   diff_stack = NULL;

   diff_checkUpdateUniverse();
}

/**
 * @brief Clean up after diff_loadAvailable().
 */
void diff_free( void )
{
   diff_clear();
   for ( int i = 0; i < array_size( diff_available ); i++ ) {
      UniDiffData_t *d = &diff_available[i];
      free( d->name );
      free( d->filename );
   }
   array_free( diff_available );
   diff_available = NULL;
}

/**
 * @brief Creates a new UniDiff_t for usage.
 *
 *    @return A newly created UniDiff_t.
 */
static UniDiff_t *diff_newDiff( void )
{
   /* Check if needs initialization. */
   if ( diff_stack == NULL )
      diff_stack = array_create( UniDiff_t );
   return &array_grow( &diff_stack );
}

/**
 * @brief Removes a diff.
 *
 *    @param diff Diff to remove.
 *    @return 0 on success.
 */
static int diff_removeDiff( UniDiff_t *diff )
{
   for ( int i = 0; i < array_size( diff->applied ); i++ ) {
      UniHunk_t hunk = diff->applied[i];
      /* Invert the type for reverting. */
      switch ( hunk.type ) {
      case HUNK_TYPE_SPOB_ADD:
         hunk.type = HUNK_TYPE_SPOB_REMOVE;
         break;
      case HUNK_TYPE_SPOB_REMOVE:
         hunk.type = HUNK_TYPE_SPOB_ADD;
         break;

      case HUNK_TYPE_VSPOB_ADD:
         hunk.type = HUNK_TYPE_VSPOB_REMOVE;
         break;
      case HUNK_TYPE_VSPOB_REMOVE:
         hunk.type = HUNK_TYPE_VSPOB_ADD;
         break;

      case HUNK_TYPE_JUMP_ADD:
         hunk.type = HUNK_TYPE_JUMP_REMOVE;
         break;
      case HUNK_TYPE_JUMP_REMOVE:
         hunk.type = HUNK_TYPE_JUMP_ADD;
         break;

      case HUNK_TYPE_SSYS_BACKGROUND:
         hunk.type = HUNK_TYPE_SSYS_BACKGROUND_REVERT;
         break;
      case HUNK_TYPE_SSYS_FEATURES:
         hunk.type = HUNK_TYPE_SSYS_FEATURES_REVERT;
         break;

      case HUNK_TYPE_TECH_ADD:
         hunk.type = HUNK_TYPE_TECH_REMOVE;
         break;
      case HUNK_TYPE_TECH_REMOVE:
         hunk.type = HUNK_TYPE_TECH_ADD;
         break;

      case HUNK_TYPE_SPOB_FACTION:
         hunk.type = HUNK_TYPE_SPOB_FACTION_REMOVE;
         break;

      case HUNK_TYPE_SPOB_POPULATION:
         hunk.type = HUNK_TYPE_SPOB_POPULATION_REMOVE;
         break;

      case HUNK_TYPE_SPOB_DISPLAYNAME:
         hunk.type = HUNK_TYPE_SPOB_DISPLAYNAME_REVERT;
         break;

      case HUNK_TYPE_SPOB_DESCRIPTION:
         hunk.type = HUNK_TYPE_SPOB_DESCRIPTION_REVERT;
         break;

      case HUNK_TYPE_SPOB_BAR:
         hunk.type = HUNK_TYPE_SPOB_BAR_REVERT;
         break;

      case HUNK_TYPE_SPOB_SERVICE_ADD:
         hunk.type = HUNK_TYPE_SPOB_SERVICE_REMOVE;
         break;
      case HUNK_TYPE_SPOB_SERVICE_REMOVE:
         hunk.type = HUNK_TYPE_SPOB_SERVICE_ADD;
         break;

      case HUNK_TYPE_SPOB_NOMISNSPAWN_ADD:
         hunk.type = HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE;
         break;
      case HUNK_TYPE_SPOB_NOMISNSPAWN_REMOVE:
         hunk.type = HUNK_TYPE_SPOB_NOMISNSPAWN_ADD;
         break;

      case HUNK_TYPE_SPOB_TECH_ADD:
         hunk.type = HUNK_TYPE_SPOB_TECH_REMOVE;
         break;
      case HUNK_TYPE_SPOB_TECH_REMOVE:
         hunk.type = HUNK_TYPE_SPOB_TECH_ADD;
         break;

      case HUNK_TYPE_SPOB_TAG_ADD:
         hunk.type = HUNK_TYPE_SPOB_TAG_REMOVE;
         break;
      case HUNK_TYPE_SPOB_TAG_REMOVE:
         hunk.type = HUNK_TYPE_SPOB_TAG_ADD;
         break;

      case HUNK_TYPE_SPOB_SPACE:
         hunk.type = HUNK_TYPE_SPOB_SPACE_REVERT;
         break;

      case HUNK_TYPE_SPOB_EXTERIOR:
         hunk.type = HUNK_TYPE_SPOB_EXTERIOR_REVERT;
         break;

      case HUNK_TYPE_SPOB_LUA:
         hunk.type = HUNK_TYPE_SPOB_LUA_REVERT;
         break;

      case HUNK_TYPE_FACTION_VISIBLE:
         hunk.type = HUNK_TYPE_FACTION_INVISIBLE;
         break;
      case HUNK_TYPE_FACTION_INVISIBLE:
         hunk.type = HUNK_TYPE_FACTION_VISIBLE;
         break;

      case HUNK_TYPE_FACTION_ALLY:
         hunk.type = HUNK_TYPE_FACTION_REALIGN;
         break;
      case HUNK_TYPE_FACTION_ENEMY:
         hunk.type = HUNK_TYPE_FACTION_REALIGN;
         break;
      case HUNK_TYPE_FACTION_NEUTRAL:
         hunk.type = HUNK_TYPE_FACTION_REALIGN;
         break;

      default:
         WARN( _( "Unknown Hunk type '%d'." ), hunk.type );
         continue;
      }

      if ( diff_patchHunk( &hunk ) )
         WARN( _( "Failed to remove hunk type '%d'." ), hunk.type );
   }

   diff_cleanup( diff );
   array_erase( &diff_stack, diff, &diff[1] );
   return 0;
}

/**
 * @brief Cleans up a diff.
 *
 *    @param diff Diff to clean up.
 */
static void diff_cleanup( UniDiff_t *diff )
{
   free( diff->name );
   for ( int i = 0; i < array_size( diff->applied ); i++ )
      diff_cleanupHunk( &diff->applied[i] );
   array_free( diff->applied );
   for ( int i = 0; i < array_size( diff->failed ); i++ )
      diff_cleanupHunk( &diff->failed[i] );
   array_free( diff->failed );
   memset( diff, 0, sizeof( UniDiff_t ) );
}

/**
 * @brief Cleans up a hunk.
 *
 *    @param hunk Hunk to clean up.
 */
static void diff_cleanupHunk( UniHunk_t *hunk )
{
   free( hunk->target.u.name );
   hunk->target.u.name = NULL;

   switch ( hunk->type ) { /* TODO: Does it really matter? */
   case HUNK_TYPE_SPOB_ADD:
   case HUNK_TYPE_SPOB_REMOVE:
   case HUNK_TYPE_VSPOB_ADD:
   case HUNK_TYPE_VSPOB_REMOVE:
   case HUNK_TYPE_JUMP_ADD:
   case HUNK_TYPE_JUMP_REMOVE:
   case HUNK_TYPE_SSYS_BACKGROUND:
   case HUNK_TYPE_SSYS_FEATURES:
   case HUNK_TYPE_TECH_ADD:
   case HUNK_TYPE_TECH_REMOVE:
   case HUNK_TYPE_SPOB_FACTION:
   case HUNK_TYPE_SPOB_FACTION_REMOVE:
   case HUNK_TYPE_SPOB_DISPLAYNAME:
   case HUNK_TYPE_SPOB_DISPLAYNAME_REVERT:
   case HUNK_TYPE_SPOB_DESCRIPTION:
   case HUNK_TYPE_SPOB_DESCRIPTION_REVERT:
   case HUNK_TYPE_SPOB_TECH_ADD:
   case HUNK_TYPE_SPOB_TECH_REMOVE:
   case HUNK_TYPE_SPOB_TAG_ADD:
   case HUNK_TYPE_SPOB_TAG_REMOVE:
   case HUNK_TYPE_SPOB_BAR:
   case HUNK_TYPE_SPOB_BAR_REVERT:
   case HUNK_TYPE_SPOB_SPACE:
   case HUNK_TYPE_SPOB_SPACE_REVERT:
   case HUNK_TYPE_SPOB_EXTERIOR:
   case HUNK_TYPE_SPOB_EXTERIOR_REVERT:
   case HUNK_TYPE_SPOB_LUA:
   case HUNK_TYPE_SPOB_LUA_REVERT:
   case HUNK_TYPE_FACTION_VISIBLE:
   case HUNK_TYPE_FACTION_INVISIBLE:
   case HUNK_TYPE_FACTION_ALLY:
   case HUNK_TYPE_FACTION_ENEMY:
   case HUNK_TYPE_FACTION_NEUTRAL:
   case HUNK_TYPE_FACTION_REALIGN:
      free( hunk->u.name );
      hunk->u.name = NULL;
      break;

   default:
      break;
   }
   memset( hunk, 0, sizeof( UniHunk_t ) );
}

/**
 * @brief Saves the active diffs.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int diff_save( xmlTextWriterPtr writer )
{
   xmlw_startElem( writer, "diffs" );
   if ( diff_stack != NULL ) {
      for ( int i = 0; i < array_size( diff_stack ); i++ ) {
         UniDiff_t *diff = &diff_stack[i];

         xmlw_elem( writer, "diff", "%s", diff->name );
      }
   }
   xmlw_endElem( writer ); /* "diffs" */
   return 0;
}

/**
 * @brief Loads the diffs.
 *
 *    @param parent Parent node containing diffs.
 *    @return 0 on success.
 */
int diff_load( xmlNodePtr parent )
{
   xmlNodePtr node;
   int        defer = diff_universe_defer;

   /* Don't update universe here. */
   diff_universe_defer   = 1;
   diff_universe_changed = 0;
   diff_nav_spob         = NULL;
   diff_nav_hyperspace   = NULL;
   diff_clear();
   diff_universe_defer = defer;

   node = parent->xmlChildrenNode;
   do {
      if ( xml_isNode( node, "diffs" ) ) {
         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            if ( xml_isNode( cur, "diff" ) ) {
               const char *diffName = xml_get( cur );
               if ( diffName == NULL ) {
                  WARN( _( "Expected node \"diff\" to contain the name of a "
                           "unidiff. Was empty." ) );
                  continue;
               }
               diff_applyInternal( diffName, 0 );
            }
         } while ( xml_nextNode( cur ) );
      }
   } while ( xml_nextNode( node ) );

   /* Update as necessary. */
   diff_checkUpdateUniverse();

   return 0;
}

/**
 * @brief Checks and updates the universe if necessary.
 */
static int diff_checkUpdateUniverse( void )
{
   Pilot *const *pilots;

   if ( !diff_universe_changed || diff_universe_defer )
      return 0;

   /* Update presences, then safelanes. */
   space_reconstructPresences();
   safelanes_recalculate();

   /* Re-compute the economy. */
   economy_execQueued();
   economy_initialiseCommodityPrices();

   /* Have to update planet graphics if necessary. */
   if ( cur_system != NULL ) {
      space_gfxUnload( cur_system );
      space_gfxLoad( cur_system );
   }

   /* Have to pilot targetting just in case. */
   pilots = pilot_getAll();
   for ( int i = 0; i < array_size( pilots ); i++ ) {
      Pilot *p          = pilots[i];
      p->nav_spob       = -1;
      p->nav_hyperspace = -1;

      /* Hack in case the pilot was actively jumping, this won't run the hook,
       * but I guess it's too much effort to properly fix for a situation that
       * will likely never happen. */
      if ( !pilot_isWithPlayer( p ) && pilot_isFlag( p, PILOT_HYPERSPACE ) )
         pilot_delete( p );
      else
         pilot_rmFlag(
            p, PILOT_HYPERSPACE ); /* Corner case player, just have it not crash
                                      and randomly stop the jump. */

      /* Have to reset in the case of starting. */
      if ( pilot_isFlag( p, PILOT_HYP_BEGIN ) ||
           pilot_isFlag( p, PILOT_HYP_BRAKE ) ||
           pilot_isFlag( p, PILOT_HYP_PREP ) )
         pilot_hyperspaceAbort( p );
   }

   /* Try to restore the targets if possible. */
   if ( diff_nav_spob != NULL ) {
      int found = 0;
      for ( int i = 0; i < array_size( cur_system->spobs ); i++ ) {
         if ( strcmp( cur_system->spobs[i]->name, diff_nav_spob ) == 0 ) {
            found              = 1;
            player.p->nav_spob = i;
            player_targetSpobSet( i );
            break;
         }
      }
      if ( !found )
         player_targetSpobSet( -1 );
   } else
      player_targetSpobSet( -1 );
   if ( diff_nav_hyperspace != NULL ) {
      int found = 0;
      for ( int i = 0; i < array_size( cur_system->jumps ); i++ ) {
         if ( strcmp( cur_system->jumps[i].target->name,
                      diff_nav_hyperspace ) == 0 ) {
            found                    = 1;
            player.p->nav_hyperspace = i;
            player_targetHyperspaceSet( i, 0 );
            break;
         }
      }
      if ( !found )
         player_targetHyperspaceSet( -1, 0 );
   } else
      player_targetHyperspaceSet( -1, 0 );

   diff_universe_changed = 0;
   return 1;
}

/**
 * @brief Sets whether or not to defer universe change stuff.
 *
 *    @param enable Whether or not to enable deferring.
 */
void unidiff_universeDefer( int enable )
{
   int defer           = diff_universe_defer;
   diff_universe_defer = enable;
   if ( defer && !enable )
      diff_checkUpdateUniverse();
}
