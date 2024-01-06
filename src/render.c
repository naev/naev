/*
 * See Licensing and Copyright notice in naev.h
 */
#include "render.h"

#include "array.h"
#include "conf.h"
#include "font.h"
#include "gui.h"
#include "hook.h"
#include "map_overlay.h"
#include "naev.h"
#include "ntracing.h"
#include "menu.h"
#include "opengl.h"
#include "pause.h"
#include "player.h"
#include "nlua_canvas.h"
#include "ntracing.h"
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
   unsigned int flags; /**< Flags to use. */
   double dt; /**< Used when computing u_time. */
   GLuint program; /**< Main shader program. */
   /* Shared uniforms. */
   GLint ClipSpaceFromLocal;
   GLint u_time; /**< Special uniform. */
   /* Fragment Shader. */
   GLint MainTex;
   GLint love_ScreenSize;
   /* Vertex shader. */
   GLint VertexPosition;
   GLint VertexTexCoord;
   /* Textures. */
   LuaTexture_t *tex;
} PPShader;

static unsigned int pp_shaders_id = 0;
static PPShader *pp_shaders_list[PP_LAYER_MAX]; /**< Post-processing shaders for game layer. */

static LuaShader_t gamma_correction_shader;
static int pp_gamma_correction = 0; /**< Gamma correction shader. */

/**
 * @brief Renders an FBO.
 */
static void render_fbo( double dt, GLuint fbo, GLuint tex, PPShader *shader )
{
   /* Have to consider alpha premultiply. */
   glBlendFuncSeparate( GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   glUseProgram( shader->program );

   /* Screen size. */
   if (shader->love_ScreenSize >= 0)
      /* TODO don't have to upload this every frame, only when resized... */
      glUniform4f( shader->love_ScreenSize, SCREEN_W, SCREEN_H, 1., 0. );

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
   const mat4 ortho = mat4_ortho(0., 1., 1., 0., 1., -1.);
   gl_uniformMat4(shader->ClipSpaceFromLocal, &ortho);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shader->VertexPosition );
   if (shader->VertexTexCoord >= 0)
      glDisableVertexAttribArray( shader->VertexTexCoord );
   glUseProgram( 0 );

   /* Restore the default mode. */
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

/**
 * @brief Renders a list of FBOs.
 */
static void render_fbo_list( double dt, PPShader *list, int *current, int done )
{
   PPShader *pplast;
   int i, cur, next;
   cur = *current;

   /* Render all except the last post-process shader. */
   for (i=0; i<array_size(list)-1; i++) {
      PPShader *pp = &list[i];
      next = 1-cur;
      /* Render cur to next. */
      render_fbo( dt, gl_screen.fbo[next], gl_screen.fbo_tex[cur], pp );
      cur = next;
   }

   /* Final render is to the screen. */
   pplast = &list[i];
   if (done) {
      gl_screen.current_fbo = 0;
      /* Do the render. */
      render_fbo( dt, gl_screen.current_fbo, gl_screen.fbo_tex[cur], pplast );
      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
      return;

   }

   /* Draw the last shader. */
   next = 1-cur;
   render_fbo( dt, gl_screen.fbo[next], gl_screen.fbo_tex[cur], pplast );
   cur = next;

   /* Set the framebuffer again. */
   gl_screen.current_fbo = gl_screen.fbo[cur];
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);

   /* Set the new current framebuffer. */
   *current = cur;
}

/**
 * @brief Renders the game itself (player flying around and friends).
 *
 * Blitting order (layers):
 *   - BG
 *     - stars and spobs
 *     - background player stuff (spob targeting)
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
   NTracingZone( _ctx, 1 );

   double dt;
   int pp_core, pp_final, pp_gui, pp_game;
   int cur = 0;

   /* See what post-processing is up. */
   pp_game  = (array_size(pp_shaders_list[PP_LAYER_GAME]) > 0);
   pp_gui   = (array_size(pp_shaders_list[PP_LAYER_GUI]) > 0);
   pp_final = (array_size(pp_shaders_list[PP_LAYER_FINAL]) > 0);
   pp_core  = (array_size(pp_shaders_list[PP_LAYER_CORE]) > 0);

   /* Case we have a post-processing shader we use the framebuffers. */
   if (pp_game || pp_gui || pp_final || pp_core) {
      /* Clear main screen. */
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /* Clear back buffer. */
      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.fbo[1]);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /* Set to front buffer. */
      gl_screen.current_fbo = gl_screen.fbo[cur];
   }
   else
      gl_screen.current_fbo = 0;

   /* Bind and clear new drawing area. */
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   dt = (paused) ? 0. : game_dt;

   /* Set up the default viewport. */
   gl_defViewport();

   /* Background stuff */
   space_render( real_dt ); /* Nebula looks really weird otherwise. */
   render_reset(); /* space_render can use a lua background. */
   NTracingZoneName( _ctx_renderbg, "hooks[renderbg]", 1 );
   hooks_run( "renderbg" );
   NTracingZoneEnd( _ctx_renderbg );
   render_reset();
   spobs_render();
   spfx_render(SPFX_LAYER_BACK, dt);
   weapons_render(WEAPON_LAYER_BG, dt);
   /* Middle stuff */
   player_renderUnderlay(dt);
   pilots_render();
   weapons_render(WEAPON_LAYER_FG, dt);
   spfx_render(SPFX_LAYER_MIDDLE, dt);
   /* Foreground stuff */
   player_render(dt);
   spfx_render(SPFX_LAYER_FRONT, dt);
   space_renderOverlay(dt);
   render_reset(); /* space_render can use a lua background. */
   gui_renderReticles(dt);
   pilots_renderOverlay();
   NTracingZoneName( _ctx_renderfg, "hooks[renderfg]", 1 );
   hooks_run( "renderfg" );
   NTracingZoneEnd( _ctx_renderfg );
   render_reset();

   /* Process game stuff only. */
   if (pp_game) {
      NTracingZoneName( _ctx_pp_game, "postprocess_shader[game]", 1 );
      render_fbo_list( dt, pp_shaders_list[PP_LAYER_GAME], &cur, !(pp_core || pp_final || pp_gui) );
      NTracingZoneEnd( _ctx_pp_game );
   }

   /* GUi stuff. */
   gui_render(dt);
   render_reset();

   if (pp_gui) {
      NTracingZoneName( _ctx_pp_gui, "postprocess_shader[gui]", 1 );
      render_fbo_list( dt, pp_shaders_list[PP_LAYER_GUI], &cur, !(pp_core || pp_final) );
      NTracingZoneEnd( _ctx_pp_gui );
   }

   /* We set the to fullscreen, ignoring the GUI modifications. */
   gl_viewport( 0, 0, gl_screen.nw, gl_screen.nh );

   /* Top stuff. */
   ovr_render( real_dt ); /* Using real_dt is sort of a hack for now. */
   NTracingZoneName( _ctx_rendertop, "hooks[rendertop]", 1 );
   hooks_run( "rendertop" );
   NTracingZoneEnd( _ctx_rendertop );
   render_reset();
   fps_display( real_dt ); /* Exception using real_dt. */
   if (!menu_open)
      toolkit_render( real_dt );

   /* Final post-processing. */
   if (pp_final) {
      NTracingZoneName( _ctx_pp_final, "postprocess_shader[final]", 1 );
      render_fbo_list( dt, pp_shaders_list[PP_LAYER_FINAL], &cur, !(pp_core) );
      NTracingZoneEnd( _ctx_pp_final );
   }

   if (menu_open)
      toolkit_render( real_dt );

   /* Final post-processing. */
   if (pp_core) {
      NTracingZoneName( _ctx_pp_core, "postprocess_shader[core]", 1 );
      render_fbo_list( dt, pp_shaders_list[PP_LAYER_CORE], &cur, 1 );
      NTracingZoneEnd( _ctx_pp_core );
   }

   /* check error every loop */
   gl_checkErr();

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Sorts shaders by priority.
 */
static int ppshader_compare( const void *a, const void *b )
{
   const PPShader *ppa = a;
   const PPShader *ppb = b;
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
 *    @param layer Which layer to apply the shader to.
 *    @param priority When it should be run (lower is sooner).
 *    @param flags Properties of the shader.
 *    @return The shader ID.
 */
unsigned int render_postprocessAdd( LuaShader_t *shader, int layer, int priority, unsigned int flags )
{
   PPShader *pp, **pp_shaders;
   unsigned int id;

   /* Select the layer. */
   if (layer < 0 || layer >= PP_LAYER_MAX) {
      WARN(_("Unknown post-processing shader layer '%d'!"), layer);
      return 0;
   }
   pp_shaders = &pp_shaders_list[layer];

   if (*pp_shaders==NULL)
      *pp_shaders = array_create( PPShader );
   pp = &array_grow( pp_shaders );
   id = ++pp_shaders_id;
   pp->id               = id;
   pp->priority         = priority;
   pp->flags            = flags;
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
   pp->love_ScreenSize = glGetUniformLocation( pp->program, "love_ScreenSize" );
   pp->dt = 0.;

   /* Resort n case stuff is weird. */
   qsort( *pp_shaders, array_size(*pp_shaders), sizeof(PPShader), ppshader_compare );

   gl_checkErr();

   return id;
}

/**
 * @brief Removes a post-process shader by ID.
 *
 *    @param id ID of shader to remove.
 *    @return 0 on success.
 */
int render_postprocessRm( unsigned int id )
{
   int j;
   int found = -1;
   for (j=0; j<PP_LAYER_MAX; j++) {
      PPShader *pp_shaders = pp_shaders_list[j];
      for (int i=0; i<array_size(pp_shaders); i++) {
         const PPShader *pp = &pp_shaders[i];
         if (pp->id != id)
            continue;
         found = i;
         break;
      }
      if (found>=0)
         break;
   }
   if (found==-1) {
      /* Don't warn since they can get cleaned up twice: once from postprocessCleanup, once from Lua gc. */
      //WARN(_("Trying to remove non-existant post-processing shader with id '%d'!"), id);
      return -1;
   }

   /* No need to resort. */
   array_erase( &pp_shaders_list[j], &pp_shaders_list[j][found], &pp_shaders_list[j][found+1] );
   return 0;
}

/**
 * @brief Cleans up the post-processing shaders.
 */
void render_postprocessCleanup (void)
{
   for (int j=0; j<PP_LAYER_MAX; j++) {
      PPShader *pp_shaders = pp_shaders_list[j];
      for (int i=array_size(pp_shaders)-1; i>=0; i--) {
         const PPShader *pp = &pp_shaders[i];
         if (pp->flags & PP_SHADER_PERMANENT)
            continue;
         array_erase( &pp_shaders_list[j], &pp_shaders_list[j][i], &pp_shaders_list[j][i+1] );
      }
   }
   /* No need to resort. */
}

/**
 * @brief Sets up the post-processing stuff.
 */
void render_init (void)
{
   LuaShader_t *s = &gamma_correction_shader;
   memset( s, 0, sizeof(LuaShader_t) );
   s->program            = shaders.gamma_correction.program;
   s->VertexPosition     = shaders.gamma_correction.VertexPosition;
   s->ClipSpaceFromLocal = shaders.gamma_correction.ClipSpaceFromLocal;
   s->MainTex            = shaders.gamma_correction.MainTex;

   /* Initialize the gamma. */
   render_setGamma( conf.gamma_correction );
}

/**
 * @brief Cleans up the post-processing stuff.
 */
void render_exit (void)
{
   for (int i=0; i<PP_LAYER_MAX; i++) {
      array_free( pp_shaders_list[i] );
      pp_shaders_list[i] = NULL;
   }
}

/**
 * @brief Sets the gamma.
 */
void render_setGamma( double gamma )
{
   if (pp_gamma_correction > 0) {
      render_postprocessRm( pp_gamma_correction );
      pp_gamma_correction = 0;
   }

   /* Ignore small gamma. */
   if (fabs(gamma-1.) < 1e-3)
      return;

   /* Set gamma and upload. */
   glUseProgram( shaders.gamma_correction.program );
   glUniform1f( shaders.gamma_correction.gamma, gamma );
   glUseProgram( 0 );
   pp_gamma_correction = render_postprocessAdd( &gamma_correction_shader, PP_LAYER_CORE, 98, PP_SHADER_PERMANENT );
}

static int needsReset = 0;
/**
 * @brief Resets the OpenGL stuff if it was changed by Lua.
 */
void render_reset (void)
{
   if (!needsReset)
      return;
   needsReset = 0;

   glBlendEquation( GL_FUNC_ADD );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   gl_unclipRect();
   canvas_reset();
}

/**
 * @brief Tells the rendering engine that it needs to restore the OpenGL state when possible.
 */
void render_needsReset (void)
{
   needsReset = 1;
}
