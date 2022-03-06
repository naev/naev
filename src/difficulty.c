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

#define DIFFICULTY_XML_ID     "difficulties"
#define DIFFICULTY_XML_NODE   "difficulty"

static Difficulty *difficulty_stack = NULL;
static const Difficulty *difficulty_default = NULL;
static const Difficulty *difficulty_global  = NULL;
static const Difficulty *difficulty_current = NULL;

/**
 * @brief Loads the difficulty modes.
 */
int difficulty_load (void)
{
   xmlDocPtr doc;
   xmlNodePtr node;
   Difficulty d;
   difficulty_stack = array_create( Difficulty );

   /* Load and read the data. */
   doc = xml_parsePhysFS( DIFFICULTY_PATH );
   if (doc == NULL)
      return -1;

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,DIFFICULTY_XML_ID)) {
      ERR( _("Malformed '%s' file: missing root element '%s'"), DIFFICULTY_PATH, DIFFICULTY_XML_ID);
      return -1;
   }

   node = node->xmlChildrenNode;
   do {
      xmlNodePtr cur;
      xml_onlyNodes(node);

      /* Initialize. */
      memset( &d, 0, sizeof(Difficulty) );

      /* Properties. */
      xmlr_attr_strd( node, "name", d.name );
      xmlr_attr_int_opt( node, "default", d.def );

      /* Get the stats. */
      cur = node->xmlChildrenNode;
      do {
         xml_onlyNodes(cur);
         ShipStatList *ll = ss_listFromXML( cur );
         if (ll != NULL) {
            ll->next = d.stats;
            d.stats = ll;
            continue;
         }
         WARN(_("Difficulty '%s' has unknown node '%s'"), d.name, cur->name);
      } while (xml_nextNode(cur));

      array_push_back( &difficulty_stack, d );
   } while (xml_nextNode(node));

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
      difficulty_global = difficulty_get( conf.difficulty );

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

/**
 * @brief Sets the difficulty mode.
 */
void difficulty_set( const Difficulty *d )
{
   if (d==NULL)
      difficulty_current = difficulty_getDefault();
   else
      difficulty_current = d;
}

/**
 * @brief Sets the global difficulty.
 */
void difficulty_setGlobal( const Difficulty *d )
{
   difficulty_global = d;
}

/**
 * @brief Applies the current difficulty mode setting to the player.
 */
int difficulty_apply( ShipStats *s )
{
   return ss_statsModFromList( s, difficulty_current->stats );
}
