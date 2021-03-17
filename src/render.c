/*
 * See Licensing and Copyright notice in naev.h
 */


#include "render.h"

#include "font.h"                                                               
#include "gui.h"                                                                
#include "map_overlay.h"
#include "naev.h"
#include "opengl.h"                                                             
#include "pause.h"
#include "player.h"                                                             
#include "space.h"                                                              
#include "spfx.h"                                                               
#include "toolkit.h"                                                            
#include "weapon.h" 


/**
 * @brief Renders the game itself (player flying around and friends).
 *
 * Blitting order (layers):
 *   - BG
 *     - stars and planets
 *     - background player stuff (planet targeting)
 *     - background particles
 *     - back layer weapons
 *   - N
 *     - NPC ships
 *     - front layer weapons
 *     - normal layer particles (above ships)
 *   - FG
 *     - player
 *     - foreground particles
 *     - text and GUI
 */
void render_all( double game_dt, double real_dt )
{
   double dt;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   int postprocess = 0;

   if (postprocess) {
      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.fbo);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      gl_screen.current_fbo = gl_screen.fbo;
   }
   else
      gl_screen.current_fbo = 0;

   dt = (paused) ? 0. : game_dt;

   /* setup */
   spfx_begin(dt, real_dt);
   /* Background stuff */
   space_render( real_dt ); /* Nebula looks really weird otherwise. */
   planets_render();
   spfx_render(SPFX_LAYER_BACK);
   weapons_render(WEAPON_LAYER_BG, dt);
   /* Middle stuff */
   pilots_render(dt);
   weapons_render(WEAPON_LAYER_FG, dt);
   spfx_render(SPFX_LAYER_MIDDLE);
   /* Foreground stuff */
   player_render(dt);
   spfx_render(SPFX_LAYER_FRONT);
   space_renderOverlay(dt);
   gui_renderReticles(dt);
   pilots_renderOverlay(dt);
   spfx_end();
   gui_render(dt);

   /* Top stuff. */
   ovr_render(dt);
   display_fps( real_dt ); /* Exception using real_dt. */
   toolkit_render();

   if (postprocess) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      glUseProgram(shaders.texture.program);
      glBindTexture( GL_TEXTURE_2D, gl_screen.fbo_tex );

      /* Set up stuff .*/
      glEnableVertexAttribArray( shaders.texture.vertex );
      gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture.vertex,
            0, 2, GL_FLOAT, 0 );

      /* Set shader uniforms. */
      gl_uniformColor(shaders.texture.color, &cWhite);
      gl_Matrix4_Uniform(shaders.texture.projection, gl_Matrix4_Ortho(0, 1, 0, 1, 1, -1));
      gl_Matrix4_Uniform(shaders.texture.tex_mat, gl_Matrix4_Identity());

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      /* Clear state. */
      glDisableVertexAttribArray( shaders.texture.vertex );
   }

   /* check error every loop */
   gl_checkErr();
}

