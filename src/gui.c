/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file gui.c
 *
 * @brief Contains the GUI stuff for the player.
 */


#include "gui.h"

#include "naev.h"

#include <stdlib.h>

#include "player.h"
#include "nxml.h"
#include "pilot.h"
#include "log.h"
#include "opengl.h"
#include "font.h"
#include "ndata.h"
#include "space.h"
#include "rng.h"
#include "land.h"
#include "sound.h"
#include "economy.h"
#include "pause.h"
#include "menu.h"
#include "toolkit.h"
#include "dialogue.h"
#include "mission.h"
#include "nlua_misn.h"
#include "ntime.h"
#include "hook.h"
#include "map.h"
#include "nfile.h"
#include "spfx.h"
#include "unidiff.h"
#include "comm.h"
#include "intro.h"
#include "perlin.h"
#include "ai.h"
#include "music.h"
#include "nmath.h"
#include "gui_osd.h"
#include "conf.h"
#include "nebula.h"
#include "camera.h"
#include "pilot.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_gfx.h"
#include "nlua_gui.h"
#include "nlua_tex.h"
#include "nlua_tk.h"
#include "gui_omsg.h"
#include "nstring.h"


#define XML_GUI_ID   "GUIs" /**< XML section identifier for GUI document. */
#define XML_GUI_TAG  "gui" /**<  XML Section identifier for GUI tags. */

#define INTERFERENCE_LAYERS      16 /**< Number of interference layers. */
#define INTERFERENCE_CHANGE_DT   0.1 /**< Speed to change at. */

#define RADAR_BLINK_PILOT        1. /**< Blink rate of the pilot target on radar. */
#define RADAR_BLINK_PLANET       1. /**< Blink rate of the planet target on radar. */


/* for interference. */
static int interference_layer = 0; /**< Layer of the current interference. */
double interference_alpha     = 0.; /**< Alpha of the current interference layer. */
static double interference_t  = 0.; /**< Interference timer to control transitions. */

/* some blinking stuff. */
static double blink_pilot     = 0.; /**< Timer on target blinking on radar. */
static double blink_planet    = 0.; /**< Timer on planet blinking on radar. */

/* for VBO. */
static gl_vbo *gui_triangle_vbo = NULL;
static gl_vbo *gui_planet_vbo = NULL;
static gl_vbo *gui_radar_select_vbo = NULL;
static gl_vbo *gui_planet_blink_vbo = NULL;

static int gui_getMessage     = 1; /**< Whether or not the player should receive messages. */

/*
 * pilot stuff for GUI
 */
extern Pilot** pilot_stack; /**< @todo remove */
extern int pilot_nstack; /**< @todo remove */


extern int land_wid; /**< From land.c */


/**
 * GUI Lua stuff.
 */
static nlua_env gui_env = LUA_NOREF; /**< Current GUI Lua environment. */
static int gui_L_mclick = 0; /**< Use mouse click callback. */
static int gui_L_mmove = 0; /**< Use mouse movement callback. */


/**
 * Cropping.
 */
static double gui_viewport_x = 0.; /**< GUI Viewport X offset. */
static double gui_viewport_y = 0.; /**< GUI Viewport Y offset. */
static double gui_viewport_w = 0.; /**< GUI Viewport width. */
static double gui_viewport_h = 0.; /**< GUI Viewport height. */


/**
 * @struct Radar
 *
 * @brief Represents the player's radar.
 */
typedef struct Radar_ {
   double w; /**< Width. */
   double h; /**< Height. */
   double x; /**< X position. */
   double y; /**< Y position. */
   RadarShape shape; /**< Shape */
   double res; /**< Resolution */
   glTexture *interference[INTERFERENCE_LAYERS]; /**< Interference texture. */
} Radar;
/* radar resolutions */
#define RADAR_RES_MAX      100. /**< Maximum radar resolution. */
#define RADAR_RES_MIN      10. /**< Minimum radar resolution. */
#define RADAR_RES_INTERVAL 10. /**< Steps used to increase/decrease resolution. */
#define RADAR_RES_DEFAULT  50. /**< Default resolution. */
static Radar gui_radar;

/* needed to render properly */
static double gui_xoff = 0.; /**< X Offset that GUI introduces. */
static double gui_yoff = 0.; /**< Y offset that GUI introduces. */

/* messages */
#define MESG_SIZE_MAX        256 /**< Maxmimu message length. */
static int mesg_max        = 128; /**< Maximum messages onscreen */
static int mesg_pointer    = 0; /**< Current pointer message is at (for when scrolling. */
static int mesg_viewpoint  = -1; /**< Position of viewing. */
static double mesg_timeout = 15.; /**< Timeout length. */
/**
 * @struct Mesg
 *
 * @brief On screen player message.
 */
typedef struct Mesg_ {
   char str[MESG_SIZE_MAX]; /**< The message. */
   double t; /**< Time to live for the message. */
   glFontRestore restore; /**< Hack for font restoration. */
} Mesg;
static Mesg* mesg_stack = NULL; /**< Stack of messages, will be of mesg_max size. */
static int gui_mesg_w   = 0; /**< Width of messages. */
static int gui_mesg_x   = 0; /**< X positioning of messages. */
static int gui_mesg_y   = 0; /**< Y positioning of messages. */

/* Calculations to speed up borders. */
static double gui_tr = 0.; /**< Border top-right. */
static double gui_br = 0.; /**< Border bottom-right. */
static double gui_tl = 0.; /**< Border top-left. */
static double gui_bl = 0.; /**< Border bottom-left. */

/* Intrinsic graphical stuff. */
static glTexture *gui_ico_hail      = NULL; /**< Hailing icon. */
static glTexture *gui_target_planet = NULL; /**< Planet targeting icon. */
static glTexture *gui_target_pilot  = NULL; /**< Pilot targeting icon. */


/*
 * prototypes
 */
/*
 * external
 */
extern void weapon_minimap( const double res, const double w, const double h,
      const RadarShape shape, double alpha ); /**< from weapon.c */
/*
 * internal
 */
/* gui */
static void gui_createInterference( Radar *radar );
static void gui_borderIntersection( double *cx, double *cy, double rx, double ry, double hw, double hh );
/* Render GUI. */
static void gui_renderPilotTarget( double dt );
static void gui_renderPlanetTarget( double dt );
static void gui_renderBorder( double dt );
static void gui_renderMessages( double dt );
static const glColour *gui_getPlanetColour( int i );
static void gui_renderRadarOutOfRange( RadarShape sh, int w, int h, int cx, int cy, const glColour *col );
static void gui_planetBlink( int w, int h, int rc, int cx, int cy, GLfloat vr, RadarShape shape );
static const glColour* gui_getPilotColour( const Pilot* p );
static void gui_renderInterference (void);
static void gui_calcBorders (void);
/* Lua GUI. */
static int gui_doFunc( const char* func );
static int gui_prepFunc( const char* func );
static int gui_runFunc( const char* func, int nargs, int nret );



/**
 * Sets the GUI to defaults.
 */
void gui_setDefaults (void)
{
   gui_radar.res = RADAR_RES_DEFAULT;
   memset( mesg_stack, 0, sizeof(Mesg)*mesg_max );
}


/**
 * @brief Initializes the message system.
 *
 *    @param width Message width.
 *    @param x X position to set at.
 *    @param y Y position to set at.
 */
void gui_messageInit( int width, int x, int y )
{
   gui_mesg_w = width;
   gui_mesg_x = x;
   gui_mesg_y = y;
}


/**
 * @brief Scrolls up the message box.
 *
 *    @param lines Number of lines to scroll up.
 */
void gui_messageScrollUp( int lines )
{
   int o;

   /* Handle hacks. */
   if (mesg_viewpoint == -1) {
      mesg_viewpoint = mesg_pointer;
      return;
   }

   /* Get offset. */
   o  = mesg_pointer - mesg_viewpoint;
   if (o < 0)
      o += mesg_max;
   o = mesg_max - 2*conf.mesg_visible - o;


   /* Calculate max line movement. */
   if (lines > o)
      lines = o;

   /* Move viewpoint. */
   mesg_viewpoint = (mesg_viewpoint - lines) % mesg_max;
}


/**
 * @brief Scrolls up the message box.
 *
 *    @param lines Number of lines to scroll up.
 */
void gui_messageScrollDown( int lines )
{
   int o;

   /* Handle hacks. */
   if (mesg_viewpoint == mesg_pointer) {
      mesg_viewpoint = -1;
      return;
   }
   else if (mesg_viewpoint == -1)
      return;

   /* Get offset. */
   o  = mesg_pointer - mesg_viewpoint;
   if (o < 0)
      o += mesg_max;

   /* Calculate max line movement. */
   if (lines > o)
      lines = o;

   /* Move viewpoint. */
   mesg_viewpoint = (mesg_viewpoint + lines) % mesg_max;
}


/**
 * @brief Toggles if player should receive messages.
 *
 *    @param enable Whether or not to enable player receiving messages.
 */
void player_messageToggle( int enable )
{
   gui_getMessage = enable;
}


/**
 * @brief Adds a mesg to the queue to be displayed on screen.
 *
 *    @param str Message to add.
 */
void player_messageRaw( const char *str )
{
   int i, p, l;

   /* Must be receiving messages. */
   if (!gui_getMessage)
      return;

   /* Must be non-null. */
   if (str == NULL)
      return;

   /* Get length. */
   l = strlen(str);
   i = gl_printWidthForText( NULL, str, gui_mesg_w - ((str[0] == '\t') ? 45. : 15.) );
   p = 0;
   while (p < l) {
      /* Move pointer. */
      mesg_pointer   = (mesg_pointer + 1) % mesg_max;
      if (mesg_viewpoint != -1)
         mesg_viewpoint++;

      /* Buffer overrun safety. */
      if (i > MESG_SIZE_MAX-1)
         i = MESG_SIZE_MAX-1;

      /* Add the new one */
      if (p == 0) {
         nsnprintf( mesg_stack[mesg_pointer].str, i+1, "%s", &str[p] );
         gl_printRestoreInit( &mesg_stack[mesg_pointer].restore );
      }
      else {
         mesg_stack[mesg_pointer].str[0] = '\t'; /* Hack to indent. */
         nsnprintf( &mesg_stack[mesg_pointer].str[1], i+1, "%s", &str[p] );
         gl_printStoreMax( &mesg_stack[mesg_pointer].restore, str, p );
      }
      mesg_stack[mesg_pointer].t = mesg_timeout;

      /* Get length. */
      p += i;
      if ((str[p] == '\n') || (str[p] == ' '))
         p++; /* Skip "empty char". */
      i  = gl_printWidthForText( NULL, &str[p], gui_mesg_w - 45. ); /* They're tabbed so it's shorter. */
   }
}

/**
 * @brief Adds a mesg to the queue to be displayed on screen.
 *
 *    @param fmt String with formatting like printf.
 */
void player_message( const char *fmt, ... )
{
   va_list ap;
   char buf[1024];

   /* Must be receiving messages. */
   if (!gui_getMessage)
      return;

   /* Must be non-null. */
   if (fmt == NULL)
      return;

   /* Add the new one */
   va_start(ap, fmt);
   vsnprintf( buf, sizeof(buf), fmt, ap );
   va_end(ap);

   /* Add the message. */
   player_messageRaw( buf );
}


/**
 * @brief Sets up rendering of planet and jump point targeting reticles.
 *
 *    @param dt Current delta tick.
 */
static void gui_renderPlanetTarget( double dt )
{
   (void) dt;
   double x,y, w,h;
   const glColour *c;
   Planet *planet;
   JumpPoint *jp;
   AsteroidAnchor *field;
   Asteroid *ast;
   AsteroidType *at;

   /* no need to draw if pilot is dead */
   if (player_isFlag(PLAYER_DESTROYED) || player_isFlag(PLAYER_CREATING) ||
      (player.p == NULL) || pilot_isFlag(player.p,PILOT_DEAD))
      return;

   /* Make sure target exists. */
   if ((player.p->nav_planet < 0) && (player.p->nav_hyperspace < 0) 
       && (player.p->nav_asteroid < 0))
      return;

   /* Make sure targets are still in range. */
#if 0
   if (!pilot_inRangePlanet( player.p, player.p->nav_planet )) {
      player_targetPlanetSet( -1 );
      return;
   }
#endif

   /* Draw planet and jump point target graphics. */
   if (player.p->nav_hyperspace >= 0) {
      jp = &cur_system->jumps[player.p->nav_hyperspace];

      c = &cGreen;

      x = jp->pos.x - jumppoint_gfx->sw/2.;
      y = jp->pos.y + jumppoint_gfx->sh/2.;
      w = jumppoint_gfx->sw;
      h = jumppoint_gfx->sh;
      gui_renderTargetReticles( x, y, w, h, c );
   }
   if (player.p->nav_planet >= 0) {
      planet = cur_system->planets[player.p->nav_planet];
      c = planet_getColour( planet );

      x = planet->pos.x - planet->gfx_space->w / 2.;
      y = planet->pos.y + planet->gfx_space->h / 2.;
      w = planet->gfx_space->w;
      h = planet->gfx_space->h;
      gui_renderTargetReticles( x, y, w, h, c );
   }
   if (player.p->nav_asteroid >= 0) {
      field = &cur_system->asteroids[player.p->nav_anchor];
      ast   = &field->asteroids[player.p->nav_asteroid];
      c = &cWhite;

      /* Recover the right gfx */
      at = space_getType( ast->type );
      if (ast->gfxID > at->ngfx+1)
         WARN(_("Gfx index out of range"));

      x = ast->pos.x - at->gfxs[ast->gfxID]->w / 2.;
      y = ast->pos.y + at->gfxs[ast->gfxID]->h / 2.;
      w = at->gfxs[ast->gfxID]->w;
      h = at->gfxs[ast->gfxID]->h;
      gui_renderTargetReticles( x, y, w, h, c );
   }
}


/**
 * @brief Renders planet and jump point targeting reticles.
 *
 *    @param x X position of reticle segment.
 *    @param y Y position of reticle segment.
 *    @param w Width.
 *    @param h Height.
 *    @param c Colour.
 */
void gui_renderTargetReticles( int x, int y, int w, int h, const glColour* c )
{
   /* Must not be NULL. */
   if (gui_target_planet == NULL)
      return;

   gl_blitSprite( gui_target_planet, x, y, 0, 0, c ); /* top left */

   x += w;
   gl_blitSprite( gui_target_planet, x, y, 1, 0, c ); /* top right */

   y -= h;
   gl_blitSprite( gui_target_planet, x, y, 1, 1, c ); /* bottom right */

   x -= w;
   gl_blitSprite( gui_target_planet, x, y, 0, 1, c ); /* bottom left */
}


/**
 * @brief Renders the players pilot target.
 *
 *    @param dt Current delta tick.
 */
static void gui_renderPilotTarget( double dt )
{
   (void) dt;
   Pilot *p;
   const glColour *c;
   double x, y;

   /* Player is most likely dead. */
   if (gui_target_pilot == NULL)
      return;

   /* Get the target. */
   if (player.p->target != PLAYER_ID)
      p = pilot_get(player.p->target);
   else p = NULL;

   /* Make sure pilot exists and is still alive. */
   if ((p==NULL) || pilot_isFlag(p,PILOT_DEAD)) {
      pilot_setTarget( player.p, player.p->id );
      gui_setTarget();
      return;
   }

   /* Make sure target is still in range. */
   if (!pilot_inRangePilot( player.p, p, NULL )) {
      pilot_setTarget( player.p, player.p->id );
      gui_setTarget();
      return;
   }

   /* Draw the pilot target. */
   if (pilot_isDisabled(p))
      c = &cInert;
   else if (pilot_isHostile(p))
      c = &cHostile;
   else if (pilot_isFriendly(p))
      c = &cFriend;
   else
      c = &cNeutral;

   x = p->solid->pos.x - p->ship->gfx_space->sw * PILOT_SIZE_APROX/2.;
   y = p->solid->pos.y + p->ship->gfx_space->sh * PILOT_SIZE_APROX/2.;
   gl_blitSprite( gui_target_pilot, x, y, 0, 0, c ); /* top left */

   x += p->ship->gfx_space->sw * PILOT_SIZE_APROX;
   gl_blitSprite( gui_target_pilot, x, y, 1, 0, c ); /* top right */

   y -= p->ship->gfx_space->sh * PILOT_SIZE_APROX;
   gl_blitSprite( gui_target_pilot, x, y, 1, 1, c ); /* bottom right */

   x -= p->ship->gfx_space->sw * PILOT_SIZE_APROX;
   gl_blitSprite( gui_target_pilot, x, y, 0, 1, c ); /* bottom left */
}


/**
 * @brief Gets the intersection with the border.
 *
 * http://en.wikipedia.org/wiki/Intercept_theorem
 *
 *    @param[out] cx X intersection.
 *    @param[out] cy Y intersection.
 *    @param rx Center X position of intersection.
 *    @param ry Center Y position of intersection.
 *    @param hw Screen half-width.
 *    @param hh Screen half-height.
 */
static void gui_borderIntersection( double *cx, double *cy, double rx, double ry, double hw, double hh )
{
   double a;
   double w, h;

   /* Get angle. */
   a = atan2( ry, rx );
   if (a < 0.)
      a += 2.*M_PI;

   /* Helpers. */
   w = hw-7.;
   h = hh-7.;

   /* Handle by quadrant. */
   if ((a > gui_tr) && (a < gui_tl)) { /* Top. */
      *cx = h * (rx/ry);
      *cy = h;
   }
   else if ((a > gui_tl) && (a < gui_bl)) { /* Left. */
      *cx = -w;
      *cy = -w * (ry/rx);
   }
   else if ((a > gui_bl) && (a < gui_br)) { /* Bottom. */
      *cx = -h * (rx/ry);
      *cy = -h;
   }
   else { /* Right. */
      *cx = w;
      *cy = w * (ry/rx);
   }

   /* Translate. */
   *cx += hw;
   *cy += hh;
}


/**
 * @brief Renders the ships/planets in the border.
 *
 *    @param dt Current delta tick.
 */
static void gui_renderBorder( double dt )
{
   (void) dt;
   int i;
   Pilot *plt;
   Planet *pnt;
   JumpPoint *jp;
   int hw, hh;
   double rx,ry;
   double cx,cy;
   const glColour *col;
   glColour ccol;
   double int_a;

   /* Get player position. */
   hw    = SCREEN_W/2;
   hh    = SCREEN_H/2;

   /* Interference. */
   int_a = 1. - interference_alpha;

   /* Render borders to enhance contrast. */
   gl_renderRect( 0., 0., 15., SCREEN_H, &cBlackHilight );
   gl_renderRect( SCREEN_W - 15., 0., 15., SCREEN_H, &cBlackHilight );
   gl_renderRect( 15., 0., SCREEN_W - 30., 15., &cBlackHilight );
   gl_renderRect( 15., SCREEN_H - 15., SCREEN_W - 30., 15., &cBlackHilight );

   /* Draw planets. */
   for (i=0; i<cur_system->nplanets; i++) {
      /* Check that it's real. */
      if (cur_system->planets[i]->real != ASSET_REAL)
         continue;

      pnt = cur_system->planets[i];

      /* Skip if unknown. */
      if (!planet_isKnown( pnt ))
         continue;

      /* Check if out of range. */
      if (!gui_onScreenAsset( &rx, &ry, NULL, pnt )) {

         /* Get border intersection. */
         gui_borderIntersection( &cx, &cy, rx, ry, hw, hh );

         col = gui_getPlanetColour(i);
         ccol.r = col->r;
         ccol.g = col->g;
         ccol.b = col->b;
         ccol.a = int_a;
         gl_drawCircle(cx, cy, 5, &ccol, 0);
      }
   }

   /* Draw jump routes. */
   for (i=0; i<cur_system->njumps; i++) {
      jp  = &cur_system->jumps[i];

      /* Skip if unknown or exit-only. */
      if (!jp_isKnown( jp ) || jp_isFlag( jp, JP_EXITONLY ))
         continue;

      /* Check if out of range. */
      if (!gui_onScreenAsset( &rx, &ry, jp, NULL )) {

         /* Get border intersection. */
         gui_borderIntersection( &cx, &cy, rx, ry, hw, hh );

         if (i==player.p->nav_hyperspace)
            col = &cGreen;
         else
            col = &cWhite;
         ccol.r = col->r;
         ccol.g = col->g;
         ccol.b = col->b;
         ccol.a = int_a;

         gl_beginSolidProgram(gl_Matrix4_Translate(gl_view_matrix, cx, cy, 0), &ccol);
         gl_vboActivateAttribOffset( gui_triangle_vbo, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
         glDrawArrays( GL_LINE_STRIP, 0, 4 );
         gl_endSolidProgram();
      }
   }

   /* Draw pilots. */
   for (i=1; i<pilot_nstack; i++) { /* skip the player */
      plt = pilot_stack[i];

      /* See if in sensor range. */
      if (!pilot_inRangePilot(player.p, plt, NULL))
         continue;

      /* Check if out of range. */
      if (!gui_onScreenPilot( &rx, &ry, plt )) {

         /* Get border intersection. */
         gui_borderIntersection( &cx, &cy, rx, ry, hw, hh );

         col = gui_getPilotColour(plt);
         ccol.r = col->r;
         ccol.g = col->g;
         ccol.b = col->b;
         ccol.a = int_a;
         gl_renderRectEmpty(cx-5, cy-5, 10, 10, &ccol);
      }
   }
}


/**
 * @brief Takes a pilot and returns whether it's on screen, plus its relative position.
 *
 * @param[out] rx Relative X position (factoring in viewport offset)
 * @param[out] ry Relative Y position (factoring in viewport offset)
 * @param pilot Pilot to determine the visibility and position of
 * @return Whether or not the pilot is on-screen.
 */
int gui_onScreenPilot( double *rx, double *ry, Pilot *pilot )
{
   double z;
   int cw, ch;
   glTexture *tex;

   z = cam_getZoom();

   tex = pilot->ship->gfx_space;

   /* Get relative positions. */
   *rx = (pilot->solid->pos.x - player.p->solid->pos.x)*z;
   *ry = (pilot->solid->pos.y - player.p->solid->pos.y)*z;

   /* Correct for offset. */
   *rx -= gui_xoff;
   *ry -= gui_yoff;

   /* Compare dimensions. */
   cw = SCREEN_W/2 + tex->sw/2;
   ch = SCREEN_H/2 + tex->sh/2;

   if ((ABS(*rx) > cw) || (ABS(*ry) > ch))
      return  0;

   return 1;
}


/**
 * @brief Takes a planet or jump point and returns whether it's on screen, plus its relative position.
 *
 * @param[out] rx Relative X position (factoring in viewport offset)
 * @param[out] ry Relative Y position (factoring in viewport offset)
 * @param jp Jump point to determine the visibility and position of
 * @param pnt Planet to determine the visibility and position of
 * @return Whether or not the given asset is on-screen.
 */
int gui_onScreenAsset( double *rx, double *ry, JumpPoint *jp, Planet *pnt )
{
   double z;
   int cw, ch;
   glTexture *tex;

   z = cam_getZoom();

   if (jp == NULL) {
      tex = pnt->gfx_space;
      *rx = (pnt->pos.x - player.p->solid->pos.x)*z;
      *ry = (pnt->pos.y - player.p->solid->pos.y)*z;
   }
   else {
      tex = jumppoint_gfx;
      *rx = (jp->pos.x - player.p->solid->pos.x)*z;
      *ry = (jp->pos.y - player.p->solid->pos.y)*z;
   }

   /* Correct for offset. */
   *rx -= gui_xoff;
   *ry -= gui_yoff;

   /* Compare dimensions. */
   cw = SCREEN_W/2 + tex->sw/2;
   ch = SCREEN_H/2 + tex->sh/2;

   if ((ABS(*rx) > cw) || (ABS(*ry) > ch))
      return  0;

   return 1;
}


/**
 * @brief Renders the gui targeting reticles.
 *
 * @param dt Current deltatick.
 */
void gui_renderReticles( double dt )
{
   /* Player must be alive. */
   if (player.p == NULL)
      return;

   gui_renderPlanetTarget(dt);
   gui_renderPilotTarget(dt);
}


static int can_jump = 0; /**< Stores whether or not the player is able to jump. */
/**
 * @brief Renders the player's GUI.
 *
 *    @param dt Current delta tick.
 */
void gui_render( double dt )
{
   int i;
   double x;
   glColour col;

   /* If player is dead just render the cinematic mode. */
   if (!menu_isOpen(MENU_MAIN) &&
         (player_isFlag(PLAYER_DESTROYED) || player_isFlag(PLAYER_CREATING) ||
            ((player.p != NULL) && pilot_isFlag(player.p,PILOT_DEAD)))) {
      spfx_cinematic();
      return;
   }

   /* Make sure player is valid. */
   if (player.p == NULL)
      return;

   /* Cinematics mode. */
   if (player_isFlag( PLAYER_CINEMATICS_GUI ))
      return;

   /*
    * Countdown timers.
    */
   blink_pilot    -= dt / dt_mod;
   if (blink_pilot < 0.)
      blink_pilot += RADAR_BLINK_PILOT;
   blink_planet   -= dt / dt_mod;
   if (blink_planet < 0.)
      blink_planet += RADAR_BLINK_PLANET;
   if (interference_alpha > 0.)
      interference_t += dt;

   /* Render the border ships and targets. */
   gui_renderBorder(dt);

   /* Set viewport. */
   gl_viewport( 0., 0., gl_screen.rw, gl_screen.rh );

   /* Run Lua. */
   if (gui_env != LUA_NOREF) {
      gui_prepFunc( "render" );
      lua_pushnumber( naevL, dt );
      lua_pushnumber( naevL, dt_mod );
      gui_runFunc( "render", 2, 0 );
      if (pilot_isFlag(player.p, PILOT_COOLDOWN)) {
         gui_prepFunc( "render_cooldown" );
         lua_pushnumber( naevL, player.p->ctimer / player.p->cdelay  );
         lua_pushnumber( naevL, player.p->ctimer );
         gui_runFunc( "render_cooldown", 2, 0 );
      }
   }

   /* Messages. */
   gui_renderMessages(dt);


   /* OSD. */
   osd_render();

   /* Noise when getting near a jump. */
   if (player.p->nav_hyperspace >= 0) { /* hyperspace target */

      /* Determine if we have to play the "enter hyperspace range" sound. */
      i = space_canHyperspace(player.p);
      if ((i != 0) && (i != can_jump))
         if (!pilot_isFlag(player.p, PILOT_HYPERSPACE))
            player_soundPlayGUI(snd_jump, 1);
      can_jump = i;
   }

   /* Hyperspace. */
   if (pilot_isFlag(player.p, PILOT_HYPERSPACE) &&
         (player.p->ptimer < HYPERSPACE_FADEOUT)) {
      x = (HYPERSPACE_FADEOUT-player.p->ptimer) / HYPERSPACE_FADEOUT;
      col.r = 1.;
      col.g = 1.;
      col.b = 1.;
      col.a = x;
      gl_renderRect( 0., 0., SCREEN_W, SCREEN_H, &col );
   }

   /* Reset viewport. */
   gl_defViewport();

   /* Render messages. */
   omsg_render( dt );
}


/**
 * @brief Initializes the radar.
 *
 *    @param circle Whether or not the radar is circular.
 *    @param w Radar width.
 *    @param h Radar height.
 */
int gui_radarInit( int circle, int w, int h )
{
   gui_radar.shape   = circle ? RADAR_CIRCLE : RADAR_RECT;
   gui_radar.res     = RADAR_RES_DEFAULT;
   gui_radar.w       = w;
   gui_radar.h       = h;
   gui_createInterference( &gui_radar );
   return 0;
}


/**
 * @brief Renders the GUI radar.
 *
 *    @param x X position to render at.
 *    @param y Y position to render at.
 */
void gui_radarRender( double x, double y )
{
   int i, j;
   Radar *radar;
   AsteroidAnchor *ast;
   gl_Matrix4 view_matrix_prev;

   /* The global radar. */
   radar = &gui_radar;
   gui_radar.x = x;
   gui_radar.y = y;

   /* TODO: modifying gl_view_matrix like this is a bit of a hack */
   /* TODO: use stensil test for RADAR_CIRCLE */
   view_matrix_prev = gl_view_matrix;
   if (radar->shape==RADAR_RECT) {
      gl_clipRect( x, y, radar->w, radar->h );
      gl_view_matrix = gl_Matrix4_Translate(gl_view_matrix,
            x + radar->w/2., y + radar->h/2., 0 );
   }
   else if (radar->shape==RADAR_CIRCLE)
      gl_view_matrix = gl_Matrix4_Translate(gl_view_matrix,
            x, y, 0 );

   /*
    * planets
    */
   for (i=0; i<cur_system->nplanets; i++)
      if ((cur_system->planets[ i ]->real == ASSET_REAL) && (i != player.p->nav_planet))
         gui_renderPlanet( i, radar->shape, radar->w, radar->h, radar->res, 0 );
   if (player.p->nav_planet > -1)
      gui_renderPlanet( player.p->nav_planet, radar->shape, radar->w, radar->h, radar->res, 0 );

   /*
    * Jump points.
    */
   for (i=0; i<cur_system->njumps; i++)
      if (i != player.p->nav_hyperspace && jp_isUsable(&cur_system->jumps[i]))
         gui_renderJumpPoint( i, radar->shape, radar->w, radar->h, radar->res, 0 );
   if (player.p->nav_hyperspace > -1)
      gui_renderJumpPoint( player.p->nav_hyperspace, radar->shape, radar->w, radar->h, radar->res, 0 );

   /*
    * weapons
    */
   weapon_minimap( radar->res, radar->w, radar->h,
         radar->shape, 1.-interference_alpha );


   /* render the pilot_nstack */
   j = 0;
   for (i=1; i<pilot_nstack; i++) { /* skip the player */
      if (pilot_stack[i]->id == player.p->target)
         j = i;
      else
         gui_renderPilot( pilot_stack[i], radar->shape, radar->w, radar->h, radar->res, 0 );
   }
   /* render the targeted pilot */
   if (j!=0)
      gui_renderPilot( pilot_stack[j], radar->shape, radar->w, radar->h, radar->res, 0 );

   /* render the asteroids */
   for (i=0; i<cur_system->nasteroids; i++) {
      ast = &cur_system->asteroids[i];
      for (j=0; j<ast->nb; j++)
         gui_renderAsteroid( &ast->asteroids[j], radar->w, radar->h, radar->res, 0 );
   }

   /* Interference. */
   gui_renderInterference();

   /* Render the player cross. */
   gui_renderPlayer( radar->res, 0 );

   gl_view_matrix = view_matrix_prev;
   if (radar->shape==RADAR_RECT)
      gl_unclipRect();
}


/**
 * @brief Gets the radar's position.
 *
 *    @param[out] x X position.
 *    @param[out] y Y position.
 */
void gui_radarGetPos( int *x, int *y )
{
   *x = gui_radar.x;
   *y = gui_radar.y;
}


/**
 * @brief Gets the radar's dimensions.
 *
 *    @param[out] w Width.
 *    @param[out] h Height.
 */
void gui_radarGetDim( int *w, int *h )
{
   *w = gui_radar.w;
   *h = gui_radar.h;
}


/**
 * @brief Outputs the radar's resolution.
 *
 *    @param[out] res Current zoom ratio.
 */
void gui_radarGetRes( int *res )
{
   *res = gui_radar.res;
}


/**
 * @brief Clears the GUI messages.
 */
void gui_clearMessages (void)
{
   memset( mesg_stack, 0, sizeof(Mesg)*mesg_max );
}


/**
 * @brief Renders the player's messages on screen.
 *
 *    @param dt Current delta tick.
 */
static void gui_renderMessages( double dt )
{
   double x, y, h, hs, vx, vy, dy;
   int v, i, m, o;
   glColour c;

   /* Coordinate translation. */
   x = gui_mesg_x;
   y = gui_mesg_y;

   /* Handle viewpoint hacks. */
   v = mesg_viewpoint;
   if (v == -1)
      v = mesg_pointer;

   /* Colour. */
   c.r = 1.;
   c.g = 1.;
   c.b = 1.;

   /* Render background. */
   h = 0;

   /* Set up position. */
   vx = x;
   vy = y;

   /* Must be run here. */
   hs = 0.;
   o = 0;
   if (mesg_viewpoint != -1) {
      /* Data. */
      hs = h*(double)conf.mesg_visible/(double)mesg_max;
      o  = mesg_pointer - mesg_viewpoint;
      if (o < 0)
         o += mesg_max;
   }

   /* Render text. */
   for (i=0; i<conf.mesg_visible; i++) {
      /* Reference translation. */
      m  = (v - i) % mesg_max;
      if (m < 0)
         m += mesg_max;

      /* Timer handling. */
      if ((mesg_viewpoint != -1) || (mesg_stack[m].t >= 0.)) {
         /* Decrement timer. */
         if (mesg_viewpoint == -1)
            mesg_stack[m].t -= dt / dt_mod;

         /* Only handle non-NULL messages. */
         if (mesg_stack[m].str[0] != '\0') {
            if (mesg_stack[m].str[0] == '\t') {
               gl_printRestore( &mesg_stack[m].restore );
               dy = gl_printHeightRaw( NULL, gui_mesg_w, &mesg_stack[m].str[1]) + 6;
               gl_renderRect( x-4., y-1., gui_mesg_w-13., dy, &cBlackHilight );
               gl_printMaxRaw( NULL, gui_mesg_w - 45., x + 30, y + 3, &cFontWhite, -1., &mesg_stack[m].str[1] );
            } else {
               dy = gl_printHeightRaw( NULL, gui_mesg_w, &mesg_stack[m].str[1]) + 6;
               gl_renderRect( x-4., y-1., gui_mesg_w-13., dy, &cBlackHilight );
               gl_printMaxRaw( NULL, gui_mesg_w - 15., x, y + 3, &cFontWhite, -1., mesg_stack[m].str );
            }
            h += dy;
            y += dy;
         }
      }

      /* Increase position. */
   }

   /* Render position. */
   if (mesg_viewpoint != -1) {
      /* Border. */
      c.a = 0.2;
      gl_renderRect( vx + gui_mesg_w-10., vy, 10, h, &c );

      /* Inside. */
      c.a = 0.5;
      gl_renderRect( vx + gui_mesg_w-10., vy + hs/2. + (h-hs)*((double)o/(double)(mesg_max-conf.mesg_visible)), 10, hs, &c );
   }
}


/**
 * @brief Renders interference if needed.
 */
static void gui_renderInterference (void)
{
   glColour c;
   glTexture *tex;
   int t;

   /* Must be displaying interference. */
   if (interference_alpha <= 0.)
      return;

   /* Calculate frame to draw. */
   if (interference_t > INTERFERENCE_CHANGE_DT) { /* Time to change */
      t = RNG(0, INTERFERENCE_LAYERS-1);
      if (t != interference_layer)
         interference_layer = t;
      else
         interference_layer = (interference_layer == INTERFERENCE_LAYERS-1) ?
               0 : interference_layer+1;
      interference_t -= INTERFERENCE_CHANGE_DT;
   }

   /* Render the interference. */
   c.r = c.g = c.b = 1.;
   c.a = interference_alpha;
   tex = gui_radar.interference[interference_layer];
   if (gui_radar.shape == RADAR_CIRCLE)
      gl_blitStatic( tex, -gui_radar.w, -gui_radar.w, &c );
   else if (gui_radar.shape == RADAR_RECT)
      gl_blitStatic( tex, -gui_radar.w, -gui_radar.h, &c );
}


/**
 * @brief Gets a pilot's colour, with a special colour for targets.
 *
 *    @param p Pilot to get colour of.
 *    @return The colour of the pilot.
 *
 * @sa pilot_getColour
 */
static const glColour* gui_getPilotColour( const Pilot* p )
{
   const glColour *col;

   if (p->id == player.p->target)
      col = &cRadar_tPilot;
   else
      col = pilot_getColour(p);

   return col;
}


/**
 * @brief Renders a pilot in the GUI radar.
 *
 *    @param p Pilot to render.
 *    @param shape Shape of the radar (RADAR_RECT or RADAR_CIRCLE).
 *    @param w Width.
 *    @param h Height.
 *    @param res Radar resolution.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderPilot( const Pilot* p, RadarShape shape, double w, double h, double res, int overlay )
{
   int x, y, sx, sy;
   double px, py;
   glColour col;

   /* Make sure is in range. */
   if (!pilot_inRangePilot( player.p, p, NULL ))
      return;

   /* Get position. */
   if (overlay) {
      x = (int)(p->solid->pos.x / res);
      y = (int)(p->solid->pos.y / res);
   }
   else {
      x = (int)((p->solid->pos.x - player.p->solid->pos.x) / res);
      y = (int)((p->solid->pos.y - player.p->solid->pos.y) / res);
   }
   /* Get size. */
   sx = (int)(PILOT_SIZE_APROX/2. * p->ship->gfx_space->sw / res);
   sy = (int)(PILOT_SIZE_APROX/2. * p->ship->gfx_space->sh / res);
   if (sx < 1.)
      sx = 1.;
   if (sy < 1.)
      sy = 1.;

   /* Check if pilot in range. */
   if ( ((shape==RADAR_RECT) &&
            ((ABS(x) > w/2+sx) || (ABS(y) > h/2.+sy)) ) ||
         ((shape==RADAR_CIRCLE) &&
            ((x*x+y*y) > (int)(w*w))) ) {

      /* Draw little targeted symbol. */
      if (p->id == player.p->target && !overlay)
         gui_renderRadarOutOfRange( shape, w, h, x, y, &cRadar_tPilot );
      return;
   }

   /* Transform coordinates into the 0,0 -> SCREEN_W, SCREEN_H range. */
   if (overlay) {
      x += SCREEN_W / 2;
      y += SCREEN_H / 2;
      w *= 2.;
      h *= 2.;
   }

   /* Draw selection if targeted. */
   if (p->id == player.p->target) {
      if (blink_pilot < RADAR_BLINK_PILOT/2.) {
         col = cRadar_tPilot;
         col.a = 1.-interference_alpha;

         gl_beginSolidProgram(gl_Matrix4_Translate(gl_view_matrix, x, y, 0), &cRadar_tPilot);
         gl_vboActivateAttribOffset( gui_radar_select_vbo, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
         glDrawArrays( GL_LINES, 0, 8 );
         gl_endSolidProgram();
      }
   }

   /* Draw square. */
   px     = MAX(x-sx,-w);
   py     = MAX(y-sy, -h);
   if (pilot_isFlag(p, PILOT_HILIGHT) && (blink_pilot < RADAR_BLINK_PILOT/2.))
      col = cRadar_hilight;
   else
      col = *gui_getPilotColour(p);
   col.a = 1.-interference_alpha;
   gl_renderRect( px, py, MIN( 2*sx, w-px ), MIN( 2*sy, h-py ), &col );

   /* Draw name. */
   if (overlay && pilot_isFlag(p, PILOT_HILIGHT))
      gl_printRaw( &gl_smallFont, x+2*sx+5., y-gl_smallFont.h/2., &col, -1., p->name );
}


/**
 * @brief Renders an asteroid in the GUI radar.
 *
 *    @param a Asteroid to render.
 *    @param w Width.
 *    @param h Height.
 *    @param res Radar resolution.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderAsteroid( const Asteroid* a, double w, double h, double res, int overlay )
{
   int x, y, sx, sy, i, j, targeted;
   double px, py;
   const glColour *col;
   glColour ccol;

   /* Skip invisible asteroids */
   if (a->appearing == ASTEROID_INVISIBLE)
      return;

   /* Recover the asteroid and field IDs. */
   i = a->id;
   j = a->parent;

   /* Make sure is in range. */
   if (!pilot_inRangeAsteroid( player.p, i, j ))
      return;

   /* Get position. */
   if (overlay) {
      x = (int)(a->pos.x / res);
      y = (int)(a->pos.y / res);
   }
   else {
      x = (int)((a->pos.x - player.p->solid->pos.x) / res);
      y = (int)((a->pos.y - player.p->solid->pos.y) / res);
   }
   /* Get size. */
   sx = 1.;
   sy = 1.;

   /* Transform coordinates into the 0,0 -> SCREEN_W, SCREEN_H range. */
   if (overlay) {
      x += SCREEN_W / 2;
      y += SCREEN_H / 2;
      w *= 2.;
      h *= 2.;
   }

   /* Draw square. */
   px     = MAX(x-sx,-w);
   py     = MAX(y-sy, -h);

   targeted = ((i==player.p->nav_asteroid) && (j==player.p->nav_anchor));

   /* Colour depends if the asteroid is selected. */
   if (targeted)
      col = &cWhite;
   else
      col = &cGrey70;

   ccol.r = col->r;
   ccol.g = col->g;
   ccol.b = col->b;
   ccol.a = 1.-interference_alpha;
   gl_renderRect( px, py, MIN( 2*sx, w-px ), MIN( 2*sy, h-py ), &ccol );

   if (targeted && (blink_pilot >= RADAR_BLINK_PILOT/2.)) {
      gl_beginSolidProgram(gl_Matrix4_Translate(gl_view_matrix, x, y, 0), &ccol);
      gl_vboActivateAttribOffset( gui_radar_select_vbo, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
      glDrawArrays( GL_LINES, 0, 8 );
      gl_endSolidProgram();
   }
}


/**
 * @brief Renders the player cross on the radar or whatever.
 */
void gui_renderPlayer( double res, int overlay )
{
   double x, y, r;
   glColour textCol = { cRadar_player.r, cRadar_player.g, cRadar_player.b, 0.99 };
   /* XXX: textCol is a hack to prevent the text from overly obscuring
    * overlay display of other things. Effectively disables outlines for
    * the text. Should ultimately be replaced with some other method of
    * rendering the text, since the problem could be caused by as little
    * as a font change, but using this fix for now. */

   if (overlay) {
      x = player.p->solid->pos.x / res + SCREEN_W / 2;
      y = player.p->solid->pos.y / res + SCREEN_H / 2;
      r = 5.;
   }
   else {
      x = 0.;
      y = 0.;
      r = 3.;
   }

   /* Render the cross. */
   gl_renderCross( x, y, r, &cRadar_player );

   if (overlay)
      gl_printRaw( &gl_smallFont, x+r+5., y-gl_smallFont.h/2., &textCol, -1., _("You") );
}


/**
 * @brief Gets the colour of a planet.
 *
 *    @param i Index of the planet to get colour of.
 *    @return Colour of the planet.
 */
static const glColour *gui_getPlanetColour( int i )
{
   const glColour *col;
   Planet *planet;

   planet = cur_system->planets[i];

   if (i == player.p->nav_planet)
      col = &cRadar_tPlanet;
   else
      col = planet_getColour( planet );

   return col;
}


/**
 * @brief Force sets the planet and pilot radar blink.
 */
void gui_forceBlink (void)
{
   blink_pilot  = 0.;
   blink_planet = 0.;
}


/**
 * @brief Renders the planet blink around a position on the minimap.
 */
static void gui_planetBlink( int w, int h, int rc, int cx, int cy, GLfloat vr, RadarShape shape )
{
   (void) w;
   (void) h;
   (void) rc;
   (void) shape;
   glColour col;
   gl_Matrix4 projection;

   if (blink_planet < RADAR_BLINK_PLANET/2.) {
      col = cRadar_tPlanet;
      col.a = 1.-interference_alpha;

      projection = gl_Matrix4_Translate(gl_view_matrix, cx, cy, 0);
      projection = gl_Matrix4_Scale(projection, vr, vr, 1);
      gl_beginSolidProgram(projection, &col);
      gl_vboActivateAttribOffset( gui_planet_blink_vbo, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
      glDrawArrays( GL_LINES, 0, 8 );
      gl_endSolidProgram();
   }
}


/**
 * @brief Renders an out of range marker for the planet.
 */
static void gui_renderRadarOutOfRange( RadarShape sh, int w, int h, int cx, int cy, const glColour *col )
{
   double a, x, y, x2, y2;
   glColour c;

   /* Draw a line like for pilots. */
   a = ANGLE(cx,cy);
   if (sh == RADAR_CIRCLE) {
      x = w * cos(a);
      y = w * sin(a);
   }
   else {
      int cxa, cya;
      cxa = ABS(cx);
      cya = ABS(cy);
      /* Determine position. */
      if (cy >= cxa) { /* Bottom */
         x = w/2. * (cx*1./cy);
         y = h/2.;
      } else if (cx >= cya) { /* Left */
         x = w/2.;
         y = h/2. * (cy*1./cx);
      } else if (cya >= cxa) { /* Top */
         x = -w/2. * (cx*1./cy);
         y = -h/2.;
      } else { /* Right */
         x = -w/2.;
         y = -h/2. * (cy*1./cx);
      }
   }
   x2 = x - .15 * w * cos(a);
   y2 = y - .15 * w * sin(a);

   c = *col;
   c.a = 1.-interference_alpha;

   gl_drawLine( x, y, x2, y2, &c );
}


/**
 * @brief Draws the planets in the minimap.
 *
 * Matrix mode is already displaced to center of the minimap.
 */
void gui_renderPlanet( int ind, RadarShape shape, double w, double h, double res, int overlay )
{
   int x, y;
   int cx, cy, r, rc;
   GLfloat vr;
   glColour col;
   Planet *planet;

   /* Make sure is known. */
   if ( !planet_isKnown( cur_system->planets[ind] ))
      return;

   /* Default values. */
   planet = cur_system->planets[ind];
   r     = (int)(planet->radius*2. / res);
   vr    = MAX( r, 3. ); /* Make sure it's visible. */
   if (overlay) {
      cx    = (int)(planet->pos.x / res);
      cy    = (int)(planet->pos.y / res);
   }
   else {
      cx    = (int)((planet->pos.x - player.p->solid->pos.x) / res);
      cy    = (int)((planet->pos.y - player.p->solid->pos.y) / res);
   }
   if (shape==RADAR_CIRCLE)
      rc = (int)(w*w);
   else
      rc = 0;

   /* Check if in range. */
   if (shape == RADAR_RECT) {
      /* Out of range. */
      if ((ABS(cx) - r > w/2.) || (ABS(cy) - r  > h/2.)) {
         if ((player.p->nav_planet == ind) && !overlay)
            gui_renderRadarOutOfRange( RADAR_RECT, w, h, cx, cy, &cRadar_tPlanet );
         return;
      }
   }
   else if (shape == RADAR_CIRCLE) {
      x = ABS(cx)-r;
      y = ABS(cy)-r;
      /* Out of range. */
      if (x*x + y*y > pow2(w-r)) {
         if ((player.p->nav_planet == ind) && !overlay)
            gui_renderRadarOutOfRange( RADAR_CIRCLE, w, w, cx, cy, &cRadar_tPlanet );
         return;
      }
   }

   if (overlay) {
      /* Transform coordinates. */
      cx += SCREEN_W / 2;
      cy += SCREEN_H / 2;
      w  *= 2.;
      h  *= 2.;
   }

   /* Do the blink. */
   if (ind == player.p->nav_planet)
      gui_planetBlink( w, h, rc, cx, cy, vr, shape );

   /* Get the colour. */
   col = *gui_getPlanetColour(ind);
   if (!overlay)
      col.a = 1.-interference_alpha;

   gl_beginSolidProgram(gl_Matrix4_Scale(gl_Matrix4_Translate(gl_view_matrix, cx, cy, 0), vr, vr, 1), &col);
   gl_vboActivateAttribOffset( gui_planet_vbo, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 5 );
   gl_endSolidProgram();

   /* Render name. */
   /* XXX: Hack to prevent the text from overly obscuring overlay
    * display of other things. Effectively disables outlines for this
    * text. Should ultimately be replaced with some other method of
    * rendering the text, since the problem could be caused by as little
    * as a font change, but using this fix for now. */
   col.a = MIN( col.a, 0.99 );
   if (overlay)
      gl_printRaw( &gl_smallFont, cx+vr+5., cy, &col, -1., planet->name );
}


/**
 * @brief Renders a jump point on the minimap.
 *
 *    @param ind Jump point to render.
 *    @param shape Shape of the radar (RADAR_RECT or RADAR_CIRCLE).
 *    @param w Width.
 *    @param h Height.
 *    @param res Radar resolution.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderJumpPoint( int ind, RadarShape shape, double w, double h, double res, int overlay )
{
   int cx, cy, x, y, r, rc;
   GLfloat vr;
   glColour col;
   JumpPoint *jp;
   gl_Matrix4 projection;

   /* Default values. */
   jp    = &cur_system->jumps[ind];
   r     = (int)(jumppoint_gfx->sw / res);
   vr    = MAX( r, 3. ); /* Make sure it's visible. */
   if (overlay) {
      cx    = (int)(jp->pos.x / res);
      cy    = (int)(jp->pos.y / res);
   }
   else {
      cx    = (int)((jp->pos.x - player.p->solid->pos.x) / res);
      cy    = (int)((jp->pos.y - player.p->solid->pos.y) / res);
   }
   if (shape==RADAR_CIRCLE)
      rc = (int)(w*w);
   else
      rc = 0;

   /* Check if known */
   if (!jp_isKnown(jp))
      return;

   /* Check if in range. */
   if (shape == RADAR_RECT) {
      /* Out of range. */
      if ((ABS(cx) - r > w/2.) || (ABS(cy) - r  > h/2.)) {
         if ((player.p->nav_hyperspace == ind) && !overlay)
            gui_renderRadarOutOfRange( RADAR_RECT, w, h, cx, cy, &cRadar_tPlanet );
         return;
      }
   }
   else if (shape == RADAR_CIRCLE) {
      x = ABS(cx)-r;
      y = ABS(cy)-r;
      /* Out of range. */
      if (x*x + y*y > pow2(w-r)) {
         if ((player.p->nav_hyperspace == ind) && !overlay)
            gui_renderRadarOutOfRange( RADAR_CIRCLE, w, w, cx, cy, &cRadar_tPlanet );
         return;
      }
   }

   if (overlay) {
      /* Transform coordinates. */
      cx += SCREEN_W / 2;
      cy += SCREEN_H / 2;
      w  *= 2.;
      h  *= 2.;
   }

   /* Do the blink. */
   if (ind == player.p->nav_hyperspace) {
      gui_planetBlink( w, h, rc, cx, cy, vr, shape );
      col = cGreen;
   }
   else if (jp_isFlag(jp, JP_HIDDEN))
      col = cRed;
   else
      col = cWhite;

   if (!overlay)
      col.a = 1.-interference_alpha;

   projection = gl_Matrix4_Translate(gl_view_matrix, cx, cy, 0);
   projection = gl_Matrix4_Rotate2d(projection, -M_PI/2-jp->angle);
   gl_beginSolidProgram(projection, &col);
   gl_vboActivateAttribOffset( gui_triangle_vbo, shaders.solid.vertex, 0, 2, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 4 );
   gl_endSolidProgram();

   /* Render name. */
   /* XXX: Hack to prevent the text from overly obscuring overlay
    * display of other things. Effectively disables outlines for this
    * text. Should ultimately be replaced with some other method of
    * rendering the text, since the problem could be caused by as little
    * as a font change, but using this fix for now. */
   col.a = MIN( col.a, 0.99 );
   if (overlay)
      gl_printRaw( &gl_smallFont, cx+vr+5., cy, &col, -1., sys_isKnown(jp->target) ? jp->target->name : _("Unknown") );
}


/**
 * @brief Sets the viewport.
 */
void gui_setViewport( double x, double y, double w, double h )
{
   gui_viewport_x = x;
   gui_viewport_y = y;
   gui_viewport_w = w;
   gui_viewport_h = h;

   /* We now set the viewport. */
   gl_setDefViewport( gui_viewport_x, gui_viewport_y, gui_viewport_w, gui_viewport_h );
   gl_defViewport();

   /* Run border calculations. */
   gui_calcBorders();

   /* Regenerate the Nebula stuff. */
   if ((cur_system != NULL) && (cur_system->nebu_density > 0.))
      nebu_genOverlay();
}


/**
 * @brief Resets the viewport.
 */
void gui_clearViewport (void)
{
   gl_setDefViewport( 0., 0., gl_screen.nw, gl_screen.nh );
   gl_defViewport();
}


/**
 * @brief Calculates and sets the GUI borders.
 */
static void gui_calcBorders (void)
{
   double w,h;

   /* Precalculations. */
   w  = SCREEN_W/2.;
   h  = SCREEN_H/2.;

   /*
    * Borders.
    */
   gui_tl = atan2( +h, -w );
   if (gui_tl < 0.)
      gui_tl += 2*M_PI;
   gui_tr = atan2( +h, +w );
   if (gui_tr < 0.)
      gui_tr += 2*M_PI;
   gui_bl = atan2( -h, -w );
   if (gui_bl < 0.)
      gui_bl += 2*M_PI;
   gui_br = atan2( -h, +w );
   if (gui_br < 0.)
      gui_br += 2*M_PI;
}


/**
 * @brief Initializes the GUI system.
 *
 *    @return 0 on success;
 */
int gui_init (void)
{
   GLfloat vertex[16];

   /*
    * radar
    */
   gui_radar.res = RADAR_RES_DEFAULT;

   /*
    * messages
    */
   gui_mesg_x = 20;
   gui_mesg_y = 30;
   gui_mesg_w = SCREEN_W - 400;
   if (mesg_stack == NULL) {
      mesg_stack = calloc(mesg_max, sizeof(Mesg));
      if (mesg_stack == NULL) {
         ERR(_("Out of Memory"));
         return -1;
      }
   }

   /*
    * VBO.
    */
   if (gui_triangle_vbo == NULL) {
         vertex[0] = -5.;
         vertex[1] = -5.;
         vertex[2] = 5.;
         vertex[3] = -5.;
         vertex[4] = 0;
         vertex[5] = 5.;
         vertex[6] = -5.;
         vertex[7] = -5.;
      gui_triangle_vbo = gl_vboCreateStatic( sizeof(GLfloat) * 8, vertex );
   }

   if (gui_planet_vbo == NULL) {
      vertex[0] = 0;
      vertex[1] = 1;
      vertex[2] = 1;
      vertex[3] = 0;
      vertex[4] = 0;
      vertex[5] = -1;
      vertex[6] = -1;
      vertex[7] = 0;
      vertex[8] = 0;
      vertex[9] = 1;
      gui_planet_vbo = gl_vboCreateStatic( sizeof(GLfloat) * 10, vertex );
   }

   if (gui_radar_select_vbo == NULL) {
      vertex[0] = -1.5;
      vertex[1] = 1.5;
      vertex[2] = -3.3;
      vertex[3] = 3.3;
      vertex[4] = 1.5;
      vertex[5] = 1.5;
      vertex[6] = 3.3;
      vertex[7] = 3.3;
      vertex[8] = 1.5;
      vertex[9] = -1.5;
      vertex[10] = 3.3;
      vertex[11] = -3.3;
      vertex[12] = -1.5;
      vertex[13] = -1.5;
      vertex[14] = -3.3;
      vertex[15] = -3.3;
      gui_radar_select_vbo = gl_vboCreateStatic( sizeof(GLfloat) * 16, vertex );
   }

   if (gui_planet_blink_vbo == NULL) {
      vertex[0] = -1;
      vertex[1] = 1;
      vertex[2] = -1.2;
      vertex[3] = 1.2;
      vertex[4] = 1;
      vertex[5] = 1;
      vertex[6] = 1.2;
      vertex[7] = 1.2;
      vertex[8] = 1;
      vertex[9] = -1;
      vertex[10] = 1.2;
      vertex[11] = -1.2;
      vertex[12] = -1;
      vertex[13] = -1;
      vertex[14] = -1.2;
      vertex[15] = -1.2;
      gui_planet_blink_vbo = gl_vboCreateStatic( sizeof(GLfloat) * 16, vertex );
   }

   /*
    * OSD
    */
   osd_setup( 30., SCREEN_H-90., 150., 300. );

   /*
    * Set viewport.
    */
   gui_setViewport( 0., 0., gl_screen.w, gl_screen.h );

   /*
    * Icons.
    */
   gui_ico_hail = gl_newSprite( GUI_GFX_PATH"hail.png", 5, 2, 0 );

   return 0;
}


/**
 * @brief Runs a GUI Lua function.
 *
 *    @param func Name of the function to run.
 *    @return 0 on success.
 */
static int gui_doFunc( const char* func )
{
   gui_prepFunc( func );
   return gui_runFunc( func, 0, 0 );
}


/**
 * @brief Prepares to run a function.
 */
static int gui_prepFunc( const char* func )
{
#if DEBUGGING
   if (gui_env == LUA_NOREF) {
      WARN( _("Trying to run GUI func '%s' but no GUI is loaded!"), func );
      return -1;
   }
#endif /* DEBUGGING */

   /* Set up function. */
   nlua_getenv( gui_env, func );
   return 0;
}


/**
 * @brief Runs a function.
 * @note Function must be prepared beforehand.
 *    @param func Name of the function to run.
 *    @param nargs Arguments to the function.
 *    @param nret Parameters to get returned from the function.
 */
static int gui_runFunc( const char* func, int nargs, int nret )
{
   int ret;
   const char* err;

   /* Run the function. */
   ret = nlua_pcall( gui_env, nargs, nret );
   if (ret != 0) { /* error has occurred */
      err = (lua_isstring(naevL,-1)) ? lua_tostring(naevL,-1) : NULL;
      WARN(_("GUI Lua -> '%s': %s"),
            func, (err) ? err : _("unknown error"));
      lua_pop(naevL,2);
      return ret;
   }

   return ret;
}


/**
 * @brief Reloads the GUI.
 */
void gui_reload (void)
{
   if (gui_env == LUA_NOREF)
      return;

   gui_load( gui_pick() );
}


/**
 * @brief Player just changed their cargo.
 */
void gui_setCargo (void)
{
   if (gui_env != LUA_NOREF)
      gui_doFunc( "update_cargo" );
}


/**
 * @brief PlNULLayer just changed their nav computer target.
 */
void gui_setNav (void)
{
   if (gui_env != LUA_NOREF)
      gui_doFunc( "update_nav" );
}


/**
 * @brief Player just changed their pilot target.
 */
void gui_setTarget (void)
{
   if (gui_env != LUA_NOREF)
      gui_doFunc( "update_target" );
}


/**
 * @brief Player just upgraded their ship or modified it.
 */
void gui_setShip (void)
{
   if (gui_env != LUA_NOREF)
      gui_doFunc( "update_ship" );
}


/**
 * @brief Player just changed their system.
 */
void gui_setSystem (void)
{
   if (gui_env != LUA_NOREF)
      gui_doFunc( "update_system" );
}


/**
 * @brief Player's relationship with a faction was modified.
 */
void gui_updateFaction (void)
{
   if (gui_env != LUA_NOREF && player.p->nav_planet != -1)
      gui_doFunc( "update_faction" );
}


/**
 * @brief Calls trigger functions depending on who the pilot is.
 *
 *    @param pilot The pilot to act based upon.
 */
void gui_setGeneric( Pilot* pilot )
{
   if (gui_env == LUA_NOREF)
      return;

   if ((player.p->target != PLAYER_ID) && (pilot->id == player.p->target))
      gui_setTarget();
   else if (pilot_isPlayer(pilot)) {
      gui_setCargo();
      gui_setShip();
   }
}


/**
 * @brief Determines which GUI should be used.
 */
char* gui_pick (void)
{
   char* gui;

   if (player.gui && (player.guiOverride == 1 || strcmp(player.p->ship->gui,"default")==0))
      gui = player.gui;
   else
      gui = player.p->ship->gui;
   return gui;
}


/**
 * @brief Attempts to load the actual GUI.
 *
 *    @param name Name of the GUI to load.
 *    @return 0 on success.
 */
int gui_load( const char* name )
{
   (void) name;
   char *buf, path[PATH_MAX];
   size_t bufsize;

   /* Set defaults. */
   gui_cleanup();

   /* Open file. */
   nsnprintf( path, sizeof(path), GUI_PATH"%s.lua", name );
   buf = ndata_read( path, &bufsize );
   if (buf == NULL) {
      WARN(_("Unable to find GUI '%s'."), path );
      return -1;
   }

   /* Clean up. */
   if (gui_env != LUA_NOREF) {
      nlua_freeEnv(gui_env);
      gui_env = LUA_NOREF;
   }

   /* Create Lua state. */
   gui_env = nlua_newEnv(1);
   if (nlua_dobufenv( gui_env, buf, bufsize, path ) != 0) {
      WARN(_("Failed to load GUI Lua: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check"),
            path, lua_tostring(naevL,-1));
      nlua_freeEnv( gui_env );
      gui_env = LUA_NOREF;
      free(buf);
      return -1;
   }
   free(buf);
   nlua_loadStandard( gui_env );
   nlua_loadGFX( gui_env );
   nlua_loadGUI( gui_env );
   nlua_loadTk( gui_env );

   /* Run create function. */
   if (gui_doFunc( "create" )) {
      nlua_freeEnv( gui_env );
      gui_env = LUA_NOREF;
   }

   /* Recreate land window if landed. */
   if (landed) {
      land_genWindows( 0, 1 );
      window_lower( land_wid );
   }

   return 0;
}


/**
 * @brief Creates the interference map for the current gui.
 */
static void gui_createInterference( Radar *radar )
{
   uint8_t raw;
   int i, j, k;
   float *map;
   uint32_t *pix;
   SDL_Surface *sur;
   int w,h, hw,hh;
   float c;
   int r;

   /* Dimension shortcuts. */
   if (radar->shape == RADAR_CIRCLE) {
      w = radar->w*2.;
      h = w;
   }
   else if (radar->shape == RADAR_RECT) {
      w = radar->w*2.;
      h = radar->h*2.;
   }
   else {
      WARN( _("Radar shape is invalid.") );
      return;
   }

   for (k=0; k<INTERFERENCE_LAYERS; k++) {

      /* Free the old texture. */
      if (radar->interference[k] != NULL)
         gl_freeTexture(radar->interference[k]);

      /* Create the temporary surface. */
      sur = SDL_CreateRGBSurface( SDL_SWSURFACE, w, h, 32, RGBAMASK );
      pix = sur->pixels;

      /* Clear pixels. */
      memset( pix, 0, sizeof(uint32_t)*w*h );

      /* Load the interference map. */
      map = noise_genRadarInt( w, h, (w+h)/2*1.2 );

      /* Create the texture. */
      SDL_LockSurface( sur );
      if (radar->shape == RADAR_CIRCLE) {
         r = pow2((int)radar->w);
         hw = w/2;
         hh = h/2;
         for (i=0; i<h; i++) {
            for (j=0; j<w; j++) {
               /* Must be in circle. */
               if (pow2(i-hh) + pow2(j-hw) > r)
                  continue;
               c = map[i*w + j];
               raw = 0xff & (uint8_t)((float)0xff * c);
               memset( &pix[i*w + j], raw, sizeof(uint32_t) );
               pix[i*w + j] |= AMASK;
            }
         }
      }
      else if (radar->shape == RADAR_RECT) {
         for (i=0; i<h*w; i++) {
            /* Process pixels. */
            c = map[i];
            raw = 0xff & (uint8_t)((float)0xff * c);
            memset( &pix[i], raw, sizeof(uint32_t) );
            pix[i] |= AMASK;
         }
      }
      SDL_UnlockSurface( sur );

      /* Set the interference. */
      radar->interference[k] = gl_loadImage( sur, 0 );

      /* Clean up. */
      free(map);
   }
}


/**
 * @brief Cleans up the GUI.
 */
void gui_cleanup (void)
{
   int i;

   /* Disable mouse voodoo. */
   gui_mouseClickEnable( 0 );
   gui_mouseMoveEnable( 0 );

   /* Interference. */
   for (i=0; i<INTERFERENCE_LAYERS; i++) {
      if (gui_radar.interference[i] != NULL) {
         gl_freeTexture(gui_radar.interference[i]);
         gui_radar.interference[i] = NULL;
      }
   }

   /* Set the viewport. */
   gui_clearViewport();

   /* Reset FPS. */
   fps_setPos( 15., (double)(gl_screen.h-15-gl_defFont.h) );

   /* Clean up interference. */
   interference_alpha = 0.;
   interference_layer = 0;
   interference_t     = 0.;

   /* Destroy offset. */
   gui_xoff = 0.;
   gui_yoff = 0.;

   /* Destroy lua. */
   if (gui_env != LUA_NOREF) {
      nlua_freeEnv( gui_env );
      gui_env = LUA_NOREF;
   }

   /* OMSG */
   omsg_position( SCREEN_W/2., SCREEN_H*2./3., SCREEN_W*2./3. );
}


/**
 * @brief Frees the gui stuff.
 */
void gui_free (void)
{
   /* Clean up gui. */
   gui_cleanup();

   /* Free messages. */
   if (mesg_stack != NULL) {
      free(mesg_stack);
      mesg_stack = NULL;
   }

   /* Free VBOs. */
   if (gui_triangle_vbo != NULL) {
      gl_vboDestroy( gui_triangle_vbo );
      gui_triangle_vbo = NULL;
   }
   if (gui_planet_vbo != NULL) {
      gl_vboDestroy( gui_planet_vbo );
      gui_planet_vbo = NULL;
   }
   if (gui_radar_select_vbo != NULL) {
      gl_vboDestroy( gui_radar_select_vbo );
      gui_radar_select_vbo = NULL;
   }
   if (gui_planet_blink_vbo != NULL) {
      gl_vboDestroy( gui_planet_blink_vbo );
      gui_planet_blink_vbo = NULL;
   }

   /* Clean up the OSD. */
   osd_exit();

   /* Free icons. */
   if (gui_ico_hail != NULL)
      gl_freeTexture( gui_ico_hail );
   gui_ico_hail = NULL;
   if (gui_target_planet != NULL)
      gl_freeTexture( gui_target_planet );
   gui_target_planet = NULL;
   if (gui_target_pilot != NULL)
      gl_freeTexture( gui_target_pilot );
   gui_target_pilot = NULL;

   /* Free overlay messages. */
   omsg_cleanup();
}


/**
 * @brief Modifies the radar resolution.
 *
 *    @param mod Number of intervals to jump (up or down).
 */
void gui_setRadarRel( int mod )
{
   gui_radar.res += mod * RADAR_RES_INTERVAL;
   gui_radar.res = CLAMP( RADAR_RES_MIN, RADAR_RES_MAX, gui_radar.res );

   player_message( _("\apRadar set to %dx."), (int)gui_radar.res );
}


/**
 * @brief Gets the GUI offset.
 *
 *    @param x X offset.
 *    @param y Y offset.
 */
void gui_getOffset( double *x, double *y )
{
   *x = gui_xoff;
   *y = gui_yoff;
}


/**
 * @brief Gets the hail icon texture.
 */
glTexture* gui_hailIcon (void)
{
   return gui_ico_hail;
}


/**
 * @brief Sets the planet target GFX.
 */
void gui_targetPlanetGFX( glTexture *gfx )
{
   if (gui_target_planet != NULL)
      gl_freeTexture( gui_target_planet );
   gui_target_planet = gl_dupTexture( gfx );
}


/**
 * @brief Sets the pilot target GFX.
 */
void gui_targetPilotGFX( glTexture *gfx )
{
   if (gui_target_pilot != NULL)
      gl_freeTexture( gui_target_pilot );
   gui_target_pilot = gl_dupTexture( gfx );
}


/**
 * @brief Handles GUI events.
 */
int gui_handleEvent( SDL_Event *evt )
{
   int ret;
   int x, y;

   if (player.p == NULL)
      return 0;
   if ((evt->type == SDL_MOUSEBUTTONDOWN) &&
         (pilot_isFlag(player.p,PILOT_HYP_PREP) ||
         pilot_isFlag(player.p,PILOT_HYP_BEGIN) ||
         pilot_isFlag(player.p,PILOT_HYPERSPACE)))
      return 0;

   ret = 0;
   switch (evt->type) {
      /* Mouse motion. */
      case SDL_MOUSEMOTION:
         if (!gui_L_mmove)
            break;
         gui_prepFunc( "mouse_move" );
         gl_windowToScreenPos( &x, &y, evt->motion.x - gui_viewport_x,
               evt->motion.y - gui_viewport_y );
         lua_pushnumber( naevL, x );
         lua_pushnumber( naevL, y );
         gui_runFunc( "mouse_move", 2, 0 );
         break;

      /* Mouse click. */
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
         if (!gui_L_mclick)
            break;
         gui_prepFunc( "mouse_click" );
         lua_pushnumber( naevL, evt->button.button+1 );
         gl_windowToScreenPos( &x, &y, evt->button.x - gui_viewport_x,
            evt->button.y - gui_viewport_y );
         lua_pushnumber( naevL, x );
         lua_pushnumber( naevL, y );
         lua_pushboolean( naevL, (evt->type==SDL_MOUSEBUTTONDOWN) );
         gui_runFunc( "mouse_click", 4, 1 );
         ret = lua_toboolean( naevL, -1 );
         lua_pop( naevL, 1 );
         break;

      /* Not interested in the rest. */
      default:
         break;
   }
   return ret;
}


/**
 * @brief Enables the mouse click callback.
 */
void gui_mouseClickEnable( int enable )
{
   gui_L_mclick = enable;
}


/**
 * @brief Enables the mouse movement callback.
 */
void gui_mouseMoveEnable( int enable )
{
   gui_L_mmove = enable;
}
