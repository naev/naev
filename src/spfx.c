/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file spfx.c
 *
 * @brief Handles the special effects.
 */


#include "spfx.h"

#include "SDL.h"

#include "naev.h"
#include "log.h"
#include "pilot.h"
#include "physics.h"
#include "opengl.h"
#include "pause.h"
#include "rng.h"


#define SPFX_GFX        "gfx/spfx/" /**< location of the graphic */

#define SPFX_CHUNK      32 /**< chunk to alloc when needed */

#define SHAKE_VEL_MOD   0.0012 /**< Shake modifier. */
#define SHAKE_MOD_FACTOR 0.2 /**< Shake modifier factor. */


/*
 * special hardcoded special effects
 */
/* shake aka rumble */
static double shake_rad = 0.; /**< Current shake radius (0 = no shake). */
Vector2d shake_pos = { .x = 0., .y = 0. }; /**< Current shake position. Used in nebulae.c */
static Vector2d shake_vel = { .x = 0., .y = 0. }; /**< Current shake velocity. */
static int shake_off = 1; /**< 1 if shake is not active. */


/**
 * @struct SPFX_Base
 *
 * @brief Generic special effect.
 */
typedef struct SPFX_Base_ {
   char* name; /**< Name of the special effect. */

   double ttl; /**< Time to live */
   double anim; /**< Total duration in ms */

   glTexture *gfx; /**< will use each sprite as a frame */
} SPFX_Base;

static SPFX_Base *spfx_effects = NULL; /**< Total special effects. */
static int spfx_neffects = 0; /**< Total number of special effects. */


/**
 * @struct SPFX
 *
 * @brief An actual in-game active special effect.
 */
typedef struct SPFX_ {
   Vector2d pos; /**< Current position. */
   Vector2d vel; /**< Current velocity. */

   int lastframe; /**< Needed when paused */
   int effect; /**< The real effect */

   double timer; /**< Time left */
} SPFX;


/* front stack is for effects on player, back is for the rest */
static SPFX *spfx_stack_front = NULL; /**< Frontal special effect layer. */
static int spfx_nstack_front = 0; /**< Number of special effects in front. */
static int spfx_mstack_front = 0; /**< Memory allocated for frontal special effects. */
static SPFX *spfx_stack_back = NULL; /**< Back special effect layer. */
static int spfx_nstack_back = 0; /**< Number of special effects in back. */
static int spfx_mstack_back = 0; /**< Memory allocated for special effects in back. */


/*
 * prototypes
 */
static int spfx_base_load( char* name, int ttl, int anim, char* gfx, int sx, int sy );
static void spfx_base_free( SPFX_Base *effect );
static void spfx_destroy( SPFX *layer, int *nlayer, int spfx );
static void spfx_update_layer( SPFX *layer, int *nlayer, const double dt );


/**
 * @fn static int spfx_base_load( char* name, int ttl, int anim, char* gfx, int sx, int sy )
 *
 * @brief Loads a SPFX_Base into the stack based on some parameters.
 *
 *    @param name Name of the spfx.
 *    @param ttl Time to live of the spfx.
 *    @param anim Total duration in ms of the animation.
 *    @param gfx Name of the graphic effect to use.
 *    @param sx Number of x sprites in the graphic.
 *    @param sy Number of y sprites in the graphic.
 *    @return 0 on success.
 */
static int spfx_base_load( char* name, int ttl, int anim, char* gfx, int sx, int sy )
{
   SPFX_Base *cur;
   char buf[PATH_MAX];

   /* Create new effect */
   spfx_effects = realloc( spfx_effects, ++spfx_neffects*sizeof(SPFX_Base) );
   cur = &spfx_effects[spfx_neffects-1];

   /* Fill it with ze data */
   cur->name = strdup(name);
   cur->anim = (double)anim / 1000.;
   cur->ttl = (double)ttl / 1000.;
   sprintf(buf, SPFX_GFX"%s", gfx);
   cur->gfx = gl_newSprite( buf, sx, sy );

   return 0;
}


/**
 * @fn static void spfx_base_free( SPFX_Base *effect )
 *
 * @brief Frees a SPFX_Base.
 *
 *    @param effect SPFX_Base to free.
 */
static void spfx_base_free( SPFX_Base *effect )
{
   if (effect->name != NULL) free(effect->name);
   if (effect->gfx != NULL) gl_freeTexture(effect->gfx);
}


/**
 * @fn int spfx_get( char* name )
 *
 * @brief Gets the id of an spfx based on name.
 *
 *    @param name Name to match.
 *    @return ID of the special effect or -1 on error.
 */
int spfx_get( char* name )
{
   int i;
   for (i=0; i<spfx_neffects; i++)
      if (strcmp(spfx_effects[i].name, name)==0)
         return i;
   WARN("SPFX '%s' not found!", name );
   return -1;
}


/**
 * @fn int spfx_load (void)
 *
 * @brief Loads the spfx stack.
 *
 *    @return 0 on success.
 *
 * @todo Make spfx not hardcoded.
 */
int spfx_load (void)
{
   /* Standard explosion effects */
   spfx_base_load( "ExpS", 400, 400, "exps.png", 6, 5 );
   spfx_base_load( "ExpM", 450, 450, "expm.png", 6, 5 );
   spfx_base_load( "ExpL", 500, 500, "expl.png", 6, 5 );
   spfx_base_load( "cargo", 15000, 5000, "cargo.png", 6, 6 );

   return 0;
}


/**
 * @fn void spfx_free (void)
 *
 * @brief Frees the spfx stack.
 */
void spfx_free (void)
{
   int i;

   /* get rid of all the particles and free the stacks */
   spfx_clear();
   if (spfx_stack_front) free(spfx_stack_front);
   spfx_stack_front = NULL;
   spfx_mstack_front = 0;
   if (spfx_stack_back) free(spfx_stack_back);
   spfx_stack_back = NULL;
   spfx_mstack_back = 0;

   /* now clear the effects */
   for (i=0; i<spfx_neffects; i++)
      spfx_base_free( &spfx_effects[i] );
   free(spfx_effects);
   spfx_effects = NULL;
   spfx_neffects = 0;
}


/**
 * @fn void spfx_add( int effect,
 *       const double px, const double py,
 *       const double vx, const double vy,
 *       const int layer )
 *
 * @brief Creates a new special effect.
 *
 *    @param effect Base effect identifier to use.
 *    @param px X position of the effect.
 *    @param py Y position of the effect.
 *    @param vx X velocity of the effect.
 *    @param vy Y velocity of the effect.
 *    @param layer Layer to put the effect on.
 */
void spfx_add( int effect,
      const double px, const double py,
      const double vx, const double vy,
      const int layer )
{
   SPFX *cur_spfx;
   double ttl, anim;

   if ((effect < 0) || (effect > spfx_neffects)) {
      WARN("Trying to add spfx with invalid effect!");
      return;
   }

   /*
    * Select the Layer
    */
   if (layer == SPFX_LAYER_FRONT) { /* front layer */
      if (spfx_mstack_front < spfx_nstack_front+1) { /* need more memory */
         spfx_mstack_front += SPFX_CHUNK;
         spfx_stack_front = realloc( spfx_stack_front, spfx_mstack_front*sizeof(SPFX) );
      }
      cur_spfx = &spfx_stack_front[spfx_nstack_front];
      spfx_nstack_front++;
   }
   else if (layer == SPFX_LAYER_BACK) { /* back layer */
      if (spfx_mstack_back < spfx_nstack_back+1) { /* need more memory */
         spfx_mstack_back += SPFX_CHUNK;
         spfx_stack_back = realloc( spfx_stack_back, spfx_mstack_back*sizeof(SPFX) );
      }
      cur_spfx = &spfx_stack_back[spfx_nstack_back];
      spfx_nstack_back++;
   }

   /* The actual adding of the spfx */
   cur_spfx->effect = effect;
   vect_csetmin( &cur_spfx->pos, px, py );
   vect_csetmin( &cur_spfx->vel, vx, vy );
   /* Timer magic if ttl != anim */
   ttl = spfx_effects[effect].ttl;
   anim = spfx_effects[effect].anim;
   if (ttl != anim)
      cur_spfx->timer = ttl + RNGF()*anim;
   else
      cur_spfx->timer = ttl;
}


/**
 * @fn void spfx_clear (void)
 *
 * @brief Clears all the currently running effects.
 */
void spfx_clear (void)
{
   int i;

   /* Clear front layer */
   for (i=spfx_nstack_front-1; i>=0; i--)
      spfx_destroy( spfx_stack_front, &spfx_nstack_front, i );

   /* Clear back layer */
   for (i=spfx_nstack_back-1; i>=0; i--)
      spfx_destroy( spfx_stack_back, &spfx_nstack_back, i );

   /* Clear rumble */
   shake_rad = 0.;
   shake_pos.x = shake_pos.y = 0.;
   shake_vel.x = shake_vel.y = 0.;
}

/**
 * @fn static void spfx_destroy( SPFX *layer, int *nlayer, int spfx )
 *
 * @brief Destroys an active spfx.
 *
 *    @param layer Layer the spfx is on.
 *    @param nlayer Pointer to the number of elements in the layer.
 *    @param spfx Position of the spfx in the stack.
 */
static void spfx_destroy( SPFX *layer, int *nlayer, int spfx )
{
   (*nlayer)--;
   memmove( &layer[spfx], &layer[spfx+1], (*nlayer-spfx)*sizeof(SPFX) );
}


/**
 * @fn void spfx_update( const double dt )
 *
 * @brief Updates all the spfx.
 *
 *    @param dt Current delta tick.
 */
void spfx_update( const double dt )
{
   spfx_update_layer( spfx_stack_front, &spfx_nstack_front, dt );
   spfx_update_layer( spfx_stack_back, &spfx_nstack_back, dt );
}


/**
 * @fn static void spfx_update_layer( SPFX *layer, int *nlayer, const double dt )
 *
 * @brief Updates an individual spfx.
 *
 *    @param layer Layer the spfx is on.
 *    @param nlayer Pointer to the assosciated nlayer.
 *    @param dt Current delta tick.
 */
static void spfx_update_layer( SPFX *layer, int *nlayer, const double dt )
{
   int i;

   for (i=0; i<*nlayer; i++) {
      layer[i].timer -= dt; /* less time to live */

      /* time to die! */
      if (layer[i].timer < 0.) {
         spfx_destroy( layer, nlayer, i );
         i--;
         continue;
      }

      /* actually update it */
      vect_cadd( &layer[i].pos, dt*VX(layer[i].vel), dt*VY(layer[i].vel) );
   }
}


/**
 * @fn void spfx_start( double dt )
 *
 * @brief Preperase the rendering for the special effects.
 *
 * Should be called at the beginning of the rendering loop.
 *
 *    @param dt Current delta tick.
 */
void spfx_start( const double dt )
{
   GLdouble bx, by, x, y;
   double inc;

   if (shake_off == 1) return; /* save the cycles! */

   /* set defaults */
   bx = SCREEN_W/2;
   by = SCREEN_H/2;

   if (!paused) {
      inc = dt*100000.;

      /* calculate new position */
      if (shake_rad > 0.01) {
         vect_cadd( &shake_pos, shake_vel.x * inc, shake_vel.y * inc );

         if (VMOD(shake_pos) > shake_rad) { /* change direction */
            vect_pset( &shake_pos, shake_rad, VANGLE(shake_pos) );
            vect_pset( &shake_vel, SHAKE_VEL_MOD*shake_rad, 
                  -VANGLE(shake_pos) + (RNGF()-0.5) * M_PI );
         }

         /* the shake decays over time */
         shake_rad -= SHAKE_DECAY*dt*SHAKE_MOD_FACTOR;
         if (shake_rad < 0.) shake_rad = 0.;

         x = shake_pos.x;
         y = shake_pos.y;  
      }
      else {
         shake_rad = 0.;
         shake_off = 1;
         x = 0.;
         y = 0.;
      }
   }

   /* set the new viewport */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho( -bx+x, bx+x, -by+y, by+y, -1., 1. );
}


/**
 * @fn void spfx_shake( double mod )
 *
 * @brief Increases the current rumble level.
 *
 * Rumble will decay over time.
 * 
 *    @param mod Modifier to increase level by.
 */
void spfx_shake( double mod )
{
   shake_rad += mod*SHAKE_MOD_FACTOR;
   if (shake_rad > SHAKE_MAX) shake_rad = SHAKE_MAX;
   shake_off = 0;

   vect_pset( &shake_vel, SHAKE_VEL_MOD*shake_rad, RNGF() * 2. * M_PI );
}


/**
 * @fn void spfx_cinematic (void)
 *
 * @brief Sets the cinematic mode.
 *
 * Should be run at the end of the render loop if needed.
 */
void spfx_cinematic (void)
{
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix(); /* translation matrix */
      glTranslated( -(double)SCREEN_W/2., -(double)SCREEN_H/2., 0);

   COLOUR(cBlack);
   glBegin(GL_QUADS);
      glVertex2d( 0.,       0.           );
      glVertex2d( 0.,       SCREEN_H*0.2 );
      glVertex2d( SCREEN_W, SCREEN_H*0.2 );
      glVertex2d( SCREEN_W, 0.           );
      glVertex2d( 0.,       SCREEN_H     );
      glVertex2d( SCREEN_W, SCREEN_H     );
      glVertex2d( SCREEN_W, SCREEN_H*0.8 );
      glVertex2d( 0.,       SCREEN_H*0.8 );
   glEnd(); /* GL_QUADS */

   glPopMatrix(); /* translation matrx */
}


/**
 * @fn void spfx_render( const int layer )
 *
 * @brief Renders the entire spfx layer.
 *
 *    @param layer Layer to render.
 */
void spfx_render( const int layer )
{
   SPFX *spfx_stack;
   int i, spfx_nstack;
   SPFX_Base *effect;
   int sx, sy;
   double time;

   
   /* get the appropriate layer */
   switch (layer) {
      case SPFX_LAYER_FRONT:
         spfx_stack = spfx_stack_front;
         spfx_nstack = spfx_nstack_front;
         break;

      case SPFX_LAYER_BACK:
         spfx_stack = spfx_stack_back;
         spfx_nstack = spfx_nstack_back;
         break;
   }

   /* Now render the layer */
   for (i=spfx_nstack-1; i>=0; i--) {
      effect = &spfx_effects[ spfx_stack[i].effect ];

      /* Simplifies */
      sx = (int)effect->gfx->sx;
      sy = (int)effect->gfx->sy;

      if (!paused) { /* don't calculate frame if paused */
         time = fmod(spfx_stack[i].timer,effect->anim) / effect->anim;
         spfx_stack[i].lastframe = sx * sy * MIN(time, 1.);
      }
      
      /* Renders */
      gl_blitSprite( effect->gfx, 
            VX(spfx_stack[i].pos), VY(spfx_stack[i].pos),
            spfx_stack[i].lastframe % sx,
            spfx_stack[i].lastframe / sx,
            NULL );
   }
}

