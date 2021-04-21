/*
 * See Licensing and Copyright notice in naev.h
 */


#include "render.h"

#include "array.h"
#include "font.h"
#include "gui.h"
#include "hook.h"
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
   double dt; /**< Used when computing u_time. */
   GLuint program; /**< Main shader program. */
   /* Shared uniforms. */
   GLint ClipSpaceFromLocal;
   GLint u_time; /**< Special uniform. */
   /* Fragment Shader. */
   GLint MainTex;
   /* Vertex shader. */
   GLint VertexPosition;
   GLint VertexTexCoord;
   /* Textures. */
   LuaTexture_t *tex;
} PPShader;


static unsigned int pp_shaders_id = 0;
static PPShader *pp_shaders_list[PP_LAYER_MAX] = {NULL, NULL}; /**< Post-processing shaders for game layer. */


/**
 * @brief Renders an FBO.
 */
static void render_fbo( double dt, GLuint fbo, GLuint tex, PPShader *shader )
{
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   glUseProgram( shader->program );

   /* Time stuff. */
   if (shader->u_time >= 0) {
      shader->dt += dt;
      glUniform1f( shader->u_time, shader->dt );
   }

   /* Set up stuff .*/
   glEnableVertexAttribArray( shader->VertexPosition );
   gl_vboActivateAttribOffset( gl_squareVBO, shader->VertexPosition, 0, 2, GL_FLOAT, 0 );
   if (shader->VertexTexCoord >= 0) {
      glEnableVertexAttribArray( shader->VertexTexCoord );
      gl_vboActivateAttribOffset( gl_squareVBO, shader->VertexTexCoord, 0, 2, GL_FLOAT, 0 );
   }

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
   gl_Matrix4_Uniform(shader->ClipSpaceFromLocal, gl_Matrix4_Ortho(0, 1, 1, 0, 1, -1));

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shader->VertexPosition );
   if (shader->VertexTexCoord >= 0)
      glDisableVertexAttribArray( shader->VertexTexCoord );
   glUseProgram( 0 );
}


/**
 * @brief Renders a list of FBOs.
 */
static void render_fbo_list( double dt, PPShader *list, int *current, int done )
{
   PPShader *pp;
   int i, cur, next;
   cur = *current;

   for (i=0; i<array_size(list)-1; i++) {
      pp = &list[i];
      next = 1-cur;
      render_fbo( dt, gl_screen.fbo[next], gl_screen.fbo_tex[cur], pp );
      cur = next;
   }
   /* Final render is to the screen. */
   pp = &list[i];
   gl_screen.current_fbo = (done) ? 0 : gl_screen.fbo[cur];
   render_fbo( dt, gl_screen.current_fbo, gl_screen.fbo_tex[cur], pp );
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);

   *current = cur;
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
   int pp_final, pp_game;
   int cur = 0;

   /* See what post-processing is up. */
   pp_game  = (array_size(pp_shaders_list[PP_LAYER_GAME]) > 0);
   pp_final = (array_size(pp_shaders_list[PP_LAYER_FINAL]) > 0);

   if (pp_game || pp_final)
      gl_screen.current_fbo = gl_screen.fbo[cur];
   else
      gl_screen.current_fbo = 0;
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   dt = (paused) ? 0. : game_dt;

   /* Background stuff */
   space_render( real_dt ); /* Nebula looks really weird otherwise. */
   hooks_run( "renderbg" );
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
   hooks_run( "renderfg" );

   /* Process game stuff only. */
   if (pp_game)
      render_fbo_list( dt, pp_shaders_list[PP_LAYER_GAME], &cur, !pp_final );

   /* GUi stuff. */
   gui_render(dt);

   /* Top stuff. */
   ovr_render(dt);
   display_fps( real_dt ); /* Exception using real_dt. */
   toolkit_render();

   /* Final post-processing. */
   if (pp_final)
      render_fbo_list( dt, pp_shaders_list[PP_LAYER_FINAL], &cur, 1 );

   /* check error every loop */
   gl_checkErr();
}


/**
 * @brief Sorts shaders by priority.
 */
static int ppshader_compare( const void *a, const void *b )
{
   PPShader *ppa, *ppb;
   ppa = (PPShader*) a;
   ppb = (PPShader*) b;
   if (ppa->priority > ppb->priority)
      return +1;
   if (ppa->priority < ppb->priority)
      return -1;
   return 0;
}


/**
 * @brief Adds a new post-processing shader.
 *
 *    @param shader Shader to add.
 *    @param priority When it should be run (lower is sooner).
 *    @return The shader ID.
 */
unsigned int render_postprocessAdd( LuaShader_t *shader, int layer, int priority )
{
   PPShader *pp, **pp_shaders;

   /* Select the layer. */
   if (layer < 0 || layer >= PP_LAYER_MAX) {
      WARN(_("Unknown post-processing shader layer '%d'!"), layer);
      return 0;
   }
   pp_shaders = &pp_shaders_list[layer];

   if (*pp_shaders==NULL)
      *pp_shaders = array_create( PPShader );
   pp = &array_grow( pp_shaders );
   pp->id               = ++pp_shaders_id;
   pp->priority         = priority;
   pp->program          = shader->program;
   pp->ClipSpaceFromLocal = shader->ClipSpaceFromLocal;
   pp->MainTex          = shader->MainTex;
   pp->VertexPosition   = shader->VertexPosition;
   pp->VertexTexCoord   = shader->VertexTexCoord;
   if (shader->tex != NULL)
      pp->tex = array_copy( LuaTexture_t, shader->tex );
   else
      pp->tex = NULL;
   /* Special uniforms. */
   pp->u_time = glGetUniformLocation( pp->program, "u_time" );
   pp->dt = 0.;

   /* Resort n case stuff is weird. */
   qsort( *pp_shaders, array_size(*pp_shaders), sizeof(PPShader), ppshader_compare );

   gl_checkErr();

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
   int i, j, found;
   PPShader *pp, *pp_shaders;

   found = -1;
   for (j=0; j<PP_LAYER_MAX; j++) {
      pp_shaders = pp_shaders_list[j];
      for (i=0; i<array_size(pp_shaders); i++) {
         pp = &pp_shaders[i];
         if (pp->id != id)
            continue;
         found = i;
         break;
      }
      if (found>=0)
         break;
   }
   if (found==-1) {
      WARN(_("Trying to remove non-existant post-processing shader with id '%d'!"), id);
      return -1;
   }

   /* No need to resort. */
   array_erase( &pp_shaders_list[j], &pp_shaders_list[j][found], &pp_shaders_list[j][found+1] );
   return 0;
}


/**
 * @brief Cleans up the post-processing stuff.
 */
void render_exit (void)
{
   int i;
   for (i=0; i<PP_LAYER_MAX; i++) {
      array_free( pp_shaders_list[i] );
      pp_shaders_list[i] = NULL;
   }
}

