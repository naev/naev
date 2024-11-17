/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdint.h>
/** @endcond */

#include "opengl_tex.h"

#define ECON_CRED_STRLEN                                                       \
   32 /**< Maximum length a credits2str/price2str string can reach. */
#define ECON_MASS_STRLEN                                                       \
   32 /**< Maximum length a tonnes2str string can reach. */

typedef int64_t credits_t;
#define CREDITS_MAX                                                            \
   ( ( (credits_t)1 )                                                          \
     << 53 ) /**< Maximum credits_t value that round-trips through Lua. */
#define CREDITS_MIN                                                            \
   ( -CREDITS_MAX ) /**< Minimum credits_t value that round-trips through Lua. \
                     */
#define CREDITS_PRI PRIu64

#define COMMODITY_FLAG_STANDARD                                                \
   ( 1 << 0 ) /**< Commodity is considered a standard commodity available      \
                 everywhere. */
#define COMMODITY_FLAG_ALWAYS_CAN_SELL                                         \
   ( 1 << 1 ) /**< Commodity is always sellable, even when not available       \
                 normally at a spob. */
#define COMMODITY_FLAG_PRICE_CONSTANT                                          \
   ( 1 << 2 ) /**< Commodity price is always constant. */
#define commodity_isFlag( c, f )                                               \
   ( ( c )->flags & ( f ) ) /**< Checks commodity flag. */
#define commodity_setFlag( c, f )                                              \
   ( ( c )->flags |= ( f ) ) /**< Sets a commodity flag. */
#define commodity_rmFlag( c, f )                                               \
   ( ( c )->flags &= ~( f ) ) /**< Removes a commodity flag. */

/**
 * @struct CommodityModifier
 *
 * @brief Represents a dictionary of values used to modify a commodity.
 */
typedef struct CommodityModifier_ {
   char                      *name;
   float                      value;
   struct CommodityModifier_ *next;
} CommodityModifier;

/**
 * @struct Commodity
 *
 * @brief Represents a commodity.
 */
typedef struct Commodity_ {
   char        *name;        /**< Name of the commodity. */
   char        *description; /**< Description of the commodity. */
   unsigned int flags;       /**< Commodity flags. */

   /* Prices. */
   char *price_ref; /**< Name of the commodity to which this one is referenced
                       to. */
   double     price_mod; /**< Modifier on price_ref. */
   double     raw_price; /**< Raw price of the commodity. */
   double     price;     /**< Base price of the commodity. */
   glTexture *gfx_store; /**< Store graphic. */
   glTexture *gfx_space; /**< Space graphic. */

   /* Misc stuff. */
   credits_t
      lastPurchasePrice; /**< Price paid when last purchasing this commodity. */
   int  istemp;          /**< This commodity is temporary. */
   int *illegalto;       /**< Factions this commodity is illegal to. */

   /* Dynamic economy stuff. */
   CommodityModifier
         *spob_modifier; /**< The price modifier for different spob types. */
   double period;        /**< Period of price fluctuation. */
   double population_modifier; /**< Scale of price modification due to
                                  population. */
   CommodityModifier
      *faction_modifier; /**< Price modifier for different factions. */
} Commodity;

typedef struct CommodityPrice_ {
   double price;      /**< Average price of a commodity on a particular spob */
   double spobPeriod; /**< Minor time period (days) over which commidity price
                         varies */
   double sysPeriod;  /** Major time period */
   double spobVariation; /**< Mmount by which a commodity price varies */
   double sysVariation; /**< System level commodity price variation.  At a given
                           time, commodity price is equal to price +
                           sysVariation*sin(2pi t/sysPeriod) +
                           spobVariation*sin(2pi t/spobPeriod) */
   int64_t updateTime; /**< used for averaging and to hold the time last average
                          was calculated. */
   char *name;         /**< used for keeping tabs during averaging */
   double
      sum; /**< used when averaging over jump points during setup, and then for
              capturing the moving average when the player visits a spob. */
   double sum2; /**< sum of (squared prices seen), used for calc of standard
                   deviation. */
   int cnt;     /**< used for calc of mean and standard deviation - number of
                   records in the data. */
} CommodityPrice;

/*
 * Commodity stuff.
 */
Commodity *commodity_getAll( void );
Commodity *commodity_get( const char *name );
Commodity *commodity_getW( const char *name );
int        commodity_getN( void );

Commodity *commodity_getByIndex( const int indx );
int        commodity_load( void );
void       commodity_free( void );

int commodity_checkIllegal( const Commodity *com, int faction );

/*
 * Temporary commodities.
 */
int        commodity_isTemp( const char *name );
Commodity *commodity_newTemp( const char *name, const char *desc );
int        commodity_tempIllegalto( Commodity *com, int faction );

/*
 * Misc stuff.
 */
void credits2str( char *str, credits_t credits, int decimals );
void price2str( char *str, credits_t price, credits_t credits, int decimals );
void tonnes2str( char *str, int tonnes );
int  commodity_compareTech( const void *commodity1, const void *commodity2 );
Commodity **standard_commodities( void );
