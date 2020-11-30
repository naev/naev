/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map_overlay.h"

#include "naev.h"

#include "SDL.h"

#include "log.h"
#include "opengl.h"
#include "font.h"
#include "gui.h"
#include "pilot.h"
#include "player.h"
#include "space.h"
#include "input.h"
#include "array.h"
#include "conf.h"


/**
 * Structure for map overlay optimization.
 */
typedef struct MapOverlayPosOpt_ {
   /* Same as MapOverlayPos (double buffering). */
   float radius; /**< Radius for display on the map overlay. */
   float text_offx; /**< x offset of the caption text. */
   float text_offy; /**< y offset of the caption text. */
   /* Below are temporary values for optimization. */
   float radius_base; /**< Radius for display on the map overlay. */
   float text_offx_base; /**< Base x position of the caption text. */
   float text_offy_base; /**< Base y position of the caption text. */
   float text_width; /**< width of the caption text. */
} MapOverlayPosOpt;


/**
 * @brief An overlay map marker.
 */
typedef struct ovr_marker_s {
   unsigned int id; /**< ID of the marker. */
   char *text; /**< Marker display text. */
   int type; /**< Marker type. */
   union {
      struct {
         double x; /**< X center of point marker. */
         double y; /**< Y center of point marker. */
      } pt; /**< Point marker. */
   } u; /**< Type data. */
} ovr_marker_t;
static unsigned int mrk_idgen = 0; /**< ID generator for markers. */
static ovr_marker_t *ovr_markers = NULL; /**< Overlay markers. */


static Uint32 ovr_opened = 0; /**< Time last opened. */
static int ovr_open = 0; /**< Is the overlay open? */
static double ovr_res = 10.; /**< Resolution. */


/*
 * Prototypes
 */
static int update_collision( float *ox, float *oy, float weight,
      float x, float y, float w, float h,
      float mx, float my, float mw, float mh );
static void ovr_optimizeLayout( int items, const Vector2d** pos,
      MapOverlayPos** mo, MapOverlayPosOpt* moo, float res );
static void ovr_init_position( float *px, float *py, float res, float x, float y, float w, float h,
      float margin, const Vector2d** pos, MapOverlayPos** mo, MapOverlayPosOpt* moo, int items, int self,
      float pixbuf, float object_weight, float text_weight );
static int ovr_refresh_compute_overlap( float *ox, float *oy,
      float res, float x, float y, float w, float h, const Vector2d** pos,
      MapOverlayPos** mo, MapOverlayPosOpt* moo, int items, int self, int radius, float pixbuf,
      float object_weight, float text_weight );
/* Markers. */
static void ovr_mrkRenderAll( double res );
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
   x = ((double)mx - (double)map_overlay_center_x()) * ovr_res;
   y = ((double)my - (double)map_overlay_center_y()) * ovr_res;

   return input_clickPos( event, x, y, 1., 10. * ovr_res, 15. * ovr_res );
}


/**
 * @brief Refreshes the map overlay recalculating the dimensions it should have.
 *
 * This should be called if the planets or the likes change at any given time.
 */
void ovr_refresh (void)
{
   double max_x, max_y;
   int i, items;
   Planet *pnt;
   JumpPoint *jp;
   const Vector2d **pos;
   MapOverlayPos **mo;
   MapOverlayPosOpt *moo;
   int ires;
   float res;

   /* Must be open. */
   if (!ovr_isOpen())
      return;

   /* Calculate max size. */
   gui_radarGetRes( &ires );
   res = (float)ires;
   items = 0;
   pos = calloc(cur_system->njumps + cur_system->nplanets, sizeof(Vector2d*));
   mo  = calloc(cur_system->njumps + cur_system->nplanets, sizeof(MapOverlayPos*));
   moo = calloc(cur_system->njumps + cur_system->nplanets, sizeof(MapOverlayPosOpt));
   max_x = 0.;
   max_y = 0.;
   for (i=0; i<cur_system->njumps; i++) {
      jp = &cur_system->jumps[i];
      max_x = MAX( max_x, ABS(jp->pos.x) );
      max_y = MAX( max_y, ABS(jp->pos.y) );
      if (!jp_isUsable(jp) || !jp_isKnown(jp))
         continue;
      /* Initialize the map overlay stuff. */
      moo[items].radius_base = MAX( jumppoint_gfx->sw / res, 10. );
      moo[items].text_width = gl_printWidthRaw( &gl_smallFont, _(jp->target->name) );
      pos[items] = &jp->pos;
      mo[items]  = &jp->mo;
      items++;
   }
   for (i=0; i<cur_system->nplanets; i++) {
      pnt = cur_system->planets[i];
      max_x = MAX( max_x, ABS(pnt->pos.x) );
      max_y = MAX( max_y, ABS(pnt->pos.y) );
      if ((pnt->real != ASSET_REAL) || !planet_isKnown(pnt))
         continue;
      /* Initialize the map overlay stuff. */
      moo[items].radius_base = MAX( pnt->radius*2. / res, 15. );
      moo[items].text_width = gl_printWidthRaw( &gl_smallFont, _(pnt->name) );
      pos[items] = &pnt->pos;
      mo[items]  = &pnt->mo;
      items++;
   }

   /* We need to calculate the radius of the rendering from the maximum radius of the system. */
   ovr_res = 2. * 1.2 * MAX( max_x / map_overlay_width(), max_y / map_overlay_height() );

   /* Compute text overlap and try to minimize it. */
   ovr_optimizeLayout( items, pos, mo, moo, res );

   /* Free the moos. */
   free( mo );
   free( moo );
   free( pos );
}


/**
 * @brief Makes a best effort to fit the given assets' overlay indicators and labels fit without collisions.
 */
static void ovr_optimizeLayout( int items, const Vector2d** pos, MapOverlayPos** mo, MapOverlayPosOpt* moo, float res )
{
   int i, iter, changed;
   float cx,cy, ox,oy, r, off;

   /* Parameters for the map overlay optimization. */
   const float update_rate = 0.02; /**< how big of an update to do each step. */
   const int max_iters = 100; /**< Maximum amount of iterations to do. */
   const float pixbuf = 5.; /**< Pixels to buffer around for text (not used for optimizing radius). */
   const float pixbuf_initial = 50; /**< Initial pixel buffer to consider. */
   const float radius_shrink_ratio = 0.95; /**< How fast to shrink the radius. */
   const float radius_grow_ratio = 1./radius_shrink_ratio; /**< How fast to grow the radius. */
   const float position_threshold_x = 20.; /**< How far to start penalizing x position. */
   const float position_threshold_y = 10.; /**< How far to start penalizing y position. */
   const float position_weight = .1; /**< How much to penalize the position. */
   const float object_weight = 1.; /**< Weight for overlapping with objects. */
   const float text_weight = 2.; /**< Weight for overlapping with text. */

   /* Initialize radius. Other will be put at infinity. */
   for (i=0; i<items; i++) {
      mo[i]->radius = moo[i].radius_base;
      mo[i]->text_offx = HUGE_VAL;
      mo[i]->text_offy = HUGE_VAL;
   }

   /* Initialize all items. */
   for (i=0; i<items; i++) {
      moo[i].radius = moo[i].radius_base;
      /* Test to see what side is best to put the text on.
       * We actually compute the text overlap also so hopefully it will alternate
       * sides when stuff is clustered together. */
      cx = pos[i]->x / res;
      cy = pos[i]->y / res;
      ovr_init_position( &moo[i].text_offx_base, &moo[i].text_offy_base,
            res, cx, cy, moo[i].text_width, gl_smallFont.h, pixbuf, pos, mo, moo, items, i,
            pixbuf_initial, object_weight, text_weight );
      moo[i].text_offx = moo[i].text_offx_base;
      moo[i].text_offy = moo[i].text_offy_base;
      /* Initialize mo. */
      //mo[i]->radius = moo[i].radius;
      mo[i]->text_offx = moo[i].text_offx;
      mo[i]->text_offy = moo[i].text_offy;
   }

   /* Optimize over them. */
   for (iter=0; iter<max_iters; iter++) {
      changed = 0;
      for (i=0; i<items; i++) {
         cx = pos[i]->x / res;
         cy = pos[i]->y / res;
         r  = mo[i]->radius;
         /* Modify radius if overlap.
          * We ignore the text here, as the text should move away on it's own.
          * Furthermore, no pixel buffer so that everything is tighter. */
         if (ovr_refresh_compute_overlap( &ox, &oy, res, cx-r/2., cy-r/2., r, r, pos, mo, moo, items, i, 1, 0., object_weight, 0. )) {
            moo[i].radius *= radius_shrink_ratio;
            changed = 1;
         }
         else if (mo[i]->radius < moo[i].radius_base) {
            moo[i].radius *= radius_grow_ratio;
            changed = 1;
         }
         /* Move text if overlap. */
         if (ovr_refresh_compute_overlap( &ox, &oy, res, cx+mo[i]->text_offx, cy+mo[i]->text_offy, moo[i].text_width, gl_smallFont.h, pos, mo, moo, items, i, 0, pixbuf, object_weight, text_weight )) {
            //moo[i].text_offx += ox / sqrt(fabs(ox)+epsilon) * update_rate;
            //moo[i].text_offy += oy / sqrt(fabs(oy)+epsilon) * update_rate;
            moo[i].text_offx += ox * update_rate;
            moo[i].text_offy += 30 * oy * update_rate; /* Boost y offset as it's more likely to be the solution. */
            changed = 1;
         }

         /* Penalize offsets changes */
         off = moo[i].text_offx_base - mo[i]->text_offx;
         if (fabs(off) > position_threshold_x) {
            off = FSIGN(off) * pow2(fabs(off)-position_threshold_x);
            moo[i].text_offx += position_weight * off;
            changed = 1;
         }
         off = moo[i].text_offy_base - mo[i]->text_offy;
         if (fabs(off) > position_threshold_y) {
            off = FSIGN(off) * pow2(fabs(off)-position_threshold_y);
            moo[i].text_offy += position_weight * off;
            changed = 1;
         }

         /* Propagate updates. */
         mo[i]->radius = moo[i].radius;
         mo[i]->text_offx = moo[i].text_offx;
         mo[i]->text_offy = moo[i].text_offy;
      }
      /* Converged (or unnecessary). */
      if (!changed)
         break;
      /* Propagate updates. */
      /*
      for (i=0; i<items; i++) {
         mo[i]->radius = moo[i].radius;
         mo[i]->text_offx = moo[i].text_offx;
         mo[i]->text_offy = moo[i].text_offy;
      }
      */
   }
}


/**
 * @brief Initializes the position of a map overlay object by checking a number of fixed positions.
 */
static void ovr_init_position( float *px, float *py, float res, float x, float y, float w, float h,
      float margin, const Vector2d** pos, MapOverlayPos** mo, MapOverlayPosOpt* moo, int items, int self,
      float pixbuf, float object_weight, float text_weight )
{
   int i;
   float ox,oy, cx,cy, bx,by;
   float off, val, best;

   off = moo[self].radius_base/2.+margin*1.5;
   /* Order is left -> right -> top -> bottom */
   //float tx[8] = {   off, -off-w, -w/2.,  -w/2., off, -off-w,    off, -off-w };
   //float ty[8] = { -h/2.,  -h/2.,   off, -off-h, off,    off, -off-h, -off-h };
   const float tx[4] = {   off, -off-w, -w/2. , -w/2. };
   const float ty[4] = { -h/2.,  -h/2., off, -off-h };

   /* Check all combinations. */
   best = HUGE_VAL;
   for (i=0; i<4; i++) {
      cx = x + tx[i];
      cy = y + ty[i];
      ovr_refresh_compute_overlap( &ox, &oy, res, cx, cy, w, h, pos, mo, moo, items, self, 1, pixbuf, object_weight, text_weight );
      val = pow2(ox)+pow2(oy);
      /* Keep best. */
      if (val < best) {
         bx = tx[i];
         by = ty[i];
         best = val;
      }
   }

   *px = bx;
   *py = by;
}

/**
 * @brief Compute a collision between two rectangles and direction to move one away from another.
 */
static int update_collision( float *ox, float *oy, float weight,
      float x, float y, float w, float h,
      float mx, float my, float mw, float mh )
{
   /* No collision. */
   if (((x+w) < mx) || (x > (mx+mw)))
      return 0;
   if (((y+h) < my) || (y > (my+mh)))
      return 0;

   /* Case A is left of B. */
   if (x < mx)
      *ox += weight*(mx-(x+w));
   /* Case A is to the right of B. */
   else
      *ox += weight*((mx+mw)-x);

   /* Case A is below B. */
   if (y < my)
      *oy += weight*(my-(y+h));
   /* Case A is above B. */
   else
      *oy += weight*((my+mh)-y);

   return 1;
}


/**
 * @brief Compute how an element overlaps with text and direction to move away.
 */
static int ovr_refresh_compute_overlap( float *ox, float *oy,
      float res, float x, float y, float w, float h, const Vector2d** pos,
      MapOverlayPos** mo, MapOverlayPosOpt* moo, int items, int self, int radius, float pixbuf,
      float object_weight, float text_weight )
{
   int i, collided;
   float mx, my, mw, mh;
   const float pb2 = pixbuf*2.;

   *ox = *oy = 0.;
   collided = 0;

   for (i=0; i<items; i++) {
      if (i != self || !radius) {
         /* convert center coordinates to bottom left*/
         mw = mo[i]->radius + pb2;
         mh = mw;
         mx = pos[i]->x/res - mw/2.;
         my = pos[i]->y/res - mh/2.;
         collided |= update_collision( ox, oy, object_weight, x, y, w, h, mx, my, mw, mh );
      }
      if (i != self || radius) {
         /* no need to convert coordinates, just add pixbuf */
         mw = moo[i].text_width + pb2;
         mh = gl_smallFont.h + pb2;
         mx = pos[i]->x/res + mo[i]->text_offx-pixbuf;
         my = pos[i]->y/res + mo[i]->text_offy-pixbuf;
         collided |= update_collision( ox, oy, text_weight, x, y, w, h, mx, my, mw, mh );
      }
   }

   return collided;
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
   }
   else if (ovr_open) {
      ovr_open = 0;
      input_mouseHide();
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


/**
 * @brief Renders the overlay map.
 *
 *    @param dt Current delta tick.
 */
void ovr_render( double dt )
{
   (void) dt;
   int i, j;
   Pilot **pstk;
   AsteroidAnchor *ast;
   int n;
   double w, h, res;
   double x,y;

   /* Must be open. */
   if (!ovr_open)
      return;

   /* Player must be alive. */
   if (player_isFlag( PLAYER_DESTROYED ) || (player.p == NULL))
      return;

   /* Default values. */
   w     = map_overlay_width();
   h     = map_overlay_height();
   res   = ovr_res;

   /* First render the background overlay. */
   glColour c = { .r=0., .g=0., .b=0., .a= conf.map_overlay_opacity };
   gl_renderRect( (double)gui_getMapOverlayBoundLeft(), (double)gui_getMapOverlayBoundRight(), w, h, &c );

   /* Render planets. */
   for (i=0; i<cur_system->nplanets; i++)
      if ((cur_system->planets[ i ]->real == ASSET_REAL) && (i != player.p->nav_planet))
         gui_renderPlanet( i, RADAR_RECT, w, h, res, 1 );
   if (player.p->nav_planet > -1)
      gui_renderPlanet( player.p->nav_planet, RADAR_RECT, w, h, res, 1 );

   /* Render jump points. */
   for (i=0; i<cur_system->njumps; i++)
      if ((i != player.p->nav_hyperspace) && !jp_isFlag(&cur_system->jumps[i], JP_EXITONLY))
         gui_renderJumpPoint( i, RADAR_RECT, w, h, res, 1 );
   if (player.p->nav_hyperspace > -1)
      gui_renderJumpPoint( player.p->nav_hyperspace, RADAR_RECT, w, h, res, 1 );

   /* Render pilots. */
   pstk  = pilot_getAll( &n );
   j     = 0;
   for (i=0; i<n; i++) {
      if (pstk[i]->id == PLAYER_ID) /* Skip player. */
         continue;
      if (pstk[i]->id == player.p->target)
         j = i;
      else
         gui_renderPilot( pstk[i], RADAR_RECT, w, h, res, 1 );
   }
   /* Render the targeted pilot */
   if (j!=0)
      gui_renderPilot( pstk[j], RADAR_RECT, w, h, res, 1 );

   /* Check if player has goto target. */
   if (player_isFlag(PLAYER_AUTONAV) && (player.autonav == AUTONAV_POS_APPROACH)) {
      x = player.autonav_pos.x / res + map_overlay_center_x();
      y = player.autonav_pos.y / res + map_overlay_center_y();
      gl_renderCross( x, y, 5., &cRadar_hilight );
      gl_printMarkerRaw( &gl_smallFont, x+10., y-gl_smallFont.h/2., &cRadar_hilight, _("TARGET") );
   }

   /* render the asteroids */
   for (i=0; i<cur_system->nasteroids; i++) {
      ast = &cur_system->asteroids[i];
      for (j=0; j<ast->nb; j++)
         gui_renderAsteroid( &ast->asteroids[j], w, h, res, 1 );
   }

   /* Render the player. */
   gui_renderPlayer( res, 1 );

   /* Render markers. */
   ovr_mrkRenderAll( res );
}


/**
 * @brief Renders all the markers.
 *
 *    @param res Resolution to render at.
 */
static void ovr_mrkRenderAll( double res )
{
   int i;
   ovr_marker_t *mrk;
   double x, y;

   if (ovr_markers == NULL)
      return;

   for (i=0; i<array_size(ovr_markers); i++) {
      mrk = &ovr_markers[i];

      x = mrk->u.pt.x / res + map_overlay_center_x();
      y = mrk->u.pt.y / res + map_overlay_center_y();
      gl_renderCross( x, y, 5., &cRadar_hilight );

      if (mrk->text != NULL)
         gl_printMarkerRaw( &gl_smallFont, x+10., y-gl_smallFont.h/2., &cRadar_hilight, mrk->text );
   }
}


/**
 * @brief Frees up and clears all marker related stuff.
 */
void ovr_mrkFree (void)
{
   /* Clear markers. */
   ovr_mrkClear();

   /* Free array. */
   if (ovr_markers != NULL)
      array_free( ovr_markers );
   ovr_markers = NULL;
}


/**
 * @brief Clears the current markers.
 */
void ovr_mrkClear (void)
{
   int i;
   if (ovr_markers == NULL)
      return;
   for (i=0; i<array_size(ovr_markers); i++)
      ovr_mrkCleanup( &ovr_markers[i] );
   array_erase( &ovr_markers, ovr_markers, &ovr_markers[ array_size(ovr_markers) ] );
}


/**
 * @brief Clears up after an individual marker.
 *
 *    @param mrk Marker to clean up after.
 */
static void ovr_mrkCleanup( ovr_marker_t *mrk )
{
   if (mrk->text != NULL)
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

   mrk = ovr_mrkNew();
   mrk->type = 0;
   if (text != NULL)
      mrk->text = strdup( text );
   mrk->u.pt.x = x;
   mrk->u.pt.y = y;

   return mrk->id;
}


/**
 * @brief Removes a marker by id.
 *
 *    @param id ID of the marker to remove.
 */
void ovr_mrkRm( unsigned int id )
{
   int i;
   if (ovr_markers == NULL)
      return;
   for (i=0; i<array_size(ovr_markers); i++) {
      if (id!=ovr_markers[i].id)
         continue;
      ovr_mrkCleanup( &ovr_markers[i] );
      array_erase( &ovr_markers, &ovr_markers[i], &ovr_markers[i+1] );
      break;
   }
}


