/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file dev_uniedit.c
 *
 * @brief Handles the star system editor.
 */
/** @cond */
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "dev_uniedit.h"

#include "array.h"
#include "conf.h"
#include "dev_spob.h"
#include "dev_sysedit.h"
#include "dev_system.h"
#include "dialogue.h"
#include "economy.h"
#include "map.h"
#include "map_find.h"
#include "ndata.h"
#include "nstring.h"
#include "opengl.h"
#include "pause.h"
#include "safelanes.h"
#include "space.h"
#include "tk/toolkit_priv.h"
#include "toolkit.h"
#include "unidiff.h"

#define BUTTON_WIDTH 100 /**< Map button width. */
#define BUTTON_HEIGHT 30 /**< Map button height. */

#define UNIEDIT_EDIT_WIDTH 400  /**< System editor width. */
#define UNIEDIT_EDIT_HEIGHT 450 /**< System editor height. */

#define UNIEDIT_FIND_WIDTH 400  /**< System editor width. */
#define UNIEDIT_FIND_HEIGHT 500 /**< System editor height. */

#define UNIEDIT_DRAG_THRESHOLD 300        /**< Drag threshold. */
#define UNIEDIT_MOVE_THRESHOLD 10         /**< Movement threshold. */
#define UNIEDIT_CLICK_THRESHOLD 20.       /**< Click threshold (px). */
#define UNIEDIT_DOUBLECLICK_THRESHOLD 300 /**< Drag threshold (ms). */

#define UNIEDIT_ZOOM_STEP 1.2 /**< Factor to zoom by for each zoom level. */
#define UNIEDIT_ZOOM_MAX 5.   /**< Maximum uniedit zoom level (close). */
#define UNIEDIT_ZOOM_MIN -5.  /**< Minimum uniedit zoom level (far). */

/*
 * The editor modes.
 */
typedef enum UniEditMode_ {
   UNIEDIT_DEFAULT, /**< Default editor mode. */
   UNIEDIT_JUMP,    /**< Jump point toggle mode. */
   UNIEDIT_NEWSYS,  /**< New system editor mode. */
   UNIEDIT_ROTATE,  /**< Rotation mode. */

} UniEditMode;

typedef enum UniEditViewMode_ {
   UNIEDIT_VIEW_DEFAULT,
   UNIEDIT_VIEW_VIRTUALSPOBS,
   UNIEDIT_VIEW_RADIUS,
   UNIEDIT_VIEW_NOLANES,
   UNIEDIT_VIEW_BACKGROUND,
   UNIEDIT_VIEW_ASTEROIDS,
   UNIEDIT_VIEW_INTERFERENCE,
   UNIEDIT_VIEW_TECH,
   UNIEDIT_VIEW_PRESENCE_SUM,
   UNIEDIT_VIEW_PRESENCE,
} UniEditViewMode;

extern StarSystem *systems_stack;

static UniEditMode     uniedit_mode = UNIEDIT_DEFAULT; /**< Editor mode. */
static UniEditViewMode uniedit_viewmode =
   UNIEDIT_VIEW_DEFAULT;              /**< Editor view mode. */
static int uniedit_view_faction = -1; /**< Faction currently being viewed. */
static unsigned int uniedit_wid = 0;  /**< Sysedit wid. */
static unsigned int uniedit_widEdit   = 0;  /**< Sysedit editor wid. */
static unsigned int uniedit_widFind   = 0;  /**< Sysedit find wid. */
static double       uniedit_xpos      = 0.; /**< Viewport X position. */
static double       uniedit_ypos      = 0.; /**< Viewport Y position. */
static double       uniedit_zoom      = 1.; /**< Viewport zoom level. */
static int          uniedit_moved     = 0; /**< Space moved since mouse down. */
static unsigned int uniedit_lastClick = 0; /**< Time last clicked. */
static int          uniedit_drag      = 0; /**< Dragging viewport around. */
static int          uniedit_dragSys   = 0; /**< Dragging system around. */
static int          uniedit_dragSel   = 0; /**< Dragging selection box. */
static double       uniedit_dragSelX =
   0; /**< Dragging selection initial X position */
static double uniedit_dragSelY =
   0; /**< Dragging selection initial Y position */
static double       uniedit_rotate    = 0.; /**< Rotated angle (in radians). */
static double       uniedit_rotate_cx = 0.; /**< Center position of rotation. */
static double       uniedit_rotate_cy = 0.; /**< Center position of rotation. */
static StarSystem **uniedit_sys       = NULL; /**< Selected systems. */
static StarSystem  *uniedit_tsys = NULL; /**< Temporarily clicked system. */
static int uniedit_tadd = 0; /**< Temporarily clicked system should be added. */
static double uniedit_mx       = 0.;   /**< X mouse position. */
static double uniedit_my       = 0.;   /**< Y mouse position. */
static double uniedit_dt       = 0.;   /**< Deltatick. */
static char **uniedit_tagslist = NULL; /**< List of tags. */

static map_find_t *found_cur  = NULL; /**< Pointer to found stuff. */
static int         found_ncur = 0;    /**< Number of found stuff. */

/*
 * Universe editor Prototypes.
 */
/* Selection. */
static void uniedit_deselect( void );
static void uniedit_selectAdd( StarSystem *sys );
static void uniedit_selectRm( StarSystem *sys );
/* System and spob search. */
static void uniedit_findSys( void );
static void uniedit_findSysClose( unsigned int wid, const char *name );
static void uniedit_findSearch( unsigned int wid, const char *str );
static void uniedit_findShowResults( unsigned int wid, map_find_t *found,
                                     int n );
static void uniedit_centerSystem( unsigned int wid, const char *unused );
static int  uniedit_sortCompare( const void *p1, const void *p2 );
/* System editing. */
static void uniedit_editSys( void );
static void uniedit_editSysClose( unsigned int wid, const char *name );
static void uniedit_editGenList( unsigned int wid );
static void uniedit_btnEditRename( unsigned int wid, const char *unused );
static void uniedit_btnEditRmSpob( unsigned int wid, const char *unused );
static void uniedit_btnEditAddSpob( unsigned int wid, const char *unused );
static void uniedit_btnEditAddSpobAdd( unsigned int wid, const char *unused );
static void uniedit_btnViewModeSet( unsigned int wid, const char *unused );
static void uniedit_chkNolanes( unsigned int wid, const char *wgtname );
/* System renaming. */
static int  uniedit_checkName( const char *name );
static void uniedit_renameSys( void );
/* New system. */
static void uniedit_newSys( double x, double y );
/* Jump handling. */
static void uniedit_toggleJump( StarSystem *sys );
static void uniedit_jumpAdd( StarSystem *sys, StarSystem *targ );
static void uniedit_jumpRm( StarSystem *sys, StarSystem *targ );
/* Tags. */
static void uniedit_btnEditTags( unsigned int wid, const char *unused );
static void uniedit_genTagsList( unsigned int wid );
static void uniedit_btnAddTag( unsigned int wid, const char *unused );
static void uniedit_btnRmTag( unsigned int wid, const char *unused );
static void uniedit_btnNewTag( unsigned int wid, const char *unused );
static void uniedit_btnTagsClose( unsigned int wid, const char *unused );
/* Custom system editor widget. */
static void uniedit_buttonZoom( unsigned int wid, const char *str );
static void uniedit_render( double bx, double by, double w, double h,
                            void *data );
static void uniedit_renderOverlay( double bx, double by, double bw, double bh,
                                   void *data );
static void uniedit_focusLose( unsigned int wid, const char *wgtname );
static int  uniedit_mouse( unsigned int wid, const SDL_Event *event, double mx,
                           double my, double w, double h, double rx, double ry,
                           void *data );
static void uniedit_renderFactionDisks( double x, double y, double r );
static void uniedit_renderVirtualSpobs( double x, double y, double r );
/* Button functions. */
static void uniedit_close( unsigned int wid, const char *wgt );
static void uniedit_save( unsigned int wid_unused, const char *unused );
static void uniedit_btnView( unsigned int wid_unused, const char *unused );
static void uniedit_btnJump( unsigned int wid_unused, const char *unused );
static void uniedit_btnRename( unsigned int wid_unused, const char *unused );
static void uniedit_btnEdit( unsigned int wid_unused, const char *unused );
static void uniedit_btnNew( unsigned int wid_unused, const char *unused );
static void uniedit_btnOpen( unsigned int wid_unused, const char *unused );
static void uniedit_btnFind( unsigned int wid_unused, const char *unused );
/* Keybindings handling. */
static int uniedit_keys( unsigned int wid, SDL_Keycode key, SDL_Keymod mod,
                         int isrepeat );

/**
 * @brief Opens the system editor interface.
 */
void uniedit_open( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   unsigned int   wid;
   int            buttonPos = 0;
   const glColour cBG       = { 0., 0., 0., 0.95 };

   /* Pause. */
   pause_game();

   /* Must have no diffs applied. */
   diff_clear();

   /* Reset some variables. */
   uniedit_mode         = UNIEDIT_DEFAULT;
   uniedit_viewmode     = UNIEDIT_VIEW_DEFAULT;
   uniedit_view_faction = -1;
   uniedit_drag         = 0;
   uniedit_dragSys      = 0;
   uniedit_dragSel      = 0;
   uniedit_tsys         = NULL;
   uniedit_tadd         = 0;
   uniedit_zoom         = 1.;
   uniedit_xpos         = 0.;
   uniedit_ypos         = 0.;
   uniedit_dt           = 0.;

   /* Create the window. */
   wid = window_create( "wdwUniverseEditor", _( "Universe Editor" ), -1, -1, -1,
                        -1 );
   window_setDynamic( wid, 1 );
   window_handleKeys( wid, uniedit_keys );
   window_setBorder( wid, 0 );
   uniedit_wid = wid;

   /* Actual viewport, below everything. */
   window_addCust( wid, 0, 0, SCREEN_W, SCREEN_H, "cstSysEdit", 1,
                   uniedit_render, uniedit_mouse, NULL, uniedit_focusLose,
                   NULL );
   window_custSetOverlay( wid, "cstSysEdit", uniedit_renderOverlay );

   /* Overlay background. */
   window_addRect( wid, SCREEN_W - 130, 0, 130, SCREEN_H, "rctRCol", &cBG, 0 );
   window_addRect( wid, 0, 0, SCREEN_W, 60, "rctBBar", &cBG, 0 );

   /* Close button. */
   window_addButtonKey( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose", _( "Exit" ),
                        uniedit_close, SDLK_x );
   buttonPos++;

   /* Autosave toggle. */
   window_addCheckbox( wid, -150, 25, SCREEN_W / 2 - 150, 20, "chkEditAutoSave",
                       _( "Automatically save changes" ), uniedit_autosave,
                       conf.devautosave );

   /* Save button. */
   window_addButton( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                     BUTTON_WIDTH, BUTTON_HEIGHT, "btnSave", _( "Save All" ),
                     uniedit_save );
   buttonPos++;

   /* View button. */
   window_addButton( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                     BUTTON_WIDTH, BUTTON_HEIGHT, "btnView", _( "View Mode" ),
                     uniedit_btnView );
   buttonPos++;

   /* Jump toggle. */
   window_addButtonKey( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnJump", _( "Jump" ),
                        uniedit_btnJump, SDLK_j );
   buttonPos++;

   /* Rename system. */
   window_addButton( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                     BUTTON_WIDTH, BUTTON_HEIGHT, "btnRename", _( "Rename" ),
                     uniedit_btnRename );
   buttonPos++;

   /* Edit system. */
   window_addButtonKey( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnEdit", _( "Edit" ),
                        uniedit_btnEdit, SDLK_e );
   buttonPos++;

   /* New system. */
   window_addButtonKey( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnNew", _( "New Sys" ),
                        uniedit_btnNew, SDLK_n );
   buttonPos++;

   /* Open a system. */
   window_addButtonKey( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnOpen", _( "Open" ),
                        uniedit_btnOpen, SDLK_o );
   buttonPos++;

   /* Find a system or spob. */
   window_addButtonKey( wid, -20, 20 + ( BUTTON_HEIGHT + 20 ) * buttonPos,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnFind", _( "Find" ),
                        uniedit_btnFind, SDLK_f );
   buttonPos++;

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+",
                     uniedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-",
                     uniedit_buttonZoom );

   /* Nebula. */
   window_addText( wid, -10, -20, 110, 200, 0, "txtNebula", &gl_smallFont, NULL,
                   _( "N/A" ) );

   /* Presence. */
   window_addText( wid, -10, -80 - gl_smallFont.h - 5, 110, 200, 0,
                   "txtPresence", &gl_smallFont, NULL, _( "N/A" ) );

   /* Selected text. */
   window_addText( wid, 140, 10, SCREEN_W / 2 - 140, 30, 0, "txtSelected",
                   &gl_smallFont, NULL, NULL );

   /* Deselect everything. */
   uniedit_deselect();
}

/**
 * @brief Handles keybindings.
 */
static int uniedit_keys( unsigned int wid, SDL_Keycode key, SDL_Keymod mod,
                         int isrepeat )
{
   (void)wid;
   (void)isrepeat;
   int n;

   switch ( key ) {
   /* Mode changes. */
   case SDLK_ESCAPE:
      uniedit_mode = UNIEDIT_DEFAULT;
      return 1;

   case SDLK_a:
      if ( mod & ( KMOD_LCTRL | KMOD_RCTRL ) ) {
         uniedit_deselect();
         for ( int i = 0; i < array_size( systems_stack ); i++ )
            uniedit_selectAdd( &systems_stack[i] );
         return 1;
      }
      return 0;

   case SDLK_r:
      n = array_size( uniedit_sys );
      if ( n > 1 ) {
         uniedit_mode      = UNIEDIT_ROTATE;
         uniedit_rotate    = 0.; /* Initialize rotation. */
         uniedit_rotate_cx = uniedit_rotate_cy = 0.;
         for ( int i = 0; i < n; i++ ) {
            uniedit_rotate_cx += uniedit_sys[i]->pos.x;
            uniedit_rotate_cy += uniedit_sys[i]->pos.y;
         }
         uniedit_rotate_cx /= (double)n;
         uniedit_rotate_cy /= (double)n;
      }
      return 1;

   default:
      return 0;
   }
}

/**
 * @brief Closes the system editor widget.
 */
static void uniedit_close( unsigned int wid, const char *wgt )
{
   /* Frees some memory. */
   uniedit_deselect();

   /* Reconstruct jumps. */
   systems_reconstructJumps();

   /* Unpause. */
   unpause_game();

   /* Close the window. */
   window_close( wid, wgt );
}

/*
 * @brief Saves the systems.
 */
static void uniedit_save( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   dsys_saveAll();
   dpl_saveAll();
}

/*
 * @brief Toggles autosave.
 */
void uniedit_autosave( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   conf.devautosave = window_checkboxState( wid_unused, "chkEditAutoSave" );
}

/*
 * @brief Updates autosave check box.
 */
void uniedit_updateAutosave( void )
{
   window_checkboxSet( uniedit_wid, "chkEditAutoSave", conf.devautosave );
}

static int factionGenerates( int f, int tocheck, double *w )
{
   const FactionGenerator *fg = faction_generators( f );
   for ( int i = 0; i < array_size( fg ); i++ ) {
      if ( fg[i].id == tocheck ) {
         if ( w != NULL )
            *w = fg[i].weight;
         return 1;
      }
   }
   return 0;
}

/**
 * @brief Allows selecting the view.
 */
static void uniedit_btnView( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   unsigned int wid;
   int          n, h, k;
   Spob        *spobs;
   char       **str;
   int         *factions;

   /* Find usable factions. */
   factions = faction_getAll();
   spobs    = spob_getAll();
   for ( int i = 0; i < array_size( factions ); i++ ) {
      int f       = factions[i];
      int hasfact = 0;
      for ( int j = 0; j < array_size( spobs ); j++ ) {
         Spob *p = &spobs[j];
         if ( ( p->presence.faction != f ) &&
              !factionGenerates( p->presence.faction, f, NULL ) )
            continue;
         if ( p->presence.base == 0. && p->presence.bonus == 0. )
            continue;
         hasfact = 1;
         break;
      }
      if ( !hasfact )
         factions[i] = -1;
   }

   /* Create the window. */
   wid = window_create( "wdwUniEditView", _( "Select a View Mode" ), -1, -1,
                        UNIEDIT_EDIT_WIDTH, UNIEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   /* Add virtual spob list. */
   n      = 9; /* Number of special cases. */
   str    = malloc( sizeof( char    *) * ( array_size( factions ) + n ) );
   str[0] = strdup( _( "Default" ) );
   str[1] = strdup( _( "Virtual Spobs" ) );
   str[2] = strdup( _( "System Radius" ) );
   str[3] = strdup( _( "No Lanes" ) );
   str[4] = strdup( _( "Background" ) );
   str[5] = strdup( _( "Asteroids" ) );
   str[6] = strdup( _( "Interference" ) );
   str[7] = strdup( _( "Tech" ) );
   str[8] = strdup( _( "Sum of Presences" ) );
   k      = n;
   for ( int i = 0; i < array_size( factions ); i++ ) {
      int f = factions[i];
      if ( f >= 0 )
         str[k++] = strdup(
            faction_name( f ) ); /* Not translating so we can use faction_get */
   }
   qsort( &str[n], k - n, sizeof( char * ), strsort );
   h = UNIEDIT_EDIT_HEIGHT - 60 - ( BUTTON_HEIGHT + 20 );
   window_addList( wid, 20, -40, UNIEDIT_EDIT_WIDTH - 40, h, "lstViewModes",
                   str, k, 0, NULL, uniedit_btnViewModeSet );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                     _( "Close" ), window_close );

   /* Add button. */
   window_addButton( wid, -20 - ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnSet", _( "Set" ),
                     uniedit_btnViewModeSet );

   /* Clean up. */
   array_free( factions );
}

/**
 * @brief Enters the editor in new jump mode.
 */
static void uniedit_btnJump( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   uniedit_mode = UNIEDIT_JUMP;
}

/**
 * @brief Renames selected systems.
 */
static void uniedit_btnRename( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   uniedit_renameSys();
}

/**
 * @brief Enters the editor in new system mode.
 */
static void uniedit_btnNew( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   uniedit_mode = UNIEDIT_NEWSYS;
}

/**
 * @brief Opens up a system.
 */
static void uniedit_btnOpen( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   if ( array_size( uniedit_sys ) != 1 )
      return;

   sysedit_open( uniedit_sys[0] );
}

/**
 * @brief Opens the system property editor.
 */
static void uniedit_btnFind( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   uniedit_findSys();
}

/**
 * @brief Opens the system property editor.
 */
static void uniedit_btnEdit( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   uniedit_editSys();
}

static void uniedit_renderFactionDisks( double x, double y, double r )
{
   const glColour *col;
   glColour        c;
   double          tx, ty, sr, presence;

   col = faction_colour( uniedit_view_faction );
   c.r = col->r;
   c.g = col->g;
   c.b = col->b;
   c.a = 0.5;

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      StarSystem *sys = system_getIndex( i );

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      presence = system_getPresence( sys, uniedit_view_faction );

      /* draws the disk representing the faction */
      sr = 0.5 * M_PI * sqrt( presence ) * uniedit_zoom;

      // glUseProgram(shaders.factiondisk.program);
      // glUniform1f(shaders.factiondisk.paramf, r / sr );
      // gl_renderShader( tx, ty, sr, sr, 0., &shaders.factiondisk, &c, 1 );
      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderVirtualSpobs( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys = system_getIndex( i );

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* draws the disk representing the faction */
      sr = 5. * M_PI * sqrt( (double)array_size( sys->spobs_virtual ) ) *
           uniedit_zoom;

      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderRadius( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys = system_getIndex( i );

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* draws the disk representing the faction */
      sr = 5. * M_PI * sqrt( sys->radius / 10e3 ) * uniedit_zoom;

      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderNolanes( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys = system_getIndex( i );

      if ( !sys_isFlag( sys, SYSTEM_NOLANES ) )
         continue;

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* draws the disk representing the faction */
      sr = 5. * M_PI * uniedit_zoom;

      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderBackground( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys = system_getIndex( i );

      if ( sys->background == NULL )
         continue;

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* draws the disk representing the faction */
      sr = 7. * M_PI * uniedit_zoom;

      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderAsteroids( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys     = system_getIndex( i );
      double      density = sys->asteroid_density;

      if ( density <= 0. )
         continue;

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* Draw disk. */
      sr = 0.3 * M_PI * sqrt( density ) * uniedit_zoom;
      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderInterference( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys = system_getIndex( i );

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* draws the disk representing the faction */
      sr = 5. * M_PI * sqrt( sys->interference / 20. ) * uniedit_zoom;

      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderTech( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys     = system_getIndex( i );
      int         hastech = 0;

      for ( int j = 0; j < array_size( sys->spobs ); j++ ) {
         if ( sys->spobs[j]->tech != NULL ) {
            hastech = 1;
            break;
         }
      }
      if ( !hastech )
         continue;

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* Draw disk. */
      sr = 7. * M_PI * uniedit_zoom;
      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

static void uniedit_renderPresenceSum( double x, double y, double r )
{
   const glColour c = { .r = 1., .g = 1., .b = 1., .a = 0.3 };

   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      double      tx, ty, sr;
      StarSystem *sys = system_getIndex( i );

      double total = 0.;
      for ( int j = 0; j < array_size( sys->presence ); j++ )
         total += MAX( 0., sys->presence[j].value );

      tx = x + sys->pos.x * uniedit_zoom;
      ty = y + sys->pos.y * uniedit_zoom;

      /* draws the disk representing the faction */
      sr = 0.2 * M_PI * sqrt( total ) * uniedit_zoom;

      (void)r;
      gl_renderDisk( tx, ty, sr, &c );
   }
}

/* @brief Renders important map stuff.
 */
void uniedit_renderMap( double bx, double by, double w, double h, double x,
                        double y, double zoom, double r )
{
   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   if ( UNIEDIT_VIEW_DEFAULT )
      map_renderDecorators( x, y, zoom, 1, 1. );

   /* Render faction disks. */
   switch ( uniedit_viewmode ) {
   case UNIEDIT_VIEW_DEFAULT:
      map_renderFactionDisks( x, y, zoom, r, 1, 1. );
      map_renderSystemEnvironment( x, y, zoom, 1, 1. );
      break;

   case UNIEDIT_VIEW_VIRTUALSPOBS:
      uniedit_renderVirtualSpobs( x, y, r );
      break;

   case UNIEDIT_VIEW_RADIUS:
      uniedit_renderRadius( x, y, r );
      break;

   case UNIEDIT_VIEW_NOLANES:
      uniedit_renderNolanes( x, y, r );
      break;

   case UNIEDIT_VIEW_BACKGROUND:
      uniedit_renderBackground( x, y, r );
      break;

   case UNIEDIT_VIEW_ASTEROIDS:
      uniedit_renderAsteroids( x, y, r );
      break;

   case UNIEDIT_VIEW_INTERFERENCE:
      uniedit_renderInterference( x, y, r );
      break;

   case UNIEDIT_VIEW_TECH:
      uniedit_renderTech( x, y, r );
      break;

   case UNIEDIT_VIEW_PRESENCE_SUM:
      uniedit_renderPresenceSum( x, y, r );
      break;

   case UNIEDIT_VIEW_PRESENCE:
      if ( uniedit_view_faction >= 0 )
         uniedit_renderFactionDisks( x, y, r );
      break;
   }

   /* Render jump paths. */
   map_renderJumps( x, y, zoom, r, 1 );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, zoom, w, h, r, MAPMODE_EDITOR );

   /* Render system names. */
   map_renderNames( bx, by, x, y, zoom, w, h, 1, 1. );

   glClear( GL_DEPTH_BUFFER_BIT );
}

/**
 * @brief System editor custom widget rendering.
 */
static void uniedit_render( double bx, double by, double w, double h,
                            void *data )
{
   (void)data;
   double x, y, r;

   uniedit_dt += naev_getrealdt();

   /* Parameters. */
   map_renderParams( bx, by, uniedit_xpos, uniedit_ypos, w, h, uniedit_zoom, &x,
                     &y, &r );

   /* Render map stuff. */
   uniedit_renderMap( bx, by, w, h, x, y, uniedit_zoom, r );

   /* Render the selected system selections. */
   for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
      StarSystem *sys = uniedit_sys[i];
      glUseProgram( shaders.selectspob.program );
      glUniform1f( shaders.selectspob.dt, uniedit_dt );
      gl_renderShader( x + sys->pos.x * uniedit_zoom,
                       y + sys->pos.y * uniedit_zoom, 1.5 * r, 1.5 * r, 0.,
                       &shaders.selectspob, &cWhite, 1 );
   }
}

static char getValCol( double val )
{
   if ( val > 0. )
      return 'g';
   else if ( val < 0. )
      return 'r';
   return '0';
}
static int getPresenceVal( int f, SpobPresence *ap, double *base,
                           double *bonus )
{
   int    gf = 0;
   double w;
   if ( ( ap->faction != f ) &&
        !( gf = factionGenerates( ap->faction, f, &w ) ) )
      return 0;
   if ( gf == 0 ) {
      *base  = ap->base;
      *bonus = ap->bonus;
   } else {
      *base  = ap->base * w;
      *bonus = ap->bonus * w;
   }
   return 1;
}

/**
 * @brief Renders the overlay.
 */
static void uniedit_renderOverlay( double bx, double by, double bw, double bh,
                                   void *data )
{
   double          x, y, mx, my, sx, sy;
   double          value, base, bonus;
   char            buf[STRMAX] = { '\0' };
   StarSystem     *sys, *cur, *mousesys;
   SystemPresence *sp;
   (void)data;

   x = bx + uniedit_mx;
   y = by + uniedit_my;

   /* Correct coordinates. */
   mx = uniedit_mx - bw / 2. + uniedit_xpos;
   my = uniedit_my - bh / 2. + uniedit_ypos;
   mx /= uniedit_zoom;
   my /= uniedit_zoom;

   /* Display location. */
   gl_print( &gl_defFontMono, bx + 5, by + 65, &cWhite, "% 7.2f x % 7.2f", mx,
             my );

   /* Select drag stuff. */
   if ( uniedit_dragSel ) {
      double         l, r, b, t, rx, ry;
      const glColour col = { .r = 0.2, .g = 0.2, .b = 0.8, .a = 0.5 };

      l = MIN( uniedit_dragSelX, mx );
      r = MAX( uniedit_dragSelX, mx );
      b = MIN( uniedit_dragSelY, my );
      t = MAX( uniedit_dragSelY, my );

      /* Project back to screen space. */
      rx = ( l * uniedit_zoom ) + bw / 2. - uniedit_xpos;
      ry = ( b * uniedit_zoom ) + bh / 2. - uniedit_ypos;

      gl_renderRect( rx, ry, ( r - l ) * uniedit_zoom, ( t - b ) * uniedit_zoom,
                     &col );
   }

   /* Don't cover up stuff if possible. */
   if ( ( x > SCREEN_W - 130 ) || ( y < 60 ) )
      return;

   if ( uniedit_mode == UNIEDIT_NEWSYS ) {
      toolkit_drawAltText( x, y, _( "Click to add a new system" ) );
      return;
   } else if ( uniedit_mode == UNIEDIT_JUMP ) {
      toolkit_drawAltText( x, y, _( "Click to toggle jump route" ) );
      return;
   } else if ( uniedit_viewmode == UNIEDIT_VIEW_DEFAULT )
      return;

   /* Find mouse over system. */
   mousesys = NULL;
   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      sys = system_getIndex( i );
      sx  = sys->pos.x;
      sy  = sys->pos.y;
      if ( ( pow2( sx - mx ) + pow2( sy - my ) ) >
           pow2( UNIEDIT_CLICK_THRESHOLD ) )
         continue;
      mousesys = sys;
      break;
   }
   if ( mousesys == NULL )
      return;
   sys = mousesys;
   sx  = sys->pos.x;
   sy  = sys->pos.y;

   /* Handle virtual spob viewer. */
   if ( uniedit_viewmode == UNIEDIT_VIEW_VIRTUALSPOBS ) {
      int l;

      if ( array_size( sys->spobs_virtual ) == 0 )
         return;

      /* Count spobs. */
      l = 0;
      for ( int j = 0; j < array_size( sys->spobs_virtual ); j++ ) {
         VirtualSpob *va = sys->spobs_virtual[j];
         l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s",
                         ( l > 0 ) ? "\n" : "", va->name );
      }

      toolkit_drawAltText( x, y, buf );
      return;
   }

   /* Handle radius view. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_RADIUS ) {
      scnprintf( &buf[0], sizeof( buf ), _( "System Radius: %s" ),
                 num2strU( sys->radius, 0 ) );
      toolkit_drawAltText( x, y, buf );
      return;
   }

   /* Handle background. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_BACKGROUND ) {
      if ( sys->background != NULL ) {
         scnprintf( &buf[0], sizeof( buf ), _( "Background: %s" ),
                    sys->background );
         toolkit_drawAltText( x, y, buf );
      }
      return;
   }

   /* Handle asteroids. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_ASTEROIDS ) {
      if ( array_size( sys->asteroids ) > 0 ) {
         int l = 0;
         l     = scnprintf( &buf[l], sizeof( buf ) - l, _( "Density: %g" ),
                            sys->asteroid_density );
         for ( int i = 0; i < array_size( sys->asteroids ); i++ ) {
            AsteroidAnchor *ast = &sys->asteroids[i];
            for ( int j = 0; j < array_size( ast->groups ); j++ )
               l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s",
                               ( l > 0 ) ? "\n" : "", ast->groups[j]->name );
         }
         toolkit_drawAltText( x, y, buf );
      }
      return;
   }

   /* Handle interference. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_INTERFERENCE ) {
      if ( sys->interference > 0. ) {
         scnprintf( &buf[0], sizeof( buf ), _( "Interference: %.0f%%" ),
                    sys->interference );
         toolkit_drawAltText( x, y, buf );
      }
      return;
   }

   /* Handle tech radius. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_TECH ) {
      char     *techlist[256];
      int       ntechs = 0;
      const int len    = sizeof( techlist ) / sizeof( char    *);
      int       l;

      if ( array_size( sys->spobs ) == 0 )
         return;

      /* Count spobs. */
      l = 0;
      for ( int j = 0; j < array_size( sys->spobs ); j++ ) {
         Spob  *spob = sys->spobs[j];
         int    n;
         char **techs;
         if ( spob->tech == NULL )
            continue;
         techs = tech_getItemNames( spob->tech, &n );
         for ( int k = 0; ( k < n ) && ( ntechs < len - 1 ); k++ )
            techlist[ntechs++] = techs[k];
         free( techs );
      }
      qsort( techlist, ntechs, sizeof( char * ), strsort );
      for ( int k = 0; k < ntechs; k++ ) {
         if ( ( k > 0 ) && ( strcmp( techlist[k - 1], techlist[k] ) == 0 ) )
            continue;
         l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s",
                         ( l > 0 ) ? "\n" : "", techlist[k] );
      }
      for ( int k = 0; k < ntechs; k++ )
         free( techlist[k] );

      toolkit_drawAltText( x, y, buf );
      return;
   }

   /* Handle presence sum. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_PRESENCE_SUM ) {
      int l;

      if ( array_size( sys->presence ) == 0 )
         return;

      value = 0.;
      for ( int j = 0; j < array_size( sys->presence ); j++ )
         value += MAX( sys->presence[j].value, 0. );

      /* Count spobs. */
      l = scnprintf( buf, sizeof( buf ), _( "Total: %.0f" ), value );
      for ( int j = 0; j < array_size( sys->presence ); j++ ) {
         sp = &sys->presence[j];
         if ( sp->value <= 0. )
            continue;
         l += scnprintf( &buf[l], sizeof( buf ) - l, "\n%s: %.0f = %.0f + %.0f",
                         faction_name( sp->faction ), sp->value, sp->base,
                         sp->bonus );
      }
      toolkit_drawAltText( x, y, buf );
      return;
   }

   /* Handle presence mode. */
   else if ( uniedit_viewmode == UNIEDIT_VIEW_PRESENCE ) {
      int l;
      int f = uniedit_view_faction;
      if ( f < 0 )
         return;

      /* Total presence. */
      value = system_getPresenceFull( sys, f, &base, &bonus );
      l     = scnprintf(
         buf, sizeof( buf ), "#%c%.0f#0 = #%c%.0f#0 + #%c%.0f#0 [%s - %s]",
         getValCol( value ), value, getValCol( base ), base, getValCol( bonus ),
         bonus, _( sys->name ), faction_name( f ) );

      /* Local presence sources. */
      for ( int j = 0; j < array_size( sys->spobs ); j++ ) {
         Spob *spob = sys->spobs[j];
         if ( !getPresenceVal( f, &spob->presence, &base, &bonus ) )
            continue;
         l += scnprintf( &buf[l], sizeof( buf ) - l,
                         "\n#%c%.0f#0 (#%c%+.0f#0) [%s]", getValCol( base ),
                         base, getValCol( bonus ), bonus, spob_name( spob ) );
      }
      for ( int j = 0; j < array_size( sys->spobs_virtual ); j++ ) {
         VirtualSpob *va = sys->spobs_virtual[j];
         for ( int p = 0; p < array_size( va->presences ); p++ ) {
            if ( !getPresenceVal( f, &va->presences[p], &base, &bonus ) )
               continue;
            l += scnprintf( &buf[l], sizeof( buf ) - l,
                            "\n#%c%.0f#0 (#%c%+.0f#0) [%s]", getValCol( base ),
                            base, getValCol( bonus ), bonus, _( va->name ) );
         }
      }

      /* Find neighbours if possible. */
      for ( int k = 0; k < array_size( sys->jumps ); k++ ) {
         cur = sys->jumps[k].target;
         for ( int j = 0; j < array_size( cur->spobs ); j++ ) {
            Spob *spob = cur->spobs[j];
            if ( !getPresenceVal( f, &spob->presence, &base, &bonus ) )
               continue;
            l += scnprintf( &buf[l], sizeof( buf ) - l,
                            "\n#%c%.0f#0 (#%c%+.0f#0) [%s (%s)]",
                            getValCol( base ), base * 0.5, getValCol( bonus ),
                            bonus * 0.5, spob_name( spob ), _( cur->name ) );
         }
         for ( int j = 0; j < array_size( cur->spobs_virtual ); j++ ) {
            VirtualSpob *va = cur->spobs_virtual[j];
            for ( int p = 0; p < array_size( va->presences ); p++ ) {
               if ( !getPresenceVal( f, &va->presences[p], &base, &bonus ) )
                  continue;
               l +=
                  scnprintf( &buf[l], sizeof( buf ) - l,
                             "\n#%c%.0f#0 (#%c%+.0f#0) [%s (%s)]",
                             getValCol( base ), base * 0.5, getValCol( bonus ),
                             bonus * 0.5, _( va->name ), _( cur->name ) );
            }
         }
      }

      toolkit_drawAltText( x, y, buf );
      return;
   }
}

/**
 * @brief Called when it's de-focused.
 */
static void uniedit_focusLose( unsigned int wid, const char *wgtname )
{
   (void)wid;
   (void)wgtname;
   uniedit_drag = uniedit_dragSys = uniedit_dragSel = 0;
}

/**
 * @brief System editor custom widget mouse handling.
 */
static int uniedit_mouse( unsigned int wid, const SDL_Event *event, double mx,
                          double my, double w, double h, double rx, double ry,
                          void *data )
{
   (void)data;
   unsigned int lastClick;
   StarSystem  *clickedsys;
   int          inselection;
   SDL_Keymod   mod;

   /* Handle modifiers. */
   mod = SDL_GetModState();

   switch ( event->type ) {

   case SDL_MOUSEWHEEL:
      /* Must be in bounds. */
      if ( ( mx < 0. ) || ( mx > w - 130. ) || ( my < 60. ) || ( my > h ) )
         return 0;

      if ( event->wheel.y > 0 )
         uniedit_buttonZoom( 0, "btnZoomIn" );
      else if ( event->wheel.y < 0 )
         uniedit_buttonZoom( 0, "btnZoomOut" );

      return 1;

   case SDL_MOUSEBUTTONDOWN:
      /* Must be in bounds. */
      if ( ( mx < 0. ) || ( mx > w - 130. ) || ( my < 60. ) || ( my > h ) )
         return 0;
      window_setFocus( wid, "cstSysEdit" );
      lastClick         = uniedit_lastClick;
      uniedit_lastClick = SDL_GetTicks();

      /* Selecting star system */
      mx -= w / 2. - uniedit_xpos;
      my -= h / 2. - uniedit_ypos;
      mx /= uniedit_zoom;
      my /= uniedit_zoom;

      /* Finish rotation. */
      if ( uniedit_mode == UNIEDIT_ROTATE ) {
         uniedit_mode = UNIEDIT_DEFAULT;
         return 1;
      }

      /* Create new system if applicable. */
      if ( uniedit_mode == UNIEDIT_NEWSYS ) {
         uniedit_newSys( mx, my );
         uniedit_mode = UNIEDIT_DEFAULT;
         return 1;
      }

      /* Find clicked system. */
      clickedsys = NULL;
      for ( int i = 0; i < array_size( systems_stack ); i++ ) {
         StarSystem *sys = system_getIndex( i );
         if ( ( pow2( mx - sys->pos.x ) + pow2( my - sys->pos.y ) ) >
              pow2( UNIEDIT_CLICK_THRESHOLD ) )
            continue;
         clickedsys = sys;
         break;
      }

      /* Set jump if applicable. */
      if ( clickedsys != NULL && uniedit_mode == UNIEDIT_JUMP ) {
         uniedit_toggleJump( clickedsys );
         uniedit_mode = UNIEDIT_DEFAULT;
         return 1;
      }

      /* See if it is selected. */
      inselection = 0;
      if ( clickedsys != NULL ) {
         for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
            if ( uniedit_sys[i] != clickedsys )
               continue;
            inselection = 1;
            break;
         }
      }

      /* Handle double click. */
      if ( clickedsys != NULL && inselection &&
           array_size( uniedit_sys ) == 1 ) {
         if ( ( SDL_GetTicks() - lastClick < UNIEDIT_DOUBLECLICK_THRESHOLD ) &&
              ( uniedit_moved < UNIEDIT_MOVE_THRESHOLD ) ) {
            sysedit_open( uniedit_sys[0] );
            uniedit_drag    = 0;
            uniedit_dragSys = 0;
            uniedit_dragSel = 0;
            return 1;
         }
      }

      /* Clicked on selected system. */
      if ( ( clickedsys != NULL ) && inselection ) {
         uniedit_dragSys = 1;
         uniedit_tsys    = clickedsys;
         /* Check modifier. */
         if ( mod & ( KMOD_LCTRL | KMOD_RCTRL ) )
            uniedit_tadd = 0;
         else
            uniedit_tadd = -1;
         uniedit_moved = 0;
         return 1;
      }

      /* Clicked on non-selected system. */
      if ( clickedsys != NULL ) {
         /* Add the system if not selected. */
         if ( !( mod & ( KMOD_LCTRL | KMOD_RCTRL ) ) )
            uniedit_deselect();
         uniedit_selectAdd( clickedsys );
         uniedit_tsys = NULL;
         return 1;
      }

      /* Start dragging. */
      uniedit_moved = 0;
      uniedit_tsys  = NULL;
      if ( mod & ( KMOD_LCTRL | KMOD_RCTRL | KMOD_LSHIFT | KMOD_RSHIFT ) ) {
         if ( mod & ( KMOD_LSHIFT | KMOD_RSHIFT ) )
            uniedit_deselect();
         uniedit_dragSel  = 1;
         uniedit_dragSelX = mx;
         uniedit_dragSelY = my;
         return 1;
      } else {
         uniedit_drag = 1;
         return 1;
      }
      break;

   case SDL_MOUSEBUTTONUP:
      if ( uniedit_drag ) {
         if ( ( SDL_GetTicks() - uniedit_lastClick < UNIEDIT_DRAG_THRESHOLD ) &&
              ( uniedit_moved < UNIEDIT_MOVE_THRESHOLD ) ) {
            if ( uniedit_tsys == NULL )
               uniedit_deselect();
            else
               uniedit_selectAdd( uniedit_tsys );
         }
         uniedit_drag = 0;
      }
      if ( uniedit_dragSys ) {
         if ( ( SDL_GetTicks() - uniedit_lastClick < UNIEDIT_DRAG_THRESHOLD ) &&
              ( uniedit_moved < UNIEDIT_MOVE_THRESHOLD ) &&
              ( uniedit_tsys != NULL ) ) {
            if ( uniedit_tadd == 0 )
               uniedit_selectRm( uniedit_tsys );
            else {
               uniedit_deselect();
               uniedit_selectAdd( uniedit_tsys );
            }
         }
         uniedit_dragSys = 0;
         if ( conf.devautosave )
            for ( int i = 0; i < array_size( uniedit_sys ); i++ )
               dsys_saveSystem( uniedit_sys[i] );
      }
      if ( uniedit_dragSel ) {
         double l, r, b, t;

         /* Selecting star system */
         mx -= w / 2. - uniedit_xpos;
         my -= h / 2. - uniedit_ypos;
         mx /= uniedit_zoom;
         my /= uniedit_zoom;

         /* Get bounds. */
         l = MIN( uniedit_dragSelX, mx );
         r = MAX( uniedit_dragSelX, mx );
         b = MIN( uniedit_dragSelY, my );
         t = MAX( uniedit_dragSelY, my );

         for ( int i = 0; i < array_size( systems_stack ); i++ ) {
            StarSystem *sys = &systems_stack[i];
            double      x   = sys->pos.x;
            double      y   = sys->pos.y;
            if ( ( x >= l ) && ( x <= r ) && ( y >= b ) && ( y <= t ) )
               uniedit_selectAdd( sys );
         }

         uniedit_dragSel = 0;
      }
      break;

   case SDL_MOUSEMOTION:
      /* Update mouse positions. */
      uniedit_mx = mx;
      uniedit_my = my;

      /* Handle rotation. */
      if ( uniedit_mode == UNIEDIT_ROTATE ) {
         double a1, a2, amod;
         double cx = mx - w / 2. + uniedit_xpos;
         double cy = my - h / 2. + uniedit_ypos;
         cx /= uniedit_zoom;
         cy /= uniedit_zoom;
         cx -= uniedit_rotate_cx;
         cy -= uniedit_rotate_cy;
         a1 = atan2( cy, cx );
         cx -= rx / uniedit_zoom;
         cy += ry / uniedit_zoom;
         a2   = atan2( cy, cx );
         amod = a1 - a2;
         uniedit_rotate += amod;
         for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
            StarSystem *s  = uniedit_sys[i];
            double      sx = s->pos.x - uniedit_rotate_cx;
            double      sy = s->pos.y - uniedit_rotate_cy;
            double      a  = atan2( sy, sx );
            double      m  = hypot( sx, sy );
            s->pos.x       = uniedit_rotate_cx + m * cos( a + amod );
            s->pos.y       = uniedit_rotate_cy + m * sin( a + amod );
         }
      }

      /* Handle dragging. */
      if ( uniedit_drag ) {
         /* axis is inverted */
         uniedit_xpos -= rx;
         uniedit_ypos += ry;

         /* Update mouse movement. */
         uniedit_moved += ABS( rx ) + ABS( ry );
      } else if ( uniedit_dragSys && ( array_size( uniedit_sys ) > 0 ) ) {
         if ( ( uniedit_moved > UNIEDIT_MOVE_THRESHOLD ) ||
              ( SDL_GetTicks() - uniedit_lastClick >
                UNIEDIT_DRAG_THRESHOLD ) ) {
            for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
               uniedit_sys[i]->pos.x += rx / uniedit_zoom;
               uniedit_sys[i]->pos.y -= ry / uniedit_zoom;
            }
         }

         /* Update mouse movement. */
         uniedit_moved += ABS( rx ) + ABS( ry );
      }
      break;
   }

   return 0;
}

/**
 * @brief Checks to see if a system name is already in use.
 *
 *    @return 1 if system name is already in use.
 */
static int uniedit_checkName( const char *name )
{
   /* Avoid name collisions. */
   for ( int i = 0; i < array_size( systems_stack ); i++ ) {
      if ( strcmp( name, system_getIndex( i )->name ) == 0 ) {
         dialogue_alert( _( "The Star System '%s' already exists!" ), name );
         return 1;
      }
   }
   return 0;
}

char *uniedit_nameFilter( const char *name )
{
   char *out = calloc( 1, ( strlen( name ) + 1 ) );
   int   pos = 0;
   for ( int i = 0; i < (int)strlen( name ); i++ ) {
      if ( !ispunct( name[i] ) ) {
         if ( name[i] == ' ' )
            out[pos] = '_';
         else
            out[pos] = tolower( name[i] );
         pos++;
      }
   }

   return out;
}

/**
 * @brief Renames all the currently selected systems.
 */
static void uniedit_renameSys( void )
{
   int cancelall_prompt = 0;
   for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
      char       *name, *oldName, *newName, *filtered;
      StarSystem *sys = uniedit_sys[i];

      /* Get name. */
      name =
         dialogue_input( _( "Rename Star System" ), 1, 32,
                         _( "What do you want to rename #r%s#0?" ), sys->name );

      /* Keep current name. */
      if ( name == NULL ) {
         if ( !cancelall_prompt && ( i < array_size( uniedit_sys ) ) ) {
            if ( dialogue_YesNoRaw( _( "Cancel batch renaming?" ),
                                    _( "Do you want to cancel renaming all "
                                       "selected star systems?" ) ) )
               break;
            cancelall_prompt = 1;
         }
         continue;
      }

      /* Try again. */
      if ( uniedit_checkName( name ) ) {
         free( name );
         i--;
         continue;
      }

      /* Change the name. */
      filtered = uniedit_nameFilter( sys->name );
      SDL_asprintf( &oldName, "%s/%s.xml", conf.dev_save_sys, filtered );
      free( filtered );

      filtered = uniedit_nameFilter( name );
      SDL_asprintf( &newName, "%s/%s.xml", conf.dev_save_sys, filtered );
      free( filtered );

      if ( rename( oldName, newName ) )
         WARN( _( "Failed to rename '%s' to '%s'!" ), oldName, newName );

      free( oldName );
      free( newName );
      free( sys->name );

      sys->name = name;
      dsys_saveSystem( sys );

      /* Re-save adjacent systems. */
      for ( int j = 0; j < array_size( sys->jumps ); j++ )
         dsys_saveSystem( sys->jumps[j].target );
   }
}

/**
 * @brief Creates a new system.
 */
static void uniedit_newSys( double x, double y )
{
   char       *name;
   StarSystem *sys;

   /* Get name. */
   name = dialogue_inputRaw( _( "New Star System Creation" ), 1, 32,
                             _( "What do you want to name the new system?" ) );

   /* Abort. */
   if ( name == NULL ) {
      dialogue_alert( _( "Star System creation aborted!" ) );
      return;
   }

   /* Make sure there is no collision. */
   if ( uniedit_checkName( name ) ) {
      free( name );
      uniedit_newSys( x, y );
      return;
   }

   /* Create the system. */
   sys            = system_new();
   sys->name      = name;
   sys->pos.x     = x;
   sys->pos.y     = y;
   sys->spacedust = DUST_DENSITY_DEFAULT;
   sys->radius    = RADIUS_DEFAULT;

   /* Select new system. */
   uniedit_deselect();
   uniedit_selectAdd( sys );

   if ( conf.devautosave )
      dsys_saveSystem( sys );
}

/**
 * @brief Toggles the jump point for the selected systems.
 */
static void uniedit_toggleJump( StarSystem *sys )
{
   StarSystem *isys = NULL;
   for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
      int rm = 0;
      isys   = uniedit_sys[i];
      for ( int j = 0; j < array_size( isys->jumps ); j++ ) {
         StarSystem *target = isys->jumps[j].target;
         /* Target already exists, remove. */
         if ( target == sys ) {
            uniedit_jumpRm( isys, sys );
            uniedit_jumpRm( sys, isys );
            rm = 1;
            break;
         }
      }
      /* Target doesn't exist, add. */
      if ( !rm ) {
         uniedit_jumpAdd( isys, sys );
         uniedit_jumpAdd( sys, isys );
      }
   }

   /* Reconstruct jumps just in case. */
   systems_reconstructJumps();

   /* Reconstruct universe presences. */
   space_reconstructPresences();
   safelanes_recalculate();

   if ( conf.devautosave ) {
      dsys_saveSystem( sys );
      if ( isys != NULL )
         dsys_saveSystem( isys );
   }

   /* Update sidebar text. */
   uniedit_selectText();
}

/**
 * @brief Adds a new Star System jump.
 */
static void uniedit_jumpAdd( StarSystem *sys, StarSystem *targ )
{
   /* Add the jump. */
   JumpPoint *jp = &array_grow( &sys->jumps );
   memset( jp, 0, sizeof( JumpPoint ) );

   /* Fill it out with basics. */
   jp->target   = targ;
   jp->targetid = targ->id;
   jp->radius   = 200.;
   jp->flags    = JP_AUTOPOS; /* Will automatically create position. */
   jp->hide     = HIDE_DEFAULT_JUMP;
}

/**
 * @brief Removes a Star System jump.
 */
static void uniedit_jumpRm( StarSystem *sys, StarSystem *targ )
{
   int i;

   /* Find associated jump. */
   for ( i = 0; i < array_size( sys->jumps ); i++ )
      if ( sys->jumps[i].target == targ )
         break;

   /* Not found. */
   if ( i >= array_size( sys->jumps ) ) {
      WARN( _( "Jump for system '%s' not found in system '%s' for removal." ),
            targ->name, sys->name );
      return;
   }

   /* Remove the jump. */
   array_erase( &sys->jumps, &sys->jumps[i], &sys->jumps[i + 1] );
}

/**
 * @brief Deselects selected targets.
 */
static void uniedit_deselect( void )
{
   array_free( uniedit_sys );
   uniedit_sys = NULL;

   /* Change window stuff. */
   window_disableButton( uniedit_wid, "btnJump" );
   window_disableButton( uniedit_wid, "btnRename" );
   window_disableButton( uniedit_wid, "btnEdit" );
   window_disableButton( uniedit_wid, "btnOpen" );
   window_modifyText( uniedit_wid, "txtSelected", _( "No selection" ) );
   window_modifyText( uniedit_wid, "txtNebula", _( "N/A" ) );
   window_modifyText( uniedit_wid, "txtPresence", _( "N/A" ) );
}

/**
 * @brief Adds a system to the selection.
 */
static void uniedit_selectAdd( StarSystem *sys )
{
   if ( uniedit_sys == NULL )
      uniedit_sys = array_create( StarSystem * );

   array_push_back( &uniedit_sys, sys );

   /* Set text again. */
   uniedit_selectText();

   /* Enable buttons again. */
   window_enableButton( uniedit_wid, "btnJump" );
   window_enableButton( uniedit_wid, "btnRename" );
   window_enableButton( uniedit_wid, "btnEdit" );
   if ( array_size( uniedit_sys ) == 1 )
      window_enableButton( uniedit_wid, "btnOpen" );
   else
      window_disableButton( uniedit_wid, "btnOpen" );
}

/**
 * @brief Removes a system from the selection.
 */
static void uniedit_selectRm( StarSystem *sys )
{
   for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
      if ( uniedit_sys[i] == sys ) {
         array_erase( &uniedit_sys, &uniedit_sys[i], &uniedit_sys[i + 1] );
         uniedit_selectText();
         if ( array_size( uniedit_sys ) == 1 )
            window_enableButton( uniedit_wid, "btnOpen" );
         else
            window_disableButton( uniedit_wid, "btnOpen" );
         return;
      }
   }
   WARN( _( "Trying to remove system '%s' from selection when not selected." ),
         sys->name );
}

/**
 * @brief Sets the selected system text.
 */
void uniedit_selectText( void )
{
   int  l;
   char buf[STRMAX];

   l = 0;
   for ( int i = 0; i < array_size( uniedit_sys ); i++ ) {
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s", uniedit_sys[i]->name,
                      ( i == array_size( uniedit_sys ) - 1 ) ? "" : ", " );
   }

   if ( l == 0 )
      uniedit_deselect();
   else {
      window_modifyText( uniedit_wid, "txtSelected", buf );

      /* Presence text. */
      if ( array_size( uniedit_sys ) == 1 ) {
         StarSystem *sys = uniedit_sys[0];

         buf[0] = '\0';
         l      = 0;
         if ( sys->nebu_density > 0. )
            l += scnprintf( &buf[l], sizeof( buf ) - l,
                            _( "%.0f Density\n%.1f Volatility\n%.0f Hue" ),
                            sys->nebu_density, sys->nebu_volatility,
                            sys->nebu_hue * 360. );
         if ( sys->interference > 0. )
            l += scnprintf( &buf[l], sizeof( buf ) - l,
                            _( "%s%.1f Interference" ), ( l > 0 ) ? "\n" : "",
                            sys->interference );

         window_modifyText( uniedit_wid, "txtNebula", buf );

         /* Update presence stuff. */
         map_updateFactionPresence( uniedit_wid, "txtPresence", sys, 1 );
      } else {
         window_modifyText( uniedit_wid, "txtNebula",
                            _( "Multiple selected" ) );
         window_modifyText( uniedit_wid, "txtPresence",
                            _( "Multiple selected" ) );
      }
   }

   window_moveWidget( uniedit_wid, "txtPresence", -10,
                      -40 - window_getTextHeight( uniedit_wid, "txtNebula" ) );
}

/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void uniedit_buttonZoom( unsigned int wid, const char *str )
{
   (void)wid;
   /* Transform coords to normal. */
   uniedit_xpos /= uniedit_zoom;
   uniedit_ypos /= uniedit_zoom;

   /* Apply zoom. */
   if ( strcmp( str, "btnZoomIn" ) == 0 ) {
      uniedit_zoom *= UNIEDIT_ZOOM_STEP;
      uniedit_zoom =
         MIN( pow( UNIEDIT_ZOOM_STEP, UNIEDIT_ZOOM_MAX ), uniedit_zoom );
   } else if ( strcmp( str, "btnZoomOut" ) == 0 ) {
      uniedit_zoom /= UNIEDIT_ZOOM_STEP;
      uniedit_zoom =
         MAX( pow( UNIEDIT_ZOOM_STEP, UNIEDIT_ZOOM_MIN ), uniedit_zoom );
   }

   /* Transform coords back. */
   uniedit_xpos *= uniedit_zoom;
   uniedit_ypos *= uniedit_zoom;
}

/**
 * @brief Finds systems and spobs.
 */
static void uniedit_findSys( void )
{
   unsigned int wid;
   int          x, y;

   x = 40;

   /* Create the window. */
   wid = window_create( "wdwFindSystemsandSpobs", _( "Find Systems and Spobs" ),
                        x, -1, UNIEDIT_FIND_WIDTH, UNIEDIT_FIND_HEIGHT );
   uniedit_widFind = wid;

   x = 20;

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                     _( "Close" ), uniedit_findSysClose );

   /* Find input widget. */
   y = -45;
   window_addInput( wid, x, y, UNIEDIT_FIND_WIDTH - 40, 20, "inpFind", 32, 1,
                    NULL );
   window_setInputCallback( wid, "inpFind", uniedit_findSearch );

   /* Close when escape is pressed. */
   window_setCancel( wid, uniedit_findSysClose );

   /* Generate the list. */
   uniedit_findSearch( wid, NULL );

   /* Focus the input widget. */
   window_setFocus( wid, "inpFind" );
}

/**
 * @brief Searches for spobs and systems.
 */
static void uniedit_findSearch( unsigned int wid, const char *str )
{
   (void)str;
   int         n, nspobs, nsystems;
   const char *name;
   char      **spobs, **systems;
   map_find_t *found;

   name = window_getInput( wid, "inpFind" );

   /* Search for names. */
   spobs   = spob_searchFuzzyCase( name, &nspobs );
   systems = system_searchFuzzyCase( name, &nsystems );

   free( found_cur );
   found_cur = NULL;

   /* Construct found table. */
   found = malloc( sizeof( map_find_t ) * ( nspobs + nsystems ) );
   n     = 0;

   /* Add spobs to the found table. */
   for ( int i = 0; i < nspobs; i++ ) {
      /* Spob must be real. */
      Spob *spob = spob_get( spobs[i] );
      if ( spob == NULL )
         continue;

      const char *sysname = spob_getSystem( spobs[i] );
      if ( sysname == NULL )
         continue;

      StarSystem *sys = system_get( sysname );
      if ( sys == NULL )
         continue;

      /* Set some values. */
      found[n].spob = spob;
      found[n].sys  = sys;

      /* Set fancy name. */
      snprintf( found[n].display, sizeof( found[n].display ),
                _( "%s (%s system)" ), spobs[i], sys->name );
      n++;
   }
   free( spobs );

   /* Add systems to the found table. */
   for ( int i = 0; i < nsystems; i++ ) {
      StarSystem *sys = system_get( systems[i] );

      /* Set some values. */
      found[n].spob = NULL;
      found[n].sys  = sys;

      strncpy( found[n].display, sys->name, sizeof( found[n].display ) - 1 );
      n++;
   }
   free( systems );

   /* Globals. */
   found_cur  = found;
   found_ncur = n;

   /* Display results. */
   uniedit_findShowResults( wid, found, n );
}

/**
 * @brief Generates the virtual spob list.
 */
static void uniedit_findShowResults( unsigned int wid, map_find_t *found,
                                     int n )
{
   int    y, h;
   char **str;

   /* Destroy if exists. */
   if ( widget_exists( wid, "lstResults" ) )
      window_destroyWidget( wid, "lstResults" );

   y = -45 - BUTTON_HEIGHT - 20;

   if ( n == 0 ) {
      str    = malloc( sizeof( char    *) );
      str[0] = strdup( _( "None" ) );
      n      = 1;
   } else {
      qsort( found, n, sizeof( map_find_t ), uniedit_sortCompare );

      str = malloc( sizeof( char * ) * n );
      for ( int i = 0; i < n; i++ )
         str[i] = strdup( found[i].display );
   }

   /* Add list. */
   h = UNIEDIT_FIND_HEIGHT + y - BUTTON_HEIGHT - 30;
   window_addList( wid, 20, y, UNIEDIT_FIND_WIDTH - 40, h, "lstResults", str, n,
                   0, uniedit_centerSystem, NULL );
}

/**
 * @brief Closes the search dialogue.
 */
static void uniedit_findSysClose( unsigned int wid, const char *name )
{
   /* Clean up if necessary. */
   free( found_cur );
   found_cur = NULL;

   /* Close the window. */
   window_close( wid, name );
}

/**
 * @brief Centers the selected system.
 */
static void uniedit_centerSystem( unsigned int wid, const char *unused )
{
   (void)unused;
   StarSystem *sys;
   int         pos;

   /* Make sure it's valid. */
   if ( found_ncur == 0 || found_cur == NULL )
      return;

   pos = toolkit_getListPos( wid, "lstResults" );
   sys = found_cur[pos].sys;

   if ( sys == NULL )
      return;

   /* Center. */
   uniedit_xpos = sys->pos.x * uniedit_zoom;
   uniedit_ypos = sys->pos.y * uniedit_zoom;
}

/**
 * @brief qsort compare function for map finds.
 */
static int uniedit_sortCompare( const void *p1, const void *p2 )
{
   map_find_t *f1, *f2;

   /* Convert pointer. */
   f1 = (map_find_t *)p1;
   f2 = (map_find_t *)p2;

   /* Sort by name, nothing more. */
   return strcasecmp( f1->sys->name, f2->sys->name );
}

/**
 * @brief Edits an individual system or group of systems.
 */
static void uniedit_editSys( void )
{
   unsigned int wid;
   int          x, y, l;
   char         buf[STRMAX_SHORT];
   const char  *s;
   StarSystem  *sys;

   /* Must have a system. */
   if ( array_size( uniedit_sys ) == 0 )
      return;
   sys = uniedit_sys[0];

   /* Create the window. */
   wid             = window_create( "wdwStarSystemPropertyEditor",
                                    _( "Star System Property Editor" ), -1, -1,
                                    UNIEDIT_EDIT_WIDTH, UNIEDIT_EDIT_HEIGHT );
   uniedit_widEdit = wid;
   window_setCancel( wid, uniedit_editSysClose );

   x = 20;

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                     _( "Close" ), uniedit_editSysClose );

   /* Rename button. */
   y = -45;
   snprintf( buf, sizeof( buf ), "%s #n%s", _( "Name:" ),
             ( array_size( uniedit_sys ) > 1 ) ? _( "#rvarious" )
                                               : uniedit_sys[0]->name );
   window_addText( wid, x, y, 180, 15, 0, "txtName", &gl_smallFont, NULL, buf );
   window_addButton( wid, 200, y + 3, BUTTON_WIDTH, 21, "btnRename",
                     _( "Rename" ), uniedit_btnEditRename );

   /* New row. */
   y -= gl_defFont.h + 15;

   /* Add general stats */
   s = _( "Radius" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtRadius", NULL, NULL, s );
   window_addInput( wid, x += l + 7, y, 80, 20, "inpRadius", 10, 1, NULL );
   window_setInputFilter( wid, "inpRadius", INPUT_FILTER_NUMBER );
   x += 80 + 12;
   s = _( "(Scales spob positions)" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtRadiusComment", NULL, NULL, s );

   /* New row. */
   x = 20;
   y -= gl_defFont.h + 15;

   s = _( "Dust" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtDust", NULL, NULL, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpDust", 4, 1, NULL );
   window_setInputFilter( wid, "inpDust", INPUT_FILTER_NUMBER );
   x += 50 + 12;

   s = _( "Interference" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtInterference", NULL, NULL, s );
   window_addInput( wid, x += l + 7, y, 55, 20, "inpInterference", 5, 1, NULL );
   window_setInputFilter( wid, "inpInterference", INPUT_FILTER_NUMBER );

   /* New row. */
   x = 20;
   y -= gl_defFont.h + 15;

   s = _( "Nebula" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtNebula", NULL, NULL, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpNebula", 4, 1, NULL );
   window_setInputFilter( wid, "inpNebula", INPUT_FILTER_NUMBER );
   x += 50 + 12;

   s = _( "Volatility" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtVolatility", NULL, NULL, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpVolatility", 4, 1, NULL );
   window_setInputFilter( wid, "inpVolatility", INPUT_FILTER_NUMBER );
   x += 50 + 12;

   s = _( "Hue" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtHue", NULL, NULL, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpHue", 4, 1, NULL );
   window_setInputFilter( wid, "inpHue", INPUT_FILTER_NUMBER );
   x += 50 + 12;

   /* Next row. */
   x = 20;
   y -= gl_defFont.h + 15;

   s = _( "No lanes" );
   window_addCheckbox( wid, x, y, 100, gl_defFont.h, "chkNolanes", s,
                       uniedit_chkNolanes, sys_isFlag( sys, SYSTEM_NOLANES ) );

   /* Tags. */
   x = 20;
   y -= gl_defFont.h + 15;
   l = scnprintf( buf, sizeof( buf ), "#n%s#0", _( "Tags: " ) );
   for ( int i = 0; i < array_size( sys->tags ); i++ )
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s",
                      ( i == 0 ) ? "" : ", ", sys->tags[i] );
   window_addText( wid, x, y, UNIEDIT_EDIT_WIDTH - 40, 20, 0, "txtTags", NULL,
                   NULL, buf );

   /* Load values */
   snprintf( buf, sizeof( buf ), "%g", sys->radius );
   window_setInput( wid, "inpRadius", buf );
   snprintf( buf, sizeof( buf ), "%d", sys->spacedust );
   window_setInput( wid, "inpDust", buf );
   snprintf( buf, sizeof( buf ), "%g", sys->interference );
   window_setInput( wid, "inpInterference", buf );
   snprintf( buf, sizeof( buf ), "%g", sys->nebu_density );
   window_setInput( wid, "inpNebula", buf );
   snprintf( buf, sizeof( buf ), "%g", sys->nebu_volatility );
   window_setInput( wid, "inpVolatility", buf );
   snprintf( buf, sizeof( buf ), "%g", sys->nebu_hue * 360. );
   window_setInput( wid, "inpHue", buf );

   /* Generate the list. */
   uniedit_editGenList( wid );
}

/**
 * @brief Generates the virtual spob list.
 */
static void uniedit_editGenList( unsigned int wid )
{
   int                j, n;
   StarSystem        *sys;
   const VirtualSpob *va;
   char             **str;
   int                y, h, has_spobs;

   /* Destroy if exists. */
   if ( widget_exists( wid, "lstSpobs" ) )
      window_destroyWidget( wid, "lstSpobs" );

   y = -180;

   /* Check to see if it actually has virtual spobs. */
   sys       = uniedit_sys[0];
   n         = array_size( sys->spobs_virtual );
   has_spobs = !!n;

   /* Generate list. */
   j   = 0;
   str = malloc( sizeof( char * ) * ( n + 1 ) );
   if ( has_spobs ) {
      /* Virtual spob button. */
      for ( int i = 0; i < n; i++ ) {
         va       = sys->spobs_virtual[i];
         str[j++] = strdup( va->name );
      }
   } else
      str[j++] = strdup( _( "None" ) );

   /* Add list. */
   h = UNIEDIT_EDIT_HEIGHT + y - 20 - 2 * ( BUTTON_HEIGHT + 20 );
   window_addList( wid, 20, y, UNIEDIT_EDIT_WIDTH - 40, h, "lstSpobs", str, j,
                   0, NULL, NULL );
   y -= h + 20;

   /* Add buttons if needed. */
   if ( !widget_exists( wid, "btnRmSpob" ) )
      window_addButton( wid, -20, y + 3, BUTTON_WIDTH, BUTTON_HEIGHT,
                        "btnRmSpob", _( "Remove" ), uniedit_btnEditRmSpob );
   if ( !widget_exists( wid, "btnAddSpob" ) )
      window_addButton( wid, -20 - ( 20 + BUTTON_WIDTH ), y + 3, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnAddSpob", _( "Add" ),
                        uniedit_btnEditAddSpob );

   if ( !widget_exists( wid, "btnEditTags" ) )
      window_addButton( wid, -20 - ( 20 + BUTTON_WIDTH ) * 2, y + 3,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnEditTags",
                        _( "Edit Tags" ), uniedit_btnEditTags );
}

/**
 * @brief Closes the system property editor, saving the changes made.
 */
static void uniedit_editSysClose( unsigned int wid, const char *name )
{
   StarSystem *sys;
   double      scale;

   /* We already know the system exists because we checked when opening the
    * dialog. */
   sys = uniedit_sys[0];

   /* Changes in radius need to scale the system spob positions. */
   scale = atof( window_getInput( wid, "inpRadius" ) ) / sys->radius;
   sysedit_sysScale( sys, scale );

   sys->spacedust       = atoi( window_getInput( wid, "inpDust" ) );
   sys->interference    = atof( window_getInput( wid, "inpInterference" ) );
   sys->nebu_density    = atof( window_getInput( wid, "inpNebula" ) );
   sys->nebu_volatility = atof( window_getInput( wid, "inpVolatility" ) );
   sys->nebu_hue        = atof( window_getInput( wid, "inpHue" ) ) / 360.;

   /* Reconstruct universe presences. */
   space_reconstructPresences();
   safelanes_recalculate();

   /* Text might need changing. */
   uniedit_selectText();

   if ( conf.devautosave )
      dsys_saveSystem( uniedit_sys[0] );

   /* Close the window. */
   window_close( wid, name );
}

/**
 * @brief Removes a selected spob.
 */
static void uniedit_btnEditRmSpob( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected;
   int         ret;

   /* Get selection. */
   selected = toolkit_getList( wid, "lstSpobs" );

   /* Make sure it's valid. */
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;

   /* Remove the spob. */
   ret = system_rmVirtualSpob( uniedit_sys[0], selected );
   if ( ret != 0 ) {
      dialogue_alert( _( "Failed to remove virtual spob '%s'!" ), selected );
      return;
   }

   /* Run galaxy modifications. */
   space_reconstructPresences();
   economy_execQueued();

   uniedit_editGenList( wid );
}

/**
 * @brief Adds a new virtual spob.
 */
static void uniedit_btnEditAddSpob( unsigned int parent, const char *unused )
{
   (void)parent;
   (void)unused;
   unsigned int       wid;
   const VirtualSpob *va;
   char             **str;
   int                h;

   /* Get all spobs. */
   va = virtualspob_getAll();
   if ( array_size( va ) == 0 ) {
      dialogue_alert( _( "No virtual spobs to add! Please add virtual spobs to "
                         "the '%s' directory first." ),
                      VIRTUALSPOB_DATA_PATH );
      return;
   }

   /* Create the window. */
   wid = window_create( "wdwAddaVirtualSpob", _( "Add a Virtual Spob" ), -1, -1,
                        UNIEDIT_EDIT_WIDTH, UNIEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   /* Add virtual spob list. */
   str = malloc( sizeof( char * ) * array_size( va ) );
   for ( int i = 0; i < array_size( va ); i++ )
      str[i] = strdup( va[i].name );
   h = UNIEDIT_EDIT_HEIGHT - 60 - ( BUTTON_HEIGHT + 20 );
   window_addList( wid, 20, -40, UNIEDIT_EDIT_WIDTH - 40, h, "lstSpobs", str,
                   array_size( va ), 0, NULL, NULL );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                     _( "Close" ), window_close );

   /* Add button. */
   window_addButton( wid, -20 - ( BUTTON_WIDTH + 20 ), 20, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnAdd", _( "Add" ),
                     uniedit_btnEditAddSpobAdd );
}

/**
 * @brief Actually adds the virtual spob.
 */
static void uniedit_btnEditAddSpobAdd( unsigned int wid, const char *unused )
{
   const char *selected;
   int         ret;

   /* Get selection. */
   selected = toolkit_getList( wid, "lstSpobs" );
   if ( selected == NULL )
      return;

   /* Add virtual presence. */
   ret = system_addVirtualSpob( uniedit_sys[0], selected );
   if ( ret != 0 ) {
      dialogue_alert( _( "Failed to add virtual spob '%s'!" ), selected );
      return;
   }

   /* Run galaxy modifications. */
   space_reconstructPresences();
   economy_execQueued();

   /* Regenerate the list. */
   uniedit_editGenList( uniedit_widEdit );

   if ( conf.devautosave )
      dsys_saveSystem( uniedit_sys[0] );

   /* Close the window. */
   window_close( wid, unused );
}

/**
 * @brief Edits a spob's tags.
 */
static void uniedit_btnEditTags( unsigned int wid, const char *unused )
{
   (void)unused;
   int y, w, bw;

   /* Create the window. */
   wid = window_create( "wdwSystemTagsEditor", _( "System Tags Editor" ), -1,
                        -1, UNIEDIT_EDIT_WIDTH, UNIEDIT_EDIT_HEIGHT );
   window_setCancel( wid, uniedit_btnTagsClose );

   w  = ( UNIEDIT_EDIT_WIDTH - 40 - 15 ) / 2.;
   bw = ( UNIEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   /* Close button. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     uniedit_btnTagsClose );
   y = 20 + BUTTON_HEIGHT + 15;

   /* Remove button. */
   window_addButton( wid, -20 - ( w + 15 ), y, w, BUTTON_HEIGHT, "btnRm",
                     _( "Rm Tag" ), uniedit_btnRmTag );

   /* Add button. */
   window_addButton( wid, -20, y, w, BUTTON_HEIGHT, "btnAdd", _( "Add Tag" ),
                     uniedit_btnAddTag );

   /* New tag. */
   window_addButton( wid, -20 - ( w + 15 ), 20, w, BUTTON_HEIGHT, "btnNew",
                     _( "New Tag" ), uniedit_btnNewTag );

   /* Generate list of tags. */
   if ( uniedit_tagslist == NULL ) {
      StarSystem *systems_all = system_getAll();
      uniedit_tagslist        = array_create( char        *);
      for ( int i = 0; i < array_size( systems_all ); i++ ) {
         StarSystem *s = &systems_all[i];
         for ( int j = 0; j < array_size( s->tags ); j++ ) {
            char *t     = s->tags[j];
            int   found = 0;
            for ( int k = 0; k < array_size( uniedit_tagslist ); k++ )
               if ( strcmp( uniedit_tagslist[k], t ) == 0 ) {
                  found = 1;
                  break;
               }
            if ( !found )
               array_push_back( &uniedit_tagslist, strdup( t ) );
         }
      }
      qsort( uniedit_tagslist, array_size( uniedit_tagslist ), sizeof( char * ),
             strsort );
   }

   uniedit_genTagsList( wid );
}

/*
 * Tags are closed so update tags.
 */
static void uniedit_btnTagsClose( unsigned int wid, const char *unused )
{
   char        buf[STRMAX_SHORT];
   StarSystem *s = uniedit_sys[0];
   int         l = scnprintf( buf, sizeof( buf ), "#n%s#0", _( "Tags: " ) );
   for ( int i = 0; i < array_size( s->tags ); i++ )
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s",
                      ( ( i > 0 ) ? ", " : "" ), s->tags[i] );
   window_modifyText( uniedit_widEdit, "txtTags", buf );

   window_close( wid, unused );
}

/**
 * @brief Generates the spob tech list.
 */
static void uniedit_genTagsList( unsigned int wid )
{
   StarSystem *s;
   char      **have, **lack;
   int         n, x, y, w, h, hpos, lpos, empty;

   hpos = lpos = -1;

   /* Destroy if exists. */
   if ( widget_exists( wid, "lstTagsHave" ) &&
        widget_exists( wid, "lstTagsLacked" ) ) {
      hpos = toolkit_getListPos( wid, "lstTagsHave" );
      lpos = toolkit_getListPos( wid, "lstTagsLacked" );
      window_destroyWidget( wid, "lstTagsHave" );
      window_destroyWidget( wid, "lstTagsLacked" );
   }

   s = uniedit_sys[0];
   w = ( UNIEDIT_EDIT_WIDTH - 40 - 15 ) / 2.;
   x = -20 - w - 15;
   y = 20 + BUTTON_HEIGHT * 2 + 30;
   h = UNIEDIT_EDIT_HEIGHT - y - 30;

   /* Get all the techs the spob has. */
   n = array_size( s->tags );
   if ( n > 0 ) {
      have = malloc( n * sizeof( char * ) );
      for ( int i = 0; i < n; i++ )
         have[i] = strdup( s->tags[i] );
      empty = 0;
   } else {
      have      = malloc( sizeof( char      *) );
      have[n++] = strdup( _( "None" ) );
      empty     = 1;
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTagsHave", have, n, 0, NULL,
                   uniedit_btnRmTag );
   x += w + 15;

   /* Omit the techs that the spob already has from the list.  */
   n    = 0;
   lack = malloc( array_size( uniedit_tagslist ) * sizeof( char * ) );
   for ( int i = 0; i < array_size( uniedit_tagslist ); i++ ) {
      char *t = uniedit_tagslist[i];
      if ( empty )
         lack[n++] = strdup( t );
      else {
         int found = 0;
         for ( int j = 0; j < array_size( s->tags ); j++ )
            if ( strcmp( s->tags[j], t ) == 0 ) {
               found = 1;
               break;
            }
         if ( !found )
            lack[n++] = strdup( t );
      }
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTagsLacked", lack, n, 0, NULL,
                   uniedit_btnAddTag );

   /* Restore positions. */
   if ( hpos != -1 && lpos != -1 ) {
      toolkit_setListPos( wid, "lstTagsHave", hpos );
      toolkit_setListPos( wid, "lstTagsLacked", lpos );
   }
}

/**
 * @brief Adds a tech to a spob.
 */
static void uniedit_btnAddTag( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected;
   StarSystem *s;

   selected = toolkit_getList( wid, "lstTagsLacked" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;

   s = uniedit_sys[0];
   if ( s->tags == NULL )
      s->tags = array_create( char * );
   array_push_back( &s->tags, strdup( selected ) );

   /* Regenerate the list. */
   uniedit_genTagsList( wid );
}

/**
 * @brief Removes a tech from a spob.
 */
static void uniedit_btnRmTag( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected;
   StarSystem *s;
   int         i;

   selected = toolkit_getList( wid, "lstTagsHave" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;

   s = uniedit_sys[0];
   for ( i = 0; i < array_size( s->tags ); i++ )
      if ( strcmp( selected, s->tags[i] ) == 0 )
         break;
   if ( i >= array_size( s->tags ) )
      return;
   free( s->tags[i] );
   array_erase( &s->tags, &s->tags[i], &s->tags[i + 1] );

   /* Regenerate the list. */
   uniedit_genTagsList( wid );
}

/**
 * @brief Adds a tech to a system.
 */
static void uniedit_btnNewTag( unsigned int wid, const char *unused )
{
   (void)unused;
   StarSystem *s;

   char *tag =
      dialogue_input( _( "Add New System Tag" ), 1, 128,
                      _( "Please write the new tag to add to the system." ) );
   if ( tag == NULL )
      return;

   s = uniedit_sys[0];
   if ( s->tags == NULL )
      s->tags = array_create( char * );
   array_push_back( &s->tags, tag ); /* gets freed later */

   /* Also add to list of all tags. */
   array_push_back( &uniedit_tagslist, strdup( tag ) );

   /* Regenerate the list. */
   uniedit_genTagsList( wid );
}

/**
 * @brief Renames the systems in the system editor.
 */
static void uniedit_btnEditRename( unsigned int wid, const char *unused )
{
   (void)unused;
   char buf[STRMAX_SHORT];

   /* Rename systems. */
   uniedit_renameSys();

   /* Update text. */
   snprintf( buf, sizeof( buf ), "%s #n%s", _( "Name:" ),
             ( array_size( uniedit_sys ) > 1 ) ? _( "#rvarious" )
                                               : uniedit_sys[0]->name );
   window_modifyText( wid, "txtName", buf );
}

/**
 * @brief Actually adds the virtual spob.
 */
static void uniedit_btnViewModeSet( unsigned int wid, const char *unused )
{
   const char *selected;
   int         pos;

   /* Check default. */
   pos                  = toolkit_getListPos( wid, "lstViewModes" );
   uniedit_view_faction = -1;
   if ( pos == 0 ) {
      uniedit_viewmode = UNIEDIT_VIEW_DEFAULT;
      window_close( wid, unused );
      return;
   } else if ( pos == 1 ) {
      uniedit_viewmode = UNIEDIT_VIEW_VIRTUALSPOBS;
      window_close( wid, unused );
      return;
   } else if ( pos == 2 ) {
      uniedit_viewmode = UNIEDIT_VIEW_RADIUS;
      window_close( wid, unused );
      return;
   } else if ( pos == 3 ) {
      uniedit_viewmode = UNIEDIT_VIEW_NOLANES;
      window_close( wid, unused );
      return;
   } else if ( pos == 4 ) {
      uniedit_viewmode = UNIEDIT_VIEW_BACKGROUND;
      window_close( wid, unused );
      return;
   } else if ( pos == 5 ) {
      uniedit_viewmode = UNIEDIT_VIEW_ASTEROIDS;
      window_close( wid, unused );
      return;
   } else if ( pos == 6 ) {
      uniedit_viewmode = UNIEDIT_VIEW_INTERFERENCE;
      window_close( wid, unused );
      return;
   } else if ( pos == 7 ) {
      uniedit_viewmode = UNIEDIT_VIEW_TECH;
      window_close( wid, unused );
      return;
   } else if ( pos == 8 ) {
      uniedit_viewmode = UNIEDIT_VIEW_PRESENCE_SUM;
      window_close( wid, unused );
      return;
   }

   /* Get selection. */
   selected = toolkit_getList( wid, "lstViewModes" );
   if ( selected == NULL )
      return;

   uniedit_viewmode     = UNIEDIT_VIEW_PRESENCE;
   uniedit_view_faction = faction_get( selected );

   /* Close the window. */
   window_close( wid, unused );
}

static void uniedit_chkNolanes( unsigned int wid, const char *wgtname )
{
   int         s   = window_checkboxState( wid, wgtname );
   StarSystem *sys = uniedit_sys[0];
   if ( s )
      sys_setFlag( sys, SYSTEM_NOLANES );
   else
      sys_rmFlag( sys, SYSTEM_NOLANES );
}
