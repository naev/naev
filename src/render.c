/*
 * See Licensing and Copyright notice in naev.h
 */


#include "render.h"

#include "array.h"
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

#include "nlua_shader.h"


/**
 * @brief Post-Processing Shader.
 *
 * It is a sort of minimal version of a LuaShader_t.
 */
typedef struct PPShader_s {
   unsigned int id; /*< Global id (greater than 0). */
   int priority; /**< Used when sorting, lower is more important. */
   GLuint program; /**< Main shader program. */
   /* Shared uniforms. */
   GLint ClipSpaceFromLocal;
   GLint love_ScreenSize;
   /* Fragment Shader. */
   GLint MainTex;
   /* Vertex shader. */
   GLint VertexPosition;
   /* Textures. */
   LuaTexture_t *tex;
} PPShader;


static unsigned int pp_shaders_id = 0;
static PPShader *pp_shaders = NULL; /**< Post-processing shaders. */


/**
 * @brief Renders an FBO.
 */
static void render_fbo( GLuint fbo, GLuint tex, PPShader *shader )
{
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   glUseProgram( shader->program );
   glBindTexture( GL_TEXTURE_2D, tex );

   /* Set up stuff .*/
   glEnableVertexAttribArray( shader->VertexPosition );
   gl_vboActivateAttribOffset( gl_squareVBO, shader->VertexPosition, 0, 2, GL_FLOAT, 0 );

   /* Set the texture(s). */
   glBindTexture( GL_TEXTURE_2D, tex );
   glUniform1i( shader->MainTex, 0 );
   for (int i=0; i<array_size(shader->tex); i++) {
      LuaTexture_t *t = &shader->tex[i];
      glActiveTexture( t->active );
      glBindTexture( GL_TEXTURE_2D, t->texid );
      glUniform1i( t->uniform, t->value );
   }
   glActiveTexture( GL_TEXTURE0 );

   /* Set shader uniforms. */
   gl_Matrix4_Uniform(shader->ClipSpaceFromLocal, gl_Matrix4_Ortho(0, 1, 0, 1, 1, -1));

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shader->VertexPosition );
}


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
   int i, postprocess, next;
   int cur = 0;

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   postprocess = (array_size(pp_shaders) > 0);

   if (postprocess) {
      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.fbo[cur]);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      gl_screen.current_fbo = gl_screen.fbo[cur];
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
      for (i=0; i<array_size(pp_shaders)-1; i++) {
         next = 1-cur;
         render_fbo( gl_screen.fbo[next], gl_screen.fbo_tex[cur], &pp_shaders[i] );
         cur = next;
      }
      /* Final render is to the screen. */
      render_fbo( 0, gl_screen.fbo_tex[cur], &pp_shaders[i] );
   }

   /* check error every loop */
   gl_checkErr();
}


/**
 * @brief Adds a new post-processing shader.
 *
 *    @param shader Shader to add.
 *    @param priority When it should be run (lower is sooner).
 *    @return The shader ID.
 */
unsigned int render_postprocessAdd( LuaShader_t *shader, int priority )
{
   PPShader *pp;

   if (pp_shaders==NULL)
      pp_shaders = array_create( PPShader );
   pp = &array_grow( &pp_shaders );
   pp->id               = ++pp_shaders_id;
   pp->priority         = priority;
   pp->program          = shader->program;
   pp->ClipSpaceFromLocal = shader->ClipSpaceFromLocal;
   pp->love_ScreenSize  = shader->love_ScreenSize;
   pp->MainTex          = shader->MainTex;
   pp->VertexPosition   = shader->VertexPosition;
   if (shader->tex != NULL)
      pp->tex = array_copy( LuaTexture_t, shader->tex );
   else
      pp->tex = NULL;

   return pp->id;
}


/**
 * @brief Removes a post-process shader by ID.
 *
 *    @param id ID of shader to remove.
 *    @return 0 on success.
 */
int render_postprocessRm( unsigned int id )
{
   int i, found;
   PPShader *pp;
   found = 0;
   for (i=0; i<array_size(pp_shaders); i++) {
      pp = &pp_shaders[i];
      if (pp->id != id)
         continue;
      found = i;
      break;
   }
   if (found==0) {
      WARN(_("Trying to remove non-existant post-processing shader with id '%d'!"), id);
      return -1;
   }

   array_erase( &pp_shaders, &pp_shaders[found], &pp_shaders[found+1] );
   return 0;
}


/**
 * @brief Cleans up the post-processing stuff.
 */
void render_exit (void)
{
   array_free( pp_shaders );
   pp_shaders = NULL;
}

