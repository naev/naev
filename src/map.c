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

#include "conf.h"
#include "array.h"
#include "colour.h"
#include "dialogue.h"
#include "faction.h"
#include "gui.h"
#include "log.h"
#include "mapData.h"
#include "map_find.h"
#include "map_system.h"
#include "mission.h"
#include "nebula.h"
#include "ndata.h"
#include "nmath.h"
#include "nstring.h"
#include "nxml.h"
#include "opengl.h"
#include "player.h"
#include "space.h"
#include "toolkit.h"
#include "utf8.h"

#define BUTTON_WIDTH    100 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */
#define MAP_LOOP_PROT   1000 /**< Number of iterations max in pathfinding before aborting. */
#define MAP_TEXT_INDENT   45 /**< Indentation of the text below the titles. */
#define MAP_MARKER_CYCLE  750 /**< Time of a mission marker's animation cycle in milliseconds. */
#define MAP_MOVE_THRESHOLD 20. /**< Mouse movement distance threshold */
#define EASE_ALPHA   ease_QuadraticInOut /**< Ease function for alpha. */

static const int RCOL_X = -10;         /**< Position of text in the right column. */
static const int RCOL_TEXT_W = 135;    /**< Width of normal text in the right column. */
static const int RCOL_HEADER_W = 140;  /**< Width of the header text in the right column. */
//static const int RCOL_W = RCOL_HEADER_W - RCOL_X*2; /**< Real width of the right column. */
/* Below is a hack because older GCC claims the above line is not constant... */
static const int RCOL_W = 140 - (-10*2);
static const int BBAR_H = 60; /**< Height of the bottom bar. */

/**
 * @brief Faction presence container to be used for the map information stuff.
 */
typedef struct FactionPresence_ {
   const char *name; /**< Name of the faction with presence. */
   double value;     /**< Value of the presence. */
   int known;        /**< Whether or not the faction is known. */
} FactionPresence;

/**
 * @brief Map widget data.
 */
typedef struct CstMapWidget_ {
   double xoff;            /**< X offset for centering. */
   double yoff;            /**< Y offset for centering. */
   double zoom;            /**< Level of zoom. */
   double xpos;            /**< Centered x position. */
   double ypos;            /**< Centered y position. */
   double xtarget;         /**< Target X position. */
   double ytarget;         /**< Target Y position. */
   int drag;               /**< Is the user dragging the map? */
   double alpha_decorators;/**< Alpha for decorators. */
   double alpha_faction;   /**< Alpha for factions. */
   double alpha_env;       /**< Alpha for environmental stuff. */
   double alpha_path;      /**< Alpha for path stuff. */
   double alpha_names;     /**< Alpha for system names. */
   double alpha_commod;    /**< Alpha for commodity prices. */
   double alpha_markers;   /**< Alpha for system markers. */
   MapMode mode;           /**< Default map mode. */
} CstMapWidget;

/* map decorator stack */
static MapDecorator* decorator_stack = NULL; /**< Contains all the map decorators. */

static int map_selected       = -1;     /**< What system is selected on the map. */
static MapMode map_mode       = MAPMODE_TRAVEL; /**< Default map mode. */
static StarSystem **map_path  = NULL;   /**< Array (array.h): The path to current selected system. */
static int cur_commod         = -1;     /**< Current commodity selected. */
static int cur_commod_mode    = 0;      /**< 0 for cost, 1 for difference. */
static Commodity **commod_known = NULL; /**< index of known commodities */
static char** map_modes       = NULL;   /**< Array (array.h) of the map modes' names, e.g. "Gold: Cost". */
static int listMapModeVisible = 0;      /**< Whether the map mode list widget is visible. */
static double commod_av_gal_price = 0;  /**< Average price across the galaxy. */
static double map_dt          = 0.;     /**< Nebula animation stuff. */
static int map_minimal_mode   = 0;      /**< Map is in minimal mode. */
static double map_flyto_speed = 1500.;  /**< Linear speeed at which the map flies to a location. */
static double map_mx          = 0.;     /**< X mouse position */
static double map_my          = 0.;     /**< Y mouse position */
static char map_show_notes    = 0;      /**< Boolean for showing system notes */

/*
 * extern
 */
/* space.c */
extern StarSystem *systems_stack;

/*land.c*/
extern int landed;
extern Spob* land_spob;

/*
 * prototypes
 */
/* Update. */
static void map_update_autonav( unsigned int wid );
static void map_update_status( unsigned int wid, const char *buf );
static void map_update( unsigned int wid );
/* Render. */
static void map_render( double bx, double by, double w, double h, void *data );
static void map_renderPath( double x, double y, double zoom, double radius, double alpha );
static void map_renderMarkers( double x, double y, double zoom, double r, double a );
static void map_renderCommod( double bx, double by, double x, double y,
                              double zoom, double w, double h, double r, int editor, double a );
static void map_renderCommodIgnorance( double x, double y, double zoom,
      const StarSystem *sys, const Commodity *c, double a );
static void map_drawMarker( double x, double y, double zoom,
      double r, double a, int num, int cur, int type );
/* Mouse. */
static void map_focusLose( unsigned int wid, const char* wgtname );
static int map_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, double rx, double ry, void *data );
/* Misc. */
static void map_setup (void);
static void map_updateInternal( CstMapWidget *cst, double dt );
static void map_reset( CstMapWidget* cst, MapMode mode );
static CstMapWidget* map_globalCustomData( unsigned int wid );
static int map_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod, int isrepeat );
static void map_buttonZoom( unsigned int wid, const char* str );
static void map_setMinimal( unsigned int wid, int value );
static void map_buttonMarkSystem( unsigned int wid, const char* str );
static void map_buttonSystemMap( unsigned int wid, const char* str );
static void map_buttonMinimal( unsigned int wid, const char* str );
static void map_buttonCommodity( unsigned int wid, const char* str );
static void map_selectCur (void);
static void map_genModeList(void);
static void map_update_commod_av_price();
static void map_onClose( unsigned int wid, const char *str );

/**
 * @brief Initializes the map subsystem.
 *
 *    @return 0 on success.
 */
int map_init (void)
{
   return 0;
}

/**
 * @brief Destroys the map subsystem.
 */
void map_exit (void)
{
   if (decorator_stack != NULL) {
      for (int i=0; i<array_size(decorator_stack); i++)
         gl_freeTexture( decorator_stack[i].image );
      array_free( decorator_stack );
      decorator_stack = NULL;
   }
}

/**
 * @brief Handles key input to the map window.
 */
static int map_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod, int isrepeat )
{
   (void) mod;
   (void) isrepeat;

   if ((key == SDLK_SLASH) || (key == SDLK_f)) {
      map_inputFind( wid, NULL );
      return 1;
   }

   return 0;
}

static void map_setup (void)
{
   /* Mark systems as discovered as necessary. */
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = &systems_stack[i];
      sys_rmFlag( sys, SYSTEM_DISCOVERED | SYSTEM_INTEREST );

      /* Check to see if system has landable spobs. */
      sys_rmFlag( sys, SYSTEM_HAS_LANDABLE );
      for (int j=0; j<array_size(sys->spobs); j++) {
         Spob *p = sys->spobs[j];
         if (!spob_isKnown(p))
            continue;
         if (!spob_hasService(p, SPOB_SERVICE_LAND))
            continue;
         sys_setFlag( sys, SYSTEM_HAS_KNOWN_LANDABLE );
         spob_updateLand( p );
         if (p->can_land)
            sys_setFlag( sys, SYSTEM_HAS_LANDABLE );
      }

      int known = 1;
      for (int j=0; j<array_size(sys->jumps); j++) {
         JumpPoint *jp = &sys->jumps[j];
         if (jp_isFlag(jp, JP_EXITONLY) || jp_isFlag(jp, JP_HIDDEN))
            continue;
         if (!jp_isFlag(jp, JP_KNOWN)) {
            known = 0;
            break;
         }
      }
      if (known) {
         /* Check spobs. */
         for (int j=0; j<array_size(sys->spobs); j++) {
            Spob *p = sys->spobs[j];
            if (!spob_isKnown(p)) {
               known = 0;
               break;
            }
         }
      }

      if (known)
         sys_setFlag( sys, SYSTEM_DISCOVERED );
   }

   /* mark systems as needed */
   mission_sysMark();
}

/**
 * @brief Opens the map window.
 */
void map_open (void)
{
   unsigned int wid;
   StarSystem *cur;
   int w, h, x, y, rw;
   //int tw, th;
   CstMapWidget *cst;
   const char *title = _("Star Map");
   const glColour cBG = { 0., 0., 0., 0.95 };

   map_minimal_mode = player.map_minimal;
   listMapModeVisible = 0;

   /* Not under manual control. */
   if (pilot_isFlag( player.p, PILOT_MANUAL_CONTROL ))
      return;

   /* Destroy window if exists. */
   wid = window_get(MAP_WDWNAME);
   if (wid > 0) {
      if (window_isTop(wid))
         window_destroy( wid );
      return;
   }

   /* Set up stuff. */
   map_setup();

   /* Attempt to select current map if none is selected */
   if (map_selected == -1)
      map_selectCur();

   /* get the selected system. */
   cur = system_getIndex( map_selected );

   /* create the window. */
   wid = window_create( MAP_WDWNAME, title, -1, -1, -1, -1 );
   window_setCancel( wid, window_close );
   window_onClose( wid, map_onClose );
   window_handleKeys( wid, map_keyHandler );
   window_dimWindow( wid, &w, &h );
   window_setBorder( wid, 0 );

   /*
    * The map itself.
    */
   map_show( wid, 0, 0, w, h, 1., RCOL_W/2., BBAR_H/2. ); /* Reset zoom. */

   /* Map title. */
#if 0
   rw = gl_printWidthRaw( NULL, title );
   tw = rw+30;
   th = gl_defFont.h + 20;
   window_addRect( wid, (w-tw)/2., h-th, tw, th, "rctTBar", &cBG, 0 );
   window_addText( wid, (w-rw)/2., h-gl_defFont.h-10., rw, gl_defFont.h, 1, "txtTitle",
      &gl_defFont, NULL, title );
#endif

   /* Overlay background. */
   window_addRect( wid, w-RCOL_W, 0, RCOL_W, h, "rctRCol", &cBG, 0 );
   window_addRect( wid, 0, 0, w, BBAR_H, "rctBBar", &cBG, 0 );

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
    * Spobs:
    *   $Spob1, $Spob2, ...
    *
    * Services:
    *   $Services
    *
    * ...
    * [Autonav]
    * [ Find ]
    * [ Close ]
    */
   x  = RCOL_X; /* Right column X offset. */
   rw = RCOL_TEXT_W; /* Right column indented width maximum. */
   y  = -20;

   /* System Name */
   window_addText( wid, -90 + 80, y, 160, 20, 1, "txtSysname",
         &gl_defFont, NULL, _(cur->name) );
   y -= 10;

   /* Faction image */
   window_addImage( wid, -90 + 32, y - 32, 0, 0, "imgFaction", NULL, 0 );
   y -= 64 + 10;

   /* Faction */
   window_addText( wid, x, y, RCOL_HEADER_W, 20, 0, "txtSFaction",
         &gl_smallFont, &cFontGrey, _("Faction:") );
   window_addText( wid, x, y-gl_smallFont.h-5, rw, 300, 0, "txtFaction",
         &gl_smallFont, NULL, NULL );
   y -= 2 * gl_smallFont.h + 5 + 15;

   /* Standing */
   window_addText( wid, x, y, RCOL_HEADER_W, 20, 0, "txtSStanding",
         &gl_smallFont, &cFontGrey, _("Standing:") );
   window_addText( wid, x, y-gl_smallFont.h-5, rw, 300, 0, "txtStanding",
         &gl_smallFont, NULL, NULL );
   y -= 2 * gl_smallFont.h + 5 + 15;

   /* Presence. */
   window_addText( wid, x, y, RCOL_HEADER_W, 20, 0, "txtSPresence",
         &gl_smallFont, &cFontGrey, _("Presence:") );
   window_addText( wid, x, y-gl_smallFont.h-5, rw, 300, 0, "txtPresence",
         &gl_smallFont, NULL, NULL );
   y -= 2 * gl_smallFont.h + 5 + 15;

   /* Spobs */
   window_addText( wid, x, y, RCOL_HEADER_W, 20, 0, "txtSSpobs",
         &gl_smallFont, &cFontGrey, _("Space Objects:") );
   window_addText( wid, x, y-gl_smallFont.h-5, rw, 300, 0, "txtSpobs",
         &gl_smallFont, NULL, NULL );
   y -= 2 * gl_smallFont.h + 5 + 15;

   /* Services */
   window_addText( wid, x, y, RCOL_HEADER_W, 20, 0, "txtSServices",
         &gl_smallFont, &cFontGrey, _("Services:") );
   window_addText( wid, x, y-gl_smallFont.h-5, rw, 300, 0, "txtServices",
         &gl_smallFont, NULL, NULL );

   /* Close button */
   y = 15;
   window_addButton( wid, -20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnClose", _("Close"), window_close );
   /* Commodity button */
   window_addButton( wid, -20 - (BUTTON_WIDTH+20), y, BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommod", _("Mode"), map_buttonCommodity );
   /* Find button */
   window_addButtonKey( wid, -20 - 2*(BUTTON_WIDTH+20), y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnFind", _("Find"), map_inputFind, SDLK_f );
   /* Autonav button */
   window_addButtonKey( wid, -20 - 3*(BUTTON_WIDTH+20), y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnAutonav", _("Autonav"), player_autonavStartWindow, SDLK_a );
   /* MInimal button */
   window_addButtonKey( wid, -20 - 4*(BUTTON_WIDTH+20), y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnMinimal", NULL, map_buttonMinimal, SDLK_v );
   map_setMinimal( wid, map_minimal_mode );
   /* System info button */
   window_addButtonKey( wid, -20 - 5*(BUTTON_WIDTH+20), y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnSystem", _("System Info"), map_buttonSystemMap, SDLK_s );
   /* Mark this system button */
   window_addButtonKey( wid, -20 - 6*(BUTTON_WIDTH+20), y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnMarkSystem", _("Toggle Note"), map_buttonMarkSystem, SDLK_n );

   /*
    * Bottom stuff
    *
    * [+] [-]  Nebula, Interference
    */
   /* Zoom buttons */
   window_addButtonKey( wid, -60, 30 + BUTTON_HEIGHT, 30, BUTTON_HEIGHT, "btnZoomIn", "+", map_buttonZoom, SDLK_EQUALS );
   window_addButtonKey( wid, -20, 30 + BUTTON_HEIGHT, 30, BUTTON_HEIGHT, "btnZoomOut", "-", map_buttonZoom, SDLK_MINUS );
   /* Situation text */
   window_addText( wid, 20, 15, w - 40 - 7*(BUTTON_WIDTH+20), 30, 0,
         "txtSystemStatus", &gl_smallFont, NULL, NULL );

   /* Fuel. */
   window_addText( wid, -20, 40+BUTTON_HEIGHT*2, rw, 30, 0,
         "txtPlayerStatus", &gl_smallFont, NULL, NULL );

   map_genModeList();
   cst = map_globalCustomData(wid);
   map_reset( cst, map_mode );
   map_update( wid );

   /*
    * Disable Autonav button if player lacks fuel or if target is not a valid hyperspace target.
    */
   if ((player.p->fuel < player.p->fuel_consumption) || pilot_isFlag( player.p, PILOT_NOJUMP)
         || map_selected == cur_system - systems_stack || array_size(map_path) == 0)
      window_disableButton( wid, "btnAutonav" );
}

/*
 * Prepares economy info for rendering.  Called when cur_commod changes.
 */
static void map_update_commod_av_price (void)
{
   Commodity *c;

   if (cur_commod == -1 || map_selected == -1) {
      commod_av_gal_price = 0;
      return;
   }

   c = commod_known[cur_commod];
   if (cur_commod_mode == 0) {
      double totPrice = 0;
      int totPriceCnt = 0;
      for (int i=0; i<array_size(systems_stack); i++) {
         StarSystem *sys = system_getIndex( i );

         /* if system is not known, reachable, or marked. and we are not in the editor */
         if ((!sys_isKnown(sys) && !sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
              && !space_sysReachable(sys)))
            continue;
         if ((sys_isKnown(sys)) && (system_hasSpob(sys))) {
            double sumPrice = 0;
            int sumCnt = 0;
            double thisPrice;
            for (int j=0 ; j<array_size(sys->spobs); j++) {
               Spob *p = sys->spobs[j];
               for (int k=0; k<array_size(p->commodities); k++) {
                  if (p->commodities[k] == c) {
                     if (p->commodityPrice[k].cnt > 0) { /*commodity is known about*/
                        thisPrice = p->commodityPrice[k].sum / p->commodityPrice[k].cnt;
                        sumPrice += thisPrice;
                        sumCnt += 1;
                        break;
                     }
                  }
               }
            }
            if (sumCnt>0) {
               totPrice += sumPrice / sumCnt;
               totPriceCnt++;
            }
         }
      }
      if (totPriceCnt > 0)
         totPrice /= totPriceCnt;
      commod_av_gal_price = totPrice;

   }
   else
      commod_av_gal_price = 0;
}

static void map_update_autonav( unsigned int wid )
{
   char buf[STRMAX];
   StarSystem *sys;
   int autonav, th;
   int jumps = floor(player.p->fuel / player.p->fuel_consumption);
   int p = 0;
   int rw = RCOL_HEADER_W;
   p += scnprintf(&buf[p], sizeof(buf)-p, "#n%s#0", _("Fuel: ") );
   p += scnprintf(&buf[p], sizeof(buf)-p, n_("%d jump", "%d jumps", jumps), jumps );
   sys = map_getDestination( &autonav );
   p += scnprintf(&buf[p], sizeof(buf)-p, "\n#n%s#0", _("Autonav: ") );
   if (sys==NULL)
      p += scnprintf(&buf[p], sizeof(buf)-p, _("Off") );
   else {
      if (autonav > jumps)
         p += scnprintf(&buf[p], sizeof(buf)-p, "#r" );
      p += scnprintf(&buf[p], sizeof(buf)-p, n_("%d jump", "%d jumps", autonav), autonav );
      if (autonav > jumps)
         p += scnprintf(&buf[p], sizeof(buf)-p, "#0" );
   }
   th = gl_printHeightRaw( &gl_smallFont, rw, buf );
   window_resizeWidget( wid, "txtPlayerStatus", rw, th );
   window_moveWidget( wid, "txtPlayerStatus", RCOL_X, 40+BUTTON_HEIGHT*2 );
   window_modifyText( wid, "txtPlayerStatus", buf );
}

static void map_update_status( unsigned int wid, const char *buf )
{
   int w, h;
   window_modifyText( wid, "txtSystemStatus", buf );
   if (buf==NULL)
      return;
   window_dimWidget( wid, "txtSystemStatus", &w, NULL );
   h = gl_printHeightRaw( &gl_smallFont, w, buf );
   window_moveWidget( wid, "txtSystemStatus", 20, (BBAR_H-h)/2 );
   window_resizeWidget( wid, "txtSystemStatus", w, h );
}

/**
 * @brief Updates the map window.
 *
 *    @param wid Window id.
 */
static void map_update( unsigned int wid )
{
   int found, multiple;
   StarSystem *sys;
   int f, fh, h, x, y, logow, logoh;
   unsigned int services, services_u, services_h, services_f, services_r;
   int hasSpobs;
   char t;
   const char *adj;
   char buf[STRMAX];
   int p;
   const glTexture *logo;
   double w, dmg, itf;

   /* Needs map to update. */
   if (!map_isOpen())
      return;

   /* Propagate updates to map_mode, if an. */
   map_globalCustomData(wid)->mode = map_mode;

   /* Get selected system. */
   sys = system_getIndex( map_selected );

   /* Not known and no markers. */
   if (!(sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)) &&
         !sys_isKnown(sys) && !space_sysReachable(sys)) {
      map_selectCur();
      sys = system_getIndex( map_selected );
   }
   /* Average commodity price */
   map_update_commod_av_price();

   /* Economy button */
   if (map_mode == MAPMODE_TRADE) {
      Commodity *c = commod_known[cur_commod];
      if (cur_commod_mode == 1) {
         snprintf( buf, sizeof(buf),
                   _("Showing %s prices relative to %s:\n"
                     "Positive/blue indicate profit while negative/orange values indicate loss when sold at the corresponding system."),
                   _(c->name), _(sys->name) );
         map_update_status( wid, buf );
      }
      else {
         snprintf(buf, sizeof(buf), _("Showing known %s prices.\nGalaxy-wide average: %.2f"), _(c->name), commod_av_gal_price);
         map_update_status( wid, buf );
      }
   }
   else
      map_update_status( wid, NULL );

   /*
    * Right Text
    */
   x = RCOL_X; /* Side bar X offset. */
   w = RCOL_TEXT_W; /* Width of the side bar. */
   y = -20 - 20 - 64 - gl_defFont.h; /* Initialized to position for txtSFaction. */

   if (!sys_isKnown(sys)) { /* System isn't known, erase all */
      /*
       * Right Text
       */
      if (sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED))
         window_modifyText( wid, "txtSysname", _(sys->name) );
      else
         window_modifyText( wid, "txtSysname", _("Unknown") );

      /* Faction */
      window_modifyImage( wid, "imgFaction", NULL, 0, 0 );
      window_moveWidget( wid, "txtSFaction", x, y);
      window_moveWidget( wid, "txtFaction", x, y - gl_smallFont.h - 5 );
      window_modifyText( wid, "txtFaction", _("Unknown") );
      y -= 2 * gl_smallFont.h + 5 + 15;

      /* Standing */
      window_moveWidget( wid, "txtSStanding", x, y );
      window_moveWidget( wid, "txtStanding", x, y - gl_smallFont.h - 5 );
      window_modifyText( wid, "txtStanding", _("Unknown") );
      y -= 2 * gl_smallFont.h + 5 + 15;

      /* Presence. */
      window_moveWidget( wid, "txtSPresence", x, y );
      window_moveWidget( wid, "txtPresence",  x, y - gl_smallFont.h - 5 );
      window_modifyText( wid, "txtPresence", _("Unknown") );
      y -= 2 * gl_smallFont.h + 5 + 15;

      /* Spobs */
      window_moveWidget( wid, "txtSSpobs", x, y );
      window_moveWidget( wid, "txtSpobs", x, y - gl_smallFont.h - 5 );
      window_modifyText( wid, "txtSpobs", _("Unknown") );
      y -= 2 * gl_smallFont.h + 5 + 15;

      /* Services */
      window_moveWidget( wid, "txtSServices", x, y );
      window_moveWidget( wid, "txtServices", x, y -gl_smallFont.h - 5 );
      window_modifyText( wid, "txtServices", _("Unknown") );

      /* Update autonav stuff. */
      map_update_autonav( wid );

      /*
       * Bottom Text
       */
      map_update_status( wid, NULL );
      return;
   }

   /* System is known */
   window_modifyText( wid, "txtSysname", _(sys->name) );

   f = -1;
   multiple = 0;
   for (int i=0; i<array_size(sys->spobs); i++) {
      if (!spob_isKnown(sys->spobs[i]))
         continue;
      if ((sys->spobs[i]->presence.faction >= 0)
            && (!faction_isKnown(sys->spobs[i]->presence.faction)) )
         continue;

      if ((f == -1) && (sys->spobs[i]->presence.faction >= 0)) {
         f = sys->spobs[i]->presence.faction;
      }
      else if (f != sys->spobs[i]->presence.faction /** @todo more verbosity */
               && (sys->spobs[i]->presence.faction >= 0)) {
         snprintf( buf, sizeof(buf), _("Multiple") );
         multiple = 1;
         break;
      }
   }
   if (f == -1) {
      window_modifyImage( wid, "imgFaction", NULL, 0, 0 );
      window_modifyText( wid, "txtFaction", _("N/A") );
      window_modifyText( wid, "txtStanding", _("N/A") );
      h = gl_smallFont.h;
      fh = gl_smallFont.h;
   }
   else {
      const char *fcttext;
      if (!multiple) /* saw them all and all the same */
         snprintf( buf, sizeof(buf), "%s", faction_longname(f) );

      /* Modify the image. */
      logo = faction_logo(f);
      logow = logo == NULL ? 0 : logo->w * (double)FACTION_LOGO_SM / MAX( logo->w, logo->h );
      logoh = logo == NULL ? 0 : logo->h * (double)FACTION_LOGO_SM / MAX( logo->w, logo->h );
      window_modifyImage( wid, "imgFaction", logo, logow, logoh );
      if (logo != NULL)
         window_moveWidget( wid, "imgFaction",
               -90 + logow/2, -20 - 32 - 10 - gl_defFont.h + logoh/2);
      fcttext = faction_getStandingText( f );

      /* Modify the text */
      window_modifyText( wid, "txtFaction", buf );
      window_modifyText( wid, "txtStanding", fcttext );

      h = gl_printHeightRaw( &gl_smallFont, w, buf );
      fh = gl_printHeightRaw( &gl_smallFont, w, fcttext );
   }

   /* Faction */
   window_moveWidget( wid, "txtSFaction", x, y);
   window_moveWidget( wid, "txtFaction", x, y-gl_smallFont.h - 5 );
   y -= gl_smallFont.h + h + 5 + 15;

   /* Standing */
   window_moveWidget( wid, "txtSStanding", x, y );
   window_moveWidget( wid, "txtStanding", x, y-gl_smallFont.h - 5 );
   y -= gl_smallFont.h + fh + 5 + 15;

   window_moveWidget( wid, "txtSPresence", x, y );
   window_moveWidget( wid, "txtPresence", x, y-gl_smallFont.h-5 );
   map_updateFactionPresence( wid, "txtPresence", sys, 0 );
   /* Scroll down. */
   h = window_getTextHeight( wid, "txtPresence" );
   y -= 40 + (h - gl_smallFont.h);

   /* Get spobs */
   hasSpobs = 0;
   p = 0;
   buf[0] = '\0';
   for (int i=0; i<array_size(sys->spobs); i++) {
      const char *prefix, *suffix;
      Spob *s = sys->spobs[i];

      if (!spob_isKnown(s))
         continue;

      /* Colourize output. */
      spob_updateLand( s );
      t = spob_getColourChar( s );
      prefix = spob_getSymbol( s );
      //suffix = (spob_isFlag( s, SPOB_MARKED ) ? _(" #o(M)") : "");
      suffix = "";

      if (!hasSpobs)
         p += scnprintf( &buf[p], sizeof(buf)-p, "#%c%s%s%s#n",
               t, prefix, spob_name( s ), suffix );
      else
         p += scnprintf( &buf[p], sizeof(buf)-p, ",\n#%c%s%s%s#n",
               t, prefix, spob_name( s ), suffix );
      hasSpobs = 1;
   }
   if (hasSpobs == 0) {
      strncpy( buf, _("None"), sizeof(buf)-1 );
      buf[sizeof(buf)-1] = '\0';
   }
   /* Update text. */
   window_modifyText( wid, "txtSpobs", buf );
   window_moveWidget( wid, "txtSSpobs", x, y );
   window_moveWidget( wid, "txtSpobs", x, y-gl_smallFont.h-5 );
   /* Scroll down. */
   h  = gl_printHeightRaw( &gl_smallFont, w, buf );
   y -= 40 + (h - gl_smallFont.h);

   /* Get the services */
   window_moveWidget( wid, "txtSServices", x, y );
   window_moveWidget( wid, "txtServices", x, y-gl_smallFont.h-5 );
   services = 0;
   services_u = 0;
   services_h = 0;
   services_f = 0;
   services_r = 0;
   for (int i=0; i<array_size(sys->spobs); i++) {
      Spob *pnt = sys->spobs[i];
      if (!spob_isKnown(pnt))
         continue;

      if (!spob_hasService( pnt, SPOB_SERVICE_INHABITED ))
         services_u |= pnt->services;
      else if (pnt->can_land) {
         if (areAllies(pnt->presence.faction,FACTION_PLAYER))
            services_f |= pnt->services;
         else
            services |= pnt->services;
      }
      else if (areEnemies(pnt->presence.faction,FACTION_PLAYER))
         services_h |= pnt->services;
      else
         services_r |= pnt->services;
   }
   buf[0] = '\0';
   p = 0;
   /*snprintf(buf, sizeof(buf), "%f\n", sys->prices[0]);*/ /*Hack to control prices. */
   for (int i=SPOB_SERVICE_LAND; i<=SPOB_SERVICE_SHIPYARD; i<<=1) {
      if (services_f & i)
         p += scnprintf( &buf[p], sizeof(buf)-p, "#F+ %s\n", _(spob_getServiceName(i)) );
      else if (services & i)
         p += scnprintf( &buf[p], sizeof(buf)-p, "#N~ %s\n", _(spob_getServiceName(i)) );
      else if (services_h & i)
         p += scnprintf( &buf[p], sizeof(buf)-p, "#H!! %s\n", _(spob_getServiceName(i)) );
      else if (services_r & i)
         p += scnprintf( &buf[p], sizeof(buf)-p, "#R* %s\n", _(spob_getServiceName(i)) );
      else if (services_u & i)
         p += scnprintf( &buf[p], sizeof(buf)-p, "#I= %s\n", _(spob_getServiceName(i)) );
   }
   if (buf[0] == '\0')
      p += scnprintf( &buf[p], sizeof(buf)-p, _("None"));

   window_modifyText( wid, "txtServices", buf );

   /*
    * System Status, if not showing commodity info
    */
   if (map_mode == MAPMODE_TRAVEL) {
      buf[0] = '\0';
      p = 0;

      /* Special feature text. */
      if (sys->features != NULL)
         p += scnprintf(&buf[p], sizeof(buf)-p, "%s", _(sys->features) );

      /* Mention spob features. */
      for (int i=0; i<array_size(sys->spobs); i++) {
         Spob *spob = sys->spobs[i];
         if (spob->feature == NULL)
            continue;
         if (!spob_isKnown(spob))
            continue;
         if (buf[0] != '\0')
            p += scnprintf(&buf[p], sizeof(buf)-p, _(", "));
         p += scnprintf(&buf[p], sizeof(buf)-p, "%s", _(spob->feature));
      }

      /* Mention trade lanes if applicable. */
      found = 0;
      for (int i=0; i<array_size(sys->jumps); i++) {
         if (sys->jumps[i].hide<=0.) {
            found = 1;
            break;
         }
      }
      if (found) {
         if (buf[0] != '\0')
            p += scnprintf(&buf[p], sizeof(buf)-p, _(", "));
         p += scnprintf(&buf[p], sizeof(buf)-p, "#g%s#0", _("Trade Lane") );
      }

      /* Nebula. */
      if (sys->nebu_density > 0.) {
         if (buf[0] != '\0')
            p += scnprintf(&buf[p], sizeof(buf)-p, _(", "));

         /* Density. */
         if (sys->nebu_density > 700.)
            adj = _("Dense ");
         else if (sys->nebu_density < 300.)
            adj = _("Light ");
         else
            adj = "";

         /* Volatility */
         dmg = sys->nebu_volatility;
         if (sys->nebu_volatility > 50.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#r" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Volatile %sNebula (%.1f MW)"), adj, dmg);
         }
         else if (sys->nebu_volatility > 20.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#o" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Dangerous %sNebula (%.1f MW)"), adj, dmg);
         }
         else if (sys->nebu_volatility > 0.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#y" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Unstable %sNebula (%.1f MW)"), adj, dmg);
         }
         else
            p += scnprintf(&buf[p], sizeof(buf)-p, _("%sNebula"), adj);
         p += scnprintf(&buf[p], sizeof(buf)-p, "#0" );
      }
      /* Interference. */
      if (sys->interference > 0.) {
         if (buf[0] != '\0')
            p += scnprintf(&buf[p], sizeof(buf)-p, _(", "));

         itf = sys->interference;
         if (sys->interference > 700.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#r" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Dense Interference (%.0f%%)"), itf);
         }
         else if (sys->interference < 300.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#o" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Light Interference (%.0f%%)"), itf);
         }
         else {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#y" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Interference (%.0f%%)"), itf);
         }
         p += scnprintf(&buf[p], sizeof(buf)-p, "#0" );
      }
      /* Asteroids. */
      if (array_size(sys->asteroids) > 0) {
         double density = sys->asteroid_density;

         if (buf[0] != '\0')
            p += scnprintf(&buf[p], sizeof(buf)-p, _(", "));

         if (density >= 1000.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#o" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Dense Asteroids"));
         }
         else if (density <= 300.) {
            p += scnprintf(&buf[p], sizeof(buf)-p, "#y" );
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Light Asteroids"));
         }
         else
            p += scnprintf(&buf[p], sizeof(buf)-p, _("Asteroids"));
         p += scnprintf(&buf[p], sizeof(buf)-p, "#0" );
      }
      /* Update the string. */
      map_update_status( wid, buf );
   }

   /* Player info. */
   map_update_autonav( wid );
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
 * @param a Colour alpha to use.
 * @param num Total number of markers.
 * @param cur Current marker to draw.
 * @param type Type to draw.
 */
static void map_drawMarker( double x, double y, double zoom,
      double r, double a, int num, int cur, int type )
{
   (void) zoom;
   const glColour* colours[] = {
      &cMarkerNew, &cMarkerPlot, &cMarkerHigh, &cMarkerLow, &cMarkerComputer, &cMarkerNew
   };
   double alpha;
   glColour col;

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

   x = x + 3.0*r * cos(alpha);
   y = y + 3.0*r * sin(alpha);
   r *= 2.0;

   /* Special case notes marker. */
   if (type==5) {
      col = cFontOrange;
      col.a *= a;
      glUseProgram(shaders.notemarker.program);
      gl_renderShader( x, y, r, r, alpha, &shaders.notemarker, &col, 1 );
      return;
   }

   glUseProgram(shaders.sysmarker.program);
   if (type==0) {
      col_blend( &col, colours[type], &cWhite, MIN(1.0, 0.75 + 0.25*sin(2.0*M_PI*map_dt)) );
      x += 0.25*r * cos(alpha);
      y += 0.25*r * sin(alpha);
      r *= 1.25;
      glUniform1i( shaders.sysmarker.parami, 1 );
   }
   else {
      col_blend( &col, colours[type], &cWhite, MIN(1.0, 1.0 + 0.25*sin(2.0*M_PI*map_dt)) );
      glUniform1i( shaders.sysmarker.parami, 0 );
   }
   col.a *= a;
   gl_renderShader( x, y, r, r, alpha, &shaders.sysmarker, &col, 1 );
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
   CstMapWidget *cst = data;
   double x,y,z,r;
   glColour col;
   StarSystem *sys;
   double dt = naev_getrealdt();

   /* Update timer. */
   map_dt += dt;
   map_updateInternal( cst, dt );

   /* Parameters. */
   map_renderParams( bx, by, cst->xpos, cst->ypos, w, h, cst->zoom, &x, &y, &r );
   z = cst->zoom;

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   if (cst->alpha_decorators > 0.)
      map_renderDecorators( x, y, z, 0, EASE_ALPHA(cst->alpha_decorators) );

   /* Render faction disks. */
   if (cst->alpha_faction > 0.)
      map_renderFactionDisks( x, y, z, r, 0, EASE_ALPHA(cst->alpha_faction) );

      /* Render environmental features. */
   if (cst->alpha_env > 0.)
      map_renderSystemEnvironment( x, y, z, 0, EASE_ALPHA(cst->alpha_env) );

   /* Render jump routes. */
   map_renderJumps( x, y, z, r, 0 );

   /* Render the player's jump route. */
   if (cst->alpha_path > 0.)
      map_renderPath( x, y, z, r, EASE_ALPHA(cst->alpha_path) );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, z, w, h, r, cst->mode );

   /* Render system markers and notes. */
   if (cst->alpha_markers > 0.)
      map_renderMarkers( x, y, z, r, EASE_ALPHA(cst->alpha_markers) );

   /* Render system names and notes. */
   if (cst->alpha_names > 0.)
      map_renderNames( bx, by, x, y, z, w, h, 0, EASE_ALPHA(cst->alpha_names) );

   /* Render commodity info. */
   if (cst->alpha_commod > 0.)
      map_renderCommod(  bx, by, x, y, z, w, h, r, 0, EASE_ALPHA(cst->alpha_commod) );

   /* We want the notes on top of everything. */
   if (cst->alpha_markers > 0.)
      map_renderNotes( bx, by, x, y, z, w, h, 0, EASE_ALPHA(cst->alpha_markers) );

   /* Values from cRadar_tSpob */
   col.r = cRadar_tSpob.r;
   col.g = cRadar_tSpob.g;
   col.b = cRadar_tSpob.b;

   /* Selected system. */
   if (map_selected != -1) {
      sys = system_getIndex( map_selected );
      glUseProgram( shaders.selectspob.program );
      glUniform1f( shaders.selectspob.dt, map_dt ); /* good enough. */
      gl_renderShader( x + sys->pos.x * z, y + sys->pos.y * z,
            1.7*r, 1.7*r, 0., &shaders.selectspob, &cRadar_tSpob, 1 );
   }

   /* Current spob. */
   gl_renderCircle( x + cur_system->pos.x * z,
         y + cur_system->pos.y * z,
         1.5*r, &col, 0 );

   glClear( GL_DEPTH_BUFFER_BIT );
}

/**
 * @brief Gets the render parameters.
 */
void map_renderParams( double bx, double by, double xpos, double ypos,
      double w, double h, double zoom, double *x, double *y, double *r )
{
   *r = round(CLAMP(6., 20., 8.*zoom));
   *x = round((bx - xpos + w/2) * 1.);
   *y = round((by - ypos + h/2) * 1.);
}

/**
 * @brief Renders the map background decorators.
 *
 * TODO visibility check should be cached and computed when opening only.
 */
void map_renderDecorators( double x, double y, double zoom, int editor, double alpha )
{
   const glColour ccol = { .r=1., .g=1., .b=1., .a=2./3.*alpha }; /**< White */

   /* Fade in the decorators to allow toggling between commodity and nothing */
   for (int i=0; i<array_size(decorator_stack); i++) {
      int visible;
      MapDecorator *decorator = &decorator_stack[i];

      /* only if pict couldn't be loaded */
      if (decorator->image == NULL)
         continue;

      visible = 0;
      if (!editor) {
         for (int j=0; j<array_size(systems_stack) && visible==0; j++) {
            StarSystem *sys = system_getIndex( j );

            if (sys_isFlag(sys, SYSTEM_HIDDEN))
               continue;

            if (!sys_isKnown(sys))
               continue;

            if ((decorator->x < sys->pos.x + decorator->detection_radius) &&
                  (decorator->x > sys->pos.x - decorator->detection_radius) &&
                  (decorator->y < sys->pos.y + decorator->detection_radius) &&
                  (decorator->y > sys->pos.y - decorator->detection_radius)) {
               visible = 1;
            }
         }
      }

      if (editor || visible==1) {
         double tx = x + decorator->x*zoom;
         double ty = y + decorator->y*zoom;

         int sw = decorator->image->sw*zoom;
         int sh = decorator->image->sh*zoom;

         gl_renderScale( decorator->image,
               tx - sw/2, ty - sh/2, sw, sh, &ccol );
      }
   }
}

/**
 * @brief Renders the faction disks.
 */
void map_renderFactionDisks( double x, double y, double zoom, double r, int editor, double alpha )
{
   for (int i=0; i<array_size(systems_stack); i++) {
      glColour c;
      double tx, ty;
      StarSystem *sys = system_getIndex( i );

      if (sys_isFlag(sys,SYSTEM_HIDDEN))
         continue;

      if ((!sys_isFlag(sys, SYSTEM_HAS_KNOWN_LANDABLE) || !sys_isKnown(sys)) && !editor)
         continue;

      tx = x + sys->pos.x*zoom;
      ty = y + sys->pos.y*zoom;

      /* System has faction and is known or we are in editor. */
      if (sys->faction != -1) {
         const glColour *col;
         double presence = sqrt(sys->ownerpresence);

         /* draws the disk representing the faction */
         double sr = (40. + presence * 3.) * zoom * 0.5;

         col = faction_colour(sys->faction);
         c.r = col->r;
         c.g = col->g;
         c.b = col->b;
         c.a = 0.6 * alpha;

         glUseProgram(shaders.factiondisk.program);
         glUniform1f(shaders.factiondisk.paramf, r / sr );
         gl_renderShader( tx, ty, sr, sr, 0., &shaders.factiondisk, &c, 1 );
      }
   }
}

/**
 * @brief Renders the faction disks.
 */
void map_renderSystemEnvironment( double x, double y, double zoom, int editor, double alpha )
{
   for (int i=0; i<array_size(systems_stack); i++) {
      double tx, ty;
      /* Fade in the disks to allow toggling between commodity and nothing */
      StarSystem *sys = system_getIndex( i );

      if (sys_isFlag(sys,SYSTEM_HIDDEN))
         continue;

      if (!sys_isKnown(sys) && !editor)
         continue;

      tx = x + sys->pos.x*zoom;
      ty = y + sys->pos.y*zoom;

      /* Draw background. */
      /* TODO draw asteroids too! */
      if (sys->nebu_density > 0.) {
         mat4 projection;
         double sw, sh;
         sw = (50. + sys->nebu_density * 50. / 1000.) * zoom;
         sh = sw;

         /* Set the vertex. */
         projection = gl_view_matrix;
         mat4_translate( &projection, tx-sw/2., ty-sh/2., 0. );
         mat4_scale( &projection, sw, sh, 1. );

         /* Start the program. */
         glUseProgram(shaders.nebula_map.program);

         /* Set shader uniforms. */
         glUniform1f(shaders.nebula_map.hue, sys->nebu_hue);
         glUniform1f(shaders.nebula_map.alpha, alpha);
         gl_uniformMat4(shaders.nebula_map.projection, &projection);
         glUniform1f(shaders.nebula_map.time, map_dt / 10.0);
         glUniform2f(shaders.nebula_map.globalpos, sys->pos.x, sys->pos.y );
         glUniform1f(shaders.nebula_map.volatility, sys->nebu_volatility );

         /* Draw. */
         glEnableVertexAttribArray( shaders.nebula_map.vertex );
         gl_vboActivateAttribOffset( gl_squareVBO, shaders.nebula_map.vertex, 0, 2, GL_FLOAT, 0 );
         glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

         /* Clean up. */
         glDisableVertexAttribArray( shaders.nebula_map.vertex );
         glUseProgram(0);
         gl_checkErr();
      }
      else if (sys->map_shader != NULL) {
         mat4 projection;
         double sw, sh;
         sw = 100. * zoom;
         sh = sw;

         /* Set the vertex. */
         projection = gl_view_matrix;
         mat4_translate( &projection, tx-sw/2., ty-sh/2., 0. );
         mat4_scale( &projection, sw, sh, 1. );

         /* Start the program. */
         glUseProgram( sys->ms->program );

         /* Set shader uniforms. */
         gl_uniformMat4(sys->ms->projection, &projection);
         glUniform1f(sys->ms->time, map_dt);
         glUniform2f(sys->ms->globalpos, sys->pos.x, sys->pos.y );
         glUniform1f(sys->ms->alpha, alpha);

         /* Draw. */
         glEnableVertexAttribArray( sys->ms->vertex );
         gl_vboActivateAttribOffset( gl_squareVBO, sys->ms->vertex, 0, 2, GL_FLOAT, 0 );
         glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

         /* Clean up. */
         glDisableVertexAttribArray( sys->ms->vertex );
         glUseProgram(0);
         gl_checkErr();
      }
   }
}

/**
 * @brief Renders the jump routes between systems.
 */
void map_renderJumps( double x, double y, double zoom, double radius, int editor )
{
   for (int i=0; i<array_size(systems_stack); i++) {
      double x1,y1;
      StarSystem *sys = system_getIndex( i );

      if (sys_isFlag(sys,SYSTEM_HIDDEN))
         continue;

      if (!sys_isKnown(sys) && !editor)
         continue; /* we don't draw hyperspace lines */

      x1 = x + sys->pos.x * zoom;
      y1 = y + sys->pos.y * zoom;

      for (int j=0; j < array_size(sys->jumps); j++) {
         double x2,y2, rx,ry, r, rw,rh;
         const glColour *col, *cole;
         StarSystem *jsys = sys->jumps[j].target;
         if (sys_isFlag(jsys,SYSTEM_HIDDEN))
            continue;
         if (!space_sysReachableFromSys(jsys,sys) && !editor)
            continue;

         /* Choose colours. */
         cole = &cAquaBlue;
         for (int k=0; k < array_size(jsys->jumps); k++) {
            if (jsys->jumps[k].target == sys) {
               if (jp_isFlag(&jsys->jumps[k], JP_EXITONLY))
                  cole = &cGrey80;
               else if (jp_isFlag(&jsys->jumps[k], JP_HIDDEN))
                  cole = &cRed;
               break;
            }
         }
         if (jp_isFlag(&sys->jumps[j], JP_EXITONLY))
            col = &cGrey80;
         else if (jp_isFlag(&sys->jumps[j], JP_HIDDEN))
            col = &cRed;
         else
            col = &cAquaBlue;

         x2 = x + jsys->pos.x * zoom;
         y2 = y + jsys->pos.y * zoom;
         rx = x2-x1;
         ry = y2-y1;
         r  = atan2( ry, rx );
         rw = MOD(rx,ry)/2.;

         if (sys->jumps[j].hide<=0.) {
            col = &cGreen;
            rh = 2.5;
         }
         else {
            rh = 1.5;
         }

         glUseProgram( shaders.jumplane.program );
         gl_uniformColor( shaders.jumplane.paramv, cole );
         glUniform1f( shaders.jumplane.paramf, radius );
         gl_renderShader( (x1+x2)/2., (y1+y2)/2., rw, rh, r, &shaders.jumplane, col, 1 );
      }
   }
}

/**
 * @brief Renders the systems.
 */
void map_renderSystems( double bx, double by, double x, double y,
      double zoom, double w, double h, double r, MapMode mode )
{
   for (int i=0; i<array_size(systems_stack); i++) {
      const glColour *col;
      double tx, ty;
      StarSystem *sys = system_getIndex( i );

      if (sys_isFlag(sys,SYSTEM_HIDDEN))
         continue;

      /* if system is not known, reachable, or marked. and we are not in the editor */
      if ((!sys_isKnown(sys) && !sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
           && !space_sysReachable(sys)) && mode != MAPMODE_EDITOR)
         continue;

      tx = x + sys->pos.x*zoom;
      ty = y + sys->pos.y*zoom;

      /* Skip if out of bounds. */
      if (!rectOverlap(tx-r, ty-r, 2.*r, 2.*r, bx, by, w, h))
         continue;

      /* Draw an outer ring. */
      if (mode == MAPMODE_EDITOR || mode == MAPMODE_TRAVEL || mode == MAPMODE_TRADE)
         gl_renderCircle( tx, ty, r, &cInert, 0 );

      /* Ignore not known systems when not in the editor. */
      if (mode != MAPMODE_EDITOR && !sys_isKnown(sys))
         continue;

      if (mode == MAPMODE_EDITOR || mode == MAPMODE_TRAVEL || mode == MAPMODE_TRADE) {
         if (!system_hasSpob(sys))
            continue;
         if (!sys_isFlag(sys, SYSTEM_HAS_KNOWN_LANDABLE) && mode != MAPMODE_EDITOR)
            continue;
         /* Spob colours */
         if (mode != MAPMODE_EDITOR && !sys_isKnown(sys))
            col = &cInert;
         else if (sys->faction < 0)
            col = &cInert;
         else if (mode == MAPMODE_EDITOR)
            col = &cNeutral;
         else if (areEnemies(FACTION_PLAYER,sys->faction))
            col = &cHostile;
         else if (!sys_isFlag(sys, SYSTEM_HAS_LANDABLE))
            col = &cRestricted;
         else if (areAllies(FACTION_PLAYER,sys->faction))
            col = &cFriend;
         else
            col = &cNeutral;

         if (mode == MAPMODE_EDITOR) {
            /* Radius slightly shorter. */
            gl_renderCircle( tx, ty, 0.5 * r, col, 1 );
         }
         else
            gl_renderCircle( tx, ty, 0.65 * r, col, 1 );
      }
      else if (mode == MAPMODE_DISCOVER) {
         gl_renderCircle( tx, ty, r, &cInert, 0 );
         if (sys_isFlag( sys, SYSTEM_DISCOVERED ))
            gl_renderCircle( tx, ty,  0.65 * r, &cGreen, 1 );
      }
   }
}

/**
 * @brief Render the map path.
 */
static void map_renderPath( double x, double y, double zoom, double radius, double alpha )
{
   StarSystem *sys1 = cur_system;
   int jmax, jcur;

   if (array_size(map_path) == 0)
      return;

   /* Player must exist. */
   if (player.p==NULL)
      return;

   jmax = pilot_getJumps(player.p); /* Maximum jumps. */
   jcur = jmax; /* Jump range remaining. */

   for (int j=0; j<array_size(map_path); j++) {
      glColour col;
      double x1,y1, x2,y2, rx,ry, rw,rh, r;
      StarSystem *sys2 = map_path[j];
      if (sys_isFlag(sys1,SYSTEM_HIDDEN) || sys_isFlag(sys2,SYSTEM_HIDDEN))
         continue;
      if (jcur == jmax && jmax > 0)
         col = cGreen;
      else if (jcur < 1)
         col = cRed;
      else
         col = cYellow;
      col.a = alpha;

      x1 = x + sys1->pos.x * zoom;
      y1 = y + sys1->pos.y * zoom;
      x2 = x + sys2->pos.x * zoom;
      y2 = y + sys2->pos.y * zoom;
      rx = x2-x1;
      ry = y2-y1;
      r  = atan2( ry, rx );
      rw = (MOD(rx,ry)+radius)/2.;
      rh = 5.;

      glUseProgram( shaders.jumplanegoto.program );
      glUniform1f( shaders.jumplanegoto.dt, map_dt );
      glUniform1f( shaders.jumplanegoto.paramf, radius );
      glUniform1i( shaders.jumplanegoto.parami, (jcur >= 1) );
      gl_renderShader( (x1+x2)/2., (y1+y2)/2., rw, rh, r, &shaders.jumplanegoto, &col, 1 );

      jcur--;
      sys1 = sys2;
   }
}

/**
 * @brief Renders the system note on the map if mouse is close.
 */
void map_renderNotes( double bx, double by, double x, double y,
      double zoom, double w, double h, int editor, double alpha )
{
   (void) w;
   (void) h;

   if ((zoom <= 0.5) || editor)
      return;

   if (map_show_notes)
      glClear( GL_DEPTH_BUFFER_BIT );

   /* Find mouse over system and draw. */
   for (int i=0; i<array_size(systems_stack); i++) {
      double tx,ty, tw,th;
      glColour col;
      glFont *font;
      StarSystem *sys = &systems_stack[i];

      if (!sys_isFlag(sys,SYSTEM_PMARKED))
         continue;

      if (sys->note == NULL)
         continue;

      /* Set up position. */
      tx = x + sys->pos.x*zoom;
      ty = y + sys->pos.y*zoom;

      /* Mouse is over. */
      if (!map_show_notes && ((pow2(tx-map_mx-bx)+pow2(ty-map_my-by)) > pow2(MAP_MOVE_THRESHOLD)))
         continue;

      if (!map_show_notes)
         glClear( GL_DEPTH_BUFFER_BIT );

      font = (zoom >= 1.5) ? &gl_defFont : &gl_smallFont;
      tx += 12.*zoom;
      ty -= font->h*2.;
      tw = gl_printWidthRaw( font, sys->note )+8.;
      th = font->h+8.;

      /* Background. */
      col = cBlack;
      col.a = alpha*0.8;
      gl_renderRect( tx-4., ty-4., tw, th, &col );

      /* Render note */
      col = cFontOrange;
      col.a = alpha;
      gl_printRaw( font, tx, ty, &col, -1, sys->note );
   }
}

/**
 * @brief Renders the system names and notes on the map.
 */
void map_renderNames( double bx, double by, double x, double y,
      double zoom, double w, double h, int editor, double alpha )
{
   double tx,ty, vx,vy, d,n;
   int textw;
   char buf[32];
   glColour col;
   glFont *font;

   if (zoom <= 0.5)
      return;

   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = system_getIndex( i );

      if (sys_isFlag(sys,SYSTEM_HIDDEN))
         continue;

      /* Skip system. */
      if (!editor && !sys_isKnown(sys))
         continue;

      font = (zoom >= 1.5) ? &gl_defFont : &gl_smallFont;

      textw = gl_printWidthRaw( font, _(sys->name) );
      tx = x + (sys->pos.x+12.) * zoom;
      ty = y + (sys->pos.y) * zoom - font->h*0.5;

      /* Skip if out of bounds. */
      if (!rectOverlap(tx, ty, textw, font->h, bx, by, w, h))
         continue;

      col = cWhite;
      col.a = alpha;
      gl_printRaw( font, tx, ty, &col, -1, _(sys->name) );
   }

   /* Raw hidden values if we're in the editor. */
   if (!editor || (zoom <= 1.0))
      return;

   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = system_getIndex( i );
      for (int j=0; j<array_size(sys->jumps); j++) {
         StarSystem *jsys = sys->jumps[j].target;
         /* Calculate offset. */
         vx  = jsys->pos.x - sys->pos.x;
         vy  = jsys->pos.y - sys->pos.y;
         n   = sqrt( pow2(vx) + pow2(vy) );
         vx /= n;
         vy /= n;
         d   = MAX(n*0.3*zoom, 15);
         tx  = x + zoom*sys->pos.x + d*vx;
         ty  = y + zoom*sys->pos.y + d*vy;
         /* Display. */
         n = sys->jumps[j].hide;
         if (n == 0.)
            snprintf( buf, sizeof(buf), "#gH: %.2f", n );
         else
            snprintf( buf, sizeof(buf), "H: %.2f", n );
         col = cGrey70;
         col.a = alpha;
         gl_printRaw( &gl_smallFont, tx, ty, &col, -1, buf );
      }
   }
}

/**
 * @brief Renders the map markers.
 */
static void map_renderMarkers( double x, double y, double zoom, double r, double a )
{
   for (int i=0; i<array_size(systems_stack); i++) {
      double tx, ty;
      int j, n, m;
      StarSystem *sys = system_getIndex( i );

      /* We only care about marked now. */
      if (!sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED | SYSTEM_PMARKED))
         continue;

      /* Get the position. */
      tx = x + sys->pos.x*zoom;
      ty = y + sys->pos.y*zoom;

      /* Count markers. */
      n  = (sys_isFlag(sys, SYSTEM_CMARKED)) ? 1 : 0;
      n += (sys_isFlag(sys, SYSTEM_PMARKED)) ? 1 : 0;
      n += sys->markers_plot;
      n += sys->markers_high;
      n += sys->markers_low;
      n += sys->markers_computer;

      /* Draw the markers. */
      j = 0;
      if (sys_isFlag(sys, SYSTEM_PMARKED)) { /* Notes have be first. */
         map_drawMarker( tx, ty, zoom, r, a, n, j, 5 );
         j++;
      }
      if (sys_isFlag(sys, SYSTEM_CMARKED)) {
         map_drawMarker( tx, ty, zoom, r, a, n, j, 0 );
         j++;
      }
      for (m=0; m<sys->markers_plot; m++) {
         map_drawMarker( tx, ty, zoom, r, a, n, j, 1 );
         j++;
      }
      for (m=0; m<sys->markers_high; m++) {
         map_drawMarker( tx, ty, zoom, r, a, n, j, 2 );
         j++;
      }
      for (m=0; m<sys->markers_low; m++) {
         map_drawMarker( tx, ty, zoom, r, a, n, j, 3 );
         j++;
      }
      for (m=0; m<sys->markers_computer; m++) {
         map_drawMarker( tx, ty, zoom, r, a, n, j, 4 );
         j++;
      }
   }
}

/*
 * Makes all systems dark grey.
 */
static void map_renderSysBlack( double bx, double by, double x, double y, double zoom, double w, double h, double r, int editor )
{
   for (int i=0; i<array_size(systems_stack); i++) {
      double tx,ty;
      glColour ccol;
      StarSystem *sys = system_getIndex( i );

      if (sys_isFlag(sys,SYSTEM_HIDDEN))
         continue;

      /* if system is not known, reachable, or marked. and we are not in the editor */
      if ((!sys_isKnown(sys) && !sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
           && !space_sysReachable(sys)) && !editor)
         continue;

      tx = x + sys->pos.x*zoom;
      ty = y + sys->pos.y*zoom;

      /* Skip if out of bounds. */
      if (!rectOverlap(tx - r, ty - r, r, r, bx, by, w, h))
         continue;

      /* If system is known fill it. */
      if ((sys_isKnown(sys)) && (system_hasSpob(sys))) {
         ccol = cGrey10;
         gl_renderCircle( tx, ty, r, &ccol, 1 );
      }
   }
}

/*
 * Renders the economy information
 */
void map_renderCommod( double bx, double by, double x, double y,
      double zoom, double w, double h, double r, int editor, double a )
{
   Commodity *c;
   glColour ccol;
   double best,worst,maxPrice,minPrice,curMaxPrice,curMinPrice,thisPrice;

   /* If not plotting commodities, return */
   if (cur_commod == -1 || map_selected == -1 || commod_known == NULL)
      return;

   c = commod_known[cur_commod];
   if (cur_commod_mode == 1) { /*showing price difference to selected system*/
      StarSystem *sys = system_getIndex( map_selected );
      /* Get commodity price in selected system.  If selected system is current
         system, and if landed, then get price of commodity where we are */
      curMaxPrice = 0.;
      curMinPrice = 0.;
      if (sys == cur_system && landed) {
         int k;
         for (k=0; k<array_size(land_spob->commodities); k++) {
            if (land_spob->commodities[k] == c) {
               /* current spob has the commodity of interest */
               curMinPrice = land_spob->commodityPrice[k].sum / land_spob->commodityPrice[k].cnt;
               curMaxPrice = curMinPrice;
               break;
            }
         }
         if (k==array_size(land_spob->commodities)) { /* commodity of interest not found */
            map_renderCommodIgnorance( x, y, zoom, sys, c, a );
            map_renderSysBlack( bx, by, x, y, zoom, w, h, r, editor );
            return;
         }
      }
      else {
         /* not currently landed, so get max and min price in the selected system. */
         if ((sys_isKnown(sys)) && (system_hasSpob(sys))) {
            minPrice = HUGE_VAL;
            maxPrice = 0;
            for (int j=0; j<array_size(sys->spobs); j++) {
               Spob *p = sys->spobs[j];
               for (int k=0; k<array_size(p->commodities); k++) {
                  if (p->commodities[k] != c)
                     continue;
                  if (p->commodityPrice[k].cnt <= 0) /* commodity is not known about */
                     continue;
                  thisPrice = p->commodityPrice[k].sum / p->commodityPrice[k].cnt;
                  maxPrice = MAX( thisPrice, maxPrice );
                  minPrice = MIN( thisPrice, minPrice );
                  break;
               }

            }
            if (maxPrice == 0) { /* no prices are known here */
               map_renderCommodIgnorance( x, y, zoom, sys, c, a );
               map_renderSysBlack( bx, by, x, y, zoom, w, h, r, editor );
               return;
            }
            curMaxPrice = maxPrice;
            curMinPrice = minPrice;
         }
         else {
            map_renderCommodIgnorance( x, y, zoom, sys, c, a );
            map_renderSysBlack( bx, by, x, y, zoom, w, h, r, editor );
            return;
         }
      }
      for (int i=0; i<array_size(systems_stack); i++) {
         double tx, ty;
         sys = system_getIndex( i );

         if (sys_isFlag(sys,SYSTEM_HIDDEN))
            continue;

         /* if system is not known, reachable, or marked. and we are not in the editor */
         if ((!sys_isKnown(sys) && !sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
              && !space_sysReachable(sys)) && !editor)
            continue;

         tx = x + sys->pos.x*zoom;
         ty = y + sys->pos.y*zoom;

         /* Skip if out of bounds. */
         if (!rectOverlap(tx - r, ty - r, r, r, bx, by, w, h))
            continue;

         /* If system is known fill it. */
         if ((sys_isKnown(sys)) && (system_hasSpob(sys))) {
            minPrice = HUGE_VAL;
            maxPrice = 0;
            for (int j=0; j<array_size(sys->spobs); j++) {
               Spob *p = sys->spobs[j];
               for (int k=0; k<array_size(p->commodities); k++) {
                  if (p->commodities[k] != c)
                     continue;
                  if (p->commodityPrice[k].cnt <= 0) /*commodity is not known about */
                     continue;
                  thisPrice = p->commodityPrice[k].sum / p->commodityPrice[k].cnt;
                  maxPrice = MAX( thisPrice, maxPrice );
                  minPrice = MIN( thisPrice, minPrice );
                  break;
               }
            }

            /* Calculate best and worst profits */
            if (maxPrice > 0) {
               /* Commodity sold at this system */
               best = maxPrice - curMinPrice;
               worst= minPrice - curMaxPrice;
               if (best >= 0) { /* draw circle above */
                  ccol = cLightBlue;
                  ccol.a = a;
                  gl_print(&gl_smallFont, x + (sys->pos.x+11) * zoom, y + (sys->pos.y-22)*zoom, &ccol, "%.1f",best);
                  best = tanh ( 2*best / curMinPrice );
                  col_blend( &ccol, &cFontBlue, &cFontYellow, best );
                  ccol.a = a;
                  gl_renderCircle( tx, ty /*+ r*/ , /*(0.1 + best) **/ r, &ccol, 1 );
               }
               else {/* draw circle below */
                  ccol = cOrange;
                  ccol.a = a;
                  gl_print(&gl_smallFont, x + (sys->pos.x+12) * zoom, y + (sys->pos.y)*zoom-gl_smallFont.h*0.5, &ccol, _("%.1f ¤"),worst);
                  worst = tanh ( -2*worst/ curMaxPrice );
                  col_blend( &ccol, &cFontOrange, &cFontYellow, worst );
                  ccol.a = a;
                  gl_renderCircle( tx, ty /*- r*/ , /*(0.1 - worst) **/ r, &ccol, 1 );
               }
            }
            else {
               /* Commodity not sold here */
               ccol = cGrey10;
               ccol.a = a;
               gl_renderCircle( tx, ty, r, &ccol, 1 );
            }
         }
      }
   }
   else { /* cur_commod_mode == 0, showing actual prices */
      /* First calculate av price in all systems
       * This has already been done in map_update_commod_av_price
       * Now display the costs */
      for (int i=0; i<array_size(systems_stack); i++) {
         double tx, ty;
         StarSystem *sys = system_getIndex( i );

         if (sys_isFlag(sys,SYSTEM_HIDDEN))
            continue;

         /* if system is not known, reachable, or marked. and we are not in the editor */
         if ((!sys_isKnown(sys) && !sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
              && !space_sysReachable(sys)) && !editor)
            continue;

         tx = x + sys->pos.x*zoom;
         ty = y + sys->pos.y*zoom;

         /* Skip if out of bounds. */
         if (!rectOverlap(tx - r, ty - r, r, r, bx, by, w, h))
            continue;

         /* If system is known fill it. */
         if ((sys_isKnown(sys)) && (system_hasSpob(sys))) {
            double sumPrice = 0;
            int sumCnt = 0;
            for (int j=0; j<array_size(sys->spobs); j++) {
               Spob *p = sys->spobs[j];
               for (int k=0; k<array_size(p->commodities); k++) {
                  if (p->commodities[k] != c)
                     continue;
                  if (p->commodityPrice[k].cnt <= 0) /* commodity is not known about */
                     continue;
                  thisPrice = p->commodityPrice[k].sum / p->commodityPrice[k].cnt;
                  sumPrice += thisPrice;
                  sumCnt += 1;
                  break;
               }
            }

            if (sumCnt > 0) {
               /* Commodity sold at this system */
               /* Colour as a % of global average */
               double frac;
               sumPrice /= sumCnt;
               if (sumPrice < commod_av_gal_price) {
                  frac = tanh(5*(commod_av_gal_price / sumPrice - 1));
                  col_blend( &ccol, &cFontOrange, &cFontYellow, frac );
               }
               else {
                  frac = tanh(5*(sumPrice / commod_av_gal_price - 1));
                  col_blend( &ccol, &cFontBlue, &cFontYellow, frac );
               }
               ccol.a = a;
               gl_print(&gl_smallFont, x + (sys->pos.x+12)*zoom, y + (sys->pos.y)*zoom - gl_smallFont.h*0.5, &ccol, _("%.1f ¤"),sumPrice);
               gl_renderCircle( tx, ty, r, &ccol, 1 );
            }
            else {
               /* Commodity not sold here */
               ccol = cGrey10;
               ccol.a = a;
               gl_renderCircle( tx, ty, r, &ccol, 1 );
            }
         }
      }
   }
}

/*
 * Renders the economy information.
 */
static void map_renderCommodIgnorance( double x, double y, double zoom,
      const StarSystem *sys, const Commodity *c, double a )
{
   int textw;
   char buf[80], *line2;
   size_t charn;
   glColour col = cFontRed;
   col.a = a;

   snprintf( buf, sizeof(buf), _("No price info for\n%s here"), _(c->name) );
   line2 = u8_strchr( buf, '\n', &charn );
   if (line2 != NULL) {
      *line2++ = '\0';
      textw = gl_printWidthRaw( &gl_smallFont, line2 );
      gl_printRaw( &gl_smallFont, x + (sys->pos.x)*zoom - textw/2, y + (sys->pos.y-15)*zoom, &col, -1, line2 );
   }
   textw = gl_printWidthRaw( &gl_smallFont, buf );
   gl_printRaw( &gl_smallFont,x + sys->pos.x *zoom- textw/2, y + (sys->pos.y+10)*zoom, &col, -1, buf );
}

static int factionPresenceCompare( const void *a, const void *b )
{
   FactionPresence *fpa, *fpb;
   fpa = (FactionPresence*) a;
   fpb = (FactionPresence*) b;
   if (fpa->value < fpb->value)
      return 1;
   else if (fpb->value < fpa->value)
      return -1;
   return strcmp( fpa->name, fpb->name );
}
/**
 * @brief Updates a text widget with a system's presence info.
 *
 *    @param wid Window to which the text widget belongs.
 *    @param name Name of the text widget.
 *    @param sys System whose faction presence we're reporting.
 *    @param omniscient Whether to display complete information (editor view).
 *                      (As currently interpreted, this also means un-translated, even if the user isn't using English.)
 */
void map_updateFactionPresence( const unsigned int wid, const char *name, const StarSystem *sys, int omniscient )
{
   size_t l;
   char   buf[STRMAX_SHORT] = {'\0'};
   FactionPresence *presence;

   /* Build the faction presence array. */
   presence = array_create( FactionPresence );
   for (int i=0; i<array_size(sys->presence); i++) {
      int matched;
      FactionPresence fp;
      if (sys->presence[i].value <= 0.)
         continue;

      /* Determine properties. */
      fp.known = 1;
      if (!omniscient && !faction_isKnown( sys->presence[i].faction )) {
         fp.name  = N_("Unknown");
         fp.known = 0;
      }
      else if (omniscient)
         fp.name  = faction_name( sys->presence[i].faction );
      else
         fp.name  = faction_mapname( sys->presence[i].faction );
      fp.value = sys->presence[i].value;

      /* Try to add to existing. */
      matched = 0;
      for (int j=0; j<array_size(presence); j++) {
         if (strcmp(fp.name,presence[j].name)==0) {
            presence[j].value += fp.value;
            matched = 1;
            break;
         }
      }
      /* Insert new. */
      if (!matched)
         array_push_back( &presence, fp );
   }
   qsort( presence, array_size(presence), sizeof(FactionPresence), factionPresenceCompare );

   l = 0;
   for (int i=0; i<array_size(presence); i++) {
      char col;
      FactionPresence *p = &presence[i];
      if (faction_exists( p->name ))
         col = faction_getColourChar( faction_get(p->name) );
      else
         col = 'N';

      /* Use map grey instead of default neutral colour */
      l += scnprintf( &buf[l], sizeof(buf) - l, "%s#0%s: #%c%.0f", (l==0) ? "" : "\n",
            _(p->name), col, p->value );
   }

   if (array_size(presence)==0)
      snprintf( buf, sizeof(buf), _("None") );

   window_modifyText( wid, name, buf );

   /* Cleanup. */
   array_free(presence);
}

/**
 * @brief Called when it's de-focused.
 */
static void map_focusLose( unsigned int wid, const char* wgtname )
{
   (void) wgtname;
   CstMapWidget *cst = map_globalCustomData( wid );
   cst->drag = 0;
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
static int map_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, double rx, double ry, void *data )
{
   (void) rx;
   (void) ry;
   CstMapWidget *cst = data;

   const double t = 15.*15.; /* threshold */

   switch (event->type) {
   case SDL_MOUSEWHEEL:
      /* Must be in bounds. */
      if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
         return 0;
      if (event->wheel.y > 0)
         map_buttonZoom( wid, "btnZoomIn" );
      else if (event->wheel.y < 0)
         map_buttonZoom( wid, "btnZoomOut" );
      return 1;

   case SDL_MOUSEBUTTONDOWN:
      /* Must be in bounds. */
      if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
         return 0;
      window_setFocus( wid, "cstMap" );

      /* selecting star system */
      mx -= w/2 - cst->xpos;
      my -= h/2 - cst->ypos;
      cst->drag = 1;

      for (int i=0; i<array_size(systems_stack); i++) {
         double x, y;
         StarSystem *sys = system_getIndex( i );

         if (sys_isFlag(sys, SYSTEM_HIDDEN))
            continue;

         /* must be reachable */
         if (!sys_isFlag(sys, SYSTEM_MARKED | SYSTEM_CMARKED)
             && !space_sysReachable(sys))
            continue;

         /* get position */
         x = sys->pos.x * cst->zoom;
         y = sys->pos.y * cst->zoom;

         if ((pow2(mx-x)+pow2(my-y)) < t) {
            if (map_selected != -1) {
               if (sys == system_getIndex( map_selected ) && sys_isKnown(sys)) {
                  map_system_open( map_selected );
                  cst->drag = 0;
               }
            }
            map_select( sys, (SDL_GetModState() & KMOD_SHIFT) );
            break;
         }
      }
      return 1;

   case SDL_MOUSEBUTTONUP:
      cst->drag = 0;
      break;

   case SDL_MOUSEMOTION:
      if (cst->drag) {
         /* axis is inverted */
         cst->xtarget = cst->xpos -= rx;
         cst->ytarget = cst->ypos += ry;
      }
      map_mx = mx;
      map_my = my;
      break;
   }

   return 0;
}
/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void map_buttonZoom( unsigned int wid, const char* str )
{
   CstMapWidget *cst = map_globalCustomData(wid);

   /* Transform coords to normal. */
   cst->xpos /= cst->zoom;
   cst->ypos /= cst->zoom;
   cst->xtarget /= cst->zoom;
   cst->ytarget /= cst->zoom;

   /* Apply zoom. */
   if (strcmp(str,"btnZoomIn")==0) {
      cst->zoom *= 1.2;
      cst->zoom = MIN(2.5, cst->zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      cst->zoom *= 0.8;
      cst->zoom = MAX(0.5, cst->zoom);
   }

   map_setZoom( wid, cst->zoom );

   /* Transform coords back. */
   cst->xpos *= cst->zoom;
   cst->ypos *= cst->zoom;
   cst->xtarget *= cst->zoom;
   cst->ytarget *= cst->zoom;
}

/**
 * @brief Generates the list of map modes, i.e. commodities that have been seen so far.
 */
static void map_genModeList(void)
{
   int totGot = 0;
   const char *odd_template, *even_template;

   map_onClose( 0, NULL );  /* so commod_known, map_modes are freed */
   commod_known = calloc( commodity_getN(), sizeof(Commodity*) );
   for (int i=0; i<array_size(systems_stack); i++) {
      StarSystem *sys = system_getIndex( i );
      for (int j=0 ; j<array_size(sys->spobs); j++) {
         Spob *p = sys->spobs[j];
         for (int k=0; k<array_size(p->commodities); k++) {
            if (p->commodityPrice[k].cnt > 0 ) {/*commodity is known about*/
               int l;
               /* find out which commodity this is */
               for (l=0 ; l<totGot; l++) {
                  if ( p->commodities[k] == commod_known[l] )
                     break;
               }
               if (l == totGot) {
                  commod_known[totGot] = p->commodities[k];
                  totGot++;
               }
            }
         }
      }
   }
   map_modes = array_create_size( char*, 2*totGot + 1 );
   array_push_back( &map_modes, strdup(_("Travel (Default)")) );
   array_push_back( &map_modes, strdup(_("Discovery")) );

   even_template = _("%s: Cost");
   odd_template = _("%s: Trade");
   for (int i=0; i<totGot; i++) {
      const char *commod_text = _(commod_known[i]->name);
      SDL_asprintf( &array_grow( &map_modes ), even_template, commod_text );
      SDL_asprintf( &array_grow( &map_modes ), odd_template, commod_text );
   }
}

/**
 * @brief Updates the map mode list.  This is called when the map update list is clicked.
 *    Unfortunately, also called when scrolled.
 *    @param wid Window of the map window.
 *    @param str Unused.
 */
static void map_modeUpdate( unsigned int wid, const char* str )
{
   (void) str;
   if (listMapModeVisible==2) {
      listMapModeVisible=1;
   }
   else if (listMapModeVisible == 1) {
      int listpos = toolkit_getListPos( wid, "lstMapMode" );
      /* TODO: make this more robust. */
      if (listpos == 0) {
         map_mode = MAPMODE_TRAVEL;
         //cur_commod = -1;
         //cur_commod_mode = 0;
      }
      else if (listpos == 1) {
         map_mode = MAPMODE_DISCOVER;
         //cur_commod = -1;
         //cur_commod_mode = 0;
      }
      else {
         map_mode = MAPMODE_TRADE;
         cur_commod = (listpos - MAPMODE_TRADE) / 2;
         cur_commod_mode = (listpos - MAPMODE_TRADE) % 2 ; /* if 0, showing cost, if 1 showing difference */
      }
   }
   map_update(wid);
}

/**
 * @brief Double click the map mode widget.
 */
static void map_modeActivate( unsigned int wid, const char* str )
{
   map_modeUpdate( wid, str );
   listMapModeVisible = 0;
   window_destroyWidget( wid, str );
}

static void map_setMinimal( unsigned int wid, int value )
{
   map_minimal_mode = value;
   player.map_minimal = value;
   window_buttonCaption( wid, "btnMinimal", (value) ? _("Normal View") : _("Minimal View") );
}

/**
 * @brief Mark current system.
 */
static void map_buttonMarkSystem( unsigned int wid, const char* str )
{
   (void) wid;
   (void) str;
   if (map_selected != -1) {
      StarSystem *sys = system_getIndex( map_selected );

      /* Remove old note */
      if (sys->note != NULL) {
         free(sys->note);
         sys->note = NULL;
      }

      /* Switch marking */
      if (sys_isFlag(sys, SYSTEM_PMARKED))
         sys_rmFlag(sys, SYSTEM_PMARKED);
      else {
         sys->note = dialogue_input(_("Add System Note"), 0, 60, _("Write a note about the #o%s#0 system:"), sys_isKnown(sys) ? _(sys->name) : _("Unknown") );
         if (sys->note != NULL)
            sys_setFlag(sys, SYSTEM_PMARKED);
      }
   }
}

/**
 * @brief Open system info.
 */
static void map_buttonSystemMap( unsigned int wid, const char* str )
{
   (void) wid;
   (void) str;
   if (map_selected != -1)
      if (sys_isKnown(system_getIndex( map_selected )))
         map_system_open( map_selected );
}

/**
 * @brief Toggle map minimal mode.
 */
static void map_buttonMinimal( unsigned int wid, const char* str )
{
   (void) str;
   map_setMinimal( wid, !map_minimal_mode );
}

/**
 * @brief Handles the button commodity clicks.
 *
 *    @param wid Window widget.
 *    @param str Name of the button creating the event.
 */
static void map_buttonCommodity( unsigned int wid, const char* str )
{
   (void) str;
   SDL_Keymod mods;
   char **this_map_modes;
   static int cur_commod_last = 0;
   static int cur_commod_mode_last = 0;
   static int map_mode_last = MAPMODE_TRAVEL;
   int defpos;
   /* Clicking the mode button - by default will show (or remove) the list of map modes.
      If ctrl is pressed, will toggle between current mode and default */
   mods = SDL_GetModState();
   if (mods & (KMOD_LCTRL | KMOD_RCTRL)) {/* toggle on/off */
      if (map_mode == MAPMODE_TRAVEL) {
         map_mode = map_mode_last;
         cur_commod = cur_commod_last;
         if (cur_commod == -1)
            cur_commod = 0;
         cur_commod_mode = cur_commod_mode_last;
      } else {
         map_mode_last = map_mode;
         map_mode = MAPMODE_TRAVEL;
         cur_commod_last = cur_commod;
         cur_commod_mode_last = cur_commod_mode;
         cur_commod = -1;
      }
      if (cur_commod >= (array_size(map_modes)-1)/2 )
         cur_commod = -1;
      /* And hide the list if it was visible. */
      if (listMapModeVisible) {
         listMapModeVisible = 0;
         window_destroyWidget( wid, "lstMapMode" );
      }
      map_update(wid);
   } else {/* no keyboard modifier */
      if (listMapModeVisible) {/* Hide the list widget */
         listMapModeVisible = 0;
         window_destroyWidget( wid, "lstMapMode" );
      } else {/* show the list widget */
         this_map_modes = calloc( sizeof(char*), array_size(map_modes) );
         for (int i=0; i<array_size(map_modes);i++) {
            this_map_modes[i]=strdup(map_modes[i]);
         }
         listMapModeVisible = 2;
         if (map_mode == MAPMODE_TRAVEL)
            defpos = 0;
         else if (map_mode == MAPMODE_DISCOVER)
            defpos = 1;
         else
            defpos = cur_commod*2 + MAPMODE_TRADE - cur_commod_mode;

         window_addList( wid, -10, 60, 200, 200, "lstMapMode",
               this_map_modes, array_size(map_modes), defpos, map_modeUpdate, map_modeActivate );
      }
   }
}

/**
 * @brief Cleans up the map stuff.
 */
static void map_onClose( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;
   free( commod_known );
   commod_known = NULL;
   for (int i=0; i<array_size(map_modes); i++)
      free( map_modes[i] );
   array_free( map_modes );
   map_modes = NULL;
}

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
   unsigned int wid = window_get(MAP_WDWNAME);
   if (wid > 0)
      window_destroy(wid);
}

/**
 * @brief Sets the map to safe defaults
 */
void map_clear (void)
{
   array_free(map_path);
   map_path = NULL;
   map_selected = -1;
}

static void map_updateInternal( CstMapWidget *cst, double dt )
{
   double dx, dy, mod, angle;
   double mapmin = 1.-map_minimal_mode;

#define AMAX(x) (x) = MIN( 1., (x) + dt )
#define AMIN(x) (x) = MAX( 0., (x) - dt )
#define ATAR(x,y) \
if ((x) < y) (x) = MIN( y, (x) + dt ); \
else (x) = MAX( y, (x) - dt )
   switch (cst->mode) {
      case MAPMODE_EDITOR: /* fall through */
      case MAPMODE_TRAVEL:
         ATAR( cst->alpha_decorators, mapmin );
         ATAR( cst->alpha_faction, mapmin );
         ATAR( cst->alpha_env, mapmin );
         AMAX( cst->alpha_path );
         AMAX( cst->alpha_names );
         AMIN( cst->alpha_commod );
         AMAX( cst->alpha_markers );
         break;

      case MAPMODE_DISCOVER:
         ATAR( cst->alpha_decorators, 0.5 * mapmin );
         ATAR( cst->alpha_faction, 0.5 * mapmin );
         ATAR( cst->alpha_env, mapmin );
         AMIN( cst->alpha_path );
         AMAX( cst->alpha_names );
         AMIN( cst->alpha_commod );
         AMAX( cst->alpha_markers );
         break;

      case MAPMODE_TRADE:
         AMIN( cst->alpha_decorators );
         AMIN( cst->alpha_faction );
         AMIN( cst->alpha_env );
         AMIN( cst->alpha_path );
         AMIN( cst->alpha_names );
         AMAX( cst->alpha_commod );
         ATAR( cst->alpha_markers, 0.5 );
         break;
   }
#undef AMAX
#undef AMIN
#undef ATAR

   dx = (cst->xtarget - cst->xpos);
   dy = (cst->ytarget - cst->ypos);
   mod = MOD(dx,dy);
   if (mod > 1e-5) {
      angle = ANGLE(dx,dy);
      /* TODO we should really do this with some nicer easing. */
      mod = MIN( mod, dt*map_flyto_speed);
      cst->xpos += mod * cos(angle);
      cst->ypos += mod * sin(angle);
   }
}

/**
 * @brief Set the steady-state values.
 */
static void map_reset( CstMapWidget* cst, MapMode mode )
{
   cst->mode = mode;
   map_updateInternal( cst, 1000. );
}

/**
 * @brief Returns the data pointer for "the" map window's widget (or NULL if map isn't open).
 */
static CstMapWidget* map_globalCustomData( unsigned int wid )
{
   if (wid==0)
      wid = window_get(MAP_WDWNAME);
   return (wid > 0) ? window_custGetData( wid, "cstMap" ) : NULL;
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
 * @brief Gets the destination system.
 *
 *    @param[out] jumps Number of jumps until the destination.
 *    @return The destination system or NULL if there is no path set.
 */
StarSystem* map_getDestination( int *jumps )
{
   if (array_size( map_path ) == 0)
      return NULL;

   if (jumps != NULL)
      *jumps = array_size( map_path );

   return array_back( map_path );
}

/**
 * @brief Updates the map after a jump.
 */
void map_jump (void)
{
   /* set selected system to self */
   map_selectCur();

   /* update path if set */
   if (array_size(map_path) != 0) {
      array_erase( &map_path, &map_path[0], &map_path[1] );
      if (array_size(map_path) == 0)
         player_targetHyperspaceSet( -1, 1 );
      else { /* get rid of bottom of the path */
         int j;
         /* set the next jump to be to the next in path */
         for (j=0; j<array_size(cur_system->jumps); j++) {
            if (map_path[0] == cur_system->jumps[j].target) {
               /* Restore selected system. */
               map_selected = array_back( map_path ) - systems_stack;

               player_targetHyperspaceSet( j, 1 );
               break;
            }
         }
         /* Overrode jump route manually, must clear target. */
         if (j>=array_size(cur_system->jumps))
            player_targetHyperspaceSet( -1, 0 );
      }
   }
   else
      player_targetHyperspaceSet( -1, 0 );

   gui_setNav();
}

/**
 * @brief Selects the system in the map.
 *
 *    @param sys System to select.
 */
void map_select( const StarSystem *sys, char shifted )
{
   int autonav;
   unsigned int wid = 0;

   if (window_exists(MAP_WDWNAME))
      wid = window_get(MAP_WDWNAME);

   if (sys == NULL) {
      map_selectCur();
      autonav = 0;
   }
   else {
      map_selected = sys - systems_stack;

      /* select the current system and make a path to it */
      if (!shifted) {
         array_free( map_path );
         map_path  = NULL;
      }

      /* Try to make path if is reachable. */
      if (space_sysReachable(sys)) {
         map_path = map_getJumpPath( cur_system->name, sys->name, 0, 1, map_path );

         if (array_size(map_path)==0) {
            player_hyperspacePreempt(0);
            player_targetHyperspaceSet( -1, 0 );
            player_autonavAbortJump(NULL);
            autonav = 0;
         }
         else  {
            /* see if it is a valid hyperspace target */
            for (int i=0; i<array_size(cur_system->jumps); i++) {
               if (map_path[0] == cur_system->jumps[i].target) {
                  player_hyperspacePreempt(1);
                  player_targetHyperspaceSet( i, 0 );
                  break;
               }
            }
            autonav = 1;
         }
      }
      else { /* unreachable. */
         player_targetHyperspaceSet( -1, 0 );
         player_autonavAbortJump(NULL);
         autonav = 0;
      }
   }

   if (wid != 0) {
      if (autonav)
         window_enableButton( wid, "btnAutonav" );
      else
         window_disableButton( wid, "btnAutonav" );
   }

   map_update(wid);
   gui_setNav();
}

/**
 * @brief Cycle and autonav the mission systems in the map.
 *
 *    @param dir -1 for prev, 1 for next.
 */
void map_cycleMissions(int dir)
{
  // StarSystem* systems_stack = system_getAll();
   StarSystem *dest = map_getDestination( NULL );
   StarSystem *target;
   int found_next_i = -1;
   int found_prev_i = -1;
   int found_b = 0;
   int i;

   /* Select current selection - do nothing */
   if (dir==0)
      return;

   /* Default : points to current system */
   if (dest==NULL)
      dest = cur_system;

   /* Universally find prev and next mission system */
   for (i=0;i<array_size(systems_stack);i++) {
      if (!sys_isFlag(&systems_stack[i], SYSTEM_MARKED | SYSTEM_PMARKED) || !space_sysReachable(&systems_stack[i]) )
         continue;

      /* Pre-select first in case we will wrap */
      if (found_next_i<0)
         found_next_i = i;

      /* We found next system */
      if (found_b) {
         found_next_i = i;
         break;
      }

      /* We found currently selected system */
      if (&systems_stack[i]==dest)
         found_b=1;
      else
         found_prev_i = i; /* Follow trail as we go */
   }

   /* No trail for prev system - current one was first one in list - just finish loop and find last one */
   if (found_prev_i<0)
      for (;i<array_size(systems_stack);i++)
         if (sys_isMarked(&systems_stack[i]) && space_sysReachable(&systems_stack[i]))
               found_prev_i = i;

   /* Select found system or return if no suitable was found */
   if (dir>0 && found_next_i>=0)
      target = &systems_stack[found_next_i];
   else if (dir<0 && found_prev_i>=0)
      target = &systems_stack[found_prev_i];
   else
      return;

   /* Select and center system. */
   map_select( target, 0 );
   map_center( window_get( MAP_WDWNAME ), target->name );

   /* Autonav to selected system */
   //player_hyperspacePreempt( 1 );
   //player_autonavStart();
}

/**
 * @brief Toggle note-showing on/off.
 */
void map_toggleNotes()
{
   map_show_notes = !map_show_notes;
}
/*
 * A* algorithm for shortest path finding
 *
 * Note since that we can't actually get an admissible heurestic for A* this is
 * in reality just Djikstras. I've removed the heurestic bit to make sure I
 * don't try to implement an admissible heuristic when I'm pretty sure there is
 * none.
 */
/**
 * @brief Node structure for A* pathfinding.
 */
typedef struct SysNode_ {
   struct SysNode_ *next; /**< Next node */
   struct SysNode_ *gnext; /**< Next node in the garbage collector. */

   struct SysNode_ *parent; /**< Parent node. */
   StarSystem* sys; /**< System in node. */
   int g; /**< step */
} SysNode; /**< System Node for use in A* pathfinding. */
static SysNode *A_gc;
/* prototypes */
static SysNode* A_newNode( StarSystem* sys );
static int A_g( SysNode* n );
static SysNode* A_add( SysNode *first, SysNode *cur );
static SysNode* A_rm( SysNode *first, StarSystem *cur );
static SysNode* A_in( SysNode *first, StarSystem *cur );
static SysNode* A_lowest( SysNode *first );
static void A_freeList( SysNode *first );
static int map_decorator_parse( MapDecorator *temp, const char *file );
/** @brief Creates a new node link to star system. */
static SysNode* A_newNode( StarSystem* sys )
{
   SysNode *n = malloc(sizeof(SysNode));

   n->next  = NULL;
   n->sys   = sys;

   n->gnext = A_gc;
   A_gc     = n;

   return n;
}
/** @brief Gets the g from a node. */
static int A_g( SysNode* n )
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
      if (n->g < lowest->g)
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
      free(p);
      p = n;
   } while ((n=n->gnext) != NULL);
   free(p);
}

/** @brief Sets map_zoom to zoom and recreates the faction disk texture. */
void map_setZoom( unsigned int wid, double zoom )
{
   CstMapWidget *cst = map_globalCustomData(wid);
   cst->zoom = zoom;
}

/**
 * @brief Gets the jump path between two systems.
 *
 *    @param sysstart Name of the system to start from.
 *    @param sysend Name of the system to end at.
 *    @param ignore_known Whether or not to ignore if systems and jump points are known.
 *    @param show_hidden Whether or not to use hidden jumps points.
 *    @param old_data the old path (if we're merely extending)
 *    @return Array (array.h): the systems in the path. NULL on failure.
 */
StarSystem** map_getJumpPath( const char* sysstart, const char* sysend,
    int ignore_known, int show_hidden, StarSystem** old_data )
{
   int j, cost, njumps, ojumps;
   StarSystem *ssys, *esys, **res;

   SysNode *cur,   *neighbour;
   SysNode *open,  *closed;
   SysNode *ocost, *ccost;

   A_gc = NULL;
   res = old_data;
   ojumps = array_size( old_data );

   /* initial and target systems */
   ssys = system_get(sysstart); /* start */
   esys = system_get(sysend); /* goal */

   /* Set up. */
   if (ojumps > 0)
      ssys = system_get( array_back( old_data )->name );

   /* Check self. */
   if (ssys==esys || array_size(ssys->jumps)==0) {
      array_free( res );
      return NULL;
   }

   /* system target must be known and reachable */
   if (!ignore_known && !sys_isKnown(esys) && !space_sysReachable(esys)) {
      /* can't reach - don't make path */
      array_free( res );
      return NULL;
   }

   /* start the linked lists */
   open     = closed = NULL;
   cur      = A_newNode( ssys );
   cur->parent = NULL;
   cur->g   = 0;
   open     = A_add( open, cur ); /* Initial open node is the start system */

   j = 0;
   while ((cur = A_lowest(open))) {
      /* End condition. */
      if (cur->sys == esys)
         break;

      /* Break if infinite loop. */
      j++;
      if (j > MAP_LOOP_PROT)
         break;

      /* Get best from open and toss to closed */
      open   = A_rm( open, cur->sys );
      closed = A_add( closed, cur );
      cost   = A_g(cur) + 1; /* Base unit is jump and always increases by 1. */

      for (int i=0; i<array_size(cur->sys->jumps); i++) {
         JumpPoint *jp  = &cur->sys->jumps[i];
         StarSystem *sys = jp->target;

         /* Make sure it's reachable */
         if (!ignore_known) {
            if (!jp_isKnown(jp))
               continue;
            if (!sys_isKnown(sys) && !space_sysReachable(sys))
               continue;
         }
         if (jp_isFlag( jp, JP_EXITONLY ))
            continue;

         /* Skip hidden jumps if they're not specifically requested */
         if (!show_hidden && jp_isFlag( jp, JP_HIDDEN ))
            continue;

         /* Check to see if it's already in the closed set. */
         ccost = A_in(closed, sys);
         if ((ccost != NULL) && (cost >= A_g(ccost)))
            continue;
            //closed = A_rm( closed, sys );

         /* Remove if it exists and current is better. */
         ocost = A_in(open, sys);
         if (ocost != NULL) {
            if (cost < A_g(ocost))
               open = A_rm( open, sys ); /* New path is better */
            else
               continue; /* This node is worse, so ignore it. */
         }

         /* Create the node. */
         neighbour         = A_newNode( sys );
         neighbour->parent = cur;
         neighbour->g      = cost;
         open              = A_add( open, neighbour );
      }

      /* Safety check in case not linked. */
      if (open == NULL)
         break;
   }

   /* Build path backwards if not broken from loop. */
   if (cur != NULL && esys == cur->sys) {
      njumps = A_g(cur) + ojumps;
      assert( njumps > ojumps );
      if (res == NULL)
         res = array_create_size( StarSystem*, njumps );
      array_resize( &res, njumps );
      /* Build path. */
      for (int i=0; i<njumps-ojumps; i++) {
         res[njumps-i-1] = cur->sys;
         cur = cur->parent;
      }
   }
   else {
      res = NULL;
      array_free( old_data );
   }

   /* free the linked lists */
   A_freeList(A_gc);
   return res;
}

/**
 * @brief Marks maps around a radius of currently system as known.
 *
 *    @param targ_sys System at center of the "known" circle.
 *    @param r Radius (in jumps) to mark as known.
 *    @return 0 on success.
 */
int map_map( const Outfit *map )
{
   for (int i=0; i<array_size(map->u.map->systems);i++)
      sys_setFlag(map->u.map->systems[i], SYSTEM_KNOWN);

   for (int i=0; i<array_size(map->u.map->spobs);i++)
      spob_setKnown(map->u.map->spobs[i]);

   for (int i=0; i<array_size(map->u.map->jumps);i++)
      jp_setFlag(map->u.map->jumps[i], JP_KNOWN);

   return 1;
}

/**
 * @brief Check to see if map data is limited to locations which are known
 *        or in a nonexistent status for plot reasons.
 *
 *    @param map Map outfit to check.
 *    @return 1 if already mapped, 0 if it wasn't.
 */
int map_isUseless( const Outfit* map )
{
   for (int i=0; i<array_size(map->u.map->systems);i++)
      if (!sys_isKnown(map->u.map->systems[i]))
         return 0;

   for (int i=0; i<array_size(map->u.map->spobs);i++) {
      Spob *p = map->u.map->spobs[i];
      if (!spob_hasSystem( p ) )
         continue;
      if (!spob_isKnown(p))
         return 0;
   }

   for (int i=0; i<array_size(map->u.map->jumps);i++)
      if (!jp_isKnown(map->u.map->jumps[i]))
         return 0;

   return 1;
}

/**
 * @brief Maps a local map.
 */
int localmap_map( const Outfit *lmap )
{
   double detect, mod;

   if (cur_system==NULL)
      return 0;

   mod = pow2( 200. / (cur_system->interference + 200.) );

   detect = lmap->u.lmap.jump_detect;
   for (int i=0; i<array_size(cur_system->jumps); i++) {
      JumpPoint *jp = &cur_system->jumps[i];
      if (jp_isFlag(jp, JP_EXITONLY) || jp_isFlag(jp, JP_HIDDEN))
         continue;
      if (mod*jp->hide <= detect)
         jp_setFlag( jp, JP_KNOWN );
   }

   detect = lmap->u.lmap.spob_detect;
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *p = cur_system->spobs[i];
      if (!spob_hasSystem( p ) )
         continue;
      if (mod*p->hide <= detect)
         spob_setKnown( p );
   }
   return 0;
}

/**
 * @brief Checks to see if the local map is limited to locations which are known
 *        or in a nonexistent status for plot reasons.
 */
int localmap_isUseless( const Outfit *lmap )
{
   double detect, mod;

   if (cur_system==NULL)
      return 1;

   mod = pow2( 200. / (cur_system->interference + 200.) );

   detect = lmap->u.lmap.jump_detect;
   for (int i=0; i<array_size(cur_system->jumps); i++) {
      JumpPoint *jp = &cur_system->jumps[i];
      if (jp_isFlag(jp, JP_EXITONLY) || jp_isFlag(jp, JP_HIDDEN))
         continue;
      if ((mod*jp->hide <= detect) && !jp_isKnown( jp ))
         return 0;
   }

   detect = lmap->u.lmap.spob_detect;
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *p = cur_system->spobs[i];
      if ((mod*p->hide <= detect) && !spob_isKnown( p ))
         return 0;
   }
   return 1;
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
 *    @param xoff X offset when centering.
 *    @param yoff Y offset when centering.
 */
void map_show( int wid, int x, int y, int w, int h, double zoom, double xoff, double yoff )
{
   CstMapWidget *cst = calloc( 1, sizeof(CstMapWidget) );

   /* New widget. */
   window_addCust( wid, x, y, w, h,
         "cstMap", 1, map_render, map_mouse, NULL, map_focusLose, cst );
   window_custAutoFreeData( wid, "cstMap" );

   /* Set up stuff. */
   map_setup();

   /* Centering stuff. */
   cst->xoff = xoff;
   cst->yoff = yoff;

   /* Set position to focus on current system. */
   cst->xtarget = cst->xpos = cur_system->pos.x * zoom + cst->xoff;
   cst->ytarget = cst->ypos = cur_system->pos.y * zoom + cst->yoff;

   /* Set zoom. */
   map_setZoom( wid, zoom );

   map_reset( cst, MAPMODE_TRAVEL );
}

/**
 * @brief Centers the map on a spob.
 *
 *    @param wid ID of the window with the map widget, or 0 for "the" map window.
 *    @param sys System to center the map on (internal name).
 *    @return 0 on success.
 */
int map_center( int wid, const char *sys )
{
   double d;
   CstMapWidget *cst = map_globalCustomData( wid );

   /* Get the system. */
   StarSystem *ssys = system_get( sys );
   if (ssys == NULL)
      return -1;

   /* Center on the system. */
   cst->xtarget = ssys->pos.x * cst->zoom + cst->xoff;
   cst->ytarget = ssys->pos.y * cst->zoom + cst->yoff;

   /* Compute flyto speed. */
   d = MOD( cst->xtarget-cst->xpos, cst->ytarget-cst->ypos );
   map_flyto_speed = MIN( 2000., d / 0.2 );

   return 0;
}

/**
 * @brief Loads all the map decorators.
 *
 *    @return 0 on success.
 */
int map_load (void)
{
   Uint32 time = SDL_GetTicks();
   char **decorator_files = ndata_listRecursive( MAP_DECORATOR_DATA_PATH );

   decorator_stack = array_create( MapDecorator );
   for (int i=0; i<array_size(decorator_files); i++) {
      MapDecorator temp;
      int ret = map_decorator_parse( &temp, decorator_files[i] );
      if (ret == 0)
         array_push_back( &decorator_stack, temp );
      free( decorator_files[i] );
   }
   array_free( decorator_files );

   if (conf.devmode) {
      time = SDL_GetTicks() - time;
      DEBUG( n_( "Loaded %d map decorator in %.3f s", "Loaded %d map decorators in %.3f s", array_size(decorator_stack) ), array_size(decorator_stack), time/1000. );
   }
   else
      DEBUG( n_( "Loaded %d map decorator", "Loaded %d map decorators", array_size(decorator_stack) ), array_size(decorator_stack) );

   return 0;
}

static int map_decorator_parse( MapDecorator *temp, const char *file )
{
   xmlDocPtr doc;
   xmlNodePtr node, parent;

   doc = xml_parsePhysFS( file );
   if (doc == NULL)
      return -1;

   parent = doc->xmlChildrenNode; /* map node */
   if (strcmp((char*)parent->name,"decorator")) {
      ERR(_("Malformed %s file: missing root element 'decorator'"), file );
      return -1;
   }

   /* Clear memory. */
   memset( temp, 0, sizeof(MapDecorator) );

   temp->detection_radius = 10;

   /* Parse body. */
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      xmlr_float(node, "x", temp->x);
      xmlr_float(node, "y", temp->y);
      xmlr_int(node, "detection_radius", temp->detection_radius);
      if (xml_isNode(node,"image")) {
         temp->image = xml_parseTexture( node,
               MAP_DECORATOR_GFX_PATH"%s", 1, 1, OPENGL_TEX_MIPMAPS );

         if (temp->image == NULL)
            WARN(_("Could not load map decorator texture '%s'."), xml_get(node));

         continue;
      }
      WARN(_("Map decorator has unknown node '%s'."), node->name);
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);

   return 0;
}
