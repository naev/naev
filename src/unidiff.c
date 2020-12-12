/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file unidiff.c
 *
 * @brief Handles the application and removal of 'diffs' to the universe.
 *
 * Diffs allow changing planets, fleets, factions, etc... in the universe.
 *  These are meant to be applied after the player triggers them, mostly
 *  through missions.
 */


#include "unidiff.h"

#include "naev.h"

#include <stdlib.h>
#include "nstring.h"

#include "log.h"
#include "array.h"
#include "nxml.h"
#include "space.h"
#include "ndata.h"
#include "economy.h"
#include "fleet.h"
#include "map_overlay.h"


#define CHUNK_SIZE      32 /**< Size of chunk to allocate. */


/**
 * @brief Universe diff filepath list.
 */
typedef struct UniDiffData_ {
   char *name; /**< Name of the diff (read from XML). */
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
   HUNK_TARGET_ASSET,
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
   } u; /**< Union of possible target types. */
} UniHunkTarget_t;


/**
 * @enum UniHunkType_t
 *
 * @brief Represents the different type of hunk actions.
 */
typedef enum UniHunkType_ {
   HUNK_TYPE_NONE,
   /* Target should be system. */
   HUNK_TYPE_ASSET_ADD,
   HUNK_TYPE_ASSET_REMOVE,
   HUNK_TYPE_ASSET_BLACKMARKET,
   HUNK_TYPE_ASSET_LEGALMARKET,
   HUNK_TYPE_JUMP_ADD,
   HUNK_TYPE_JUMP_REMOVE,
   /* Target should be tech. */
   HUNK_TYPE_TECH_ADD,
   HUNK_TYPE_TECH_REMOVE,
   /* Target should be asset. */
   HUNK_TYPE_ASSET_FACTION,
   HUNK_TYPE_ASSET_FACTION_REMOVE, /* For internal usage. */
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
   xmlNodePtr node; /**< Parent node. */
   union {
      char *name;
   } u; /**< Actual data to patch. */
   union {
      char *name;
      int data;
   } o; /** Old data to possibly replace. */
} UniHunk_t;


/**
 * @struct UniDiff_t
 *
 * @brief Represents each Universe Diff.
 */
typedef struct UniDiff_ {
   char *name; /**< Name of the diff. */

   UniHunk_t *applied; /**< Applied hunks. */
   int napplied; /**< Number of applied hunks. */
   int mapplied; /**< Memory of applied hunks. */

   UniHunk_t *failed; /**< Failed hunks. */
   int nfailed; /**< Number of failed hunks. */
   int mfailed; /**< Memory of failed hunks. */
} UniDiff_t;


/*
 * Diff stack.
 */
static UniDiff_t *diff_stack = NULL; /**< Currently applied universe diffs. */


/*
 * Prototypes.
 */
NONNULL( 1 ) static UniDiff_t *diff_get( const char *name );
static UniDiff_t *diff_newDiff (void);
static int diff_removeDiff( UniDiff_t *diff );
static int diff_patchSystem( UniDiff_t *diff, xmlNodePtr node );
static int diff_patchTech( UniDiff_t *diff, xmlNodePtr node );
static int diff_patch( xmlNodePtr parent );
static int diff_patchHunk( UniHunk_t *hunk );
static void diff_hunkFailed( UniDiff_t *diff, UniHunk_t *hunk );
static void diff_hunkSuccess( UniDiff_t *diff, UniHunk_t *hunk );
static void diff_cleanup( UniDiff_t *diff );
static void diff_cleanupHunk( UniHunk_t *hunk );
/* Externed. */
int diff_save( xmlTextWriterPtr writer ); /**< Used in save.c */
int diff_load( xmlNodePtr parent ); /**< Used in save.c */


/**
 * @brief Loads available universe diffs.
 *
 *    @return 0 on success.
 */
int diff_loadAvailable (void)
{
   int i;
   char **diff_files;
   size_t bufsize;
   char *filebuf;
   xmlDocPtr doc;
   xmlNodePtr node;
   UniDiffData_t *diff;

   diff_files     = ndata_listRecursive( UNIDIFF_DATA_PATH );
   diff_available = array_create_size( UniDiffData_t, array_size( diff_files ) );
   for ( i = 0; i < array_size( diff_files ); i++ ) {
      /* Load string. */
      filebuf = ndata_read( diff_files[i], &bufsize );
      if (filebuf == NULL) {
         WARN(_("Unable to read data from '%s'"), diff_files[i]);
         return -1;
      }

      /* Parse the header. */
      doc = xmlParseMemory( filebuf, bufsize );
      if (doc == NULL) {
         WARN(_("Unable to parse document XML for UniDiff '%s'"), diff_files[i]);
         return -1;
      }

      node = doc->xmlChildrenNode;
      if (!xml_isNode(node,"unidiff")) {
         ERR( _("Malformed XML header for '%s' UniDiff: missing root element '%s'"), diff_files[i], "unidiff" );
         return -1;
      }

      diff = &array_grow(&diff_available);
      diff->filename = diff_files[i];
      xmlr_attr_strd(node, "name", diff->name);
      xmlFreeDoc(doc);
      free(filebuf);
   }
   array_free( diff_files );
   array_shrink(&diff_available);

   DEBUG( ngettext("Loaded %d UniDiff", "Loaded %d UniDiffs", array_size(diff_available) ), array_size(diff_available) );

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
   if (diff_get(name) != NULL)
      return 1;
   return 0;
}


/**
 * @brief Gets a diff by name.
 *
 *    @param name Name of the diff to get.
 *    @return The diff if found or NULL if not found.
 */
static UniDiff_t* diff_get( const char *name )
{
   int i;
   if (diff_stack == NULL)
      return NULL;
   for (i=0; i<array_size(diff_stack); i++)
      if (strcmp(diff_stack[i].name,name)==0)
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
   xmlNodePtr node;
   xmlDocPtr doc;
   size_t bufsize;
   char *buf;
   char *filename;
   int i;

   /* Check if already applied. */
   if (diff_isApplied(name))
      return 0;

   filename = NULL;
   for (i=0; i<array_size(diff_available); i++) {
      if (strcmp(diff_available[i].name,name)==0) {
         filename = diff_available[i].filename;
         break;
      }
   }
   if (filename == NULL) {
      WARN(_("UniDiff '%s' not found in %s!"), name, UNIDIFF_DATA_PATH);
      return -1;
   }

   buf = ndata_read( filename, &bufsize );
   doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (strcmp((char*)node->name,"unidiff")) {
      ERR(_("Malformed unidiff file: missing root element 'unidiff'"));
      return 0;
   }

   /* Apply it. */
   diff_patch( node );

   xmlFreeDoc(doc);
   free(buf);

   /* Re-compute the economy. */
   economy_execQueued();
   economy_initialiseCommodityPrices();

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
   UniHunk_t base, hunk;
   xmlNodePtr cur;
   char *buf;

   /* Set the target. */
   memset(&base, 0, sizeof(UniHunk_t));
   base.target.type = HUNK_TARGET_SYSTEM;
   xmlr_attr_strd(node,"name",base.target.u.name);
   if (base.target.u.name==NULL) {
      WARN(_("Unidiff '%s' has a system node without a 'name' tag, not applying."), diff->name);
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes(cur);
      if (xml_isNode(cur,"asset")) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Get the asset to modify. */
         xmlr_attr_strd(cur,"name",hunk.u.name);

         /* Get the type. */
         if (strcmp(buf,"add")==0)
            hunk.type = HUNK_TYPE_ASSET_ADD;
         else if (strcmp(buf,"remove")==0)
            hunk.type = HUNK_TYPE_ASSET_REMOVE;
         else if (strcmp(buf,"blackmarket")==0)
            hunk.type = HUNK_TYPE_ASSET_BLACKMARKET;
         else if (strcmp(buf,"legalmarket")==0)
            hunk.type = HUNK_TYPE_ASSET_LEGALMARKET;
         else
            WARN(_("Unidiff '%s': Unknown hunk type '%s' for asset '%s'."), diff->name, buf, hunk.u.name);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      else if (xml_isNode(cur,"jump")) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Get the jump point to modify. */
         xmlr_attr_strd(cur,"target",hunk.u.name);

         /* Get the type. */
         if (strcmp(buf,"add")==0)
            hunk.type = HUNK_TYPE_JUMP_ADD;
         else if (strcmp(buf,"remove")==0)
            hunk.type = HUNK_TYPE_JUMP_REMOVE;
         else
            WARN(_("Unidiff '%s': Unknown hunk type '%s' for jump '%s'."), diff->name, buf, hunk.u.name);

         hunk.node = cur;

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN(_("Unidiff '%s' has unknown node '%s'."), diff->name, node->name);
   } while (xml_nextNode(cur));

   /* Clean up some stuff. */
   free(base.target.u.name);
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
   UniHunk_t base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset(&base, 0, sizeof(UniHunk_t));
   base.target.type = HUNK_TARGET_TECH;
   xmlr_attr_strd(node,"name",base.target.u.name);
   if (base.target.u.name==NULL) {
      WARN(_("Unidiff '%s' has an target node without a 'name' tag"), diff->name);
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes(cur);
      if (xml_isNode(cur,"add")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_TECH_ADD;

         /* Get the data. */
         hunk.u.name = xml_getStrd(cur);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      else if (xml_isNode(cur,"remove")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_TECH_REMOVE;

         /* Get the data. */
         hunk.u.name = xml_getStrd(cur);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN(_("Unidiff '%s' has unknown node '%s'."), diff->name, node->name);
   } while (xml_nextNode(cur));

   /* Clean up some stuff. */
   free(base.target.u.name);
   base.target.u.name = NULL;

   return 0;
}


/**
 * @brief Patches a asset.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the asset.
 *    @return 0 on success.
 */
static int diff_patchAsset( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t base, hunk;
   xmlNodePtr cur;

   /* Set the target. */
   memset(&base, 0, sizeof(UniHunk_t));
   base.target.type = HUNK_TARGET_ASSET;
   xmlr_attr_strd(node,"name",base.target.u.name);
   if (base.target.u.name==NULL) {
      WARN(_("Unidiff '%s' has an target node without a 'name' tag"), diff->name);
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes(cur);
      if (xml_isNode(cur,"faction")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_ASSET_FACTION;

         /* Get the data. */
         hunk.u.name = xml_getStrd(cur);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN(_("Unidiff '%s' has unknown node '%s'."), diff->name, node->name);
   } while (xml_nextNode(cur));

   /* Clean up some stuff. */
   free(base.target.u.name);
   base.target.u.name = NULL;

   return 0;
}


/**
 * @brief Patches a faction.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the asset.
 *    @return 0 on success.
 */
static int diff_patchFaction( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t base, hunk;
   xmlNodePtr cur;
   char *buf;

   /* Set the target. */
   memset(&base, 0, sizeof(UniHunk_t));
   base.target.type = HUNK_TARGET_FACTION;
   xmlr_attr_strd(node,"name",base.target.u.name);
   if (base.target.u.name==NULL) {
      WARN(_("Unidiff '%s' has an target node without a 'name' tag"), diff->name);
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes(cur);
      if (xml_isNode(cur,"visible")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Faction type is constant. */
         hunk.type = HUNK_TYPE_FACTION_VISIBLE;

         /* There is no name. */
         hunk.u.name = NULL;

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      else if (xml_isNode(cur,"invisible")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Faction type is constant. */
         hunk.type = HUNK_TYPE_FACTION_INVISIBLE;

         /* There is no name. */
         hunk.u.name = NULL;

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      else if (xml_isNode(cur,"faction")) {
         buf = xml_get( cur );
         if ( buf == NULL ) {
            WARN( _( "Unidiff '%s': Null hunk type." ), diff->name );
            continue;
         }

         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Get the faction to set the association of. */
         xmlr_attr_strd(cur,"name",hunk.u.name);

         /* Get the type. */
         if (strcmp(buf,"ally")==0)
            hunk.type = HUNK_TYPE_FACTION_ALLY;
         else if (strcmp(buf,"enemy")==0)
            hunk.type = HUNK_TYPE_FACTION_ENEMY;
         else if (strcmp(buf,"neutral")==0)
            hunk.type = HUNK_TYPE_FACTION_NEUTRAL;
         else
            WARN(_("Unidiff '%s': Unknown hunk type '%s' for faction '%s'."), diff->name, buf, hunk.u.name);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
         continue;
      }
      WARN(_("Unidiff '%s' has unknown node '%s'."), diff->name, node->name);
   } while (xml_nextNode(cur));

   /* Clean up some stuff. */
   free(base.target.u.name);
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
   int i, univ_update;
   UniDiff_t *diff;
   UniHunk_t *fail;
   xmlNodePtr node;
   char *target;

   /* Prepare it. */
   diff = diff_newDiff();
   memset(diff, 0, sizeof(UniDiff_t));
   xmlr_attr_strd(parent,"name",diff->name);

   /* Whether or not we need to update the universe. */
   univ_update = 0;

   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,"system")) {
         univ_update = 1;
         diff_patchSystem( diff, node );
      }
      else if (xml_isNode(node, "tech"))
         diff_patchTech( diff, node );
      else if (xml_isNode(node, "asset")) {
         univ_update = 1;
         diff_patchAsset( diff, node );
      }
      else if (xml_isNode(node, "faction")) {
         univ_update = 1;
         diff_patchFaction( diff, node );
      }
      else
         WARN(_("Unidiff '%s' has unknown node '%s'."), diff->name, node->name);
   } while (xml_nextNode(node));

   if (diff->nfailed > 0) {
      WARN(
         ngettext( "Unidiff '%s' failed to apply %d hunk.", "Unidiff '%s' failed to apply %d hunks.", diff->nfailed ),
         diff->name, diff->nfailed );
      for (i=0; i<diff->nfailed; i++) {
         fail   = &diff->failed[i];
         target = fail->target.u.name;
         switch (fail->type) {
            case HUNK_TYPE_ASSET_ADD:
               WARN(_("   [%s] asset add: '%s'"), target, fail->u.name);
               break;
            case HUNK_TYPE_ASSET_REMOVE:
               WARN(_("   [%s] asset remove: '%s'"), target, fail->u.name);
               break;
            case HUNK_TYPE_ASSET_BLACKMARKET:
               WARN(_("   [%s] asset blackmarket: '%s'"), target, fail->u.name);
               break;
            case HUNK_TYPE_ASSET_LEGALMARKET:
               WARN(_("   [%s] asset legalmarket: '%s'"), target, fail->u.name);
               break;
            case HUNK_TYPE_JUMP_ADD:
               WARN(_("   [%s] jump add: '%s'"), target, fail->u.name);
               break;
            case HUNK_TYPE_JUMP_REMOVE:
               WARN(_("   [%s] jump remove: '%s'"), target, fail->u.name);
               break;
            case HUNK_TYPE_TECH_ADD:
               WARN(_("   [%s] tech add: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_TECH_REMOVE:
               WARN(_("   [%s] tech remove: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_ASSET_FACTION:
               WARN(_("   [%s] asset faction: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_ASSET_FACTION_REMOVE:
               WARN(_("   [%s] asset faction removal: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_FACTION_VISIBLE:
               WARN(_("   [%s] faction visible: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_FACTION_INVISIBLE:
               WARN(_("   [%s] faction invisible: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_FACTION_ALLY:
               WARN(_("   [%s] faction set ally: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_FACTION_ENEMY:
               WARN(_("   [%s] faction set enemy: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_FACTION_NEUTRAL:
               WARN(_("   [%s] faction set neutral: '%s'"), target,
                     fail->u.name );
               break;
            case HUNK_TYPE_FACTION_REALIGN:
               WARN(_("   [%s] faction alignment reset: '%s'"), target,
                     fail->u.name );
               break;

            default:
               WARN(_("   unknown hunk '%d'"), fail->type);
               break;
         }
      }
   }

   /* Prune presences if necessary. */
   if (univ_update)
      space_reconstructPresences();

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
   Planet *p;
   int a, b;

   switch (hunk->type) {

      /* Adding an asset. */
      case HUNK_TYPE_ASSET_ADD:
         planet_updateLand( planet_get(hunk->u.name) );
         return system_addPlanet( system_get(hunk->target.u.name), hunk->u.name );
      /* Removing an asset. */
      case HUNK_TYPE_ASSET_REMOVE:
         return system_rmPlanet( system_get(hunk->target.u.name), hunk->u.name );
      /* Making an asset a black market. */
      case HUNK_TYPE_ASSET_BLACKMARKET:
         planet_addService( planet_get(hunk->u.name), PLANET_SERVICE_BLACKMARKET );
         return 0;
      /* Making an asset a legal market. */
      case HUNK_TYPE_ASSET_LEGALMARKET:
         planet_rmService( planet_get(hunk->u.name), PLANET_SERVICE_BLACKMARKET );
         return 0;

      /* Adding a Jump. */
      case HUNK_TYPE_JUMP_ADD:
         return system_addJumpDiff( system_get(hunk->target.u.name), hunk->node );
      /* Removing a jump. */
      case HUNK_TYPE_JUMP_REMOVE:
         return system_rmJump( system_get(hunk->target.u.name), hunk->u.name );

      /* Adding a tech. */
      case HUNK_TYPE_TECH_ADD:
         return tech_addItem( hunk->target.u.name, hunk->u.name );
      /* Removing a tech. */
      case HUNK_TYPE_TECH_REMOVE:
         return tech_rmItem( hunk->target.u.name, hunk->u.name );

      /* Changing asset faction. */
      case HUNK_TYPE_ASSET_FACTION:
         p = planet_get( hunk->target.u.name );
         if (p==NULL)
            return -1;
         hunk->o.name = faction_name( p->faction );
         return planet_setFaction( p, faction_get(hunk->u.name) );
      case HUNK_TYPE_ASSET_FACTION_REMOVE:
         return planet_setFaction( planet_get(hunk->target.u.name), faction_get(hunk->o.name) );

      /* Making a faction visible. */
      case HUNK_TYPE_FACTION_VISIBLE:
         return faction_setInvisible( faction_get(hunk->target.u.name), 0 );
      /* Making a faction invisible. */
      case HUNK_TYPE_FACTION_INVISIBLE:
         return faction_setInvisible( faction_get(hunk->target.u.name), 1 );
      /* Making two factions allies. */
      case HUNK_TYPE_FACTION_ALLY:
         a = faction_get( hunk->target.u.name );
         b = faction_get( hunk->u.name );
         if (areAllies(a, b))
            hunk->o.data = 'A';
         else if (areEnemies(a, b))
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
         if (areAllies(a, b))
            hunk->o.data = 'A';
         else if (areEnemies(a, b))
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
         if (areAllies(a, b))
            hunk->o.data = 'A';
         else if (areEnemies(a, b))
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
         if (hunk->o.data == 'A') {
            faction_rmEnemy(a, b);
            faction_rmEnemy(b, a);
            faction_addAlly(a, b);
            faction_addAlly(b, a);
         }
         else if (hunk->o.data == 'E') {
            faction_rmAlly(a, b);
            faction_rmAlly(b, a);
            faction_addEnemy(a, b);
            faction_addAlly(b, a);
         }
         else {
            faction_rmAlly( a, b );
            faction_rmAlly( b, a );
            faction_rmEnemy( a, b );
            faction_rmEnemy( b, a );
         }
         return 0;

      default:
         WARN(_("Unknown hunk type '%d'."), hunk->type);
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
static void diff_hunkFailed( UniDiff_t *diff, UniHunk_t *hunk )
{
   if (diff == NULL)
      return;

   diff->nfailed++;
   if (diff->nfailed > diff->mfailed) {
      diff->mfailed += CHUNK_SIZE;
      diff->failed = realloc(diff->failed, sizeof(UniHunk_t) * diff->mfailed);
   }
   diff->failed[diff->nfailed-1] = *hunk;
}


/**
 * @brief Adds a hunk to the applied list.
 *
 *    @param diff Diff to add hunk to.
 *    @param hunk Hunk that applied correctly.
 */
static void diff_hunkSuccess( UniDiff_t *diff, UniHunk_t *hunk )
{
   if (diff == NULL)
      return;

   diff->napplied++;
   if (diff->napplied > diff->mapplied) {
      diff->mapplied += CHUNK_SIZE;
      diff->applied = realloc(diff->applied, sizeof(UniHunk_t) * diff->mapplied);
   }
   diff->applied[diff->napplied-1] = *hunk;
}


/**
 * @brief Removes a diff from the universe.
 *
 *    @param name Diff to remove.
 */
void diff_remove( const char *name )
{
   UniDiff_t *diff;

   /* Check if already applied. */
   diff = diff_get(name);
   if (diff == NULL)
      return;

   diff_removeDiff(diff);

   economy_execQueued();
}


/**
 * @brief Removes all active diffs. (Call before economy_destroy().)
 */
void diff_clear (void)
{
   if (diff_stack==NULL)
      return;

   while (array_size(diff_stack) > 0)
      diff_removeDiff(&diff_stack[array_size(diff_stack)-1]);

   economy_execQueued();
}


/**
 * @brief Clean up after diff_loadAvailable().
 */
void diff_free (void)
{
   diff_clear();
   for (int i = 0; i < array_size(diff_available); i++) {
      free(diff_available[i].name);
      free(diff_available[i].filename);
   }
   array_free(diff_available);
   diff_available = NULL;
}


/**
 * @brief Creates a new UniDiff_t for usage.
 *
 *    @return A newly created UniDiff_t.
 */
static UniDiff_t *diff_newDiff (void)
{
   /* Check if needs initialization. */
   if (diff_stack == NULL)
      diff_stack = array_create(UniDiff_t);
   return &array_grow(&diff_stack);
}


/**
 * @brief Removes a diff.
 *
 *    @param diff Diff to remove.
 *    @return 0 on success.
 */
static int diff_removeDiff( UniDiff_t *diff )
{
   int i;
   UniHunk_t hunk;

   for (i=0; i<diff->napplied; i++) {
      hunk = diff->applied[i];
      /* Invert the type for reverting. */
      switch (hunk.type) {
         case HUNK_TYPE_ASSET_ADD:
            hunk.type = HUNK_TYPE_ASSET_REMOVE;
            break;
         case HUNK_TYPE_ASSET_REMOVE:
            hunk.type = HUNK_TYPE_ASSET_ADD;
            break;

         case HUNK_TYPE_ASSET_BLACKMARKET:
            hunk.type = HUNK_TYPE_ASSET_LEGALMARKET;
            break;
         case HUNK_TYPE_ASSET_LEGALMARKET:
            hunk.type = HUNK_TYPE_ASSET_BLACKMARKET;
            break;

         case HUNK_TYPE_JUMP_ADD:
            hunk.type = HUNK_TYPE_JUMP_REMOVE;
            break;
         case HUNK_TYPE_JUMP_REMOVE:
            hunk.type = HUNK_TYPE_JUMP_ADD;
            break;

         case HUNK_TYPE_TECH_ADD:
            hunk.type = HUNK_TYPE_TECH_REMOVE;
            break;
         case HUNK_TYPE_TECH_REMOVE:
            hunk.type = HUNK_TYPE_TECH_ADD;
            break;

         case HUNK_TYPE_ASSET_FACTION:
            hunk.type = HUNK_TYPE_ASSET_FACTION_REMOVE;
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
            WARN(_("Unknown Hunk type '%d'."), hunk.type);
            continue;
      }

      if (diff_patchHunk(&hunk))
         WARN(_("Failed to remove hunk type '%d'."), hunk.type);
   }

   diff_cleanup(diff);
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
   int i;

   free(diff->name);
   for (i=0; i<diff->napplied; i++)
      diff_cleanupHunk(&diff->applied[i]);
   if (diff->applied != NULL)
      free(diff->applied);
   for (i=0; i<diff->nfailed; i++)
      diff_cleanupHunk(&diff->failed[i]);
   if (diff->failed != NULL)
      free(diff->failed);
   memset(diff, 0, sizeof(UniDiff_t));
}


/**
 * @brief Cleans up a hunk.
 *
 *    @param hunk Hunk to clean up.
 */
static void diff_cleanupHunk( UniHunk_t *hunk )
{
   if (hunk->target.u.name != NULL)
      free(hunk->target.u.name);

   switch (hunk->type) {
      case HUNK_TYPE_ASSET_ADD:
      case HUNK_TYPE_ASSET_REMOVE:
      case HUNK_TYPE_ASSET_BLACKMARKET:
      case HUNK_TYPE_ASSET_LEGALMARKET:
      case HUNK_TYPE_JUMP_ADD:
      case HUNK_TYPE_JUMP_REMOVE:
      case HUNK_TYPE_TECH_ADD:
      case HUNK_TYPE_TECH_REMOVE:
      case HUNK_TYPE_ASSET_FACTION:
      case HUNK_TYPE_ASSET_FACTION_REMOVE:
      case HUNK_TYPE_FACTION_VISIBLE:
      case HUNK_TYPE_FACTION_INVISIBLE:
      case HUNK_TYPE_FACTION_ALLY:
      case HUNK_TYPE_FACTION_ENEMY:
      case HUNK_TYPE_FACTION_NEUTRAL:
      case HUNK_TYPE_FACTION_REALIGN:
         if (hunk->u.name != NULL)
            free(hunk->u.name);
         hunk->u.name = NULL;
         break;

      default:
         break;
   }
   memset( hunk, 0, sizeof(UniHunk_t) );
}


/**
 * @brief Saves the active diffs.
 *
 *    @param writer XML Writer to use.
 *    @return 0 on success.
 */
int diff_save( xmlTextWriterPtr writer )
{
   int i;
   UniDiff_t *diff;

   xmlw_startElem(writer,"diffs");
   if (diff_stack != NULL) {
      for (i=0; i<array_size(diff_stack); i++) {
         diff = &diff_stack[i];

         xmlw_elem(writer, "diff", "%s", diff->name);
      }
   }
   xmlw_endElem(writer); /* "diffs" */

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
   xmlNodePtr node, cur;
   char *     diffName;

   diff_clear();

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"diffs")) {
         cur = node->xmlChildrenNode;
         do {
            if ( xml_isNode( cur, "diff" ) ) {
               diffName = xml_get( cur );
               if ( diffName == NULL ) {
                  WARN( _( "Expected node \"diff\" to contain the name of a unidiff. Was empty." ) );
                  continue;
               }
               diff_apply( diffName );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;

}


