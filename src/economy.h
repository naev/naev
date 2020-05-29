/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ECONOMY_H
#  define ECONOMY_H


#include <stdint.h>
#include "opengl.h"


#define ECON_CRED_STRLEN      32 /**< Maximum length a credits2str string can reach. */


typedef int64_t credits_t;
#define CREDITS_MAX        INT64_MAX
#define CREDITS_MIN        INT64_MIN
#define CREDITS_PRI        PRIu64

/**
 * @struct Commodity
 *
 * @brief Represents a commodity.
 *
 * @todo Use inverse normal?
 */
typedef struct Commodity_ {
   char* name; /**< Name of the commodity. */
   char* description; /**< Description of the commodity. */
   /* Prices. */
   double price; /**< Base price of the commodity. */
   glTexture* gfx_store; /**< Store graphic. */
   glTexture* gfx_space; /**< Space graphic. */
} Commodity;

typedef struct CommodityPrice_ {
  double price; /**< Average price of a commodity on a particular planet */
  double planetPeriod; /**< Minor time period (days) over which commidity price varies */
  double sysPeriod; /** Major time period */
  double planetVariation; /**< Mmount by which a commodity price varies */
  double sysVariation; /**< System level commodity price variation.  At a given time, commodity price is equal to price + sysVariation*sin(2pi t/sysPeriod) + planetVariation*sin(2pi t/planetPeriod) */
  int cnt; /**< used for averaging */
  char *name; /**< used for keeping tabs during averaging */
  double temp; /**< used when averaging over jump points during setup */
} CommodityPrice;

/**
 * @struct Gatherable
 *
 * @brief Represents stuff that can be gathered.
 */
typedef struct Gatherable_ {
   Commodity *type; /**< Type of commodity. */
   Vector2d pos; /**< Position. */
   Vector2d vel; /**< Velocity. */
   double timer; /**< Timer to de-spawn the gatherable. */
   double lifeleng; /**< nb of seconds before de-spawn. */
} Gatherable;


/*
 * Commodity stuff.
 */
Commodity* commodity_get( const char* name );
Commodity* commodity_getW( const char* name );
int commodity_load (void);
void commodity_free (void);


/*
 * Economy stuff.
 */
int economy_init (void);
void economy_addQueuedUpdate (void);
int economy_execQueued (void);
int economy_update( unsigned int dt );
int economy_refresh (void);
void economy_destroy (void);


/*
 * Gatherable objects
 */
void gatherable_init( Commodity* com, Vector2d pos, Vector2d vel );
void gatherable_render( void );
int gatherable_getClosest( Vector2d pos, double rad );
int gatherable_getPos( Vector2d* pos, Vector2d* vel, int id );
void gatherable_free( void );
void gatherable_update( double dt );
void gatherable_gather( int pilot );


/*
 * Misc stuff.
 */
void credits2str( char *str, credits_t credits, int decimals );
void price2str( char *str, credits_t price, credits_t credits, int decimals );
void commodity_Jettison( int pilot, Commodity* com, int quantity );
int commodity_compareTech( const void *commodity1, const void *commodity2 );

/*
 * Calculating the sinusoidal economy values
 */
/*int economy_calcPriceClass(char *class,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcImg(char *gfx_spaceName,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcSurface(char *gfx_exterior,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcPopulation(uint64_t population,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcFaction(char *faction,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcRange(int presenceRange,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcSysRadius(double radius,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcSysVolatility(double nebu_volatility,double interference,Commodity *commodity,CommodityPrice *commodityPrice);
int economy_calcSysJumps(int njumps,Commodity *commodity,CommodityPrice *commodityPrice);*/
void economy_initialiseCommodityPrices(void);

			      

#endif /* ECONOMY_H */
