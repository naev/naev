/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "log.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"
#include "mission.h"
#include "colour.h"
#include "player.h"
#include "faction.h"


#define MAP_WDWNAME     "Star Map" /**< Map window name. */

#define BUTTON_WIDTH    60 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define MAP_LOOP_PROT   1000 /**< Number of iterations max in pathfinding before
                                 aborting. */


static double map_zoom        = 1.; /**< Zoom of the map. */
static double map_xpos        = 0.; /**< Map X position. */
static double map_ypos        = 0.; /**< Map Y .osition. */
static int map_drag           = 0; /**< Is the user dragging the map? */
static int map_selected       = -1; /**< What system is selected on the map. */
static StarSystem **map_path  = NULL; /**< The path to current selected system. */
int map_npath                 = 0; /**< Number of systems in map_path. */
glTexture *gl_faction_disk    = NULL; /**< Texture of the disk representing factions. */

/* VBO. */
static gl_vbo *map_vbo = NULL; /**< Map VBO. */


/*
 * extern
 */
/* space.c */
extern StarSystem *systems_stack;
extern int systems_nstack;
extern int faction_nstack;


/*
 * prototypes
 */
static void map_update( unsigned int wid );
static void map_render( double bx, double by, double w, double h, void *data );
static void map_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
static void map_setZoom( double zoom );
static void map_buttonZoom( unsigned int wid, char* str );
static void map_selectCur (void);
static void map_drawMarker( double x, double y, double r,
      int num, int cur, int type );


/**
 * @brief Initializes the map subsystem.
 *
 *    @return 0 on success.
 */
int map_init (void)
{
   /* Create the VBO. */
   map_vbo = gl_vboCreateStream( sizeof(GLfloat) * 3*(2+4), NULL );
   return 0;
}


/**
 * @brief Destroys the map subsystem.
 */
void map_exit (void)
{
   /* Destroy the VBO. */
   if (map_vbo != NULL) {
      gl_vboDestroy(map_vbo);
      map_vbo = NULL;
   }
}


/**
 * @brief Opens the map window.
 */
void map_open (void)
{
   unsigned int wid;
   StarSystem *cur;
   int w,h;

   /* Destroy window if exists. */
   wid = window_get(MAP_WDWNAME);
   if (wid > 0) {
      window_destroy( wid );
      return;
   }

   /* set position to focus on current system */
   map_xpos = cur_system->pos.x;
   map_ypos = cur_system->pos.y;

   /* mark systems as needed */
   mission_sysMark();

   /* Attempt to select current map if none is selected */
   if (map_selected == -1)
      map_selectCur();

   /* get the selected system. */
   cur = system_getIndex( map_selected );

   /* Set up window size. */
   w = MAX(600, SCREEN_W - 100);
   h = MAX(540, SCREEN_H - 100);

   /* create the window. */
   wid = window_create( MAP_WDWNAME, -1, -1, w, h );
   window_setCancel( wid, window_close );

   /* 
    * SIDE TEXT
    *
    * $System
    *
    * Faction:
    *   $Faction (or Multiple)
    *
    * Status:
    *   $Status
    *
    * Planets:
    *   $Planet1, $Planet2, ...
    *
    * Services:
    *   $Services
    * 
    * ...
    *
    * [Close]
    */

   /* System Name */
   window_addText( wid, -20, -20, 100, 20, 1, "txtSysname",
         &gl_defFont, &cDConsole, cur->name );
   /* Faction */
   window_addImage( wid, -20-64, -60-64, "imgFaction", NULL, 0 );
   window_addText( wid, -20, -60, 90, 20, 0, "txtSFaction",
         &gl_smallFont, &cDConsole, "Faction:" );
   window_addText( wid, -20, -60-gl_smallFont.h-5, 80, 100, 0, "txtFaction",
         &gl_smallFont, &cBlack, NULL );
   /* Standing */
   window_addText( wid, -20, -100, 90, 20, 0, "txtSStanding",
         &gl_smallFont, &cDConsole, "Standing:" );
   window_addText( wid, -20, -100-gl_smallFont.h-5, 80, 100, 0, "txtStanding",
         &gl_smallFont, &cBlack, NULL );
   /* Presence. */
   window_addText( wid, -20, -140, 90, 20, 0, "txtSPresence",
         &gl_smallFont, &cDConsole, "Presence:" );
   window_addText( wid, -20, -140-gl_smallFont.h-5, 80, 100, 0, "txtPresence",
         &gl_smallFont, &cBlack, NULL );
   /* Planets */
   window_addText( wid, -20, -180, 90, 20, 0, "txtSPlanets",
         &gl_smallFont, &cDConsole, "Planets:" );
   window_addText( wid, -20, -180-gl_smallFont.h-5, 80, 100, 0, "txtPlanets",
         &gl_smallFont, &cBlack, NULL );
   /* Services */
   window_addText( wid, -20, -220, 90, 20, 0, "txtSServices",
         &gl_smallFont, &cDConsole, "Services:" );
   window_addText( wid, -20, -220-gl_smallFont.h-5, 80, 100, 0, "txtServices",
         &gl_smallFont, &cBlack, NULL );
   /* Close button */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnClose", "Close", window_close );

   /*
    * Bottom stuff
    *
    * [+] [-]  Nebula, Asteroids, Interference
    */
   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", map_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", map_buttonZoom );
   /* Situation text */
   window_addText( wid, 140, 10, w - 80 - 30 - 30, 30, 0,
         "txtSystemStatus", &gl_smallFont, &cBlack, NULL );

   /*
    * The map itself.
    */
   map_show( wid, 20, -40, w-150, h-100, 1. ); /* Reset zoom. */

   map_update( wid );
}
/**
 * @brief Updates the map window.
 *
 *    @param wid Window id.
 */
static void map_update( unsigned int wid )
{
   int i;
   StarSystem* sys;
   int f, y, h, multiple_faction;
   double standing, nstanding;
   unsigned int services;
   int l;
   int hasPresence, hasPlanets;
   char t;
   char buf[PATH_MAX];
   int p;
   glTexture *logo;
   double w;

   /* Needs map to update. */
   if (!map_isOpen())
      return;

   /* Get selected system. */
   sys = system_getIndex( map_selected );
   w   = 80.; /* Width of the side bar. */

   /* Not known and no markers. */
   if (!(sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)) &&
         !sys_isKnown(sys) && !space_sysReachable(sys)) {
      map_selectCur();
      sys = system_getIndex( map_selected );
   }

   /*
    * Right Text
    */
   if (!sys_isKnown(sys)) { /* System isn't known, erase all */
      /*
       * Right Text
       */
      if (sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED))
         window_modifyText( wid, "txtSysname", sys->name );
      else
         window_modifyText( wid, "txtSysname", "Unknown" );
      window_modifyImage( wid, "imgFaction", NULL );
      window_moveWidget( wid, "txtSFaction", -20, -60 );
      window_moveWidget( wid, "txtFaction", -20, -60-gl_smallFont.h-5 );
      window_modifyText( wid, "txtFaction", "Unknown" );
      /* Standing */
      window_moveWidget( wid, "txtSStanding", -20, -100 );
      window_moveWidget( wid, "txtStanding", -20, -100-gl_smallFont.h-5 );
      window_modifyText( wid, "txtStanding", "Unknown" );
      /* Presence. */
      window_moveWidget( wid, "txtSPresence", -20, -140 );
      window_moveWidget( wid, "txtPresence",  -20, -140-gl_smallFont.h-5 );
      window_modifyText( wid, "txtPresence", "Unknown" );
      /* Planets */
      window_moveWidget( wid, "txtSPlanets", -20, -180 );
      window_moveWidget( wid, "txtPlanets", -20, -180-gl_smallFont.h-5 );
      window_modifyText( wid, "txtPlanets", "Unknown" );
      /* Services */
      window_moveWidget( wid, "txtSServices", -20, -220 );
      window_moveWidget( wid, "txtServices", -20, -220-gl_smallFont.h-5 );
      window_modifyText( wid, "txtServices", "Unknown" );
      
      /*
       * Bottom Text
       */
      window_modifyText( wid, "txtSystemStatus", NULL );
      return;
   }

   /* System is known */
   window_modifyText( wid, "txtSysname", sys->name );

   standing  = 0.;
   nstanding = 0.;
   f         = -1;
   multiple_faction = 0;
   for (i=0; i<sys->nplanets; i++) {
      if(sys->planets[i]->real == ASSET_REAL) {
         if ((f==-1) && (sys->planets[i]->faction>0)) {
            f = sys->planets[i]->faction;
            standing += faction_getPlayer( f );
            nstanding++;
         }
         else if (f != sys->planets[i]->faction && /** @todo more verbosity */
                  (sys->planets[i]->faction>0)) {
            snprintf( buf, PATH_MAX, "Multiple" );
            multiple_faction = 1;
            break;
         }
      }
   }
   if (f == -1) {
      window_modifyImage( wid, "imgFaction", NULL );
      window_moveWidget( wid, "txtSFaction", -20, -60 );
      window_moveWidget( wid, "txtFaction", -20, -60-gl_smallFont.h-5 );
      window_modifyText( wid, "txtFaction", "NA" );
      window_moveWidget( wid, "txtSStanding", -20, -100 );
      window_moveWidget( wid, "txtStanding", -20, -100-gl_smallFont.h-5 );
      window_modifyText( wid, "txtStanding", "NA" );
      y = -100;

   }
   else {
      if (i==sys->nplanets) /* saw them all and all the same */
         snprintf( buf, PATH_MAX, "%s", faction_longname(f) );

      /* Scroll down. */
      y = -60;

      /* Modify the image. */
      logo = faction_logoSmall(f);
      window_modifyImage( wid, "imgFaction", logo );
      if (logo != NULL) {
         window_moveWidget( wid, "imgFaction",
               -(90-logo->w)/2-20-logo->w, y-(64-logo->h)/2-logo->h );
      /* Scroll down. */
         y -= 64 + 10;
      }

      /* Modify the text */
      window_modifyText( wid, "txtFaction", buf );
      window_modifyText( wid, "txtStanding",
            faction_getStanding( standing / nstanding ) );

      /* Lower text if needed */
      window_moveWidget( wid, "txtSFaction", -20, y );
      window_moveWidget( wid, "txtFaction", -20, y-gl_smallFont.h-5 );
      h = gl_printHeightRaw( &gl_smallFont, w, buf );
      window_moveWidget( wid, "txtSStanding", -20, y );
      window_moveWidget( wid, "txtStanding", -20, y-gl_smallFont.h-5 );
      /* Scroll down. */
      y -= 40 + (h - gl_smallFont.h);
      window_moveWidget( wid, "txtSStanding", -20, y );
      window_moveWidget( wid, "txtStanding", -20, y-gl_smallFont.h-5 );
   }

   /* Scroll down. */
   y -= 40;
   /* Get presence. */
   hasPresence = 0;
   buf[0]      = '\0';
   l           = 0;
   for(i = 0; i < sys->npresence ; i++)
      if(sys->presence[i].value > 0) {
         hasPresence = 1;
         t           = faction_getColourChar(sys->presence[i].faction);
         /* Use map grey instead of default neutral colour */
         l += snprintf( &buf[l], PATH_MAX-l, "%s\e0%s: \e%c%.0f",
                        (l==0)?"":"\n", faction_name(sys->presence[i].faction),
                        (t=='N')?'M':t, sys->presence[i].value);
      }
   if(hasPresence == 0)
      snprintf(buf, PATH_MAX, "N/A");
   window_moveWidget( wid, "txtSPresence", -20, y );
   window_moveWidget( wid, "txtPresence", -20, y-gl_smallFont.h-5 );
   window_modifyText( wid, "txtPresence", buf );
   /* Scroll down. */
   h  = gl_printHeightRaw( &gl_smallFont, w, buf );
   y -= 40 + (h - gl_smallFont.h);

   /* Get planets */
   hasPlanets = 0;
   p = 0;
   buf[0] = '\0';
   if (sys->nplanets > 0)
      if(sys->planets[0]->real == ASSET_REAL) {
         hasPlanets = 1;
         p += snprintf( &buf[p], PATH_MAX-p, "%s", sys->planets[0]->name );
      }
   for (i=1; i<sys->nplanets; i++) {
      if(sys->planets[i]->real == ASSET_UNREAL)
         continue;
      hasPlanets = 1;
      p += snprintf( &buf[p], PATH_MAX-p, ",\n%s", sys->planets[i]->name );
   }
   if(hasPlanets == 0)
      strncpy( buf, "None", PATH_MAX );
   /* Update text. */
   window_modifyText( wid, "txtPlanets", buf );
   window_moveWidget( wid, "txtSPlanets", -20, y );
   window_moveWidget( wid, "txtPlanets", -20, y-gl_smallFont.h-5 );
   /* Scroll down. */
   h  = gl_printHeightRaw( &gl_smallFont, w, buf );
   y -= 40 + (h - gl_smallFont.h);

   /* Get the services */
   window_moveWidget( wid, "txtSServices", -20, y );
   window_moveWidget( wid, "txtServices", -20, y-gl_smallFont.h-5 );
   services = 0;
   for (i=0; i<sys->nplanets; i++)
      services |= sys->planets[i]->services;
   buf[0] = '\0';
   p = 0;
   /*snprintf(buf, sizeof(buf), "%f\n", sys->prices[0]);*/ /*Hack to control prices. */
   if (services & PLANET_SERVICE_COMMODITY)
      p += snprintf( &buf[p], PATH_MAX-p, "Commodity\n");
   if (services & PLANET_SERVICE_OUTFITS)
      p += snprintf( &buf[p], PATH_MAX-p, "Outfits\n");
   if (services & PLANET_SERVICE_SHIPYARD)
      p += snprintf( &buf[p], PATH_MAX-p, "Shipyard\n");
   if (buf[0] == '\0')
      p += snprintf( &buf[p], PATH_MAX-p, "None");
   window_modifyText( wid, "txtServices", buf );


   /*
    * System Status
    */
   buf[0] = '\0';
   p = 0;
   /* Nebula. */
   if (sys->nebu_density > 0.) {

      /* Volatility */
      if (sys->nebu_volatility > 700.)
         p += snprintf(&buf[p], PATH_MAX-p, " Volatile");
      else if (sys->nebu_volatility > 300.)
         p += snprintf(&buf[p], PATH_MAX-p, " Dangerous");
      else if (sys->nebu_volatility > 0.)
         p += snprintf(&buf[p], PATH_MAX-p, " Unstable");

      /* Density */
      if (sys->nebu_density > 700.)
         p += snprintf(&buf[p], PATH_MAX-p, " Dense");
      else if (sys->nebu_density < 300.)
         p += snprintf(&buf[p], PATH_MAX-p, " Light");
      p += snprintf(&buf[p], PATH_MAX-p, " Nebula");
   }
   /* Interference. */
   if (sys->interference > 0.) {

      if (buf[0] != '\0')
         p += snprintf(&buf[p], PATH_MAX-p, ",");

      /* Density. */
      if (sys->interference > 700.)
         p += snprintf(&buf[p], PATH_MAX-p, " Dense");
      else if (sys->interference < 300.)
         p += snprintf(&buf[p], PATH_MAX-p, " Light");

      p += snprintf(&buf[p], PATH_MAX-p, " Interference");
   }
   window_modifyText( wid, "txtSystemStatus", buf );
}


/**
 * @brief Checks to see if the map is open.
 *
 *    @return 0 if map is closed, non-zero if it's open.
 */
int map_isOpen (void)
{
   return window_exists(MAP_WDWNAME);
}


/**
 * @brief Draws a mission marker on the map.
 *
 * @param x X position to draw at.
 * @param y Y position to draw at.
 * @param r Radius of system.
 * @param num Total number of markers.
 * @param cur Current marker to draw.
 * @param type Type to draw.
 */
static void map_drawMarker( double x, double y, double r,
      int num, int cur, int type )
{
   const double beta = M_PI / 9;
   static const glColour* colours[] = {
      &cGreen, &cBlue, &cRed, &cOrange
   };

   int i;
   double alpha, cos_alpha, sin_alpha;
   GLfloat vertex[3*(2+4)];


   /* Calculate the angle. */
   if ((num == 1) || (num == 2) || (num == 4))
      alpha = M_PI/4.;
   else if (num == 3)
      alpha = M_PI/6.;
   else if (num == 5)
      alpha = M_PI/10.;
   else
      alpha = M_PI/2.;

   alpha += M_PI*2. * (double)cur/(double)num;
   cos_alpha = r * cos(alpha);
   sin_alpha = r * sin(alpha);
   r = 3 * r;

   /* Draw the marking triangle. */
   vertex[0] = x + cos_alpha;
   vertex[1] = y + sin_alpha;
   vertex[2] = x + cos_alpha + r * cos(beta + alpha);
   vertex[3] = y + sin_alpha + r * sin(beta + alpha);
   vertex[4] = x + cos_alpha + r * cos(beta - alpha);
   vertex[5] = y + sin_alpha - r * sin(beta - alpha);

   for (i=0; i<3; i++) {
      vertex[6 + 4*i + 0] = colours[type]->r;
      vertex[6 + 4*i + 1] = colours[type]->g;
      vertex[6 + 4*i + 2] = colours[type]->b;
      vertex[6 + 4*i + 3] = colours[type]->a;
   }

   glEnable(GL_POLYGON_SMOOTH);
   gl_vboSubData( map_vbo, 0, sizeof(GLfloat) * 3*(2+4), vertex );
   gl_vboActivateOffset( map_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( map_vbo, GL_COLOR_ARRAY,
         sizeof(GLfloat) * 2*3, 4, GL_FLOAT, 0 );
   glDrawArrays( GL_TRIANGLES, 0, 3 );
   gl_vboDeactivate();
   glDisable(GL_POLYGON_SMOOTH);
}

/**
 * @brief Generates a texture to represent factions
 *
 * @param radius radius of the disk
 * @return the texture
 */
static glTexture *gl_genFactionDisk( int radius )
{
   int i, j;
   uint8_t *pixels;
   SDL_Surface *sur;
   int dist;
   double alpha;

   /* Calculate parameters. */
   const int w = 2 * radius + 1;
   const int h = 2 * radius + 1;

   /* Create the surface. */
   sur = SDL_CreateRGBSurface( SDL_SRCALPHA | SDL_HWSURFACE, w, h, 32, RGBAMASK );

   pixels = sur->pixels;
   memset(pixels, 0xff, sizeof(uint8_t) * 4 * h * w);

   /* Generate the circle. */
   SDL_LockSurface( sur );

   /* Draw the circle with filter. */
   for (i=0; i<h; i++) {
      for (j=0; j<w; j++) {
         /* Calculate blur. */
         dist = (i - radius) * (i - radius) + (j - radius) * (j - radius);
         alpha = 0.;

         if (dist < radius * radius) {
            /* Computes alpha with an empirically chosen formula.
             * This formula accounts for the fact that the eyes
             * has a logarithmic sensitivity to light */
            alpha = 1. * dist / (radius * radius);
            alpha = (exp(1 / (alpha + 1) - 0.5) - 1) * 0xFF;
         }

         /* Sets the pixel alpha which is the forth byte
          * in the pixel representation. */
         pixels[i*sur->pitch + j*4 + 3] = (uint8_t)alpha;
      }
   }

   SDL_UnlockSurface( sur );

   /* Return texture. */
   return gl_loadImage( sur, OPENGL_TEX_MIPMAPS );
}

/**
 * @brief Renders the custom map widget.
 *
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void map_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   int i,j, n,m;
   double x,y,r, tx,ty, fuel;
   StarSystem *sys, *jsys, *hsys, *lsys;
   glColour *col, c;
   GLfloat vertex[8*(2+4)];
   int sw, sh;

   /* Parameters. */
   r = round(CLAMP(5., 15., 6.*map_zoom));
   x = round((bx - map_xpos + w/2) * 1.);
   y = round((by - map_ypos + h/2) * 1.);

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /*
    * First pass renders everything almost (except names and markers).
    */
   for (i=0; i<systems_nstack; i++) {
      sys = system_getIndex( i );

      /* check to make sure system is known or adjacent to known (or marked) */
      if (!sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
            && !space_sysReachable(sys))
         continue;

      tx = x + sys->pos.x*map_zoom;
      ty = y + sys->pos.y*map_zoom;

      /* draws the disk representing the faction */
      if (sys_isKnown(sys) && (sys->faction != -1)) {
         sw = gl_faction_disk->sw;
         sh = gl_faction_disk->sw;

         col = faction_colour(sys->faction);
         c.r = col->r;
         c.g = col->g;
         c.b = col->b;
         c.a = 0.7;

         gl_blitTexture(
               gl_faction_disk,
               tx - sw/2, ty - sh/2, sw, sh,
               0., 0., gl_faction_disk->srw, gl_faction_disk->srw, &c );
      }

      /* Draw the system. */
      if (!sys_isKnown(sys) || sys->faction == -1)
         col = &cInert;
      else
         col = faction_colour(sys->faction);

      gl_drawCircleInRect( tx, ty, r, bx, by, w, h, col, 0 );

      /* If system is known fill it. */
      if (sys_isKnown(sys) && (system_hasPlanet(sys))) {
         /* Planet colours */
         if (!sys_isKnown(sys)) col = &cInert;
         else if (sys->nplanets==0) col = &cInert;
         else col = faction_getColour( sys->faction);

         /* Radius slightly shorter. */
         gl_drawCircleInRect( tx, ty, 0.5*r, bx, by, w, h, col, 1 );
      }

      if (!sys_isKnown(sys))
         continue; /* we don't draw hyperspace lines */

      /* draw the hyperspace paths */
      glShadeModel(GL_SMOOTH);
      col = &cDarkBlue;
      /* first we draw all of the paths. */  
      for (j=0; j<sys->njumps; j++) {

         jsys = system_getIndex( sys->jumps[j] );
         if (hyperspace_target != -1)
            hsys = system_getIndex( cur_system->jumps[hyperspace_target] );

         /* Draw the lines. */
         vertex[0]  = x + sys->pos.x * map_zoom;
         vertex[1]  = y + sys->pos.y * map_zoom;
         vertex[2]  = vertex[0] + (jsys->pos.x - sys->pos.x)/2. * map_zoom;
         vertex[3]  = vertex[1] + (jsys->pos.y - sys->pos.y)/2. * map_zoom;
         vertex[4]  = x + jsys->pos.x * map_zoom;
         vertex[5]  = y + jsys->pos.y * map_zoom;
         vertex[6]  = col->r;
         vertex[7]  = col->g;
         vertex[8]  = col->b;
         vertex[9]  = 0.;
         vertex[10] = col->r;
         vertex[11] = col->g;
         vertex[12] = col->b;
         vertex[13] = col->a;
         vertex[14] = col->r;
         vertex[15] = col->g;
         vertex[16] = col->b;
         vertex[17] = 0.;
         gl_vboSubData( map_vbo, 0, sizeof(GLfloat) * 3*(2+4), vertex );
         gl_vboActivateOffset( map_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
         gl_vboActivateOffset( map_vbo, GL_COLOR_ARRAY,
               sizeof(GLfloat) * 2*3, 4, GL_FLOAT, 0 );
         glDrawArrays( GL_LINE_STRIP, 0, 3 );
         gl_vboDeactivate();
      }
      glShadeModel( GL_FLAT );
   }
   
   /* Now we'll draw over the lines with the new pathways. */
   if (map_path != NULL) {
      lsys = cur_system;
      glShadeModel(GL_SMOOTH);
      col = &cGreen;
      fuel = player->fuel;
      
      for (j=0; j<map_npath; j++) {
         jsys = map_path[j];
         if (fuel == player->fuel)
            col = &cGreen;
         else if (fuel < 100.)
            col = &cRed;
         else
            col = &cYellow;
         fuel -= 100;
         
         /* Draw the lines. */
         vertex[0]  = x + lsys->pos.x * map_zoom;
         vertex[1]  = y + lsys->pos.y * map_zoom;
         vertex[2]  = vertex[0] + (jsys->pos.x - lsys->pos.x)/2. * map_zoom;
         vertex[3]  = vertex[1] + (jsys->pos.y - lsys->pos.y)/2. * map_zoom;
         vertex[4]  = x + jsys->pos.x * map_zoom;
         vertex[5]  = y + jsys->pos.y * map_zoom;
         vertex[6]  = col->r;
         vertex[7]  = col->g;
         vertex[8]  = col->b;
         vertex[9]  = 0.;
         vertex[10] = col->r;
         vertex[11] = col->g;
         vertex[12] = col->b;
         vertex[13] = col->a;
         vertex[14] = col->r;
         vertex[15] = col->g;
         vertex[16] = col->b;
         vertex[17] = 0.;
         gl_vboSubData( map_vbo, 0, sizeof(GLfloat) * 3*(2+4), vertex );
         gl_vboActivateOffset( map_vbo, GL_VERTEX_ARRAY, 0, 2, GL_FLOAT, 0 );
         gl_vboActivateOffset( map_vbo, GL_COLOR_ARRAY,
               sizeof(GLfloat) * 2*3, 4, GL_FLOAT, 0 );
         glDrawArrays( GL_LINE_STRIP, 0, 3 );
         gl_vboDeactivate();
         
         lsys = jsys;
      }
      
      glShadeModel( GL_FLAT );
   }

   /*
    * Second pass - System names
    */
   for (i=0; i<systems_nstack; i++) {
      sys = system_getIndex( i );

      /* Skip system. */
      if (!sys_isKnown(sys) || (map_zoom <= 0.5 ))
         continue;

      tx = x + (sys->pos.x+11.) * map_zoom;
      ty = y + (sys->pos.y-5.) * map_zoom;
      gl_print( &gl_smallFont,
            tx + SCREEN_W/2., ty + SCREEN_H/2.,
            &cWhite, sys->name );
   }


   /*
    * Third pass - system markers
    */
   for (i=0; i<systems_nstack; i++) {
      sys = system_getIndex( i );

      /* We only care about marked now. */
      if (!sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED))
         continue;

      /* Get the position. */
      tx = x + sys->pos.x*map_zoom;
      ty = y + sys->pos.y*map_zoom;

      /* Count markers. */
      n  = (sys_isFlag(sys, SYSTEM_CMARKED)) ? 1 : 0;
      n += sys->markers_misc;
      n += sys->markers_cargo;
      n += sys->markers_rush;

      /* Draw the markers. */
      j = 0;
      if (sys_isFlag(sys, SYSTEM_CMARKED)) {
         map_drawMarker( tx, ty, r, n, j, 0 );
         j++;
      }
      for (m=0; m<sys->markers_misc; m++) {
         map_drawMarker( tx, ty, r, n, j, 1 );
         j++;
      }
      for (m=0; m<sys->markers_rush; m++) {
         map_drawMarker( tx, ty, r, n, j, 2 );
         j++;
      }
      for (m=0; m<sys->markers_cargo; m++) {
         map_drawMarker( tx, ty, r, n, j, 3 );
         j++;
      }
   }

   /* Selected planet. */
   if (map_selected != -1) {
      sys = system_getIndex( map_selected );
      gl_drawCircleInRect( x + sys->pos.x * map_zoom, y + sys->pos.y * map_zoom,
            1.5*r, bx, by, w, h, &cRed, 0 );
   }

   /* Current planet. */
   gl_drawCircleInRect( x + cur_system->pos.x * map_zoom,
         y + cur_system->pos.y * map_zoom,
         1.5*r, bx, by, w, h, &cRadar_tPlanet, 0 );
}
/**
 * @brief Map custom widget mouse handling.
 *
 *    @param wid Window sending events.
 *    @param event Event window is sending.
 *    @param mx Mouse X position.
 *    @param my Mouse Y position.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void map_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i;
   double x,y, t;
   StarSystem *sys;

   t = 15.*15.; /* threshold */

   switch (event->type) {
      
      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return;

         /* Zooming */
         if (event->button.button == SDL_BUTTON_WHEELUP)
            map_buttonZoom( 0, "btnZoomIn" );
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            map_buttonZoom( 0, "btnZoomOut" );

         /* selecting star system */
         else {
            mx -= w/2 - map_xpos;
            my -= h/2 - map_ypos;

            for (i=0; i<systems_nstack; i++) {
               sys = system_getIndex( i );

               /* must be reachable */
               if (!sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
                     && !space_sysReachable(sys))
                  continue;

               /* get position */
               x = sys->pos.x * map_zoom;
               y = sys->pos.y * map_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  map_select( sys, (SDL_GetModState() & KMOD_SHIFT) );
                  break;
               }
            }
            map_drag = 1;
         }
         break;

      case SDL_MOUSEBUTTONUP:
         if (map_drag)
            map_drag = 0;
         break;

      case SDL_MOUSEMOTION:
         if (map_drag) {
            /* axis is inverted */
            map_xpos -= event->motion.xrel;
            map_ypos += event->motion.yrel;
         }
         break;
   }
}
/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void map_buttonZoom( unsigned int wid, char* str )
{
   (void) wid;

   /* Transform coords to normal. */
   map_xpos /= map_zoom;
   map_ypos /= map_zoom;

   /* Apply zoom. */
   if (strcmp(str,"btnZoomIn")==0) {
      map_zoom += (map_zoom >= 1.) ? 0.5 : 0.25;
      map_zoom = MIN(2.5, map_zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      map_zoom -= (map_zoom > 1.) ? 0.5 : 0.25;
      map_zoom = MAX(0.5, map_zoom);
   }

   map_setZoom(map_zoom);

   /* Transform coords back. */
   map_xpos *= map_zoom;
   map_ypos *= map_zoom;
}


/**
 * @brief Cleans up the map stuff.
 */
void map_cleanup (void)
{
   map_close();
   map_clear();
}


/**
 * @brief Closes the map.
 */
void map_close (void)
{
   unsigned int wid;

   wid = window_get(MAP_WDWNAME);
   if (wid > 0)
      window_destroy(wid);
}


/**
 * @brief Sets the map to sane defaults
 */
void map_clear (void)
{
   map_setZoom(1.);

   if (cur_system != NULL) {
      map_xpos = cur_system->pos.x;
      map_ypos = cur_system->pos.y;
   }
   else {
      map_xpos = 0.;
      map_ypos = 0.;
   }
   if (map_path != NULL) {
      free(map_path);
      map_path = NULL;
      map_npath = 0;
   }

   /* default system is current system */
   map_selectCur();
}


/**
 * @brief Tries to select the current system.
 */
static void map_selectCur (void)
{
   if (cur_system != NULL)
      map_selected = cur_system - systems_stack;
   else
      /* will probably segfault now */
      map_selected = -1;
}

/**
 * @brief Updates the map after a jump.
 */
void map_jump (void)
{
   int j;

   /* set selected system to self */
   map_selectCur();

   map_xpos = cur_system->pos.x;
   map_ypos = cur_system->pos.y;

   /* update path if set */
   if (map_path != NULL) {
      map_npath--;
      if (map_npath == 0) { /* path is empty */
         free (map_path);
         map_path = NULL;
         planet_target = -1;
         hyperspace_target = -1;
      }
      else { /* get rid of bottom of the path */
         memmove( &map_path[0], &map_path[1], sizeof(StarSystem*) * map_npath );
         map_path = realloc( map_path, sizeof(StarSystem*) * map_npath );

         /* set the next jump to be to the next in path */
         for (j=0; j<cur_system->njumps; j++) {
            if (map_path[0]==system_getIndex(cur_system->jumps[j])) {
               planet_target = -1; /* override planet_target */
               hyperspace_target = j;
               break;
            }
         }

      }
   }
}


/**
 * @brief Selects the system in the map.
 *
 *    @param sys System to select.
 */
void map_select( StarSystem *sys, char shifted )
{
   unsigned int wid;
   int i;

   wid = window_get(MAP_WDWNAME);

   if (sys == NULL) {
      map_selectCur();
   }
   else {
      map_selected = sys - systems_stack;

      /* select the current system and make a path to it */
      if (!shifted) {
          if (map_path)
             free(map_path);
          map_path  = NULL;
          map_npath = 0;
      }

      /* Try to make path if is reachable. */
      if (space_sysReachable(sys)) {
         if (!shifted) {
             map_path = map_getJumpPath( &map_npath,
                   cur_system->name, sys->name, 0 , NULL );
         }
         else {
             map_path = map_getJumpPath( &map_npath,
                   cur_system->name, sys->name, 0 , map_path );
         }

         if (map_npath==0)
            hyperspace_target = -1;
         else  {
            /* see if it is a valid hyperspace target */
            for (i=0; i<cur_system->njumps; i++) {
               if (map_path[0] == system_getIndex(cur_system->jumps[i])) {
                  planet_target     = -1; /* override planet_target */
                  hyperspace_target = i;
                  break;
               }
            }
         }
      }
      else { /* unreachable. */
         hyperspace_target = -1;
      }
   }

   map_update(wid);
}

/*
 * A* algorithm for shortest path finding
 */
/**
 * @brief Node structure for A* pathfinding.
 */
typedef struct SysNode_ {
   struct SysNode_ *next; /**< Next node */
   struct SysNode_ *gnext; /**< Next node in the garbage collector. */

   struct SysNode_ *parent; /**< Parent node. */
   StarSystem* sys; /**< System in node. */
   double r; /**< ranking */
   int g; /**< step */
} SysNode; /**< System Node for use in A* pathfinding. */
static SysNode *A_gc;
/* prototypes */
static SysNode* A_newNode( StarSystem* sys, SysNode* parent );
static double A_h( StarSystem *n, StarSystem *g );
static double A_g( SysNode* n );
static SysNode* A_add( SysNode *first, SysNode *cur );
static SysNode* A_rm( SysNode *first, StarSystem *cur );
static SysNode* A_in( SysNode *first, StarSystem *cur );
static SysNode* A_lowest( SysNode *first );
static void A_freeList( SysNode *first );
/** @brief Creates a new node linke to star system. */
static SysNode* A_newNode( StarSystem* sys, SysNode* parent )
{
   SysNode* n;

   n = malloc(sizeof(SysNode));

   n->next = NULL;
   n->parent = parent;
   n->sys = sys;
   n->r = DBL_MAX;
   n->g = 0.;

   n->gnext = A_gc;
   A_gc = n;

   return n;
}
/** @brief Heurestic model to use. */
static double A_h( StarSystem *n, StarSystem *g )
{
   (void)n;
   (void)g;
   /* Euclidean distance */
   /*return sqrt(pow2(n->pos.x - g->pos.x) + pow2(n->pos.y - g->pos.y))/100.;*/
   return 0.;
}
/** @brief Gets the g from a node. */
static double A_g( SysNode* n )
{
   return n->g;
}
/** @brief Adds a node to the linked list. */
static SysNode* A_add( SysNode *first, SysNode *cur )
{
   SysNode *n;

   if (first == NULL)
      return cur;

   n = first;
   while (n->next != NULL)
      n = n->next;
   n->next = cur;

   return first;
}
/* @brief Removes a node from a linked list. */
static SysNode* A_rm( SysNode *first, StarSystem *cur )
{
   SysNode *n, *p;

   if (first->sys == cur) {
      n = first->next;
      first->next = NULL;
      return n;
   }

   p = first;
   n = p->next;
   do {
      if (n->sys == cur) {
         n->next = NULL;
         p->next = n->next;
         break;
      }
      p = n;
   } while ((n=n->next) != NULL);

   return first;
}
/** @brief Checks to see if node is in linked list. */
static SysNode* A_in( SysNode *first, StarSystem *cur )
{
   SysNode *n;

   if (first == NULL)
      return NULL;

   n = first;
   do {
      if (n->sys == cur)
         return n;
   } while ((n=n->next) != NULL);
   return NULL;
}
/** @brief Returns the lowest ranking node from a linked list of nodes. */
static SysNode* A_lowest( SysNode *first )
{
   SysNode *lowest, *n;

   if (first == NULL)
      return NULL;

   n = first;
   lowest = n;
   do {
      if (n->r < lowest->r)
         lowest = n;
   } while ((n=n->next) != NULL);
   return lowest;
}
/** @brief Frees a linked list. */
static void A_freeList( SysNode *first )
{
   SysNode *p, *n;

   if (first == NULL)
      return;

   p = NULL;
   n = first;
   do {
      if (p != NULL)
         free(p);
      p = n;
   } while ((n=n->gnext) != NULL);
   free(p);
}

/** @brief Sets map_zoom to zoom and recreats the faction disk texture. */
void map_setZoom(double zoom)
{
   map_zoom = zoom;
   if (gl_faction_disk != NULL)
      gl_freeTexture( gl_faction_disk );
   gl_faction_disk = gl_genFactionDisk( 50 * zoom );
}

/**
 * @brief Gets the jump path between two systems.
 *
 *    @param[out] njumps Number of jumps in the path.
 *    @param sysstart Name of the system to start from.
 *    @param sysend Name of the system to end at.
 *    @param ignore_known Whether or not to ignore if systems are known.
 *    @param the old star system (if we're merely extending the list)
 *    @return NULL on failure, the list of njumps elements systems in the path.
 */
StarSystem** map_getJumpPath( int* njumps, const char* sysstart,
    const char* sysend, int ignore_known, StarSystem** old_data )
{
   int i, j, cost, ojumps;

   StarSystem *sys, *ssys, *esys, **res;

   SysNode *cur, *neighbour;
   SysNode *open, *closed;
   SysNode *ocost, *ccost;

   A_gc = NULL;

   /* initial and target systems */
   ssys = system_get(sysstart); /* start */
   esys = system_get(sysend); /* goal */

   /* Set up. */
   ojumps = 0;
   if ((old_data != NULL) && (*njumps>0)) {
      ssys   = system_get( old_data[ (*njumps)-1 ]->name );
      ojumps = *njumps;
   }

   /* Check self. */
   if (ssys == esys) {
      (*njumps) = 0;
      if (old_data != NULL)
         free( old_data );
      return NULL;
   }

   /* system target must be known and reachable */
   if (!ignore_known && !sys_isKnown(esys) && !space_sysReachable(esys)) {
      /* can't reach - don't make path */
      (*njumps) = 0;
      if (old_data != NULL)
         free( old_data );
      return NULL;
   }

   /* start the linked lists */
   open = closed =  NULL;
   cur = A_newNode( ssys, NULL );
   open = A_add( open, cur ); /* inital open node is the start system */

   j = 0;
   while ((cur = A_lowest(open))->sys != esys) {

      /* Break if infinite loop. */
      j++;
      if (j > MAP_LOOP_PROT)
         break;

      /* get best from open and toss to closed */
      open = A_rm( open, cur->sys );
      closed = A_add( closed, cur );
      cost = A_g(cur) + 1;

      for (i=0; i<cur->sys->njumps; i++) {
         sys = system_getIndex( cur->sys->jumps[i] );

         /* Make sure it's reachable */
         if (!ignore_known &&
               ((!sys_isKnown(sys) && 
                  (!sys_isKnown(cur->sys) || !space_sysReachable(esys)))))
            continue;

         neighbour = A_newNode( sys, NULL );

         ocost = A_in(open, sys);
         if ((ocost != NULL) && (cost < ocost->g)) {
            open = A_rm( open, sys ); /* new path is better */
         }

         ccost = A_in(closed, sys);
         if (ccost != NULL) {
            closed = A_rm( closed, sys ); /* shouldn't happen */
         }
         
         if ((ocost == NULL) && (ccost == NULL)) {
            neighbour->g = cost;
            neighbour->r = A_g(neighbour) + A_h(cur->sys,sys);
            neighbour->parent = cur;
            open = A_add( open, neighbour );
         }
      }
   }

   /* build path backwards if not broken from loop. */
   if (j <= MAP_LOOP_PROT) {
      (*njumps) = A_g(cur);
      if (old_data == NULL)
         res = malloc( sizeof(StarSystem*) * (*njumps) );
      else {
         *njumps = *njumps + ojumps;
         res = realloc( old_data, sizeof(StarSystem*) * (*njumps) );
      }
      for (i=0; i<((*njumps)-ojumps); i++) {
         res[(*njumps)-i-1] = cur->sys;
         cur = cur->parent;
      }
   }
   else {
      (*njumps) = 0;
      res = NULL;
      if (old_data != NULL)
         free( old_data );
   }

   /* free the linked lists */
   A_freeList(A_gc);
   return res;
}


/**
 * @brief Marks maps around a radius of currenty system as known.
 *
 *    @param targ_sys System at center of the "known" circle.
 *    @param r Radius (in jumps) to mark as known.
 *    @return 0 on success.
 */
int map_map( const char* targ_sys, int r )
{
   int i, dep;
   StarSystem *sys, *jsys;
   SysNode *closed, *open, *cur, *neighbour;

   A_gc = NULL;
   open = closed = NULL;

   if (targ_sys == NULL) sys = cur_system;
   else sys = system_get( targ_sys );
   sys_setFlag(sys,SYSTEM_KNOWN);
   open = A_newNode( sys, NULL );
   open->r = 0;

   while ((cur = A_lowest(open)) != NULL) {

      /* mark system as known and go to next */
      sys = cur->sys;
      dep = cur->r;
      sys_setFlag(sys,SYSTEM_KNOWN);
      open = A_rm( open, sys );
      closed = A_add( closed, cur );

      /* check it's jumps */
      for (i=0; i<sys->njumps; i++) {
         jsys = system_getIndex( cur->sys->jumps[i] );

         /* System has already been parsed or is too deep */
         if ((A_in(closed,jsys) != NULL) || (dep+1 > r))
             continue;

         /* create new node and such */
         neighbour = A_newNode( jsys, NULL );
         neighbour->r = dep+1;
         open = A_add( open, neighbour );
      }
   }

   A_freeList(A_gc);
   return 0;
}


/**
 * @brief Check to see if radius is mapped (known).
 *
 *    @param targ_sys Name of the system in the center of the "known" circle.
 *    @param r Radius to check (in jumps) if is mapped.
 *    @return 1 if circle was already mapped, 0 if it wasn't.
 */
int map_isMapped( const char* targ_sys, int r )
{
   int i, dep, ret;
   StarSystem *sys, *jsys;
   SysNode *closed, *open, *cur, *neighbour;

   A_gc = NULL;
   open = closed = NULL;

   if (targ_sys == NULL)
      sys = cur_system;
   else
      sys = system_get( targ_sys );
   open     = A_newNode( sys, NULL );
   open->r  = 0;
   ret      = 1;

   while ((cur = A_lowest(open)) != NULL) {

      /* Check if system is known. */
      sys      = cur->sys;
      dep      = cur->r;
      if (!sys_isFlag(sys,SYSTEM_KNOWN)) {
         ret = 0;
         break;
      }

      /* We close the current system. */
      open     = A_rm( open, sys );
      closed   = A_add( closed, cur );

      /* System is past the limit. */
      if (dep+1 > r)
         continue;

      /* check it's jumps */
      for (i=0; i<sys->njumps; i++) {
         jsys = system_getIndex( sys->jumps[i] );
        
         /* SYstem has already been parsed. */
         if (A_in(closed,jsys) != NULL)
             continue;

         /* create new node and such */
         neighbour      = A_newNode( jsys, NULL );
         neighbour->r   = dep+1;
         open           = A_add( open, neighbour );
      }
   }

   A_freeList(A_gc);
   return ret;
}


/**
 * @brief Shows a map at x, y (relative to wid) with size w,h.
 *
 *    @param wid Window to show map on.
 *    @param x X position to put map at.
 *    @param y Y position to put map at.
 *    @param w Width of map to open.
 *    @param h Height of map to open.
 *    @param zoom Default zoom to use.
 */
void map_show( int wid, int x, int y, int w, int h, double zoom )
{
   StarSystem *sys;

   /* mark systems as needed */
   mission_sysMark();

   /* Set position to focus on current system. */
   map_xpos = cur_system->pos.x * zoom;
   map_ypos = cur_system->pos.y * zoom;

   /* Set zoom. */
   map_setZoom(zoom);

   /* Make sure selected is sane. */
   sys = system_getIndex( map_selected );
   if (!(sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)) &&
         !sys_isKnown(sys) && !space_sysReachable(sys))
      map_selectCur();

   window_addCust( wid, x, y, w, h,
         "cstMap", 1, map_render, map_mouse, NULL );
}


/**
 * @brief Centers the map on a planet.
 *
 *    @param sys System to center the map on.
 *    @return 0 on success.
 */
int map_center( const char *sys )
{
   StarSystem *ssys;

   /* Get the system. */
   ssys = system_get( sys );
   if (sys == NULL)
      return -1;

   /* Center on the system. */
   map_xpos = ssys->pos.x * map_zoom;
   map_ypos = ssys->pos.y * map_zoom;

   return 0;
}


