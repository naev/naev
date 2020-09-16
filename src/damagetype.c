/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file damagetype.c
 *
 * @brief Handles damage types.
 */


#include "damagetype.h"
#include "naev.h"

#include <inttypes.h>

#include "SDL.h"

#include "log.h"
#include "array.h"
#include "pilot.h"
#include "pause.h"
#include "rng.h"
#include "ndata.h"
#include "nxml.h"
#include "shipstats.h"


#define DTYPE_XML_ID     "dtypes"   /**< XML Document tag. */
#define DTYPE_XML_TAG    "dtype"    /**< DTYPE XML node tag. */

/**
 * @struct DTYPE
 *
 * @brief A damage type.
 */
typedef struct DTYPE_ {
   char* name;    /**< Name of the damage type */
   double sdam;   /**< Shield damage multiplier */
   double adam;   /**< Armour damage multiplier */
   double knock;  /**< Knockback */
   size_t soffset; /**< Offset for shield modifier ship statistic. */
   size_t aoffset; /**< Offset for armour modifier ship statistic. */
} DTYPE;

static DTYPE* dtype_types  = NULL;  /**< Total damage types. */


/*
 * prototypes
 */
static int DTYPE_parse( DTYPE *temp, const xmlNodePtr parent );
static void DTYPE_free( DTYPE *damtype );
static DTYPE* dtype_validType( int type );


/**
 * @brief Parses an xml node containing a DTYPE.
 *
 *    @param temp Address to load DTYPE into.
 *    @param parent XML Node containing the DTYPE data.
 *    @return 0 on success.
 */
static int DTYPE_parse( DTYPE *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;
   char *stat;

   /* Clear data. */
   memset( temp, 0, sizeof(DTYPE) );

   /* Get the name (mallocs). */
   temp->name = xml_nodeProp(parent,"name");

   /* Extract the data. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      if (xml_isNode(node, "shield")) {
         temp->sdam = xml_getFloat(node);

         xmlr_attr(node, "stat", stat);
         if (stat != NULL) {
            temp->soffset = ss_offsetFromType( ss_typeFromName(stat) );
            free(stat);
         }

         continue;
      }
      else if (xml_isNode(node, "armour")) {
         temp->adam = xml_getFloat(node);

         xmlr_attr(node, "stat", stat);
         if (stat != NULL) {
            temp->aoffset = ss_offsetFromType( ss_typeFromName(stat) );
            free(stat);
         }

         continue;
      }
      xmlr_float(node, "armour", temp->adam);
      xmlr_float(node, "knockback", temp->knock);

      WARN("Unknown node of type '%s' in damage node '%s'.", node->name, temp->name);
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
   if (o) WARN("DTYPE '%s' invalid '"s"' element", temp->name) /**< Define to help check for data errors. */
   MELEMENT(temp->sdam<0.,"shield");
   MELEMENT(temp->adam<0.,"armour");
   MELEMENT(temp->knock<0.,"knockback");
#undef MELEMENT

   return 0;
}


/**
 * @brief Frees a DTYPE.
 *
 *    @param damtype DTYPE to free.
 */
static void DTYPE_free( DTYPE *damtype )
{
   if (damtype->name != NULL) {
      free(damtype->name);
      damtype->name = NULL;
   }
}


/**
 * @brief Gets the id of a dtype based on name.
 *
 *    @param name Name to match.
 *    @return ID of the damage type or -1 on error.
 */
int dtype_get( char* name )
{
   int i;
   for (i=0; i<array_size(dtype_types); i++)
      if (strcmp(dtype_types[i].name, name)==0)
         return i;
   WARN("Damage type '%s' not found in stack.", name);
   return -1;
}


/**
 * @brief Gets the damage type.
 */
static DTYPE* dtype_validType( int type )
{
   if ((type < 0) || (type >= array_size(dtype_types))) {
      WARN("Damage type '%d' is invalid.", type);
      return NULL;
   }
   return &dtype_types[ type ];
}


/**
 * @brief Gets the human readable string from damage type.
 */
char* dtype_damageTypeToStr( int type )
{
   DTYPE *dmg = dtype_validType( type );
   if (dmg == NULL)
      return NULL;
   return dmg->name;
}


/**
 * @brief Loads the dtype stack.
 *
 *    @return 0 on success.
 */
int dtype_load (void)
{
   size_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Load and read the data. */
   buf = ndata_read( DTYPE_DATA_PATH, &bufsize );
   if (buf == NULL) {
      WARN(_("Unable to read data from '%s'"), DTYPE_DATA_PATH);
      return -1;
   }

   /* Load the document. */
   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL) {
      WARN(_("Unable to parse document '%s'"), DTYPE_DATA_PATH);
      return -1;
   }

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,DTYPE_XML_ID)) {
      ERR("Malformed '"DTYPE_DATA_PATH"' file: missing root element '"DTYPE_XML_ID"'");
      return -1;
   }

   /* Check to see if is populated. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"DTYPE_DATA_PATH"' file: does not contain elements");
      return -1;
   }

   /* Load up the individual damage types. */
   dtype_types = array_create(DTYPE);
   do {
      xml_onlyNodes(node);

      if (!xml_isNode(node,DTYPE_XML_TAG)) {
         WARN("'"DTYPE_DATA_PATH"' has unknown node '%s'.", node->name);
         continue;
      }

      DTYPE_parse( &array_grow( &dtype_types ), node );

   } while (xml_nextNode(node));
   /* Shrink back to minimum - shouldn't change ever. */
   array_shrink( &dtype_types );

   /* Clean up. */
   xmlFreeDoc(doc);
   free(buf);

   return 0;
}


/**
 * @brief Frees the dtype stack.
 */
void dtype_free (void)
{
   int i;

   /* clear the damtypes */
   for (i=0; i<array_size(dtype_types); i++)
      DTYPE_free( &dtype_types[i] );
   array_free( dtype_types );
   dtype_types    = NULL;
}


/**
 * @brief Gives the real shield damage, armour damage and knockback modifier.
 *
 *    @param[out] dshield Real shield damage.
 *    @param[out] darmour Real armour damage.
 *    @param[out] knockback Knockback modifier.
 *    @param[in] absorb Absorption value.
 *    @param[in] dmg Damage information.
 */
void dtype_calcDamage( double *dshield, double *darmour, double absorb, double *knockback, const Damage *dmg, ShipStats *s )
{
   DTYPE *dtype;
   char *ptr;
   double multiplier;

   /* Must be valid. */
   dtype = dtype_validType( dmg->type );
   if (dtype == NULL)
      return;

   /* Set if non-nil. */
   if (dshield != NULL) {
      if ((dtype->soffset <= 0) || (s == NULL))
         *dshield    = dtype->sdam * dmg->damage * absorb;
      else {
         /*
          * If an offset has been specified, look for a double at that offset
          * in the ShipStats struct, and used it as a multiplier.
          *
          * The 2. - n logic serves to undo the initialization done by
          * ss_statsInit and turn the value into a multiplier.
          */
         ptr = (char*) s;
         memcpy(&multiplier, &ptr[ dtype->soffset ], sizeof(double));
         *dshield = dtype->sdam * dmg->damage * absorb *
               (2. - multiplier);
      }
   }
   if (darmour != NULL) {
      if ((dtype->aoffset) <= 0 || (s == NULL))
         *darmour    = dtype->adam * dmg->damage * absorb;
      else {
         ptr = (char*) s;
         memcpy(&multiplier, &ptr[ dtype->aoffset ], sizeof(double));
         *darmour = dtype->adam * dmg->damage * absorb *
               (2. - multiplier);
      }
   }

   if (knockback != NULL)
      *knockback  = dtype->knock;
}


