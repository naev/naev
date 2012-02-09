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
#include "pilot.h"
#include "pause.h"
#include "rng.h"
#include "ndata.h"
#include "nxml.h"


#define DTYPE_XML_ID     "dtypes"   /**< XML Document tag. */
#define DTYPE_XML_TAG    "dtype"    /**< DTYPE XML node tag. */

#define DTYPE_DATA       "dat/damagetype.xml" /**< Location of the spfx datafile. */

#define DTYPE_CHUNK_MIN  256     /**< Minimum chunk to alloc when needed */

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
} DTYPE;

static DTYPE* dtype_types  = NULL;  /**< Total damage types. */
static int dtype_ntypes    = 0;     /**< Total number of damage types. */


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

   /* Clear data. */
   memset( temp, 0, sizeof(DTYPE) );

   /* Get the name (mallocs). */
   temp->name = xml_nodeProp(parent,"name");

   /* Extract the data. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);

      xmlr_float(node, "shield", temp->sdam);
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
   for (i=0; i<dtype_ntypes; i++)
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
   if ((type < 0) || (type >= dtype_ntypes)) {
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
   int mem;
   uint32_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Load and read the data. */
   buf = ndata_read( DTYPE_DATA, &bufsize );
   doc = xmlParseMemory( buf, bufsize );

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,DTYPE_XML_ID)) {
      ERR("Malformed '"DTYPE_DATA"' file: missing root element '"DTYPE_XML_ID"'");
      return -1;
   }

   /* Check to see if is populated. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"DTYPE_DATA"' file: does not contain elements");
      return -1;
   }

   /* Load up the individual damage types. */
   mem = 0;
   do {
      xml_onlyNodes(node);

      if (!xml_isNode(node,DTYPE_XML_TAG)) {
         WARN("'"DTYPE_DATA"' has unknown node '%s'.", node->name);
         continue;
      }

      dtype_ntypes++;
      if (dtype_ntypes > mem) {
         if (mem == 0)
            mem = DTYPE_CHUNK_MIN;
         else
            mem *= 2;
         dtype_types = realloc(dtype_types, sizeof(DTYPE)*mem);
      }
      DTYPE_parse( &dtype_types[dtype_ntypes-1], node );

   } while (xml_nextNode(node));
   /* Shrink back to minimum - shouldn't change ever. */
   dtype_types = realloc(dtype_types, sizeof(DTYPE) * dtype_ntypes);

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
   for (i=0; i<dtype_ntypes; i++)
      DTYPE_free( &dtype_types[i] );
   free( dtype_types );
   dtype_types    = NULL;
   dtype_ntypes   = 0;
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
void dtype_calcDamage( double *dshield, double *darmour, double absorb, double *knockback, const Damage *dmg )
{
   DTYPE *dtype;

   /* Must be valid. */
   dtype = dtype_validType( dmg->type );
   if (dtype == NULL)
      return;

   /* Set if non-nil. */
   if (dshield != NULL)
      *dshield    = dtype->sdam * dmg->damage * absorb;
   if (darmour != NULL)
      *darmour    = dtype->adam * dmg->damage * absorb;
   if (knockback != NULL)
      *knockback  = dtype->knock;
}


