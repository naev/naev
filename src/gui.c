/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file gui.c
 *
 * @brief Contains the GUI stuff for the player.
 */
/** @cond */
#include <stdlib.h>

#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "gui.h"

#include "ai.h"
#include "array.h"
#include "camera.h"
#include "conf.h"
#include "constants.h"
#include "font.h"
#include "gui_omsg.h"
#include "gui_osd.h"
#include "input.h"
#include "land.h"
#include "log.h"
#include "map_overlay.h"
#include "menu.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_gfx.h"
#include "nlua_gui.h"
#include "nlua_tk.h"
#include "ntracing.h"
#include "opengl.h"
#include "pause.h"
#include "pilot.h"
#include "player.h"
#include "player_gui.h"
#include "render.h"
#include "space.h"
#include "start.h"
#include "toolkit.h"

#define XML_GUI_ID "GUIs" /**< XML section identifier for GUI document. */
#define XML_GUI_TAG "gui" /**<  XML Section identifier for GUI tags. */

#define RADAR_BLINK_PILOT 0.5 /**< Blink rate of the pilot target on radar. */
#define RADAR_BLINK_SPOB 1.   /**< Blink rate of the spob target on radar. */

/* some blinking stuff. */
static double blink_pilot  = 0.; /**< Timer on target blinking on radar. */
static double blink_spob   = 0.; /**< Timer on spob blinking on radar. */
static double animation_dt = 0.; /**< Used for animations. */

/* for VBO. */
static gl_vbo *gui_radar_select_vbo = NULL;

static int gui_getMessage =
   1; /**< Whether or not the player should receive messages. */
static char   *gui_name = NULL; /**< Name of the GUI (for errors and such). */
static IntList gui_qtquery;     /**< For querying collisions. */

extern unsigned int land_wid; /**< From land.c */

/**
 * GUI Lua stuff.
 */
static nlua_env *gui_env      = NULL; /**< Current GUI Lua environment. */
static int       gui_L_mclick = 0;    /**< Use mouse click callback. */
static int       gui_L_mmove  = 0;    /**< Use mouse movement callback. */

/**
 * @struct Radar
 *
 * @brief Represents the player's radar.
 */
typedef struct Radar_ {
   double     w;     /**< Width. */
   double     h;     /**< Height. */
   double     x;     /**< X position. */
   double     y;     /**< Y position. */
   RadarShape shape; /**< Shape */
   double     res;   /**< Resolution */
} Radar;
/* radar resolutions */
#define RADAR_RES_MAX 300. /**< Maximum radar resolution. */
#define RADAR_RES_REF 100. /**< Reference radar resolution. */
#define RADAR_RES_MIN 10.  /**< Minimum radar resolution. */
#define RADAR_RES_INTERVAL                                                     \
   10. /**< Steps used to increase/decrease resolution. */
static Radar gui_radar;

/* messages */
static const int mesg_max = 128; /**< Maximum messages onscreen */
static int       mesg_pointer =
   0; /**< Current pointer message is at (for when scrolling). */
static int          mesg_viewpoint = -1;  /**< Position of viewing. */
static const double mesg_timeout   = 15.; /**< Timeout length. */
/**
 * @struct Mesg
 *
 * @brief On screen player message.
 */
typedef struct Mesg_ {
   int           dup;     /**< Times duplicated. */
   char         *str;     /**< The message (allocated). */
   char         *dstr;    /**< The display message. */
   double        t;       /**< Time to live for the message. */
   glFontRestore restore; /**< Hack for font restoration. */
} Mesg;
static Mesg *mesg_stack =
   NULL;                   /**< Stack of messages, will be of mesg_max size. */
static int gui_mesg_w = 0; /**< Width of messages. */
static int gui_mesg_x = 0; /**< X positioning of messages. */
static int gui_mesg_y = 0; /**< Y positioning of messages. */

/* Intrinsic graphical stuff. */
static glTexture *gui_ico_hail     = NULL; /**< Hailing icon. */
static glTexture *gui_target_spob  = NULL; /**< Spob targeting icon. */
static glTexture *gui_target_pilot = NULL; /**< Pilot targeting icon. */

/* Lua Stuff. */
static int gui_lua_create          = LUA_NOREF;
static int gui_lua_render          = LUA_NOREF;
static int gui_lua_render_cooldown = LUA_NOREF;
static int gui_lua_cooldown_end    = LUA_NOREF;
static int gui_lua_mouse_move      = LUA_NOREF;
static int gui_lua_mouse_click     = LUA_NOREF;
static int gui_lua_update_cargo    = LUA_NOREF;
static int gui_lua_update_nav      = LUA_NOREF;
static int gui_lua_update_target   = LUA_NOREF;
static int gui_lua_update_ship     = LUA_NOREF;
static int gui_lua_update_system   = LUA_NOREF;
static int gui_lua_update_faction  = LUA_NOREF;
static int gui_lua_update_effects  = LUA_NOREF;

/*
 * prototypes
 */
/*
 * external
 */
extern void weapon_minimap( const double res, const double w, const double h,
                            const RadarShape shape,
                            double           alpha ); /**< from weapon.c */
/*
 * internal
 */
/* gui */
static void gui_renderTargetReticles( const SimpleShader *shd, double x,
                                      double y, double radius, double angle,
                                      const glColour *c );
/* Render GUI. */
static void            gui_renderPilotTarget( void );
static void            gui_renderSpobTarget( void );
static void            gui_renderMessages( double dt );
static const glColour *gui_getSpobColour( int i );
static void gui_renderRadarOutOfRange( RadarShape sh, int w, int h, int cx,
                                       int cy, const glColour *col );
static void gui_blink( double cx, double cy, double vr, const glColour *col,
                       double blinkInterval, double blinkVar );
static const glColour *gui_getPilotColour( const Pilot *p );
/* Lua GUI. */
static int gui_doFunc( int func_ref, const char *func_name );
static int gui_prepFunc( int func_ref, const char *func_name );
static int gui_runFunc( const char *func, int nargs, int nret );

static double radar_linrange =
   40.; /**< Distance (in pixels) at which it becomes linear. */
static double radar_logscale = 40.; /**< Factor used to make a smooth transition
                                       from linear to logarithmic. */
static void logradar( double *x, double *y, double res )
{
   double m = hypotf( *x, *y ) / res;
   if ( m > radar_linrange ) {
      m        = log( m ) * radar_logscale;
      double a = atan2( *y, *x );
      *x       = m * cos( a );
      *y       = m * sin( a );
   } else {
      *x /= res;
      *y /= res;
   }
}

static double unlogradar( double *x, double *y, double res )
{
   double m = hypotf( *x, *y );
   if ( m > radar_linrange ) {
      double m2 = res * exp( m / radar_logscale );
      double a  = atan2( *y, *x );
      *x        = m2 * cos( a );
      *y        = m2 * sin( a );
      // Area of "1" pixel near the location
      return m2 - res * exp( ( m - 1. ) / radar_logscale );
   } else {
      *x *= res;
      *y *= res;
      return res;
   }
}

/**
 * Sets the GUI to defaults.
 */
void gui_setDefaults( void )
{
   gui_setRadarResolution( player.radar_res );
   gui_clearMessages();
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
   if ( mesg_viewpoint == -1 ) {
      mesg_viewpoint = mesg_pointer;
      return;
   }

   /* Get offset. */
   o = mesg_pointer - mesg_viewpoint;
   if ( o < 0 )
      o += mesg_max;
   o = mesg_max - 2 * conf.mesg_visible - o;

   /* Calculate max line movement. */
   if ( lines > o )
      lines = o;

   /* Move viewpoint. */
   const int new_point = ( mesg_max + mesg_viewpoint - lines ) % mesg_max;
   if ( mesg_stack && mesg_stack[new_point].str )
      mesg_viewpoint = new_point;
}

/**
 * @brief Scrolls down the message box.
 *
 *    @param lines Number of lines to scroll down.
 */
void gui_messageScrollDown( int lines )
{
   int o;

   /* Handle hacks. */
   if ( mesg_viewpoint == mesg_pointer ) {
      mesg_viewpoint = -1;
      return;
   } else if ( mesg_viewpoint == -1 )
      return;

   /* Get offset. */
   o = mesg_pointer - mesg_viewpoint;
   if ( o < 0 )
      o += mesg_max;

   /* Calculate max line movement. */
   if ( lines > o )
      lines = o;

   /* Move viewpoint. */
   mesg_viewpoint = ( mesg_viewpoint + lines ) % mesg_max;
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
   glPrintLineIterator iter;

   /* Must be receiving messages. */
   if ( !gui_getMessage )
      return;

   gl_printLineIteratorInit( &iter, &gl_smallFont, str,
                             gui_mesg_w - ( ( str[0] == '\t' ) ? 45 : 15 ) );
   while ( gl_printLineIteratorNext( &iter ) ) {
      Mesg *m = &mesg_stack[mesg_pointer];

      /* See if same message. */
      if ( ( m->t > 0. ) && strcmp( mesg_stack[mesg_pointer].str, str ) == 0 ) {
         free( m->dstr );
         m->dup++;
         SDL_asprintf( &m->dstr, p_( "player_message", "%s x%d" ), str,
                       m->dup );
         if ( gl_printWidthRaw( &gl_smallFont, m->dstr ) <=
              gui_mesg_w - ( ( str[0] == '\t' ) ? 45 : 15 ) ) {
            m->t = mesg_timeout; /* Reset time out. */
            return;
         }
         free( m->dstr );
         m->dstr = NULL;
      }

      /* Move pointer. */
      mesg_pointer = ( mesg_pointer + 1 ) % mesg_max;
      if ( mesg_viewpoint != -1 )
         mesg_viewpoint++;
      m = &mesg_stack[mesg_pointer];

      /* Add the new one */
      free( m->str );
      free( m->dstr );
      m->dstr = NULL;
      if ( iter.l_begin == 0 ) {
         m->str = SDL_strndup( &str[iter.l_begin], iter.l_end - iter.l_begin );
         gl_printRestoreInit( &m->restore );
      } else {
         m->str = malloc( iter.l_end - iter.l_begin + 2 );
         snprintf( m->str, iter.l_end - iter.l_begin + 2, "\t%s",
                   &str[iter.l_begin] );
         gl_printStoreMax( &m->restore, str, iter.l_begin );
      }
      m->t   = mesg_timeout;
      m->dup = 1;

      iter.width =
         gui_mesg_w - 45; /* Remaining lines are tabbed so it's shorter. */
   }
}

/**
 * @brief Adds a mesg to the queue to be displayed on screen.
 *
 *    @param fmt String with formatting like `printf`.
 */
void player_message( const char *fmt, ... )
{
   va_list ap;
   char   *buf;

   /* Must be receiving messages. */
   if ( !gui_getMessage )
      return;

   /* Add the new one */
   va_start( ap, fmt );
   /* Requires SDL2 >=2.0.18 */
   SDL_vasprintf( &buf, fmt, ap );
   va_end( ap );
   player_messageRaw( buf );
   free( buf );
}

/**
 * @brief Sets up rendering of spob and jump point targeting reticles.
 */
static void gui_renderSpobTarget( void )
{
   double          x, y, r;
   const glColour *c;

   /* no need to draw if pilot is dead */
   if ( player_isFlag( PLAYER_DESTROYED ) || player_isFlag( PLAYER_CREATING ) ||
        ( player.p == NULL ) || pilot_isFlag( player.p, PILOT_DEAD ) )
      return;

   /* Make sure target exists. */
   if ( ( player.p->nav_spob < 0 ) && ( player.p->nav_hyperspace < 0 ) &&
        ( player.p->nav_asteroid < 0 ) )
      return;

   /* Make sure targets are still in range. */
#if 0
   if (!pilot_inRangeSpob( player.p, player.p->nav_spob )) {
      player_targetSpobSet( -1 );
      return;
   }
#endif

   /* Draw spob and jump point target graphics. */
   if ( player.p->nav_hyperspace >= 0 ) {
      const JumpPoint *jp = &cur_system->jumps[player.p->nav_hyperspace];
      if ( jp_isKnown( jp ) ) {
         c = &cGreen;
         x = jp->pos.x;
         y = jp->pos.y;
         r = tex_sw( jumppoint_gfx ) * 0.5;
         gui_renderTargetReticles( &shaders.targetspob, x, y, r, 0., c );
      }
   }
   if ( player.p->nav_spob >= 0 ) {
      const Spob *spob = cur_system->spobs[player.p->nav_spob];
      c                = spob_getColour( spob );
      x                = spob->pos.x;
      y                = spob->pos.y;
      r                = spob->radius;
      gui_renderTargetReticles( &shaders.targetspob, x, y, r, 0., c );
   }
   if ( player.p->nav_asteroid >= 0 ) {
      const AsteroidAnchor *field =
         &cur_system->asteroids[player.p->nav_anchor];
      const Asteroid *ast = &field->asteroids[player.p->nav_asteroid];
      c                   = &cWhite;

      x = ast->sol.pos.x;
      y = ast->sol.pos.y;
      r = tex_sw( ast->gfx ) * 0.5;
      gui_renderTargetReticles( &shaders.targetship, x, y, r, 0., c );
   }
}

/**
 * @brief Renders spob and jump point targeting reticles.
 *
 *    @param shd Shader to use to render.
 *    @param x X position of reticle segment.
 *    @param y Y position of reticle segment.
 *    @param radius Radius.
 *    @param angle Angle to rotate.
 *    @param c Colour.
 */
static void gui_renderTargetReticles( const SimpleShader *shd, double x,
                                      double y, double radius, double angle,
                                      const glColour *c )
{
   double rx, ry, r;
   /* Must not be NULL. */
   if ( gui_target_spob == NULL )
      return;

   gl_gameToScreenCoords( &rx, &ry, x, y );
   r = (double)radius * 1.2 * cam_getZoom();

   glUseProgram( shd->program );
   glUniform1f( shd->dt, animation_dt );
   glUniform1f( shd->paramf, radius );
   gl_renderShader( rx, ry, r, r, angle, shd, c, 1 );
}

/**
 * @brief Renders the players pilot target.
 */
static void gui_renderPilotTarget( void )
{
   Pilot          *p;
   const glColour *c;

   /* Player is most likely dead. */
   if ( gui_target_pilot == NULL )
      return;

   /* Get the target. */
   if ( player.p->target != PLAYER_ID )
      p = pilot_get( player.p->target );
   else
      p = NULL;

   /* Make sure pilot exists and is still alive. */
   if ( ( p == NULL ) || pilot_isFlag( p, PILOT_DEAD ) ) {
      pilot_setTarget( player.p, player.p->id );
      gui_setTarget();
      return;
   }

   /* Make sure target is still valid and in range. */
   if ( !pilot_validTarget( player.p, p ) ) {
      pilot_setTarget( player.p, player.p->id );
      gui_setTarget();
      return;
   }

   /* Draw the pilot target. */
   if ( pilot_isDisabled( p ) )
      c = &cInert;
   else if ( pilot_isHostile( p ) )
      c = &cHostile;
   else if ( pilot_isFriendly( p ) )
      c = &cFriend;
   else
      c = &cNeutral;

   gui_renderTargetReticles( &shaders.targetship, p->solid.pos.x,
                             p->solid.pos.y, p->ship->size * 0.5, p->solid.dir,
                             c );
}

/**
 * @brief Takes a pilot and returns whether it's on screen, plus its relative
 * position.
 *
 * @param[out] rx Relative X position.
 * @param[out] ry Relative Y position.
 * @param pilot Pilot to determine the visibility and position of
 * @return Whether or not the pilot is on-screen.
 */
int gui_onScreenPilot( double *rx, double *ry, const Pilot *pilot )
{
   double z;
   int    cw, ch, w, h;

   z = cam_getZoom();

   /* Get relative positions. */
   *rx = ( pilot->solid.pos.x - player.p->solid.pos.x ) * z;
   *ry = ( pilot->solid.pos.y - player.p->solid.pos.y ) * z;

   /* Get size. */
   w = pilot->ship->size;
   h = pilot->ship->size;

   /* Compare dimensions. */
   cw = SCREEN_W / 2 + w / 2;
   ch = SCREEN_H / 2 + h / 2;

   if ( ( ABS( *rx ) > cw ) || ( ABS( *ry ) > ch ) )
      return 0;

   return 1;
}

/**
 * @brief Takes a spob or jump point and returns whether it's on screen, plus
 * its relative position.
 *
 * @param[out] rx Relative X position.
 * @param[out] ry Relative Y position.
 * @param jp Jump point to determine the visibility and position of
 * @param pnt Spob to determine the visibility and position of
 * @return Whether or not the given spob is on-screen.
 */
int gui_onScreenSpob( double *rx, double *ry, const JumpPoint *jp,
                      const Spob *pnt )
{
   double     z;
   int        cw, ch;
   glTexture *tex;

   z = cam_getZoom();

   if ( jp == NULL ) {
      tex = pnt->gfx_space;
      *rx = ( pnt->pos.x - player.p->solid.pos.x ) * z;
      *ry = ( pnt->pos.y - player.p->solid.pos.y ) * z;
   } else {
      tex = jumppoint_gfx;
      *rx = ( jp->pos.x - player.p->solid.pos.x ) * z;
      *ry = ( jp->pos.y - player.p->solid.pos.y ) * z;
   }

   /* Compare dimensions. */
   cw = SCREEN_W / 2;
   ch = SCREEN_H / 2;
   if ( tex != NULL ) {
      cw += tex_sw( tex ) / 2;
      ch += tex_sh( tex ) / 2;
   }

   if ( ( ABS( *rx ) > cw ) || ( ABS( *ry ) > ch ) )
      return 0;

   return 1;
}

/**
 * @brief Renders the GUI targeting reticles.
 *
 * @param dt Current delta time.
 */
void gui_renderReticles( double dt )
{
   (void)dt;

   /* Player must be alive. */
   if ( player.p == NULL )
      return;

   /* Disable in cinematics. */
   if ( player_isFlag( PLAYER_CINEMATICS ) )
      return;

   gui_renderSpobTarget();
   gui_renderPilotTarget();
}

static int can_jump =
   0; /**< Stores whether or not the player is able to jump. */
/**
 * @brief Renders the player's GUI.
 *
 *    @param dt Current delta tick.
 */
void gui_render( double dt )
{
   double fade, direction;

   /* If player is dead just render the cinematic mode. */
   if ( !menu_isOpen( MENU_MAIN ) &&
        ( player_isFlag( PLAYER_DESTROYED ) ||
          player_isFlag( PLAYER_CREATING ) ||
          ( ( player.p != NULL ) && pilot_isFlag( player.p, PILOT_DEAD ) ) ) ) {
      NTracingZone( _ctx, 1 );

      // spfx_cinematic();
      gl_defViewport();

      NTracingZoneEnd( _ctx );
      return;
   }

   /* Make sure player is valid. */
   if ( player.p == NULL )
      return;

   /* Cinematics mode. */
   if ( player_isFlag( PLAYER_CINEMATICS_GUI ) )
      return;

   NTracingZone( _ctx, 1 );
   gl_debugGroupStart();

   /*
    * Countdown timers.
    */
   animation_dt += dt / dt_mod;
   blink_pilot -= dt / dt_mod;
   if ( blink_pilot < 0. )
      blink_pilot += RADAR_BLINK_PILOT;
   blink_spob -= dt / dt_mod;
   if ( blink_spob < 0. )
      blink_spob += RADAR_BLINK_SPOB;

   glClear( GL_DEPTH_BUFFER_BIT );

   /* Run Lua. */
   if ( gui_env != NULL ) {
      if ( gui_prepFunc( gui_lua_render, "render" ) == 0 ) {
         lua_pushnumber( naevL, dt );
         lua_pushnumber( naevL, dt_mod );
         gui_runFunc( "render", 2, 0 );
      }
      if ( pilot_isFlag( player.p, PILOT_COOLDOWN ) ) {
         if ( gui_prepFunc( gui_lua_render_cooldown, "render_cooldown" ) ==
              0 ) {
            lua_pushnumber( naevL, player.p->ctimer / player.p->cdelay );
            lua_pushnumber( naevL, player.p->ctimer );
            gui_runFunc( "render_cooldown", 2, 0 );
         }
      }
   }

   /* Messages. */
   gui_renderMessages( dt );

   /* OSD. */
   osd_render();

   /* Noise when getting near a jump. */
   if ( player.p->nav_hyperspace >= 0 ) { /* hyperspace target */
      /* Determine if we have to play the "enter hyperspace range" sound. */
      int i = space_canHyperspace( player.p );
      if ( ( i != 0 ) && ( i != can_jump ) )
         if ( !pilot_isFlag( player.p, PILOT_HYPERSPACE ) )
            player_soundPlayGUI( snd_jump, 1 );
      can_jump = i;
   }

   /* Determine if we need to fade in/out. */
   fade = direction = 0.;
   if ( pilot_isFlag( player.p, PILOT_HYPERSPACE ) &&
        ( player.p->ptimer < HYPERSPACE_FADEOUT ) ) {
      fade = ( HYPERSPACE_FADEOUT - player.p->ptimer ) / HYPERSPACE_FADEOUT;
      direction = VANGLE( player.p->solid.vel );
   } else if ( pilot_isFlag( player.p, PILOT_HYP_END ) &&
               player.p->ptimer > 0. ) {
      fade      = player.p->ptimer / HYPERSPACE_FADEIN;
      direction = VANGLE( player.p->solid.vel ) + M_PI;
   }
   /* Perform the fade. */
   if ( fade > 0. ) {
      mat4 projection = gl_view_matrix;

      /* Set up the program. */
      glUseProgram( shaders.jump.program );
      glEnableVertexAttribArray( shaders.jump.vertex );
      gl_vboActivateAttribOffset( gl_squareVBO, shaders.jump.vertex, 0, 2,
                                  GL_FLOAT, 0 );

      /* Set up the projection. */
      mat4_scale( &projection, gl_screen.nw, gl_screen.nh, 1. );

      /* Pass stuff over. */
      gl_uniformMat4( shaders.jump.projection, &projection );
      glUniform1f( shaders.jump.progress, fade );
      glUniform1f( shaders.jump.direction, direction );
      glUniform2f( shaders.jump.dimensions, gl_screen.nw, gl_screen.nh );
      glUniform1f( shaders.jump.brightness, conf.jump_brightness );

      /* Set the subroutine. */
      if ( gl_has( OPENGL_SUBROUTINES ) ) {
         if ( cur_system->nebu_density > 0. )
            glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1,
                                     &shaders.jump.jump_func.jump_nebula );
         else
            glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1,
                                     &shaders.jump.jump_func.jump_wind );
      }

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      /* Clear state. */
      glDisableVertexAttribArray( shaders.jump.vertex );
      glUseProgram( 0 );

      /* Check errors. */
      gl_checkErr();
   }

   /* Render messages. */
   omsg_render( dt );

   gl_debugGroupEnd();
   NTracingZoneEnd( _ctx );
}

/**
 * @brief Notifies GUI scripts that the player broke out of cooldown.
 */
void gui_cooldownEnd( void )
{
   gui_doFunc( gui_lua_cooldown_end, "cooldown_end" );
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
   gui_radar.shape = circle ? RADAR_CIRCLE : RADAR_RECT;
   gui_radar.w     = w;
   gui_radar.h     = h;
   if ( circle ) {
      // In the case of the circle it's radius directly
      radar_linrange = (double)w;
   } else {
      // Average width + height and compute radius
      radar_linrange = (double)( w + h ) * 0.5 * 0.5;
   }
   // Make it be roughly 80% of the viewport
   radar_linrange *= 0.8;
   gui_setRadarResolution( player.radar_res );
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
   int           f;
   Radar        *radar;
   mat4          view_matrix_prev;
   Pilot *const *pilot_stack;

   if ( !conf.always_radar && ovr_isOpen() )
      return;

   NTracingZone( _ctx, 1 );

   /* The global radar. */
   radar       = &gui_radar;
   gui_radar.x = x;
   gui_radar.y = y;

   /* TODO: modifying gl_view_matrix like this is a bit of a hack */
   /* TODO: use stencil test for RADAR_CIRCLE */
   view_matrix_prev = gl_view_matrix;
   if ( radar->shape == RADAR_RECT ) {
      gl_clipRect( x, y, radar->w, radar->h );
      mat4_translate_xy( &gl_view_matrix, x + radar->w / 2.,
                         y + radar->h / 2. );
   } else if ( radar->shape == RADAR_CIRCLE )
      mat4_translate_xy( &gl_view_matrix, x, y );

   // Draw linear range area
   const glColour cRange = { .r = 0.45, .g = 0.45, .b = 0.6, .a = 0.1 };
   gl_renderCircle( 0., 0., radar_linrange, &cRange, 0 );

   /*
    * spobs
    */
   for ( int i = 0; i < array_size( cur_system->spobs ); i++ )
      if ( i != player.p->nav_spob )
         gui_renderSpob( i, radar->shape, radar->w, radar->h, radar->res, 1.,
                         0 );
   if ( player.p->nav_spob > -1 )
      gui_renderSpob( player.p->nav_spob, radar->shape, radar->w, radar->h,
                      radar->res, 1., 0 );

   /*
    * Jump points.
    */
   for ( int i = 0; i < array_size( cur_system->jumps ); i++ ) {
      const JumpPoint *jp = &cur_system->jumps[i];
      if ( i != player.p->nav_hyperspace && jp_isUsable( jp ) )
         gui_renderJumpPoint( i, radar->shape, radar->w, radar->h, radar->res,
                              1., 0 );
   }
   if ( player.p->nav_hyperspace > -1 )
      gui_renderJumpPoint( player.p->nav_hyperspace, radar->shape, radar->w,
                           radar->h, radar->res, 1., 0 );

   /*
    * weapons
    */
   weapon_minimap( radar->res, radar->w, radar->h, radar->shape, 1. );

   /* render the pilot */
   pilot_stack = pilot_getAll();
   f           = 0;
   for ( int i = 1; i < array_size( pilot_stack ); i++ ) { /* skip the player */
      if ( pilot_stack[i]->id == player.p->target )
         f = i;
      else
         gui_renderPilot( pilot_stack[i], radar->shape, radar->w, radar->h,
                          radar->res, 0 );
   }
   /* render the targeted pilot */
   if ( f != 0 )
      gui_renderPilot( pilot_stack[f], radar->shape, radar->w, radar->h,
                       radar->res, 0 );

   /* Render the asteroids */
   const double render_limit =
      radar->shape == RADAR_CIRCLE ? radar->w : INFINITY;
   for ( int i = 0; i < array_size( cur_system->asteroids ); i++ ) {
      AsteroidAnchor *ast   = &cur_system->asteroids[i];
      double          range = CTS.EW_ASTEROID_DIST *
                     player.p->stats.ew_detect; /* TODO don't hardcode. */
      int ax, ay, r;
      ax = round( player.p->solid.pos.x );
      ay = round( player.p->solid.pos.y );
      r  = ceil( range );
      asteroid_collideQueryIL( ast, &gui_qtquery, ax - r, ay - r, ax + r,
                               ay + r );
      for ( int j = 0; j < il_size( &gui_qtquery ); j++ ) {
         const Asteroid *a = &ast->asteroids[il_get( &gui_qtquery, j, 0 )];
         gui_renderAsteroid( a, radar->w, radar->h, radar->res, render_limit,
                             0 );
      }
   }

   /* Render the viewport frame */
   gui_renderViewportFrame( radar->res, render_limit, 0 );

   /* Render the player. */
   gui_renderPlayer( radar->res, 0 );

   /* Undo the horrible hack. */
   gl_view_matrix = view_matrix_prev;
   if ( radar->shape == RADAR_RECT )
      gl_unclipRect();

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Outputs the radar's resolution.
 *
 *    @param[out] res Current zoom ratio.
 */
void gui_radarGetRes( double *res )
{
   *res = gui_radar.res;
}

/**
 * @brief Clears the GUI messages.
 */
void gui_clearMessages( void )
{
   for ( int i = 0; i < mesg_max; i++ ) {
      free( mesg_stack[i].str );
      free( mesg_stack[i].dstr );
   }
   memset( mesg_stack, 0, sizeof( Mesg ) * mesg_max );
}

/**
 * @brief Renders the player's messages on screen.
 *
 *    @param dt Current delta tick.
 */
static void gui_renderMessages( double dt )
{
   double         x, y, h, hs, vx, vy, dy;
   int            v, o;
   glColour       c    = { .r = 1., .g = 1., .b = 1. };
   const glColour msgc = { .r = 0., .g = 0., .b = 0., .a = 0.6 };

   NTracingZone( _ctx, 1 );

   /* Coordinate translation. */
   x = gui_mesg_x;
   y = gui_mesg_y;

   /* Handle viewpoint hacks. */
   v = mesg_viewpoint;
   if ( v == -1 )
      v = mesg_pointer;

   /* Render background. */
   h = 0;

   /* Set up position. */
   vx = x;
   vy = y;

   /* Must be run here. */
   hs = 0.;
   o  = 0;
   if ( mesg_viewpoint != -1 ) {
      /* Data. */
      hs = h * (double)conf.mesg_visible / (double)mesg_max;
      o  = mesg_pointer - mesg_viewpoint;
      if ( o < 0 )
         o += mesg_max;
   }

   /* Render text. */
   for ( int i = 0; i < conf.mesg_visible; i++ ) {
      /* Reference translation. */
      int m = ( v - i ) % mesg_max;
      if ( m < 0 )
         m += mesg_max;

      /* Timer handling. */
      if ( ( mesg_viewpoint != -1 ) || ( mesg_stack[m].t >= 0. ) ) {
         /* Decrement timer. */
         if ( mesg_viewpoint == -1 )
            mesg_stack[m].t -= dt / dt_mod;

         /* Only handle non-NULL messages. */
         if ( mesg_stack[m].str != NULL ) {
            const char *str = ( mesg_stack[m].dstr != NULL )
                                 ? mesg_stack[m].dstr
                                 : mesg_stack[m].str;

            glColour fadedWhite = cFontWhite;
            fadedWhite.a = ( mesg_viewpoint != -1 ) || ( mesg_stack[m].t > 2. )
                              ? 1.
                              : mesg_stack[m].t / 2.;

            if ( str[0] == '\t' ) {
               gl_printRestore( &mesg_stack[m].restore );
               dy = gl_printHeightRaw( &gl_smallFont, gui_mesg_w, &str[1] ) + 6;
               gl_renderRect( x - 4., y - 1., gui_mesg_w - 13., dy, &msgc );
               gl_printMaxRaw( &gl_smallFont, gui_mesg_w - 45., x + 30, y + 3,
                               &fadedWhite, -1., &str[1] );
            } else {
               dy = gl_printHeightRaw( &gl_smallFont, gui_mesg_w, str ) + 6;
               gl_renderRect( x - 4., y - 1., gui_mesg_w - 13., dy, &msgc );
               gl_printMaxRaw( &gl_smallFont, gui_mesg_w - 15., x, y + 3,
                               &fadedWhite, -1., str );
            }
            h += dy;
            y += dy;
         }
      }

      /* Increase position. */
   }

   /* Render position. */
   if ( mesg_viewpoint != -1 ) {
      /* Border. */
      c.a = 0.2;
      gl_renderRect( vx + gui_mesg_w - 10., vy, 10, h, &c );

      /* Inside. */
      c.a = 0.5;
      gl_renderRect(
         vx + gui_mesg_w - 10.,
         vy + hs / 2. +
            ( h - hs ) *
               ( (double)o / (double)( mesg_max - conf.mesg_visible ) ),
         10, hs, &c );
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Gets a pilot's colour, with a special colour for targets.
 *
 *    @param p Pilot to get colour of.
 *    @return The colour of the pilot.
 *
 * @sa pilot_getColour
 */
static const glColour *gui_getPilotColour( const Pilot *p )
{
   const glColour *col;

   if ( p->id == player.p->target )
      col = &cRadar_tPilot;
   else
      col = pilot_getColour( p );

   return col;
}

/**
 * @brief Renders a pilot in the GUI radar.
 *
 *    @param p Pilot to render.
 *    @param shape Shape of the radar (`RADAR_RECT` or `RADAR_CIRCLE`).
 *    @param w Width.
 *    @param h Height.
 *    @param res Radar resolution.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderPilot( const Pilot *p, RadarShape shape, double w, double h,
                      double res, int overlay )
{
   double          x, y, scale, ssize;
   const glColour *col;
   int             scanning;

   /* Make sure is in range. */
   if ( !pilot_validTarget( player.p, p ) )
      return;

   /* Get position. */
   if ( overlay ) {
      x = ( p->solid.pos.x / res );
      y = ( p->solid.pos.y / res );
   } else {
      x = p->solid.pos.x - player.p->solid.pos.x;
      y = p->solid.pos.y - player.p->solid.pos.y;
      logradar( &x, &y, res );
   }
   /* Get size. */
   ssize = sqrt( (double)ship_size( p->ship ) );
   scale = ( ssize + 1. ) / 2. * ( 1. + RADAR_RES_REF / res );

   /* Check if pilot in range. */
   if ( ( ( shape == RADAR_RECT ) && ( ( ABS( x ) > ( w + scale ) * 0.5 ) ||
                                       ( ABS( y ) > ( h + scale ) * 0.5 ) ) ) ||
        ( ( shape == RADAR_CIRCLE ) &&
          ( ( pow2( x ) + pow2( y ) ) > pow2( w + scale ) ) ) ) {

      /* Draw little targeted symbol. */
      if ( p->id == player.p->target && !overlay )
         gui_renderRadarOutOfRange( shape, w, h, x, y, &cRadar_tPilot );
      return;
   }

   /* Transform coordinates into the 0,0 -> SCREEN_W, SCREEN_H range. */
   if ( overlay ) {
      double ox, oy;
      ovr_center( &ox, &oy );
      x += ox;
      y += oy;
   }

   if ( p->id == player.p->target )
      col = &cRadar_hilight;
   else
      col = gui_getPilotColour( p );

   scale = MAX( scale + 2.0, 3.5 + ssize ); /* Compensate for outline. */
   scanning =
      ( pilot_isFlag( p, PILOT_SCANNING ) && ( p->target == PLAYER_ID ) );

   int hilight;
   if ( scanning || pilot_isFlag( p, PILOT_HILIGHT ) ||
        pilot_isFlag( p, PILOT_HAILING ) )
      hilight = 1;
   else
      hilight = 0;

   if ( hilight ) {
      glColour highlighted = ( scanning ) ? cRadar_scanning : cRadar_hilight;
      highlighted.a        = 0.3;
      glUseProgram( shaders.hilight.program );
      glUniform1f( shaders.hilight.dt, animation_dt );
      gl_renderShader( x, y, scale * 2.0, scale * 2.0, 0., &shaders.hilight,
                       &highlighted, 1 );
      hilight = 1;
   }

   glUseProgram( shaders.pilotmarker.program );
   gl_renderShader( x, y, scale, scale, p->solid.dir, &shaders.pilotmarker, col,
                    1 );

   /* Draw selection if targeted. */
   if ( p->id == player.p->target )
      gui_blink( x, y, MAX( scale * 2., 10.0 ), &cRadar_hilight,
                 RADAR_BLINK_PILOT, blink_pilot );

   /* Draw name. */
   if ( overlay && hilight ) {
      /* TODO try to minimize overlap here. */
      gl_printMarkerRaw( &gl_smallFont, x + scale + 5., y - gl_smallFont.h / 2.,
                         col, p->name );
      if ( scanning ) {
         glUseProgram( shaders.pilotscanning.program );
         gl_renderShader( x + scale + 3., y + scale + gl_smallFont.h / 2. + 3.,
                          5., 5., 1.5 * animation_dt, &shaders.pilotscanning,
                          col, 1 );
      }
   } else {
      /* Draw scanning icon. */
      if ( scanning ) {
         glUseProgram( shaders.pilotscanning.program );
         gl_renderShader( x + scale + 3., y + scale + 3., 5., 5.,
                          1.5 * animation_dt, &shaders.pilotscanning, col, 1 );
      }
   }
}

/**
 * @brief Renders an asteroid in the GUI radar.
 *
 *    @param a Asteroid to render.
 *    @param w Width.
 *    @param h Height.
 *    @param res Radar resolution.
 *    @param render_radius Radar radius. INFINITY if it's not a circle shape
 * radar.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderAsteroid( const Asteroid *a, double w, double h, double res,
                         double render_radius, int overlay )
{
   int             i, j, targeted;
   double          x, y, r, sx, sy;
   double          px, py;
   const glColour *col;

   /* Skip invisible asteroids */
   if ( a->state != ASTEROID_FG )
      return;

   /* Recover the asteroid and field IDs. */
   i = a->id;
   j = a->parent;

   /* Make sure is in range. */
   if ( !pilot_inRangeAsteroid( player.p, i, j ) )
      return;

   /* Get position. */
   if ( overlay ) {
      x = ( a->sol.pos.x / res );
      y = ( a->sol.pos.y / res );
   } else {
      x = a->sol.pos.x - player.p->solid.pos.x;
      y = a->sol.pos.y - player.p->solid.pos.y;
      logradar( &x, &y, res );
   }

   /* Get size. */
   sx = 1.;
   sy = 1.;

   /* Transform coordinates into the 0,0 -> SCREEN_W, SCREEN_H range. */
   if ( overlay ) {
      double ox, oy;
      ovr_center( &ox, &oy );
      x += ox;
      y += oy;
      w *= 2.;
      h *= 2.;
   }

   /* Draw square. */
   px = MAX( x - sx, -w );
   py = MAX( y - sy, -h );
   r  = ( sx + sy ) / 2.0 + 1.5;
   if ( isfinite( render_radius ) &&
        pow2( x ) + pow2( y ) > pow2( render_radius - r ) ) {
      return;
   }

   targeted =
      ( ( i == player.p->nav_asteroid ) && ( j == player.p->nav_anchor ) );

   /* Colour depends if the asteroid is selected. */
   if ( targeted )
      col = &cWhite;
   else
      col = &cGrey70;

   // gl_renderRect( px, py, MIN( 2*sx, w-px ), MIN( 2*sy, h-py ), col );
   glUseProgram( shaders.asteroidmarker.program );
   gl_renderShader( px, py, r, r, 0., &shaders.asteroidmarker, col, 1 );

   if ( targeted )
      gui_blink( px, py, MAX( 7., 2.0 * r ), col, RADAR_BLINK_PILOT,
                 blink_pilot );
}

/**
 * @brief Renders the viewport frame in the GUI radar.
 *
 *    @param res Radar resolution.
 *    @param render_radius Radar radius. INFINITY if it's not a circle shape
 * radar.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderViewportFrame( double res, double render_radius, int overlay )
{
   if ( !conf.show_viewport ) {
      return;
   }

   double vp_corner_x, vp_corner_y;
   if ( overlay ) {
      const double z = cam_getZoom() * res;
      vp_corner_x    = SCREEN_W / 2.0 / z;
      vp_corner_y    = SCREEN_H / 2.0 / z * CTS.CAMERA_VIEW_INV;
   } else {
      const double z = cam_getZoom();
      // Centered on 0,0 so should be correct
      vp_corner_x = SCREEN_W / 2.0 / z;
      vp_corner_y = SCREEN_H / 2.0 / z * CTS.CAMERA_VIEW_INV;
      logradar( &vp_corner_x, &vp_corner_y, res );
   }

   if ( isfinite( render_radius ) &&
        pow2( vp_corner_x ) + pow2( vp_corner_y ) > pow2( render_radius ) ) {
      return;
   }

   double cx, cy;
   if ( overlay ) {
      /* overlay */
      cam_getPos( &cx, &cy );
      cx /= res;
      cy /= res;
      double ox, oy;
      ovr_center( &ox, &oy );
      cx += ox;
      cy += oy;
   } else {
      cx = 0.0;
      cy = 0.0;
   }
   const double right_angle = M_PI / 2.0;
   glUseProgram( shaders.viewport_frame.program );
   gl_renderShader( cx, cy - vp_corner_y, vp_corner_x, 1.0, 0.,
                    &shaders.viewport_frame, &cRadar_viewport, 1 );
   glUseProgram( shaders.viewport_frame.program );
   gl_renderShader( cx + vp_corner_x, cy, vp_corner_y, 1.0, right_angle,
                    &shaders.viewport_frame, &cRadar_viewport, 1 );
   glUseProgram( shaders.viewport_frame.program );
   gl_renderShader( cx, cy + vp_corner_y, vp_corner_x, 1.0, 0.,
                    &shaders.viewport_frame, &cRadar_viewport, 1 );
   glUseProgram( shaders.viewport_frame.program );
   gl_renderShader( cx - vp_corner_x, cy, vp_corner_y, 1.0, right_angle,
                    &shaders.viewport_frame, &cRadar_viewport, 1 );
}

/**
 * @brief Renders the player cross on the radar or whatever.
 */
void gui_renderPlayer( double res, int overlay )
{
   double x, y, r;

   /* Based on gui_renderPilot but with larger fixed size (4x normal ship, but
    * the shader is actually quite smaller so it ends up being just a bit
    * larger than a capital ship.. */
   r = ( sqrt( 24. ) + 1. ) / 2. * ( 1. + RADAR_RES_REF / res );
   if ( overlay ) {
      double ox, oy;
      ovr_center( &ox, &oy );
      x = player.p->solid.pos.x / res + ox;
      y = player.p->solid.pos.y / res + oy;
      r = MAX( 17., r );
   } else {
      x = y = 0.;
      r     = MAX( 11., r );
   }

   glUseProgram( shaders.playermarker.program );
   gl_renderShader( x, y, r, r, player.p->solid.dir, &shaders.playermarker,
                    &cRadar_player, 1 );
}

/**
 * @brief Gets the colour of a spob.
 *
 *    @param i Index of the spob to get colour of.
 *    @return Colour of the spob.
 */
static const glColour *gui_getSpobColour( int i )
{
   const glColour *col;
   const Spob     *spob = cur_system->spobs[i];

   if ( i == player.p->nav_spob )
      col = &cRadar_tSpob;
   else
      col = spob_getColour( spob );

   return col;
}

/**
 * @brief Force sets the spob and pilot radar blink.
 */
void gui_forceBlink( void )
{
   blink_pilot = 0.;
   blink_spob  = 0.;
}

/**
 * @brief Renders the spob blink around a position on the minimap.
 */
static void gui_blink( double cx, double cy, double vr, const glColour *col,
                       double blinkInterval, double blinkVar )
{
   if ( blinkVar > blinkInterval / 2. )
      return;
   glUseProgram( shaders.blinkmarker.program );
   gl_renderShader( cx, cy, vr, vr, 0., &shaders.blinkmarker, col, 1 );
}

/**
 * @brief Renders an out of range marker for the spob.
 */
static void gui_renderRadarOutOfRange( RadarShape sh, int w, int h, int cx,
                                       int cy, const glColour *col )
{
   double a, x, y, x2, y2;

   /* Draw a line like for pilots. */
   a = ANGLE( cx, cy );
   if ( sh == RADAR_CIRCLE ) {
      x = w * cos( a );
      y = w * sin( a );
   } else {
      int cxa, cya;
      cxa = ABS( cx );
      cya = ABS( cy );
      /* Determine position. */
      if ( cy >= cxa ) { /* Bottom */
         x = w / 2. * ( cx * 1. / cy );
         y = h / 2.;
      } else if ( cx >= cya ) { /* Left */
         x = w / 2.;
         y = h / 2. * ( cy * 1. / cx );
      } else if ( cya >= cxa ) { /* Top */
         x = -w / 2. * ( cx * 1. / cy );
         y = -h / 2.;
      } else { /* Right */
         x = -w / 2.;
         y = -h / 2. * ( cy * 1. / cx );
      }
   }
   x2 = x - .15 * w * cos( a );
   y2 = y - .15 * w * sin( a );

   gl_renderLine( x, y, x2, y2, col );
}

/**
 * @brief Draws the spobs in the minimap.
 *
 * Matrix mode is already displaced to centre of the minimap.
 */
void gui_renderSpob( int ind, RadarShape shape, double w, double h, double res,
                     double alpha, int overlay )
{
   GLfloat             cx, cy, r, vr;
   glColour            col;
   Spob               *spob;
   const SimpleShader *shd;

   /* Make sure is known. */
   if ( !spob_isKnown( cur_system->spobs[ind] ) )
      return;

   /* Default values. */
   spob = cur_system->spobs[ind];
   r    = spob->radius / res;
   vr   = overlay ? spob->mo.radius : MAX( r, 7.5 );

   if ( overlay ) {
      cx = spob->pos.x / res;
      cy = spob->pos.y / res;
   } else {
      double x = spob->pos.x - player.p->solid.pos.x;
      double y = spob->pos.y - player.p->solid.pos.y;
      logradar( &x, &y, res );
      cx = x;
      cy = y;
   }

   /* Check if in range. */
   if ( shape == RADAR_CIRCLE ) {
      GLfloat x = ABS( cx ) - r;
      GLfloat y = ABS( cy ) - r;
      /* Out of range. */
      if ( x * x + y * y > pow2( w + 2. * r ) ) {
         if ( ( player.p->nav_spob == ind ) && !overlay )
            gui_renderRadarOutOfRange( RADAR_CIRCLE, w, w, cx, cy,
                                       &cRadar_tSpob );
         return;
      }
   } else {
      if ( shape == RADAR_RECT ) {
         /* Out of range. */
         if ( ( ABS( cx ) - r > w * 0.5 ) || ( ABS( cy ) - r > h * 0.5 ) ) {
            if ( ( player.p->nav_spob == ind ) && !overlay )
               gui_renderRadarOutOfRange( RADAR_RECT, w, h, cx, cy,
                                          &cRadar_tSpob );
            return;
         }
      }
   }

   if ( overlay ) {
      double ox, oy;
      ovr_center( &ox, &oy );
      /* Transform coordinates. */
      cx += ox;
      cy += oy;
   }

   /* Scale according to marker. */
   vr *= spob->marker_scale;

   /* Is marked. */
   if ( spob_isKnown( spob ) && spob_isFlag( spob, SPOB_MARKED ) ) {
      glColour highlighted = cRadar_hilight;
      highlighted.a        = 0.3;
      glUseProgram( shaders.hilight.program );
      glUniform1f( shaders.hilight.dt, animation_dt );
      gl_renderShader( cx, cy, vr * 3.0, vr * 3.0, 0., &shaders.hilight,
                       &highlighted, 1 );
   }

   /* Get the colour. */
   col = *gui_getSpobColour( ind );
   col.a *= alpha;

   /* Do the blink. */
   if ( ind == player.p->nav_spob )
      gui_blink( cx, cy, vr * 2., &col, RADAR_BLINK_SPOB, blink_spob );

   if ( spob->marker != NULL )
      shd = spob->marker;
   else if ( spob_hasService( spob, SPOB_SERVICE_LAND ) ) {
      if ( spob_hasService( spob, SPOB_SERVICE_INHABITED ) )
         shd = &shaders.spobmarker_earth;
      else
         shd = &shaders.spobmarker_uninhabited;
   } else
      shd = &shaders.spobmarker_empty;

   glUseProgram( shd->program );
   gl_renderShader( cx, cy, vr, vr, 0., shd, &col, 1 );

   if ( overlay ) {
      char buf[STRMAX_SHORT];
      snprintf( buf, sizeof( buf ), "%s%s", spob_getSymbol( spob ),
                spob_name( spob ) );
      gl_printMarkerRaw( &gl_smallFont, cx + spob->mo.text_offx,
                         cy + spob->mo.text_offy, &col, buf );
   }
}

/**
 * @brief Renders a jump point on the minimap.
 *
 *    @param ind Jump point to render.
 *    @param shape Shape of the radar (`RADAR_RECT` or `RADAR_CIRCLE`).
 *    @param w Width.
 *    @param h Height.
 *    @param res Radar resolution.
 *    @param alpha Alpha to use.
 *    @param overlay Whether to render onto the overlay.
 */
void gui_renderJumpPoint( int ind, RadarShape shape, double w, double h,
                          double res, double alpha, int overlay )
{
   GLfloat     cx, cy, x, y, r, vr;
   glColour    col;
   StarSystem *s;
   JumpPoint  *jp = &cur_system->jumps[ind];

   /* Check if known */
   if ( !jp_isUsable( jp ) )
      return;

   /* Default values. */
   r  = tex_sw( jumppoint_gfx ) / 2. / res;
   vr = overlay ? jp->mo.radius : MAX( r, 5. );
   if ( overlay ) {
      cx = jp->pos.x / res;
      cy = jp->pos.y / res;
   } else {
      double jx = jp->pos.x - player.p->solid.pos.x;
      double jy = jp->pos.y - player.p->solid.pos.y;
      logradar( &jx, &jy, res );
      cx = jx;
      cy = jy;
   }

   /* Check if in range. */
   if ( shape == RADAR_RECT ) {
      /* Out of range. */
      if ( ( ABS( cx ) - r > w * 0.5 ) || ( ABS( cy ) - r > h * 0.5 ) ) {
         if ( ( player.p->nav_hyperspace == ind ) && !overlay )
            gui_renderRadarOutOfRange( RADAR_RECT, w, h, cx, cy,
                                       &cRadar_tSpob );
         return;
      }
   } else if ( shape == RADAR_CIRCLE ) {
      x = ABS( cx ) - r;
      y = ABS( cy ) - r;
      /* Out of range. */
      if ( x * x + y * y > pow2( w + 2. * r ) ) {
         if ( ( player.p->nav_hyperspace == ind ) && !overlay )
            gui_renderRadarOutOfRange( RADAR_CIRCLE, w, w, cx, cy,
                                       &cRadar_tSpob );
         return;
      }
   }

   if ( overlay ) {
      double ox, oy;
      ovr_center( &ox, &oy );
      /* Transform coordinates. */
      cx += ox;
      cy += oy;
   }

   /* See if far side is marked. */
   s = jp->target;
   if ( sys_isMarked( s ) ) {
      glColour highlighted = cRadar_hilight;
      highlighted.a        = 0.3;
      glUseProgram( shaders.hilight.program );
      glUniform1f( shaders.hilight.dt, animation_dt );
      gl_renderShader( cx, cy, vr * 3.0, vr * 3.0, 0., &shaders.hilight,
                       &highlighted, 1 );
   }

   if ( ind == player.p->nav_hyperspace )
      col = cWhite;
   else if ( jp_isFlag( jp, JP_HIDDEN ) )
      col = cRed;
   else
      col = cGreen;
   col.a *= alpha;

   glUseProgram( shaders.jumpmarker.program );
   gl_renderShader( cx, cy, vr * 1.5, vr * 1.5, M_PI - jp->angle,
                    &shaders.jumpmarker, &col, 1 );

   /* Blink ontop. */
   if ( ind == player.p->nav_hyperspace )
      gui_blink( cx, cy, vr * 3., &col, RADAR_BLINK_SPOB, blink_spob );

   /* Render name. */
   if ( overlay ) {
      char buf[STRMAX_SHORT];
      snprintf( buf, sizeof( buf ), "%s%s", jump_getSymbol( jp ),
                sys_isKnown( jp->target ) ? _( jp->target->name )
                                          : _( "Unknown" ) );
      gl_printMarkerRaw( &gl_smallFont, cx + jp->mo.text_offx,
                         cy + jp->mo.text_offy, &col, buf );
   }
}

/**
 * @brief Initializes the GUI system.
 *
 *    @return 0 on success;
 */
int gui_init( void )
{
   /* radar */
   gui_setRadarResolution( player.radar_res );

   /* Messages */
   gui_mesg_x = 20;
   gui_mesg_y = 30;
   gui_mesg_w = SCREEN_W - 400;
   if ( mesg_stack == NULL )
      mesg_stack = calloc( mesg_max, sizeof( Mesg ) );

   /* VBO. */
   if ( gui_radar_select_vbo == NULL ) {
      GLfloat vertex[16];
      vertex[0]  = -1.5;
      vertex[1]  = 1.5;
      vertex[2]  = -3.3;
      vertex[3]  = 3.3;
      vertex[4]  = 1.5;
      vertex[5]  = 1.5;
      vertex[6]  = 3.3;
      vertex[7]  = 3.3;
      vertex[8]  = 1.5;
      vertex[9]  = -1.5;
      vertex[10] = 3.3;
      vertex[11] = -3.3;
      vertex[12] = -1.5;
      vertex[13] = -1.5;
      vertex[14] = -3.3;
      vertex[15] = -3.3;
      gui_radar_select_vbo =
         gl_vboCreateStatic( sizeof( GLfloat ) * 16, vertex );
      gl_vboLabel( gui_radar_select_vbo, "GUI Radar VBO" );
   }

   /* OSD */
   osd_setup( 30., SCREEN_H - 90., 150., 300. );

   /* Icons. */
   gui_ico_hail = gl_newSprite( GUI_GFX_PATH "hail", 5, 2, 0 );

   /* Quadtrees. */
   il_create( &gui_qtquery, 1 );

   return 0;
}

/**
 * @brief Runs a GUI Lua function.
 *
 *    @param func_ref Reference of the function to run.
 *    @param func_name Name of the function to run.
 *    @return 0 on success.
 */
static int gui_doFunc( int func_ref, const char *func_name )
{
   int ret;
   if ( gui_env == NULL )
      return -1;

   ret = gui_prepFunc( func_ref, func_name );
   if ( ret )
      return ret;
   return gui_runFunc( func_name, 0, 0 );
}

/**
 * @brief Prepares to run a function.
 *
 *    @param func_ref Reference of the function to prepare.
 *    @param func_name Name of the function to prepare.
 *    @return 0 on success.
 */
static int gui_prepFunc( int func_ref, const char *func_name )
{
   (void)func_name;
#if DEBUGGING
   if ( gui_env == NULL ) {
      WARN( _( "GUI '%s': Trying to run GUI func '%s' but no GUI is loaded!" ),
            gui_name, func_name );
      return -1;
   }
#endif /* DEBUGGING */

   /* Must have a player. */
   if ( player_isFlag( PLAYER_DESTROYED ) || player_isFlag( PLAYER_CREATING ) ||
        ( player.p == NULL ) || pilot_isFlag( player.p, PILOT_DEAD ) ) {
      return -1;
   }

   /* Set up function. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, func_ref );
#if DEBUGGING
   if ( lua_isnil( naevL, -1 ) ) {
      WARN( _( "GUI '%s': no function '%s' defined!" ), gui_name, func_name );
      lua_pop( naevL, 1 );
      return -1;
   }
#endif /* DEBUGGING */
   return 0;
}

/**
 * @brief Runs a function.
 * @note Function must be prepared beforehand.
 *    @param func Name of the function to run.
 *    @param nargs Arguments to the function.
 *    @param nret Parameters to get returned from the function.
 */
static int gui_runFunc( const char *func, int nargs, int nret )
{
   /* Run the function. */
   int ret = nlua_pcall( gui_env, nargs, nret );
   if ( ret != 0 ) { /* error has occurred */
      const char *err =
         ( lua_isstring( naevL, -1 ) ) ? lua_tostring( naevL, -1 ) : NULL;
      WARN( _( "GUI '%s' Lua -> '%s': %s" ), gui_name, func,
            ( err ) ? err : _( "unknown error" ) );
      lua_pop( naevL, 1 );
      return ret;
   }

   return ret;
}

/**
 * @brief Reloads the GUI.
 */
void gui_reload( void )
{
   if ( gui_env == NULL )
      return;

   gui_load( gui_pick() );
}

/**
 * @brief Player just changed their cargo.
 */
void gui_setCargo( void )
{
   gui_doFunc( gui_lua_update_cargo, "update_cargo" );
}

/**
 * @brief Player just changed their navigation computer target.
 */
void gui_setNav( void )
{
   gui_doFunc( gui_lua_update_nav, "update_nav" );
}

/**
 * @brief Player just changed their pilot target.
 */
void gui_setTarget( void )
{
   gui_doFunc( gui_lua_update_target, "update_target" );
}

/**
 * @brief Player just upgraded their ship or modified it.
 */
void gui_setShip( void )
{
   gui_doFunc( gui_lua_update_ship, "update_ship" );
}

/**
 * @brief Player just changed their system.
 */
void gui_setSystem( void )
{
   gui_doFunc( gui_lua_update_system, "update_system" );
}

/**
 * @brief Player's relationship with a faction was modified.
 */
void gui_updateFaction( void )
{
   if ( player.p != NULL && player.p->nav_spob != -1 )
      gui_doFunc( gui_lua_update_faction, "update_faction" );
}

void gui_updateEffects( void )
{
   if ( player.p != NULL )
      gui_doFunc( gui_lua_update_effects, "update_effects" );
}

/**
 * @brief Calls trigger functions depending on who the pilot is.
 *
 *    @param pilot The pilot to act based upon.
 */
void gui_setGeneric( const Pilot *pilot )
{
   if ( gui_env == NULL )
      return;

   if ( player_isFlag( PLAYER_DESTROYED ) || player_isFlag( PLAYER_CREATING ) ||
        ( player.p == NULL ) || pilot_isFlag( player.p, PILOT_DEAD ) )
      return;

   if ( ( player.p->target != PLAYER_ID ) && ( pilot->id == player.p->target ) )
      gui_setTarget();
   else if ( pilot_isPlayer( pilot ) ) {
      gui_setCargo();
      gui_setShip();
   }
}

/**
 * @brief Determines which GUI should be used.
 */
const char *gui_pick( void )
{
   const char *gui;
   /* Don't do set a gui if player is dead. This can be triggered through
    * naev_resize and can cause an issue if player is dead. */
   if ( ( player.p == NULL ) || pilot_isFlag( player.p, PILOT_DEAD ) )
      gui = NULL;
   else if ( player.gui != NULL )
      gui = player.gui;
   else
      gui = start_gui();
   return gui;
}

/**
 * @brief Checks to see if a GUI exists.
 *
 *    @return non-zero if filename exists. zero otherwise.
 */
int gui_exists( const char *name )
{
   char path[PATH_MAX];
   snprintf( path, sizeof( path ), GUI_PATH "%s.lua", name );
   return PHYSFS_exists( path );
}

/**
 * @brief Attempts to load the actual GUI.
 *
 *    @param name Name of the GUI to load.
 *    @return 0 on success.
 */
int gui_load( const char *name )
{
   char  *buf, path[PATH_MAX];
   size_t bufsize;

   /* Set defaults. */
   gui_cleanup();
   if ( name == NULL )
      return 0;
   gui_name = strdup( name );

   /* Open file. */
   snprintf( path, sizeof( path ), GUI_PATH "%s.lua", name );
   buf = ndata_read( path, &bufsize );
   if ( buf == NULL ) {
      WARN( _( "Unable to find GUI '%s'." ), path );
      return -1;
   }

   /* Clean up. */
   nlua_freeEnv( gui_env );
   gui_env = NULL;

   /* Create Lua state. */
   gui_env = nlua_newEnv( name );
   nlua_loadStandard( gui_env );
   nlua_loadGFX( gui_env );
   nlua_loadGUI( gui_env );
   nlua_loadTk( gui_env );

   /* Run script. */
   if ( nlua_dobufenv( gui_env, buf, bufsize, path ) != 0 ) {
      WARN( _( "Failed to load GUI Lua: %s\n"
               "%s\n"
               "Most likely Lua file has improper syntax, please check" ),
            path, lua_tostring( naevL, -1 ) );
      nlua_freeEnv( gui_env );
      gui_env = NULL;
      free( buf );
      return -1;
   }
   free( buf );

   /* Load references. */
#define LUA_FUNC( funcname )                                                   \
   gui_lua_##funcname = nlua_refenvtype( gui_env, #funcname, LUA_TFUNCTION );
   LUA_FUNC( create );
   LUA_FUNC( render );
   LUA_FUNC( render_cooldown );
   LUA_FUNC( cooldown_end );
   LUA_FUNC( mouse_move );
   LUA_FUNC( mouse_click );
   LUA_FUNC( update_cargo );
   LUA_FUNC( update_nav );
   LUA_FUNC( update_target );
   LUA_FUNC( update_ship );
   LUA_FUNC( update_system );
   LUA_FUNC( update_faction );
   LUA_FUNC( update_effects );
#undef LUA_FUNC

   /* Run create function. */
   if ( gui_doFunc( gui_lua_create, "create" ) ) {
      nlua_freeEnv( gui_env );
      gui_env = NULL;
   }

   /* Recreate land window if landed. */
   if ( landed && land_spob ) {
      land_genWindows( 0 );
      window_lower( land_wid );
   }

   /* Add the GUI to the player. */
   player_guiAdd( name );

   return 0;
}

/**
 * @brief Cleans up the GUI.
 */
void gui_cleanup( void )
{
   /* Clear messages. */
   gui_clearMessages();

   /* Disable mouse voodoo. */
   gui_mouseClickEnable( 0 );
   gui_mouseMoveEnable( 0 );

   /* Set overlay bounds. */
   ovr_boundsSet( 0, 0, 0, 0 );

   /* Reset FPS. */
   fps_setPos( 15., (double)( gl_screen.h - 15 - gl_defFontMono.h ) );

   /* Destroy lua. */
   nlua_freeEnv( gui_env );
   gui_env = NULL;

   /* OMSG */
   omsg_position( SCREEN_W / 2., SCREEN_H * 2. / 3., SCREEN_W * 2. / 3. );

   /* Delete the name. */
   free( gui_name );
   gui_name = NULL;

   /* Clear timers. */
   animation_dt = 0.;

   /* Lua stuff. */
#define LUA_CLEANUP( varname )                                                 \
   if ( varname != LUA_NOREF )                                                 \
      luaL_unref( naevL, LUA_REGISTRYINDEX, varname );                         \
   varname = LUA_NOREF
   LUA_CLEANUP( gui_lua_create );
   LUA_CLEANUP( gui_lua_render );
   LUA_CLEANUP( gui_lua_render_cooldown );
   LUA_CLEANUP( gui_lua_cooldown_end );
   LUA_CLEANUP( gui_lua_mouse_move );
   LUA_CLEANUP( gui_lua_mouse_click );
   LUA_CLEANUP( gui_lua_update_cargo );
   LUA_CLEANUP( gui_lua_update_nav );
   LUA_CLEANUP( gui_lua_update_target );
   LUA_CLEANUP( gui_lua_update_ship );
   LUA_CLEANUP( gui_lua_update_system );
   LUA_CLEANUP( gui_lua_update_faction );
   LUA_CLEANUP( gui_lua_update_effects );
#undef LUA_CLEANUP
}

/**
 * @brief Frees the GUI stuff.
 */
void gui_free( void )
{
   gui_cleanup();

   free( mesg_stack );
   mesg_stack = NULL;

   gl_vboDestroy( gui_radar_select_vbo );
   gui_radar_select_vbo = NULL;

   osd_exit();

   gl_freeTexture( gui_ico_hail );
   gui_ico_hail = NULL;
   gl_freeTexture( gui_target_spob );
   gui_target_spob = NULL;
   gl_freeTexture( gui_target_pilot );
   gui_target_pilot = NULL;

   il_destroy( &gui_qtquery );

   omsg_cleanup();
}

/**
 * @brief Sets the radar resolution.
 *
 *    @param res Resolution to set to.
 */
void gui_setRadarResolution( double res )
{
   gui_radar.res = CLAMP( RADAR_RES_MIN, RADAR_RES_MAX, res );

   // Scalings for the radar
   radar_logscale = radar_linrange / log( radar_linrange );
}

/**
 * @brief Modifies the radar resolution.
 *
 *    @param mod Number of intervals to jump (up or down).
 */
void gui_setRadarRel( int mod )
{
   gui_radar.res += mod * RADAR_RES_INTERVAL;
   gui_setRadarResolution( gui_radar.res );
   player_message( _( "#oRadar set to %.0fx.#0" ), round( gui_radar.res ) );
}

/**
 * @brief Gets the hail icon texture.
 */
glTexture *gui_hailIcon( void )
{
   return gui_ico_hail;
}

/**
 * @brief Sets the spob target GFX.
 */
void gui_targetSpobGFX( const glTexture *gfx )
{
   gl_freeTexture( gui_target_spob );
   gui_target_spob = gl_dupTexture( gfx );
}

/**
 * @brief Sets the pilot target GFX.
 */
void gui_targetPilotGFX( const glTexture *gfx )
{
   gl_freeTexture( gui_target_pilot );
   gui_target_pilot = gl_dupTexture( gfx );
}

/**
 * @brief Translates a mouse position from an SDL_Event to GUI coordinates.
 */
static void gui_eventToScreenPos( float *sx, float *sy, float ex, float ey )
{
   gl_windowToScreenPos( sx, sy, ex, ey );
}

/**
 * @brief Handles a click at a position in the current system
 *
 *    @brief event The click event.
 *    @return Whether the click was used to trigger an action.
 */
int gui_radarClickEvent( SDL_Event *event )
{
   float  mxr, myr;
   int    in_bounds;
   double x, y, cx, cy;

   gui_eventToScreenPos( &mxr, &myr, event->button.x, event->button.y );
   if ( gui_radar.shape == RADAR_RECT ) {
      cx        = gui_radar.x + gui_radar.w / 2.;
      cy        = gui_radar.y + gui_radar.h / 2.;
      in_bounds = ( 2 * ABS( mxr - cx ) <= gui_radar.w &&
                    2 * ABS( myr - cy ) <= gui_radar.h );
   } else {
      cx = gui_radar.x;
      cy = gui_radar.y;
      in_bounds =
         ( pow2( mxr - cx ) + pow2( myr - cy ) <= pow2( gui_radar.w ) );
   }
   if ( !in_bounds )
      return 0;
   x                  = mxr - cx;
   y                  = myr - cy;
   double sensitivity = unlogradar( &x, &y, gui_radar.res );
   x += player.p->solid.pos.x;
   y += player.p->solid.pos.y;
   return input_clickPos( event, x, y, 1., 10. * sensitivity,
                          15. * sensitivity );
}

/**
 * @brief Handles GUI events.
 */
int gui_handleEvent( SDL_Event *evt )
{
   int   ret;
   float x, y;

   if ( player.p == NULL )
      return 0;
   if ( ( evt->type == SDL_EVENT_MOUSE_BUTTON_DOWN ) &&
        ( pilot_isFlag( player.p, PILOT_HYP_PREP ) ||
          pilot_isFlag( player.p, PILOT_HYP_BEGIN ) ||
          pilot_isFlag( player.p, PILOT_HYPERSPACE ) ) )
      return 0;

   ret = 0;
   switch ( evt->type ) {
   /* Mouse motion. */
   case SDL_EVENT_MOUSE_MOTION:
      if ( !gui_L_mmove )
         break;
      if ( gui_prepFunc( gui_lua_mouse_move, "mouse_move" ) == 0 ) {
         gui_eventToScreenPos( &x, &y, evt->motion.x, evt->motion.y );
         lua_pushnumber( naevL, x );
         lua_pushnumber( naevL, y );
         gui_runFunc( "mouse_move", 2, 0 );
      }
      break;

   /* Mouse click. */
   case SDL_EVENT_MOUSE_BUTTON_DOWN:
   case SDL_EVENT_MOUSE_BUTTON_UP:
      if ( !gui_L_mclick )
         break;
      if ( gui_prepFunc( gui_lua_mouse_click, "mouse_click" ) == 0 ) {
         lua_pushnumber( naevL, evt->button.button + 1 );
         gui_eventToScreenPos( &x, &y, evt->button.x, evt->button.y );
         lua_pushnumber( naevL, x );
         lua_pushnumber( naevL, y );
         lua_pushboolean( naevL, ( evt->type == SDL_EVENT_MOUSE_BUTTON_DOWN ) );
         gui_runFunc( "mouse_click", 4, 1 );
         ret = lua_toboolean( naevL, -1 );
         lua_pop( naevL, 1 );
      }
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
