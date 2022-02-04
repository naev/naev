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

#include "array.h"
#include "asteroid.h"
#include "space.h"
#include "opengl.h"
#include "toolkit.h"
#include "ndata.h"

#define DEBRIS_BUFFER         1000 /**< Buffer to smooth appearance of debris */

/*
 * Asteroid types stack.
 */
static AsteroidType *asteroid_types = NULL; /**< Asteroid types stack. */

/*
 * Misc.
 */
static glTexture **asteroid_gfx = NULL; /**< Graphics for the asteroids. */
static size_t nasterogfx = 0; /**< Nb of asteroid gfx. */

/* Prototypes. */
static int asteroidTypes_cmp( const void *p1, const void *p2 );
static int asteroidTypes_parse( AsteroidType *at, const char *file );
static int system_parseAsteroidField( const xmlNodePtr node, StarSystem *sys );
static int asteroidTypes_load (void);
static int system_parseAsteroidExclusion( const xmlNodePtr node, StarSystem *sys );

static void space_renderAsteroid( const Asteroid *a );
static void space_renderDebris( const Debris *d, double x, double y );
static void asteroid_explode( Asteroid *a, AsteroidAnchor *field, int give_reward );
static void debris_init( Debris *deb );
static void asteroid_init( Asteroid *ast, AsteroidAnchor *field );


/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 */
void asteroids_update( double dt )
{
   /* Asteroids/Debris update */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      double x, y;
      Pilot *pplayer;
      int has_exclusion = 0;
      AsteroidAnchor *ast = &cur_system->asteroids[i];

      for (int k=0; k<array_size(cur_system->astexclude); k++) {
         AsteroidExclusion *exc = &cur_system->astexclude[k];
         if (vect_dist2( &ast->pos, &exc->pos ) < pow2(ast->radius+exc->radius)) {
            exc->affects = 1;
            has_exclusion = 1;
         }
         else
            exc->affects = 0;
      }

      for (int j=0; j<ast->nb; j++) {
         double offx, offy, d;
         Asteroid *a = &ast->asteroids[j];
         int setvel = 0;

         /* Skip inexistent asteroids. */
         if (a->state == ASTEROID_XX) {
            a->timer -= dt;
            if (a->timer < 0.) {
               a->state = ASTEROID_XX_TO_BG;
               a->timer_max = a->timer = 1. + 3.*RNGF();
            }
            continue;
         }

         /* Push back towards center. */
         offx = ast->pos.x - a->pos.x;
         offy = ast->pos.y - a->pos.y;
         d = pow2(offx)+pow2(offy);
         if (d >= pow2(ast->radius)) {
            d = sqrt(d);
            a->vel.x += ast->thrust * dt * offx / d;
            a->vel.y += ast->thrust * dt * offy / d;
            setvel = 1;
         }
         else if (has_exclusion) {
            /* Push away from exclusion areas. */
            for (int k=0; k<array_size(cur_system->astexclude); k++) {
               AsteroidExclusion *exc = &cur_system->astexclude[k];
               double ex, ey, ed;

               /* Ignore exclusion zones that shouldn't affect. */
               if (!exc->affects)
                  continue;

               ex = a->pos.x - exc->pos.x;
               ey = a->pos.y - exc->pos.y;
               ed = pow2(ex) + pow2(ey);
               if (ed <= pow2(exc->radius)) {
                  ed = sqrt(ed);
                  a->vel.x += ast->thrust * dt * ex / ed;
                  a->vel.y += ast->thrust * dt * ey / ed;
                  setvel = 1;
               }
            }
         }

         if (setvel) {
            /* Enforce max speed. */
            d = MOD(a->vel.x, a->vel.y);
            if (d > ast->maxspeed) {
               a->vel.x *= ast->maxspeed / d;
               a->vel.y *= ast->maxspeed / d;
            }
         }

         /* Update position. */
         a->pos.x += a->vel.x * dt;
         a->pos.y += a->vel.y * dt;

         a->timer -= dt;
         if (a->timer < 0.) {
            switch (a->state) {
               /* Transition states. */
               case ASTEROID_FG:
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
                  a->timer_max = a->timer = 30. + 60.*RNGF();
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
      }

      x = 0.;
      y = 0.;
      pplayer = pilot_get( PLAYER_ID );
      if (pplayer != NULL) {
         Solid *psolid = pplayer->solid;
         x = psolid->vel.x;
         y = psolid->vel.y;
      }

      if (!space_isSimulation()) {
         for (int j=0; j<ast->ndebris; j++) {
            Debris *d = &ast->debris[j];

            d->pos.x += (d->vel.x-x) * dt;
            d->pos.y += (d->vel.y-y) * dt;

            /* Check boundaries */
            if (d->pos.x > SCREEN_W + DEBRIS_BUFFER)
               d->pos.x -= SCREEN_W + 2.*DEBRIS_BUFFER;
            else if (d->pos.y > SCREEN_H + DEBRIS_BUFFER)
               d->pos.y -= SCREEN_H + 2.*DEBRIS_BUFFER;
            else if (d->pos.x < -DEBRIS_BUFFER)
               d->pos.x += SCREEN_W + 2.*DEBRIS_BUFFER;
            else if (d->pos.y < -DEBRIS_BUFFER)
               d->pos.y += SCREEN_H + 2.*DEBRIS_BUFFER;
         }
      }
   }
}

/**
 * @brief Initializes the system.
 */
void asteroids_init (void)
{
   /* Set up asteroids. */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      ast->id = i;

      /* Add the asteroids to the anchor */
      ast->asteroids = realloc( ast->asteroids, (ast->nb) * sizeof(Asteroid) );
      for (int j=0; j<ast->nb; j++) {
         double r = RNGF();
         Asteroid *a = &ast->asteroids[j];
         a->id = j;
         asteroid_init(a, ast);
         if (r > 0.6)
            a->state = ASTEROID_FG;
         else if (r > 0.8)
            a->state = ASTEROID_XB;
         else if (r > 0.9)
            a->state = ASTEROID_BX;
         else
            a->state = ASTEROID_XX;
         a->timer = a->timer_max = 30.*RNGF();
      }
      /* Add the debris to the anchor */
      ast->debris = realloc( ast->debris, (ast->ndebris) * sizeof(Debris) );
      for (int j=0; j<ast->ndebris; j++) {
         Debris *d = &ast->debris[j];
         debris_init(d);
      }
   }
}

/**
 * @brief Initializes an asteroid.
 *    @param ast Asteroid to initialize.
 *    @param field Asteroid field the asteroid belongs to.
 */
static void asteroid_init( Asteroid *ast, AsteroidAnchor *field )
{
   int id;
   double mod, theta;
   AsteroidType *at;
   int attempts = 0;

   ast->parent = field->id;
   ast->scanned = 0;

   /* randomly init the type of asteroid */
   id = RNG(0,array_size(field->type)-1);
   ast->type = field->type[id];
   /* randomly init the gfx ID */
   at = &asteroid_types[ast->type];
   ast->gfxID = RNG(0, array_size(at->gfxs)-1);
   ast->armour = at->armour;

   do {
      double angle = RNGF() * 2. * M_PI;
      double radius = RNGF() * field->radius;
      ast->pos.x = radius * cos(angle) + field->pos.x;
      ast->pos.y = radius * sin(angle) + field->pos.y;

      /* If this is the first time and it's spawned outside the field,
       * we get rid of it so that density remains roughly consistent. */
      if ((ast->state == ASTEROID_XX_TO_BG) &&
            (asteroids_inField(&ast->pos) < 0)) {
         ast->state = ASTEROID_XX;
         ast->timer_max = ast->timer = 10. + RNGF()*20.;
         return;
      }

      attempts++;
   } while ((asteroids_inField(&ast->pos) < 0) && (attempts < 1000));

   /* And a random velocity */
   theta = RNGF()*2.*M_PI;
   mod   = RNGF()*field->maxspeed;
   vect_pset( &ast->vel, mod, theta );

   /* Fade in stuff. */
   ast->state = ASTEROID_XX;
   ast->timer_max = ast->timer = -1.;
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
   vect_pset( &deb->vel, mod, theta );

   /* Randomly init the gfx ID */
   deb->gfxID = RNG(0,(int)nasterogfx-1);

   /* Random height vs player. */
   deb->height = 0.8 + RNGF()*0.4;
}

/**
 * @brief Parses a single asteroid field for a system.
 *
 *    @param node Parent node containing asteroid field information.
 *    @param sys System.
 *    @return 0 on success.
 */
static int system_parseAsteroidField( const xmlNodePtr node, StarSystem *sys )
{
   AsteroidAnchor *a;
   xmlNodePtr cur;
   int pos;

   /* Allocate more space. */
   a = &array_grow( &sys->asteroids );
   memset( a, 0, sizeof(AsteroidAnchor) );

   /* Initialize stuff. */
   pos         = 1;
   a->density  = 0.2;
   a->area     = 0.;
   a->type     = array_create( int );
   a->radius   = 0.;
   a->maxspeed = 20.;
   a->thrust   = 1.;
   vect_cset( &a->pos, 0., 0. );

   /* Parse label if available. */
   xmlr_attr_strd( node, "label", a->label );

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xmlr_float( cur,"density", a->density );
      xmlr_float( cur, "radius", a->radius );
      xmlr_float( cur, "maxspeed", a->maxspeed );
      xmlr_float( cur, "thrust", a->thrust );

      /* Handle types of asteroids. */
      if (xml_isNode(cur,"type")) {
         const char *name = xml_get(cur);
         const AsteroidType q = { .name=(char*)name };
         /* Find the ID */
         AsteroidType *at = bsearch( &q, asteroid_types, array_size(asteroid_types), sizeof(AsteroidType), asteroidTypes_cmp );
         if (at != NULL)
            array_push_back( &a->type, at-asteroid_types );
         else
            WARN("Unknown AsteroidType '%s' in StarSystem '%s'", name, sys->name);
         continue;
      }

      /* Handle position. */
      if (xml_isNode(cur,"pos")) {
         double x, y;
         pos = 1;
         xmlr_attr_float( cur, "x", x );
         xmlr_attr_float( cur, "y", y );

         /* Set position. */
         vect_cset( &a->pos, x, y );
         continue;
      }

   } while (xml_nextNode(cur));

   if (!pos)
      WARN(_("Asteroid field in %s has no position."), sys->name);

   if (a->radius == 0.)
      WARN(_("Asteroid field in %s has no radius."), sys->name);

   /* By default, take the first in the list. */
   if (array_size(a->type) == 0) {
      WARN(_("Asteroid field in system '%s' has no asteroid types defined!"),sys->name);
      array_push_back( &a->type, 0 );
   }

   /* Update internals. */
   asteroids_computeInternals( a );

   return 0;
}

/**
 * @brief Updates internal alues of an asteroid field.
 */
void asteroids_computeInternals( AsteroidAnchor *a )
{
   /* Calculate area */
   a->area = M_PI * a->radius * a->radius;

   /* Compute number of asteroids */
   a->nb      = floor( ABS(a->area) / ASTEROID_REF_AREA * a->density );
   a->ndebris = floor( 100.*a->density );

   /* Computed from your standard physics equations (with a bit of margin). */
   a->margin   = pow2(a->maxspeed) / (4.*a->thrust) + 50.;
}

/**
 * @brief Parses a single asteroid exclusion zone for a system.
 *
 *    @param node Parent node containing asteroid exclusion information.
 *    @param sys System.
 *    @return 0 on success.
 */
static int system_parseAsteroidExclusion( const xmlNodePtr node, StarSystem *sys )
{
   AsteroidExclusion *a;
   xmlNodePtr cur;
   double x, y;
   int pos;

   /* Allocate more space. */
   a = &array_grow( &sys->astexclude );
   memset( a, 0, sizeof(*a) );

   /* Initialize stuff. */
   pos         = 0;
   a->radius   = 0.;
   vect_cset( &a->pos, 0., 0. );

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes( cur );

      xmlr_float( cur, "radius", a->radius );

      /* Handle position. */
      if (xml_isNode(cur,"pos")) {
         pos = 1;
         xmlr_attr_float( cur, "x", x );
         xmlr_attr_float( cur, "y", y );

         /* Set position. */
         vect_cset( &a->pos, x, y );
      }

   } while (xml_nextNode(cur));

   if (!pos)
      WARN(_("Asteroid exclusion in %s has no position."), sys->name);

   if (a->radius == 0.)
      WARN(_("Asteroid exclusion in %s has no radius."), sys->name);

   return 0;
}

/**
 * @brief Loads the asteroid anchor into a system.
 *
 *    @param parent System parent node.
 *    @param sys System.
 */
void system_parseAsteroids( const xmlNodePtr parent, StarSystem *sys )
{
   xmlNodePtr node = parent->xmlChildrenNode;
   do { /* load all the data */
      xml_onlyNodes(node);
      if (xml_isNode(node,"asteroids")) {
         xmlNodePtr cur = node->children;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"asteroid"))
               system_parseAsteroidField( cur, sys );
            else if (xml_isNode(cur,"exclusion"))
               system_parseAsteroidExclusion( cur, sys );
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   array_shrink( &sys->asteroids );
   array_shrink( &sys->astexclude );
}

/**
 * @brief Loads the entire universe into ram - pretty big feat eh?
 *
 *    @return 0 on success.
 */
int asteroids_load (void)
{
   int ret;
   char **asteroid_files, file[PATH_MAX];

   /* Load asteroid types. */
   ret = asteroidTypes_load();
   if (ret < 0)
      return ret;

   /* Load asteroid graphics. */
   asteroid_files = PHYSFS_enumerateFiles( SPOB_GFX_SPACE_PATH"asteroid/" );
   for (nasterogfx=0; asteroid_files[nasterogfx]!=NULL; nasterogfx++) {}
   asteroid_gfx = malloc( sizeof(glTexture*) * nasterogfx );

   for (size_t i=0; asteroid_files[i]!=NULL; i++) {
      snprintf( file, sizeof(file), "%s%s", SPOB_GFX_SPACE_PATH"asteroid/", asteroid_files[i] );
      asteroid_gfx[i] = gl_newImage( file, OPENGL_TEX_MIPMAPS );
   }

   return 0;
}

static int asteroidTypes_cmp( const void *p1, const void *p2 )
{
   const AsteroidType *at1, *at2;
   at1 = (const AsteroidType*) p1;
   at2 = (const AsteroidType*) p2;
   return strcmp(at1->name,at2->name);
}

/**
 * @brief Loads the asteroids types.
 *
 *    @return 0 on success.
 */
static int asteroidTypes_load (void)
{
   char **asteroid_files = ndata_listRecursive( ASTEROID_DATA_PATH );
   asteroid_types = array_create( AsteroidType );
   for (int i=0; i < array_size( asteroid_files ); i++) {
      if (ndata_matchExt( asteroid_files[i], "xml" )) {
         int ret = asteroidTypes_parse( &array_grow(&asteroid_types), asteroid_files[i] );
         if (ret < 0) {
            int n = array_size(asteroid_types);
            array_erase( &asteroid_types, &asteroid_types[n-1], &asteroid_types[n] );
         }
      }
      free( asteroid_files[i] );
   }
   array_free( asteroid_files );

   qsort( asteroid_types, array_size(asteroid_types), sizeof(AsteroidType), asteroidTypes_cmp );

   /* Shrink to minimum. */
   array_shrink( &asteroid_types );

   return 0;
}

/**
 * @brief Parses the XML of an asteroid type.
 *
 *    @param[out] at Outfit asteroid type.
 *    @param file File containing the XML information.
 */
static int asteroidTypes_parse( AsteroidType *at, const char *file )
{
   char *str;
   xmlNodePtr parent, node, cur;
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
   at->material   = array_create( Commodity* );
   at->quantity   = array_create( int );

   xmlr_attr_strd(parent,"name",at->name);
   if (at->name == NULL)
      WARN(_("Asteroid '%s' has invalid or no name"), file);

   node = parent->xmlChildrenNode;
   do {
      /* Only handle nodes. */
      xml_onlyNodes(node);

      xmlr_float( node, "armour", at->armour );

      if (xml_isNode(node,"gfx")) {
         array_push_back( &at->gfxs, xml_parseTexture( node, SPOB_GFX_SPACE_PATH"asteroid/%s", 1, 1,  OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS ) );
         continue;
      }
      else if (xml_isNode(node,"commodity")) {
         /* Check that name and quantity are defined. */
         int namdef = 0;
         int qttdef = 0;

         cur = node->xmlChildrenNode;
         do {
            xml_onlyNodes(cur);
            if (xml_isNode(cur,"name")) {
               str = xml_get(cur);
               array_push_back( &at->material, commodity_get( str ) );
               namdef = 1;
               continue;
            }
            else if (xml_isNode(cur,"quantity")) {
               array_push_back( &at->quantity, xml_getInt(cur) );
               qttdef = 1;
               continue;
            }
            WARN(_("Asteroid has unknown node '%s'"), cur->name);
         } while (xml_nextNode(cur));

         if (namdef == 0 || qttdef == 0)
            WARN(_("Asteroid type's commodity lacks name or quantity."));
         continue;
      }
      WARN(_("Asteroid has unknown node '%s'"), node->name);
   } while (xml_nextNode(node));

   if (array_size(at->gfxs)==0)
      WARN(_("Asteroid type has no gfx associated."));

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}

/**
 * @brief Renders the system overlay.
 */
void asteroids_renderOverlay (void)
{
   /* Render the debris. */
   Pilot *pplayer = pilot_get( PLAYER_ID );
   if (pplayer != NULL) {
      Solid *psolid  = pplayer->solid;
      for (int i=0; i < array_size(cur_system->asteroids); i++) {
         double x, y;
         AsteroidAnchor *ast = &cur_system->asteroids[i];
         x = psolid->pos.x - SCREEN_W/2;
         y = psolid->pos.y - SCREEN_H/2;
         for (int j=0; j < ast->ndebris; j++) {
           if (ast->debris[j].height > 1.)
              space_renderDebris( &ast->debris[j], x, y );
         }
      }
   }
}

/**
 * @brief Renders the current systems' spobs.
 */
void asteroids_render (void)
{
   Pilot *pplayer;
   Solid *psolid;

   /* Get the player in order to compute the offset for debris. */
   pplayer = pilot_get( PLAYER_ID );
   if (pplayer != NULL)
      psolid  = pplayer->solid;

   /* Render the asteroids & debris. */
   for (int i=0; i < array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      for (int j=0; j < ast->nb; j++)
        space_renderAsteroid( &ast->asteroids[j] );

      if (pplayer != NULL) {
         double x = psolid->pos.x - SCREEN_W/2;
         double y = psolid->pos.y - SCREEN_H/2;
         for (int j=0; j < ast->ndebris; j++) {
           if (ast->debris[j].height < 1.)
              space_renderDebris( &ast->debris[j], x, y );
         }
      }
   }

   /* Render gatherable stuff. */
   gatherable_render();
}

/**
 * @brief Renders an asteroid.
 */
static void space_renderAsteroid( const Asteroid *a )
{
   double nx, ny;
   AsteroidType *at;
   char c[32];
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

   at = &asteroid_types[a->type];
   gl_renderSprite( at->gfxs[a->gfxID], a->pos.x, a->pos.y, 0, 0, &col );

   /* Add the commodities if scanned. */
   if (!a->scanned || (a->state != ASTEROID_FG))
      return;
   gl_gameToScreenCoords( &nx, &ny, a->pos.x, a->pos.y );
   for (int i=0; i<array_size(at->material); i++) {
      Commodity *com = at->material[i];
      gl_renderSprite( com->gfx_space, a->pos.x, a->pos.y-10.*i, 0, 0, NULL );
      snprintf(c, sizeof(c), "x%i", at->quantity[i]);
      gl_printRaw( &gl_smallFont, nx+10, ny-5-10.*i, &cFontWhite, -1., c );
   }
}

/**
 * @brief Renders a debris.
 */
static void space_renderDebris( const Debris *d, double x, double y )
{
   double scale = 0.5;
   Vector2d v;

   v.x = d->pos.x + x;
   v.y = d->pos.y + y;

   if (asteroids_inField( &v ) == 0)
      gl_renderSpriteInterpolateScale( asteroid_gfx[d->gfxID], asteroid_gfx[d->gfxID], 1,
                                       v.x, v.y, scale, scale, 0, 0, &cInert );
}

/**
 * @brief Cleans up the system.
 */
void asteroids_free (void)
{
   /* Free asteroid graphics. */
   for (int i=0; i<(int)nasterogfx; i++)
      gl_freeTexture(asteroid_gfx[i]);
   free(asteroid_gfx);

   /* Free the asteroid types. */
   for (int i=0; i < array_size(asteroid_types); i++) {
      AsteroidType *at = &asteroid_types[i];
      free(at->name);
      array_free(at->material);
      array_free(at->quantity);
      for (int j=0; j<array_size(at->gfxs); j++)
         gl_freeTexture(at->gfxs[j]);
      array_free(at->gfxs);
   }
   array_free(asteroid_types);
   asteroid_types = NULL;

   /* Free the gatherable stack. */
   gatherable_free();
}

/**
 * @brief See if the position is in an asteroid field.
 *
 *    @param p pointer to the position.
 *    @return -1 If false; index of the field otherwise.
 */
int asteroids_inField( const Vector2d *p )
{
   /* Always return -1 if in an exclusion zone */
   for (int i=0; i < array_size(cur_system->astexclude); i++) {
      AsteroidExclusion *e = &cur_system->astexclude[i];
      if (vect_dist2( p, &e->pos ) <= pow2(e->radius))
         return -1;
   }

   /* Check if in asteroid field */
   for (int i=0; i < array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *a = &cur_system->asteroids[i];
      if (vect_dist2( p, &a->pos ) <= pow2(a->radius))
         return i;
   }

   return -1;
}

/**
 * @brief Returns the asteroid type corresponding to an ID
 *
 *    @param ID ID of the type.
 *    @return AsteroidType object.
 */
const AsteroidType *space_getType ( int ID )
{
   return &asteroid_types[ ID ];
}

/**
 * @brief Hits an asteroid.
 *
 *    @param a hit asteroid
 *    @param dmg Damage being done
 */
void asteroid_hit( Asteroid *a, const Damage *dmg )
{
   double darmour;
   dtype_calcDamage( NULL, &darmour, 1, NULL, dmg, NULL );

   a->armour -= darmour;
   if (a->armour <= 0) {
      a->state = ASTEROID_BG_TO_XX;
      a->timer_max = a->timer = 0.5;

      asteroid_explode( a, &cur_system->asteroids[a->parent], 1 );
   }
}

/**
 * @brief Makes an asteroid explode.
 *
 *    @param a asteroid to make explode
 *    @param field Asteroid field the asteroid belongs to.
 *    @param give_reward Whether a pilot blew the asteroid up and should be rewarded.
 */
static void asteroid_explode( Asteroid *a, AsteroidAnchor *field, int give_reward )
{
   Damage dmg;
   char buf[16];

   /* Manage the explosion */
   dmg.type          = dtype_get("explosion_splash");
   dmg.damage        = 100.;
   dmg.penetration   = 1.; /* Full penetration. */
   dmg.disable       = 0.;
   expl_explode( a->pos.x, a->pos.y, a->vel.x, a->vel.y,
                 50., &dmg, NULL, EXPL_MODE_SHIP );

   /* Play random explosion sound. */
   snprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
   sound_playPos( sound_get(buf), a->pos.x, a->pos.y, a->vel.x, a->vel.y );

   if (give_reward) {
      /* Release commodity. */
      AsteroidType *at = &asteroid_types[a->type];

      for (int i=0; i < array_size(at->material); i++) {
         int nb = RNG(0,at->quantity[i]);
         Commodity *com = at->material[i];
         for (int j=0; j < nb; j++) {
            Vector2d pos, vel;
            pos = a->pos;
            vel = a->vel;
            pos.x += (RNGF()*30.-15.);
            pos.y += (RNGF()*30.-15.);
            vel.x += (RNGF()*20.-10.);
            vel.y += (RNGF()*20.-10.);
            gatherable_init( com, pos, vel, -1., RNG(1,5) );
         }
      }
   }

   /* Remove the asteroid target to any pilot. */
   pilot_untargetAsteroid( a->parent, a->id );

   /* Make it respawn elsewhere */
   asteroid_init( a, field );
   a->state = ASTEROID_XX;
   a->timer_max = a->timer = 10. + RNGF()*20.;
}
