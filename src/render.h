/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef RENDER_H
#  define RENDER_H


#include "nlua_shader.h"


#define PP_LAYER_GAME      0
#define PP_LAYER_FINAL     1
#define PP_LAYER_MAX       2 /**< Sentinal. */


void fps_setPos( double x, double y );
void render_all( double game_dt, double real_dt );
void render_exit (void);

unsigned int render_postprocessAdd( LuaShader_t *shader, int layer, int priority );
int render_postprocessRm( unsigned int id );


#endif /* RENDER_H */

