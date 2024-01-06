/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <float.h>
#include "SDL.h"
/** @endcond */

#include "map_overlay.h"

#include "array.h"
#include "conf.h"
#include "font.h"
#include "gui.h"
#include "quadtree.h"
#include "input.h"
#include "log.h"
#include "naev.h"
#include "ntracing.h"
#include "nstring.h"
#include "opengl.h"
#include "pilot.h"
#include "player.h"
#include "safelanes.h"
#include "space.h"

static const double OVERLAY_FADEIN = 1.0/3.0; /**< How long it takes to fade in newly discovered things. */

static IntList ovr_qtquery; /**< For querying collisions. */

/**
 * Structure for map overlay size.
 */
typedef struct MapOverlayRadiusConstraint_ {
   int i;      /**< this radius... */
   int j;      /**< plus this radius... */
   double dist;/**< ... is at most this big. */
} MapOverlayRadiusConstraint;

typedef enum ovr_marker_type_e {
   OVR_MARKER_POINT,
   OVR_MARKER_CIRCLE,
} ovr_marker_type_t;

/**
 * @brief An overlay map marker.
 */
typedef struct ovr_marker_s {
   unsigned int id;  /**< ID of the marker. */
   char *text;       /**< Marker display text. */
   ovr_marker_type_t type; /**< Marker type. */
   int refcount;     /**< Refcount. */
   vec2 pos;         /**< Center of the marker. */
   MapOverlayPos mo; /**< Map overlay display position. */
   union {
      struct {
         double r;
         vec2 textpos;
      } circle;
   } u; /**< Type data. */
} ovr_marker_t;
static unsigned int mrk_idgen = 0; /**< ID generator for markers. */
static ovr_marker_t *ovr_markers = NULL; /**< Overlay markers. */

static SafeLane* ovr_render_safelanes = NULL; /**< Render safe lanes. */

static Uint32 ovr_opened = 0; /**< Time last opened. */
static int ovr_open = 0;      /**< Is the overlay open? */
static double ovr_res = 10.;  /**< Resolution. */
static double ovr_dt = 0.; /**< For animations and stuff. */
static const double ovr_text_pixbuf = 5.; /**< Extra margin around overlay text. */
/* Rem: high pix_buffer ovr_text_pixbuff allows to do less iterations. */
typedef struct OverlayBounds_s {
   double t;
   double r;
   double b;
   double l;
   double w;
   double h;
   double x;
   double y;
} OverlayBounds_t;
static OverlayBounds_t ovr_bounds;

/* For autonav. */
static int autonav_pos = 0;
static vec2 autonav_pos_v;
static MapOverlayPos autonav_pos_mo;

/* For optimizing overlay layout. */
static MapOverlayPos **ovr_refresh_mo = NULL;
static const vec2 **ovr_refresh_pos = NULL;

/*
 * Prototypes
 */
static void force_collision( float *ox, float *oy,
      float x, float y, float w, float h,
      float mx, float my, float mw, float mh );
static void ovr_optimizeLayout( int items, const vec2** pos,
      MapOverlayPos** mo );
static void ovr_refresh_uzawa_overlap( float *forces_x, float *forces_y,
      float x, float y, float w, float h, const vec2** pos,
      MapOverlayPos** mo, int items, int self,
      float *offx, float *offy, float *offdx, float *offdy );
/* Render. */
static int ovr_safelaneKnown( SafeLane *sf, vec2 *posns[2] );
static void map_overlayToScreenPos( double *ox, double *oy, double x, double y );
/* Markers. */
static void ovr_mrkRenderAll( double res, int fg );
static void ovr_mrkCleanup(  ovr_marker_t *mrk );
static ovr_marker_t *ovr_mrkNew (void);

/**
 * @brief Check to see if the map overlay is open.
 */
int ovr_isOpen (void)
{
   return !!ovr_open;
}

/**
 * @brief Updates the bound parameters.
 */
static void ovr_boundsUpdate (void)
{
   ovr_bounds.w = SCREEN_W - ovr_bounds.l - ovr_bounds.r;
   ovr_bounds.h = SCREEN_H - ovr_bounds.t - ovr_bounds.b;
   ovr_bounds.x = ovr_bounds.w / 2. + ovr_bounds.l;
   ovr_bounds.y = ovr_bounds.h / 2. + ovr_bounds.b;
}

/**
 * @brief Sets the bounds for the overlay map (in pixels).
 *
 *    @param top Top bound.
 *    @param right Right bound.
 *    @param bottom Bottom bound.
 *    @param left Left bound.
 */
void ovr_boundsSet( double top, double right, double bottom, double left )
{
   ovr_bounds.t = top;
   ovr_bounds.r = right;
   ovr_bounds.b = bottom;
   ovr_bounds.l = left;
   ovr_boundsUpdate();
}

/**
 * @brief Gets the center of the overlay map (in pixels).
 *
 *    @param[out] x X center position.
 *    @param[out] y Y center position.
 */
void ovr_center( double *x, double *y )
{
   *x = ovr_bounds.x;
   *y = ovr_bounds.y;
}

/**
 * @brief Converts map positions to screen positions for rendering.
 */
static void map_overlayToScreenPos( double *ox, double *oy, double x, double y )
{
   *ox = ovr_bounds.x + x / ovr_res;
   *oy = ovr_bounds.y + y / ovr_res;
}

/**
 * @brief Handles input to the map overlay.
 */
int ovr_input( SDL_Event *event )
{
   int mx, my;
   double x, y;

   /* We only want mouse events. */
   if (event->type != SDL_MOUSEBUTTONDOWN)
      return 0;

   /* Player must not be NULL. */
   if (player_isFlag(PLAYER_DESTROYED) || (player.p == NULL))
      return 0;

   /* Player must not be dead. */
   if (pilot_isFlag(player.p, PILOT_DEAD))
      return 0;

   /* Mouse targeting only uses left and right buttons. */
   if (event->button.button != SDL_BUTTON_LEFT &&
            event->button.button != SDL_BUTTON_RIGHT)
      return 0;

   /* Translate from window to screen. */
   mx = event->button.x;
   my = event->button.y;
   gl_windowToScreenPos( &mx, &my, mx, my );

   /* Translate to space coords. */
   x = ((double)mx - ovr_bounds.x) * ovr_res;
   y = ((double)my - ovr_bounds.y) * ovr_res;

   return input_clickPos( event, x, y, 1., 10. * ovr_res, 15. * ovr_res );
}

/**
 * @brief Refreshes the map overlay recalculating the dimensions it should have.
 *
 * This should be called if the spobs or the likes change at any given time.
 */
void ovr_refresh (void)
{
   double max_x, max_y;
   int n, items, jumpitems, spobitems;
   const vec2 **pos;
   MapOverlayPos **mo;
   char buf[STRMAX_SHORT];

   /* Must be open. */
   if (!ovr_isOpen())
      return;

   /* Clean up leftovers. */
   if (ovr_refresh_mo)
     free( ovr_refresh_mo );
   if (ovr_refresh_pos)
     free( ovr_refresh_pos );

   /* Update bounds if necessary. */
   ovr_boundsUpdate();

   /* Calculate max size. */
   items = 0;
   n = array_size(cur_system->jumps) + array_size(cur_system->spobs) + array_size(ovr_markers) + autonav_pos;
   pos = calloc( n, sizeof(vec2*) );
   mo  = calloc( n, sizeof(MapOverlayPos*) );
   max_x = 0.;
   max_y = 0.;
   if (autonav_pos) {
      max_x = MAX( max_x, ABS(autonav_pos_v.x) );
      max_y = MAX( max_y, ABS(autonav_pos_v.y) );
      pos[items] = &autonav_pos_v;
      mo[items]  = &autonav_pos_mo;
      mo[items]->radius = 9.; /* Gets set properly below. */
      mo[items]->text_width = gl_printWidthRaw( &gl_smallFont, _("TARGET") );
      items++;
   }
   for (int i=0; i<array_size(cur_system->jumps); i++) {
      JumpPoint *jp = &cur_system->jumps[i];
      max_x = MAX( max_x, ABS(jp->pos.x) );
      max_y = MAX( max_y, ABS(jp->pos.y) );
      if (!jp_isUsable(jp))
         continue;
      /* Initialize the map overlay stuff. */
      snprintf( buf, sizeof(buf), "%s%s", jump_getSymbol(jp), sys_isKnown(jp->target) ? _(jp->target->name) : _("Unknown") );
      pos[items] = &jp->pos;
      mo[items]  = &jp->mo;
      mo[items]->radius = jumppoint_gfx->sw / 2.;
      mo[items]->text_width = gl_printWidthRaw(&gl_smallFont, buf);
      items++;
   }
   jumpitems = items;
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *pnt = cur_system->spobs[i];
      max_x = MAX( max_x, ABS(pnt->pos.x) );
      max_y = MAX( max_y, ABS(pnt->pos.y) );
      if (!spob_isKnown(pnt))
         continue;
      /* Initialize the map overlay stuff. */
      snprintf( buf, sizeof(buf), "%s%s", spob_getSymbol(pnt), spob_name(pnt) );
      pos[items] = &pnt->pos;
      mo[items]  = &pnt->mo;
      mo[items]->radius = pnt->radius / 2.;  /* halved since it's awkwardly large if drawn to scale relative to the player. */
      /* +2.0 represents a margin used by the SDF shader. */
      mo[items]->text_width = gl_printWidthRaw( &gl_smallFont, buf );
      items++;
   }
   spobitems = items;
   for (int i=0; i<array_size(ovr_markers); i++) {
      double r;
      ovr_marker_t *mrk = &ovr_markers[i];
      max_x = MAX( max_x, ABS(mrk->pos.x) );
      max_y = MAX( max_y, ABS(mrk->pos.y) );
      if (mrk->text==NULL)
         continue;
      /* Initialize the map overlay stuff. */
      mo[items] = &mrk->mo;
      switch (mrk->type) {
         case OVR_MARKER_POINT:
            pos[items] = &mrk->pos;
            r = 13.; /* Will get set approprietaly with the max. */
            break;
         case OVR_MARKER_CIRCLE:
            pos[items] = &mrk->u.circle.textpos;
            r = 13.; /* We're not using the full area. */
            break;
      }
      mo[items]->radius = r;
      mo[items]->text_width = gl_printWidthRaw( &gl_smallFont, mrk->text );
      items++;
   }

   /* We need to calculate the radius of the rendering from the maximum radius of the system. */
   ovr_res = 2. * 1.2 * MAX( max_x / ovr_bounds.w, max_y / ovr_bounds.h );
   ovr_res = MAX( ovr_res, 25. );
   for (int i=0; i<items; i++) {
      double rm;
      if (autonav_pos && (i==0))
         rm = 9.;
      else if (i<jumpitems)
         rm = 5.;
      else if (i<spobitems)
         rm = 7.5;
      else
         rm = 13.;
      mo[i]->radius = MAX( 2.+mo[i]->radius / ovr_res, rm );
   }

   /* Compute text overlap and try to minimize it. */
   ovr_optimizeLayout( items, pos, mo );

   /* Sove the moos. */
   ovr_refresh_pos = pos;
   ovr_refresh_mo = mo;
}

/**
 * @brief Makes a best effort to fit the given spobs' overlay indicators and labels fit without collisions.
 */
static void ovr_optimizeLayout( int items, const vec2** pos, MapOverlayPos** mo )
{
   float cx, cy, r, sx, sy;
   float x, y, w, h, mx, my, mw, mh;
   float fx, fy, best, bx, by;
   float *forces_xa, *forces_ya, *off_buffx, *off_buffy, *off_0x, *off_0y, old_bx, old_by, *off_dx, *off_dy;

   /* Parameters for the map overlay optimization. */
   const int max_iters = 15;    /**< Maximum amount of iterations to do. */
   const float kx      = 0.015; /**< x softness factor. */
   const float ky      = 0.045; /**< y softness factor (moving along y is more likely to be the right solution). */
   const float eps_con = 1.3;   /**< Convergence criterion. */

   /* Nothing to do. */
   if (items <= 0)
      return;

   /* Fix radii which fit together. */
   MapOverlayRadiusConstraint cur, *fits = array_create(MapOverlayRadiusConstraint);
   uint8_t *must_shrink = malloc( items );
   for (cur.i=0; cur.i<items; cur.i++)
      for (cur.j=cur.i+1; cur.j<items; cur.j++) {
         cur.dist = hypot( pos[cur.i]->x - pos[cur.j]->x, pos[cur.i]->y - pos[cur.j]->y ) / ovr_res;
         if (cur.dist < mo[cur.i]->radius + mo[cur.j]->radius)
            array_push_back( &fits, cur );
      }
   for (int iter=0; (iter<max_iters) && (array_size(fits) > 0); iter++) {
      float shrink_factor = 0.;
      memset( must_shrink, 0, items );
      for (int i=0; i < array_size( fits ); i++) {
         r = fits[i].dist / (mo[fits[i].i]->radius + mo[fits[i].j]->radius);
         if (r >= 1)
            array_erase( &fits, &fits[i], &fits[i+1] );
         else {
            shrink_factor = MAX( shrink_factor, r - FLT_EPSILON );
            must_shrink[fits[i].i] = must_shrink[fits[i].j] = 1;
         }
      }
      for (int i=0; i<items; i++)
         if (must_shrink[i])
            mo[i]->radius *= shrink_factor;

   }
   free( must_shrink );
   array_free( fits );

   /* Limit shrinkage. */
   for (int i=0; i<items; i++)
      mo[i]->radius = MAX( mo[i]->radius, 4. );

   /* Initialization offset list. */
   off_0x = calloc( items, sizeof(float) );
   off_0y = calloc( items, sizeof(float) );

   /* Initialize all items. */
   for (int i=0; i<items; i++) {
      /* Test to see what side is best to put the text on.
       * We actually compute the text overlap also so hopefully it will alternate
       * sides when stuff is clustered together. */
      x = pos[i]->x/ovr_res - ovr_text_pixbuf;
      y = pos[i]->y/ovr_res - ovr_text_pixbuf;
      w = mo[i]->text_width + 2.*ovr_text_pixbuf;
      h = gl_smallFont.h + 2.*ovr_text_pixbuf;

      const float tx[4] = { mo[i]->radius+ovr_text_pixbuf+0.1, -mo[i]->radius-0.1-w, -mo[i]->text_width/2. , -mo[i]->text_width/2. };
      const float ty[4] = { -gl_smallFont.h/2.,  -gl_smallFont.h/2., mo[i]->radius+ovr_text_pixbuf+0.1, -mo[i]->radius-0.1-h };

      /* Check all combinations. */
      bx = 0.;
      by = 0.;
      best = HUGE_VALF;
      for (int k=0; k<4; k++) {
         double val = 0.;

         /* Test intersection with the spob indicators. */
         for (int j=0; j<items; j++) {
            fx = fy = 0.;
            mw = 2.*mo[j]->radius;
            mh = mw;
            mx = pos[j]->x/ovr_res - mw/2.;
            my = pos[j]->y/ovr_res - mh/2.;

            force_collision( &fx, &fy, x+tx[k], y+ty[k], w, h, mx, my, mw, mh );

            val += ABS(fx) + ABS(fy);
         }
         /* Keep best. */
         if (k == 0 || val < best) {
            bx = tx[k];
            by = ty[k];
            best = val;
         }
         if (val==0.)
            break;
      }

      /* Store offsets. */
      off_0x[i] = bx;
      off_0y[i] = by;
   }

   /* Uzawa optimization algorithm.
    * We minimize the (weighted) L2 norm of vector of offsets and radius changes
    * Under the constraint of no interpenetration
    * As the algorithm is Uzawa, this constraint won't necessary be attained.
    * This is similar to a contact problem is mechanics. */

   /* Initialize the matrix that stores the dual variables (forces applied between objects).
    * matrix is column-major, this means it is interesting to store in each column the forces
    * received by a given object. Then these forces are summed to obtain the total force on the object.
    * Odd lines are forces from objects and Even lines from other texts. */

   forces_xa = calloc( 2*items*items, sizeof(float) );
   forces_ya = calloc( 2*items*items, sizeof(float) );

   /* And buffer lists. */
   off_buffx = calloc( items, sizeof(float) );
   off_buffy = calloc( items, sizeof(float) );
   off_dx    = calloc( items, sizeof(float) );
   off_dy    = calloc( items, sizeof(float) );

   /* Main Uzawa Loop. */
   for (int iter=0; iter<max_iters; iter++) {
      double val = 0.; /* This stores the stagnation indicator. */
      for (int i=0; i<items; i++) {
         cx = pos[i]->x / ovr_res;
         cy = pos[i]->y / ovr_res;
         /* Compute the forces. */
         ovr_refresh_uzawa_overlap(
               forces_xa, forces_ya,
               cx + off_dx[i] + off_0x[i] - ovr_text_pixbuf,
               cy + off_dy[i] + off_0y[i] - ovr_text_pixbuf,
               mo[i]->text_width + 2*ovr_text_pixbuf,
               gl_smallFont.h + 2*ovr_text_pixbuf,
               pos, mo, items, i, off_0x, off_0y, off_dx, off_dy );

         /* Do the sum. */
         sx = sy = 0.;
         for (int j=0; j<2*items; j++) {
            sx += forces_xa[2*items*i+j];
            sy += forces_ya[2*items*i+j];
         }

         /* Store old version of buffers. */
         old_bx = off_buffx[i];
         old_by = off_buffy[i];

         /* Update positions (in buffer). Diagonal stiffness. */
         off_buffx[i] = kx * sx;
         off_buffy[i] = ky * sy;

         val = MAX( val, ABS(old_bx-off_buffx[i]) + ABS(old_by-off_buffy[i]) );
      }

      /* Offsets are actually updated once the first loop is over. */
      for (int i=0; i<items; i++) {
         off_dx[i] = off_buffx[i];
         off_dy[i] = off_buffy[i];
      }

      /* Test stagnation. */
      if (val <= eps_con)
         break;
   }

   /* Permanently add the initialization offset to total offset. */
   for (int i=0; i<items; i++) {
      mo[i]->text_offx = off_dx[i] + off_0x[i];
      mo[i]->text_offy = off_dy[i] + off_0y[i];
   }

   /* Free the forces matrix and the various buffers. */
   free( forces_xa );
   free( forces_ya );
   free( off_buffx );
   free( off_buffy );
   free( off_0x );
   free( off_0y );
   free( off_dx );
   free( off_dy );
}

/**
 * @brief Compute a collision between two rectangles and direction to deduce the force.
 */
static void force_collision( float *ox, float *oy,
      float x, float y, float w, float h,
      float mx, float my, float mw, float mh )
{
   /* No contact because of y offset (+tolerance). */
   if ((y+h < my+ovr_text_pixbuf) || (y+ovr_text_pixbuf > my+mh))
      *ox = 0.;
   else {
      /* Case A is left of B. */
      if (x+0.5*w < mx+0.5*mw) {
         *ox += mx-(x+w);
         *ox = MIN(0., *ox);
      }
      /* Case A is to the right of B. */
      else {
         *ox += (mx+mw)-x;
         *ox = MAX(0., *ox);
      }
   }

   /* No contact because of x offset (+tolerance). */
   if ((x+w < mx+ovr_text_pixbuf) || (x+ovr_text_pixbuf > mx+mw))
      *oy = 0.;
   else {
      /* Case A is below B. */
      if (y+0.5*h < my+0.5*mh) {
         *oy += my-(y+h);
         *oy = MIN(0., *oy);
      }
      /* Case A is above B. */
      else {
         *oy += (my+mh)-y;
         *oy = MAX(0., *oy);
      }
   }
}

/**
 * @brief Compute how an element overlaps with text and force to move away.
 */
static void ovr_refresh_uzawa_overlap( float *forces_x, float *forces_y,
      float x, float y, float w, float h, const vec2** pos,
      MapOverlayPos** mo, int items, int self,
      float *offx, float *offy, float *offdx, float *offdy )
{
   for (int i=0; i<items; i++) {
      float mx, my, mw, mh;
      const float pb2 = ovr_text_pixbuf*2.;

      /* Collisions with spob circles and jp triangles (odd indices). */
      mw = 2.*mo[i]->radius;
      mh = mw;
      mx = pos[i]->x/ovr_res - mw/2.;
      my = pos[i]->y/ovr_res - mh/2.;
      force_collision( &forces_x[2*items*self+2*i+1], &forces_y[2*items*self+2*i+1], x, y, w, h, mx, my, mw, mh );

      if (i == self)
         continue;

      /* Collisions with other texts (even indices) */
      mw = mo[i]->text_width + pb2;
      mh = gl_smallFont.h + pb2;
      mx = pos[i]->x/ovr_res + offdx[i] + offx[i] - ovr_text_pixbuf;
      my = pos[i]->y/ovr_res + offdy[i] + offy[i] - ovr_text_pixbuf;
      force_collision( &forces_x[2*items*self+2*i], &forces_y[2*items*self+2*i], x, y, w, h, mx, my, mw, mh );
   }
}

void ovr_initAlpha (void)
{
   SafeLane *safelanes;
   for (int i=0; i<array_size(cur_system->jumps); i++) {
      JumpPoint *jp = &cur_system->jumps[i];
      if (!jp_isUsable(jp))
         jp->map_alpha = 0.;
      else
         jp->map_alpha = 1.;
   }
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *pnt = cur_system->spobs[i];
      if (!spob_isKnown(pnt))
         pnt->map_alpha = 0.;
      else
         pnt->map_alpha = 1.;
   }

   safelanes = safelanes_get( -1, 0, cur_system );
   for (int i=0; i<array_size(safelanes); i++) {
      vec2 *posns[2];
      if (!ovr_safelaneKnown( &safelanes[i], posns ))
         safelanes[i].map_alpha = 0.;
      else
         safelanes[i].map_alpha = 1.;
   }
   array_free( ovr_render_safelanes );
   ovr_render_safelanes = safelanes;

   ovr_dt = 0.;
}

int ovr_init (void)
{
   il_create( &ovr_qtquery, 1 );
   return 0;
}

void ovr_exit (void)
{
   il_destroy( &ovr_qtquery );
}

/**
 * @brief Properly opens or closes the overlay map.
 *
 *    @param open Whether or not to open it.
 */
void ovr_setOpen( int open )
{
   if (open && !ovr_open) {
      ovr_open = 1;
      input_mouseShow();
      ovr_initAlpha();
   }
   else if (ovr_open) {
      ovr_open = 0;
      input_mouseHide();
      array_free( ovr_render_safelanes );
      ovr_render_safelanes = NULL;
      free( ovr_refresh_pos );
      ovr_refresh_pos = NULL;
      free( ovr_refresh_mo );
      ovr_refresh_mo = NULL;
   }
}

/**
 * @brief Handles a keypress event.
 *
 *    @param type Type of event.
 */
void ovr_key( int type )
{
   if (type > 0) {
      if (ovr_open)
         ovr_setOpen(0);
      else {
         ovr_setOpen(1);

         /* Refresh overlay size. */
         ovr_refresh();
         ovr_opened = SDL_GetTicks();
      }
   }
   else if (type < 0) {
      if (SDL_GetTicks() - ovr_opened > 300)
         ovr_setOpen(0);
   }
}

static int ovr_safelaneKnown( SafeLane *sf, vec2 *posns[2] )
{
   /* This is a bit asinine, but should be easily replaceable by decent code when we have a System Objects API.
      * Specifically, a generic pos and isKnown test would clean this up nicely. */
   int known = 1;
   for (int j=0; j<2; j++) {
      Spob *pnt;
      JumpPoint *jp;
      switch(sf->point_type[j]) {
         case SAFELANE_LOC_SPOB:
            pnt = spob_getIndex( sf->point_id[j] );
            posns[j] = &pnt->pos;
            if (!spob_isKnown( pnt ))
               known = 0;
            break;
         case SAFELANE_LOC_DEST_SYS:
            jp = jump_getTarget( system_getIndex( sf->point_id[j] ), cur_system );
            posns[j] = &jp->pos;
            if (!jp_isKnown( jp ))
               known = 0;
            break;
         default:
            ERR( _("Invalid vertex type.") );
      }
   }
   return known;
}

/**
 * @brief Renders the overlay map.
 *
 *    @param dt Current delta tick.
 */
void ovr_render( double dt )
{
   SafeLane *safelanes;
   double w, h, res;
   double x,y;
   double rx,ry, x2,y2, rw,rh;

   /* Must be open. */
   if (!ovr_open)
      return;

   /* Player must be alive. */
   if (player_isFlag( PLAYER_DESTROYED ) || (player.p == NULL))
      return;

   NTracingZone( _ctx, 1 );

   /* Have to clear for text. */
   glClear(GL_DEPTH_BUFFER_BIT);

   /* Default values. */
   w     = ovr_bounds.w;
   h     = ovr_bounds.h;
   res   = ovr_res;
   ovr_dt += dt;

   /* First render the background overlay. */
   glColour c = { .r=0., .g=0., .b=0., .a= conf.map_overlay_opacity };
   gl_renderRect( ovr_bounds.l, ovr_bounds.b, w, h, &c );

   /* Render the safe lanes */
   safelanes = safelanes_get( -1, 0, cur_system );
   for (int i=0; i<array_size(safelanes); i++) {
      glColour col;
      vec2 *posns[2];
      double r;
      if (!ovr_safelaneKnown( &safelanes[i], posns ))
         continue;

      /* Copy over alpha. FIXME: Based on past bug reports, we aren't successfully resetting ovr_render_safelanes
       * when the lane-set changes. We really want coherency rather than this array_size check. */
      if (i < array_size(ovr_render_safelanes))
         safelanes[i].map_alpha = ovr_render_safelanes[i].map_alpha;

      if (safelanes[i].map_alpha < 1.0)
         safelanes[i].map_alpha = MIN( safelanes[i].map_alpha+OVERLAY_FADEIN*dt, 1.0 );

      if (faction_isPlayerFriend( safelanes[i].faction ))
         col = cFriend;
      else if (faction_isPlayerEnemy( safelanes[i].faction ))
         col = cHostile;
      else
         col = cNeutral;
      col.a = 0.15 * MIN( safelanes[i].map_alpha, 1.0 );

      /* Get positions and stuff. */
      map_overlayToScreenPos( &x,  &y,  posns[0]->x, posns[0]->y );
      map_overlayToScreenPos( &x2, &y2, posns[1]->x, posns[1]->y );
      rx = x2-x;
      ry = y2-y;
      r  = atan2( ry, rx );
      rw = MOD(rx,ry)/2.;
      rh = 9.;

      /* Render. */
      glUseProgram(shaders.safelane.program);
      gl_renderShader( x+rx/2., y+ry/2., rw, rh, r, &shaders.safelane, &col, 1 );
   }
   array_free( ovr_render_safelanes );
   ovr_render_safelanes = safelanes;

   /* Render markers background. */
   ovr_mrkRenderAll( res, 0 );

   /* Check if player has goto target. */
   if (autonav_pos) {
      glColour col = cRadar_hilight;
      col.a = 0.6;
      map_overlayToScreenPos( &x, &y, autonav_pos_v.x, autonav_pos_v.y );
      glUseProgram( shaders.selectposition.program );
      gl_renderShader( x, y, 9., 9., 0., &shaders.selectposition, &col, 1 );
      gl_printMarkerRaw( &gl_smallFont, x+autonav_pos_mo.text_offx, y+autonav_pos_mo.text_offy, &cRadar_hilight, _("TARGET") );
   }

   /* Render spobs. */
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *pnt = cur_system->spobs[i];
      if (pnt->map_alpha < 1.0)
         pnt->map_alpha = MIN( pnt->map_alpha+OVERLAY_FADEIN*dt, 1.0 );
      if (i != player.p->nav_spob)
         gui_renderSpob( i, RADAR_RECT, w, h, res, pnt->map_alpha, 1 );
   }
   if (player.p->nav_spob > -1)
      gui_renderSpob( player.p->nav_spob, RADAR_RECT, w, h, res, cur_system->spobs[player.p->nav_spob]->map_alpha, 1 );

   /* Render jump points. */
   for (int i=0; i<array_size(cur_system->jumps); i++) {
      JumpPoint *jp = &cur_system->jumps[i];
      if (jp->map_alpha < 1.0)
         jp->map_alpha = MIN( jp->map_alpha+OVERLAY_FADEIN*dt, 1.0 );
      if ((i != player.p->nav_hyperspace) && !jp_isFlag(jp, JP_EXITONLY))
         gui_renderJumpPoint( i, RADAR_RECT, w, h, res, jp->map_alpha, 1 );
   }
   if (player.p->nav_hyperspace > -1)
      gui_renderJumpPoint( player.p->nav_hyperspace, RADAR_RECT, w, h, res, cur_system->jumps[player.p->nav_hyperspace].map_alpha, 1 );

   /* Render the asteroids */
   for (int i=0; i<array_size(cur_system->asteroids); i++) {
      AsteroidAnchor *ast = &cur_system->asteroids[i];
      double range = EW_ASTEROID_DIST * player.p->stats.ew_detect; /* TODO don't hardcode. */
      int ax, ay, r;
      ax = round(player.p->solid.pos.x);
      ay = round(player.p->solid.pos.y);
      r = ceil(range);
      asteroid_collideQueryIL( ast, &ovr_qtquery, ax-r, ay-r, ax+r, ay+r );
      for (int j=0; j<il_size(&ovr_qtquery); j++) {
         Asteroid *a = &ast->asteroids[ il_get( &ovr_qtquery, j, 0 ) ];
         gui_renderAsteroid( a, w, h, res, 1 );
      }
   }

   /* Render pilots. */
   Pilot *const* pstk = pilot_getAll();
   int t = 0;
   for (int i=0; i<array_size(pstk); i++) {
      if (pstk[i]->id == PLAYER_ID) /* Skip player. */
         continue;
      if (pstk[i]->id == player.p->target)
         t = i;
      else
         gui_renderPilot( pstk[i], RADAR_RECT, w, h, res, 1 );
   }

   /* Stealth rendering. */
   if (pilot_isFlag( player.p, PILOT_STEALTH )) {
      double detect;
      glColour col = {0., 0., 1., 1.};

      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.fbo[2] );
      glClearColor( 0., 0., 0., 0. );
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBlendEquation( GL_FUNC_ADD );
      glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE );
      for (int i=0; i<array_size(cur_system->asteroids); i++) {
         AsteroidAnchor *ast = &cur_system->asteroids[i];
         detect = vec2_dist2( &player.p->solid.pos, &ast->pos );
         if (detect < pow2(pilot_sensorRange() * player.p->stats.ew_detect + ast->radius)) {
            double r;
            map_overlayToScreenPos( &x, &y, ast->pos.x, ast->pos.y );
            r = ast->radius / res;
            glUseProgram( shaders.astaura.program );
            gl_renderShader( x, y, r, r, 0., &shaders.astaura, &col, 1 );
         }
      }

      glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
      glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE );
      for (int i=0; i<array_size(cur_system->astexclude); i++) {
         double r;
         AsteroidExclusion *aexcl = &cur_system->astexclude[i];
         map_overlayToScreenPos( &x, &y, aexcl->pos.x, aexcl->pos.y );
         r = aexcl->radius / res;
         glUseProgram( shaders.astaura.program );
         gl_renderShader( x, y, r, r, 0., &shaders.astaura, &col, 1 );
      }

      glBlendEquation( GL_MAX );
      for (int i=0; i<array_size(cur_system->jumps); i++) {
         double r;
         JumpPoint *jp = &cur_system->jumps[i];
         if (!jp_isUsable(jp) || jp_isFlag(jp,JP_HIDDEN))
            continue;
         map_overlayToScreenPos( &x, &y, jp->pos.x, jp->pos.y );
         r = EW_JUMP_BONUS_RANGE / res;
         glUseProgram( shaders.astaura.program );
         gl_renderShader( x, y, r, r, 0., &shaders.astaura, &col, 1 );
      }

      detect = player.p->ew_stealth / res;
      col.r = 1.;
      col.g = 0.;
      col.b = 0.;
      for (int i=0; i<array_size(pstk); i++) {
         double r;
         if (pilot_isDisabled(pstk[i]))
            continue;
         if (areAllies( player.p->faction, pstk[i]->faction ) || pilot_isFriendly(pstk[i]))
            continue;
         /* Only show pilots the player can see. */
         if (!pilot_validTarget( player.p, pstk[i] ))
            continue;
         map_overlayToScreenPos( &x, &y, pstk[i]->solid.pos.x, pstk[i]->solid.pos.y );
         r = detect * pstk[i]->stats.ew_detect; /* Already divided by res */
         if (r > 0.) {
            glUseProgram( shaders.stealthaura.program );
            gl_renderShader( x, y, r, r, 0., &shaders.stealthaura, &col, 1 );
         }
      }

      glBlendEquation( GL_FUNC_ADD );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
      glClearColor( 0., 0., 0., 1. );

      glUseProgram(shaders.stealthoverlay.program);
      glBindTexture( GL_TEXTURE_2D, gl_screen.fbo_tex[2] );

      glEnableVertexAttribArray( shaders.stealthoverlay.vertex );
      gl_vboActivateAttribOffset( gl_squareVBO, shaders.stealthoverlay.vertex,
            0, 2, GL_FLOAT, 0 );

      /* Set shader uniforms. */
      gl_uniformColour(shaders.stealthoverlay.colour, &cWhite);
      const mat4 ortho = mat4_ortho( 0., 1., 0., 1., 1., -1. );
      const mat4 I     = mat4_identity();
      gl_uniformMat4(shaders.stealthoverlay.projection, &ortho );
      gl_uniformMat4(shaders.stealthoverlay.tex_mat, &I );

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      /* Clear state. */
      glDisableVertexAttribArray( shaders.stealthoverlay.vertex );
   }

   /* Render markers foreground. */
   ovr_mrkRenderAll( res, 1 );

   /* Render the targeted pilot */
   if (t!=0)
      gui_renderPilot( pstk[t], RADAR_RECT, w, h, res, 1 );

   /* Render the player. */
   gui_renderPlayer( res, 1 );

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Renders all the markers.
 *
 *    @param res Resolution to render at.
 */
static void ovr_mrkRenderAll( double res, int fg )
{
   NTracingZone( _ctx, 1 );

   for (int i=0; i<array_size(ovr_markers); i++) {
      double x, y;
      ovr_marker_t *mrk = &ovr_markers[i];
      map_overlayToScreenPos( &x, &y, mrk->pos.x, mrk->pos.y );

      if (!fg) {
         double r;
         const glColour highlighted = COL_ALPHA( cRadar_hilight, 0.3 );

         switch (mrk->type) {
            case OVR_MARKER_POINT:
               glUseProgram( shaders.hilight.program );
               glUniform1f( shaders.hilight.dt, ovr_dt );
               gl_renderShader( x, y, 13., 13., 0., &shaders.hilight, &highlighted, 1 );
               break;

            case OVR_MARKER_CIRCLE:
               r = MAX( mrk->u.circle.r / res, 13. ); /* Don't allow to be smaller than a "point" */
               glUseProgram( shaders.hilight_circle.program );
               glUniform1f( shaders.hilight_circle.dt, ovr_dt );
               gl_renderShader( x, y, r, r, 0., &shaders.hilight_circle, &highlighted, 1 );
               break;
         }
      }

      if (fg && mrk->text != NULL) {
         switch (mrk->type) {
            case OVR_MARKER_POINT:
               gl_printMarkerRaw( &gl_smallFont, x+mrk->mo.text_offx, y+mrk->mo.text_offy, &cRadar_hilight, mrk->text );
               break;

            case OVR_MARKER_CIRCLE:
               map_overlayToScreenPos( &x, &y, mrk->u.circle.textpos.x, mrk->u.circle.textpos.y );
               gl_printMarkerRaw( &gl_smallFont, x+mrk->mo.text_offx, y+mrk->mo.text_offy, &cRadar_hilight, mrk->text );
               break;
         }
      }
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Frees up and clears all marker related stuff.
 */
void ovr_mrkFree (void)
{
   /* Clear markers. */
   ovr_mrkClear();

   /* Free array. */
   array_free( ovr_markers );
   ovr_markers = NULL;
   array_free( ovr_render_safelanes );
   ovr_render_safelanes = NULL;
}

/**
 * @brief Clears the current markers.
 */
void ovr_mrkClear (void)
{
   for (int i=0; i<array_size(ovr_markers); i++)
      ovr_mrkCleanup( &ovr_markers[i] );
   array_erase( &ovr_markers, array_begin(ovr_markers), array_end(ovr_markers) );

   /* Refresh if necessary. */
   if (ovr_open)
      ovr_refresh();
}

/**
 * @brief Clears up after an individual marker.
 *
 *    @param mrk Marker to clean up after.
 */
static void ovr_mrkCleanup( ovr_marker_t *mrk )
{
   free( mrk->text );
   mrk->text = NULL;
}

/**
 * @brief Creates a new marker.
 *
 *    @return The newly created marker.
 */
static ovr_marker_t *ovr_mrkNew (void)
{
   ovr_marker_t *mrk;

   if (ovr_markers == NULL)
      ovr_markers = array_create(  ovr_marker_t );

   mrk = &array_grow( &ovr_markers );
   memset( mrk, 0, sizeof( ovr_marker_t ) );
   mrk->id = ++mrk_idgen;
   mrk->refcount = 1;
   return mrk;
}

/**
 * @brief Creates a new point marker.
 *
 *    @param text Text to display with the marker.
 *    @param x X position of the marker.
 *    @param y Y position of the marker.
 *    @return The id of the newly created marker.
 */
unsigned int ovr_mrkAddPoint( const char *text, double x, double y )
{
   ovr_marker_t *mrk;

   /* Check existing ones first. */
   for (int i=0; i<array_size(ovr_markers); i++) {
      mrk = &ovr_markers[i];

      if (mrk->type != OVR_MARKER_POINT)
         continue;

      if (((text==NULL) && (mrk->text!=NULL)) ||
            ((text!=NULL) && ((mrk->text==NULL) || strcmp(text,mrk->text)!=0)))
         continue;

      if (hypotf( x-mrk->pos.x, y-mrk->pos.y ) > 1e-3)
         continue;

      /* Found same marker already! */
      mrk->refcount++;
      return mrk->id;
   }

   /* Create new one. */
   mrk = ovr_mrkNew();
   mrk->type = OVR_MARKER_POINT;
   if (text != NULL)
      mrk->text = strdup( text );
   vec2_cset( &mrk->pos, x, y );

   /* Refresh if necessary. */
   if (ovr_open)
      ovr_refresh();

   return mrk->id;
}

/**
 * @brief Creates a new circle marker.
 *
 *    @param text Text to display with the marker.
 *    @param x X position of the marker.
 *    @param y Y position of the marker.
 *    @param r Radius of the marker.
 *    @return The id of the newly created marker.
 */
unsigned int ovr_mrkAddCircle( const char *text, double x, double y, double r )
{
   ovr_marker_t *mrk;

   /* Check existing ones first. */
   for (int i=0; i<array_size(ovr_markers); i++) {
      mrk = &ovr_markers[i];

      if (mrk->type != OVR_MARKER_CIRCLE)
         continue;

      if (((text==NULL) && (mrk->text!=NULL)) ||
            ((text!=NULL) && ((mrk->text==NULL) || strcmp(text,mrk->text)!=0)))
         continue;

      if ((mrk->u.circle.r-r) > 1e-3 || hypotf( x-mrk->pos.x, y-mrk->pos.y ) > 1e-3)
         continue;

      /* Found same marker already! */
      mrk->refcount++;
      return mrk->id;
   }

   /* Create new one. */
   mrk = ovr_mrkNew();
   mrk->type = OVR_MARKER_CIRCLE;
   if (text != NULL)
      mrk->text = strdup( text );
   vec2_cset( &mrk->pos, x, y );
   mrk->u.circle.r = r;
   vec2_cset( &mrk->u.circle.textpos, x+r*M_SQRT1_2, y-r*M_SQRT1_2 );

   /* Refresh if necessary. */
   if (ovr_open)
      ovr_refresh();

   return mrk->id;
}

/**
 * @brief Removes a marker by id.
 *
 *    @param id ID of the marker to remove.
 */
void ovr_mrkRm( unsigned int id )
{
   for (int i=0; i<array_size(ovr_markers); i++) {
      ovr_marker_t *m = &ovr_markers[i];
      if (id != m->id)
         continue;

      m->refcount--;
      if (m->refcount > 0)
         return;

      ovr_mrkCleanup( m );
      array_erase( &ovr_markers, &m[0], &m[1] );

      /* Refresh if necessary. */
      if (ovr_open)
         ovr_refresh();
      break;
   }
}

void ovr_autonavPos( double x, double y )
{
   autonav_pos = 1;
   vec2_cset( &autonav_pos_v, x, y );

   /* Refresh if necessary. */
   if (ovr_open)
      ovr_refresh();
}

void ovr_autonavClear (void)
{
   autonav_pos = 0;

   /* Refresh if necessary. */
   if (ovr_open)
      ovr_refresh();
}
