/*
 * See Licensing and Copyright notice in naev.h
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


#define SPFX_GFX     "gfx/spfx/" /* location of the graphic */

#define SPFX_CHUNK   32 /* chunk to alloc when needed */


/*
 * special hardcoded special effects
 */
/* shake aka rumble */
static double shake_rad = 0.;
Vector2d shake_pos = { .x = 0., .y = 0. }; /* used in nebulae.c */
static Vector2d shake_vel = { .x = 0., .y = 0. };
static int shake_off = 1;


/*
 * generic SPFX template
 */
typedef struct SPFX_Base_ {
   char* name;

   double ttl; /* Time to live */
   double anim; /* Total duration in ms */

   glTexture *gfx; /* will use each sprite as a frame */
} SPFX_Base;

static SPFX_Base *spfx_effects = NULL;
static int spfx_neffects = 0;


typedef struct SPFX_ {
   Vector2d pos, vel; /* they don't accelerate */

   int lastframe; /* needed when paused */
   int effect; /* the real effect */

   double timer; /* time left */
} SPFX;


/* front stack is for effects on player, back is for the rest */
static SPFX *spfx_stack_front = NULL;
static int spfx_nstack_front = 0;
static int spfx_mstack_front = 0;
static SPFX *spfx_stack_back = NULL;
static int spfx_nstack_back = 0;
static int spfx_mstack_back = 0;


/*
 * prototypes
 */
static int spfx_base_load( char* name, int ttl, int anim, char* gfx, int sx, int sy );
static void spfx_base_free( SPFX_Base *effect );
static void spfx_destroy( SPFX *layer, int *nlayer, int spfx );
static void spfx_update_layer( SPFX *layer, int *nlayer, const double dt );


/*
 * loads the SPFX_Base
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


/*
 * frees the SPFX_Base
 */
static void spfx_base_free( SPFX_Base *effect )
{
   if (effect->name != NULL) free(effect->name);
   if (effect->gfx != NULL) gl_freeTexture(effect->gfx);
}


int spfx_get( char* name )
{
   int i;
   for (i=0; i<spfx_neffects; i++)
      if (strcmp(spfx_effects[i].name, name)==0)
         return i;
   WARN("SPFX '%s' not found!", name );
   return 0;
}


/*
 * load and unload functions
 * TODO make it customizeable?
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


/*
 * adds a new spfx to layer
 */
void spfx_add( int effect,
      const double px, const double py,
      const double vx, const double vy,
      const int layer )
{
   SPFX *cur_spfx;
   double ttl, anim;

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


/*
 * clears the current particles from the stacks
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
   shake_rad = 0;
}

/*
 * destroys an active spfx
 */
static void spfx_destroy( SPFX *layer, int *nlayer, int spfx )
{
   (*nlayer)--;
   memmove( &layer[spfx], &layer[spfx+1], (*nlayer-spfx)*sizeof(SPFX) );
}


/*
 * updates all the spfx
 */
void spfx_update( const double dt )
{
   spfx_update_layer( spfx_stack_front, &spfx_nstack_front, dt );
   spfx_update_layer( spfx_stack_back, &spfx_nstack_back, dt );
}


/*
 * updates a spfx
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


/*
 * prepares the rendering for special effects
 */
void spfx_start( double dt )
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
            vect_pset( &shake_vel, shake_rad, 
                  -VANGLE(shake_pos) + (RNGF()-0.5) * M_PI );
         }

         /* the shake decays over time */
         shake_rad -= SHAKE_DECAY*dt ;
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


/*
 * adds ruuuuuuuuumble!
 */
void spfx_shake( double mod )
{
   shake_rad += mod/5.;
   if (shake_rad > SHAKE_MAX) shake_rad = SHAKE_MAX;
   shake_off = 0;

   vect_pset( &shake_vel, shake_rad, RNGF() * 2. * M_PI );
}


/*
 * creates cinematic mode, should be run last
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


/*
 * renders all the spfx
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
   for (i=0; i<spfx_nstack; i++) {
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

