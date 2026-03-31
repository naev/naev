/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file commodity.c
 *
 * @brief Handles commodities.
 */
/** @cond */
#include <stdio.h>
/** @endcond */

#include "commodity.h"

#define CRED_TEXT_MAX                                                          \
   ( ECON_CRED_STRLEN -                                                        \
     4 ) /* Maximum length of just credits2str text, no markup */

/**
 * @brief Converts credits to a usable string for displaying.
 *
 *    @param[out] str Output is stored here, must have at least a size of
 * ECON_CRED_STRLEN.
 *    @param credits Credits to display, negative value to display full string.
 *    @param decimals Decimals to use.
 */
void credits2str( char *str, credits_t credits, int decimals )
{
   if ( decimals < 0 ) {
      /* TODO support , separator like fmt.credits(). */
      snprintf( str, CRED_TEXT_MAX, _( "%.*f ¤" ), 0, (double)credits );
   } else if ( credits >= 1000000000000000000LL )
      snprintf( str, CRED_TEXT_MAX, _( "%.*f E¤" ), decimals,
                (double)credits / 1e18 );
   else if ( credits >= 1000000000000000LL )
      snprintf( str, CRED_TEXT_MAX, _( "%.*f P¤" ), decimals,
                (double)credits / 1e15 );
   else if ( credits >= 1000000000000LL )
      snprintf( str, CRED_TEXT_MAX, _( "%.*f T¤" ), decimals,
                (double)credits / 1e12 );
   else if ( credits >= 1000000000L )
      snprintf( str, CRED_TEXT_MAX, _( "%.*f G¤" ), decimals,
                (double)credits / 1e9 );
   else if ( credits >= 1000000 )
      snprintf( str, CRED_TEXT_MAX, _( "%.*f M¤" ), decimals,
                (double)credits / 1e6 );
   else if ( credits >= 1000 )
      snprintf( str, CRED_TEXT_MAX, _( "%.*f k¤" ), decimals,
                (double)credits / 1e3 );
   else
      snprintf( str, CRED_TEXT_MAX, _( "%.*f ¤" ), decimals, (double)credits );
}

/**
 * @brief Given a price and on-hand credits, outputs a colourized string.
 *
 *    @param[out] str Output is stored here, must have at least a size of
 * ECON_CRED_STRLEN.
 *    @param price Price to display.
 *    @param credits Credits available.
 *    @param decimals Decimals to use.
 */
void price2str( char *str, credits_t price, credits_t credits, int decimals )
{
   char buf[CRED_TEXT_MAX];

   if ( price <= credits ) {
      credits2str( str, price, decimals );
      return;
   }

   credits2str( buf, price, decimals );
   snprintf( str, ECON_CRED_STRLEN, "#r%s#0", (char *)buf );
}

/**
 * @brief Converts tonnes to a usable string for displaying.
 *
 *    @param[out] str Output is stored here, must have at least a size of
 * ECON_MASS_STRLEN.
 *    @param tonnes Number of tonnes to display.
 */
void tonnes2str( char *str, int tonnes )
{
   snprintf( str, ECON_MASS_STRLEN, n_( "%d tonne", "%d tonnes", tonnes ),
             tonnes );
}

/**
 * @brief Function meant for use with C89, C99 algorithm qsort().
 *
 *    @param commodity1 First argument to compare.
 *    @param commodity2 Second argument to compare.
 *    @return -1 if first argument is inferior, +1 if it's superior, 0 if ties.
 */
int commodity_compareTech( const void *commodity1, const void *commodity2 )
{
   CommodityRef c1, c2;

   /* Get commodities. */
   c1 = *(CommodityRef *)commodity1;
   c2 = *(CommodityRef *)commodity2;

   /* Compare price. */
   credits_t c1p = commodity_price( c1 );
   credits_t c2p = commodity_price( c2 );
   if ( c1p < c2p )
      return +1;
   else if ( c1p > c2p )
      return -1;

   /* It turns out they're the same. */
   return strcmp( commodity_name( c1 ), commodity_name( c2 ) );
}
