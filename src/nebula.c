/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nebula.c
 *
 * @brief Handles rendering and generating the nebula.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "nebula.h"

#include "camera.h"
#include "conf.h"
#include "gui.h"
#include "log.h"
#include "menu.h"
#include "opengl.h"
#include "player.h"
#include "ntracing.h"
#include "rng.h"
#include "spfx.h"
#include "vec2.h"

#define NEBULA_PUFF_BUFFER   300 /**< Nebula buffer */

/* Nebula properties */
static double nebu_hue = 0.; /**< The hue. */
static double nebu_density = 0.; /**< The density. */
static double nebu_dx   = 0.; /**< Length scale (space coords) for turbulence/eddies we draw. */
static double nebu_view = 0.; /**< How far player can see. */
static double nebu_dt   = 0.; /**< How fast nebula changes. */
static double nebu_time = 0.; /**< Elapsed time since entering system. */

/* Nebula scaling stuff. */
static double nebu_scale = 4.; /**< How much to scale nebula. */
static int nebu_dofbo    = 0;
static GLuint nebu_fbo   = GL_INVALID_VALUE;
static GLuint nebu_tex   = GL_INVALID_VALUE;
static GLfloat nebu_render_w= 0.;
static GLfloat nebu_render_h= 0.;
static mat4 nebu_render_P;

/**
 * @struct NebulaPuff
 *
 * @brief Represents a nebula puff.
 */
typedef struct NebulaPuff_ {
   vec2 pos;   /**< Position. */
   double height;/**< Height vs player (1.0==player) */
   double s;   /**< Size of the puff (radius). */
   double rx;  /**< Random seed. */
   double ry;  /**< Random seed. */
} NebulaPuff;
static NebulaPuff *nebu_puffs = NULL; /**< Stack of puffs. */
static int nebu_npuffs        = 0; /**< Number of puffs. */
static double puff_x          = 0.;
static double puff_y          = 0.;

/*
 * prototypes
 */
/* Puffs. */
static void nebu_renderPuffs( int below_player );
/* Nebula render methods. */
static void nebu_renderBackground( const double dt );
static void nebu_blitFBO (void);

/**
 * @brief Initializes the nebula.
 *
 *    @return 0 on success.
 */
int nebu_init (void)
{
   nebu_time = -1000.0 * RNGF();
   return nebu_resize();
}

/**
 * @brief Handles a screen s
 *
 *    @return 0 on success.
 */
int nebu_resize (void)
{
   double scale;
   GLfloat fbo_w, fbo_h;

   scale = conf.nebu_scale * gl_screen.scale;
   fbo_w = round(gl_screen.nw/scale);
   fbo_h = round(gl_screen.nh/scale);
   if (scale == nebu_scale && fbo_w == nebu_render_w && fbo_h == nebu_render_h)
      return 0;

   nebu_scale = scale;
   nebu_render_w = fbo_w;
   nebu_render_h = fbo_h;
   nebu_dofbo = (nebu_scale != 1.);
   glDeleteTextures( 1, &nebu_tex );
   glDeleteFramebuffers( 1, &nebu_fbo );

   if (nebu_dofbo)
      gl_fboCreate( &nebu_fbo, &nebu_tex, nebu_render_w, nebu_render_h );

   /* Set up the matrices. */
   nebu_render_P = mat4_identity();
   mat4_translate_scale_xy( &nebu_render_P, -nebu_render_w/2., -nebu_render_h/2., nebu_render_w, nebu_render_h );
   glUseProgram(shaders.nebula_background.program);
   gl_uniformMat4(shaders.nebula_background.projection, &nebu_render_P);
   glUseProgram(shaders.nebula.program);
   gl_uniformMat4(shaders.nebula.projection, &nebu_render_P);
   glUseProgram(0);

   return 0;
}

/**
 * @brief Gets the nebula view radius.
 *
 *    @return The nebula view radius.
 */
double nebu_getSightRadius (void)
{
   return nebu_view;
}

/**
 * @brief Cleans up the nebu subsystem.
 */
void nebu_exit (void)
{
   if (nebu_dofbo) {
      glDeleteFramebuffers( 1, &nebu_fbo );
      glDeleteTextures( 1, &nebu_tex );
   }
}

/**
 * @brief Renders the nebula.
 *
 *    @param dt Current delta tick.
 */
void nebu_render( const double dt )
{
   NTracingZone( _ctx, 1 );

   nebu_renderBackground(dt);
   nebu_renderPuffs( 1 );

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Renders the nebula using the multitexture approach.
 *
 *    @param dt Current delta tick.
 */
static void nebu_renderBackground( const double dt )
{
   /* calculate frame to draw */
   nebu_time += dt * nebu_dt;

   if (nebu_dofbo) {
      glBindFramebuffer(GL_FRAMEBUFFER, nebu_fbo);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   /* Start the program. */
   glUseProgram(shaders.nebula_background.program);

   /* Set shader uniforms. */
   glUniform1f(shaders.nebula_background.eddy_scale, nebu_view * cam_getZoom() / nebu_scale);
   glUniform1f(shaders.nebula_background.time, nebu_time);
   glUniform1f(shaders.nebula_background.nonuniformity, conf.nebu_nonuniformity);

   /* Draw. */
   glEnableVertexAttribArray( shaders.nebula_background.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.nebula_background.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   nebu_blitFBO();

   /* Clean up. */
   glDisableVertexAttribArray( shaders.nebula_background.vertex );
   glUseProgram(0);
   gl_checkErr();
}

/**
 * @brief If we're drawing the nebula buffered, copy to the screen.
 */
static void nebu_blitFBO (void)
{
   if (!nebu_dofbo)
      return;

   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);

   glUseProgram(shaders.texture.program);

   glBindTexture( GL_TEXTURE_2D, nebu_tex );

   glEnableVertexAttribArray( shaders.texture.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.texture.vertex,
         0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformColour(shaders.texture.colour, &cWhite);

   const mat4 ortho = mat4_ortho(0, 1, 0, 1, 1, -1);
   const mat4 I = mat4_identity();
   gl_uniformMat4( shaders.texture.projection, &ortho );
   gl_uniformMat4( shaders.texture.tex_mat, &I );

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clear state. */
   glDisableVertexAttribArray( shaders.texture.vertex );
}

/**
 * @brief Updates visibility and stuff.
 */
void nebu_update( double dt )
{
   (void) dt;
   double mod = 1.;
   double bonus = 0.;

   if (player.p != NULL) {
      mod = player.p->stats.ew_detect;
      bonus = player.p->stats.nebu_visibility;
   }

   /* At density 1200 you have zero visibility. */
   nebu_view = (1200. - nebu_density) * mod + bonus;

   /* Below only care if not simulating. */
   if (space_isSimulation())
      return;

   /* Update puffs. */
   for (int i=0; i<nebu_npuffs; i++) {
      double dx, dy;
      NebulaPuff *puff = &nebu_puffs[i];

      /* Get camera stuff. */
      cam_getDPos( &dx, &dy );

      /* Calculate new position */
      puff->pos.x -= dx * puff->height;
      puff->pos.y -= dy * puff->height;

      /* Check boundaries */
      if (puff->pos.x > SCREEN_W + NEBULA_PUFF_BUFFER)
         puff->pos.x -= SCREEN_W + 2.*NEBULA_PUFF_BUFFER;
      else if (puff->pos.y > SCREEN_H + NEBULA_PUFF_BUFFER)
         puff->pos.y -= SCREEN_H + 2.*NEBULA_PUFF_BUFFER;
      else if (puff->pos.x < -NEBULA_PUFF_BUFFER)
         puff->pos.x += SCREEN_W + 2.*NEBULA_PUFF_BUFFER;
      else if (puff->pos.y < -NEBULA_PUFF_BUFFER)
         puff->pos.y += SCREEN_H + 2.*NEBULA_PUFF_BUFFER;
   }
}

/**
 * @brief Renders the nebula overlay (hides what player can't see).
 *
 *    @param dt Current delta tick.
 */
void nebu_renderOverlay( const double dt )
{
   (void) dt;
   double gx, gy, z;

   NTracingZone( _ctx, 1 );

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Get zoom. */
   z = cam_getZoom();

   /*
    * Renders the puffs
    */
   nebu_renderPuffs( 0 );

   /* Prepare the matrix */
   if (nebu_dofbo) {
      glBindFramebuffer(GL_FRAMEBUFFER, nebu_fbo);
      glClearColor( 0., 0., 0., 0. );
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }

   /* Start the program. */
   glUseProgram(shaders.nebula.program);

   /* Set shader uniforms. */
   glUniform1f(shaders.nebula.horizon, nebu_view * z / nebu_scale);
   glUniform1f(shaders.nebula.eddy_scale, nebu_dx * z / nebu_scale);
   glUniform1f(shaders.nebula.time, nebu_time);
   glUniform1f(shaders.nebula.nonuniformity, conf.nebu_nonuniformity);

   /* Draw. */
   glEnableVertexAttribArray(shaders.nebula.vertex);
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.nebula.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   nebu_blitFBO();

   /* Clean up. */
   glDisableVertexAttribArray( shaders.nebula.vertex );
   glClearColor( 0., 0., 0., 1. );
   glUseProgram(0);
   gl_checkErr();

   /* Reset puff movement. */
   puff_x = 0.;
   puff_y = 0.;

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Renders the puffs.
 *
 *    @param below_player Render the puffs below player or above player?
 */
static void nebu_renderPuffs( int below_player )
{
   /* Main menu shouldn't have puffs */
   if (menu_isOpen(MENU_MAIN))
      return;

   for (int i=0; i<nebu_npuffs; i++) {
      double x, y, s;
      mat4 projection;
      NebulaPuff *puff = &nebu_puffs[i];

      /* Separate by layers */
      if ((!below_player && (puff->height < 1.)) ||
            (below_player && (puff->height > 1.)))
         continue;

      /* Set up variables and do quick visibility check. */
      /* TODO this is weird in the sense that size gets scaled, but position doesn't with zoom... Probably should be harmonized... */
      s = puff->s * cam_getZoom();
      x = puff->pos.x - NEBULA_PUFF_BUFFER - s;
      y = puff->pos.y - NEBULA_PUFF_BUFFER - s;
      if ((x < -s) || (x > SCREEN_W+s) ||
            (y < -s) || (y > SCREEN_H+s))
         continue;

      /* Render */
      glUseProgram( shaders.nebula_puff.program );

      projection = gl_view_matrix;
      mat4_translate_scale_xy( &projection, x, y, s, s );
      glEnableVertexAttribArray(shaders.nebula_puff.vertex);
      gl_vboActivateAttribOffset( gl_circleVBO, shaders.nebula_puff.vertex, 0, 2, GL_FLOAT, 0 );

      /* Uniforms. */
      gl_uniformMat4( shaders.nebula_puff.projection, &projection );
      glUniform1f( shaders.nebula_puff.time, nebu_time / 1.5 );
      glUniform2f( shaders.nebula_puff.r, puff->rx, puff->ry );

      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      glDisableVertexAttribArray(shaders.nebula_puff.vertex);
      glUseProgram(0);
      gl_checkErr();
   }
}

/**
 * @brief Prepares the nebualae to be rendered.
 *
 *    @param density Density of the nebula (0-1000).
 *    @param volatility Volatility of the nebula.
 *    @param hue Hue of the nebula (0-1).
 */
void nebu_prep( double density, double volatility, double hue )
{
   glColour col;

   NTracingZone( _ctx, 1 );

   /* Set up ambient colour. */
   col_hsv2rgb( &col, nebu_hue*360., 1., 1.  );
   gltf_light( 3.0*col.r, 3.0*col.g, 3.0*col.b, 0.5 );

   /* Set the hue. */
   nebu_hue = hue;
   glUseProgram(shaders.nebula.program);
   glUniform1f(shaders.nebula.hue, nebu_hue);
   glUseProgram(shaders.nebula_background.program);
   glUniform1f(shaders.nebula_background.hue, nebu_hue);
   glUniform1f(shaders.nebula_background.volatility, volatility);

   /* Also set the hue for trails */
   col_hsv2rgb( &col, nebu_hue*360., 0.7, 1.0 );
   glUseProgram(shaders.trail.program);
   glUniform3f( shaders.trail.nebu_col, col.r, col.g, col.b );

   /* Also set the hue for puffs. */
   col_hsv2rgb( &col, nebu_hue*360., 0.95, 1.0 );
   glUseProgram(shaders.nebula_puff.program);
   glUniform3f( shaders.nebula_puff.nebu_col, col.r, col.g, col.b );

   /* Done setting shaders. */
   glUseProgram(0);

   if (density > 0.) {
      /* Set density parameters. */
      nebu_density = density;
      nebu_update( 0. );
      nebu_dt   = (2.*density + 200.) / 10e3; /* Faster at higher density */
      nebu_dx   = 15e3 / pow(density, 1./3.); /* Closer at higher density */
      nebu_time = 0.;

      nebu_npuffs = density/2.;
      nebu_puffs = realloc(nebu_puffs, sizeof(NebulaPuff)*nebu_npuffs);
      for (int i=0; i<nebu_npuffs; i++) {
         NebulaPuff *np = &nebu_puffs[i];

         /* Position */
         np->pos.x = (SCREEN_W+2.*NEBULA_PUFF_BUFFER)*RNGF();
         np->pos.y = (SCREEN_H+2.*NEBULA_PUFF_BUFFER)*RNGF();

         /* Maybe make size related? */
         np->s = RNG(10,32);
         np->height = RNGF() + 0.2;

         /* Seed. */
         np->rx = RNGF()*2000.-1000.;
         np->ry = RNGF()*2000.-1000.;
      }
   }

   NTracingZoneEnd( _ctx );
}
