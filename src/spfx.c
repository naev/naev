/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file spfx.c
 *
 * @brief Handles the special effects.
 */


/** @cond */
#include <inttypes.h>
#include "SDL.h"
#include "SDL_haptic.h"

#include "naev.h"
/** @endcond */

#include "spfx.h"

#include "array.h"
#include "camera.h"
#include "debris.h"
#include "log.h"
#include "ndata.h"
#include "nxml.h"
#include "opengl.h"
#include "pause.h"
#include "perlin.h"
#include "physics.h"
#include "rng.h"
#include "space.h"


#define SPFX_XML_ID     "spfxs" /**< XML Document tag. */
#define SPFX_XML_TAG    "spfx" /**< SPFX XML node tag. */

#define SHAKE_MASS      (1./400.) /** Shake mass. */
#define SHAKE_K         (1./50.) /**< Constant for virtual spring. */
#define SHAKE_B         (3.*sqrt(SHAKE_K*SHAKE_MASS)) /**< Constant for virtual dampener. */

#define HAPTIC_UPDATE_INTERVAL   0.1 /**< Time between haptic updates. */


/* Trail stuff. */
#define TRAIL_UPDATE_DT       0.05
static TrailSpec* trail_spec_stack;
static Trail_spfx** trail_spfx_stack;


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

/* Haptic stuff. */
extern SDL_Haptic *haptic; /**< From joystick.c */
extern unsigned int haptic_query; /**< From joystick.c */
static int haptic_rumble         = -1; /**< Haptic rumble effect ID. */
static SDL_HapticEffect haptic_rumbleEffect; /**< Haptic rumble effect. */
static double haptic_lastUpdate  = 0.; /**< Timer to update haptic effect again. */


/*
 * Trail colours handling.
 */
static int trailSpec_load (void);
static void trailSpec_parse( xmlNodePtr cur, TrailSpec *tc );
static TrailSpec* trailSpec_getRaw( const char* name );


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
static SPFX *spfx_stack_middle = NULL; /**< Middle special effect layer. */
static SPFX *spfx_stack_back = NULL; /**< Back special effect layer. */


/*
 * prototypes
 */
/* General. */
static int spfx_base_parse( SPFX_Base *temp, const xmlNodePtr parent );
static void spfx_base_free( SPFX_Base *effect );
static void spfx_update_layer( SPFX *layer, const double dt );
/* Haptic. */
static int spfx_hapticInit (void);
static void spfx_hapticRumble( double mod );
/* Trail. */
static void spfx_update_trails( double dt );
static void spfx_trail_update( Trail_spfx* trail, double dt );
static void spfx_trail_draw( const Trail_spfx* trail );
static void spfx_trail_free( Trail_spfx* trail );


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

   xmlr_attr_strd( parent, "name", temp->name );

   /* Extract the data. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      xmlr_float(node, "anim", temp->anim);
      xmlr_float(node, "ttl", temp->ttl);
      if (xml_isNode(node,"gfx")) {
         temp->gfx = xml_parseTexture( node,
               SPFX_GFX_PATH"%s", 6, 5, 0 );
         continue;
      }
      WARN(_("SPFX '%s' has unknown node '%s'."), temp->name, node->name);
   } while (xml_nextNode(node));

   /* Convert from ms to s. */
   temp->anim /= 1000.;
   temp->ttl  /= 1000.;
   if (temp->ttl == 0.)
      temp->ttl = temp->anim;

#define MELEMENT(o,s) \
   if (o) WARN( _("SPFX '%s' missing/invalid '%s' element"), temp->name, s) /**< Define to help check for data errors. */
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
   free(effect->name);
   effect->name = NULL;
   gl_freeTexture(effect->gfx);
   effect->gfx = NULL;
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
   for (i=0; i<array_size(spfx_effects); i++)
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
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Load and read the data. */
   doc = xml_parsePhysFS( SPFX_DATA_PATH );
   if (doc == NULL)
      return -1;

   /* Check to see if document exists. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,SPFX_XML_ID)) {
      ERR( _("Malformed '%s' file: missing root element '%s'"), SPFX_DATA_PATH, SPFX_XML_ID);
      return -1;
   }

   /* Check to see if is populated. */
   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), SPFX_DATA_PATH);
      return -1;
   }

   /* First pass, loads up ammunition. */
   spfx_effects = array_create(SPFX_Base);
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,SPFX_XML_TAG)) {
         spfx_base_parse( &array_grow(&spfx_effects), node );
      }
      else
         WARN( _("'%s' has unknown node '%s'."), SPFX_DATA_PATH, node->name);
   } while (xml_nextNode(node));
   /* Shrink back to minimum - shouldn't change ever. */
   array_shrink(&spfx_effects);

   /* Clean up. */
   xmlFreeDoc(doc);

   /* Trail colour sets. */
   trailSpec_load();

   /*
    * Now initialize force feedback.
    */
   spfx_hapticInit();
   shake_noise = noise_new( 1, NOISE_DEFAULT_HURST, NOISE_DEFAULT_LACUNARITY );

   /* Stacks. */
   spfx_stack_front = array_create( SPFX );
   spfx_stack_middle = array_create( SPFX );
   spfx_stack_back = array_create( SPFX );

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
   array_free(spfx_stack_front);
   spfx_stack_front = NULL;
   array_free(spfx_stack_middle);
   spfx_stack_middle = NULL;
   array_free(spfx_stack_back);
   spfx_stack_back = NULL;

   /* now clear the effects */
   for (i=0; i<array_size(spfx_effects); i++)
      spfx_base_free( &spfx_effects[i] );
   array_free(spfx_effects);
   spfx_effects = NULL;

   /* Free the noise. */
   noise_delete( shake_noise );

   /* Free the trails. */
   for (i=0; i<array_size(trail_spfx_stack); i++)
      spfx_trail_free( trail_spfx_stack[i] );
   array_free( trail_spfx_stack );

   /* Free the trail styles. */
   for (i=0; i<array_size(trail_spec_stack); i++)
      free( trail_spec_stack[i].name );
   array_free( trail_spec_stack );
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

   if ((effect < 0) || (effect > array_size(spfx_effects))) {
      WARN(_("Trying to add spfx with invalid effect!"));
      return;
   }

   /*
    * Select the Layer
    */
   if (layer == SPFX_LAYER_FRONT) /* front layer */
      cur_spfx = &array_grow( &spfx_stack_front );
   else if (layer == SPFX_LAYER_MIDDLE) /* middle layer */
      cur_spfx = &array_grow( &spfx_stack_middle );
   else if (layer == SPFX_LAYER_BACK) /* back layer */
      cur_spfx = &array_grow( &spfx_stack_back );
   else {
      WARN(_("Invalid SPFX layer."));
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

   /* Clear rumble */
   shake_set = 0;
   shake_off = 1;
   shake_force_mod = 0.;
   vectnull( &shake_pos );
   vectnull( &shake_vel );
   for (i=0; i<array_size(trail_spfx_stack); i++)
      spfx_trail_free( trail_spfx_stack[i] );
   array_erase( &trail_spfx_stack, array_begin(trail_spfx_stack), array_end(trail_spfx_stack) );
}


/**
 * @brief Updates all the spfx.
 *
 *    @param dt Current delta tick.
 */
void spfx_update( const double dt )
{
   spfx_update_layer( spfx_stack_front, dt );
   spfx_update_layer( spfx_stack_middle, dt );
   spfx_update_layer( spfx_stack_back, dt );
   spfx_update_trails( dt );
}


/**
 * @brief Updates an individual spfx.
 *
 *    @param layer Layer the spfx is on.
 *    @param dt Current delta tick.
 */
static void spfx_update_layer( SPFX *layer, const double dt )
{
   int i;

   for (i=0; i<array_size(layer); i++) {
      layer[i].timer -= dt; /* less time to live */

      /* time to die! */
      if (layer[i].timer < 0.) {
         array_erase( &layer, &layer[i], &layer[i+1] );
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
 * @brief Initalizes a trail.
 *
 *    @return Pointer to initialized trail. When done (due e.g. to pilot death), call spfx_trail_remove.
 */
Trail_spfx* spfx_trail_create( const TrailSpec* spec )
{
   Trail_spfx *trail;

   trail = calloc( 1, sizeof(Trail_spfx) );
   trail->spec = spec;
   trail->capacity = 1;
   trail->iread = trail->iwrite = 0;
   trail->point_ringbuf = calloc( trail->capacity, sizeof(TrailPoint) );
   trail->refcount = 1;
   trail->r = RNGF();

   if ( trail_spfx_stack == NULL )
      trail_spfx_stack = array_create( Trail_spfx* );
   array_push_back( &trail_spfx_stack, trail );

   return trail;
}


/**
 * @brief Updates all trails (handling dispersal/fadeout).
 *
 *    @param dt Update interval.
 */
void spfx_update_trails( double dt ) {
   int i, n;
   Trail_spfx *trail;

   n = array_size( trail_spfx_stack );
   for (i=0; i<n; i++) {
      trail = trail_spfx_stack[i];
      spfx_trail_update( trail, dt );
      if (!trail->refcount && !trail_size(trail) ) {
         spfx_trail_free( trail );
         trail_spfx_stack[i--] = trail_spfx_stack[--n];
      }
   }
   if (n < array_size( trail_spfx_stack ) )
      array_resize( &trail_spfx_stack, n );
}


/**
 * @brief Updates a trail.
 *
 *    @param trail Trail to update.
 *    @param dt Update interval.
 */
static void spfx_trail_update( Trail_spfx* trail, double dt )
{
   size_t i;
   GLfloat rel_dt;

   rel_dt = dt/ trail->spec->ttl;
   /* Remove outdated elements. */
   while (trail->iread < trail->iwrite && trail_front(trail).t < rel_dt)
      trail->iread++;

   /* Update others' timestamps. */
   for (i = trail->iread; i < trail->iwrite; i++)
      trail_at( trail, i ).t -= rel_dt;

   /* Update timer. */
   trail->dt += dt;
}


/**
 * @brief Makes a trail grow.
 *
 *    @param trail Trail to update.
 *    @param x X position of the new control point.
 *    @param y Y position of the new control point.
 *    @param mode Type of trail emission at this point.
 */
void spfx_trail_sample( Trail_spfx* trail, double x, double y, TrailMode mode )
{
   TrailPoint p;

   if (trail->spec->style[mode].col.a <= 0.)
      return;

   p.x = x;
   p.y = y;
   p.t = 1.;
   p.mode = mode;

   /* The "back" of the trail should always reflect our most recent state.  */
   trail_back( trail ) = p;

   /* We may need to insert a control point, but not if our last sample was recent enough. */
   if (trail_size(trail) > 1 && trail_at( trail, trail->iwrite-2 ).t >= 1.-TRAIL_UPDATE_DT)
      return;

   /* If the last time we inserted a control point was recent enough, we don't need a new one. */
   if (trail_size(trail) == trail->capacity) {
      /* Full! Double capacity, and make the elements contiguous. (We've made space to grow rightward.) */
      trail->point_ringbuf = realloc( trail->point_ringbuf, 2 * trail->capacity * sizeof(TrailPoint) );
      trail->iread %= trail->capacity;
      trail->iwrite = trail->iread + trail->capacity;
      memmove( &trail->point_ringbuf[trail->capacity], trail->point_ringbuf, trail->iread * sizeof(TrailPoint) );
      trail->capacity *= 2;
   }
   trail_at( trail, trail->iwrite++ ) = p;
}


/**
 * @brief Removes a trail.
 *
 *    @param trail Trail to remove.
 */
void spfx_trail_remove( Trail_spfx* trail )
{
   if (trail != NULL)
      trail->refcount--;
}


/**
 * @brief Deallocates an unreferenced, expired trail.
 */
static void spfx_trail_free( Trail_spfx* trail )
{
   assert(trail->refcount == 0);
   free(trail->point_ringbuf);
   free(trail);
}


/**
 * @brief Draws a trail on screen.
 */
static void spfx_trail_draw( const Trail_spfx* trail )
{
   double x1, y1, x2, y2, z;
   const TrailPoint *tp, *tpp;
   const TrailStyle *sp, *spp, *styles;
   size_t i, n;
   GLfloat len;
   gl_Matrix4 projection;
   double s;

   n = trail_size(trail);
   if (n==0)
      return;
   styles = trail->spec->style;

   /* Stuff that doesn't change for the entire trail. */
   glUseProgram( shaders.trail.program );
   if (gl_has( OPENGL_SUBROUTINES ))
      glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &trail->spec->type );
   glEnableVertexAttribArray( shaders.trail.vertex );
   gl_vboActivateAttribOffset( gl_squareVBO, shaders.trail.vertex, 0, 2, GL_FLOAT, 0 );
   glUniform1f( shaders.trail.dt, trail->dt );
   glUniform1f( shaders.trail.r, trail->r );

   z   = cam_getZoom();
   len = 0.;
   for (i = trail->iread + 1; i < trail->iwrite; i++) {
      tp  = &trail_at( trail, i );
      tpp = &trail_at( trail, i-1 );
      sp  = &styles[tp->mode];
      spp = &styles[tpp->mode];
      gl_gameToScreenCoords( &x1, &y1,  tp->x,  tp->y );
      gl_gameToScreenCoords( &x2, &y2, tpp->x, tpp->y );

      s = hypotf( x2-x1, y2-y1 );

      /* Set vertex. */
      projection = gl_Matrix4_Translate(gl_view_matrix, x1, y1, 0);
      projection = gl_Matrix4_Rotate2dv(projection, (x2-x1)/s, (y2-y1)/s);
      projection = gl_Matrix4_Scale(projection, s, z*(sp->thick+spp->thick), 1);
      projection = gl_Matrix4_Translate(projection, 0., -.5, 0);

      /* Set uniforms. */
      gl_Matrix4_Uniform(shaders.trail.projection, projection);
      gl_uniformColor(shaders.trail.c1, &sp->col);
      gl_uniformColor(shaders.trail.c2, &spp->col);
      glUniform1f(shaders.trail.t1, tp->t);
      glUniform1f(shaders.trail.t2, tpp->t);
      glUniform2f(shaders.trail.pos2, len, sp->thick);
      len += s;
      glUniform2f(shaders.trail.pos1, len, spp->thick);

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   }

   /* Clear state. */
   glDisableVertexAttribArray( shaders.trail.vertex );
   glUseProgram(0);

   /* Check errors. */
   gl_checkErr();
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

   /* Decrement the haptic timer. */
   if (haptic_lastUpdate > 0.)
      haptic_lastUpdate -= real_dt; /* Based on real delta-tick. */

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
   gl_view_matrix = gl_Matrix4_Translate( gl_view_matrix, shake_pos.x, shake_pos.y, 0 );
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
 *    @param[out] x X shake position.
 *    @param[out] y Y shake position.
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
      WARN(_("Unable to upload haptic effect: %s."), SDL_GetError());
      return -1;
   }

   return 0;
}


/**
 * @brief Runs a rumble effect.
 *
 *    @brief Current modifier being added.
 */
static void spfx_hapticRumble( double mod )
{
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
         WARN(_("Failed to update haptic effect: %s."), SDL_GetError());
         return;
      }

      /* Run the new effect. */
      SDL_HapticRunEffect( haptic, haptic_rumble, 1 );

      /* Set timer again. */
      haptic_lastUpdate += HAPTIC_UPDATE_INTERVAL;
   }
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
   int i;
   SPFX_Base *effect;
   int sx, sy;
   double time;
   Trail_spfx *trail;

   /* get the appropriate layer */
   switch (layer) {
      case SPFX_LAYER_FRONT:
         spfx_stack = spfx_stack_front;
         break;

      case SPFX_LAYER_MIDDLE:
         spfx_stack = spfx_stack_middle;
         break;

      case SPFX_LAYER_BACK:
         spfx_stack = spfx_stack_back;
         break;

      default:
         WARN(_("Rendering invalid SPFX layer."));
         return;
   }

   /* Trails are special (for now?). */
   if (layer == SPFX_LAYER_BACK)
      for (i=0; i<array_size(trail_spfx_stack); i++) {
         trail = trail_spfx_stack[i];
         spfx_trail_draw( trail );
      }

   /* Now render the layer */
   for (i=array_size(spfx_stack)-1; i>=0; i--) {
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


/**
 * @brief Parses raw values out of a "trail" element.
 * \warning This means values like idle->thick aren't ready to use.
 */
static void trailSpec_parse( xmlNodePtr node, TrailSpec *tc )
{
   static const char *mode_tags[] = MODE_TAGS;
   char *type;
   int i;
   xmlNodePtr cur = node->children;

   do {
      xml_onlyNodes(cur);
      if (xml_isNode(cur,"thickness"))
         tc->def_thick = xml_getFloat( cur );
      else if (xml_isNode(cur, "ttl"))
         tc->ttl = xml_getFloat( cur );
      else if (xml_isNode(cur, "type")) {
         type = xml_get(cur);
         if (gl_has( OPENGL_SUBROUTINES )) {
            tc->type = glGetSubroutineIndex( shaders.trail.program, GL_FRAGMENT_SHADER, type );
            if (tc->type == GL_INVALID_INDEX)
               WARN("Trail '%s' has unknown type '%s'", tc->name, type );
         }
      }
      else if (xml_isNode(cur, "nebula"))
         tc->nebula = xml_getInt( cur );
      else {
         for (i=0; i<MODE_MAX; i++)
            if (xml_isNode(cur, mode_tags[i])) {
               xmlr_attr_float_opt( cur, "r", tc->style[i].col.r );
               xmlr_attr_float_opt( cur, "g", tc->style[i].col.g );
               xmlr_attr_float_opt( cur, "b", tc->style[i].col.b );
               xmlr_attr_float_opt( cur, "a", tc->style[i].col.a );
               xmlr_attr_float_opt( cur, "scale", tc->style[i].thick );
	       break;
            }
         if (i == MODE_MAX)
            WARN(_("Trail '%s' has unknown node '%s'."), tc->name, cur->name);
      }
   } while (xml_nextNode(cur));

#define MELEMENT(o,s)   if (o) WARN(_("Trail '%s' missing '%s' element"), tc->name, s)
   MELEMENT( tc->def_thick==0, "thickness" );
   MELEMENT( tc->ttl==0, "ttl" );
#undef MELEMENT
}


/**
 * @brief Loads the trail colour sets.
 *
 *    @return 0 on success.
 */
static int trailSpec_load (void)
{
   TrailSpec *tc, *parent;
   xmlNodePtr first, node;
   xmlDocPtr doc;
   char *name, *inherits;

   /* Load the data. */
   doc = xml_parsePhysFS( TRAIL_DATA_PATH );
   if (doc == NULL)
      return -1;

   /* Get the root node. */
   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,"Trail_types")) {
      WARN( _("Malformed '%s' file: missing root element 'Trail_types'"), TRAIL_DATA_PATH);
      return -1;
   }

   /* Get the first node. */
   first = node->xmlChildrenNode; /* first event node */
   if (node == NULL) {
      WARN( _("Malformed '%s' file: does not contain elements"), TRAIL_DATA_PATH);
      return -1;
   }

   trail_spec_stack = array_create( TrailSpec );

   node = first;
   do {
      xml_onlyNodes( node );
      if (xml_isNode(node,"trail")) {
         tc = &array_grow( &trail_spec_stack );
         memset( tc, 0, sizeof(TrailSpec) );
         for(int i=0; i<MODE_MAX; i++)
            tc->style[i].thick = 1.;
         xmlr_attr_strd( node, "name", tc->name );

         /* Do the first pass for non-inheriting trails. */
         xmlr_attr_strd( node, "inherits", inherits );
         if (inherits == NULL)
            trailSpec_parse( node, tc );
         else
            free( inherits );
      }
   } while (xml_nextNode(node));

   /* Second pass to complete inheritance. */
   node = first;
   do {
      xml_onlyNodes( node );
      if (xml_isNode(node,"trail")) {
         /* Only interested in inherits. */
         xmlr_attr_strd( node, "inherits", inherits );
         if (inherits == NULL)
            continue;
         parent = trailSpec_getRaw( inherits );

         /* Get the style itself. */
         xmlr_attr_strd( node, "name", name );
         tc = trailSpec_getRaw( name );

         /* Make sure we found stuff. */
         if ((tc==NULL) || (parent==NULL)) {
            WARN(_("Trail '%s' that inherits from '%s' has missing reference!"), name, inherits );
            continue;
         }
         free( inherits );
         free( name );

         /* Set properties. */
         name = tc->name;
         memcpy( tc, parent, sizeof(TrailSpec) );
         tc->name = name;

         /* Load remaining properties (overrides parent). */
         trailSpec_parse( node, tc );
      }
   } while (xml_nextNode(node));

   for (tc=array_begin(trail_spec_stack); tc!=array_end(trail_spec_stack); tc++) {
      for(int i=0; i<MODE_MAX; i++)
         tc->style[i].thick *= tc->def_thick;
   }
   array_shrink(&trail_spec_stack);

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}


static TrailSpec* trailSpec_getRaw( const char* name )
{
   int i;

   for (i=0; i<array_size(trail_spec_stack); i++) {
      if ( strcmp(trail_spec_stack[i].name, name)==0 )
         return &trail_spec_stack[i];
   }

   WARN(_("Trail type '%s' not found in stack"), name);
   return NULL;
}


/**
 * @brief Gets a trail spec by name.
 *
 *    @return TrailSpec reference if found, else NULL.
 */
const TrailSpec* trailSpec_get( const char* name )
{
   return trailSpec_getRaw( name );
}
