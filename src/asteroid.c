/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file asteroid.c
 *
 * @brief Handles asteroid-related stuff.
 */
/** @cond */
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "asteroid.h"

#include "conf.h"
#include "array.h"
#include "camera.h"
#include "gatherable.h"
#include "space.h"
#include "opengl.h"
#include "toolkit.h"
#include "ndata.h"
#include "player.h"
#include "nlua_asteroid.h"
#include "threadpool.h"

/**
 * @brief Represents a small asteroid debris rendered in the player frame.
 */
typedef struct Debris_ {
   const glTexture *gfx; /**< Graphic of the debris. */
   vec2 pos;  /**< Position. */
   vec2 vel;  /**< Velocity. */
   double ang; /**< Angle. */
   double height; /**< height vs player */
   double alpha;  /**< Alpha value. */
} Debris;

const double DEBRIS_BUFFER = 1000.; /**< Buffer to smooth appearance of debris */

static const double SCAN_FADE = 10.; /**< 1/time it takes to fade in/out scanning text. */

static Debris *debris_stack = NULL; /**< All the debris in the current system (array.h). */
static glTexture **debris_gfx = NULL; /**< Graphics to use for debris. */
static ThreadQueue *asteroid_vpool = NULL; /**< For threading. */
static double asteroid_dt = 0.; /**< Used as a global variable when threading. */

/*
 * Useful data for asteroids.
 */
static AsteroidType *asteroid_types = NULL; /**< Asteroid types stack (array.h). */
static AsteroidTypeGroup *asteroid_groups = NULL; /**< Asteroid type groups stack (array.h). */
static glTexture **asteroid_gfx = NULL; /**< Graphics for the asteroids (array.h). */
static int asteroid_creating = 0;

/* Prototypes. */
static int asttype_cmp( const void *p1, const void *p2 );
static int asttype_parse( AsteroidType *at, const char *file );
static int asteroid_loadPLG( AsteroidType *temp, const char *buf );
static int astgroup_cmp( const void *p1, const void *p2 );
static int astgroup_parse( AsteroidTypeGroup *ag, const char *file );
static int asttype_load (void);

static int asteroid_updateSingle( void *data );
static void asteroid_renderSingle( const Asteroid *a );
static void debris_renderSingle( const Debris *d, double cx, double cy );
static void debris_init( Debris *deb );
static int asteroid_init( Asteroid *ast, const AsteroidAnchor *field );

static int asteroid_updateSingle( void *data )
{
   Asteroid *a    = data;
   const AsteroidAnchor *ast = &cur_system->asteroids[a->parent];
   double dt      = asteroid_dt;
   double offx, offy, d;
   int setvel = 0;

   /* Skip inexistent asteroids. */
   if (a->state == ASTEROID_XX) {
      a->timer -= dt;
      if (a->timer < 0.) {
         a->state = ASTEROID_XX_TO_BG;
         a->timer_max = a->timer = 1. + 3.*RNGF();
      }
      return 0;
   }

   /* Push back towards center. */
   offx = ast->pos.x - a->sol.pos.x;
   offy = ast->pos.y - a->sol.pos.y;
   d = pow2(offx)+pow2(offy);
   if (d >= pow2(ast->radius)) {
      d = sqrt(d);
      a->sol.vel.x += ast->accel * dt * offx / d;
      a->sol.vel.y += ast->accel * dt * offy / d;
      setvel = 1;
   }
   else if (ast->has_exclusion) {
      /* Push away from exclusion areas. */
      for (int k=0; k<array_size(cur_system->astexclude); k++) {
         AsteroidExclusion *exc = &cur_system->astexclude[k];
         double ex, ey, ed;

         /* Ignore exclusion zones that shouldn't affect. */
         if (!exc->affects)
            continue;

         ex = a->sol.pos.x - exc->pos.x;
         ey = a->sol.pos.y - exc->pos.y;
         ed = pow2(ex) + pow2(ey);
         if (ed <= pow2(exc->radius)) {
            ed = sqrt(ed);
            a->sol.vel.x += ast->accel * dt * ex / ed;
            a->sol.vel.y += ast->accel * dt * ey / ed;
            setvel = 1;
         }
      }
   }

   if (setvel) {
      /* Enforce max speed. */
      d = MOD(a->sol.vel.x, a->sol.vel.y);
      if (d > ast->maxspeed) {
         a->sol.vel.x *= ast->maxspeed / d;
         a->sol.vel.y *= ast->maxspeed / d;
      }
   }

   /* Update position. */
   /* TODO use physics.c */
   a->sol.pre = a->sol.pos;
   a->sol.pos.x += a->sol.vel.x * dt;
   a->sol.pos.y += a->sol.vel.y * dt;

   /* Update angle. */
   a->ang += a->spin * dt;

   /* igure out state change if applicable. */
   a->timer -= dt;
   if (a->timer < 0.) {
      switch (a->state) {
         /* Transition states. */
         case ASTEROID_FG:
            /* Don't go away if player is close. */
            if (vec2_dist2( &player.p->solid.pos, &a->sol.pos ) < pow2(1500.))
               a->state = ASTEROID_FG-1; /* So it gets turned back into ASTEROID_FG. */
            else
               /* This should be thread safe as a single pilot can only target a single asteroid. */
               pilot_untargetAsteroid( a->parent, a->id );
            FALLTHROUGH;
         case ASTEROID_XB:
         case ASTEROID_BX:
         case ASTEROID_XX_TO_BG:
            a->timer_max = a->timer = 1. + 3.*RNGF();
            break;

         /* Longer states. */
         case ASTEROID_FG_TO_BG:
            a->timer_max = a->timer = 10. + 20.*RNGF();
            break;
         case ASTEROID_BG_TO_FG:
            a->timer_max = a->timer = 90. + 30.*RNGF();
            break;

         /* Special case needs to respawn. */
         case ASTEROID_BG_TO_XX:
            asteroid_init( a, ast );
            a->timer_max = a->timer = 10. + 20.*RNGF();
            break;

         case ASTEROID_XX:
            /* Do nothing. */
            break;
      }
      /* States should be in proper order. */
      a->state = (a->state+1) % ASTEROID_STATE_MAX;
   }

   /* Update scanned state if necessary. */
   if (a->scanned) {
      if (a->state == ASTEROID_FG)
         a->scan_alpha = MIN( a->scan_alpha+SCAN_FADE*dt, 1.);
      else
         a->scan_alpha = MAX( a->scan_alpha-SCAN_FADE*dt, 0.);
   }
   return 0;
}

/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 */
void asteroids_update( double dt )
{
   /* Asteroids/Debris update */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      ast->has_exclusion = 0;

      for (int k=0; k<array_size(cur_system->astexclude); k++) {
         AsteroidExclusion *exc = &cur_system->astexclude[k];
         if (vec2_dist2( &ast->pos, &exc->pos ) < pow2(ast->radius+exc->radius)) {
            exc->affects = 1;
            ast->has_exclusion = 1;
         }
         else
            exc->affects = 0;
      }

      /* Now just thread it and zoom. */
      for (int j=0; j<ast->nb; j++) {
         Asteroid *a = &ast->asteroids[j];
         vpool_enqueue( asteroid_vpool, asteroid_updateSingle, a );
      }
      asteroid_dt = dt;
      vpool_wait( asteroid_vpool );

      /* Do quadtree stuff. Can't be threaded. */
      qt_clear( &ast->qt );
      for (int j=0; j<ast->nb; j++) {
         const Asteroid *a = &ast->asteroids[j];
         /* Add to quadtree if in foreground. */
         if (a->state == ASTEROID_FG) {
            int x, y, w2, h2, px, py;
            x  = round(a->sol.pos.x);
            y  = round(a->sol.pos.y);
            px = round(a->sol.pre.x);
            py = round(a->sol.pre.y);
            w2 = ceil(a->gfx->sw*0.5);
            h2 = ceil(a->gfx->sh*0.5);
            qt_insert( &ast->qt, j, MIN(x,px)-w2, MIN(y,py)-h2, MAX(x,px)+w2, MAX(y,py)+h2 );
         }
      }
   }

   /* Only have to update stuff if not simulating. */
   if (!space_isSimulation()) {
      double dx, dy;
      cam_getDPos( &dx, &dy );

      for (int j=0; j<array_size(debris_stack); j++) {
         Debris *d = &debris_stack[j];
         int infield;
         vec2 v;

         d->pos.x += d->vel.x * dt - dx;
         d->pos.y += d->vel.y * dt - dy;

         /* Check boundaries */
         if (d->pos.x > SCREEN_W + DEBRIS_BUFFER)
            d->pos.x -= SCREEN_W + 2.*DEBRIS_BUFFER;
         else if (d->pos.y > SCREEN_H + DEBRIS_BUFFER)
            d->pos.y -= SCREEN_H + 2.*DEBRIS_BUFFER;
         else if (d->pos.x < -DEBRIS_BUFFER)
            d->pos.x += SCREEN_W + 2.*DEBRIS_BUFFER;
         else if (d->pos.y < -DEBRIS_BUFFER)
            d->pos.y += SCREEN_H + 2.*DEBRIS_BUFFER;

         /* Set alpha based on position. */
         /* TODO there seems to be some offset mistake or something going on
            * here, not too big of an issue though. */
         gl_screenToGameCoords( &v.x, &v.y, d->pos.x, d->pos.y );
         infield = asteroids_inField( &v );
         if (infield>=0)
            d->alpha = MIN( 1.0, d->alpha + 0.5 * dt );
         else
            d->alpha = MAX( 0.0, d->alpha - 0.5 * dt );
      }
   }
}

/**
 * @brief Initializes the system.
 */
void asteroids_init (void)
{
   double density_max = 0.;
   int ndebris;
   asteroid_creating = 1;

   if (asteroid_vpool==NULL)
      asteroid_vpool = vpool_create();

   if (debris_gfx==NULL)
      debris_gfx = array_create( glTexture* );
   array_erase( &debris_gfx, array_begin(debris_gfx), array_end(debris_gfx) );

   /* Set up asteroids. */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      int qx, qy, qr;
      ast->id = i;

      /* Add graphics to debris. */
      for (int j=0; j<array_size(ast->groups); j++) {
         AsteroidTypeGroup *ag = ast->groups[j];
         for (int k=0; k<array_size(ag->types); k++) {
            AsteroidType *at = ag->types[k];
            for (int x=0; x<array_size(at->gfxs); x++)
               array_push_back( &debris_gfx, (glTexture*)at->gfxs[x] );
         }
      }

      /* Build quadtree. */
      if (ast->qt_init)
         qt_destroy( &ast->qt );
      qx = round(ast->pos.x);
      qy = round(ast->pos.y);
      qr = ceil(ast->radius);
      qt_create( &ast->qt, qx-qr, qy-qr, qx+qr, qy+qr, 2, 5 );
      ast->qt_init = 1;

      /* Add the asteroids to the anchor */
      ast->asteroids = realloc( ast->asteroids, (ast->nb) * sizeof(Asteroid) );
      for (int j=0; j<ast->nb; j++) {
         double r = RNGF();
         Asteroid *a = &ast->asteroids[j];
         a->id = j;
         if (asteroid_init(a, ast))
            continue;
         if (r > 0.6)
            a->state = ASTEROID_FG;
         else if (r > 0.8)
            a->state = ASTEROID_XB;
         else if (r > 0.9)
            a->state = ASTEROID_BX;
         else
            a->state = ASTEROID_XX;
         a->timer = a->timer_max = 30.*RNGF();
         a->ang = RNGF() * M_PI * 2.;
      }

      density_max = MAX( density_max, ast->density );
   }

   /* Add the debris to the anchor */
   if (debris_stack == NULL)
      debris_stack = array_create( Debris );

   /* We compute a fixed amount and scale depending on how big the screen is
    * compared the reference (minimum resolution). */
   ndebris = density_max * 100. * (SCREEN_W+2.*DEBRIS_BUFFER * SCREEN_H+2.*DEBRIS_BUFFER) / (RESOLUTION_W_MIN * RESOLUTION_H_MIN);
   array_resize( &debris_stack, ndebris );

   for (int j=0; j<array_size(debris_stack); j++)
      debris_init( &debris_stack[j] );

   asteroid_creating = 0;
}

/**
 * @brief Initializes an asteroid.
 *    @param ast Asteroid to initialize.
 *    @param field Asteroid field the asteroid belongs to.
 */
static int asteroid_init( Asteroid *ast, const AsteroidAnchor *field )
{
   double mod, theta, wmax, r, r2;
   double x,y;
   AsteroidType *at = NULL;
   int outfield, id;
   int attempts = 0;
   vec2 pos, vel;

   ast->parent  = field->id;
   ast->scanned = 0;
   r2 = pow2( field->radius );

   do {
      /* Try to keep density uniform using cartesian coordinates. */
      x = field->pos.x + (RNGF()*2.-1.)*field->radius;
      y = field->pos.y + (RNGF()*2.-1.)*field->radius;

      /* Check if out of the field. */
      outfield = (asteroids_inField(&ast->sol.pos) < 0);

      /* If this is the first time and it's spawned outside the field,
       * we get rid of it so that density remains roughly consistent. */
      if (asteroid_creating && outfield && (vec2_dist2( &ast->sol.pos, &field->pos ) < r2)) {
         ast->state = ASTEROID_XX;
         ast->timer_max = ast->timer = HUGE_VAL; /* Don't reappear. */
         /* TODO probably do a more proper solution removing total number of asteroids. */
         return -1;
      }

   } while (outfield && (attempts++ < 1000));

   /* Randomly init the type of asteroid */
   r = field->groupswtotal * RNGF();
   wmax = 0.;
   for (int i=0; i<array_size(field->groups); i++) {
      wmax += field->groupsw[i];
      if (r > wmax)
         continue;
      AsteroidTypeGroup *grp = field->groups[i];
      double wi = 0.;
      r = grp->wtotal * RNGF();
      for (int j=0; j<array_size(grp->types); j++) {
         wi += grp->weights[j];
         if (r > wi)
            continue;
         at = grp->types[j];
         break;
      }
      break;
   }

   /* Randomly init the gfx ID, and associated polygon */
   id = RNG(0, array_size(at->gfxs)-1);
   ast->gfx = at->gfxs[ id ];
   ast->polygon = &at->polygon[ id ];

   ast->type = at;
   ast->armour = at->armour_min + RNGF() * (at->armour_max-at->armour_min);

   /* And a random velocity/spin */
   theta     = RNGF()*2.*M_PI;
   ast->spin = (1-2*RNGF())*field->maxspin;
   mod       = RNGF()*field->maxspeed;

   /* Fade in stuff. */
   ast->state = ASTEROID_XX;
   ast->timer_max = ast->timer = -1.;
   ast->ang = RNGF() * M_PI * 2.;

   /* Set up the solid. */
   vec2_cset( &pos, x, y );
   vec2_pset( &vel, mod, theta );
   /* TODO set a proper mass. */
   solid_init( &ast->sol, 1., theta, &pos, &vel, SOLID_UPDATE_EULER );

   return 0;
}

/**
 * @brief Initializes a debris.
 *    @param deb Debris to initialize.
 */
static void debris_init( Debris *deb )
{
   double theta, mod;
   /* Position */
   deb->pos.x = -DEBRIS_BUFFER + RNGF()*(SCREEN_W + 2.*DEBRIS_BUFFER);
   deb->pos.y = -DEBRIS_BUFFER + RNGF()*(SCREEN_H + 2.*DEBRIS_BUFFER);

   /* And a random velocity */
   theta = RNGF()*2.*M_PI;
   mod = RNGF() * 20.;
   vec2_pset( &deb->vel, mod, theta );

   /* Randomly init the gfx ID */
   //deb->gfx = asteroid_gfx[ RNG(0,(int)array_size(asteroid_gfx)-1) ];
   deb->gfx = debris_gfx[ RNG(0,(int)array_size(debris_gfx)-1) ];

   /* Random height vs player. */
   deb->height = 0.8 + RNGF()*0.4;
   deb->alpha = 0.;
   deb->ang = RNGF() * M_PI * 2.;
}

/**
 * @brief Updates internal alues of an asteroid field.
 */
void asteroids_computeInternals( AsteroidAnchor *a )
{
   /* Calculate area */
   a->area = M_PI * pow2(a->radius);

   /* Compute number of asteroids */
   a->nb      = floor( a->area / ASTEROID_REF_AREA * a->density );

   /* Computed from your standard physics equations (with a bit of margin). */
   a->margin  = pow2(a->maxspeed) / (4.*a->accel) + 50.;

   /* Compute weight total. */
   a->groupswtotal = 0.;
   for (int i=0; i<array_size(a->groupsw); i++)
      a->groupswtotal += a->groupsw[i];
}

/**
 * @brief Loads the asteroids.
 *
 *    @return 0 on success.
 */
int asteroids_load (void)
{
   int ret;
   char **asteroid_files, file[PATH_MAX];

   /* Load asteroid types. */
   ret = asttype_load();
   if (ret < 0)
      return ret;

   /* Load asteroid graphics. */
   asteroid_files = PHYSFS_enumerateFiles( SPOB_GFX_SPACE_PATH"asteroid/" );
   asteroid_gfx = array_create( glTexture* );

   for (size_t i=0; asteroid_files[i]!=NULL; i++) {
      snprintf( file, sizeof(file), "%s%s", SPOB_GFX_SPACE_PATH"asteroid/", asteroid_files[i] );
      array_push_back( &asteroid_gfx, gl_newImage( file, OPENGL_TEX_MIPMAPS ) );
   }

   PHYSFS_freeList( asteroid_files );
   return 0;
}

/**
 * @brief Compares two asteroid types.
 */
static int asttype_cmp( const void *p1, const void *p2 )
{
   const AsteroidType *at1, *at2;
   at1 = (const AsteroidType*) p1;
   at2 = (const AsteroidType*) p2;
   return strcmp(at1->name,at2->name);
}

/**
 * @brief Compares two asteroid type groups.
 */
static int astgroup_cmp( const void *p1, const void *p2 )
{
   const AsteroidTypeGroup *at1, *at2;
   at1 = (const AsteroidTypeGroup*) p1;
   at2 = (const AsteroidTypeGroup*) p2;
   return strcmp(at1->name,at2->name);
}

/**
 * @brief Loads the asteroids types.
 *
 *    @return 0 on success.
 */
static int asttype_load (void)
{
   char **asteroid_files = ndata_listRecursive( ASTEROID_TYPES_DATA_PATH );
   asteroid_types = array_create( AsteroidType );

   for (int i=0; i<array_size( asteroid_files ); i++) {
      if (ndata_matchExt( asteroid_files[i], "xml" )) {
         AsteroidType at;
         int ret = asttype_parse( &at, asteroid_files[i] );
         if (ret == 0)
            array_push_back( &asteroid_types, at );
      }
      free( asteroid_files[i] );
   }
   array_free( asteroid_files );
   array_shrink( &asteroid_types );
   qsort( asteroid_types, array_size(asteroid_types), sizeof(AsteroidType), asttype_cmp );

   /* Check for name collisions. */
   for (int i=0; i<array_size(asteroid_types)-1; i++)
      if (strcmp( asteroid_types[i].name, asteroid_types[i+1].name )==0)
         WARN(_("Asteroid Types with same name '%s'"),asteroid_types[i].name);

   /* Load the asteroid groups from XML definitions. */
   asteroid_groups = array_create( AsteroidTypeGroup );
   asteroid_files = ndata_listRecursive( ASTEROID_GROUPS_DATA_PATH );
   for (int i=0; i<array_size( asteroid_files ); i++) {
      if (ndata_matchExt( asteroid_files[i], "xml" )) {
         AsteroidTypeGroup atg;
         int ret = astgroup_parse( &atg, asteroid_files[i] );
         if (ret == 0)
            array_push_back( &asteroid_groups, atg );
      }
      free( asteroid_files[i] );
   }
   array_free( asteroid_files );
   /* Add asteroid types as individual groups. */
   for (int i=0; i<array_size( asteroid_types ); i++) {
      AsteroidType *at = &asteroid_types[i];
      AsteroidTypeGroup grp = {
         .name   = strdup(at->name),
         .types  = array_create( AsteroidType* ),
         .weights= array_create( double ),
         .wtotal = 1.
      };
      array_push_back( &grp.types, at );
      array_push_back( &grp.weights, 1. );
      array_push_back( &asteroid_groups, grp );
   }
   array_shrink( &asteroid_groups );
   qsort( asteroid_groups, array_size(asteroid_groups), sizeof(AsteroidTypeGroup), astgroup_cmp );

   /* Check for name collisions. */
   for (int i=0; i<array_size(asteroid_groups)-1; i++)
      if (strcmp( asteroid_groups[i].name, asteroid_groups[i+1].name )==0)
         WARN(_("Asteroid Type Groups with same name '%s'"),asteroid_groups[i].name);

   return 0;
}

/**
 * @brief Parses the XML of an asteroid type.
 *
 *    @param[out] at Outfit asteroid type.
 *    @param file File containing the XML information.
 */
static int asttype_parse( AsteroidType *at, const char *file )
{
   xmlNodePtr parent, node;
   xmlDocPtr doc;

   /* Load the data. */
   doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   /* Get the root node. */
   parent = doc->xmlChildrenNode;
   if (!xml_isNode(parent,"asteroid")) {
      WARN( _("Malformed '%s' file: missing root element 'asteroid'"), file);
      return -1;
   }

   /* Set up the element. */
   memset( at, 0, sizeof(AsteroidType) );
   at->gfxs       = array_create( glTexture* );
   at->polygon    = array_create( CollPoly );
   at->material   = array_create( AsteroidReward );
   at->damage     = 100;
   at->penetration = 100.;
   at->exp_radius = 50.;
   at->alert_range = 7000.;

   xmlr_attr_strd(parent,"name",at->name);
   if (at->name == NULL)
      WARN(_("Asteroid '%s' has invalid or no name"), file);

   node = parent->xmlChildrenNode;
   do {
      /* Only handle nodes. */
      xml_onlyNodes(node);

      xmlr_strd( node, "scanned", at->scanned_msg );
      xmlr_float( node, "armour_min", at->armour_min );
      xmlr_float( node, "armour_max", at->armour_max );
      xmlr_float( node, "absorb", at->absorb );
      xmlr_float( node, "damage", at->damage );
      xmlr_float( node, "disable", at->disable );
      xmlr_float( node, "penetration", at->penetration );
      xmlr_float( node, "exp_radius", at->exp_radius );
      xmlr_float( node, "alert_range", at->alert_range );

      if (xml_isNode(node,"gfx")) {
         array_push_back( &at->gfxs, xml_parseTexture( node, SPOB_GFX_SPACE_PATH"asteroid/%s", 1, 1,  OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS ) );
         asteroid_loadPLG( at, xml_get(node) );
         continue;
      }
      else if (xml_isNode(node,"commodity")) {
         /* Check that name and quantity are defined. */
         int namdef = 0;
         AsteroidReward material;
         memset( &material, 0, sizeof(material) );

         xmlNodePtr cur = node->xmlChildrenNode;
         do {
            xml_onlyNodes(cur);

            xmlr_int( cur, "quantity", material.quantity );
            xmlr_int( cur, "rarity", material.rarity );

            if (xml_isNode(cur,"name")) {
               const char *str = xml_get(cur);
               material.material = commodity_get( str );
               if (material.material->gfx_space==NULL)
                  WARN(_("Asteroid Type '%s' has Commodity '%s' with no 'gfx_space'."),at->name,str);
               namdef = 1;
               continue;
            }

            WARN(_("Asteroid Type '%s' has unknown node '%s'"), at->name, cur->name);
         } while (xml_nextNode(cur));

         if (namdef==0 || material.quantity==0)
            WARN(_("Asteroid Type '%s' has commodity that lacks name or quantity."), at->name);

         array_push_back( &at->material, material );
         continue;
      }
      WARN(_("Asteroid Type '%s' has unknown node '%s'"), at->name, node->name);
   } while (xml_nextNode(node));

   /* Clean up. */
   xmlFreeDoc(doc);

   /* Some post-process. */
   at->absorb = CLAMP( 0., 1., at->absorb / 100. );
   at->penetration = CLAMP( 0., 1., at->penetration / 100. );

   /* Checks. */
   if (at->armour_max < at->armour_min)
      WARN(_("Asteroid Type '%s' has armour_max below armour_min"), at->name);

#define MELEMENT(o,s) \
if (o) WARN(_("Asteroid Type '%s' missing/invalid '%s' element"), at->name, s) /**< Define to help check for data errors. */
   MELEMENT(at->scanned_msg==NULL,"scanned");
   MELEMENT(array_size(at->gfxs)==0,"gfx");
   MELEMENT(at->armour_min <= 0.,"armour_min");
   MELEMENT(at->armour_max <= 0.,"armour_max");
#undef MELEMENT

   return 0;
}

/**
 * @brief Loads the collision polygon for an asteroid type.
 *
 *    @param temp AsteroidType to load into.
 *    @param buf Name of the file.
 */
static int asteroid_loadPLG( AsteroidType *temp, const char *buf )
{
   char file[PATH_MAX];
   CollPoly *polygon;
   xmlDocPtr doc;
   xmlNodePtr node;

   snprintf( file, sizeof(file), "%s%s.xml", ASTEROID_POLYGON_PATH, buf );

   /* There is only one polygon per gfx, but it has to be added to all the other polygons */
   /* associated to each gfx of current AsteroidType. */
   /* In case it fails to load for some reason, its size will be set to 0. */
   polygon = &array_grow( &temp->polygon );
   polygon->npt = 0;

   /* See if the file does exist. */
   if (!PHYSFS_exists(file)) {
      WARN(_("%s xml collision polygon does not exist!\n \
               Please use the script 'polygon_from_sprite.py'\n \
               This file can be found in Naev's artwork repo."), file);
      return 0;
   }

   /* Load the XML. */
   doc  = xml_parsePhysFS( file );
   if (doc == NULL)
      return 0;

   node = doc->xmlChildrenNode; /* First polygon node */
   if (node == NULL) {
      xmlFreeDoc(doc);
      WARN(_("Malformed %s file: does not contain elements"), file);
      return 0;
   }

   do { /* load the polygon data */
      if (xml_isNode(node,"polygons")) {
         xmlNodePtr cur = node->children;
         do {
            if (xml_isNode(cur,"polygon")) {
               LoadPolygon( polygon, cur );
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   return 0;
}

/**
 * @brief Parses an asteroid type group from a file.
 *
 *    @param[out] ag Asteroid type group to load.
 *    @param file File to load from.
 *    @return 0 on success.
 */
static int astgroup_parse( AsteroidTypeGroup *ag, const char *file )
{
   xmlNodePtr parent, node;
   xmlDocPtr doc;

   /* Load the data. */
   doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   /* Get the root node. */
   parent = doc->xmlChildrenNode;
   if (!xml_isNode(parent,"asteroid_group")) {
      WARN( _("Malformed '%s' file: missing root element 'asteroid_group'"), file);
      return -1;
   }

   /* Set up the element. */
   memset( ag, 0, sizeof(AsteroidTypeGroup) );
   ag->types  = array_create( AsteroidType* );
   ag->weights = array_create( double );

   xmlr_attr_strd(parent,"name",ag->name);
   if (ag->name == NULL)
      WARN(_("Asteroid '%s' has invalid or no name"), file);

   node = parent->xmlChildrenNode;
   do {
      /* Only handle nodes. */
      xml_onlyNodes(node);

      if (xml_isNode(node,"type")) {
         double w;
         xmlr_attr_float_def(node, "weight", w, 1.);
         AsteroidType *at = asttype_getName( xml_get(node) );
         array_push_back( &ag->types, at );
         array_push_back( &ag->weights, w );
         ag->wtotal += w;
         continue;
      }
      WARN(_("Asteroid Type Group '%s' has unknown node '%s'"), ag->name, node->name);
   } while (xml_nextNode(node));

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Renders the system overlay.
 */
void asteroids_renderOverlay (void)
{
   double cx, cy;
   cam_getPos( &cx, &cy );
   cx -= SCREEN_W/2.;
   cy -= SCREEN_H/2.;

   /* Render the debris. */
   for (int j=0; j<array_size(debris_stack); j++) {
      const Debris *d = &debris_stack[j];
      if (d->height > 1.)
         debris_renderSingle( d, cx, cy );
   }
}

/**
 * @brief Renders the current systems' spobs.
 */
void asteroids_render (void)
{
   double cx, cy, z;
   cam_getPos( &cx, &cy );
   z = cam_getZoom();
   cx -= SCREEN_W/2.;
   cy -= SCREEN_H/2.;

   /* Render the asteroids & debris. */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      const AsteroidAnchor *ast = &cur_system->asteroids[i];
      double x, y, r;

      /* See if the asteroid field is in range, if not skip. */
      gl_gameToScreenCoords( &x, &y, ast->pos.x, ast->pos.y );
      r = ast->radius * z;
      if ((x < -r) || (x > SCREEN_W+r) ||
         (y < -r) || (y > SCREEN_H+r))
         continue;

      /* Render all asteroids. */
      for (int j=0; j<ast->nb; j++)
        asteroid_renderSingle( &ast->asteroids[j] );
   }

   /* Render the debris. */
   for (int j=0; j<array_size(debris_stack); j++) {
      const Debris *d = &debris_stack[j];
      if (d->height <= 1.)
         debris_renderSingle( d, cx, cy );
   }

   /* Render gatherable stuff. */
   gatherable_render();
}

/**
 * @brief Renders an asteroid.
 */
static void asteroid_renderSingle( const Asteroid *a )
{
   double nx, ny;
   const AsteroidType *at;
   glColour col;
   double progress;
   const glColour darkcol = cGrey20;

   /* Skip invisible asteroids */
   if (a->state == ASTEROID_XX)
      return;

   progress = a->timer / a->timer_max;
   switch (a->state) {
      case ASTEROID_XX_TO_BG:
         col   = darkcol;
         col.a = 1.-progress;
         break;
      case ASTEROID_XB:
      case ASTEROID_BX:
         col   = darkcol;
         break;
      case ASTEROID_BG_TO_FG:
         col_blend( &col, &darkcol, &cWhite, progress );
         break;
      case ASTEROID_FG:
         col   = cWhite;
         break;
      case ASTEROID_FG_TO_BG:
         col_blend( &col, &cWhite, &darkcol, progress );
         break;
      case ASTEROID_BG_TO_XX:
         col   = darkcol;
         col.a = progress;
         break;

      default:
         break;
   }

   at = a->type;
   gl_renderSpriteRotate( a->gfx, a->sol.pos.x, a->sol.pos.y, a->ang, 0, 0, &col );

   /* Add the commodities if scanned. */
   if (!a->scanned)
      return;
   col = cFontWhite;
   col.a = a->scan_alpha;
   gl_gameToScreenCoords( &nx, &ny, a->sol.pos.x, a->sol.pos.y );
   gl_printRaw( &gl_smallFont, nx+a->gfx->sw/2, ny-gl_smallFont.h/2, &col, -1., _(at->scanned_msg) );
   /*
   for (int i=0; i<array_size(at->material); i++) {
      AsteroidReward *mat = &at->material[i];
      Commodity *com = mat->material;
      if (com->gfx_space!=NULL)
         gl_renderSprite( com->gfx_space, a->pos.x, a->pos.y-10.*i, 0, 0, NULL );
      snprintf(c, sizeof(c), "x%i", mat->quantity);
      gl_printRaw( &gl_smallFont, nx+10, ny-5-10.*i, &cFontWhite, -1., c );
   }
   */
}

/**
 * @brief Renders a debris.
 */
static void debris_renderSingle( const Debris *d, double cx, double cy )
{
   const double scale = 0.5;
   const glColour col = COL_ALPHA( cInert, d->alpha );

   gl_renderSpriteScaleRotate( d->gfx, d->pos.x+cx, d->pos.y+cy, scale, scale, d->ang, 0, 0, &col );
}

/**
 * @brief Frees an asteroid anchor.
 *
 *    @param ast Asteroid anchor to free.
 */
void asteroid_free( AsteroidAnchor *ast )
{
   if (ast->qt_init)
      qt_destroy( &ast->qt );
   free(ast->label);
   free(ast->asteroids);
   array_free(ast->groups);
   array_free(ast->groupsw);
}

/**
 * @brief Cleans up the system.
 */
void asteroids_free (void)
{
   /* Free asteroid graphics. */
   for (int i=0; i<array_size(asteroid_gfx); i++)
      gl_freeTexture(asteroid_gfx[i]);
   array_free(asteroid_gfx);
   array_free(debris_gfx);

   /* Free the asteroid types. */
   for (int i=0; i<array_size(asteroid_types); i++) {
      AsteroidType *at = &asteroid_types[i];
      free(at->name);
      free(at->scanned_msg);
      array_free(at->material);
      for (int j=0; j<array_size(at->gfxs); j++)
         gl_freeTexture(at->gfxs[j]);
      array_free(at->gfxs);

      /* Free collision polygons. */
      for (int j=0; j<array_size(at->polygon); j++)
         FreePolygon( &at->polygon[j] );
      array_free(at->polygon);
   }
   array_free(asteroid_types);
   asteroid_types = NULL;

   /* Free the asteroid groups. */
   for (int i=0; i<array_size(asteroid_groups); i++) {
      AsteroidTypeGroup *ag = &asteroid_groups[i];
      free(ag->name);
      array_free(ag->types);
      array_free(ag->weights);
   }
   array_free(asteroid_groups);
   asteroid_groups = NULL;

   /* Clean up debris. */
   array_free( debris_stack );
   debris_stack = NULL;

   /* Free the gatherable stack. */
   gatherable_free();
}

/**
 * @brief See if the position is in an asteroid field.
 *
 *    @param p pointer to the position.
 *    @return -1 If false; index of the field otherwise.
 */
int asteroids_inField( const vec2 *p )
{
   /* Always return -1 if in an exclusion zone */
   for (int i=0; i<array_size(cur_system->astexclude); i++) {
      AsteroidExclusion *e = &cur_system->astexclude[i];
      if (vec2_dist2( p, &e->pos ) <= pow2(e->radius))
         return -1;
   }

   /* Check if in asteroid field */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *a = &cur_system->asteroids[i];
      if (vec2_dist2( p, &a->pos ) <= pow2(a->radius))
         return i;
   }

   return -1;
}

/**
 * @brief Gets all the asteroid types.
 *
 *    @return All the asteroid types (array.h).
 */
const AsteroidType *asttype_getAll (void)
{
   return asteroid_types;
}

/**
 * @brief Gets the ID of an asteroid type by name.
 *
 *    @param name Name of the asteroid type to get.
 *    @return Matching asteroid type.
 */
AsteroidType *asttype_getName( const char *name )
{
   const AsteroidType q = { .name=(char*)name };
   AsteroidType *at = bsearch( &q, asteroid_types, array_size(asteroid_types), sizeof(AsteroidType), asttype_cmp );
   if (at == NULL)
      WARN(_("Unknown Asteroid Type '%s'"),name);
   return at;
}

/**
 * @brief Gets all the asteroid type groups.
 *
 *    @return An array (array.h) containing all the asteroid type groups.
 */
const AsteroidTypeGroup *astgroup_getAll (void)
{
   return asteroid_groups;
}

/**
 * @brief Gets an asteroid type group by name.
 *
 *    @param name Name of the asteroid type group to get.
 *    @return The asteroid type group matching the name.
 */
AsteroidTypeGroup *astgroup_getName( const char *name )
{
   const AsteroidTypeGroup q = { .name=(char*)name };
   AsteroidTypeGroup *ag = bsearch( &q, asteroid_groups, array_size(asteroid_groups), sizeof(AsteroidTypeGroup), astgroup_cmp );
   if (ag == NULL)
      WARN(_("Unknown Asteroid Type Group '%s'"),name);
   return ag;
}

/**
 * @brief Hits an asteroid.
 *
 *    @param a hit asteroid
 *    @param dmg Damage being done
 *    @param max_rarity Maximum rarity of the rewards to give if destroyed.
 *    @param mining_bonus Bonus to mining.
 */
void asteroid_hit( Asteroid *a, const Damage *dmg, int max_rarity, double mining_bonus )
{
   double darmour;
   double absorb = 1. - CLAMP( 0., 1., a->type->absorb - dmg->penetration );
   dtype_calcDamage( NULL, &darmour, absorb, NULL, dmg, NULL );

   a->armour -= darmour;
   if (a->armour <= 0)
      asteroid_explode( a, max_rarity, mining_bonus );
}

/**
 * @brief Makes an asteroid explode.
 *
 *    @param a asteroid to make explode
 *    @param max_rarity Maximum rarity of the rewards to give, set to -1 for none.
 *    @param mining_bonus Bonus to mining.
 */
void asteroid_explode( Asteroid *a, int max_rarity, double mining_bonus )
{
   Damage dmg;
   char buf[16];
   double rad2;
   LuaAsteroid_t la;
   const AsteroidType *at = a->type;
   const AsteroidAnchor *field = &cur_system->asteroids[a->parent];
   Pilot *const* pilot_stack = pilot_getAll();

   /* Manage the explosion */
   dmg.type          = dtype_get("explosion_splash");
   dmg.damage        = at->damage;
   dmg.penetration   = at->penetration; /* Full penetration. */
   dmg.disable       = 0.;
   expl_explode( a->sol.pos.x, a->sol.pos.y, a->sol.vel.x, a->sol.vel.y,
                 at->exp_radius, &dmg, NULL, EXPL_MODE_SHIP );

   /* Play random explosion sound. */
   snprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
   sound_playPos( sound_get(buf), a->sol.pos.x, a->sol.pos.y, a->sol.vel.x, a->sol.vel.y );

   /* Alert nearby pilots. */
   rad2 = pow2( at->alert_range );
   la.parent = a->parent;
   la.id = a->id;
   lua_pushasteroid( naevL, la );
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];

      if (vec2_dist2( &p->solid.pos, &a->sol.pos ) > rad2)
         continue;

      pilot_msg( NULL, p, "asteroid", -1 );
   }
   lua_pop(naevL,1);

   /* Release commodity rewards. */
   if (max_rarity >= 0) {
      int ndrops = 0;
      for (int i=0; i<array_size(at->material); i++) {
         const AsteroidReward *mat = &at->material[i];
         if (mat->rarity > max_rarity)
            continue;
         ndrops++;
      }
      if (ndrops > 0) {
         double r = RNGF();
         double prob = 1./(double)ndrops;
         double accum = 0.;
         for (int i=0; i<array_size(at->material); i++) {
            const AsteroidReward *mat = &at->material[i];
            if (mat->rarity > max_rarity)
               continue;
            accum += prob;
            if (r > accum)
               continue;

            int nb = RNG(0, round((double)mat->quantity * mining_bonus));
            for (int j=0; j<nb; j++) {
               vec2 pos, vel;
               pos = a->sol.pos;
               vel = a->sol.vel;
               pos.x += (RNGF()*30.-15.);
               pos.y += (RNGF()*30.-15.);
               vel.x += (RNGF()*20.-10.);
               vel.y += (RNGF()*20.-10.);
               gatherable_init( mat->material, &pos, &vel, -1., RNG(1,5), 0 );
            }
            break;
         }
      }
   }

   /* Remove the asteroid target to any pilot. */
   pilot_untargetAsteroid( a->parent, a->id );

   /* Make it respawn elsewhere */
   asteroid_init( a, field );
   a->state = ASTEROID_BG_TO_XX;
   a->timer_max = a->timer = 0.5;
}

void asteroid_collideQueryIL( AsteroidAnchor *anc, IntList *il, int x1, int y1, int x2, int y2 )
{
   qt_query( &anc->qt, il, x1, y1, x2, y2 );
}
