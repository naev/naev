/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file spfx.c
 *
 * @brief Handles the special effects.
 */


#include "spfx.h"

#include "naev.h"

#include <inttypes.h>

#include "SDL.h"
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_haptic.h"
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

#include "log.h"
#include "pilot.h"
#include "physics.h"
#include "opengl.h"
#include "pause.h"
#include "rng.h"
#include "ndata.h"
#include "nxml.h"
#include "debris.h"
#include "perlin.h"


#define SPFX_XML_ID     "spfxs" /**< XML Document tag. */
#define SPFX_XML_TAG    "spfx" /**< SPFX XML node tag. */

#define SPFX_GFX_SUF    ".png" /**< Suffix of graphics. */

#define SPFX_CHUNK_MAX  16384 /**< Maximum chunk to alloc when needed */
#define SPFX_CHUNK_MIN  256 /**< Minimum chunk to alloc when needed */

#define SHAKE_MASS      (1./400.) /** Shake mass. */
#define SHAKE_K         (1./50.) /**< Constant for virtual spring. */
#define SHAKE_B         (3.*sqrt(SHAKE_K*SHAKE_MASS)) /**< Constant for virtual dampener. */

#define HAPTIC_UPDATE_INTERVAL   0.1 /**< Time between haptic updates. */


/*
 * special hard-coded special effects
 */
/* shake aka rumble */
static int shake_set = 0; /**< Is shake set? */
static Vector2d shake_pos = { .x = 0., .y = 0. }; /**< Current shake position. */
static Vector2d shake_vel = { .x = 0., .y = 0. }; /**< Current shake velocity. */
static double shake_force_mod = 0.; /**< Shake force modifier. */
static float shake_force_ang = 0.; /**< Shake force angle. */
static int shake_off = 1; /**< 1 if shake is not active. */
static perlin_data_t *shake_noise = NULL; /**< Shake noise. */
static const double shake_fps_min   = 1./10.; /**< Minimum fps to run shake update at. */


#if SDL_VERSION_ATLEAST(1,3,0)
extern SDL_Haptic *haptic; /**< From joystick.c */
extern unsigned int haptic_query; /**< From joystick.c */
static int haptic_rumble         = -1; /**< Haptic rumble effect ID. */
static SDL_HapticEffect haptic_rumbleEffect; /**< Haptic rumble effect. */
static double haptic_lastUpdate  = 0.; /**< Timer to update haptic effect again. */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */


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
/* General. */
static int spfx_base_parse( SPFX_Base *temp, const xmlNodePtr parent );
static void spfx_base_free( SPFX_Base *effect );
static void spfx_destroy( SPFX *layer, int *nlayer, int spfx );
static void spfx_update_layer( SPFX *layer, int *nlayer, const double dt );
/* Haptic. */
static int spfx_hapticInit (void);
static void spfx_hapticRumble( double mod );


/**
 * @brief Parses an xml node containing a SPFX.
 *
 *    @param temp Address to load SPFX into.
 *    @param parent XML Node containing the SPFX data.
 *    @return 0 on success.
 */
static int spfx_base_parse( SPFX_Base *temp, const xmlNodePtr parent )
{
   xmlNodePtr node;

   /* Clear data. */
   memset( temp, 0, sizeof(SPFX_Base) );

   /* Get the name (mallocs). */
   temp->name = xml_nodeProp(parent,"name");

   /* Extract the data. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      xmlr_float(node, "anim", temp->anim);
      xmlr_float(node, "ttl", temp->ttl);
      if (xml_isNode(node,"gfx")) {
         temp->gfx = xml_parseTexture( node,
               SPFX_GFX_PATH"%s"SPFX_GFX_SUF, 6, 5, 0 );
         continue;
      }
      WARN("SPFX '%s' has unknown node '%s'.", temp->name, node->name);
   } while (xml_nextNode(node));

   /* Convert from ms to s. */
   temp->anim /= 1000.;
   temp->ttl  /= 1000.;
   if (temp->ttl == 0.)
      temp->ttl = temp->anim;

#define MELEMENT(o,s) \
   if (o) WARN("SPFX '%s' missing/invalid '"s"' element", temp->name) /**< Define to help check for data errors. */
   MELEMENT(temp->anim==0.,"anim");
   MELEMENT(temp->ttl==0.,"ttl");
   MELEMENT(temp->gfx==NULL,"gfx");
#undef MELEMENT

   return 0;
}


/**
 * @brief Frees a SPFX_Base.
 *
 *    @param effect SPFX_Base to free.
 */
static void spfx_base_free( SPFX_Base *effect )
{
   if (effect->name != NULL) {
      free(effect->name);
      effect->name = NULL;
   }
   if (effect->gfx != NULL) {
      gl_freeTexture(effect->gfx);
      effect->gfx = NULL;
   }
}


/**
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
   return -1;
}


/**
 * @brief Loads the spfx stack.
 *
 *    @return 0 on success.
 *
 * @todo Make spfx not hard-coded.
 */
int spfx_load (void)
{
   int mem;
   uint32_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Load and read the data. */
   buf = ndata_read( SPFX_DATA_PATH, &bufsize );
   doc = xmlParseMemory( buf, bufsize );

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,SPFX_XML_ID)) {
      ERR("Malformed '"SPFX_DATA_PATH"' file: missing root element '"SPFX_XML_ID"'");
      return -1;
   }

   /* Check to see if is populated. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"SPFX_DATA_PATH"' file: does not contain elements");
      return -1;
   }

   /* First pass, loads up ammunition. */
   mem = 0;
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,SPFX_XML_TAG)) {

         spfx_neffects++;
         if (spfx_neffects > mem) {
            if (mem == 0)
               mem = SPFX_CHUNK_MIN;
            else
               mem *= 2;
            spfx_effects = realloc(spfx_effects, sizeof(SPFX_Base)*mem);
         }
         spfx_base_parse( &spfx_effects[spfx_neffects-1], node );
      }
      else
         WARN("'"SPFX_DATA_PATH"' has unknown node '%s'.", node->name);
   } while (xml_nextNode(node));
   /* Shrink back to minimum - shouldn't change ever. */
   spfx_effects = realloc(spfx_effects, sizeof(SPFX_Base) * spfx_neffects);

   /* Clean up. */
   xmlFreeDoc(doc);
   free(buf);


   /*
    * Now initialize force feedback.
    */
   spfx_hapticInit();
   shake_noise = noise_new( 1, NOISE_DEFAULT_HURST, NOISE_DEFAULT_LACUNARITY );

   return 0;
}


/**
 * @brief Frees the spfx stack.
 */
void spfx_free (void)
{
   int i;

   /* Clean up the debris. */
   debris_cleanup();

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

   /* Free the noise. */
   noise_delete( shake_noise );
}


/**
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
         if (spfx_mstack_front == 0)
            spfx_mstack_front = SPFX_CHUNK_MIN;
         else
            spfx_mstack_front += MIN( spfx_mstack_front, SPFX_CHUNK_MAX );
         spfx_stack_front = realloc( spfx_stack_front, spfx_mstack_front*sizeof(SPFX) );
      }
      cur_spfx = &spfx_stack_front[spfx_nstack_front];
      spfx_nstack_front++;
   }
   else if (layer == SPFX_LAYER_BACK) { /* back layer */
      if (spfx_mstack_back < spfx_nstack_back+1) { /* need more memory */
         if (spfx_mstack_back == 0)
            spfx_mstack_back = SPFX_CHUNK_MIN;
         else
            spfx_mstack_back += MIN( spfx_mstack_back, SPFX_CHUNK_MAX );
         spfx_stack_back = realloc( spfx_stack_back, spfx_mstack_back*sizeof(SPFX) );
      }
      cur_spfx = &spfx_stack_back[spfx_nstack_back];
      spfx_nstack_back++;
   }
   else {
      WARN("Invalid SPFX layer.");
      return;
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
   shake_set = 0;
   shake_off = 1;
   shake_force_mod = 0.;
   vectnull( &shake_pos );
   vectnull( &shake_vel );
}

/**
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
 * @brief Updates an individual spfx.
 *
 *    @param layer Layer the spfx is on.
 *    @param nlayer Pointer to the associated nlayer.
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
 * @brief Updates the shake position.
 */
static void spfx_updateShake( double dt )
{
   double mod, vmod, angle;
   double force_x, force_y;
   int forced;

   /* Must still be on. */
   if (shake_off)
      return;

   /* The shake decays over time */
   forced = 0;
   if (shake_force_mod > 0.) {
      shake_force_mod -= SHAKE_DECAY*dt;
      if (shake_force_mod < 0.)
         shake_force_mod   = 0.;
      else
         forced            = 1;
   }

   /* See if it's settled down. */
   mod      = VMOD( shake_pos );
   vmod     = VMOD( shake_vel );
   if (!forced && (mod < 0.01) && (vmod < 0.01)) {
      shake_off      = 1;
      if (shake_force_ang > 1e3)
         shake_force_ang = RNGF();
      return;
   }

   /* Calculate force. */
   force_x  = -SHAKE_K*shake_pos.x + -SHAKE_B*shake_vel.x;
   force_y  = -SHAKE_K*shake_pos.y + -SHAKE_B*shake_vel.y;

   /* Apply force if necessary. */
   if (forced) {
      shake_force_ang  += dt;
      angle             = noise_simplex1( shake_noise, &shake_force_ang ) * 5.*M_PI;
      force_x          += shake_force_mod * cos(angle);
      force_y          += shake_force_mod * sin(angle);
   }


   /* Update velocity. */
   vect_cadd( &shake_vel, (1./SHAKE_MASS) * force_x * dt, (1./SHAKE_MASS) * force_y * dt );

   /* Update position. */
   vect_cadd( &shake_pos, shake_vel.x * dt, shake_vel.y * dt );
}


/**
 * @brief Prepares the rendering for the special effects.
 *
 * Should be called at the beginning of the rendering loop.
 *
 *    @param dt Current delta tick.
 *    @param real_dt Real delta tick.
 */
void spfx_begin( const double dt, const double real_dt )
{
   double ddt;

   /* Defaults. */
   shake_set = 0;
   if (shake_off)
      return;

#if SDL_VERSION_ATLEAST(1,3,0)
   /* Decrement the haptic timer. */
   if (haptic_lastUpdate > 0.)
      haptic_lastUpdate -= real_dt; /* Based on real delta-tick. */
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   (void) real_dt; /* Avoid warning. */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* Micro basic simple control loop. */
   if (dt > shake_fps_min) {
      ddt = dt;
      while (ddt > shake_fps_min) {
         spfx_updateShake( shake_fps_min );
         ddt -= shake_fps_min;
      }
      spfx_updateShake( ddt ); /* Leftover. */
   }
   else
      spfx_updateShake( dt );

   /* set the new viewport */
   gl_matrixTranslate( shake_pos.x, shake_pos.y );
   shake_set = 1;
}


/**
 * @brief Indicates the end of the spfx loop.
 *
 * Should be called before the HUD.
 */
void spfx_end (void)
{
   /* Save cycles. */
   if (shake_set == 0)
      return;

   /* set the new viewport */
   gl_defViewport();
}


/**
 * @brief Increases the current rumble level.
 *
 * Rumble will decay over time.
 *
 *    @param mod Modifier to increase level by.
 */
void spfx_shake( double mod )
{
   /* Add the modifier. */
   shake_force_mod += mod;
   if (shake_force_mod  > SHAKE_MAX)
      shake_force_mod = SHAKE_MAX;

   /* Rumble if it wasn't rumbling before. */
   spfx_hapticRumble(mod);

   /* Notify that rumble is active. */
   shake_off = 0;
}


/**
 * @brief Gets the current shake position.
 *
 *    @param[out] X X shake position.
 *    @param[out] Y Y shake position.
 */
void spfx_getShake( double *x, double *y )
{
   if (shake_off) {
      *x = 0.;
      *y = 0.;
   }
   else {
      *x = shake_pos.x;
      *y = shake_pos.y;
   }
}


/**
 * @brief Initializes the rumble effect.
 *
 *    @return 0 on success.
 */
static int spfx_hapticInit (void)
{
#if SDL_VERSION_ATLEAST(1,3,0)
   SDL_HapticEffect *efx;

   /* Haptic must be enabled. */
   if (haptic == NULL)
      return 0;

   efx = &haptic_rumbleEffect;
   memset( efx, 0, sizeof(SDL_HapticEffect) );
   efx->type = SDL_HAPTIC_SINE;
   efx->periodic.direction.type   = SDL_HAPTIC_POLAR;
   efx->periodic.length           = 1000;
   efx->periodic.period           = 200;
   efx->periodic.magnitude        = 0x4000;
   efx->periodic.fade_length      = 1000;
   efx->periodic.fade_level       = 0;

   haptic_rumble = SDL_HapticNewEffect( haptic, efx );
   if (haptic_rumble < 0) {
      WARN("Unable to upload haptic effect: %s.", SDL_GetError());
      return -1;
   }
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   return 0;
}


/**
 * @brief Runs a rumble effect.
 *
 *    @brief Current modifier being added.
 */
static void spfx_hapticRumble( double mod )
{
#if SDL_VERSION_ATLEAST(1,3,0)
   SDL_HapticEffect *efx;
   double len, mag;

   if (haptic_rumble >= 0) {

      /* Not time to update yet. */
      if ((haptic_lastUpdate > 0.) || shake_off || (mod > SHAKE_MAX/3.))
         return;

      /* Stop the effect if it was playing. */
      SDL_HapticStopEffect( haptic, haptic_rumble );

      /* Get length and magnitude. */
      len = 1000. * shake_force_mod / SHAKE_DECAY;
      mag = 32767. * (shake_force_mod / SHAKE_MAX);

      /* Update the effect. */
      efx = &haptic_rumbleEffect;
      efx->periodic.magnitude    = (int16_t)mag;
      efx->periodic.length       = (uint32_t)len;
      efx->periodic.fade_length  = MIN( efx->periodic.length, 1000 );
      if (SDL_HapticUpdateEffect( haptic, haptic_rumble, &haptic_rumbleEffect ) < 0) {
         WARN("Failed to update haptic effect: %s.", SDL_GetError());
         return;
      }

      /* Run the new effect. */
      SDL_HapticRunEffect( haptic, haptic_rumble, 1 );

      /* Set timer again. */
      haptic_lastUpdate += HAPTIC_UPDATE_INTERVAL;
   }
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   (void) mod;
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
}


/**
 * @brief Sets the cinematic mode.
 *
 * Should be run at the end of the render loop if needed.
 */
void spfx_cinematic (void)
{
   gl_renderRect( 0., 0.,           SCREEN_W, SCREEN_H*0.2, &cBlack );
   gl_renderRect( 0., SCREEN_H*0.8, SCREEN_W, SCREEN_H,     &cBlack );
}


/**
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

      default:
         WARN("Rendering invalid SPFX layer.");
         return;
   }

   /* Now render the layer */
   for (i=spfx_nstack-1; i>=0; i--) {
      effect = &spfx_effects[ spfx_stack[i].effect ];

      /* Simplifies */
      sx = (int)effect->gfx->sx;
      sy = (int)effect->gfx->sy;

      if (!paused) { /* don't calculate frame if paused */
         time = 1. - fmod(spfx_stack[i].timer,effect->anim) / effect->anim;
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

