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
static void update_collision( float *ox, float *oy, float weight,
      float x, float y, float w, float h,
      float mx, float my, float mw, float mh );
static int ovr_refresh_compute_overlap( float *ox, float *oy,
      float res, float x, float y, float w, float h,
      int jpid, int pntid, int radius, double pixbuf );
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
   int i;
   Planet *pnt;
   JumpPoint *jp;
   float cx,cy, ox,oy, r;
   int iter, ires;
   float res;

   /* Parameters for the map overlay optimization. */
   const float update_rate = 0.5;
   const int max_iters = 100;
   const float pixbuf = 3.; /* Pixels to buffer around for text. */
   const float epsilon = 1e-4;

   /* Must be open. */
   if (!ovr_isOpen())
      return;

   /* Calculate max size. */
   gui_radarGetRes( &ires );
   res = (float)ires;
   max_x = 0.;
   max_y = 0.;
   for (i=0; i<cur_system->njumps; i++) {
      jp = &cur_system->jumps[i];
      max_x = MAX( max_x, ABS(jp->pos.x) );
      max_y = MAX( max_y, ABS(jp->pos.y) );
      jp->mo_radius_base = MAX( jumppoint_gfx->sw / res, 10. );
      jp->mo_radius = jp->mo_radius_base;
      jp->mo_text_offx = jp->mo_radius / 2.+pixbuf*1.5;
      jp->mo_text_offy = -gl_smallFont.h/2.;
      jp->mo_text_width = gl_printWidthRaw( &gl_smallFont, _(jp->target->name) );
   }
   for (i=0; i<cur_system->nplanets; i++) {
      pnt = cur_system->planets[i];
      max_x = MAX( max_x, ABS(pnt->pos.x) );
      max_y = MAX( max_y, ABS(pnt->pos.y) );
      pnt->mo_radius_base = MAX( pnt->radius*2. / res, 15. );
      pnt->mo_radius = pnt->mo_radius_base;
      pnt->mo_text_offx = pnt->mo_radius / 2.+pixbuf*1.5;
      pnt->mo_text_offy = -gl_smallFont.h/2.;
      pnt->mo_text_width = gl_printWidthRaw( &gl_smallFont, _(pnt->name) );
   }

   /* We need to calculate the radius of the rendering. */
   ovr_res = 2. * 1.2 * MAX( max_x / map_overlay_width(), max_y / map_overlay_height() );

   /* Compute text overlap and try to minimize it. */
   for (iter=0; iter<max_iters; iter++) {
      for (i=0; i<cur_system->njumps; i++) {
         jp = &cur_system->jumps[i];
         if (!jp_isUsable(jp) || !jp_isKnown(jp))
            continue;
         cx = jp->pos.x / res;
         cy = jp->pos.y / res;
         r  = jp->mo_radius;
         /* Modify radius if overlap. */
         if (ovr_refresh_compute_overlap( &ox, &oy, res, cx-r/2., cy-r/2., r, r, i, -1, 1, 0. ))
            jp->mo_radius *= 0.99;
         else if (jp->mo_radius < jp->mo_radius_base)
            jp->mo_radius *= 1.01;
         /* Move text if overlap. */
         if (ovr_refresh_compute_overlap( &ox, &oy, res ,cx+jp->mo_text_offx, cy+jp->mo_text_offy, jp->mo_text_width, gl_smallFont.h, i, -1, 0, pixbuf )) {
            jp->mo_text_offx += ox / sqrt(fabs(ox)+epsilon) * update_rate;
            jp->mo_text_offy += oy / sqrt(fabs(oy)+epsilon) * update_rate;
         }
      }
      for (i=0; i<cur_system->nplanets; i++) {
         pnt = cur_system->planets[i];
         if ((pnt->real != ASSET_REAL) || !planet_isKnown(pnt))
            continue;
         cx = pnt->pos.x / res;
         cy = pnt->pos.y / res;
         r  = pnt->mo_radius;
         /* Modify radius if overlap. */
         if (ovr_refresh_compute_overlap( &ox, &oy, res, cx-r/2., cy-r/2., r, r, -1, i, 1, 0. ))
            pnt->mo_radius *= 0.99;
         else if (pnt->mo_radius < pnt->mo_radius_base)
            pnt->mo_radius *= 1.01;
         /* Move text if overlap. */
         if (ovr_refresh_compute_overlap( &ox, &oy, res, cx+pnt->mo_text_offx, cy+pnt->mo_text_offy, pnt->mo_text_width, gl_smallFont.h, -1, i, 0, pixbuf )) {
            pnt->mo_text_offx += ox / sqrt(fabs(ox)+epsilon) * update_rate;
            pnt->mo_text_offy += oy / sqrt(fabs(oy)+epsilon) * update_rate;
         }
      }
   }
}


/**
 * @brief Compute a collision between two rectangles and direction to move one away from another.
 */
static void update_collision( float *ox, float *oy, float weight,
      float x, float y, float w, float h,
      float mx, float my, float mw, float mh )
{
   /* No collision. */
   if (((x+w) < mx) || (x > (mx+mw)))
      return;
   if (((y+h) < my) || (y > (my+mh)))
      return;

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
}


/**
 * @brief Compute how an element overlaps with text and direction to move away.
 */
static int ovr_refresh_compute_overlap( float *ox, float *oy,
      float res, float x, float y, float w, float h,
      int jpid, int pntid, int radius, double pixbuf )
{
   int i;
   Planet *pnt;
   JumpPoint *jp;
   float mx, my, mw, mh;

   *ox = *oy = 0.;

   for (i=0; i<cur_system->njumps; i++) {
      jp = &cur_system->jumps[i];
      if (!jp_isUsable(jp) || !jp_isKnown(jp))
         continue;
      if ((jpid != i) || !radius) { 
         mw = jp->mo_radius+2.*pixbuf;
         mh = mw;
         mx = jp->pos.x/res - mw/2.;
         my = jp->pos.y/res - mh/2.;
         update_collision( ox, oy, 2., x, y, w, h, mx, my, mw, mh );
      }
      if ((jpid != i) || radius) {
         mw = jp->mo_text_width+2.*pixbuf;
         mh = gl_smallFont.h+2.*pixbuf;
         mx = jp->pos.x/res + jp->mo_text_offx-pixbuf;
         my = jp->pos.x/res + jp->mo_text_offy-pixbuf;
         update_collision( ox, oy, 1., x, y, w, h, mx, my, mw, mh );
      }
   }
   for (i=0; i<cur_system->nplanets; i++) {
      pnt = cur_system->planets[i];
      if ((pnt->real != ASSET_REAL) || !planet_isKnown(pnt))
         continue;
      if ((pntid != i) || !radius) {
         mw = pnt->mo_radius+2.*pixbuf;
         mh = mw;
         mx = pnt->pos.x/res - mw/2.;
         my = pnt->pos.y/res - mh/2.;
         update_collision( ox, oy, 2., x, y, w, h, mx, my, mw, mh );
      }
      if ((pntid != i) || radius) {
         mw = pnt->mo_text_width+2.*pixbuf;
         mh = gl_smallFont.h+2.*pixbuf;
         mx = pnt->pos.x/res + pnt->mo_text_offx-pixbuf;
         my = pnt->pos.y/res + pnt->mo_text_offy-pixbuf;
         update_collision( ox, oy, 1., x, y, w, h, mx, my, mw, mh );
      }
   }

   return (*ox > 0.) || (*oy > 0.);
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
   Uint32 t;

   t = SDL_GetTicks();

   if (type > 0) {
      if (ovr_open)
         ovr_setOpen(0);
      else {
         ovr_setOpen(1);
         ovr_opened  = t;

         /* Refresh overlay size. */
         ovr_refresh();
      }
   }
   else if (type < 0) {
      if (t - ovr_opened > 300)
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


