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
#include <string.h>

#include "log.h"
#include "nxml.h"
#include "space.h"
#include "ndata.h"
#include "fleet.h"


#define CHUNK_SIZE      32 /**< Size of chunk to allocate. */


#define DIFF_DATA       "dat/unidiff.xml" /**< Unidiff XML file. */


/**
 * @enum UniHunkTargetType_t
 *
 * @brief Represents the possible hunk targets.
 */
typedef enum UniHunkTargetType_ {
   HUNK_TARGET_NONE,
   HUNK_TARGET_SYSTEM,
   HUNK_TARGET_OUTFIT,
   HUNK_TARGET_SHIP
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
   HUNK_TYPE_PLANET_ADD,
   HUNK_TYPE_PLANET_REMOVE,
   HUNK_TYPE_FLEET_ADD,
   HUNK_TYPE_FLEET_REMOVE,
   HUNK_TYPE_FLEETGROUP_ADD,
   HUNK_TYPE_FLEETGROUP_REMOVE,
   HUNK_TYPE_SHIP_TECH,
   HUNK_TYPE_OUTFIT_TECH
} UniHunkType_t;


/**
 * @struct UniHunk_t
 *
 * @brief Represents a single hunk in the diff.
 */
typedef struct UniHunk_ {
   UniHunkTarget_t target; /**< Hunk's target. */

   UniHunkType_t type; /**< Type of hunk it is. */
   union {
      char *name;
      Fleet *fleet;
      struct {
         int old; /**< Old value. */
         int new; /**< New value. */
      } i; /**< Contains old and new int values. */
   } u; /**< Actual data to patch. */
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
static int diff_nstack = 0; /**< Number of diffs in the stack. */
static int diff_mstack = 0; /**< Currently allocated diffs. */


/*
 * Prototypes.
 */
static UniDiff_t* diff_get( const char *name );
static UniDiff_t *diff_newDiff (void);
static int diff_removeDiff( UniDiff_t *diff );
static int diff_patchSystem( UniDiff_t *diff, xmlNodePtr node );
static int diff_patchShip( UniDiff_t *diff, xmlNodePtr node );
static int diff_patchOutfit( UniDiff_t *diff, xmlNodePtr node );
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
   for (i=0; i<diff_nstack; i++)
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
   uint32_t bufsize;
   char *buf;
   char *diffname;

   /* Check if already applied. */
   if (diff_isApplied(name))
      return 0;

   buf = ndata_read( DIFF_DATA, &bufsize );
   doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (strcmp((char*)node->name,"unidiffs")) {
      ERR("Malformed unidiff file: missing root element 'unidiffs'");
      return 0;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed unidiff file: does not contain elements");
      return 0;
   }

   do {
      if (xml_isNode(node,"unidiff")) {
         /* Check to see if it's the diff we're looking for. */
         xmlr_attr(node,"name",diffname);
         if (strcmp(diffname,name)==0) {
            /* Apply it. */
            diff_patch( node );

            /* Clean up. */
            free(diffname);
            xmlFreeDoc(doc);
            free(buf);

            return 0;
         }
         free(diffname);
      }
   } while (xml_nextNode(node));

   /* More clean up. */
   xmlFreeDoc(doc);
   free(buf);

   WARN("UniDiff '%s' not found in "DIFF_DATA".", name);
   return -1;
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
   xmlr_attr(node,"name",base.target.u.name);
   if (base.target.u.name==NULL)
      WARN("Unidiff '%s' has a system node without a 'name' tag", diff->name);

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      if (xml_isNode(cur,"planet")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Get the planet to modify. */
         xmlr_attr(cur,"name",hunk.u.name);

         /* Get the type. */
         buf = xml_get(cur);
         if (buf==NULL) {
            WARN("Unidiff '%s': Null hunk type.", diff->name);
            continue;
         }
         if (strcmp(buf,"add")==0)
            hunk.type = HUNK_TYPE_PLANET_ADD;
         else if (strcmp(buf,"remove")==0)
            hunk.type = HUNK_TYPE_PLANET_REMOVE;

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
      }
      else if (xml_isNode(cur, "fleet")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Get the fleet properties. */
         /* TODO Fix this up to new presence system. */
         xmlr_attr(cur,"name",buf);
         hunk.u.fleet = fleet_get(buf);
         free(buf);
         xmlr_attr(cur,"chance",buf);
         free(buf);

         /* Get the type. */
         buf = xml_get(cur);
         if (buf==NULL) {
            WARN("Unidiff '%s': Null hunk type.", diff->name);
            continue;
         }
         if (strcmp(buf,"add")==0)
            hunk.type = HUNK_TYPE_FLEET_ADD;
         else if (strcmp(buf,"remove")==0)
            hunk.type = HUNK_TYPE_FLEET_REMOVE;
         else {
            WARN("Unknown hunk type.");
            hunk.type = HUNK_TYPE_NONE;
         }

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
      }
   } while (xml_nextNode(cur));

   /* Clean up some stuff. */
   free(base.target.u.name);
   base.target.u.name = NULL;

   return 0;
}


/**
 * @brief Patches a ship.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the ship.
 *    @return 0 on success.
 */
static int diff_patchShip( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t base, hunk;
   xmlNodePtr cur;
   Ship *s;

   /* Set the target. */
   memset(&base, 0, sizeof(UniHunk_t));
   base.target.type = HUNK_TARGET_SHIP;
   xmlr_attr(node,"name",base.target.u.name);
   if (base.target.u.name==NULL) {
      WARN("Unidiff '%s' has an ship node without a 'name' tag", diff->name);
      return -1;
   }

   /* Make sure ship exists. */
   s = ship_get(base.target.u.name);
   if (s == NULL) {
      WARN("Unidiff '%s' ship '%s' to patch does not exist",
            diff->name, base.target.u.name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      if (xml_isNode(cur,"tech")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_SHIP_TECH;

         /* Get the data. */
         hunk.u.i.old = s->tech;
         hunk.u.i.new = xml_getInt(cur);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
      }
   } while (xml_nextNode(cur));

   /* Clean up some stuff. */
   free(base.target.u.name);
   base.target.u.name = NULL;

   return 0;
}


/**
 * @brief Patches an outfit.
 *
 *    @param diff Diff that is doing the patching.
 *    @param node Node containing the outfit.
 *    @return 0 on success.
 */
static int diff_patchOutfit( UniDiff_t *diff, xmlNodePtr node )
{
   UniHunk_t base, hunk;
   xmlNodePtr cur;
   Outfit *o;

   /* Set the target. */
   memset(&base, 0, sizeof(UniHunk_t));
   base.target.type = HUNK_TARGET_OUTFIT;
   xmlr_attr(node,"name",base.target.u.name);
   if (base.target.u.name==NULL) {
      WARN("Unidiff '%s' has an outfit node without a 'name' tag", diff->name);
      return -1;
   }

   /* Make sure outfit exists. */
   o = outfit_get(base.target.u.name);
   if (o == NULL) {
      WARN("Unidiff '%s' outfit '%s' to patch does not exist",
            diff->name, base.target.u.name );
      return -1;
   }

   /* Now parse the possible changes. */
   cur = node->xmlChildrenNode;
   do {
      if (xml_isNode(cur,"tech")) {
         hunk.target.type = base.target.type;
         hunk.target.u.name = strdup(base.target.u.name);

         /* Outfit type is constant. */
         hunk.type = HUNK_TYPE_OUTFIT_TECH;

         /* Get the data. */
         hunk.u.i.old = o->tech;
         hunk.u.i.new = xml_getInt(cur);

         /* Apply diff. */
         if (diff_patchHunk( &hunk ) < 0)
            diff_hunkFailed( diff, &hunk );
         else
            diff_hunkSuccess( diff, &hunk );
      }
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
   int i;
   UniDiff_t *diff;
   UniHunk_t *fail;
   xmlNodePtr node;
   char *target;

   /* Prepare it. */
   diff = diff_newDiff();
   memset(diff, 0, sizeof(UniDiff_t));
   xmlr_attr(parent,"name",diff->name);

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"system"))
         diff_patchSystem( diff, node );
      else if (xml_isNode(node, "outfit"))
         diff_patchOutfit( diff, node );
      else if (xml_isNode(node, "ship"))
         diff_patchShip( diff, node );
   } while (xml_nextNode(node));

   if (diff->nfailed > 0) {
      DEBUG("Unidiff '%s' failed to apply %d hunks.", diff->name, diff->nfailed);
      for (i=0; i<diff->nfailed; i++) {
         fail = &diff->failed[i];
         target = fail->target.u.name;
         switch (fail->type) {
            case HUNK_TYPE_PLANET_ADD:
               DEBUG("   [%s] planet add: '%s'", target, fail->u.name);
               break;
            case HUNK_TYPE_PLANET_REMOVE:
               DEBUG("   [%s] planet remove: '%s'", target, fail->u.name);
               break;
            case HUNK_TYPE_FLEET_ADD:
               DEBUG("   [%s] fleet add: '%s'", target, 
                     fail->u.fleet->name );
               break;
            case HUNK_TYPE_FLEET_REMOVE:
               DEBUG("   [%s] fleet remove: '%s'", target,
                     fail->u.fleet->name );
               break;

            default:
               DEBUG("   unknown hunk '%d'", fail->type);
               break;
         }
      }
   }

   return 0;
}


/**
 * @brief Applies a hunk and adds it to the diff.
 *
 *    @param diff Diff to which the hunk belongs.
 *    @param hunk Hunk to apply.
 *    @return 0 on success.
 */
static int diff_patchHunk( UniHunk_t *hunk )
{
   Ship *s;
   Outfit *o;
   int i;

   switch (hunk->type) {

      /* Adding a planet. */
      case HUNK_TYPE_PLANET_ADD:
         return system_addPlanet( system_get(hunk->target.u.name), hunk->u.name );
      /* Removing a planet. */
      case HUNK_TYPE_PLANET_REMOVE:
         return system_rmPlanet( system_get(hunk->target.u.name), hunk->u.name );

      /* Adding a fleet. */
      case HUNK_TYPE_FLEET_ADD:
         return system_addFleet( system_get(hunk->target.u.name), hunk->u.fleet );
      /* Removing a fleet. */
      case HUNK_TYPE_FLEET_REMOVE:
         return system_rmFleet( system_get(hunk->target.u.name), hunk->u.fleet );

      /* Changing a ship's technology. */
      case HUNK_TYPE_SHIP_TECH:
         s = ship_get(hunk->target.u.name);
         if (s==NULL)
            return -1;
         s->tech = hunk->u.i.new;
         /* Invert it so when it gets removed it works. */
         i = hunk->u.i.old;
         hunk->u.i.old = hunk->u.i.new;
         hunk->u.i.new = i;
         return 0;

      /* Changing an outfit's technology. */
      case HUNK_TYPE_OUTFIT_TECH:
         o = outfit_get(hunk->target.u.name);
         if (o==NULL)
            return -1;
         o->tech = hunk->u.i.new;
         /* Invert it so when it gets removed it works. */
         i = hunk->u.i.old;
         hunk->u.i.old = hunk->u.i.new;
         hunk->u.i.new = i;
         return 0;

      default:
         WARN("Unknown hunk type '%d'.", hunk->type);
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
   memcpy( &diff->failed[diff->nfailed-1], hunk, sizeof(UniHunk_t) );
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
   memcpy( &diff->applied[diff->napplied-1], hunk, sizeof(UniHunk_t) );
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
}


/**
 * @brief Removes all active diffs.
 */
void diff_clear (void)
{
   while (diff_nstack > 0) {
      diff_removeDiff(&diff_stack[diff_nstack-1]);
   }
}


/**
 * @brief Creates a new UniDiff_t for usage.
 *
 *    @return A newly created UniDiff_t.
 */
static UniDiff_t *diff_newDiff (void)
{
   /* Check if needs initialization. */
   if (diff_stack == NULL) {
      diff_mstack = CHUNK_SIZE;
      diff_stack = malloc(diff_mstack * sizeof(UniDiff_t));
      diff_nstack = 1;
      return &diff_stack[0];
   }

   diff_nstack++;
   /* Check if need to grow. */
   if (diff_nstack > diff_mstack) {
      diff_mstack += CHUNK_SIZE;
      diff_stack = realloc(diff_stack, diff_mstack * sizeof(UniDiff_t));
   }

   return &diff_stack[diff_nstack-1];
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
      memcpy( &hunk, &diff->applied[i], sizeof(UniHunk_t) );
      /* Invert the type for reverting. */
      switch (hunk.type) {
         case HUNK_TYPE_PLANET_ADD:
            hunk.type = HUNK_TYPE_PLANET_REMOVE;
            break;

         case HUNK_TYPE_PLANET_REMOVE:
            hunk.type = HUNK_TYPE_PLANET_ADD;
            break;

         case HUNK_TYPE_FLEET_ADD:
            hunk.type = HUNK_TYPE_FLEET_REMOVE;
            break;

         case HUNK_TYPE_FLEET_REMOVE:
            hunk.type = HUNK_TYPE_FLEET_ADD;
            break;

         /* Doesn't need invert. */
         case HUNK_TYPE_SHIP_TECH:
         case HUNK_TYPE_OUTFIT_TECH:
            break;

         default:
            WARN("Unknown Hunk type '%d'.", hunk.type);
            continue;
      }

      if (diff_patchHunk(&hunk))
         DEBUG("Failed to remove hunk type '%d'.", hunk.type);
   }

   diff_cleanup(diff);
   diff_nstack--;
   i = diff - diff_stack;
   memmove(&diff_stack[i], &diff_stack[i+1], sizeof(UniDiff_t) * (diff_nstack-i));

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
      case HUNK_TYPE_PLANET_ADD:
      case HUNK_TYPE_PLANET_REMOVE:
         if (hunk->u.name != NULL)
            free(hunk->u.name);
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
   for (i=0; i<diff_nstack; i++) {
      diff = &diff_stack[i];

      xmlw_elem(writer, "diff", "%s", diff->name);
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

   diff_clear();

   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"diffs")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"diff"))
               diff_apply( xml_get(cur) );
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;

}


