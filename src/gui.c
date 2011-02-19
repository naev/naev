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
#include <string.h>

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
#include "nlua.h"
#include "nluadef.h"
#include "nlua_gfx.h"
#include "nlua_gui.h"
#include "nlua_tex.h"
#include "gui_omsg.h"


#define XML_GUI_ID   "GUIs" /**< XML section identifier for GUI document. */
#define XML_GUI_TAG  "gui" /**<  XML Section identifier for GUI tags. */

#define GUI_GFX      "gfx/gui/" /**< Location of the GUI graphics. */

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
static gl_vbo *gui_vbo = NULL; /**< GUI VBO. */
static GLsizei gui_vboColourOffset = 0; /**< Offset of colour pixels. */

static int gui_getMessage     = 1; /**< Whether or not the palyer should recieve messages. */

/*
 * pilot stuff for GUI
 */
extern Pilot** pilot_stack; /**< @todo remove */
extern int pilot_nstack; /**< @todo remove */

/*
 * map stuff for autonav
 */
extern int map_npath; /**< @todo remove. */


/**
 * GUI Lua stuff.
 */
static lua_State *gui_L; /**< Current GUI lua State. */
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
static double mesg_fade    = 5.; /**< Fade length. */
/**
 * @struct Mesg
 *
 * @brief On screen player message.
 */
typedef struct Mesg_ {
   char str[MESG_SIZE_MAX]; /**< The message. */
   double t; /**< Time to live for the message. */
} Mesg;
static Mesg* mesg_stack = NULL; /**< Stack of mesages, will be of mesg_max size. */
static int gui_mesg_w   = 0; /**< Width of messages. */
static int gui_mesg_x   = 0; /**< X positioning of messages. */
static int gui_mesg_y   = 0; /**< Y positioning of messages. */

/* Calculations to speed up borders. */
static double gui_tr = 0.; /**< Border top-right. */
static double gui_br = 0.; /**< Border bottom-right. */
static double gui_tl = 0.; /**< Border top-left. */
static double gui_bl = 0.; /**< Border bottom-left. */

/* Intrinsec graphical stuff. */
static glTexture *gui_ico_hail      = NULL; /**< Hailing icon. */
static glTexture *gui_target_planet = NULL; /**< Planet targetting icon. */
static glTexture *gui_target_pilot  = NULL; /**< Pilot targetting icon. */


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
static glColour *gui_getPlanetColour( int i );
static void gui_renderRadarOutOfRange( RadarShape sh, int w, int h, int cx, int cy, glColour *col );
static void gui_planetBlink( int w, int h, int rc, int cx, int cy, GLfloat vr, RadarShape shape );
static glColour* gui_getPilotColour( const Pilot* p );
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
 * @brief Toggles if player should recieve messages.
 *
 *    @param enable Whether or not to enable player recieving messages.
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

   /* Must be recieving messages. */
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
      if (p == 0)
         snprintf( mesg_stack[mesg_pointer].str, i+1, "%s", &str[p] );
      else {
         mesg_stack[mesg_pointer].str[0] = '\t'; /* Hack to indent. */
         snprintf( &mesg_stack[mesg_pointer].str[1], i+1, "%s", &str[p] );
      }
      mesg_stack[mesg_pointer].t = mesg_timeout;

      /* Get length. */
      p += i;
      if ((str[p] == '\n') || (str[p] == ' '))
         p++; /* Skip "empty char". */
      i  = gl_printWidthForText( NULL, &str[p], gui_mesg_w - 45. ); /* Theyr'e tabbed so it's shorter. */
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

   /* Must be recieving messages. */
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
   glColour *c;
   Planet *planet;
   JumpPoint *jp;

   /* no need to draw if pilot is dead */
   if (player_isFlag(PLAYER_DESTROYED) || player_isFlag(PLAYER_CREATING) ||
      (player.p == NULL) || pilot_isFlag(player.p,PILOT_DEAD))
      return;

   /* Make sure target exists. */
   if ((player.p->nav_planet < 0) && (player.p->nav_hyperspace < 0))
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
      c = faction_getColour(planet->faction);

      x = planet->pos.x - planet->radius * 1.2;
      y = planet->pos.y + planet->radius * 1.2;
      w = planet->radius * 2. * 1.2;
      h = planet->radius * 2. * 1.2;
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
void gui_renderTargetReticles( int x, int y, int w, int h, glColour* c )
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
 *    @double dt Current delta tick.
 */
static void gui_renderPilotTarget( double dt )
{
   (void) dt;
   Pilot *p;
   glColour *c;
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
      player.p->target = PLAYER_ID;
      gui_setTarget();
      return;
   }

   /* Make sure target is still in range. */
   if (!pilot_inRangePilot( player.p, p )) {
      player.p->target = PLAYER_ID;
      gui_setTarget();
      return;
   }

   /* Draw the pilot target. */
   if (pilot_isDisabled(p))
      c = &cInert;
   else if (pilot_isFlag(p,PILOT_BRIBED))
      c = &cNeutral;
   else if (pilot_isHostile(p))
      c = &cHostile;
   else if (pilot_isFriendly(p))
      c = &cFriend;
   else
      c = faction_getColour(p->faction);

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
   double z;
   int i, j;
   Pilot *plt;
   Planet *pnt;
   JumpPoint *jp;
   glTexture *tex;
   int hw, hh;
   int cw, ch;
   double rx,ry, crx,cry;
   double cx,cy;
   glColour *col;
   double int_a;
   GLfloat vertex[5*2], colours[5*4];

   /* Get zoom. */
   z = cam_getZoom();

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
      if(cur_system->planets[i]->real != ASSET_REAL)
         continue;

      pnt = cur_system->planets[i];
      tex = pnt->gfx_space;

      /* See if in sensor range. */
      if (!pilot_inRangePlanet(player.p, i))
         continue;

      /* Get relative positions. */
      rx = (pnt->pos.x - player.p->solid->pos.x)*z;
      ry = (pnt->pos.y - player.p->solid->pos.y)*z;

      /* Correct for offset. */
      crx = rx - gui_xoff;
      cry = ry - gui_yoff;

      /* Compare dimensions. */
      cw = hw + tex->sw/2;
      ch = hh + tex->sh/2;

      /* Check if out of range. */
      if ((ABS(crx) > cw) || (ABS(cry) > ch)) {

         /* Get border intersection. */
         gui_borderIntersection( &cx, &cy, rx, ry, hw, hh );

         /* Set up colours. */
         col = gui_getPlanetColour(i);
         for (j=0; j<5; j++) {
            colours[4*j + 0] = col->r;
            colours[4*j + 1] = col->g;
            colours[4*j + 2] = col->b;
            colours[4*j + 3] = 1;
         }
         gl_vboSubData( gui_vbo, gui_vboColourOffset,
               sizeof(GLfloat) * 5*4, colours );
         /* Set up vertex. */
         vertex[0] = cx-5.;
         vertex[1] = cy-5.;
         vertex[2] = cx-5.;
         vertex[3] = cy+5.;
         vertex[4] = cx+5.;
         vertex[5] = cy+5.;
         vertex[6] = cx+5.;
         vertex[7] = cy-5.;
         vertex[8] = cx-5.;
         vertex[9] = cy-5.;
         gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * 5*2, vertex );
         /* Draw tho VBO. */
         gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
         gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
               gui_vboColourOffset, 4, GL_FLOAT, 0 );
         glDrawArrays( GL_LINE_STRIP, 0, 5 );
      }
   }

   /* Draw jump routes. */
   for (i=0; i<cur_system->njumps; i++) {
      jp  = &cur_system->jumps[i];
      tex = jumppoint_gfx;

      /* See if in sensor range. */
      if (!pilot_inRangePlanet(player.p, i))
         continue;

      /* Get relative positions. */
      rx = (jp->pos.x - player.p->solid->pos.x)*z;
      ry = (jp->pos.y - player.p->solid->pos.y)*z;

      /* Correct for offset. */
      crx = rx - gui_xoff;
      cry = ry - gui_yoff;

      /* Compare dimensions. */
      cw = hw + tex->sw/2;
      ch = hh + tex->sh/2;

      /* Check if out of range. */
      if ((ABS(crx) > cw) || (ABS(cry) > ch)) {

         /* Get border intersection. */
         gui_borderIntersection( &cx, &cy, rx, ry, hw, hh );

         /* Set up colours. */
         if (i==player.p->nav_hyperspace)
            col = &cGreen;
         else
            col = &cWhite;
         for (j=0; j<4; j++) {
            colours[4*j + 0] = col->r;
            colours[4*j + 1] = col->g;
            colours[4*j + 2] = col->b;
            colours[4*j + 3] = 1;
         }
         gl_vboSubData( gui_vbo, gui_vboColourOffset,
               sizeof(GLfloat) * 4*4, colours );
         /* Set up vertex. */
         vertex[0] = cx-5.;
         vertex[1] = cy-5.;
         vertex[2] = cx+5.;
         vertex[3] = cy-5.;
         vertex[4] = cx;
         vertex[5] = cy+5.;
         vertex[6] = cx-5.;
         vertex[7] = cy-5.;
         gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * 4*2, vertex );
         /* Draw tho VBO. */
         gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
         gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
               gui_vboColourOffset, 4, GL_FLOAT, 0 );
         glDrawArrays( GL_LINE_STRIP, 0, 4 );
      }
   }

   /* Draw pilots. */
   for (i=1; i<pilot_nstack; i++) { /* skip the player */
      plt = pilot_stack[i];
      tex = plt->ship->gfx_space;

      /* See if in sensor range. */
      if (!pilot_inRangePilot(player.p, plt))
         continue;

      /* Get relative positions. */
      rx = (plt->solid->pos.x - player.p->solid->pos.x)*z;
      ry = (plt->solid->pos.y - player.p->solid->pos.y)*z;

      /* Correct for offset. */
      rx -= gui_xoff;
      ry -= gui_yoff;

      /* Compare dimensions. */
      cw = hw + tex->sw/2;
      ch = hh + tex->sh/2;

      /* Check if out of range. */
      if ((ABS(rx) > cw) || (ABS(ry) > ch)) {

         /* Get border intersection. */
         gui_borderIntersection( &cx, &cy, rx, ry, hw, hh );

         /* Set up colours. */
         col = gui_getPilotColour(plt);
         for (j=0; j<4; j++) {
            colours[4*j + 0] = col->r;
            colours[4*j + 1] = col->g;
            colours[4*j + 2] = col->b;
            colours[4*j + 3] = int_a;
         }
         gl_vboSubData( gui_vbo, gui_vboColourOffset,
               sizeof(GLfloat) * 4*4, colours );
         /* Set up vertex. */
         vertex[0] = cx-5.;
         vertex[1] = cy-5.;
         vertex[2] = cx+5.;
         vertex[3] = cy+5.;
         vertex[4] = cx+5.;
         vertex[5] = cy-5.;
         vertex[6] = cx-5.;
         vertex[7] = cy+5.;
         gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * 4*2, vertex );
         /* Draw tho VBO. */
         gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
         gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
               gui_vboColourOffset, 4, GL_FLOAT, 0 );
         glDrawArrays( GL_LINES, 0, 4 );
      }
   }

   /* Deactivate the VBO. */
   gl_vboDeactivate();
}


/**
 * @brief Renders the gui targetting reticles.
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
   StarSystem *sys;

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
   blink_pilot    -= dt;
   if (blink_pilot < 0.)
      blink_pilot += RADAR_BLINK_PILOT;
   blink_planet   -= dt;
   if (blink_planet < 0.)
      blink_planet += RADAR_BLINK_PLANET;
   if (interference_alpha > 0.)
      interference_t += dt;

   /* Render the border ships and targets. */
   gui_renderBorder(dt);

   /* Set viewport. */
   gl_viewport( 0., 0., gl_screen.rw, gl_screen.rh );

   /* Run Lua. */
   if (gui_L != NULL) {
      gui_prepFunc( "render" );
      lua_pushnumber( gui_L, dt );
      gui_runFunc( "render", 1, 0 );
   }

   /* Messages. */
   gui_renderMessages(dt);


   /* OSD. */
   osd_render();

   /*
    * hyperspace
    */
   if (pilot_isFlag(player.p, PILOT_HYPERSPACE) &&
         (player.p->ptimer < HYPERSPACE_FADEOUT)) {
      x = (HYPERSPACE_FADEOUT-player.p->ptimer) / HYPERSPACE_FADEOUT;
      col.r = 1.;
      col.g = 1.;
      col.b = 1.;
      col.a = x;
      gl_renderRect( 0., 0., SCREEN_W, SCREEN_H, &col );
   }

   /* Noise when getting near a jump. */
   if (player.p->nav_hyperspace >= 0) { /* hyperspace target */

      sys = cur_system->jumps[player.p->nav_hyperspace].target;

      /* Determine if we have to play the "enter hyperspace range" sound. */
      i = space_canHyperspace(player.p);
      if ((i != 0) && (i != can_jump))
         if (!pilot_isFlag(player.p, PILOT_HYPERSPACE))
            player_playSound(snd_jump, 1);
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

   /* Reset vieport. */
   gl_defViewport();

   /* Render messages. */
   omsg_render( dt );
}


/**
 * @brief Initializes the radar.
 *
 *    @param circle Whether or not the radar is circlular.
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

   /* The global radar. */
   radar = &gui_radar;

   gl_matrixPush();
   if (radar->shape==RADAR_RECT) {
      gl_clipRect( x, y, radar->w, radar->h );
      gl_matrixTranslate( x + radar->w/2., y + radar->h/2. );
   }
   else if (radar->shape==RADAR_CIRCLE)
      gl_matrixTranslate( x, y );

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
      if (i != player.p->nav_hyperspace)
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
   /* render the targetted pilot */
   if (j!=0)
      gui_renderPilot( pilot_stack[j], radar->shape, radar->w, radar->h, radar->res, 0 );

   /* Intereference. */
   gui_renderInterference();

   /* Render the player cross. */
   gui_renderPlayer( radar->res, 0 );

   gl_matrixPop();
   if (radar->shape==RADAR_RECT)
      gl_unclipRect();
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
   double x, y, h, hs, vx, vy;
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
   h  = conf.mesg_visible*gl_defFont.h*1.2;
   gl_renderRect( x-2., y-2., gui_mesg_w-13., h+4., &cBlackHilight );

   if (mesg_viewpoint != -1) {
      /* Set up position. */
      vx = x;
      vy = y;

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
         if (mesg_viewpoint == -1) {
            mesg_stack[m].t -= dt;

            /* Handle fading out. */
            if (mesg_stack[m].t - mesg_fade < 0.)
               c.a = mesg_stack[m].t / mesg_fade;
            else
               c.a = 1.;
         }
         else
            c.a = 1.;

         /* Only handle non-NULL messages. */
         if (mesg_stack[m].str[0] != '\0') {
            if (mesg_stack[m].str[0] == '\t')
               gl_printMaxRaw( NULL, gui_mesg_w - 45., x + 30, y, &c, &mesg_stack[m].str[1] );
            else
               gl_printMaxRaw( NULL, gui_mesg_w - 15., x, y, &c, mesg_stack[m].str );
         }
      }

      /* Increase position. */
      y += (double)gl_defFont.h*1.2;
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
 * @brief Gets the pilot colour.
 *
 *    @param p Pilot to get colour of.
 *    @return The colour of the pilot.
 */
static glColour* gui_getPilotColour( const Pilot* p )
{
   glColour *col;

   if (p->id == player.p->target) col = &cRadar_tPilot;
   else if (pilot_inRangePilot(player.p, p) == -1) col = &cMapNeutral;
   else if (pilot_isDisabled(p)) col = &cInert;
   else if (pilot_isFlag(p,PILOT_BRIBED)) col = &cNeutral;
   else if (pilot_isHostile(p)) col = &cHostile;
   else if (pilot_isFriendly(p)) col = &cFriend;
   else col = faction_getColour(p->faction);

   return col;
}


/**
 * @brief Renders a pilot in the GUI radar.
 *
 *    @param p Pilot to render.
 */
#define CHECK_PIXEL(x,y)   \
(shape==RADAR_RECT && ABS(x)<w/2. && ABS(y)<h/2.) || \
   (shape==RADAR_CIRCLE && (((x)*(x)+(y)*(y)) < rc))
void gui_renderPilot( const Pilot* p, RadarShape shape, double w, double h, double res, int overlay )
{
   int i, curs;
   int x, y, sx, sy;
   double px, py;
   glColour *col, ccol;
   GLfloat vertex[2*8], colours[4*8];
   GLfloat cx, cy;
   int rc;

   /* Make sure is in range. */
   if (!pilot_inRangePilot( player.p, p ))
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

      /* Draw little targetted symbol. */
      if (p->id == player.p->target && !overlay)
         gui_renderRadarOutOfRange( shape, w, h, x, y, &cRadar_tPilot );
      return;
   }

   if (shape==RADAR_RECT)
      rc = 0;
   else if (shape==RADAR_CIRCLE)
      rc = (int)(w*w);
   else
      return;

   /* Draw selection if targetted. */
   if (p->id == player.p->target) {
      if (blink_pilot < RADAR_BLINK_PILOT/2.) {
         /* Set up colours. */
         for (i=0; i<8; i++) {
            colours[4*i + 0] = cRadar_tPilot.r;
            colours[4*i + 1] = cRadar_tPilot.g;
            colours[4*i + 2] = cRadar_tPilot.b;
            colours[4*i + 3] = 1.-interference_alpha;
         }
         gl_vboSubData( gui_vbo, gui_vboColourOffset,
               sizeof(GLfloat) * 8*4, colours );
         /* Set up vertex. */
         curs = 0;
         cx = x-sx;
         cy = y+sy;
         if (CHECK_PIXEL(cx-3.3,cy+3.3)) {
            vertex[curs++] = cx-1.5;
            vertex[curs++] = cy+1.5;
            vertex[curs++] = cx-3.3;
            vertex[curs++] = cy+3.3;
         }
         cx = x+sx;
         if (CHECK_PIXEL(cx+3.3,cy+3.3)) {
            vertex[curs++] = cx+1.5;
            vertex[curs++] = cy+1.5;
            vertex[curs++] = cx+3.3;
            vertex[curs++] = cy+3.3;
         }
         cy = y-sy;
         if (CHECK_PIXEL(cx+3.3,cy-3.3)) {
            vertex[curs++] = cx+1.5;
            vertex[curs++] = cy-1.5;
            vertex[curs++] = cx+3.3;
            vertex[curs++] = cy-3.3;
         }
         cx = x-sx;
         if (CHECK_PIXEL(cx-3.3,cy-3.3)) {
            vertex[curs++] = cx-1.5;
            vertex[curs++] = cy-1.5;
            vertex[curs++] = cx-3.3;
            vertex[curs++] = cy-3.3;
         }
         gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * (curs/2)*2, vertex );
         /* Draw tho VBO. */
         gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
         gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
               gui_vboColourOffset, 4, GL_FLOAT, 0 );
         glDrawArrays( GL_LINES, 0, curs/2 );
      }
   }

   /* Deactivate VBO. */
   gl_vboDeactivate();

   /* Draw square. */
   px     = MAX(x-sx,-w);
   py     = MAX(y-sy, -h);
   if (pilot_isFlag(p, PILOT_HILIGHT) && (blink_pilot < RADAR_BLINK_PILOT/2.))
      col = &cRadar_hilight;
   else
      col = gui_getPilotColour(p);
   ccol.r = col->r;
   ccol.g = col->g;
   ccol.b = col->b;
   ccol.a = 1.-interference_alpha;
   gl_renderRect( px, py, MIN( 2*sx, w-px ), MIN( 2*sy, h-py ), &ccol );

   /* Draw name. */
   if (overlay && pilot_isFlag(p, PILOT_HILIGHT))
      gl_printRaw( &gl_smallFont, x+2*sx+5., y-gl_smallFont.h/2., col, p->name );
}


/**
 * @brief Renders the player cross on the radar or whatever.
 */
void gui_renderPlayer( double res, int overlay )
{
   double x, y, r;

   if (overlay) {
      x = player.p->solid->pos.x / res;
      y = player.p->solid->pos.y / res;
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
      gl_printRaw( &gl_smallFont, x+r+5., y-gl_smallFont.h/2., &cRadar_player, "You" );
}


/**
 * @brief Gets the colour of a planet.
 *
 *    @param i Index of the planet to get colour of.
 *    @return Colour of the planet.
 */
static glColour *gui_getPlanetColour( int i )
{
   glColour *col;
   Planet *planet;

   planet = cur_system->planets[i];

   col = faction_getColour(planet->faction);
   if (i == player.p->nav_planet)
      col = &cRadar_tPlanet;
   else if ((col != &cHostile) && !planet_hasService(planet,PLANET_SERVICE_INHABITED))
      col = &cInert; /* Override non-hostile planets without service. */

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
   GLfloat vx, vy;
   GLfloat vertex[8*2], colours[8*4];
   int i, curs;

   if (blink_planet < RADAR_BLINK_PLANET/2.) {
      curs = 0;
      vx = cx-vr;
      vy = cy+vr;
      if (CHECK_PIXEL(vx-3.3, vy+3.3)) {
         vertex[curs++] = vx-1.5;
         vertex[curs++] = vy+1.5;
         vertex[curs++] = vx-3.3;
         vertex[curs++] = vy+3.3;
      }
      vx = cx+vr;
      if (CHECK_PIXEL(vx+3.3, vy+3.3)) {
         vertex[curs++] = vx+1.5;
         vertex[curs++] = vy+1.5;
         vertex[curs++] = vx+3.3;
         vertex[curs++] = vy+3.3;
      }
      vy = cy-vr;
      if (CHECK_PIXEL(vx+3.3, vy-3.3)) {
         vertex[curs++] = vx+1.5;
         vertex[curs++] = vy-1.5;
         vertex[curs++] = vx+3.3;
         vertex[curs++] = vy-3.3;
      }
      vx = cx-vr;
      if (CHECK_PIXEL(vx-3.3, vy-3.3)) {
         vertex[curs++] = vx-1.5;
         vertex[curs++] = vy-1.5;
         vertex[curs++] = vx-3.3;
         vertex[curs++] = vy-3.3;
      }
      gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * (curs/2)*2, vertex );
      /* Set the colours. */
      for (i=0; i<curs/2; i++) {
         colours[4*i + 0] = cRadar_tPlanet.r;
         colours[4*i + 1] = cRadar_tPlanet.g;
         colours[4*i + 2] = cRadar_tPlanet.b;
         colours[4*i + 3] = 1.-interference_alpha;
      }
      gl_vboSubData( gui_vbo, gui_vboColourOffset,
            sizeof(GLfloat) * (curs/2)*4, colours );
      /* Draw tho VBO. */
      gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
      gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
            gui_vboColourOffset, 4, GL_FLOAT, 0 );
      glDrawArrays( GL_LINES, 0, curs/2 );
   }

   /* Deactivate the VBO. */
   gl_vboDeactivate();
}


/**
 * @brief Renders an out of range marker for the planet.
 */
static void gui_renderRadarOutOfRange( RadarShape sh, int w, int h, int cx, int cy, glColour *col )
{
   GLfloat vertex[2*2], colours[8*2];
   double a;
   int i;

   /* Set the colour. */
   for (i=0; i<2; i++) {
      colours[4*i + 0] = col->r;
      colours[4*i + 1] = col->g;
      colours[4*i + 2] = col->b;
      colours[4*i + 3] = 1.-interference_alpha;
   }
   gl_vboSubData( gui_vbo, gui_vboColourOffset,
         sizeof(GLfloat) * 2*4, colours );

   /* Draw a line like for pilots. */
   if (sh == RADAR_CIRCLE) {
      a         = ANGLE(cx,cy);
      vertex[0] = w*cos(a);
      vertex[1] = w*sin(a);
      vertex[2] = 0.85*vertex[0];
      vertex[3] = 0.85*vertex[1];
   }
   else {
      a = ANGLE(cx,cy);
      int cxa, cya;
      cxa = ABS(cx);
      cya = ABS(cy);
      /* Determine position. */
      if (cy >= cxa) { /* Bottom */
         vertex[0] = w/2. * (cx*1./cy);
         vertex[1] = h/2.;
      } else if (cx >= cya) { /* Left */
         vertex[0] = w/2.;
         vertex[1] = h/2. * (cy*1./cx);
      } else if (cya >= cxa) { /* Top */
         vertex[0] = -w/2. * (cx*1./cy);
         vertex[1] = -h/2.;
      } else { /* Right */
         vertex[0] = -w/2.;
         vertex[1] = -h/2. * (cy*1./cx);
      }
      /* Calculate rest. */
      vertex[2] = vertex[0] - 0.15*w*cos(a);
      vertex[3] = vertex[1] - 0.15*w*sin(a);
   }
   /* Set the vertex. */
   gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * 2*2, vertex );
   /* Draw tho VBO. */
   gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
         gui_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_LINES, 0, 2 );

   /* Deactivate the VBO. */
   gl_vboDeactivate();
}


/**
 * @brief Draws the planets in the minimap.
 *
 * Matrix mode is already displaced to center of the minimap.
 */
void gui_renderPlanet( int ind, RadarShape shape, double w, double h, double res, int overlay )
{
   int i;
   int x, y;
   int cx, cy, r, rc;
   GLfloat vx, vy, vr;
   GLfloat a;
   glColour *col;
   Planet *planet;
   GLfloat vertex[5*2], colours[5*4];

   /* Make sure is in range. */
   if (!pilot_inRangePlanet( player.p, ind ))
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

   /* Do the blink. */
   if (ind == player.p->nav_planet)
      gui_planetBlink( w, h, rc, cx, cy, vr, shape );

   /* Get the colour. */
   col = gui_getPlanetColour(ind);
   if (overlay)
      a = 1.;
   else
      a   = 1.-interference_alpha;
   for (i=0; i<5; i++) {
      colours[4*i + 0] = col->r;
      colours[4*i + 1] = col->g;
      colours[4*i + 2] = col->b;
      colours[4*i + 3] = a;
   }
   gl_vboSubData( gui_vbo, gui_vboColourOffset,
      sizeof(GLfloat) * 5*4, colours );
   /* Now load the data. */
   vx = cx;
   vy = cy;
   vertex[0] = vx;
   vertex[1] = vy + vr;
   vertex[2] = vx + vr;
   vertex[3] = vy;
   vertex[4] = vx;
   vertex[5] = vy - vr;
   vertex[6] = vx - vr;
   vertex[7] = vy;
   vertex[8] = vertex[0];
   vertex[9] = vertex[1];
   gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * 5*2, vertex );
   /* Draw tho VBO. */
   gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
         gui_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 5 );

   /* Deactivate the VBO. */
   gl_vboDeactivate();

   /* Render name. */
   if (overlay)
      gl_printRaw( &gl_smallFont, cx+vr+5., cy, col, planet->name );
}


/**
 * @brief Renders a jump point on the minimap.
 *
 *    @param i Jump point to render.
 */
void gui_renderJumpPoint( int ind, RadarShape shape, double w, double h, double res, int overlay )
{
   int i;
   int cx, cy, x, y, r, rc;
   GLfloat a;
   GLfloat ca, sa;
   GLfloat vx, vy, vr;
   glColour *col;
   GLfloat vertex[4*2], colours[4*4];
   JumpPoint *jp;

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

   /* Check if in range. */
   if (shape == RADAR_RECT) {
      x = y = 0;
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

   /* Do the blink. */
   if (ind == player.p->nav_hyperspace) {
      gui_planetBlink( w, h, rc, cx, cy, vr, shape );
      col = &cGreen;
   }
   else
      col = &cWhite;
   if (overlay)
      a = 1.;
   else
      a = 1.-interference_alpha;

   /* Get the colour. */
   for (i=0; i<4; i++) {
      colours[4*i + 0] = col->r;
      colours[4*i + 1] = col->g;
      colours[4*i + 2] = col->b;
      colours[4*i + 3] = a;
   }
   gl_vboSubData( gui_vbo, gui_vboColourOffset,
      sizeof(GLfloat) * 5*4, colours );
   /* Now load the data. */
   vx = cx;
   vy = cy;
   ca = jp->cosa;
   sa = jp->sina;
   /* Must rotate around triangle center which with our calculations is shifted vr/3 to the left. */
   vertex[0] = vx + (4./3.*vr)*ca;
   vertex[1] = vy - (4./3.*vr)*sa;
   vertex[2] = vx - (2./3.*vr)*ca + vr*sa;
   vertex[3] = vy + (2./3.*vr)*sa + vr*ca;
   vertex[4] = vx - (2./3.*vr)*ca - vr*sa;
   vertex[5] = vy + (2./3.*vr)*sa - vr*ca;
   vertex[6] = vertex[0];
   vertex[7] = vertex[1];
   gl_vboSubData( gui_vbo, 0, sizeof(GLfloat) * 4*2, vertex );
   /* Draw tho VBO. */
   gl_vboActivateOffset( gui_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( gui_vbo, GL_COLOR_ARRAY,
         gui_vboColourOffset, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_LINE_STRIP, 0, 4 );

   /* Deactivate the VBO. */
   gl_vboDeactivate();

   /* Render name. */
   if (overlay)
      gl_printRaw( &gl_smallFont, cx+vr+5., cy, col, sys_isKnown(jp->target) ? jp->target->name : "Unknown" );
}
#undef CHECK_PIXEL


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
   gl_setDefViewport( 0., 0., gl_screen.rw, gl_screen.rh );
   gl_defViewport();
}


/**
 * @brief Calculates and sets the GUI borders.
 */
static void gui_calcBorders (void)
{
   double w,h;
   double vx,vy, vw,vh;

   /* Precalculations. */
   vx = gui_viewport_x;
   vy = gui_viewport_y;
   vw = SCREEN_W - (vx + gui_viewport_w);
   vh = SCREEN_H - (vy + gui_viewport_h);
   w  = SCREEN_W/2.;
   h  = SCREEN_H/2.;

   /*
    * Borders.
    */
   gui_tl = atan2( +SCREEN_H/2., -SCREEN_W/2. );
   if (gui_tl < 0.)
      gui_tl += 2*M_PI;
   gui_tr = atan2( +SCREEN_H/2., +SCREEN_W/2. );
   if (gui_tr < 0.)
      gui_tr += 2*M_PI;
   gui_bl = atan2( -SCREEN_H/2., -SCREEN_W/2. );
   if (gui_bl < 0.)
      gui_bl += 2*M_PI;
   gui_br = atan2( -SCREEN_H/2., +SCREEN_W/2. );
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
         ERR("Out of memory!");
         return -1;
      }
   }

   /*
    * VBO.
    */
   if (gui_vbo == NULL) {
      gui_vbo = gl_vboCreateStream( sizeof(GLfloat) * 8*(2+4), NULL );
      gui_vboColourOffset = sizeof(GLfloat) * 8*2;
   }

   /*
    * OSD
    */
   osd_setup( 30., SCREEN_H-90., 150., 300. );

   /*
    * Set viewport.
    */
   gui_setViewport( 0., 0., gl_screen.rw, gl_screen.rh );

   /*
    * Icons.
    */
   gui_ico_hail = gl_newSprite( "gfx/gui/hail.png", 5, 2, 0 );

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
   lua_State *L;

   /* For comfort. */
   L = gui_L;

#ifdef DEBUGGING
   if (L == NULL) {
      WARN( "Trying to run GUI func '%s' but no GUI is loaded!", func );
      return -1;
   }
#endif /* DEBUGGING */

   /* Set up function. */
   lua_getglobal( L, func );
   return 0;
}


/**
 * @brief Runs a function.
 */
static int gui_runFunc( const char* func, int nargs, int nret )
{
   int ret;
   const char* err;
   lua_State *L;

   /* For comfort. */
   L = gui_L;

   /* Run the function. */
   ret = lua_pcall( L, nargs, nret, 0 );
   if (ret != 0) { /* error has occured */
      err = (lua_isstring(L,-1)) ? lua_tostring(L,-1) : NULL;
      WARN("GUI Lua -> '%s': %s",
            func, (err) ? err : "unknown error");
      lua_pop(L,1);
      return -1;
   }

   return 0;
}


/**
 * @brief Player just changed his cargo.
 */
void gui_setCargo (void)
{
   if (gui_L != NULL)
      gui_doFunc( "update_cargo" );
}


/**
 * @brief Player just changed his nav computer target.
 */
void gui_setNav (void)
{
   if (gui_L != NULL)
      gui_doFunc( "update_nav" );
}


/**
 * @brief Player just changed his pilot target.
 */
void gui_setTarget (void)
{
   if (gui_L != NULL)
      gui_doFunc( "update_target" );
}


/**
 * @brief Player just upgraded his ship or modified it.
 */
void gui_setShip (void)
{
   if (gui_L != NULL)
      gui_doFunc( "update_ship" );
}


/**
 * @brief Player just changed his system.
 */
void gui_setSystem (void)
{
   if (gui_L != NULL)
      gui_doFunc( "update_system" );
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
   uint32_t bufsize;

   /* Set defaults. */
   gui_cleanup();

   /* Open file. */
   snprintf( path, sizeof(path), "dat/gui/%s.lua", name );
   buf = ndata_read( path, &bufsize );
   if (buf == NULL) {
      WARN("Unable to find GUI '%s'.", path );
      return -1;
   }

   /* Clean up. */
   if (gui_L != NULL) {
      lua_close( gui_L );
      gui_L = NULL;
   }

   /* Create Lua state. */
   gui_L = nlua_newState();
   if (luaL_dobuffer( gui_L, buf, bufsize, path ) != 0) {
      WARN("Failed to load GUI Lua: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check",
            path, lua_tostring(gui_L,-1));
      lua_close( gui_L );
      gui_L = NULL;
      free(buf);
      return -1;
   }
   free(buf);
   nlua_loadStandard( gui_L, 1 );
   nlua_loadGFX( gui_L, 0 );
   nlua_loadGUI( gui_L, 0 );

   /* Run create function. */
   if (gui_doFunc( "create" )) {
      lua_close( gui_L );
      gui_L = NULL;
   }

   /* Recreate land window if landed. */
   if (landed)
      land_genWindows( 0, 1 );

   return 0;
}


/**
 * @brief Creates teh interference map for the current gui.
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
      WARN("Radar shape is invalid.");
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
   if (gui_L != NULL) {
      lua_close( gui_L );
      gui_L = NULL;
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

   /* Free VBO. */
   if (gui_vbo != NULL) {
      gl_vboDestroy( gui_vbo );
      gui_vbo = NULL;
   }

   /* Clean up the osd. */
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

   player_message( "\epRadar set to %dx.", (int)gui_radar.res );
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

   ret = 0;
   switch (evt->type) {
      /* Mouse motion. */
      case SDL_MOUSEMOTION:
         if (!gui_L_mmove)
            break;
         gui_prepFunc( "mouse_move" );
         gl_windowToScreenPos( &x, &y, evt->motion.x, evt->motion.y );
         lua_pushnumber( gui_L, x );
         lua_pushnumber( gui_L, y );
         gui_runFunc( "mouse_move", 2, 0 );
         break;

      /* Mouse click. */
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
         if (!gui_L_mclick)
            break;
         gui_prepFunc( "mouse_click" );
         lua_pushnumber( gui_L, evt->button.button+1 );
         gl_windowToScreenPos( &x, &y, evt->button.x, evt->button.y );
         lua_pushnumber( gui_L, x );
         lua_pushnumber( gui_L, y );
         lua_pushboolean( gui_L, (evt->type==SDL_MOUSEBUTTONDOWN) );
         gui_runFunc( "mouse_click", 4, 1 );
         ret = lua_toboolean( gui_L, -1 );
         lua_pop( gui_L, 1 );
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



