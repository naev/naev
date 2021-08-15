/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef RENDER_H
#  define RENDER_H


#include "nlua_shader.h"


enum {
   PP_LAYER_GAME, /**< Applied ontop of the in-game graphics. */
   PP_LAYER_GUI, /**< Applied ontop of the in-game GUI. */
   PP_LAYER_FINAL, /**< Applied ontop of everything! */
   PP_LAYER_MAX,
};


void fps_setPos( double x, double y );
void render_all( double game_dt, double real_dt );
void render_init (void);
void render_exit (void);

unsigned int render_postprocessAdd( LuaShader_t *shader, int layer, int priority );
int render_postprocessRm( unsigned int id );

/* Special post-processing shaders. */
void render_setGamma( double gamma );


#endif /* RENDER_H */

