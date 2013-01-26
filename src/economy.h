/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ECONOMY_H
#  define ECONOMY_H


#include <stdint.h>
// #include "space.c"

#define ECON_CRED_STRLEN      32 /**< Maximum length a credits2str string can reach. */

#define PRICE(Credits,Goods)  ((Credits) / (Goods)) /**< Price of a good*/


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
   double price; /**< Base price of the commodity. */
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
int economy_init (void);
void economy_update( unsigned int dt );
void economy_destroy (void);
void refresh_prices(void);    /* if something affecting prices were to change */
double production(double mod, double goods);
credits_t price_of_buying(int n_tons, double p_creds, double p_goods);


/*
 * Misc stuff.
 */
void credits2str( char *str, credits_t credits, int decimals );
void commodity_Jettison( int pilot, Commodity* com, int quantity );
int commodity_compareTech( const void *commodity1, const void *commodity2 );


#endif /* ECONOMY_H */
