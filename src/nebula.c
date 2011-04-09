/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nebula.c
 *
 * @brief Handles rendering and generating the nebula.
 */

#include "nebula.h"

#include "naev.h"

#include <errno.h>


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
#include "spfx.h"
#include "npng.h"
#include "camera.h"


#define NEBULA_Z             16 /**< Z plane */
#define NEBULA_PUFFS         32 /**< Amount of puffs to generate */
#define NEBULA_DIR           "gen/" /**< Directory containing the nebula stuff. */
#define NEBULA_PATH_BG       NEBULA_DIR"nebu_bg_%dx%d_%02d.png" /**< Nebula path format. */

#define NEBULA_PUFF_BUFFER   300 /**< Nebula buffer */


/* Externs */
extern void loadscreen_render( double done, const char *msg ); /**< from naev.c */


/* The nebula textures */
static GLuint nebu_textures[NEBULA_Z]; /**< BG Nebula textures. */
static int nebu_w    = 0; /**< BG Nebula width. */
static int nebu_h    = 0; /**< BG Nebula height. */
static int nebu_pw   = 0; /**< BG Padded Nebula width. */
static int nebu_ph   = 0; /**< BG Padded Nebula height. */

/* Information on rendering */
static int cur_nebu[2]           = { 0, 1 }; /**< Nebulas currently rendering. */
static double nebu_timer         = 0.; /**< Timer since last render. */

/* Nebula properties */
static double nebu_view = 0.; /**< How far player can see. */
static double nebu_dt   = 0.; /**< How fast nebula changes. */

/* puff textures */
static glTexture *nebu_pufftexs[NEBULA_PUFFS]; /**< Nebula puffs. */

/* VBOs */
static gl_vbo *nebu_vboOverlay   = NULL; /**< Overlay VBO. */
static gl_vbo *nebu_vboBG        = NULL; /**< BG VBO. */

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
static NebulaPuff *nebu_puffs   = NULL; /**< Stack of puffs. */
static int nebu_npuffs           = 0; /**< Number of puffs. */


/*
 * prototypes
 */
static int nebu_checkCompat( const char* file );
static void nebu_loadTexture( SDL_Surface *sur, int w, int h, GLuint tex );
static int nebu_generate (void);
static void nebu_generatePuffs (void);
static int saveNebula( float *map, const uint32_t w, const uint32_t h, const char* file );
static SDL_Surface* loadNebula( const char* file );
static SDL_Surface* nebu_surfaceFromNebulaMap( float* map, const int w, const int h );
/* Nebula render methods. */
static void nebu_renderMultitexture( const double dt );


/**
 * @brief Initializes the nebula.
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

   /* Special code to regenerate the nebula */
   if ((nebu_w == -9) && (nebu_h == -9)) {
      nebu_generate();
   }

   /* Set expected sizes */
   nebu_w  = SCREEN_W;
   nebu_h  = SCREEN_H;
   if (gl_needPOT()) {
      nebu_pw = gl_pot(nebu_w);
      nebu_ph = gl_pot(nebu_h);
   }
   else {
      nebu_pw = nebu_w;
      nebu_ph = nebu_h;
   }

   nebu_generatePuffs();

   /* Load each, checking for compatibility and padding */
   glGenTextures( NEBULA_Z, nebu_textures );
   for (i=0; i<NEBULA_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULA_PATH_BG, nebu_w, nebu_h, i );

      if (nebu_checkCompat( nebu_file )) { /* Incompatible */
         LOG("No nebula found, generating (this may take a while).");

         /* So we generate and reload */
         ret = nebu_generate();
         if (ret != 0) /* An error has happened - break recursivity*/
            return ret;

         return nebu_init();
      }

      /* Load the file */
      nebu_sur = loadNebula( nebu_file );
      if ((nebu_sur->w != nebu_w) || (nebu_sur->h != nebu_h))
         WARN("Nebula raw size doesn't match expected! (%dx%d instead of %dx%d)",
               nebu_sur->w, nebu_sur->h, nebu_w, nebu_h );

      /* Load the texture */
      nebu_loadTexture( nebu_sur, nebu_pw, nebu_ph, nebu_textures[i] );
   }

   DEBUG("Loaded %d Nebula Layers", NEBULA_Z);


   /* Create the VBO. */
   /* Vertex. */
   vertex[0] = 0;
   vertex[1] = 0;
   vertex[2] = 0;
   vertex[3] = SCREEN_H;
   vertex[4] = SCREEN_W;
   vertex[5] = 0;
   vertex[6] = SCREEN_W;
   vertex[7] = SCREEN_H;
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
 * @brief Gets the nebula view radius.
 *
 *    @return The nebula view radius.
 */
double nebu_getSightRadius (void)
{
   return nebu_view;
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
      WARN("Nebula size doesn't match expected! (%dx%d instead of %dx%d)",
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

   /* Free the Nebula BG. */
   glDeleteTextures( NEBULA_Z, nebu_textures );

   /* Free the puffs. */
   for (i=0; i<NEBULA_PUFFS; i++)
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
 * @brief Renders the nebula.
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
 * @brief Renders the nebula using the multitexture approach.
 *
 *    @param dt Current delta tick.
 */
static void nebu_renderMultitexture( const double dt )
{
   GLfloat col[4];
   int temp;
   double sx, sy;

   /* calculate frame to draw */
   nebu_timer -= dt;
   if (nebu_timer < 0.) { /* Time to change. */
      temp         = cur_nebu[0] - cur_nebu[1];
      cur_nebu[1]  = cur_nebu[0];
      cur_nebu[0] += temp;

      if (cur_nebu[0] >= NEBULA_Z)
         cur_nebu[0] = cur_nebu[1] - 1;
      else if (cur_nebu[0] < 0)
         cur_nebu[0] = cur_nebu[1] + 1;

      /* Change timer. */
      nebu_timer += nebu_dt;

      /* Case it hasn't rendered in a while so it doesn't go crazy. */
      if (nebu_timer < 0)
         nebu_timer = nebu_dt;
   }

   /* Set the colour */
   col[0] = cBlue.r;
   col[1] = cBlue.g;
   col[2] = cBlue.b;
   col[3] = (nebu_dt - nebu_timer) / nebu_dt;

   /* Set up the targets */
   /* Texture 0 */
   nglActiveTexture( GL_TEXTURE0 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[1]]);
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
   /* Texture 1 */
   nglActiveTexture( GL_TEXTURE1 );
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, nebu_textures[cur_nebu[0]]);

   /* Prepare it */
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_REPLACE );
   glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_INTERPOLATE );
   /* Colour */
   glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col );

   /* Arguments */
   /* Arg0 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB,    GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB,   GL_SRC_COLOR );
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,  GL_TEXTURE );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
   /* Arg1 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA,  GL_PREVIOUS );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
   /* Arg2 */
   glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA,  GL_CONSTANT );
   glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA );

   /* Compensate possible rumble */
   if (!paused) {
      spfx_getShake( &sx, &sy );
      gl_matrixPush();
         gl_matrixTranslate( -sx, -sy );
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
/**
 * @brief Regenerates the overlay.
 */
void nebu_genOverlay (void)
{
   int i;
   GLfloat *data;
   double a;
   double gx, gy;
   double z;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Calculate zoom. */
   z = 1./conf.zoom_far;

   /* See if need to generate overlay. */
   if (nebu_vboOverlay == NULL) {
      nebu_vboOverlay = gl_vboCreateStatic( sizeof(GLfloat) *
            ((2+4)*18 + 2*28 + 4*7), NULL );

      /* Set colors, those will be pure static. */
      data = gl_vboMap( nebu_vboOverlay );

      /* Alpha overlay. */
      for (i=0; i<18; i++) {
         data[2*18 + 4*i + 0] = cDarkBlue.r;
         data[2*18 + 4*i + 1] = cDarkBlue.g;
         data[2*18 + 4*i + 2] = cDarkBlue.b;
         data[2*18 + 4*i + 3] = cDarkBlue.a;
      }
      data[2*18 + 3] = 0.; /* Origin is transparent. */

      /* Solid overlay. */
      for (i=0; i<7; i++) {
         data[(2+4)*18 + 2*28 + 4*i + 0] = cDarkBlue.r;
         data[(2+4)*18 + 2*28 + 4*i + 1] = cDarkBlue.g;
         data[(2+4)*18 + 2*28 + 4*i + 2] = cDarkBlue.b;
         data[(2+4)*18 + 2*28 + 4*i + 3] = cDarkBlue.a;
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
 * @brief Renders the nebula overlay (hides what player can't see).
 *
 *    @param dt Current delta tick.
 */
void nebu_renderOverlay( const double dt )
{
   double gx, gy;
   double ox, oy;
   double z;
   double sx, sy;

   /* Get GUI offsets. */
   gui_getOffset( &gx, &gy );

   /* Get zoom. */
   z = cam_getZoom();

   /*
    * Renders the puffs
    */
   nebu_renderPuffs( dt, 0 );

   /* Prepare the matrix */
   ox = gx;
   oy = gy;
   if (!paused) {
      spfx_getShake( &sx, &sy );
      ox += sx;
      oy += sy;
   }
   gl_matrixPush();
      gl_matrixTranslate( SCREEN_W/2.+ox, SCREEN_H/2.+oy );
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
    * Solid nebula for areas the player can't see
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
         if (!paused && (player.p!=NULL)) {
            nebu_puffs[i].x -= player.p->solid->vel.x * nebu_puffs[i].height * dt;
            nebu_puffs[i].y -= player.p->solid->vel.y * nebu_puffs[i].height * dt;
         }

         /* Check boundries */
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
 * @brief Prepares the nebualae to be rendered.
 *
 *    @param density Density of the nebula (0-1000).
 *    @param volatility Volatility of the nebula (0-1000).
 */
void nebu_prep( double density, double volatility )
{
   (void)volatility;
   int i;

   nebu_view = 1000. - density;  /* At density 1000 you're blind */
   nebu_dt   = 2000. / (density + 100.); /* Faster at higher density */
   nebu_timer = nebu_dt;

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
 * @brief Forces generation of new nebula on init.
 */
void nebu_forceGenerate (void)
{
   nebu_w = nebu_h = -9; /* \o/ magic numbers */
}


/**
 * @brief Generates the nebula.
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
   loadscreen_render( 0.05, "Generating Nebula (slow, run once)..." );

   /* Get resolution to create at. */
   w = SCREEN_W;
   h = SCREEN_H;

   /* Try to make the dir first if it fails. */
   nfile_dirMakeExist( "%s"NEBULA_DIR, nfile_basePath() );

   /* Generate all the nebula backgrounds */
   nebu = noise_genNebulaMap( w, h, NEBULA_Z, 5. );

   /* Start saving - compression can take a bit. */
   loadscreen_render( 0.05, "Compressing Nebula layers..." );

   /* Save each nebula as an image */
   for (i=0; i<NEBULA_Z; i++) {
      snprintf( nebu_file, PATH_MAX, NEBULA_PATH_BG, w, h, i );
      ret = saveNebula( &nebu[ i*w*h ], w, h, nebu_file );
      if (ret != 0) break; /* An error has happenend */
   }

   /* Cleanup */
   free(nebu);
   return ret;
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

   /* Warn user of what is happening. */
   loadscreen_render( 0.05, "Generating Nebula Puffs..." );

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
 * @brief Checks the validity of a nebula.
 *
 *    @param file Path of the nebula to check (relative to base directory).
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
 * @brief Saves a nebula.
 *
 *    @param map Nebula map to save.
 *    @param w Width of nebula map.
 *    @param h Height of nebula map.
 *    @param file Path to save into.
 *    @return 0 on success.
 */
static int saveNebula( float *map, const uint32_t w, const uint32_t h, const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;
   int ret;

   /* fix surface */
   sur = nebu_surfaceFromNebulaMap( map, w, h );

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
 *    @param file Path of the nebula to load.  Relative to base directory.
 *    @return A SDL surface with the nebula.
 */
static SDL_Surface* loadNebula( const char* file )
{
   char file_path[PATH_MAX];
   SDL_Surface* sur;
   SDL_RWops *rw;
   npng_t *npng;

   /* loads the file */
   snprintf(file_path, PATH_MAX, "%s%s", nfile_basePath(), file );
   rw    = SDL_RWFromFile( file_path, "rb" );;
   npng  = npng_open( rw );
   sur   = npng_readSurface( npng, 0, 1 );
   npng_close( npng );
   SDL_RWclose( rw );
   if (sur == NULL) {
      ERR("Unable to load Nebula image: %s", file);
      return NULL;
   }

   return sur;
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
