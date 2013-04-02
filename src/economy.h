/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ECONOMY_H
#  define ECONOMY_H


#include <stdint.h>

#include "ntime.h"

#define ECON_CRED_STRLEN      32 /**< Maximum length a credits2str string can reach. */

#define PRICE(Commodity, sys)  ((Commodity)->price * (sys)->real_prices[(Commodity)->index]) /**< Price of a good */
#define DEFAULT_GLOBAL_WEIGHT 1.0 /* how much systems prefer their own given values */

typedef int64_t credits_t;
#define CREDITS_MAX        INT64_MAX
#define CREDITS_MIN        INT64_MIN
#define CREDITS_PRI        PRIu64

/**
 * @struct Commodity
 *
 * @brief Represents a commodity.
 *
 */
typedef struct Commodity_ {
   int index;	/**< Index of the commodity */
   char* name; /**< Name of the commodity. */
   char* description; /**< Description of the commodity. */
   /* Prices. */
   double price; /**< Price multiplier of the commodity. */
} Commodity;

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
int econ_refreshsolutions(void);   /* to be called when initing economy or when trade routes are updated */
void econ_updateprices(void); /* to update prices when prices are changed */
void econ_init(void);
void econ_destroy (void);  /* frees ALL economy related values. Only to clean up values when exiting program. 
      * If values are changed, use econ_refreshsolutions() and econ_updateprices() or just econ_updateprices()*/
/*
 * Misc stuff.
 */
void credits2str( char *str, credits_t credits, int decimals );
void commodity_Jettison( int pilot, Commodity* com, int quantity );
int commodity_compareTech( const void *commodity1, const void *commodity2 );


#endif /* ECONOMY_H */
