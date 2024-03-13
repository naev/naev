/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file damagetype.c
 *
 * @brief Handles damage types.
 */
/** @cond */
#include "SDL.h"
#include <inttypes.h>

#include "naev.h"
/** @endcond */

#include "damagetype.h"

#include "array.h"
#include "log.h"
#include "ndata.h"
#include "nxml.h"
#include "pause.h"
#include "pilot.h"
#include "rng.h"
#include "shipstats.h"

#define DTYPE_XML_ID "dtype" /**< XML Document tag. */

/**
 * @struct DTYPE
 *
 * @brief A damage type.
 */
typedef struct DTYPE_ {
   char  *name;    /**< Name of the damage type */
   char  *display; /**< Display name of the damage type. */
   double sdam;    /**< Shield damage multiplier */
   double adam;    /**< Armour damage multiplier */
   double knock;   /**< Knockback */
   size_t soffset; /**< Offset for shield modifier ship statistic. */
   size_t aoffset; /**< Offset for armour modifier ship statistic. */
} DTYPE;

static DTYPE *dtype_types = NULL; /**< Total damage types. */

/*
 * prototypes
 */
static int    DTYPE_parse( DTYPE *temp, const char *file );
static void   DTYPE_free( DTYPE *damtype );
static DTYPE *dtype_validType( int type );
static int    dtype_cmp( const void *p1, const void *p2 );

/**
 * @brief For sorting and bsearching.
 */
static int dtype_cmp( const void *p1, const void *p2 )
{
   const DTYPE *d1 = (const DTYPE *)p1;
   const DTYPE *d2 = (const DTYPE *)p2;
   return strcmp( d1->name, d2->name );
}

/**
 * @brief Parses an XML file containing a DTYPE.
 *
 *    @param temp Address to load DTYPE into.
 *    @param file File to parse.
 *    @return 0 on success.
 */
static int DTYPE_parse( DTYPE *temp, const char *file )
{
   xmlNodePtr node, parent;
   xmlDocPtr  doc;
   char      *stat;

   /* Load and read the data. */
   doc = xml_parsePhysFS( file );
   if ( doc == NULL )
      return -1;

   /* Check to see if document exists. */
   parent = doc->xmlChildrenNode;
   if ( !xml_isNode( parent, DTYPE_XML_ID ) ) {
      WARN( _( "Malformed '%s' file: missing root element '%s'" ), file,
            DTYPE_XML_ID );
      return -1;
   }

   /* Clear data. */
   memset( temp, 0, sizeof( DTYPE ) );

   xmlr_attr_strd( parent, "name", temp->name );
   xmlr_attr_strd( parent, "display", temp->display );

   /* Extract the data. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes( node );

      if ( xml_isNode( node, "shield" ) ) {
         temp->sdam = xml_getFloat( node );

         xmlr_attr_strd( node, "stat", stat );
         if ( stat != NULL ) {
            temp->soffset = ss_offsetFromType( ss_typeFromName( stat ) );
            free( stat );
         }

         continue;
      } else if ( xml_isNode( node, "armour" ) ) {
         temp->adam = xml_getFloat( node );

         xmlr_attr_strd( node, "stat", stat );
         if ( stat != NULL ) {
            temp->aoffset = ss_offsetFromType( ss_typeFromName( stat ) );
            free( stat );
         }

         continue;
      }
      xmlr_float( node, "knockback", temp->knock );

      WARN( _( "Unknown node of type '%s' in damage node '%s'." ), node->name,
            temp->name );
   } while ( xml_nextNode( node ) );

#define MELEMENT( o, s )                                                       \
   if ( o )                                                                    \
   WARN( _( "DTYPE '%s' invalid '"s                                            \
            "' element" ),                                                     \
         temp->name ) /**< Define to help check for data errors. */
   MELEMENT( temp->sdam < 0., "shield" );
   MELEMENT( temp->adam < 0., "armour" );
   MELEMENT( temp->knock < 0., "knockback" );
#undef MELEMENT

   /* Clean up. */
   xmlFreeDoc( doc );

   return 0;
}

/**
 * @brief Frees a DTYPE.
 *
 *    @param damtype DTYPE to free.
 */
static void DTYPE_free( DTYPE *damtype )
{
   free( damtype->name );
   damtype->name = NULL;
   free( damtype->display );
   damtype->display = NULL;
}

/**
 * @brief Gets the id of a dtype based on name.
 *
 *    @param name Name to match.
 *    @return ID of the damage type or -1 on error.
 */
int dtype_get( const char *name )
{
   const DTYPE  d    = { .name = (char *)name };
   const DTYPE *dout = bsearch( &d, dtype_types, array_size( dtype_types ),
                                sizeof( DTYPE ), dtype_cmp );
   if ( dout == NULL ) {
      WARN( _( "Damage type '%s' not found in stack." ), name );
      return -1;
   }
   return dout - dtype_types;
}

/**
 * @brief Gets the damage type.
 */
static DTYPE *dtype_validType( int type )
{
   if ( ( type < 0 ) || ( type >= array_size( dtype_types ) ) ) {
      WARN( _( "Damage type '%d' is invalid." ), type );
      return NULL;
   }
   return &dtype_types[type];
}

/**
 * @brief Gets the human readable string from damage type.
 */
const char *dtype_damageTypeToStr( int type )
{
   DTYPE *dmg = dtype_validType( type );
   if ( dmg == NULL )
      return NULL;
   return ( dmg->display == NULL ) ? dmg->name : dmg->display;
}

/**
 * @brief Loads the dtype stack.
 *
 *    @return 0 on success.
 */
int dtype_load( void )
{
   const DTYPE dtype_raw_type = {
      .name    = strdup( N_( "raw" ) ),
      .display = NULL,
      .sdam    = 1.,
      .adam    = 1.,
      .knock   = 0.,
      .soffset = 0,
      .aoffset = 0,
   };
   char **dtype_files = ndata_listRecursive( DTYPE_DATA_PATH );

   /* Load up the individual damage types. */
   dtype_types = array_create( DTYPE );
   array_push_back( &dtype_types, dtype_raw_type );

   for ( int i = 0; i < array_size( dtype_files ); i++ ) {
      DTYPE dtype;
      int   ret = DTYPE_parse( &dtype, dtype_files[i] );
      if ( ret == 0 )
         array_push_back( &dtype_types, dtype );
      free( dtype_files[i] );
   }
   array_free( dtype_files );

   /* Shrink back to minimum - shouldn't change ever. */
   qsort( dtype_types, array_size( dtype_types ), sizeof( DTYPE ), dtype_cmp );
   array_shrink( &dtype_types );

   return 0;
}

/**
 * @brief Frees the dtype stack.
 */
void dtype_free( void )
{
   /* clear the damtypes */
   for ( int i = 0; i < array_size( dtype_types ); i++ )
      DTYPE_free( &dtype_types[i] );
   array_free( dtype_types );
   dtype_types = NULL;
}

/**
 * @brief Gets the raw modulation stats of a damage type.
 *
 *    @param type Type to get stats of.
 *    @param[out] shield Shield damage modulator.
 *    @param[out] armour Armour damage modulator.
 *    @param[out] knockback Knockback modulator.
 *    @return 0 on success.
 */
int dtype_raw( int type, double *shield, double *armour, double *knockback )
{
   const DTYPE *dtype = dtype_validType( type );
   if ( dtype == NULL )
      return -1;
   if ( shield != NULL )
      *shield = dtype->sdam;
   if ( armour != NULL )
      *armour = dtype->adam;
   if ( knockback != NULL )
      *knockback = dtype->knock;
   return 0;
}

/**
 * @brief Gives the real shield damage, armour damage and knockback modifier.
 *
 *    @param[out] dshield Real shield damage.
 *    @param[out] darmour Real armour damage.
 *    @param[out] knockback Knockback modifier.
 *    @param[in] absorb Absorption value.
 *    @param[in] dmg Damage information.
 *    @param[in] s Ship stats to use.
 */
void dtype_calcDamage( double *dshield, double *darmour, double absorb,
                       double *knockback, const Damage *dmg,
                       const ShipStats *s )
{
   DTYPE *dtype;
   char  *ptr;
   double multiplier;

   /* Must be valid. */
   dtype = dtype_validType( dmg->type );
   if ( dtype == NULL )
      return;

   /* Set if non-nil. */
   if ( dshield != NULL ) {
      if ( ( dtype->soffset == 0 ) || ( s == NULL ) )
         *dshield = dtype->sdam * dmg->damage * absorb;
      else {
         /*
          * If an offset has been specified, look for a double at that offset
          * in the ShipStats struct, and used it as a multiplier.
          *
          * The 1. - n logic serves to convert the value from absorption to
          * damage multiplier.
          */
         ptr = (char *)s;
         memcpy( &multiplier, &ptr[dtype->soffset], sizeof( double ) );
         multiplier = MAX( 0., 1. - multiplier );
         *dshield   = dtype->sdam * dmg->damage * absorb * multiplier;
      }
   }
   if ( darmour != NULL ) {
      if ( ( dtype->aoffset ) == 0 || ( s == NULL ) )
         *darmour = dtype->adam * dmg->damage * absorb;
      else {
         ptr = (char *)s;
         memcpy( &multiplier, &ptr[dtype->aoffset], sizeof( double ) );
         multiplier = MAX( 0., 1. - multiplier );
         *darmour   = dtype->adam * dmg->damage * absorb * multiplier;
      }
   }

   if ( knockback != NULL )
      *knockback = dtype->knock;
}
