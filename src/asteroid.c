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

#include "array.h"
#include "camera.h"
#include "space.h"
#include "opengl.h"
#include "toolkit.h"
#include "ndata.h"
#include "player.h"

#define DEBRIS_BUFFER         1000 /**< Buffer to smooth appearance of debris */

static const double scan_fade = 10.; /**< 1/time it takes to fade in/out scanning text. */

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
static int astgroup_cmp( const void *p1, const void *p2 );
static int astgroup_parse( AsteroidTypeGroup *ag, const char *file );
static int system_parseAsteroidField( const xmlNodePtr node, StarSystem *sys );
static int system_parseAsteroidExclusion( const xmlNodePtr node, StarSystem *sys );
static int asttype_load (void);

static void space_renderAsteroid( const Asteroid *a );
static void space_renderDebris( const Debris *d, double x, double y );
static void debris_init( Debris *deb );
static int asteroid_init( Asteroid *ast, const AsteroidAnchor *field );


/**
 * @brief Controls fleet spawning.
 *
 *    @param dt Current delta tick.
 */
void asteroids_update( double dt )
{
   /* Asteroids/Debris update */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
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

         /* Update scanned state if necessary. */
         if (a->scanned) {
            if (a->state == ASTEROID_FG)
               a->scan_alpha = MIN( a->scan_alpha+scan_fade*dt, 1.);
            else
               a->scan_alpha = MAX( a->scan_alpha-scan_fade*dt, 0.);
         }

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

      /* Only have to update stuff if not simulating. */
      if (!space_isSimulation()) {
         double x, y;
         if (player.p != NULL) {
            Solid *psolid = player.p->solid;
            x = psolid->vel.x;
            y = psolid->vel.y;
         }
         else {
            x = 0.;
            y = 0.;
         }

         for (int j=0; j<ast->ndebris; j++) {
            Debris *d = &ast->debris[j];
            int infield;
            Vector2d v;

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
}

/**
 * @brief Initializes the system.
 */
void asteroids_init (void)
{
   asteroid_creating = 1;
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
      }
      /* Add the debris to the anchor */
      ast->debris = realloc( ast->debris, (ast->ndebris) * sizeof(Debris) );
      for (int j=0; j<ast->ndebris; j++) {
         Debris *d = &ast->debris[j];
         debris_init(d);
      }
   }
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
   AsteroidType *at = NULL;
   int outfield;
   int attempts = 0;

   ast->parent  = field->id;
   ast->scanned = 0;
   r2 = pow2( field->radius );

   do {
      /* Try to keep density uniform using cartesian coordinates. */
      ast->pos.x = field->pos.x + (RNGF()*2.-1.)*field->radius;
      ast->pos.y = field->pos.y + (RNGF()*2.-1.)*field->radius;

      /* Check if out of the field. */
      outfield = (asteroids_inField(&ast->pos) < 0);

      /* If this is the first time and it's spawned outside the field,
       * we get rid of it so that density remains roughly consistent. */
      if (asteroid_creating && outfield && (vect_dist2( &ast->pos, &field->pos ) < r2)) {
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

   /* Randomly init the gfx ID */
   ast->type = at;
   ast->gfx = asteroid_gfx[ RNG(0, array_size(at->gfxs)-1) ];
   ast->armour = at->armour_min + RNGF() * (at->armour_max-at->armour_min);

   /* And a random velocity */
   theta = RNGF()*2.*M_PI;
   mod   = RNGF()*field->maxspeed;
   vect_pset( &ast->vel, mod, theta );

   /* Fade in stuff. */
   ast->state = ASTEROID_XX;
   ast->timer_max = ast->timer = -1.;

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
   vect_pset( &deb->vel, mod, theta );

   /* Randomly init the gfx ID */
   deb->gfx = asteroid_gfx[ RNG(0,(int)array_size(asteroid_gfx)-1) ];

   /* Random height vs player. */
   deb->height = 0.8 + RNGF()*0.4;
   deb->alpha = 0.;
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
   a->density  = ASTEROID_DEFAULT_DENSITY;
   a->groups   = array_create( AsteroidTypeGroup* );
   a->groupsw  = array_create( double );
   a->radius   = 0.;
   a->maxspeed = ASTEROID_DEFAULT_MAXSPEED;
   a->thrust   = ASTEROID_DEFAULT_THRUST;

   /* Parse label if available. */
   xmlr_attr_strd( node, "label", a->label );

   /* Parse data. */
   cur = node->xmlChildrenNode;
   do {
      xml_onlyNodes(cur);

      xmlr_float( cur, "density", a->density );
      xmlr_float( cur, "radius", a->radius );
      xmlr_float( cur, "maxspeed", a->maxspeed );
      xmlr_float( cur, "thrust", a->thrust );

      /* Handle types of asteroids. */
      if (xml_isNode(cur,"group")) {
         double w;
         const char *name = xml_get(cur);
         xmlr_attr_float_def(cur,"weight",w,1.);
         array_push_back( &a->groups, astgroup_getName(name) );
         array_push_back( &a->groupsw, w );
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

      WARN(_("Asteroid Field in Star System '%s' has unknown node '%s'"), sys->name, node->name);
   } while (xml_nextNode(cur));

   /* Update internals. */
   asteroids_computeInternals( a );

#define MELEMENT(o,s) \
if (o) WARN(_("Asteroid Field in Star System '%s' has missing/invalid '%s' element"), sys->name, s) /**< Define to help check for data errors. */
   MELEMENT(!pos,"pos");
   MELEMENT(a->radius<=0.,"radius");
   MELEMENT(array_size(a->groups)==0,"groups");
#undef MELEMENT

   return 0;
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
   a->ndebris = floor( 100.*a->density );

   /* Computed from your standard physics equations (with a bit of margin). */
   a->margin  = pow2(a->maxspeed) / (4.*a->thrust) + 50.;

   /* Compute weight total. */
   a->groupswtotal = 0.;
   for (int i=0; i<array_size(a->groupsw); i++)
      a->groupswtotal += a->groupsw[i];
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
   pos = 0;

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
         continue;
      }
      WARN(_("Asteroid Exclusion Zone in Star System '%s' has unknown node '%s'"), sys->name, node->name);
   } while (xml_nextNode(cur));

#define MELEMENT(o,s) \
if (o) WARN(_("Asteroid Exclusion Zone in Star System '%s' has missing/invalid '%s' element"), sys->name, s) /**< Define to help check for data errors. */
   MELEMENT(!pos,"pos");
   MELEMENT(a->radius<=0.,"radius");
#undef MELEMENT

   return 0;
}

/**
 * @brief Loads the asteroid anchor into a system.
 *
 *    @param parent System parent node.
 *    @param sys System.
 */
void asteroids_parse( const xmlNodePtr parent, StarSystem *sys )
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
         int ret = asttype_parse( &array_grow(&asteroid_types), asteroid_files[i] );
         if (ret < 0) {
            int n = array_size(asteroid_types);
            array_erase( &asteroid_types, &asteroid_types[n-1], &asteroid_types[n] );
         }
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
         int ret = astgroup_parse( &array_grow(&asteroid_groups), asteroid_files[i] );
         if (ret < 0) {
            int n = array_size(asteroid_groups);
            array_erase( &asteroid_groups, &asteroid_groups[n-1], &asteroid_groups[n] );
         }
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
   at->material   = array_create( AsteroidReward );
   at->damage     = 100;
   at->penetration = 100.;
   at->exp_radius = 50.;

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

      if (xml_isNode(node,"gfx")) {
         array_push_back( &at->gfxs, xml_parseTexture( node, SPOB_GFX_SPACE_PATH"asteroid/%s", 1, 1,  OPENGL_TEX_MAPTRANS | OPENGL_TEX_MIPMAPS ) );
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
   /* Render the debris. */
   if (player.p == NULL)
      return;
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      double x, y;
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      x = player.p->solid->pos.x - SCREEN_W/2;
      y = player.p->solid->pos.y - SCREEN_H/2;
      for (int j=0; j < ast->ndebris; j++) {
         if (ast->debris[j].height > 1.)
            space_renderDebris( &ast->debris[j], x, y );
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
   pplayer = player.p;
   if (pplayer != NULL)
      psolid  = pplayer->solid;

   /* Render the asteroids & debris. */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      for (int j=0; j<ast->nb; j++)
        space_renderAsteroid( &ast->asteroids[j] );

      if (pplayer != NULL) {
         double x = psolid->pos.x - SCREEN_W/2;
         double y = psolid->pos.y - SCREEN_H/2;
         for (int j=0; j<ast->ndebris; j++) {
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
   gl_renderSprite( a->gfx, a->pos.x, a->pos.y, 0, 0, &col );

   /* Add the commodities if scanned. */
   if (!a->scanned)
      return;
   col = cFontWhite;
   col.a = a->scan_alpha;
   gl_gameToScreenCoords( &nx, &ny, a->pos.x, a->pos.y );
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
static void space_renderDebris( const Debris *d, double x, double y )
{
   const double scale = 0.5;
   const glColour col = COL_ALPHA( cInert, d->alpha );

   gl_renderSpriteScale( d->gfx, d->pos.x+x, d->pos.y+y, scale, scale, 0, 0, &col );
}

/**
 * @brief Frees an asteroid anchor.
 *
 *    @param ast Asteroid anchor to free.
 */
void asteroid_free( AsteroidAnchor *ast )
{
   free(ast->label);
   free(ast->asteroids);
   free(ast->debris);
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

   /* Free the asteroid types. */
   for (int i=0; i<array_size(asteroid_types); i++) {
      AsteroidType *at = &asteroid_types[i];
      free(at->name);
      free(at->scanned_msg);
      array_free(at->material);
      for (int j=0; j<array_size(at->gfxs); j++)
         gl_freeTexture(at->gfxs[j]);
      array_free(at->gfxs);
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
   for (int i=0; i<array_size(cur_system->astexclude); i++) {
      AsteroidExclusion *e = &cur_system->astexclude[i];
      if (vect_dist2( p, &e->pos ) <= pow2(e->radius))
         return -1;
   }

   /* Check if in asteroid field */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *a = &cur_system->asteroids[i];
      if (vect_dist2( p, &a->pos ) <= pow2(a->radius))
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
   const AsteroidType *at = a->type;
   AsteroidAnchor *field = &cur_system->asteroids[a->parent];

   /* Manage the explosion */
   dmg.type          = dtype_get("explosion_splash");
   dmg.damage        = at->damage;
   dmg.penetration   = at->penetration; /* Full penetration. */
   dmg.disable       = 0.;
   expl_explode( a->pos.x, a->pos.y, a->vel.x, a->vel.y,
                 at->exp_radius, &dmg, NULL, EXPL_MODE_SHIP );

   /* Play random explosion sound. */
   snprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
   sound_playPos( sound_get(buf), a->pos.x, a->pos.y, a->vel.x, a->vel.y );

   /* Release commodity rewards. */
   if (max_rarity >= 0) {
      double prob, accum;
      int ndrops = 0;
      for (int i=0; i<array_size(at->material); i++) {
         AsteroidReward *mat = &at->material[i];
         if (mat->rarity > max_rarity)
            continue;
         ndrops++;
      }
      if (ndrops > 0) {
         double r = RNGF();
         prob = 1./(double)ndrops;
         accum = 0.;
         for (int i=0; i<array_size(at->material); i++) {
            AsteroidReward *mat = &at->material[i];
            if (mat->rarity > max_rarity)
               continue;
            accum += prob;
            if (r > accum)
               continue;

            int nb = RNG(0, round((double)mat->quantity * mining_bonus));
            for (int j=0; j<nb; j++) {
               Vector2d pos, vel;
               pos = a->pos;
               vel = a->vel;
               pos.x += (RNGF()*30.-15.);
               pos.y += (RNGF()*30.-15.);
               vel.x += (RNGF()*20.-10.);
               vel.y += (RNGF()*20.-10.);
               gatherable_init( mat->material, pos, vel, -1., RNG(1,5) );
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
