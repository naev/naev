/*
 * commodity_exchange.h
 *
 * Declarations for the Commodity Exchange helper functions.
 * These manage the UI or console flow for buying/selling,
 * as well as price modifiers, and relevant state cleanup.
 *
 * For usage details, see commodity_exchange.c
 */

#pragma once

#include "commodity.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opens the commodity exchange interface for a given window.
 *
 *    @param wid  The window or UI identifier where the exchange is displayed.
 */
void commodity_exchange_open( unsigned int wid );

/**
 * @brief Cleans up any resources used by the commodity exchange.
 *        Should be called when the exchange closes.
 */
void commodity_exchange_cleanup( void );

/**
 * @brief Refreshes or updates the commodity exchange UI.
 *
 *    @param wid   The window identifier.
 *    @param str   A string specifying update context (e.g. "tick", "refresh").
 */
void commodity_update( unsigned int wid, const char *str );

/**
 * @brief Attempts to buy a commodity from the exchange.
 *
 *    @param wid   The window identifier.
 *    @param str   Possibly the commodity name or code to buy.
 */
void commodity_buy( unsigned int wid, const char *str );

/**
 * @brief Attempts to sell a commodity to the exchange.
 *
 *    @param wid   The window identifier.
 *    @param str   Possibly the commodity name or code to sell.
 */
void commodity_sell( unsigned int wid, const char *str );

/**
 * @brief Determines if the player can buy a certain commodity with a given price modifier.
 *
 *    @param com        The commodity.
 *    @param price_mod  A multiplier that modifies the price.
 *    @return 1 if can buy, 0 if cannot.
 */
int commodity_canBuy( const Commodity *com, double price_mod );

/**
 * @brief Determines if the player can sell a certain commodity.
 *
 *    @param com   The commodity to check.
 *    @return 1 if can sell, 0 otherwise.
 */
int commodity_canSell( const Commodity *com );

/**
 * @brief Retrieves the current commodity price modifier in the station or environment.
 *
 *    @return An integer representing the mod (for demonstration).
 */
int commodity_getMod( void );

/**
 * @brief Renders the commodity price modifier or info onto the UI at the given coords.
 *
 *    @param bx   The base x-coordinate.
 *    @param by   The base y-coordinate.
 *    @param w    The width of the render area.
 *    @param h    The height of the render area.
 *    @param data A pointer to user-supplied data (can be NULL).
 */
void commodity_renderMod( double bx, double by, double w, double h, void *data );

#ifdef __cplusplus
}
#endif

