/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nebulae.c
 *
 * @brief Handles rendering and generating the nebulae.
 */

#include "nebulae.h"

#include "naev.h"

#include <errno.h>

#include "SDL_image.h"

#include "log.h"
#include "opengl.h"
#include "nfile.h"
#include "perlin.h"
#include "rng.h"
#include "menu.h"
#include "player.h"
#include "pause.h"
#include "gui.h"
#include "conf.h"


#define NEBULAE_Z             16 /**< Z plane */
#define NEBULAE_PUFFS         32 /**< Amount of puffs to generate */
#define NEBULAE_DIR           "gen/" /**< Directory containing the nebulae stuff. */
#define NEBULAE_PATH_BG       NEBULAE_DIR"nebu_bg_%dx%d_%02d.png" /**< Nebulae path format. */

#define NEBULAE_PUFF_BUFFER   300 /**< Nebulae buffer */


/* Externs */
extern Vector2d shake_pos; /**< from spfx.c */
extern void loadscreen_render( double done, const char *msg ); /**< from naev.c */


/* The nebulae textures */
static GLuint nebu_textures[NEBULAE_Z]; /**< BG Nebulae textures. */
static int nebu_w    = 0; /**< BG Nebulae width. */
static int nebu_h    = 0; /**< BG Nebulae height. */
static int nebu_pw   = 0; /**< BG Padded Nebulae width. */
static int nebu_ph   = 0; /**< BG Padded Nebulae height. */

/* Information on rendering */
static int cur_nebu[2]           = { 0, 1 }; /**< Nebulaes currently rendering. */
static unsigned int last_render  = 0; /**< When they were last rendered. */

/* Nebulae properties */
static double nebu_view = 0.; /**< How far player can see. */
static double nebu_dt   = 0.; /**< How fast nebulae changes. */

/* puff textures */
static glTexture *nebu_pufftexs[NEBULAE_PUFFS]; /**< Nebulae puffs. */

/* VBOs */
static gl_vbo *nebu_vboOverlay   = NULL; /**< Overlay VBO. */
static gl_vbo *nebu_vboBG        = NULL; /**< BG VBO. */

/**
 * @struct NebulaePuff
 *
 * @brief Represents a nebulae puff.
 */
typedef struct NebulaePuff_ {
   double x; /**< X position. */
   double y; /**< Y position */
   double height; /**< height vs player */
   int tex; /**< Texture */
} NebulaePuff;
static NebulaePuff *nebu_puffs   = NULL; /**< Stack of puffs. */
static int nebu_npuffs           = 0; /**< Number of puffs. */


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void nebu_loadTexture( SDL_Surface *sur, int w, int h, GLuint tex );
static int nebu_generate (void);
static void nebu_generatePuffs (void);
static int saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebulae( const char* file );
static SDL_Surface* nebu_surfaceFromNebulaeMap( float* map, const int w, const int h );
static void nebu_genOverlay (void);
/* Nebulae render methods. */
static void nebu_renderMultitexture( const double dt );


/**
 * @brief Initializes the nebulae.
 *
 *    @return 0 on success.
 */
int nebu_init (void)
{
   int i;
   char nebu_file[PATH_MAX];
   SDL_Surface* nebu_sur;
   int ret;
   GLfloat vertex[4*3*2];
   GLfloat tw, th;

   /* Special code to regenerate the nebulae */
   if ((nebu_w == -9) && (nebu_h == -9)) {
      nebu_generate();
   }

   /* Set expected sizes */
   nebu_w = SCREEN_W;
   nebu_h = SCREEN_H;
   nebu_pw = gl_pot(nebu_w);
   nebu_ph = gl_pot(nebu_h);

   nebu_generatePuffs();

   /* Load each, checking for compatibility and padding */
   glGenTextures( NEBULAE_Z, nebu_textures );
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH_BG, nebu_w, nebu_h, i );

      if (nebu_checkCompat( nebu_file )) { /* Incompatible */
         LOG("No nebulae found, generating (this may take a while).");

         /* So we generate and reload */
         ret = nebu_generate();
         if (ret != 0) /* An error has happened - break recursivity*/
            return ret;

         return nebu_init();
      }

      /* Load the file */
      nebu_sur = loadNebulae( nebu_file );
      if ((nebu_sur->w != nebu_w) || (nebu_sur->h != nebu_h))
         WARN("Nebulae raw size doesn't match expected! (%dx%d instead of %dx%d)",
               nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );

      /* Load the texture */
      nebu_loadTexture( nebu_sur, nebu_pw, nebu_ph, nebu_textures[i] );
   }

   DEBUG("Loaded %d Nebulae Layers", NEBULAE_Z);


   /* Create the VBO. */
   /* Vertex. */
   vertex[0] = -SCREEN_W/2;
   vertex[1] = -SCREEN_H/2;
   vertex[2] = -vertex[0];
   vertex[3] =  vertex[0];
   vertex[4] =  vertex[0];
   vertex[5] = -vertex[1];
   vertex[6] =  vertex[2];
   vertex[7] =  vertex[5];
   /* Texture 0. */
   tw = (double)nebu_w / (double)nebu_pw;
   th = (double)nebu_h / (double)nebu_ph;
   vertex[8]  = 0.;
   vertex[9]  = 0.;
   vertex[10] = tw;
   vertex[11] = 0.;
   vertex[12] = 0.;
   vertex[13] = th;
   vertex[14] = tw;
   vertex[15] = th;
   /* Texture 1. */
   vertex[16] = 0.;
   vertex[17] = 0.;
   vertex[18] = tw;
   vertex[19] = 0.;
   vertex[20] = 0.;
   vertex[21] = th;
   vertex[22] = tw;
   vertex[23] = th;
   nebu_vboBG = gl_vboCreateStatic( sizeof(GLfloat) * (4*2*3), vertex );

   return 0;
}


/**
 * @brief Loads sur into tex, checks for expected size of w and h.
 *
 *    @param sur Surface to load into texture.
 *    @param w Expected width of surface.
 *    @param h Expected height of surface.
 *    @param tex Already generated texture to load into.
 */
static void nebu_loadTexture( SDL_Surface *sur, int w, int h, GLuint tex )
{
   SDL_Surface *nebu_sur;

   nebu_sur = gl_prepareSurface( sur );
   if ((w!=0) && (h!=0) &&
         ((nebu_sur->w != w) || (nebu_sur->h != h))) {
      WARN("Nebulae size doesn't match expected! (%dx%d instead of %dx%d)",
            nebu_sur->w, nebu_sur->h, nebu_pw, nebu_ph );
      return;
   }

   /* Load the texture */
   glBindTexture( GL_TEXTURE_2D, tex );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   /* Store into opengl saving only alpha channel in video memory */
   SDL_LockSurface( nebu_sur );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, nebu_sur->w, nebu_sur->h,
         0, GL_RGBA, GL_UNSIGNED_BYTE, nebu_sur->pixels );
   SDL_UnlockSurface( nebu_sur );

   SDL_FreeSurface(nebu_sur);
   gl_checkErr();
}


/**
 * @brief Cleans up the nebu subsystem.
 */
void nebu_exit (void)
{
   int i;

   /* Free the Nebulae BG. */
   glDeleteTextures( NEBULAE_Z, nebu_textures );

   /* Free the puffs. */
   for (i=0; i<NEBULAE_PUFFS; i++)
      gl_freeTexture( nebu_pufftexs[i] );

   /* Free the VBO. */
   if (nebu_vboBG != NULL) {
      gl_vboDestroy( nebu_vboBG );
      nebu_vboBG = NULL;
   }
   if (nebu_vboOverlay != NULL) {
      gl_vboDestroy( nebu_vboOverlay );
      nebu_vboOverlay= NULL;
   }
}


/**
 * @brief Renders the nebulae.
 *
 *    @param dt Current delta tick.
 */
void nebu_render( const double dt )
{
   if (nglActiveTexture != NULL) {
      nebu_renderMultitexture(dt);
   }

   /* Now render the puffs, they are generic. */
   nebu_renderPuffs( dt, 1 );
}


/**
 * @brief Renders the nebulae using the multitexture approach.
 *
 *    @param dt Current delta tick.
 */
static void nebu_renderMultitexture( const double dt )
{
   (void) dt;
   unsigned int t;
   double ndt;
   GLfloat col[4];
   int temp;

   /* calculate frame to draw */
   t = SDL_GetTicks();
   ndt = (t - last_render) / 1000.;
   if (ndt > nebu_dt) { /* Time to change */
      temp = cur_nebu[0];
      cur_nebu[0] += cur_nebu[0] - cur_nebu[1];
      cur_nebu[1] = temp;
      if (cur_nebu[0]+1 >= NEBULAE_Z)
         cur_nebu[0] = NEBULAE_Z - 2;
      else if (cur_nebu[0] < 0)
         cur_nebu[0] = 1;

      last_render = t;
      ndt = 0.;
   }

   /* Set the colour */
   col[0] = cPurple.r;
   col[1] = cPurple.g;
   col[2] = cPurple.b;
   col[3] = ndt / nebu_dt;

   /* Set up the targets */
   /* Texture 0 */
   nglActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[1]]);
   /* Texture 1 */
   nglActiveTexture( GL_TEXTURE1 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[0]]);

   /* Prepare it */
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_INTERPOLATE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_INTERPOLATE );
   /* Colour */
   glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col );

   /* Arguments */
   /* Arg0 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB,    GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,  GL_TEXTURE );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
   /* Arg1 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB,    GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,  GL_PREVIOUS );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
   /* Arg2 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB,    GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA,  GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB,   GL_SRC_ALPHA );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA );

   /* Compensate possible rumble */
   if (!paused) {
      gl_matrixPush();
      gl_matrixTranslate( shake_pos.x, shake_pos.y );
   }

   /* Now render! */
   gl_vboActivateOffset( nebu_vboBG, GL_VERTEX_ARRAY, 
         sizeof(GL_FLOAT) * 0*2*4, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( nebu_vboBG, GL_TEXTURE0,
         sizeof(GL_FLOAT) * 1*2*4, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( nebu_vboBG, GL_TEXTURE1,
         sizeof(GL_FLOAT) * 2*2*4, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   gl_vboDeactivate();

   if (!paused)
      gl_matrixPop();

   /* Set values to defaults */
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);
   nglActiveTexture( GL_TEXTURE0 );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   glDisable(GL_TEXTURE_2D);

   /* Anything failed? */
   gl_checkErr();
}


#define ANG45     0.70710678118654757 /**< 1./sqrt(2) */
#define COS225    0.92387953251128674 /**< cos(225) */
#define SIN225    0.38268343236508978 /**< sin(225) */
static void nebu_genOverlay (void)
{
   int i;
   GLfloat *data;
   double a;
   double gx, gy;
   double z;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Calculate zoom. */
   z = 1./conf.zoom_min;

   /* See if need to generate overlay. */
   if (nebu_vboOverlay == NULL) {
      nebu_vboOverlay = gl_vboCreateStatic( sizeof(GLfloat) *
            ((2+4)*18 + 2*28 + 4*7), NULL );

      /* Set colors, those will be pure static. */
      data = gl_vboMap( nebu_vboOverlay );

      /* Alpha overlay. */
      for (i=0; i<18; i++) {
         data[2*18 + 4*i + 0] = cPurple.r;
         data[2*18 + 4*i + 1] = cPurple.g;
         data[2*18 + 4*i + 2] = cPurple.b;
         data[2*18 + 4*i + 3] = cPurple.a;
      }
      data[2*18 + 3] = 0.; /* Origin is transparent. */

      /* Solid overlay. */
      for (i=0; i<7; i++) {
         data[(2+4)*18 + 2*28 + 4*i + 0] = cPurple.r;
         data[(2+4)*18 + 2*28 + 4*i + 1] = cPurple.g;
         data[(2+4)*18 + 2*28 + 4*i + 2] = cPurple.b;
         data[(2+4)*18 + 2*28 + 4*i + 3] = cPurple.a;
      }

      gl_vboUnmap( nebu_vboOverlay );
   }

   /* Generate the main chunk. */
   data = gl_vboMap( nebu_vboOverlay );

   /* Main chunk. */
   data[0] = 0.;
   data[1] = 0.;
   for (i=0; i<17; i++) {
      a = M_PI*2./16. * (double)i;
      data[2*(i+1) + 0] = nebu_view * cos(a);
      data[2*(i+1) + 1] = nebu_view * sin(a);
   }

   /* Top Left */
   data[(2+4)*18+0]  = -SCREEN_W/2.*z-gx;
   data[(2+4)*18+1]  = SCREEN_H/2.*z-gy;
   data[(2+4)*18+2]  = -nebu_view;
   data[(2+4)*18+3]  = 0.;
   data[(2+4)*18+4]  = -nebu_view*COS225;
   data[(2+4)*18+5]  = nebu_view*SIN225;
   data[(2+4)*18+6]  = -nebu_view*ANG45;
   data[(2+4)*18+7]  = nebu_view*ANG45;
   data[(2+4)*18+8]  = -nebu_view*SIN225;
   data[(2+4)*18+9]  = nebu_view*COS225;
   data[(2+4)*18+10] = 0.;
   data[(2+4)*18+11] = nebu_view;
   data[(2+4)*18+12] = SCREEN_W/2.*z-gx;
   data[(2+4)*18+13] = SCREEN_H/2.*z-gy;

   /* Top Right */
   data[(2+4)*18+14] = SCREEN_W/2.*z-gx;
   data[(2+4)*18+15] = SCREEN_H/2.*z-gy;
   data[(2+4)*18+16] = 0.;
   data[(2+4)*18+17] = nebu_view;
   data[(2+4)*18+18] = nebu_view*SIN225;
   data[(2+4)*18+19] = nebu_view*COS225;
   data[(2+4)*18+20] = nebu_view*ANG45;
   data[(2+4)*18+21] = nebu_view*ANG45;
   data[(2+4)*18+22] = nebu_view*COS225;
   data[(2+4)*18+23] = nebu_view*SIN225;
   data[(2+4)*18+24] = nebu_view;
   data[(2+4)*18+25] = 0.;
   data[(2+4)*18+26] = SCREEN_W/2.*z-gx;
   data[(2+4)*18+27] = -SCREEN_H/2.*z-gy;

   /* Bottom Right */
   data[(2+4)*18+28] = SCREEN_W/2.*z-gx;
   data[(2+4)*18+29] = -SCREEN_H/2.*z-gy;
   data[(2+4)*18+30] = nebu_view;
   data[(2+4)*18+31] = 0.;
   data[(2+4)*18+32] = nebu_view*COS225;
   data[(2+4)*18+33] = -nebu_view*SIN225;
   data[(2+4)*18+34] = nebu_view*ANG45;
   data[(2+4)*18+35] = -nebu_view*ANG45;
   data[(2+4)*18+36] = nebu_view*SIN225;
   data[(2+4)*18+37] = -nebu_view*COS225;
   data[(2+4)*18+38] = 0.;
   data[(2+4)*18+39] = -nebu_view;
   data[(2+4)*18+40] = -SCREEN_W/2.*z-gx;
   data[(2+4)*18+41] = -SCREEN_H/2.*z-gy;
      
   /* Bottom left */
   data[(2+4)*18+42] = -SCREEN_W/2.*z-gx;
   data[(2+4)*18+43] = -SCREEN_H/2.*z-gy;
   data[(2+4)*18+44] = 0.;
   data[(2+4)*18+45] = -nebu_view;
   data[(2+4)*18+46] = -nebu_view*SIN225;
   data[(2+4)*18+47] = -nebu_view*COS225;
   data[(2+4)*18+48] = -nebu_view*ANG45;
   data[(2+4)*18+49] = -nebu_view*ANG45;
   data[(2+4)*18+50] = -nebu_view*COS225;
   data[(2+4)*18+51] = -nebu_view*SIN225;
   data[(2+4)*18+52] = -nebu_view;
   data[(2+4)*18+53] = 0.;
   data[(2+4)*18+54] = -SCREEN_W/2.*z-gx;
   data[(2+4)*18+55] = SCREEN_H/2.*z-gy;

   gl_vboUnmap( nebu_vboOverlay );
}
#undef ANG45
#undef COS225
#undef SIN225


/**
 * @brief Renders the nebulae overlay (hides what player can't see).
 *
 *    @param dt Current delta tick.
 */
void nebu_renderOverlay( const double dt )
{
   double gx, gy;
   double ox, oy;
   double z;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Get zoom. */
   gl_cameraZoomGet( &z );

   /*
    * Renders the puffs
    */
   nebu_renderPuffs( dt, 0 );

   /* Prepare the matrix */
   ox = gx;
   oy = gy;
   if (!paused) {
      ox += shake_pos.x;
      oy += shake_pos.y;
   }
   gl_matrixPush();
   gl_matrixTranslate( ox, oy );
   gl_matrixScale( z, z );

   /*
    * Mask for area player can still see (partially)
    */
   glShadeModel(GL_SMOOTH);
   gl_vboActivateOffset( nebu_vboOverlay, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( nebu_vboOverlay, GL_COLOR_ARRAY,
         sizeof(GLfloat)*2*18, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 18 );


   /*
    * Solid nebulae for areas the player can't see
    */
   glShadeModel(GL_FLAT);
   /* Colour is shared. */
   gl_vboActivateOffset( nebu_vboOverlay, GL_COLOR_ARRAY,
         sizeof(GLfloat)*((2+4)*18 + 2*28), 4, GL_FLOAT, 0 );
   /* Top left. */
   gl_vboActivateOffset( nebu_vboOverlay, GL_VERTEX_ARRAY,
         sizeof(GLfloat)*((2+4)*18 + 0*2*7), 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 7 );
   /* Top right. */
   gl_vboActivateOffset( nebu_vboOverlay, GL_VERTEX_ARRAY,
         sizeof(GLfloat)*((2+4)*18 + 1*2*7), 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 7 );
   /* Bottom right. */
   gl_vboActivateOffset( nebu_vboOverlay, GL_VERTEX_ARRAY,
         sizeof(GLfloat)*((2+4)*18 + 2*2*7), 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 7 );
   /* Bottom left. */
   gl_vboActivateOffset( nebu_vboOverlay, GL_VERTEX_ARRAY,
         sizeof(GLfloat)*((2+4)*18 + 3*2*7), 2, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLE_FAN, 0, 7 );

   gl_vboDeactivate();
   gl_matrixPop();

   gl_checkErr();
}


/**
 * @brief Renders the puffs.
 *
 *    @param dt Current delta tick.
 *    @param below_player Render the puffs below player or above player?
 */
void nebu_renderPuffs( const double dt, int below_player )
{
   int i;

   /* Main menu shouldn't have puffs */
   if (menu_isOpen(MENU_MAIN)) return;

   for (i=0; i<nebu_npuffs; i++) {

      /* Seperate by layers */
      if ((below_player && (nebu_puffs[i].height < 1.)) ||
            (!below_player && (nebu_puffs[i].height > 1.))) {

         /* calculate new position */
         if (!paused && (player!=NULL)) {
            nebu_puffs[i].x -= player->solid->vel.x * nebu_puffs[i].height * dt;
            nebu_puffs[i].y -= player->solid->vel.y * nebu_puffs[i].height * dt;
         }

         /* Check boundries */
         if (nebu_puffs[i].x > SCREEN_W + NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].x -= SCREEN_W + 2*NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].y > SCREEN_H + NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].y -= SCREEN_H + 2*NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].x < -NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].x += SCREEN_W + 2*NEBULAE_PUFF_BUFFER;
         else if (nebu_puffs[i].y < -NEBULAE_PUFF_BUFFER)
            nebu_puffs[i].y += SCREEN_H + 2*NEBULAE_PUFF_BUFFER;

         /* Render */
         gl_blitStatic( nebu_pufftexs[nebu_puffs[i].tex],
               nebu_puffs[i].x, nebu_puffs[i].y, &cPurple );
      }
   }
}


/**
 * @brief Prepares the nebualae to be rendered.
 *
 *    @param density Density of the nebulae (0-1000).
 *    @param volatility Volatility of the nebulae (0-1000).
 */
void nebu_prep( double density, double volatility )
{
   (void)volatility;
   int i;

   nebu_view = 1000. - density;  /* At density 1000 you're blind */
   nebu_dt = 2000. / (density + 100.); /* Faster at higher density */

   nebu_npuffs = density/4.;
   nebu_puffs = realloc(nebu_puffs, sizeof(NebulaePuff)*nebu_npuffs);
   for (i=0; i<nebu_npuffs; i++) {
      /* Position */
      nebu_puffs[i].x = (double)RNG(-NEBULAE_PUFF_BUFFER,
            SCREEN_W + NEBULAE_PUFF_BUFFER);
      nebu_puffs[i].y = (double)RNG(-NEBULAE_PUFF_BUFFER,
            SCREEN_H + NEBULAE_PUFF_BUFFER);
      
      /* Maybe make size related? */
      nebu_puffs[i].tex = RNG(0,NEBULAE_PUFFS-1);
      nebu_puffs[i].height = RNGF() + 0.2;
   }

   /* Generate the overlay. */
   nebu_genOverlay();
}


/**
 * @brief Forces generation of new nebulae on init.
 */
void nebu_forceGenerate (void)
{
   nebu_w = nebu_h = -9; /* \o/ magic numbers */
}


/**
 * @brief Generates the nebulae.
 *
 *    @return 0 on success.
 */
static int nebu_generate (void)
{
   int i;
   float *nebu;
   char nebu_file[PATH_MAX];
   int w,h;
   int ret;

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, "Generating Nebulae (slow, run once)..." );

   /* Get resolution to create at. */
   w = SCREEN_W;
   h = SCREEN_H;

   /* Try to make the dir first if it fails. */
   nfile_dirMakeExist( "%s"NEBULAE_DIR, nfile_basePath() );

   /* Generate all the nebulae backgrounds */
   nebu = noise_genNebulaeMap( w, h, NEBULAE_Z, 5. );

   /* Start saving - compression can take a bit. */
   loadscreen_render( 0.05, "Compressing Nebulae layers..." );

   /* Save each nebulae as an image */
   for (i=0; i<NEBULAE_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULAE_PATH_BG, w, h, i );
      ret = saveNebulae( &nebu[ i*w*h ], w, h, nebu_file );
      if (ret != 0) break; /* An error has happenend */
   }

   /* Cleanup */
   free(nebu);
   return ret;
}


/**
 * @brief Generates nebulae puffs.
 */
static void nebu_generatePuffs (void)
{
   int i;
   int w,h;
   SDL_Surface *sur;
   float *nebu;

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, "Generating Nebulae Puffs..." );

   /* Generate the nebulae puffs */
   for (i=0; i<NEBULAE_PUFFS; i++) {

      /* Generate the nebulae */
      w = h = RNG(20,64);
      nebu = noise_genNebulaePuffMap( w, h, 1. );
      sur = nebu_surfaceFromNebulaeMap( nebu, w, h );
      free(nebu);
      
      /* Load the texture */
      nebu_pufftexs[i] =  gl_loadImage( sur, 0 );
   }
}


/**
 * @brief Checks the validity of a nebulae.
 *
 *    @param file Path of the nebulae to check (relative to base directory).
 *    @return 0 on success.
 */
static int nebu_checkCompat( const char* file )
{
   /* first check to see if file exists */
   if (nfile_fileExists("%s%s", nfile_basePath(), file) == 0)
      return -1;
   return 0;
}


/**
 * @brief Saves a nebulae.
 *
 *    @param map Nebulae map to save.
 *    @param w Width of nebulae map.
 *    @param h Height of nebulae map.
 *    @param file Path to save into.
 *    @return 0 on success.
 */
static int saveNebulae( float *map, const uint32_t w, const uint32_t h, const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;
   int ret;

   /* fix surface */
   sur = nebu_surfaceFromNebulaeMap( map, w, h );

   /* save */
   ret = 0;
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   ret = SDL_SavePNG( sur, file_path );

   /* cleanup */
   SDL_FreeSurface( sur );

   return ret;
}


/**
 * @brief Loads the nebuale from file.
 *
 *    @param file Path of the nebulae to load.  Relative to base directory.
 *    @return A SDL surface with the nebulae.
 */
static SDL_Surface* loadNebulae( const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;

   /* loads the file */
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   sur = IMG_Load( file_path );
   if (sur == NULL) {
      ERR("Unable to load Nebulae image: %s", file);
      return NULL;
   }
   
   return sur;
}



/**
 * @brief Generates a SDL_Surface from a 2d nebulae map
 *
 *    @param map Nebulae map to use.
 *    @param w Map width.
 *    @param h Map height.
 *    @return A SDL Surface with the nebulae.
 */
static SDL_Surface* nebu_surfaceFromNebulaeMap( float* map, const int w, const int h )
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
