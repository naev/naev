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
   unsigned int pid;
   Pilot *p;
   int mx, my;
   double x, y, r, rp;
   double d, dp;
   Planet *pnt;
   JumpPoint *jp;
   int pntid, jpid;

   /* We only want mouse events. */
   if (event->type != SDL_MOUSEBUTTONDOWN)
      return 0;

   /* Player must not be NULL. */
   if (player_isFlag(PLAYER_DESTROYED) || (player.p == NULL))
      return 0;

   /* Selection. */
   if (event->button.button == SDL_BUTTON_LEFT) {
      /* Translate from window to screen. */
      mx = event->button.x;
      my = event->button.y;
      gl_windowToScreenPos( &mx, &my, mx, my );

      /* Translate to space coords. */
      x  = ((double)mx - SCREEN_W/2.) * ovr_res;
      y  = ((double)my - SCREEN_H/2.) * ovr_res;

      /* Get nearest pilot and jump point/planet. */
      dp    = pilot_getNearestPos( player.p, &pid, x, y, 1 );
      d     = system_getClosest( cur_system, &pntid, &jpid, x, y );
      p     = pilot_get(pid);
      rp    = MAX( 1.5 * PILOT_SIZE_APROX * p->ship->gfx_space->sw / 2, 20.*ovr_res );

      if (pntid >= 0) { /* Planet is closer. */
         pnt = cur_system->planets[ pntid ];
         r  = MAX( 1.5 * pnt->radius, 20. * ovr_res );
      }
      else if (jpid >= 0) {
         jp = &cur_system->jumps[ jpid ];
         r  = MAX( 1.5 * jp->radius, 20. * ovr_res );
      }
      else
         r  = 0.;

      /* Pilot is closest, or new jump point/planet is the same as the old. */
      if ((dp < pow2(rp) && player.p->target != pid) && (dp < d ||
            ((pntid >=0 && player.p->nav_planet == pntid) ||
            (jpid >=0 && player.p->nav_planet == jpid))))
         player_targetSet( pid );
      else if ((pntid >= 0) && (d < pow2(r)) && planet_isKnown(pnt)) /* Planet is closest. */
         player_targetPlanetSet( pntid );
      else if ((jpid >= 0) && (d < pow2(r)) && jp_isKnown(jp)) /* Jump point is closest. */
         player_targetHyperspaceSet( jpid );
      else
         return 0;
      return 1;
   }
   /* Autogo. */
   else if (event->button.button == SDL_BUTTON_RIGHT) {
      if ((player.p == NULL) || pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ) ||
            pilot_isFlag( player.p, PILOT_HYP_PREP ) ||
            pilot_isFlag( player.p, PILOT_HYP_BEGIN ) ||
            pilot_isFlag( player.p, PILOT_HYPERSPACE ))
         return 1;

      /* Translate from window to screen. */
      mx = event->button.x;
      my = event->button.y;
      gl_windowToScreenPos( &mx, &my, mx, my );

      /* Translate to space coords. */
      x  = ((double)mx - SCREEN_W/2.) * ovr_res;
      y  = ((double)my - SCREEN_H/2.) * ovr_res;

      /* Go to planet. */
      d = system_getClosest( cur_system, &pntid, &jpid, x, y );
      if (pntid >= 0) {
         pnt = cur_system->planets[ pntid ];
         r  = MAX( 1.5 * pnt->radius, 20. * ovr_res );
         if ((d < pow2(r)) && planet_isKnown(pnt)) {
            player_targetPlanetSet( pntid );
            player_autonavPnt( pnt->name );
            return 1;
         }
      }
      /* Engage regular jump autonav. */
      else if (jpid >= 0) {
         jp = &cur_system->jumps[ jpid ];
         r  = MAX( 1.5 * jp->radius, 20. * ovr_res );
         if ((d < pow2(r)) && jp_isKnown(jp)) {
            player_targetHyperspaceSet( jpid );
            player_autonavStart();
            return 1;
         }
      }

      /* Fall-through and go to position. */
      player_autonavPos( x, y );

      return 1;
   }

   return 0;
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

   /* Must be open. */
   if (!ovr_isOpen())
      return;

   /* Calculate max size. */
   max_x = 0.;
   max_y = 0.;
   for (i=0; i<cur_system->njumps; i++) {
      max_x = MAX( max_x, ABS(cur_system->jumps[i].pos.x) );
      max_y = MAX( max_y, ABS(cur_system->jumps[i].pos.y) );
   }
   for (i=0; i<cur_system->nplanets; i++) {
      max_x = MAX( max_x, ABS(cur_system->planets[i]->pos.x) );
      max_y = MAX( max_y, ABS(cur_system->planets[i]->pos.y) );
   }

   /* We need to calculate the radius of the rendering. */
   ovr_res = 2. * 1.2 * MAX( max_x / SCREEN_W, max_y / SCREEN_H );
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
   int n;
   double w, h, res;
   double x,y;
   glColour c = { .r=0., .g=0., .b=0., .a=0.5 };

   /* Must be open. */
   if (!ovr_open)
      return;

   /* Player must be alive. */
   if (player_isFlag( PLAYER_DESTROYED ) || (player.p == NULL))
      return;

   /* Default values. */
   w     = SCREEN_W;
   h     = SCREEN_H;
   res   = ovr_res;

   /* First render the background overlay. */
   gl_renderRect( 0., 0., w, h, &c );

   /* We need to center in the image first. */
   gl_matrixPush();
      gl_matrixTranslate( w/2., h/2. );

   /* Render planets. */
   for (i=0; i<cur_system->nplanets; i++)
      if ((cur_system->planets[ i ]->real == ASSET_REAL) && (i != player.p->nav_planet))
         gui_renderPlanet( i, RADAR_RECT, w, h, res, 1 );
   if (player.p->nav_planet > -1)
      gui_renderPlanet( player.p->nav_planet, RADAR_RECT, w, h, res, 1 );

   /* Render jump points. */
   for (i=0; i<cur_system->njumps; i++)
      if ((i != player.p->nav_hyperspace) && !jp_isFlag(&cur_system->jumps[i], JP_HIDDEN) && !jp_isFlag(&cur_system->jumps[i], JP_EXITONLY))
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
      x = player.autonav_pos.x / res;
      y = player.autonav_pos.y / res;
      gl_renderCross( x, y, 5., &cRadar_hilight );
      gl_printRaw( &gl_smallFont, x+10., y-gl_smallFont.h/2., &cRadar_hilight, "GOTO" );
   }

   /* Render the player. */
   gui_renderPlayer( res, 1 );

   /* Render markers. */
   ovr_mrkRenderAll( res );

   /* Pop the matrix. */
   gl_matrixPop();
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

      x = mrk->u.pt.x / res;
      y = mrk->u.pt.y / res;
      gl_renderCross( x, y, 5., &cRadar_hilight );

      if (mrk->text != NULL)
         gl_printRaw( &gl_smallFont, x+10., y-gl_smallFont.h/2., &cRadar_hilight, mrk->text );
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


