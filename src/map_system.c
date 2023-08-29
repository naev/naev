/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "map.h"

#include "array.h"
#include "background.h"
#include "colour.h"
#include "dialogue.h"
#include "economy.h"
#include "faction.h"
#include "gui.h"
#include "land_outfits.h"
#include "log.h"
#include "mapData.h"
#include "map_find.h"
#include "map_system.h"
#include "mission.h"
#include "ndata.h"
#include "nmath.h"
#include "nstring.h"
#include "opengl.h"
#include "player.h"
#include "space.h"
#include "toolkit.h"

#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */

static StarSystem *cur_sys_sel = NULL; /**< Currently selected system */
static int cur_spob_sel = 0; /**< Current spob selected by user (0 = star). */
static Spob *cur_spobObj_sel = NULL;
static Outfit **cur_spob_sel_outfits = NULL;
static Ship **cur_spob_sel_ships = NULL;
static int pitch = 0; /**< pitch of spob images. */
static int nameWidth = 0; /**< text width of spob name */
static int nshow = 0; /**< number of spobs shown. */
static char infobuf[STRMAX];
static unsigned int starCnt = 1;
glTexture **bgImages; /**< array (array.h) of nebula and star textures */

#define MAP_SYSTEM_WDWNAME "wdwSystemMap"
#define MAPSYS_OUTFITS "mapSysOutfits"
#define MAPSYS_SHIPS "mapSysShips"
#define MAPSYS_TRADE "mapSysTrade"

/*
 * extern
 */
/*land.c*/
//extern int landed;
//extern Spob* land_spob;

/*
 * prototypes
 */
void map_system_cleanup( unsigned int wid, const char *str );
int map_system_isOpen( void );

/* Update. */
void map_system_updateSelected( unsigned int wid );

/* Render. */
static void map_system_render( double bx, double by, double w, double h, void *data );
/* Mouse. */
static int map_system_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, double rx, double ry, void *data );
/* Misc. */
static int map_system_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod, int isrepeat );
void map_system_show( int wid, int x, int y, int w, int h);

static void map_system_genOutfitsList( unsigned int wid, float goodsSpace, float outfitSpace, float shipSpace );
static void map_system_genShipsList( unsigned int wid, float goodsSpace, float outfitSpace, float shipSpace );
static void map_system_genTradeList( unsigned int wid, float goodsSpace, float outfitSpace, float shipSpace );

static void map_system_array_update( unsigned int wid, const char* str );

void map_system_buyCommodPrice( unsigned int wid, const char *str );

/**
 * @brief Initializes the system map subsystem.
 *
 *    @return 0 on success.
 */
int map_system_init( void )
{
   return 0;
}

/**
 * @brief Placemarker for when required for loading system map subsystem.
 *
 *    @return 0 on success.
 */
int map_system_load( void )
{
   return 0;
}

/**
 * @brief Destroys the system map subsystem.
 */
void map_system_exit( void )
{
   for (int i=0; i<array_size(bgImages); i++)
      gl_freeTexture( bgImages[i] );
   array_free( bgImages );
   bgImages=NULL;
}

/**
 * @brief Close the map window.
 */
void map_system_cleanup( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;

   if (cur_sys_sel != cur_system)
     space_gfxUnload( cur_sys_sel );

   for (int i=0; i<array_size(bgImages); i++)
      gl_freeTexture( bgImages[i] );
   array_free( bgImages );
   bgImages = NULL;
   array_free( cur_spob_sel_outfits );
   cur_spob_sel_outfits = NULL;
   array_free( cur_spob_sel_ships );
   cur_spob_sel_ships = NULL;
}

/**
 * @brief Handles key input to the map window.
 */
static int map_system_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod, int isrepeat )
{
   (void) mod;
   (void) isrepeat;
   if (key == SDLK_m) {
      window_close( wid, NULL );
      return 1;
   }
   if (key == SDLK_UP) {
      cur_spob_sel = MAX( cur_spob_sel-1, 0 );
      map_system_updateSelected( wid );
      return 1;
   }
   if (key == SDLK_DOWN) {
      cur_spob_sel = MIN( cur_spob_sel+1, nshow );
      map_system_updateSelected( wid );
      return 1;
   }
   return 0;
}

/**
 * @brief Opens the map window.
 */
void map_system_open( int sys_selected )
{
   unsigned int wid;
   int w, h;
   StarSystem *tmp_sys;
   /* Destroy window if exists. */
   wid = window_get( MAP_SYSTEM_WDWNAME );
   if ( wid > 0 ) {
      window_destroy( wid );
      return;
   }
   cur_spobObj_sel = NULL;
   memset( infobuf,0,sizeof(infobuf) );
   pitch = 0;
   nameWidth = 0;
   nshow = 0;
   starCnt = 1;

   /* get the selected system. */
   cur_sys_sel = system_getIndex( sys_selected );
   cur_spob_sel = 0;
   /* Set up window size. */
   w = MAX(600, SCREEN_W - 140);
   h = MAX(540, SCREEN_H - 140);

   /* create the window. */
   wid = window_create( MAP_SYSTEM_WDWNAME, _("System Info"), -1, -1, w, h );
   window_setCancel( wid, window_close );
   window_onCleanup( wid, map_system_cleanup );
   window_handleKeys( wid, map_system_keyHandler );
   window_addText( wid, 40, h-30, 160, 20, 1, "txtSysname",
         &gl_defFont, &cFontGreen, _(cur_sys_sel->name) );
   window_addImage( wid, -90 + 32, h-30, 0, 0, "imgFaction", NULL, 0 );
   /* Close button */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnClose", _("Close"), window_close );
   /* commodity price purchase button */
   window_addButton( wid, -40-BUTTON_WIDTH, 20, BUTTON_WIDTH*3, BUTTON_HEIGHT,
                     "btnBuyCommodPrice", _("Buy commodity price info"), map_system_buyCommodPrice );
   window_disableButton( wid, "btnBuyCommodPrice");

   /* Load the spob gfx if necessary */
   if (cur_sys_sel != cur_system)
     space_gfxLoad( cur_sys_sel );
   /* get textures for the stars.  The first will be the nebula */
   /* There seems no other reliable way of getting the correct images -*/
   /* these are determined by a random number generator in lua */
   /* This is a bit nasty - luckily Naev is single threaded! */
   tmp_sys = cur_system;
   cur_system = cur_sys_sel;
   /* load background images */
   background_clear();
   background_load ( cur_system->background );
   bgImages = background_getTextures();
   if ( array_size( bgImages ) <= 1 )
      starCnt = 0;
   background_clear();
   /* and reload the images for the current system */
   cur_system = tmp_sys;
   background_load( cur_system->background );

   map_system_show( wid, 20, 60, w-40, h-100);
   map_system_updateSelected( wid );
}

/**
 * @brief Checks to see if the map is open.
 *
 *    @return 0 if map is closed, non-zero if it's open.
 */
int map_system_isOpen( void)
{
   return window_exists( MAP_SYSTEM_WDWNAME );
}

/**
 * @brief Shows a solar system map at x, y (relative to wid) with size w,h.
 *
 *    @param wid Window to show map on.
 *    @param x X position to put map at.
 *    @param y Y position to put map at.
 *    @param w Width of map to open.
 *    @param h Height of map to open.
 *    @param zoom Default zoom to use.
 */
void map_system_show( int wid, int x, int y, int w, int h)
{
   window_addCust( wid, x, y, w, h,
         "cstMapSys", 1, map_system_render, map_system_mouse, NULL, NULL, NULL );
}

/**
 * @brief Renders the custom solar system map widget.
 *
 * TODO this shouldn't compute all the strings every render frame.
 *
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void map_system_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   int i, vis_index;
   double iw, ih;
   StarSystem *sys = cur_sys_sel;
   Spob *p;
   static int phase=0;
   glColour ccol;
   char buf[STRMAX];
   int cnt;
   double ast_nb, ast_area;
   double f;
   int hasPresence = 0;
   double unknownPresence = 0;
   char t;
   int txtHeight;
   const glTexture *logo;
   int offset;

   phase++;

   if (phase > 150) {
      phase = 0;
      starCnt++;
      if ( starCnt >= (unsigned int) array_size( bgImages ) ) {
         if ( array_size( bgImages ) <= 1)
            starCnt = 0;
         else
            starCnt = 1;
      }
   }
   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   vis_index=0;
   offset = h - pitch*nshow;
   for (i=0; i<array_size(sys->spobs); i++) {
      p = sys->spobs[i];
      if (!spob_isKnown( p ))
         continue;
      vis_index++;
      if (p->gfx_space == NULL)
         WARN( _("No gfx for %s…"),p->name );
      else {
         ih = pitch;
         iw = ih;
         if (p->gfx_space->w > p->gfx_space->h)
            ih = ih * p->gfx_space->h / p->gfx_space->w;
         else if ( p->gfx_space->w < p->gfx_space->h )
            iw = iw * p->gfx_space->w / p->gfx_space->h;
         gl_renderScale( p->gfx_space, bx+(pitch-iw)/2+2, by+(nshow-vis_index-1)*pitch + (pitch-ih)/2 + offset, iw, ih, &cWhite );
      }
      gl_printRaw( &gl_smallFont, bx + 5 + pitch, by + (nshow-vis_index-0.5)*pitch + offset,
            (cur_spob_sel == vis_index ? &cFontGreen : &cFontWhite), -1., spob_name(p) );
   }
   /* draw the star */
   ih = pitch;
   iw = ih;
   if (array_size( bgImages ) > 0) {
      if ( bgImages[starCnt]->w > bgImages[starCnt]->h )
         ih = ih * bgImages[starCnt]->h / bgImages[starCnt]->w;
      else if ( bgImages[starCnt]->w < bgImages[starCnt]->h )
         iw = iw * bgImages[starCnt]->w / bgImages[starCnt]->h;
      ccol.r=ccol.g=ccol.b=ccol.a=1;
      if ( phase > 120 && array_size( bgImages ) > 2 )
         ccol.a = cos ( (phase-121)/30. *M_PI/2.);
      gl_renderScale( bgImages[starCnt], bx+2 , by+(nshow-1)*pitch + (pitch-ih)/2 + offset, iw , ih, &ccol );
      if ( phase > 120 && array_size( bgImages ) > 2) {
         /* fade in the next star */
         ih=pitch;
         iw=ih;
         i = starCnt + 1;
         if ( i >= array_size( bgImages ) ) {
            if ( array_size( bgImages ) <= 1 )
               i=0;
            else
               i=1;
         }
         if ( bgImages[i]->w > bgImages[i]->h )
            ih = ih * bgImages[i]->h / bgImages[i]->w;
         else if ( bgImages[i]->w < bgImages[i]->h )
            iw = iw * bgImages[i]->w / bgImages[i]->h;
         ccol.a = 1 - ccol.a;
         gl_renderScale( bgImages[i], bx+2, by+(nshow-1)*pitch + (pitch-ih)/2 + offset, iw, ih, &ccol );
      }
   }
   else {
      /* no nebula or star images - probably due to nebula */
      txtHeight = gl_printHeightRaw( &gl_smallFont,pitch,_("Obscured by the nebula") );
      gl_printTextRaw( &gl_smallFont, pitch, txtHeight, (bx+2),
            (by + (nshow-0.5)*pitch + offset), 0, &cFontRed, -1., _("Obscured by the nebula") );
   }
   gl_printRaw( &gl_smallFont, bx + 5 + pitch, by + (nshow-0.5)*pitch + offset,
         (cur_spob_sel == 0 ? &cFontGreen : &cFontWhite), -1., _(sys->name) );
   if ((cur_spob_sel==0) && array_size( bgImages ) > 0) {
      /* make use of space to draw a nice nebula */
      double imgw,imgh, s;
      iw = w - 50 - pitch - nameWidth;
      ih = h;
      imgw = bgImages[0]->w;
      imgh = bgImages[0]->h;
      s = MIN( iw / imgw, ih / imgh );
      imgw *= s;
      imgh *= s;
      gl_renderScale( bgImages[0], bx+w-iw+(iw-imgw)*0.5, by+h-ih+(ih-imgh)*0.5, imgw, imgh, &cWhite );
   }
   /* draw marker around currently selected spob */
   ccol.r=0; ccol.g=0.6+0.4*sin( phase/150.*2*M_PI ); ccol.b=0; ccol.a=1;
   ih=15;
   iw=3;
   gl_renderRect( bx+1, by+(nshow-cur_spob_sel-1)*pitch + offset, iw, ih, &ccol );
   gl_renderRect( bx+1, by+(nshow-cur_spob_sel)*pitch-ih + offset, iw, ih, &ccol );
   gl_renderRect( bx+pitch+3-iw, by+(nshow-cur_spob_sel-1)*pitch + offset, iw, ih, &ccol );
   gl_renderRect( bx+pitch+3-iw, by+(nshow-cur_spob_sel)*pitch-ih + offset, iw, ih, &ccol );
   gl_renderRect( bx+1, by+(nshow-cur_spob_sel-1)*pitch + offset, ih, iw, &ccol );
   gl_renderRect( bx+1, by+(nshow-cur_spob_sel)*pitch-iw + offset, ih, iw, &ccol );
   gl_renderRect( bx+pitch+3-ih, by+(nshow-cur_spob_sel-1)*pitch + offset, ih, iw, &ccol );
   gl_renderRect( bx+pitch+3-ih, by+(nshow-cur_spob_sel)*pitch-iw + offset, ih, iw, &ccol );
   cnt=0;
   buf[0]='\0';
   if (cur_spob_sel == 0) {
      int infopos = 0;
      int stars   = MAX( array_size( bgImages )-1, 0 );
      cnt+=scnprintf( &buf[cnt], sizeof(buf)-cnt, _("System: %s\n"), _(sys->name) );
      /* display sun information */
      cnt+=scnprintf( &buf[cnt], sizeof(buf)-cnt, n_("%d-star system\n", "%d-star system\n", stars), stars );

      /* Nebula. */
      if (sys->nebu_density > 0. ) {
         /* Volatility */
         if (sys->nebu_volatility > 700.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Nebula: Volatile, ") );
         else if (sys->nebu_volatility > 300.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Nebula: Dangerous, ") );
         else if (sys->nebu_volatility > 0.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Nebula: Unstable, ") );
         else
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Nebula: Stable, ") );

         /* Density */
         if (sys->nebu_density > 700.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Dense\n") );
         else if (sys->nebu_density < 300.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Light\n") );
         else
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Medium\n") );
      } else
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Nebula: None\n") );

      /* Interference. */
      if (sys->interference > 0. ) {
         if (sys->interference > 700.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Interference: Dense\n") );
         else if (sys->interference < 300.)
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Interference: Light\n") );
      }
      /* Asteroids. */
      if (array_size(sys->asteroids) > 0 ) {
         ast_nb = ast_area = 0.;
         for ( i=0; i<array_size(sys->asteroids); i++ ) {
            ast_nb += sys->asteroids[i].nb;
            ast_area = MAX( ast_area, sys->asteroids[i].area );
         }
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Asteroid field density: %.2g\n"), ast_nb*ASTEROID_REF_AREA/ast_area );
      }
      /* Faction */
      f = -1;
      for (i=0; i<array_size(sys->spobs); i++) {
         if (spob_isKnown( sys->spobs[i] )) {
            if ((f==-1) && (sys->spobs[i]->presence.faction>=0) ) {
               f = sys->spobs[i]->presence.faction;
            } else if (f != sys->spobs[i]->presence.faction &&  (sys->spobs[i]->presence.faction>=0) ) {
               cnt+=scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Faction: Multiple\n") );
               break;
            }
         }
      }
      if (f == -1 ) {
         cnt+=scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Faction: N/A\n") );
      }  else {
         if (i==array_size(sys->spobs)) /* saw them all and all the same */
            cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Faction: %s\nStanding: %s\n"), faction_longname(f), faction_getStandingText( f ) );
         /* display the logo */
         logo = faction_logo( f );
         if ( logo != NULL ) {
            gl_renderScale( logo, bx + pitch + nameWidth + 200,
                  by + h - 21, 20, 20, &cWhite );
         }
      }
      /* Get presence. */
      hasPresence = 0;
      unknownPresence = 0;
      for ( i=0; i < array_size(sys->presence); i++ ) {
         if (sys->presence[i].value <= 0)
            continue;
         hasPresence = 1;
         if ( faction_isKnown( sys->presence[i].faction ) ) {
            t = faction_getColourChar( sys->presence[i].faction );
            cnt += scnprintf( &buf[cnt], sizeof( buf ) - cnt, "#0%s: #%c%.0f\n",
                              faction_shortname( sys->presence[i].faction ),
                              t, sys->presence[i].value);
         } else
            unknownPresence += sys->presence[i].value;
      }
      if (unknownPresence != 0)
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, "#0%s: #%c%.0f\n",
                           _("Unknown"), 'N', unknownPresence );
      if (hasPresence == 0)
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Presence: N/A\n"));
      txtHeight=gl_printHeightRaw(&gl_smallFont,(w - nameWidth-pitch-60)/2,buf);
      gl_printTextRaw( &gl_smallFont, (w - nameWidth - pitch - 60) / 2, txtHeight,
            bx + 10 + pitch + nameWidth, by + h - 10 - txtHeight, 0, &cFontWhite, -1., buf );

      /* Jumps. */
      for (i=0; i<array_size(sys->jumps); i++) {
         if (jp_isUsable ( &sys->jumps[i])) {
            if (infopos == 0) /* First jump */
               infopos = scnprintf( infobuf, sizeof(infobuf), _("   Jump points to:\n") );
            if (sys_isKnown( sys->jumps[i].target ))
               infopos += scnprintf( &infobuf[infopos], sizeof(infobuf)-infopos, "     %s\n", _(sys->jumps[i].target->name) );
            else
               infopos += scnprintf( &infobuf[infopos], sizeof(infobuf)-infopos, _("     Unknown system\n") );
         }
      }
   } else {
      /* display spob info */
      p = cur_spobObj_sel;
      if (p->presence.faction >= 0 ) {/* show the faction */
         char factionBuf[64];
         logo = faction_logo( p->presence.faction );
         if (logo != NULL)
            gl_renderScale( logo, bx + pitch + nameWidth + 200, by + h - 21, 20, 20, &cWhite );

         snprintf( factionBuf, 64, "%s", faction_shortname( p->presence.faction ) );
         gl_printTextRaw( &gl_smallFont, (w - nameWidth-pitch - 60) / 2, 20,
               bx+pitch+nameWidth + 230, by + h - 31, 0, &cFontWhite, -1., factionBuf );
      }

      cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("Spob: %s\nPlanetary class: %s    Population: roughly %s\n"), spob_name(p), _(p->class), space_populationStr( p->population ) );
      if (!spob_hasService( p, SPOB_SERVICE_INHABITED ))
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, _("No space port here\n") );
      else if (p->can_land)
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, "#g%s#0", _("You can land here\n") );
      else if (areEnemies( FACTION_PLAYER, p->presence.faction))
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, "#o%s#0", _("Not advisable to land here\n") );
      else
         cnt += scnprintf( &buf[cnt], sizeof(buf)-cnt, "#r%s#0", _("You cannot land here\n") );

      if (infobuf[0]=='\0') {
         int infocnt = 0;

         /* Add a description */
         infocnt += scnprintf( &infobuf[infocnt], sizeof(infobuf)-infocnt, "%s\n\n", (p->description==NULL?_("No description available"):_(p->description)) );

         /* show some additional information */
         infocnt += scnprintf( &infobuf[infocnt], sizeof(infobuf)-infocnt, "%s\n"
               "%s\n%s%s%s%s",
               /* Redundant information. */
               //spob_hasService( p, SPOB_SERVICE_LAND) ? _("This system is landable") : _("This system is not landable"),
               //spob_hasService( p, SPOB_SERVICE_INHABITED) ? _("This system is inhabited") : _("This system is not inhabited"),
               spob_hasService( p, SPOB_SERVICE_REFUEL) ? _("You can refuel here") : _("You cannot refuel here"),
               spob_hasService( p, SPOB_SERVICE_BAR) ? _("This system has a bar") : _("This system does not have a bar"),
               spob_hasService( p, SPOB_SERVICE_MISSIONS) ? _("This system offers missions") : _("This system does not offer missions"),
               spob_hasService( p, SPOB_SERVICE_COMMODITY) ? "" : _("\nThis system does not have a trade outlet"),
               spob_hasService( p, SPOB_SERVICE_OUTFITS) ? "" : _("\nThis system does not sell ship equipment"),
               spob_hasService( p, SPOB_SERVICE_SHIPYARD) ? "" : _("\nThis system does not sell ships"));
         //if (p->bar_description && spob_hasService( p, SPOB_SERVICE_BAR ))
         //   infocnt += scnprintf( &infobuf[infocnt], sizeof(infobuf)-infocnt, "\n\n%s", _(p->bar_description) );
      }

      txtHeight = gl_printHeightRaw( &gl_smallFont, (w - nameWidth-pitch-60)/2, buf );

      gl_printTextRaw( &gl_smallFont, (w - nameWidth - pitch - 60) / 2, txtHeight,
            bx + 10 + pitch + nameWidth, by + h - 10 - txtHeight, 0, &cFontWhite, -1., buf );
   }

   /* show the trade/outfit/ship info */
   if (infobuf[0]!='\0') {
      txtHeight = gl_printHeightRaw( &gl_smallFont, (w - nameWidth-pitch-60)/2, infobuf );
      gl_printTextRaw( &gl_smallFont, (w - nameWidth - pitch - 60) / 2, txtHeight,
            bx + 10 + pitch + nameWidth, by + 10, 0, &cFontGrey, -1., infobuf );
   }
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
static int map_system_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, double rx, double ry, void *data )
{
   (void) data;
   (void) rx;
   (void) ry;

   switch (event->type) {
      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;
         if (mx < pitch && my > 0) {
            if (cur_spob_sel != (h-my) / pitch) {
               cur_spob_sel = ( h-my) / pitch;
               map_system_updateSelected( wid );
            }
            return 1;
         }
         break;
   }
   return 0;
}

static void map_system_array_update( unsigned int wid, const char* str )
{
   int i;
   Ship *ship;
   char buf_price[ECON_CRED_STRLEN], buf_license[STRMAX_SHORT], buf_mass[ECON_MASS_STRLEN];
   size_t l = 0;

   infobuf[0] = '\0';
   i = toolkit_getImageArrayPos( wid, str );
   if (i < 0)
      return;
   if ((strcmp( str, MAPSYS_OUTFITS ) == 0)) {
      Outfit *outfit = cur_spob_sel_outfits[i];
      double mass = outfit->mass;

      /* new text */
      price2str( buf_price, outfit->price, player.p->credits, 2 );
      if (outfit->license == NULL)
         buf_license[0] = '\0';
      else if (player_hasLicense( outfit->license ) ||
            (cur_spobObj_sel != NULL && spob_hasService( cur_spobObj_sel, SPOB_SERVICE_BLACKMARKET )))
         strncpy( buf_license, _(outfit->license), sizeof(buf_license)-1 );
      else
         snprintf( buf_license, sizeof( buf_license ), "#r%s#0", _(outfit->license) );

      if (outfit_isLauncher(outfit))
         mass += outfit_amount( outfit ) * outfit->u.lau.ammo_mass;
      else if (outfit_isFighterBay(outfit))
         mass += outfit_amount( outfit ) * outfit->u.bay.ship_mass;
      snprintf( buf_mass, sizeof(buf_mass), n_( "%d t", "%d t", (int)round( mass ) ), (int)round( mass ) );

      l += outfit_getNameWithClass( outfit, &infobuf[l], sizeof(infobuf)-l );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n%s\n\n", pilot_outfitDescription( player.p, outfit ) );

      /* FIXME: The point of this misery is to split desc_short into a 2-column layout.
       * It works poorly, but if we don't do this, check out e.g. the TeraCom Medusa Launcher in a 720p window. */
      char *desc_start = &infobuf[l];
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "%s\n\n", pilot_outfitSummary( player.p, outfit, 0 ) );
      while ( (desc_start = strchr( desc_start, '\n' )) != NULL ) {
         char *tab_pos = desc_start;
         desc_start = strchr( &tab_pos[1], '\n' );
         if (desc_start == NULL)
            break;
         *tab_pos = '\t';
         desc_start++;
      }

      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "#n%s#0 %d   ", _("Owned:"), player_outfitOwned( outfit ) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "#n%s#0 %s   ", _("Mass:"), buf_mass );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "#n%s#0 %s   ", _("Price:"), buf_price );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "%s", buf_license );
   }
   else if ((strcmp( str, MAPSYS_SHIPS ) == 0)) {
      char buf_cargo[ECON_MASS_STRLEN];
      ship = cur_spob_sel_ships[i];

   /* update text */
      price2str( buf_price, ship_buyPrice( ship ), player.p->credits, 2 );
      tonnes2str( buf_mass, ship->mass );
      tonnes2str( buf_cargo, ship->cap_cargo );
      if (ship->license == NULL)
         strncpy( buf_license, _("None"), sizeof(buf_license)-1 );
      else if (player_hasLicense( ship->license )
            || (cur_spobObj_sel != NULL && spob_hasService( cur_spobObj_sel, SPOB_SERVICE_BLACKMARKET )))
         strncpy( buf_license, _(ship->license), sizeof(buf_license)-1 );
      else
         snprintf( buf_license, sizeof(buf_license), "#r%s#0", _(ship->license) );

      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "#n%s#0 %s", _("Model:"), _(ship->name) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "    #n%s#0 %s", _("Class:"), _(ship_classDisplay(ship)) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n\n%s\n", _(ship->description) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 %s", _("Fabricator:"), _(ship->fabricator) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "    #n%s#0 %d", _("Crew:"), ship->crew );
      /* Weapons & Manoeuvrability */
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 %.0f %s", _("CPU:"), ship->cpu, n_( "teraflop", "teraflops", ship->cpu ) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "    #n%s#0 %s", _("Mass:"), buf_mass );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 ", _("Thrust:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f kN/tonne"), ship->thrust );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "    #n%s#0 ", _("Speed:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f m/s"), ship->speed );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 ", _("Turn:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f deg/s"), ship->turn*180./M_PI );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "    #n%s#0 %.0f%%", _("Time Constant:"), ship->dt_default*100. );
      /* Misc */
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 ", _("Absorption:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f%% damage"), ship->dmg_absorb*100. );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 ", _("Shield:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f MJ (%.1f MW)"), ship->shield, ship->shield_regen );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "    #n%s#0 ", _("Armour:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f MJ (%.1f MW)"), ship->armour, ship->armour_regen );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 ", _("Energy:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%.0f MJ (%.1f MW)"), ship->energy, ship->energy_regen );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 %s", _("Cargo Space:"), buf_cargo );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 %d %s", _("Fuel:"), ship->fuel, n_( "unit", "units", ship->fuel ) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "  #n%s#0 %d %s", _("Fuel Use:"),
         ship->fuel_consumption, n_( "unit", "units", ship->fuel_consumption ) );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n#n%s#0 %s", _("Price:"), buf_price );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "  #n%s#0 %s", _("License:"), buf_license );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n%s", ship->desc_stats );
   }
   else if ((strcmp( str, MAPSYS_TRADE ) == 0)) {
      Commodity *com;
      credits_t mean;
      double std;
      credits_t globalmean;
      double globalstd;
      char buf_mean[ECON_CRED_STRLEN], buf_globalmean[ECON_CRED_STRLEN];
      char buf_std[ECON_CRED_STRLEN], buf_globalstd[ECON_CRED_STRLEN];
      char buf_buy_price[ECON_CRED_STRLEN];
      int owned;
      com = cur_spobObj_sel->commodities[i];
      economy_getAveragePrice( com, &globalmean, &globalstd );
      economy_getAverageSpobPrice( com, cur_spobObj_sel, &mean, &std );
      credits2str( buf_mean, mean, -1 );
      snprintf( buf_std, sizeof(buf_std), "%.1f ¤", std ); /* TODO credit2str could learn to do this... */
      credits2str( buf_globalmean, globalmean, -1 );
      snprintf( buf_globalstd, sizeof(buf_globalstd), "%.1f ¤", globalstd ); /* TODO credit2str could learn to do this... */
      owned=pilot_cargoOwned( player.p, com );

      l = scnprintf( infobuf, sizeof(infobuf)-l, "%s\n\n%s\n\n", _(com->name), _(com->description) );

      if ( owned > 0 ) {
         credits2str( buf_buy_price, com->lastPurchasePrice, -1 );
         l += scnprintf( &infobuf[l], sizeof(infobuf)-l, n_(
                  "#nYou have:#0 %d tonne, purchased at %s/t\n",
                  "#nYou have:#0 %d tonnes, purchased at %s/t\n",
                  owned), owned, buf_buy_price );
      }
      else
         l += scnprintf( &infobuf[l], sizeof(infobuf)-l, n_(
                  "#nYou have:#0 %d tonne\n",
                  "#nYou have:#0 %d tonnes\n",
                  owned), owned );

      l += scnprintf( &infobuf[l], sizeof(infobuf)-l,"#n%s#0 ", _("Average price seen here:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%s/t ± %s/t"), buf_mean, buf_std );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l,"\n#n%s#0 ", _("Average price seen everywhere:") );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, _("%s/t ± %s/t"), buf_globalmean, buf_globalstd );
      l += scnprintf( &infobuf[l], sizeof(infobuf)-l, "\n%s", "" );
   }
   else
      WARN( _("Unexpected call to map_system_array_update\n") );
}

void map_system_updateSelected( unsigned int wid )
{
   StarSystem *sys=cur_sys_sel;
   Spob *last=NULL;
   int spobObjChanged = 0;
   int w, h;
   Spob *p;
   int textw;
   int noutfits,nships,ngoods;
   Outfit **outfits;
   Ship **ships;
   float g,o,s;
   nameWidth = 0; /* get the widest spob/star name */
   nshow=1;/* start at 1 for the sun*/
   infobuf[0] = '\0'; /* clear buffer. */
   for (int i=0; i<array_size(sys->spobs); i++) {
      p = sys->spobs[i];
      if (spob_isKnown( p )) {
         textw = gl_printWidthRaw( &gl_smallFont, spob_name(p) );
         if ( textw > nameWidth )
            nameWidth = textw;
         last = p;
         if ( cur_spob_sel == nshow ) {
            if ( cur_spobObj_sel != p )
               spobObjChanged = 1;
            cur_spobObj_sel = p;
         }
         nshow++;
      }
   }
   /* get width of star name text */
   textw = gl_printWidthRaw( &gl_smallFont, _(sys->name) );
   if ( textw > nameWidth )
      nameWidth = textw;

   window_dimWindow( wid, &w, &h );

   pitch = (h-100) / nshow;
   if ( pitch > w/5 )
      pitch = w/5;

   if ( cur_spob_sel >= nshow ) {
      cur_spob_sel = nshow-1;
      if ( cur_spobObj_sel != last ) {
         cur_spobObj_sel = last;
         spobObjChanged = 1;
      }
   }
   if ( cur_spob_sel <= 0 ) {
      /* star selected */
      cur_spob_sel = 0;
      if ( cur_spobObj_sel != NULL ) {
         cur_spobObj_sel = NULL;
         spobObjChanged = 1;
      }
   }

   if ( spobObjChanged ) {
      infobuf[0]='\0';
      if ( cur_spobObj_sel == NULL ) {
         /*The star*/
         noutfits = 0;
         nships = 0;
         ngoods = 0;
         window_disableButton( wid, "btnBuyCommodPrice" );
      } else {
         /* get number of each to decide how much space the lists can have */
         outfits = tech_getOutfit( cur_spobObj_sel->tech );
         noutfits = array_size( outfits );
         array_free( outfits );
         ships = tech_getShip( cur_spobObj_sel->tech );
         nships = array_size( ships );
         array_free( ships );
         ngoods = array_size( cur_spobObj_sel->commodities );
         /* to buy commodity info, need to be landed, and the selected system must sell them! */
         if ( landed && spob_hasService( cur_spobObj_sel, SPOB_SERVICE_COMMODITY ) )
            window_enableButton( wid, "btnBuyCommodPrice" );
         else
            window_disableButton( wid, "btnBuyCommodPrice" );
      }
      /* determine the ratio of space */
      s=g=o=0;
      if ( ngoods != 0 )
         g=0.35;

      if ( noutfits != 0 ) {
         if ( nships != 0 ) {
            s=0.25;
            o=1-g-s;
         } else
            o=1-g;
      } else if ( nships!=0 )
         s=1-g;
      /* ensure total is ~1 */
      g += 1 - g - o - s;
      map_system_genOutfitsList( wid, g, o, s );
      map_system_genShipsList( wid, g, o, s );
      map_system_genTradeList( wid, g, o, s );
   }
}

/**
 * @brief Generates the outfit list.
 *
 *    @param wid Window to generate the list on.
 */
static void map_system_genOutfitsList( unsigned int wid, float goodsSpace, float outfitSpace, float shipSpace )
{
   int i;
   ImageArrayCell *coutfits;
   int noutfits;
   int w, h;
   int xpos, xw, ypos, yh;
   int iconsize;
   static Spob *spobDone = NULL;

   window_dimWindow( wid, &w, &h );
   if (spobDone == cur_spobObj_sel) {
      if (widget_exists( wid, MAPSYS_OUTFITS ))
         return;
   } else {
      if (widget_exists( wid, MAPSYS_OUTFITS )) {
         window_destroyWidget( wid, MAPSYS_OUTFITS );
      }
   }
   spobDone = cur_spobObj_sel;

   /* Clean up array if exists. */
   array_free( cur_spob_sel_outfits );
   cur_spob_sel_outfits = NULL;

   /* set up the outfits to buy/sell */
   if (cur_spobObj_sel == NULL)
      return;

   /* No outfitter. */
   if (!spob_hasService( cur_spobObj_sel, SPOB_SERVICE_OUTFITS ))
      return;

   cur_spob_sel_outfits = tech_getOutfit( cur_spobObj_sel->tech );
   noutfits = array_size( cur_spob_sel_outfits );

   if (noutfits <= 0)
      return;
   coutfits = outfits_imageArrayCells( (const Outfit**)cur_spob_sel_outfits, &noutfits, player.p );

   xw = ( w - nameWidth - pitch - 60 ) / 2;
   xpos = 35 + pitch + nameWidth + xw;
   i = (goodsSpace!=0) + (outfitSpace!=0) + (shipSpace!=0);
   yh = (h - 100 - (i+1)*5 ) * outfitSpace;
   ypos = 65 + 5*(shipSpace!=0) + (h - 100 - (i+1)*5)*shipSpace;

   iconsize = 64;
   if (toolkit_simImageArrayVisibleElements( xw, yh, iconsize, iconsize ) < noutfits)
      iconsize = 48;
   window_addImageArray( wid, xpos, ypos,
         xw, yh, MAPSYS_OUTFITS, iconsize, iconsize,
         coutfits, noutfits, map_system_array_update, NULL, NULL );
   toolkit_unsetSelection( wid, MAPSYS_OUTFITS );
}

static void map_system_genShipsList( unsigned int wid, float goodsSpace, float outfitSpace, float shipSpace )
{
   ImageArrayCell *cships;
   int nships;
   int xpos, ypos, xw, yh;
   static Spob *spobDone=NULL;
   int i, w, h, iconsize;
   window_dimWindow( wid, &w, &h );

   /* set up the ships that can be bought here */
   if (spobDone == cur_spobObj_sel) {
      if (widget_exists( wid, MAPSYS_SHIPS ))
         return;
   }
   else {
      if (widget_exists( wid, MAPSYS_SHIPS )) {
         window_destroyWidget( wid, MAPSYS_SHIPS );
         array_free( cur_spob_sel_ships );
         cur_spob_sel_ships = NULL;
      }
      assert(cur_spob_sel_ships == NULL);
   }
   spobDone = cur_spobObj_sel;

   /* set up the outfits to buy/sell */
   if (cur_spobObj_sel == NULL)
      return;

   /* No shipyard. */
   if (!spob_hasService( cur_spobObj_sel, SPOB_SERVICE_SHIPYARD ))
      return;

   cur_spob_sel_ships = tech_getShip( cur_spobObj_sel->tech );
   nships = array_size( cur_spob_sel_ships );

   if (nships <= 0)
      return;

   cships = calloc( nships, sizeof(ImageArrayCell) );
   for ( i=0; i<nships; i++ ) {
      cships[i].image = gl_dupTexture( cur_spob_sel_ships[i]->gfx_store );
      cships[i].caption = strdup( _(cur_spob_sel_ships[i]->name) );
   }
   xw = (w - nameWidth - pitch - 60)/2;
   xpos = 35 + pitch + nameWidth + xw;
   i = (goodsSpace!=0) + (outfitSpace!=0) + (shipSpace!=0);
   yh = (h - 100 - (i+1)*5 ) * shipSpace;
   ypos = 65;

   iconsize = 48;
   if (toolkit_simImageArrayVisibleElements( xw, yh, iconsize, iconsize ) < nships )
      iconsize = 48;
   window_addImageArray( wid, xpos, ypos,
      xw, yh, MAPSYS_SHIPS, iconsize, iconsize,
      cships, nships, map_system_array_update, NULL, NULL );
   toolkit_unsetSelection( wid, MAPSYS_SHIPS );
}

static void map_system_genTradeList( unsigned int wid, float goodsSpace, float outfitSpace, float shipSpace )
{
   static Spob *spobDone=NULL;
   int i, ngoods;
   ImageArrayCell *cgoods;
   int xpos, ypos, xw, yh, w, h, iconsize;
   window_dimWindow( wid, &w, &h );

   /* set up the commodities that can be bought here */
   if ( spobDone == cur_spobObj_sel ) {
      if ( widget_exists( wid, MAPSYS_TRADE ) ) {
         return;
      }
   } else {
      if ( widget_exists( wid, MAPSYS_TRADE ) ) {
         window_destroyWidget( wid, MAPSYS_TRADE );
      }
   }

   /* goods list */
   if (cur_spobObj_sel == NULL)
      return;

   /* No shipyard. */
   if (!spob_hasService( cur_spobObj_sel, SPOB_SERVICE_COMMODITY ))
      return;

   spobDone = cur_spobObj_sel;

   ngoods = array_size( cur_spobObj_sel->commodities );

   if (ngoods <= 0)
      return;
   cgoods = calloc( ngoods, sizeof(ImageArrayCell) );
   for ( i=0; i<ngoods; i++ ) {
      cgoods[i].image = gl_dupTexture( cur_spobObj_sel->commodities[i]->gfx_store );
      cgoods[i].caption = strdup( _(cur_spobObj_sel->commodities[i]->name) );
   }
   /* set up the goods to buy/sell */
   xw = (w - nameWidth - pitch - 60)/2;
   xpos = 35 + pitch + nameWidth + xw;
   i = (goodsSpace!=0) + (outfitSpace!=0) + (shipSpace!=0);
   yh = (h - 100 - (i+1)*5 ) * goodsSpace;
   ypos = 60 + 5*i + (h-100 - (i+1)*5 )*(outfitSpace + shipSpace);

   iconsize = 48;
   if (toolkit_simImageArrayVisibleElements( xw, yh, iconsize, iconsize ) < ngoods )
      iconsize = 48;
   window_addImageArray( wid, xpos, ypos,
      xw, yh, MAPSYS_TRADE, iconsize, iconsize,
      cgoods, ngoods, map_system_array_update, NULL, NULL );
   toolkit_unsetSelection( wid, MAPSYS_TRADE );
}

/**
 * @brief Handles the button to buy commodity prices
 */
void map_system_buyCommodPrice( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;
   int njumps = 0;
   StarSystem **syslist;
   int cost;
   char coststr[ECON_CRED_STRLEN];
   ntime_t t = ntime_get();

   /* find number of jumps */
   if ((strcmp( cur_system->name, cur_sys_sel->name ) == 0)) {
      cost = 500;
      njumps = 0;
   }
   else {
      syslist=map_getJumpPath( cur_system->name, NULL, cur_sys_sel->name, 1, 0, NULL, NULL);
      if ( syslist == NULL ) {
         /* no route */
         dialogue_msg( _("Unavailable"), _("Commodity prices for %s are not available here at the moment."), _(cur_spobObj_sel->name) );
         return;
      } else {
         cost = 500 + 300 * array_size( syslist );
         array_free ( syslist );
      }
   }

   /* get the time at which this purchase will be made (2 periods per jump ago)*/
   t -= ( njumps * 2 + 0.2 ) * NT_PERIOD_SECONDS * 1000;
   credits2str( coststr, cost, -1 );
   if (!player_hasCredits( cost ))
      dialogue_msg( _("Insufficient Credits"), _("You need %s to purchase this information."), coststr );
   else if (array_size( cur_spobObj_sel->commodities ) == 0)
      dialogue_msgRaw( _("No commodities sold here"),_("There are no commodities sold here."));
   else if ( cur_spobObj_sel->commodityPrice[0].updateTime >= t )
      dialogue_msgRaw( _("Already Up-to-date"), _("You have newer information that what is available.") );
   else {
      int ret = dialogue_YesNo( _("Purchase commodity prices?"), _("Purchase %g period old pricing information for %s for %s?"),
            njumps*2+0.2, _(cur_spobObj_sel->name), coststr );
      if (ret) {
         player_modCredits( -cost );
         economy_averageSeenPricesAtTime( cur_spobObj_sel, t );
         map_system_array_update( wid,  MAPSYS_TRADE );
      }
   }
}
