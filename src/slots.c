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
#include "ndata.h"
#include "nxml.h"


#define SP_XML_ID     "Slots" /**< XML Document tag. */
#define SP_XML_TAG    "slot" /**< SP XML node tag. */

#define SP_DATA_PATH  "slots.xml" /**< Location of the sp datafile. */


/**
 * @brief Representation of a slot property.
 */
typedef struct SlotProperty_s {
   char *name;          /**< Internal name of the property. */
   char *display;       /**< Display name of the property. */
   char *description;   /**< Description of the property. */
   int required;        /**< Required slot property. */
   int exclusive;       /**< Exclusive slot property. */
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
   size_t bufsize;
   char *buf;
   xmlNodePtr node, cur;
   xmlDocPtr doc;
   SlotProperty_t *sp;

   /* Load and read the data. */
   buf = ndata_read( SP_DATA_PATH, &bufsize );
   if (buf == NULL) {
      WARN(_("Unable to read data from '%s'"), SP_DATA_PATH);
      return -1;
   }

   /* Load the document. */
   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL) {
      WARN(_("Unable to parse document '%s'"), SP_DATA_PATH);
      return -1;
   }

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,SP_XML_ID)) {
      ERR(_("Malformed '%s' file: missing root element '%s'"), SP_DATA_PATH, SP_XML_ID);
      return -1;
   }

   /* Check to see if is populated. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR(_("Malformed '%s' file: does not contain elements"), SP_DATA_PATH);
      return -1;
   }

   /* First pass, loads up ammunition. */
   sp_array = array_create( SlotProperty_t );
   do {
      xml_onlyNodes(node);
      if (!xml_isNode(node,SP_XML_TAG)) {
         WARN(_("'%s' has unknown node '%s'."), SP_DATA_PATH, node->name);
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

         WARN(_("Slot Property '%s' has unknown node '%s'."), node->name, cur->name);
      } while (xml_nextNode(cur));

   } while (xml_nextNode(node));

   /* Clean up. */
   xmlFreeDoc(doc);
   free(buf);

   return 0;
}


/**
 * @brief Cleans up after the slot properties.
 */
void sp_cleanup (void)
{
   int i;
   SlotProperty_t *sp;
   if (sp_array != NULL) {
      for (i=0; i<array_size(sp_array); i++) {
         sp = &sp_array[i];
         free( sp->name );
         free( sp->display );
         free( sp->description );
      }
      array_free( sp_array );
      sp_array = NULL;
   }
}


/**
 * @brief Gets the id of a slot property.
 *
 *    @param name Name to match.
 *    @return ID of the slot property.
 */
unsigned int sp_get( const char *name )
{
   int i;
   SlotProperty_t *sp;
   if (name==NULL)
      return 0;
   for (i=0; i<array_size(sp_array); i++) {
      sp = &sp_array[i];
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




