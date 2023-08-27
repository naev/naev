/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file slots.c
 *
 * @brief Handles the slot properties.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "slots.h"

#include "array.h"
#include "log.h"
#include "nxml.h"
#include "ndata.h"

#define SP_XML_ID    "slot" /**< SP XML node tag. */

/**
 * @brief Representation of a slot property.
 */
typedef struct SlotProperty_s {
   char *name;       /**< Internal name of the property. */
   char *display;    /**< Display name of the property. */
   char *description;/**< Description of the property. */
   int required;     /**< Required slot property. */
   int exclusive;    /**< Exclusive slot property. */
   int locked;       /**< Locked and not modifyable by the player. */
   glTexture *icon;  /**< Texture to use for the slot. */
} SlotProperty_t;

static SlotProperty_t *sp_array = NULL; /**< Slot property array. */

/*
 * Prototypes.
 */
static int sp_check( unsigned int spid );

/**
 * @brief Initializes the slot properties.
 */
int sp_load (void)
{
   char **sp_files = ndata_listRecursive( SP_DATA_PATH );

   /* First pass, loads up ammunition. */
   sp_array = array_create( SlotProperty_t );
   for (int i=0; i<array_size(sp_files); i++) {
      SlotProperty_t *sp;
      xmlNodePtr node, cur;
      xmlDocPtr doc;

      /* Load and read the data. */
      doc = xml_parsePhysFS( sp_files[i] );
      if (doc == NULL)
         continue;

      /* Check to see if document exists. */
      node = doc->xmlChildrenNode;
      if (!xml_isNode(node,SP_XML_ID)) {
         WARN(_("Malformed '%s' file: missing root element '%s'"), sp_files[i], SP_XML_ID );
         continue;
      }

      sp    = &array_grow( &sp_array );
      memset( sp, 0, sizeof(SlotProperty_t) );
      xmlr_attr_strd( node, "name", sp->name );
      cur   = node->xmlChildrenNode;
      do {
         xml_onlyNodes(cur);

         /* Load data. */
         xmlr_strd( cur, "display", sp->display );
         xmlr_strd( cur, "description", sp->description );
         if (xml_isNode( cur, "required" )) {
            sp->required = 1;
            continue;
         }
         if (xml_isNode( cur, "exclusive" )) {
            sp->exclusive = 1;
            continue;
         }
         if (xml_isNode( cur, "locked" )) {
            sp->locked = 1;
            continue;
         }
         if (xml_isNode( cur, "icon" )) {
            char path[STRMAX_SHORT];
            snprintf( path, sizeof(path), "gfx/slots/%s", xml_get(cur) );
            sp->icon = xml_parseTexture( cur, path, 1, 1, 0 );
            continue;
         }


         WARN(_("Slot Property '%s' has unknown node '%s'."), node->name, cur->name);
      } while (xml_nextNode(cur));

      /* Clean up. */
      xmlFreeDoc(doc);
   }

   for (int i=0; i<array_size(sp_files); i++)
      free( sp_files[i] );
   array_free( sp_files );
   return 0;
}

/**
 * @brief Cleans up after the slot properties.
 */
void sp_cleanup (void)
{
   for (int i=0; i<array_size(sp_array); i++) {
      SlotProperty_t *sp = &sp_array[i];
      free( sp->name );
      free( sp->display );
      free( sp->description );
      gl_freeTexture( sp->icon );
   }
   array_free( sp_array );
   sp_array = NULL;
}

/**
 * @brief Gets the id of a slot property.
 *
 *    @param name Name to match.
 *    @return ID of the slot property.
 */
unsigned int sp_get( const char *name )
{
   if (name==NULL)
      return 0;
   for (int i=0; i<array_size(sp_array); i++) {
      SlotProperty_t *sp = &sp_array[i];
      if (strcmp( sp->name, name ) == 0)
         return i+1;
   }
   WARN(_("Slot property '%s' not found in array."), name);
   return 0;
}

/**
 * @brief Checks to see if in bound of array.
 */
static int sp_check( unsigned int spid )
{
   if ((spid==0) || (spid > (unsigned int)array_size(sp_array)))
      return 1;
   return 0;
}

/**
 * @brief Gets the display name of a slot property (in English).
 */
const char *sp_display( unsigned int spid )
{
   if (sp_check(spid))
      return NULL;
   return sp_array[ spid-1 ].display;
}

/**
 * @brief Gets the description of a slot property (in English).
 */
const char *sp_description( unsigned int spid )
{
   if (sp_check(spid))
      return NULL;
   return sp_array[ spid-1 ].description;
}

/**
 * @brief Gets whether or not a slot property is required.
 */
int sp_required( unsigned int spid )
{
   if (sp_check(spid))
      return 0;
   return sp_array[ spid-1 ].required;
}

/**
 * @brief Gets whether or not a slot property is exclusive.
 */
int sp_exclusive( unsigned int spid )
{
   if (sp_check(spid))
      return 0;
   return sp_array[ spid-1 ].exclusive;
}

/**
 * @brief Gets whether or not a slot property is locked.
 */
int sp_locked( unsigned int spid )
{
   if (sp_check(spid))
      return 0;
   return sp_array[ spid-1 ].locked;
}

/**
 * @brief Gets the icon associated with the slot.
 */
const glTexture * sp_icon( unsigned int spid )
{
   if (sp_check(spid))
      return 0;
   return sp_array[ spid-1 ].icon;
}
