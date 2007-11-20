/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SPFX_H
#  define SPFX_H


#include "physics.h"


#define SPFX_LAYER_FRONT	0
#define SPFX_LAYER_BACK		1


/*
 * stack manipulation
 */
int spfx_get( char* name );
void spfx_add( const int effect,
		const Vector2d *pos, const Vector2d *vel,
		const int layer );


/*
 * stack mass manipulation functions
 */
void spfx_update( const double dt );
void spfx_render( const int layer );
void spfx_clear (void);



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



#endif /* SPFX_H */
