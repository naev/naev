/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player.c
 *
 * @brief Contains all the player related stuff.
 */
/** @cond */
#include <stdlib.h>
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "difficulty.h"

#include "conf.h"
#include "array.h"
#include "ndata.h"
#include "nxml.h"

#define DIFFICULTY_XML_ID   "difficulty"

static Difficulty *difficulty_stack = NULL;
static const Difficulty *difficulty_default = NULL;
static const Difficulty *difficulty_global  = NULL;
static const Difficulty *difficulty_local   = NULL;
static const Difficulty *difficulty_current = NULL;

/**
 * @brief Loads the difficulty modes.
 */
int difficulty_load (void)
{
   char **difficulty_files = ndata_listRecursive( DIFFICULTY_PATH );
   difficulty_stack = array_create( Difficulty );
   for (int i=0; i<array_size(difficulty_files); i++) {
      Difficulty d;
      xmlDocPtr doc;
      xmlNodePtr node, cur;

      /* Load and read the data. */
      doc = xml_parsePhysFS( difficulty_files[i] );
      if (doc == NULL)
         continue;

      /* Check to see if document exists. */
      node = doc->xmlChildrenNode;
      if (!xml_isNode(node,DIFFICULTY_XML_ID)) {
         ERR( _("Malformed '%s' file: missing root element '%s'"), difficulty_files[i], DIFFICULTY_XML_ID);
         xmlFreeDoc( doc );
         continue;
      }

      /* Initialize. */
      memset( &d, 0, sizeof(Difficulty) );

      /* Properties. */
      xmlr_attr_strd( node, "name", d.name );
      xmlr_attr_int_opt( node, "default", d.def );

      /* Get the stats. */
      cur = node->xmlChildrenNode;
      do {
         xml_onlyNodes(cur);

         /* Load the description. */
         xmlr_strd( cur, "description", d.description );

         /* Rest should be ship stats. */
         ShipStatList *ll = ss_listFromXML( cur );
         if (ll != NULL) {
            ll->next = d.stats;
            d.stats = ll;
            continue;
         }
         WARN(_("Difficulty '%s' has unknown node '%s'"), d.name, cur->name);
      } while (xml_nextNode(cur));

      xmlFreeDoc( doc );

      array_push_back( &difficulty_stack, d );
   }

   /* Find out default. */
   for (int i=0; i<array_size(difficulty_stack); i++) {
      Difficulty *dd = &difficulty_stack[i];
      if (dd->def) {
         if (difficulty_default)
            WARN(_("More than one difficulty with default flag set!"));
         difficulty_default = dd;
      }
   }
   if (difficulty_default==NULL) {
      WARN(_("No default difficulty set!"));
      difficulty_default = difficulty_stack;
   }
   difficulty_current = difficulty_default; /* Load default. */

   /* Load the global difficulty. */
   if (conf.difficulty != NULL)
      difficulty_setGlobal( difficulty_get( conf.difficulty ) );

   for (int i=0; i<array_size(difficulty_files); i++)
      free( difficulty_files[i] );
   array_free( difficulty_files );
   return 0;
}

/**
 * @brief Frees the difficulty modes.
 */
void difficulty_free (void)
{
   for (int i=0; i<array_size(difficulty_stack); i++) {
      Difficulty *d = &difficulty_stack[i];
      free( d->name );
      free( d->description );
      ss_free( d->stats );
   }
   array_free( difficulty_stack );
}

/**
 * @brief Gets the current difficulty.
 */
const Difficulty *difficulty_cur (void)
{
   return difficulty_current;
}

/**
 * @brief Returns the list of difficulty modes.
 */
const Difficulty *difficulty_getAll (void)
{
   return difficulty_stack;
}

static const Difficulty *difficulty_getDefault (void)
{
   if (difficulty_global != NULL)
      return difficulty_global;
   else
      return difficulty_default;
}

/**
 * @brief Gets a difficulty.
 */
const Difficulty *difficulty_get( const char *name )
{
   if (name==NULL)
      return difficulty_getDefault();
   for (int i=0; i<array_size(difficulty_stack); i++) {
      const Difficulty *d = &difficulty_stack[i];
      if (strcmp(name, d->name)==0)
         return d;
   }
   WARN(_("Uknown difficulty setting '%s'"), name);
   return difficulty_default;
}

static void difficulty_update (void)
{
   if (difficulty_local != NULL)
      difficulty_current = difficulty_local;
   else if (difficulty_global != NULL)
      difficulty_current = difficulty_global;
   else
      difficulty_current = difficulty_default;
}

/**
 * @brief Sets the global difficulty.
 */
void difficulty_setGlobal( const Difficulty *d )
{
   difficulty_global = d;
   difficulty_update();
}

/**
 * @brief Sets the local difficulty.
 */
void difficulty_setLocal( const Difficulty *d )
{
   difficulty_local = d;
   difficulty_update();
}

/**
 * @brief Generates a translated display text for the difficulty.
 *
 * Note: must be manually freed.
 */
char *difficulty_display( const Difficulty *d )
{
   int l;
   char *display = malloc( STRMAX );
   l = scnprintf( display, STRMAX, _("Difficulty %s"), _(d->name) );
   l += scnprintf( &display[l], STRMAX-l, "\n" );
   if (d->description != NULL)
      l += scnprintf( &display[l], STRMAX-l, "%s\n", _(d->description) );
   l += scnprintf( &display[l], STRMAX-l, _("This difficulty applies the following effect to the player ships:") );
   ss_statsListDesc( d->stats, &display[l], STRMAX-l, 1 );
   return display;
}

/**
 * @brief Applies the current difficulty mode setting to the player.
 */
int difficulty_apply( ShipStats *s )
{
   return ss_statsMergeFromList( s, difficulty_current->stats );
}
