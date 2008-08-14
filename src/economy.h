/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ECONOMY_H
#  define ECONOMY_H


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
   int low; /**< Lowest price on the market. */
   int medium; /**< Average price on the market. */
   int high; /**< Highest price on the market. */
} Commodity;

/* commodity stuff */
Commodity* commodity_get( const char* name );
int commodity_load (void);
void commodity_free (void);

/* misc stuff */
void credits2str( char *str, unsigned int credits, int decimals );
void commodity_Jettison( int pilot, Commodity* com, int quantity );


#endif /* ECONOMY_H */
