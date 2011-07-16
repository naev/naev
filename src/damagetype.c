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
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_haptic.h"
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

#include "log.h"
#include "pilot.h"
#include "pause.h"
#include "rng.h"
#include "ndata.h"
#include "nxml.h"


#define dtype_XML_ID     "dtypes" /**< XML Document tag. */
#define dtype_XML_TAG    "dtype" /**< DTYPE XML node tag. */

#define dtype_DATA       "dat/damagetype.xml" /**< Location of the spfx datafile. */

#define dtype_CHUNK_MAX  16384 /**< Maximum chunk to alloc when needed */
#define dtype_CHUNK_MIN  256 /**< Minimum chunk to alloc when needed */

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

static DTYPE *dtype_types = NULL; /**< Total damage types. */
static int dtype_ntypes = 0; /**< Total number of damage types. */

/*
 * prototypes
 */
static int DTYPE_parse( DTYPE *temp, const xmlNodePtr parent );
static void DTYPE_free( DTYPE *damtype );

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
      xmlr_float(node, "sdam", temp->sdam);
      xmlr_float(node, "adam", temp->adam);
      xmlr_float(node, "knock", temp->knock);
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
   if (o) WARN("DTYPE '%s' invalid '"s"' element", temp->name) /**< Define to help check for data errors. */
   MELEMENT(temp->sdam<0.,"sdam");
   MELEMENT(temp->adam<0.,"adam");
   MELEMENT(temp->adam<0.,"knock");
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
   return -1;
}


/**
 * @brief Gets the human readable string from damage type.
 */
char* dtype_damageTypeToStr( int type )
{
   if (type <= dtype_ntypes)
      return dtype_types[type].name;
   else
      return "none";
      /* TODO: And probably generate a warning. */
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
   buf = ndata_read( dtype_DATA, &bufsize );
   doc = xmlParseMemory( buf, bufsize );

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,dtype_XML_ID)) {
      ERR("Malformed '"dtype_DATA"' file: missing root element '"dtype_XML_ID"'");
      return -1;
   }

   /* Check to see if is populated. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"dtype_DATA"' file: does not contain elements");
      return -1;
   }

   /* Load up the individual damage types. */
   mem = 0;
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,dtype_XML_TAG)) {

         dtype_ntypes++;
         if (dtype_ntypes > mem) {
            if (mem == 0)
               mem = dtype_CHUNK_MIN;
            else
               mem *= 2;
            dtype_types = realloc(dtype_types, sizeof(DTYPE)*mem);
         }
         DTYPE_parse( &dtype_types[dtype_ntypes-1], node );
      }
      else
         WARN("'"dtype_DATA"' has unknown node '%s'.", node->name);
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
   free(dtype_types);
   dtype_types = NULL;
   dtype_ntypes = 0;
}


/**
 * @brief Gives the real shield damage, armour damage and knockback modifier.
 *
 *    @param[out] dshield Real shield damage.
 *    @param[out] darmour Real armour damage.
 *    @param[out] knockback Knocback modifier.
 *    @param[in] absorb Absorption value.
 *    @param[in] dmg Damage information.
 */
void dtype_calcDamage( double *dshield, double *darmour, double absorb, double *knockback, const Damage *dmg )
{
   double sdam, adam, knock;

   sdam           = dtype_types[dmg->type].sdam;
   adam           = dtype_types[dmg->type].adam;
   knock          = dtype_types[dmg->type].knock;

   if (dshield)   *dshield    = dmg->damage  * sdam * absorb;
   if (darmour)   *darmour    = dmg->damage  * adam * absorb;
   if (knockback) *knockback  = knock;
}


