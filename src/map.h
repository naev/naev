/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space.h"
#include "outfit.h"
#include "opengl_tex.h"

#define MAP_WDWNAME     "wdwStarMap" /**< Map window name. */

/**
 * @brief Images to be shown on the map.
 */
typedef struct MapDecorator_ {
   glTexture* image;    /**< Map decorator image. */
   double x,y;          /**< Position on the map. */
   int detection_radius;/**< Distance at which it is detected. */
} MapDecorator;

/**
 * @brief Different map modes available to the player.
 */
typedef enum MapMode_ {
   MAPMODE_TRAVEL,   /**< Standard mode with lots of shiny stuff. */
   MAPMODE_DISCOVER, /**< Indicates whether or not systems are fully discovered. */
   MAPMODE_TRADE,    /**< Shows price values and the likes. */
   MAPMODE_EDITOR,   /**< Shows price values and the likes. */
} MapMode;

/* init/exit */
int map_init (void);
void map_exit (void);

/* open the map window */
void map_open (void);
void map_close (void);
int map_isOpen (void);

/* misc */
StarSystem* map_getDestination( int *jumps );
void map_setZoom( unsigned int wid, double zoom );
void map_select( const StarSystem *sys, char shifted );
void map_cycleMissions(int dir);
void map_toggleNotes (void);
void map_cleanup (void);
void map_clear (void);
void map_jump (void);

/* manipulate universe stuff */
StarSystem **map_getJumpPath( const char *sysstart, const vec2 *posstart, const char *sysend,
      int ignore_known, int show_hidden, StarSystem **old_data, double *o_distance );
int map_map( const Outfit *map );
int map_isUseless( const Outfit* map );

/* Local map stuff. */
int localmap_map( const Outfit *lmap );
int localmap_isUseless( const Outfit *lmap );

/* shows a map at x, y (relative to wid) with size w,h  */
void map_show( int wid, int x, int y, int w, int h, double zoom, double xoff, double yoff );
int map_center( int wid, const char *sys );

/* Internal rendering sort of stuff. */
void map_renderParams( double bx, double by, double xpos, double ypos,
      double w, double h, double zoom, double *x, double *y, double *r );
void map_renderFactionDisks( double x, double y, double zoom, double r, int editor, double alpha );
void map_renderSystemEnvironment( double x, double y, double zoom, int editor, double alpha );
void map_renderDecorators( double x, double y, double zoom, int editor, double alpha );
void map_renderJumps( double x, double y, double zoom, double radius, int editor );
void map_renderSystems( double bx, double by, double x, double y,
      double zoom, double w, double h, double r, MapMode mode );
void map_renderNotes( double bx, double by, double x, double y,
      double zoom, double w, double h, int editor, double alpha );
void map_renderNames( double bx, double by, double x, double y,
      double zoom, double w, double h, int editor, double alpha );
void map_updateFactionPresence( const unsigned int wid, const char *name, const StarSystem *sys, int omniscient );
int map_load (void);
