/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua_shader.h"

enum {
   PP_LAYER_NONE, /**< Not actually done. */
   PP_LAYER_GAME, /**< Applied ontop of the in-game graphics. */
   PP_LAYER_GUI,  /**< Applied ontop of the in-game GUI. */
   PP_LAYER_FINAL,/**< Applied ontop of everything! */
   PP_LAYER_MAX,
};

#define PP_SHADER_PERMANENT   (1<<0)   /**< Shader doesn't get removed on main menu / death. */

void fps_setPos( double x, double y );
void render_all( double game_dt, double real_dt );
void render_init (void);
void render_exit (void);

unsigned int render_postprocessAdd( LuaShader_t *shader, int layer, int priority, unsigned int flags );
int render_postprocessRm( unsigned int id );
void render_postprocessCleanup (void);

/* Special post-processing shaders. */
void render_setGamma( double gamma );
