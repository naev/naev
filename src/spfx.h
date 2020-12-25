/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPFX_H
#  define SPFX_H


#include "opengl.h"
#include "physics.h"


#define SPFX_LAYER_FRONT   0 /**< Front spfx layer. */
#define SPFX_LAYER_BACK    1 /**< Back spfx layer. */

#define SHAKE_DECAY        0.3 /**< Rumble decay parameter */
#define SHAKE_MAX          1.0 /**< Rumblemax parameter */

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
void spfx_begin( const double dt, const double real_dt );
void spfx_end (void);
void spfx_shake( double mod );
void spfx_getShake( double *x, double *y );


/*
 * other effects
 */
void spfx_cinematic (void);


/*
 * spfx effect loading and freeing
 */
int spfx_load (void);
void spfx_free (void);


#endif /* SPFX_H */
