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
#include "gui.h"
#include "log.h"
#include "menu.h"
#include "opengl.h"
#include "perlin.h"
#include "player.h"
#include "rng.h"
#include "spfx.h"


#define NEBULA_PUFFS         32 /**< Amount of puffs to generate */
#define NEBULA_PUFF_BUFFER   300 /**< Nebula buffer */


/* Nebula properties */
static double nebu_density = 0.; /**< The density. */
static double nebu_view = 0.; /**< How far player can see. */
static double nebu_dt   = 0.; /**< How fast nebula changes. */
static double nebu_time = 0.; /**< Timer since last render. */

/* puff textures */
static glTexture *nebu_pufftexs[NEBULA_PUFFS]; /**< Nebula puffs. */

/* VBOs */
static gl_vbo *nebu_vboOverlay   = NULL; /**< Overlay VBO. */


/**
 * @struct NebulaPuff
 *
 * @brief Represents a nebula puff.
 */
typedef struct NebulaPuff_ {
   double x; /**< X position. */
   double y; /**< Y position */
   double height; /**< height vs player */
   int tex; /**< Texture */
} NebulaPuff;
static NebulaPuff *nebu_puffs = NULL; /**< Stack of puffs. */
static int nebu_npuffs        = 0; /**< Number of puffs. */
static double puff_x          = 0.;
static double puff_y          = 0.;


/*
 * prototypes
 */
static SDL_Surface* nebu_surfaceFromNebulaMap( float* map, const int w, const int h );
/* Puffs. */
static void nebu_generatePuffs (void);
static void nebu_renderPuffs( int below_player );
/* Nebula render methods. */
static void nebu_renderBackground( const double dt );


/**
 * @brief Initializes the nebula.
 *
 *    @return 0 on success.
 */
int nebu_init (void)
{
   nebu_generatePuffs();
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
   int i;

   /* Free the puffs. */
   for (i=0; i<NEBULA_PUFFS; i++)
      gl_freeTexture( nebu_pufftexs[i] );

   gl_vboDestroy( nebu_vboOverlay );
   nebu_vboOverlay= NULL;
}


/**
 * @brief Renders the nebula.
 *
 *    @param dt Current delta tick.
 */
void nebu_render( const double dt )
{
   nebu_renderBackground(dt);
   nebu_renderPuffs( 1 );
}


/**
 * @brief Renders the nebula using the multitexture approach.
 *
 *    @param dt Current delta tick.
 */
static void nebu_renderBackground( const double dt )
{
   double sx, sy;
   gl_Matrix4 projection;

   /* calculate frame to draw */
   nebu_time += dt / nebu_dt;

   /* Compensate possible rumble */
   spfx_getShake( &sx, &sy );

   glUseProgram(shaders.nebula_background.program);

   /* Set the vertex. */
   projection = gl_view_matrix;
   projection = gl_Matrix4_Translate(projection, -sx, -sy, 0);
   projection = gl_Matrix4_Scale(projection, gl_screen.w, gl_screen.h, 1);
   glEnableVertexAttribArray( shaders.nebula_background.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.nebula_background.vertex, 0, 2, GL_FLOAT, 0 );

   /* Set shader uniforms. */
   gl_uniformColor(shaders.nebula_background.color, &cBlue);
   gl_Matrix4_Uniform(shaders.nebula_background.projection, projection);
   glUniform2f(shaders.nebula_background.center, gl_screen.w / 2, gl_screen.h / 2);
   glUniform1f(shaders.nebula_background.radius, nebu_view * cam_getZoom());
   glUniform1f(shaders.nebula_background.time, nebu_time);

   /* Draw. */
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   /* Clean up. */
   glDisableVertexAttribArray( shaders.nebula_background.vertex );
   gl_checkErr();
   glUseProgram(0);
}


/**
 * @brief Updates visibility and stuff.
 */
void nebu_update( double dt )
{
   (void) dt;
   double mod = 1.;

   if (player.p != NULL)
      mod = player.p->ew_detect;

   /* At density 1000 you have zero visibility. */
   nebu_view = (1000. - nebu_density) * mod * 2;
}


/**
 * @brief Regenerates the overlay.
 */
void nebu_genOverlay (void)
{
   GLfloat vertex[8];

   /* See if need to generate overlay. */
   if (nebu_vboOverlay == NULL) {
      vertex[0] = -.5;
      vertex[1] = -.5;
      vertex[2] = .5;
      vertex[3] = -.5;
      vertex[4] = -.5;
      vertex[5] = .5;
      vertex[6] = .5;
      vertex[7] = .5;
      nebu_vboOverlay = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );
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
   double gx, gy;
   double z;
   gl_Matrix4 projection;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Get zoom. */
   z = cam_getZoom();

   /*
    * Renders the puffs
    */
   nebu_renderPuffs( 0 );

   /* Prepare the matrix */
   /* TODO
   ox = gx;
   oy = gy;
   spfx_getShake( &sx, &sy );
   ox += sx;
   oy += sy;
   projection = gl_Matrix4_Translate(gl_view_matrix, SCREEN_W/2.+ox, SCREEN_H/2.+oy, 0);
   projection = gl_Matrix4_Scale(projection, z, z, 1);
   */

   projection = gl_Matrix4_Identity();
   projection = gl_Matrix4_Scale(projection, gl_screen.rw, gl_screen.rh, 1);

   glUseProgram(shaders.nebula.program);
   gl_uniformColor(shaders.nebula.color, &cDarkBlue);
   gl_Matrix4_Uniform(shaders.nebula.projection, projection);
   glUniform2f(shaders.nebula.center, gl_screen.rw / 2, gl_screen.rh / 2);
   glUniform1f(shaders.nebula.radius, nebu_view * z * (1 / gl_screen.scale));

   glEnableVertexAttribArray(shaders.nebula.vertex);
   gl_vboActivateAttribOffset( nebu_vboOverlay, shaders.nebula.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

   glUseProgram(0);
   glDisableVertexAttribArray(shaders.nebula.vertex);

   /* Reset puff movement. */
   puff_x = 0.;
   puff_y = 0.;

   gl_checkErr();
}


/**
 * @brief Renders the puffs.
 *
 *    @param below_player Render the puffs below player or above player?
 */
static void nebu_renderPuffs( int below_player )
{
   int i;

   /* Main menu shouldn't have puffs */
   if (menu_isOpen(MENU_MAIN)) return;

   for (i=0; i<nebu_npuffs; i++) {

      /* Separate by layers */
      if ((below_player && (nebu_puffs[i].height < 1.)) ||
            (!below_player && (nebu_puffs[i].height > 1.))) {

         /* calculate new position */
         nebu_puffs[i].x += puff_x * nebu_puffs[i].height;
         nebu_puffs[i].y += puff_y * nebu_puffs[i].height;

         /* Check boundaries */
         if (nebu_puffs[i].x > SCREEN_W + NEBULA_PUFF_BUFFER)
            nebu_puffs[i].x -= SCREEN_W + 2*NEBULA_PUFF_BUFFER;
         else if (nebu_puffs[i].y > SCREEN_H + NEBULA_PUFF_BUFFER)
            nebu_puffs[i].y -= SCREEN_H + 2*NEBULA_PUFF_BUFFER;
         else if (nebu_puffs[i].x < -NEBULA_PUFF_BUFFER)
            nebu_puffs[i].x += SCREEN_W + 2*NEBULA_PUFF_BUFFER;
         else if (nebu_puffs[i].y < -NEBULA_PUFF_BUFFER)
            nebu_puffs[i].y += SCREEN_H + 2*NEBULA_PUFF_BUFFER;

         /* Render */
         gl_blitStatic( nebu_pufftexs[nebu_puffs[i].tex],
               nebu_puffs[i].x, nebu_puffs[i].y, &cLightBlue );
      }
   }
}


/**
 * @brief Moves the nebula puffs.
 */
void nebu_movePuffs( double x, double y )
{
   puff_x += x;
   puff_y += y;
}


/**
 * @brief Prepares the nebualae to be rendered.
 *
 *    @param density Density of the nebula (0-1000).
 *    @param volatility Volatility of the nebula (0-1000).
 */
void nebu_prep( double density, double volatility )
{
   (void)volatility;
   int i;

   nebu_density = density;
   nebu_update( 0. );
   nebu_dt   = 2000. / (density + 100.); /* Faster at higher density */
   nebu_time = 0.;

   nebu_npuffs = density/4.;
   nebu_puffs = realloc(nebu_puffs, sizeof(NebulaPuff)*nebu_npuffs);
   for (i=0; i<nebu_npuffs; i++) {
      /* Position */
      nebu_puffs[i].x = (double)RNG(-NEBULA_PUFF_BUFFER,
            SCREEN_W + NEBULA_PUFF_BUFFER);
      nebu_puffs[i].y = (double)RNG(-NEBULA_PUFF_BUFFER,
            SCREEN_H + NEBULA_PUFF_BUFFER);

      /* Maybe make size related? */
      nebu_puffs[i].tex = RNG(0,NEBULA_PUFFS-1);
      nebu_puffs[i].height = RNGF() + 0.2;
   }

   /* Generate the overlay. */
   nebu_genOverlay();
}


/**
 * @brief Generates nebula puffs.
 */
static void nebu_generatePuffs (void)
{
   int i;
   int w,h;
   SDL_Surface *sur;
   float *nebu;

   /* Generate the nebula puffs */
   for (i=0; i<NEBULA_PUFFS; i++) {
      /* Generate the nebula */
      w = h = RNG(20,64);
      nebu = noise_genNebulaPuffMap( w, h, 1. );
      sur = nebu_surfaceFromNebulaMap( nebu, w, h );
      free(nebu);

      /* Load the texture */
      nebu_pufftexs[i] =  gl_loadImage( sur, 0 );
   }
}


/**
 * @brief Generates a SDL_Surface from a 2d nebula map
 *
 *    @param map Nebula map to use.
 *    @param w Map width.
 *    @param h Map height.
 *    @return A SDL Surface with the nebula.
 */
static SDL_Surface* nebu_surfaceFromNebulaMap( float* map, const int w, const int h )
{
   int i;
   SDL_Surface *sur;
   uint32_t *pix;
   double c;

   /* the good surface */
   sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
   pix = sur->pixels;

   /* convert from mapping to actual colours */
   SDL_LockSurface( sur );
   for (i=0; i<h*w; i++) {
      c = map[i];
      pix[i] = RMASK + BMASK + GMASK + (AMASK & (uint32_t)((double)AMASK*c));
   }
   SDL_UnlockSurface( sur );

   return sur;
}
