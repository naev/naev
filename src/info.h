/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef INFO_H
#  define INFO_H


#define INFO_MAIN       0 /**< Main info window. */
#define INFO_SHIP       1 /**< Ship info window. */
#define INFO_WEAPONS    2 /**< Weapons info window. */
#define INFO_CARGO      3 /**< Cargo info window. */
#define INFO_MISSIONS   4 /**< Missions info window. */
#define INFO_STANDINGS  5 /**< Standings info window. */
#define INFO_SHIPLOG    6 /**< Ship log info window. */

/*
 * Menu opening routines.
 */
void menu_info( int window );
void info_update (void);


#endif /* INFO_H */
