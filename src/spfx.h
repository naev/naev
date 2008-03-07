/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPFX_H
#  define SPFX_H


#include "physics.h"


#define SPFX_LAYER_FRONT   0
#define SPFX_LAYER_BACK    1

#define SHAKE_DECAY  50. /* decay parameter */
#define SHAKE_MAX    50. /* max parameter */


/*
 * stack manipulation
 */
int spfx_get( char* name );
void spfx_add( const int effect,
      const double px, const double py,
      const double vx, const double vy,
      const int layer );


/*
 * stack mass manipulation functions
 */
void spfx_update( const double dt );
void spfx_render( const int layer );
void spfx_clear (void);


/*
 * get ready to rumble
 */
void spfx_start( double dt );
void spfx_shake( double mod );


/*
 * spfx effect loading and freeing
 */
int spfx_load (void);
void spfx_free (void);


/*
 * pause/unpause routines
 */
void spfx_pause (void);
void spfx_unpause (void);
void spfx_delay( unsigned int delay );



#endif /* SPFX_H */
