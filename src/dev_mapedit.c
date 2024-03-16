/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file dev_mapedit.c
 *
 * @brief Handles the star system editor.
 */
/** @cond */
#include "SDL.h"
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "dev_mapedit.h"

#include "array.h"
#include "commodity.h"
#include "conf.h"
#include "dev_spob.h"
#include "dev_sysedit.h"
#include "dev_system.h"
#include "dev_uniedit.h"
#include "dialogue.h"
#include "load.h"
#include "map.h"
#include "mapData.h"
#include "ndata.h"
#include "nfile.h"
#include "nstring.h"
#include "opengl.h"
#include "outfit.h"
#include "pause.h"
#include "space.h"
#include "tk/toolkit_priv.h"
#include "toolkit.h"
#include "unidiff.h"

extern StarSystem *systems_stack;

#define BUTTON_WIDTH 80  /**< Map button width. */
#define BUTTON_HEIGHT 30 /**< Map button height. */

#define MAPEDIT_EDIT_WIDTH 400  /**< System editor width. */
#define MAPEDIT_EDIT_HEIGHT 450 /**< System editor height. */

#define MAPEDIT_DRAG_THRESHOLD 300 /**< Drag threshold. */
#define MAPEDIT_MOVE_THRESHOLD 10  /**< Movement threshold. */

#define MAPEDIT_ZOOM_STEP 1.2 /**< Factor to zoom by for each zoom level. */
#define MAPEDIT_ZOOM_MAX 5    /**< Maximum mapedit zoom level (close). */
#define MAPEDIT_ZOOM_MIN -5   /**< Minimum mapedit zoom level (far). */

#define MAPEDIT_OPEN_WIDTH 800     /**< Open window width. */
#define MAPEDIT_OPEN_HEIGHT 500    /**< Open window height. */
#define MAPEDIT_OPEN_TXT_WIDTH 300 /**< Text width. */

#define MAPEDIT_SAVE_WIDTH 800     /**< Open window width. */
#define MAPEDIT_SAVE_HEIGHT 500    /**< Open window height. */
#define MAPEDIT_SAVE_TXT_WIDTH 300 /**< Text width. */

typedef struct mapOutfitsList_s {
   char     *fileName;
   char     *mapName;
   char     *description;
   char     *gfx_store;
   int       numSystems;
   credits_t price;
   int       rarity;
} mapOutfitsList_t;

static mapOutfitsList_t *mapList =
   NULL; /* Array of map outfits for displaying in the Open window. */

static unsigned int mapedit_wid      = 0;  /**< Sysedit wid. */
static double       mapedit_xpos     = 0.; /**< Viewport X position. */
static double       mapedit_ypos     = 0.; /**< Viewport Y position. */
static double       mapedit_zoom     = 1.; /**< Viewport zoom level. */
static int          mapedit_moved    = 0;  /**< Space moved since mouse down. */
static unsigned int mapedit_dragTime = 0;  /**< Tick last started to drag. */
static int          mapedit_drag     = 0;  /**< Dragging viewport around. */
static StarSystem **mapedit_sys      = NULL; /**< Selected systems. */
static int          mapedit_iLastClickedSystem =
   0; /**< Last clicked system (use with system_getIndex). */
static int mapedit_tadd = 0; /**< Temporarily clicked system should be added. */
static int mapedit_nsys = 0; /**< Number of selected systems. */
static int mapedit_msys = 0; /**< Memory allocated for selected systems. */
static double       mapedit_mx           = 0.;   /**< X mouse position. */
static double       mapedit_my           = 0.;   /**< Y mouse position. */
static unsigned int mapedit_widLoad      = 0;    /**< Load Map Outfit wid. */
static char        *mapedit_sLoadMapName = NULL; /**< Loaded Map Outfit. */
static int          mapedit_nospob       = 0;    /**< Don't save spobs. */
static int          mapedit_hidden = 0; /**< Mark hidden jump lanes too. */

/*
 * Universe editor Prototypes.
 */
/* Selection. */
static void mapedit_deselect( void );
static void mapedit_selectAdd( StarSystem *sys );
static void mapedit_selectRm( StarSystem *sys );
/* Custom system editor widget. */
static void mapedit_buttonZoom( unsigned int wid, const char *str );
static void mapedit_render( double bx, double by, double w, double h,
                            void *data );
static void mapedit_focusLose( unsigned int wid, const char *wgtname );
static int  mapedit_mouse( unsigned int wid, const SDL_Event *event, double mx,
                           double my, double w, double h, double xr, double yr,
                           void *data );
/* Button functions. */
static void mapedit_close( unsigned int wid, const char *wgt );
static void mapedit_btnOpen( unsigned int wid_unused, const char *unused );
static void mapedit_btnSaveMapAs( unsigned int wid_unused, const char *unused );
static void mapedit_clear( unsigned int wid_unused, const char *unused );
static void mapedit_chkSpob( unsigned int wid, const char *wgtname );
static void mapedit_chkHidden( unsigned int wid, const char *wgtname );
/* Keybindings handling. */
static int mapedit_keys( unsigned int wid, SDL_Keycode key, SDL_Keymod mod,
                         int isrepeat );
/* Loading of Map files. */
static void mapedit_loadMapMenu_open( void );
static void mapedit_loadMapMenu_close( unsigned int wdw, const char *str );
static void mapedit_loadMapMenu_update( unsigned int wdw, const char *str );
static void mapedit_loadMapMenu_load( unsigned int wdw, const char *str );
/* Saving of Map files. */
static int mapedit_saveMap( StarSystem **uniedit_sys, mapOutfitsList_t *ns );
/* Management of last loaded/saved Map file. */
void mapedit_setGlobalLoadedInfos( mapOutfitsList_t *ns );
/* Management of Map files list. */
static int  mapedit_mapsList_refresh( void );
static void mapsList_free( void );

/**
 * @brief Opens the system editor interface.
 */
void mapedit_open( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   unsigned int wid;
   int          buttonHPos = 0;
   int          buttonVPos = 1;
   int          textPos    = 0;
   int          linesPos   = 0;
   int          curLines   = 0;
   int          lineHeight = gl_smallFont.h + 5;
   int          parHeight  = 10;

   /* Pause. */
   pause_game();

   /* Must have no diffs applied. */
   diff_clear();

   /* Reset some variables. */
   mapedit_drag = 0;
   mapedit_tadd = 0;
   mapedit_zoom = 1.;
   mapedit_xpos = 0.;
   mapedit_ypos = 0.;

   /* Create the window. */
   wid = window_create( "wdwMapOutfitEditor", _( "Map Outfit Editor" ), -1, -1,
                        -1, -1 );
   window_handleKeys( wid, mapedit_keys );
   mapedit_wid = wid;

   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 350, SCREEN_H - 100, "cstSysEdit",
                   1, mapedit_render, mapedit_mouse, NULL, mapedit_focusLose,
                   NULL );

   /* Button : reset the current map. */
   buttonHPos = 2;
   window_addButtonKey( wid, -20 - ( BUTTON_WIDTH + 20 ) * buttonHPos,
                        20 + ( BUTTON_HEIGHT + 20 ) * buttonVPos, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnClear", _( "Reset" ), mapedit_clear,
                        SDLK_r );
   buttonHPos--;

   /* Button : open map file. */
   window_addButtonKey( wid, -20 - ( BUTTON_WIDTH + 20 ) * buttonHPos,
                        20 + ( BUTTON_HEIGHT + 20 ) * buttonVPos, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnOpen", _( "Open" ), mapedit_btnOpen,
                        SDLK_o );
   buttonHPos--;

   /* Button : save current map to file. */
   window_addButtonKey( wid, -20 - ( BUTTON_WIDTH + 20 ) * buttonHPos,
                        20 + ( BUTTON_HEIGHT + 20 ) * buttonVPos, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnSaveAs", _( "Save" ),
                        mapedit_btnSaveMapAs, SDLK_s );
   buttonHPos = 0;
   buttonVPos--;

   /* Button : exit editor. */
   window_addButtonKey( wid, -20 - ( BUTTON_WIDTH + 20 ) * buttonHPos,
                        20 + ( BUTTON_HEIGHT + 20 ) * buttonVPos, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnClose", _( "Exit" ), mapedit_close,
                        SDLK_x );

   /* Filename. */
   window_addText( wid, -200, -40 - textPos * parHeight - linesPos * lineHeight,
                   100, lineHeight, 0, "txtSFileName", &gl_smallFont, NULL,
                   _( "File Name:" ) );
   window_addInput( wid, -30, -40 - textPos * parHeight - linesPos * lineHeight,
                    170, lineHeight, "inpFileName", 1024, 1, &gl_smallFont );
   textPos++;
   linesPos++;

   /* Map name. */
   window_addText( wid, -200, -40 - textPos * parHeight - linesPos * lineHeight,
                   100, lineHeight, 0, "txtSMapName", &gl_smallFont, NULL,
                   _( "Map Name:" ) );
   window_addInput( wid, -30, -40 - textPos * parHeight - linesPos * lineHeight,
                    170, lineHeight, "inpMapName", 1024, 1, &gl_smallFont );
   textPos++;
   linesPos++;

   /* Map description. */
   curLines = 7;
   window_addText( wid, -20, -40 - textPos * parHeight - linesPos * lineHeight,
                   300 - 20, lineHeight, 0, "txtSDescription", &gl_smallFont,
                   NULL, _( "Description:" ) );
   window_addInput( wid, -20,
                    -40 - textPos * parHeight - ( linesPos + 1 ) * lineHeight,
                    300 - 20, curLines * lineHeight, "inpDescription", 32768, 0,
                    &gl_smallFont );
   textPos++;
   linesPos += ( curLines + 1 );

   /* Current Map # of systems. */
   curLines = 1;
   window_addText( wid, -20, -40 - textPos * parHeight - linesPos * lineHeight,
                   300 - 20, 20, 0, "txtSCurrentNumSystems", &gl_smallFont,
                   NULL, _( "Number of Systems (up to 100):" ) );
   window_addText( wid, -20, -40 - textPos * parHeight - linesPos * lineHeight,
                   60, curLines * lineHeight, 0, "txtCurrentNumSystems",
                   &gl_smallFont, NULL, "N/A" );
   textPos++;
   linesPos++;

   /* Presence. */
   curLines = 5;
   window_addText( wid, -20, -40 - textPos * parHeight - linesPos * lineHeight,
                   300 - 20, 20, 0, "txtSPresence", &gl_smallFont, NULL,
                   _( "Presence:" ) );
   window_addText( wid, -20,
                   -40 - textPos * parHeight - ( linesPos + 1 ) * lineHeight,
                   300 - 20, curLines * lineHeight, 0, "txtPresence",
                   &gl_smallFont, NULL, _( "No selection" ) );
   textPos++;
   linesPos += curLines + 1;

   /* Outift attributes. */
   curLines = 1;
   window_addText( wid, -200, -40 - textPos * parHeight - linesPos * lineHeight,
                   100, 20, 0, "txtSPrice", &gl_smallFont, NULL,
                   _( "Price:" ) );
   window_addInput( wid, -30, -40 - textPos * parHeight - linesPos * lineHeight,
                    170, lineHeight, "inpPrice", 64, 1, &gl_smallFont );
   window_setInputFilter( wid, "inpPrice", INPUT_FILTER_NUMBER );
   textPos++;
   linesPos += curLines;

   curLines = 1;
   window_addText( wid, -200, -40 - textPos * parHeight - linesPos * lineHeight,
                   100, 20, 0, "txtSGFX", &gl_smallFont, NULL,
                   _( "Graphics:" ) );
   window_addInput( wid, -30, -40 - textPos * parHeight - linesPos * lineHeight,
                    170, lineHeight, "inpGFX", 64, 1, &gl_smallFont );
   textPos++;
   linesPos += curLines;

   curLines = 1;
   window_addText( wid, -200, -40 - textPos * parHeight - linesPos * lineHeight,
                   100, 20, 0, "txtSRarity", &gl_smallFont, NULL,
                   _( "Rarity:" ) );
   window_addInput( wid, -30, -40 - textPos * parHeight - linesPos * lineHeight,
                    170, lineHeight, "inpRarity", 64, 1, &gl_smallFont );
   window_setInputFilter( wid, "inpRarity", INPUT_FILTER_NUMBER );
   textPos++;
   linesPos += curLines + 1;

   curLines       = 1;
   mapedit_nospob = 0;
   window_addCheckbox( wid, -30,
                       -40 - textPos * parHeight - linesPos * lineHeight, 170,
                       lineHeight, "chkSpob", _( "Ignore Spobs" ),
                       mapedit_chkSpob, mapedit_nospob );
   textPos++;
   linesPos += curLines + 1;

   curLines       = 1;
   mapedit_nospob = 0;
   window_addCheckbox( wid, -30,
                       -40 - textPos * parHeight - linesPos * lineHeight, 170,
                       lineHeight, "chkHidden", _( "Set Hidden Jumps" ),
                       mapedit_chkHidden, mapedit_nospob );
   textPos++;
   linesPos += curLines + 1;

   curLines = 4;
   window_addText(
      wid, -20, -40 - textPos * parHeight - linesPos * lineHeight, 300 - 20,
      curLines * lineHeight, 0, "txtSWarning", &gl_smallFont, NULL,
      _( "Warning: Editor can't (yet) manage which details are mapped within a "
         "system. Review its changes before committing." ) );
   textPos++;
   linesPos += curLines + 1;

   /* Zoom buttons */
   window_addButtonKey( wid, 40, 20, 30, 30, "btnZoomIn", "+",
                        mapedit_buttonZoom, SDLK_EQUALS );
   window_addButtonKey( wid, 80, 20, 30, 30, "btnZoomOut", "-",
                        mapedit_buttonZoom, SDLK_MINUS );

   /* Selected text. */
   window_addText( wid, 140, 10, SCREEN_W - 350 - 30 - 30 - BUTTON_WIDTH - 20,
                   30, 0, "txtSelected", &gl_smallFont, NULL, NULL );

   /* Deselect everything. */
   mapedit_deselect();
}

/**
 * @brief Handles keybindings.
 */
static int mapedit_keys( unsigned int wid, SDL_Keycode key, SDL_Keymod mod,
                         int isrepeat )
{
   (void)mod;
   (void)isrepeat;

   switch ( key ) {
   /* Mode changes. */
   case SDLK_ESCAPE:
      mapedit_close( wid, "Close" );
      return 1;

   default:
      return 0;
   }
}

/**
 * @brief Closes the system editor widget.
 */
static void mapedit_close( unsigned int wid, const char *wgt )
{
   /* Frees some memory. */
   mapedit_deselect();
   mapsList_free();

   /* Reconstruct jumps. */
   systems_reconstructJumps();

   /* Unpause. */
   unpause_game();

   /* Close the window. */
   window_close( wid, wgt );
}

/**
 * @brief Closes the system editor widget.
 */
static void mapedit_clear( unsigned int wid, const char *unused )
{
   (void)wid;
   (void)unused;

   /* Clear the map. */
   mapedit_deselect();
}

static void mapedit_chkSpob( unsigned int wid, const char *wgtname )
{
   mapedit_nospob = window_checkboxState( wid, wgtname );
}

static void mapedit_chkHidden( unsigned int wid, const char *wgtname )
{
   mapedit_hidden = window_checkboxState( wid, wgtname );
}

/**
 * @brief Opens up a map file.
 */
static void mapedit_btnOpen( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   mapedit_loadMapMenu_open();
}

/**
 * @brief System editor custom widget rendering.
 */
static void mapedit_render( double bx, double by, double w, double h,
                            void *data )
{
   (void)data;
   double x, y, r;

   /* Parameters. */
   map_renderParams( bx, by, mapedit_xpos, mapedit_ypos, w, h, mapedit_zoom, &x,
                     &y, &r );

   uniedit_renderMap( bx, by, w, h, x, y, mapedit_zoom, r );

   /* Render the selected system selections. */
   for ( int i = 0; i < mapedit_nsys; i++ ) {
      StarSystem *sys = mapedit_sys[i];
      gl_renderCircle( x + sys->pos.x * mapedit_zoom,
                       y + sys->pos.y * mapedit_zoom, 1.8 * r, &cRed, 0 );
      gl_renderCircle( x + sys->pos.x * mapedit_zoom,
                       y + sys->pos.y * mapedit_zoom, 2.0 * r, &cRed, 0 );
   }

   /* Render last clicked system */
   if ( mapedit_iLastClickedSystem != 0 ) {
      StarSystem *sys = system_getIndex( mapedit_iLastClickedSystem );
      gl_renderCircle( x + sys->pos.x * mapedit_zoom,
                       y + sys->pos.y * mapedit_zoom, 2.4 * r, &cBlue, 0 );
      gl_renderCircle( x + sys->pos.x * mapedit_zoom,
                       y + sys->pos.y * mapedit_zoom, 2.6 * r, &cBlue, 0 );
      gl_renderCircle( x + sys->pos.x * mapedit_zoom,
                       y + sys->pos.y * mapedit_zoom, 2.8 * r, &cBlue, 0 );
   }
}

/**
 * @brief Called when it's de-focused.
 */
static void mapedit_focusLose( unsigned int wid, const char *wgtname )
{
   (void)wid;
   (void)wgtname;
   mapedit_drag = 0;
}

/**
 * @brief System editor custom widget mouse handling.
 */
static int mapedit_mouse( unsigned int wid, const SDL_Event *event, double mx,
                          double my, double w, double h, double xr, double yr,
                          void *data )
{
   (void)data;
   const double t = 15. * 15.; /* threshold */

   switch ( event->type ) {
   case SDL_MOUSEWHEEL:
      /* Must be in bounds. */
      if ( ( mx < 0. ) || ( mx > w ) || ( my < 0. ) || ( my > h ) )
         return 0;
      if ( event->wheel.y > 0 )
         mapedit_buttonZoom( 0, "btnZoomIn" );
      else if ( event->wheel.y < 0 )
         mapedit_buttonZoom( 0, "btnZoomOut" );
      return 1;

   case SDL_MOUSEBUTTONDOWN:
      /* Must be in bounds. */
      if ( ( mx < 0. ) || ( mx > w ) || ( my < 0. ) || ( my > h ) )
         return 0;
      window_setFocus( wid, "cstSysEdit" );

      /* Zooming */
      if ( event->button.button == SDL_BUTTON_X1 ) {
         mapedit_buttonZoom( 0, "btnZoomIn" );
         return 1;
      } else if ( event->button.button == SDL_BUTTON_X2 ) {
         mapedit_buttonZoom( 0, "btnZoomOut" );
         return 1;
      }

      /* selecting star system */
      else {
         mx -= w / 2 - mapedit_xpos;
         my -= h / 2 - mapedit_ypos;

         for ( int i = 0; i < array_size( systems_stack ); i++ ) {
            double      x, y;
            StarSystem *sys = system_getIndex( i );

            /* get position */
            x = sys->pos.x * mapedit_zoom;
            y = sys->pos.y * mapedit_zoom;

            if ( ( pow2( mx - x ) + pow2( my - y ) ) < t ) {
               int found = 0;

               /* Set last clicked system */
               mapedit_iLastClickedSystem = i;

               /* Try to find in selected systems. */
               for ( int j = 0; j < mapedit_nsys; j++ ) {
                  /* Must match. */
                  if ( mapedit_sys[j] == sys ) {
                     found = 1;
                     break;
                  } else
                     continue;
               }

               /* Toggle system selection. */
               if ( found )
                  mapedit_selectRm( sys );
               else
                  mapedit_selectAdd( sys );
               return 1;
            }
         }

         /* Start dragging the viewport. */
         mapedit_drag     = 1;
         mapedit_dragTime = SDL_GetTicks();
         mapedit_moved    = 0;
      }
      break;

   case SDL_MOUSEBUTTONUP:
      /* Handles dragging viewport around. */
      mapedit_drag = 0;
      break;

   case SDL_MOUSEMOTION:
      /* Update mouse positions. */
      mapedit_mx = mx;
      mapedit_my = my;

      /* Handle dragging. */
      if ( mapedit_drag ) {
         /* axis is inverted */
         mapedit_xpos -= xr;
         mapedit_ypos += yr;

         /* Update mouse movement. */
         mapedit_moved += ABS( xr ) + ABS( yr );
      }
      break;
   }

   return 0;
}

/**
 * @brief Deselects selected targets.
 */
static void mapedit_deselect( void )
{
   if ( mapedit_nsys > 0 )
      free( mapedit_sys );
   mapedit_sys  = NULL;
   mapedit_nsys = 0;
   mapedit_msys = 0;

   /* Change window stuff. */
   window_modifyText( mapedit_wid, "txtSelected", _( "No selection" ) );
   window_modifyText( mapedit_wid, "txtCurrentNumSystems", "0" );
}

/**
 * @brief Adds a system to the selection.
 */
static void mapedit_selectAdd( StarSystem *sys )
{
   /* Workaround for BUG found in memory allocation */
   if ( mapedit_nsys == 100 )
      return;

   /* Allocate if needed. */
   if ( mapedit_msys < mapedit_nsys + 1 ) {
      if ( mapedit_msys == 0 )
         mapedit_msys = 1;
      mapedit_msys *= 2;
      mapedit_sys =
         realloc( mapedit_sys, sizeof( StarSystem * ) * mapedit_msys );
   }

   /* Add system. */
   mapedit_sys[mapedit_nsys] = sys;
   mapedit_nsys++;

   /* Set text again. */
   mapedit_selectText();
}

/**
 * @brief Removes a system from the selection.
 */
static void mapedit_selectRm( StarSystem *sys )
{
   for ( int i = 0; i < mapedit_nsys; i++ ) {
      if ( mapedit_sys[i] == sys ) {
         mapedit_nsys--;
         memmove( &mapedit_sys[i], &mapedit_sys[i + 1],
                  sizeof( StarSystem * ) * ( mapedit_nsys - i ) );
         mapedit_selectText();
         return;
      }
   }
}

/**
 * @brief Sets the selected system text.
 */
void mapedit_selectText( void )
{
   int  l;
   char buf[STRMAX_SHORT];

   /* Built list of all selected systems names */
   l = 0;
   for ( int i = 0; i < mapedit_nsys; i++ ) {
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s%s", mapedit_sys[i]->name,
                      ( i == mapedit_nsys - 1 ) ? "" : ", " );
   }

   if ( l == 0 )
      /* Change display to reflect that no system is selected */
      mapedit_deselect();
   else {
      /* Display list of selected systems */
      window_modifyText( mapedit_wid, "txtSelected", buf );

      /* Display number of selected systems */
      snprintf( &buf[0], 4, "%i", mapedit_nsys );
      window_modifyText( mapedit_wid, "txtCurrentNumSystems", buf );

      /* Compute and display presence text. */
      if ( mapedit_iLastClickedSystem != 0 ) {
         StarSystem *sys = system_getIndex( mapedit_iLastClickedSystem );
         map_updateFactionPresence( mapedit_wid, "txtPresence", sys, 1 );
         snprintf( &buf[0], sizeof( buf ), _( "Presence (%s)" ), sys->name );
         window_modifyText( mapedit_wid, "txtSPresence", buf );
      } else {
         window_modifyText( mapedit_wid, "txtSPresence", _( "Presence" ) );
         window_modifyText( mapedit_wid, "txtPresence",
                            _( "No system yet clicked" ) );
      }
   }
}

/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void mapedit_buttonZoom( unsigned int wid, const char *str )
{
   (void)wid;
   /* Transform coords to normal. */
   mapedit_xpos /= mapedit_zoom;
   mapedit_ypos /= mapedit_zoom;

   /* Apply zoom. */
   if ( strcmp( str, "btnZoomIn" ) == 0 ) {
      mapedit_zoom *= MAPEDIT_ZOOM_STEP;
      mapedit_zoom =
         MIN( pow( MAPEDIT_ZOOM_STEP, MAPEDIT_ZOOM_MAX ), mapedit_zoom );
   } else if ( strcmp( str, "btnZoomOut" ) == 0 ) {
      mapedit_zoom /= MAPEDIT_ZOOM_STEP;
      mapedit_zoom =
         MAX( pow( MAPEDIT_ZOOM_STEP, MAPEDIT_ZOOM_MIN ), mapedit_zoom );
   }

   /* Transform coords back. */
   mapedit_xpos *= mapedit_zoom;
   mapedit_ypos *= mapedit_zoom;
}

/**
 * @brief Opens the load map outfit menu.
 */
void mapedit_loadMapMenu_open( void )
{
   unsigned int wid;
   char       **names;
   int          n;

   /* window */
   wid = window_create( "wdwOpenMapOutfit", _( "Open Map Outfit" ), -1, -1,
                        MAPEDIT_OPEN_WIDTH, MAPEDIT_OPEN_HEIGHT );
   mapedit_widLoad = wid;

   /* Default actions */
   window_setAccept( mapedit_widLoad, mapedit_loadMapMenu_load );
   window_setCancel( mapedit_widLoad, mapedit_loadMapMenu_close );

   /* Load list of map outfits */
   mapedit_mapsList_refresh();

   /* Load the maps */
   n = array_size( mapList );
   if ( n > 0 ) {
      names = malloc( sizeof( char * ) * n );
      for ( int i = 0; i < n; i++ ) {
         mapOutfitsList_t *ns = &mapList[i];
         names[i]             = strdup( ns->mapName );
      }
   }
   /* case there are no files */
   else {
      names    = malloc( sizeof( char    *) );
      names[0] = strdup( "None" );
      n        = 1;
   }

   /* Map info text. */
   window_addText( mapedit_widLoad, -20, -40, MAPEDIT_OPEN_TXT_WIDTH,
                   MAPEDIT_OPEN_HEIGHT - 40 - 20 - 2 * ( BUTTON_HEIGHT + 20 ),
                   0, "txtMapInfo", NULL, NULL, NULL );

   window_addList( mapedit_widLoad, 20, -50,
                   MAPEDIT_OPEN_WIDTH - MAPEDIT_OPEN_TXT_WIDTH - 60,
                   MAPEDIT_OPEN_HEIGHT - 110, "lstMapOutfits", names, n, 0,
                   mapedit_loadMapMenu_update, mapedit_loadMapMenu_load );

   /* Buttons */
   window_addButtonKey( mapedit_widLoad, -20, 20 + BUTTON_HEIGHT + 20,
                        BUTTON_WIDTH, BUTTON_HEIGHT, "btnLoad", _( "Load" ),
                        mapedit_loadMapMenu_load, SDLK_l );
   window_addButton( mapedit_widLoad, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
                     "btnBack", _( "Back" ), mapedit_loadMapMenu_close );
   window_addButton( mapedit_widLoad, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
                     "btnDelete", _( "Del" ), mapedit_loadMapMenu_close );
}

/**
 * @brief Updates the load menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void mapedit_loadMapMenu_update( unsigned int wdw, const char *str )
{
   (void)str;
   int               pos;
   mapOutfitsList_t *ns;
   const char       *save;
   char              buf[STRMAX_SHORT];

   /* Make sure list is ok. */
   save = toolkit_getList( wdw, "lstMapOutfits" );
   if ( strcmp( save, "None" ) == 0 )
      return;

   /* Get position. */
   pos = toolkit_getListPos( wdw, "lstMapOutfits" );
   ns  = &mapList[pos];

   /* Display text. */
   snprintf( buf, sizeof( buf ),
             _( "File Name:\n"
                "   %s\n"
                "Map name:\n"
                "   %s\n"
                "Description:\n"
                "   %s\n"
                "Systems:\n"
                "   %i" ),
             ns->fileName, ns->mapName, ns->description, ns->numSystems );

   window_modifyText( wdw, "txtMapInfo", buf );
}

/**
 * @brief Closes the load map outfit menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void mapedit_loadMapMenu_close( unsigned int wdw, const char *str )
{
   (void)str;
   window_destroy( wdw );
}

/**
 * @brief Load the selected Map.
 */
static void mapedit_loadMapMenu_load( unsigned int wdw, const char *str )
{
   (void)str;
   int               pos, len, compareLimit, i, found;
   mapOutfitsList_t *ns;
   const char       *save;
   char             *file, *name, *systemName;
   xmlNodePtr        node;
   xmlDocPtr         doc;
   StarSystem       *sys;

   /* Debug log */

   /* Make sure list is ok. */
   save = toolkit_getList( wdw, "lstMapOutfits" );
   if ( strcmp( save, "None" ) == 0 )
      return;

   /* Reset defaults. */
   mapedit_nospob = 0;
   mapedit_hidden = 0;

   /* Get position. */
   pos = toolkit_getListPos( wdw, "lstMapOutfits" );
   ns  = &mapList[pos];

   /* Display text. */
   len  = strlen( MAP_DATA_PATH ) + strlen( ns->fileName ) + 2;
   file = malloc( len );
   snprintf( file, len, "%s%s", MAP_DATA_PATH, ns->fileName );

   doc = xml_parsePhysFS( file );

   /* Get first node, normally "outfit" */
   node = doc->xmlChildrenNode;
   if ( node == NULL ) {
      free( file );
      xmlFreeDoc( doc );
      return;
   }

   if ( !xml_isNode( node, "outfit" ) ) {
      free( file );
      xmlFreeDoc( doc );
      return;
   }

   /* Get "name" property from the "outfit" node */
   xmlr_attr_strd( node, "name", name );
   if ( strcmp( ns->mapName, name ) != 0 ) {
      free( name );
      free( file );
      xmlFreeDoc( doc );
      return;
   } else {
      free( name );
      name = NULL;
   }

   /* Loop on the nodes to find <specific> node */
   node = node->xmlChildrenNode;
   do {
      xml_onlyNodes( node );

      if ( !xml_isNode( node, "specific" ) )
         continue;

      /* Break out of the loop, either with a correct outfitType or not */
      break;
   } while ( xml_nextNode( node ) );

   mapedit_deselect();

   /* Loop on the nodes to find all <sys> node */
   node = node->xmlChildrenNode;
   do {
      xml_onlyNodes( node );

      if ( !xml_isNode( node, "sys" ) )
         continue;

      /* Display "name" property from "sys" node and increment number of systems
       * found */
      xmlr_attr_strd( node, "name", systemName );

      /* Find system */
      found = 0;
      for ( i = 0; i < array_size( systems_stack ); i++ ) {
         sys          = system_getIndex( i );
         compareLimit = strlen( systemName );
         if ( strncmp( systemName, sys->name, compareLimit ) == 0 ) {
            found = 1;
            break;
         }
      }

      /* If system exists, select it */
      if ( found )
         mapedit_selectAdd( sys );
      free( systemName );
      systemName = NULL;
   } while ( xml_nextNode( node ) );

   mapedit_setGlobalLoadedInfos( ns );

   free( file );
   xmlFreeDoc( doc );

   window_destroy( wdw );
}

/**
 * @brief Save the current Map to selected file.
 */
static void mapedit_btnSaveMapAs( unsigned int wdw, const char *unused )
{
   (void)unused;
   mapOutfitsList_t ns;

   ns.fileName    = strdup( window_getInput( wdw, "inpFileName" ) );
   ns.mapName     = strdup( window_getInput( wdw, "inpMapName" ) );
   ns.description = strdup( window_getInput( wdw, "inpDescription" ) );
   ns.gfx_store   = strdup( window_getInput( wdw, "inpGFX" ) );
   ns.numSystems  = mapedit_nsys;
   ns.price       = atoll( window_getInput( wdw, "inpPrice" ) );
   ns.rarity      = atoi( window_getInput( wdw, "inpRarity" ) );

   mapedit_saveMap( mapedit_sys, &ns );

   free( ns.fileName );
   free( ns.mapName );
   free( ns.description );
   free( ns.gfx_store );
}

/**
 * @brief Set and display the global variables describing last loaded/saved file
 */
void mapedit_setGlobalLoadedInfos( mapOutfitsList_t *ns )
{
   char buf[32];

   /* Displaying info strings */
   window_setInput( mapedit_wid, "inpFileName", ns->fileName );
   window_setInput( mapedit_wid, "inpMapName", ns->mapName );
   window_setInput( mapedit_wid, "inpDescription", ns->description );
   window_setInput( mapedit_wid, "inpGFX", ns->gfx_store );
   snprintf( buf, sizeof( buf ), "%i", ns->numSystems );
   window_modifyText( mapedit_wid, "txtCurrentNumSystems", buf );
   snprintf( buf, sizeof( buf ), "%" CREDITS_PRI, ns->price );
   window_setInput( mapedit_wid, "inpPrice", buf );
   snprintf( buf, sizeof( buf ), "%i", ns->rarity );
   window_setInput( mapedit_wid, "inpRarity", buf );

   /* Local information. */
   free( mapedit_sLoadMapName );
   mapedit_sLoadMapName = strdup( ns->mapName );
}

/**
 * @brief Gets the list of all the maps names.
 *   from outfit_mapParse()
 *
 */
static int mapedit_mapsList_refresh( void )
{
   int               is_map, nSystems;
   xmlNodePtr        node, cur;
   xmlDocPtr         doc;
   char            **map_files;
   char             *file, *name, *outfitType;
   mapOutfitsList_t *newMapItem;

   mapsList_free();
   mapList = array_create( mapOutfitsList_t );

   map_files  = PHYSFS_enumerateFiles( MAP_DATA_PATH );
   newMapItem = NULL;
   for ( size_t i = 0; map_files[i] != NULL; i++ ) {
      char     *description = NULL;
      char     *gfx_store   = NULL;
      credits_t price       = 1000;
      int       rarity      = 0;

      SDL_asprintf( &file, "%s%s", MAP_DATA_PATH, map_files[i] );

      doc = xml_parsePhysFS( file );
      if ( doc == NULL ) {
         free( file );
         continue;
      }

      /* Get first node, normally "outfit" */
      node = doc->xmlChildrenNode;
      if ( node == NULL ) {
         free( file );
         xmlFreeDoc( doc );
         return -1;
      }

      if ( !xml_isNode( node, "outfit" ) ) {
         free( file );
         xmlFreeDoc( doc );
         return -1;
      }

      /* Get "name" property from the "outfit" node */
      xmlr_attr_strd( node, "name", name );

      /* Loop on the nodes to find <specific> node */
      node = node->xmlChildrenNode;
      do {
         is_map = 0;
         xml_onlyNodes( node );

         if ( !xml_isNode( node, "specific" ) ) {
            if ( xml_isNode( node, "general" ) ) {
               cur = node->children;
               do {
                  xml_onlyNodes( cur );
                  xmlr_str( cur, "description", description );
                  xmlr_long( cur, "price", price );
                  xmlr_int( cur, "rarity", rarity );
                  xmlr_str( cur, "gfx_store", gfx_store );
               } while ( xml_nextNode( cur ) );
            }
            continue;
         }

         /* Get the "type" property from "specific" node */
         xmlr_attr_strd( node, "type", outfitType );
         is_map = outfitType == NULL ? 0 : !strncmp( outfitType, "map", 3 );
         free( outfitType );

         /* Break out of the loop, either with a map or not */
         break;
      } while ( xml_nextNode( node ) );

      /* If it's not a map, we don't care. */
      if ( !is_map ) {
         free( name );
         free( file );
         xmlFreeDoc( doc );
         continue;
      }

      /* Loop on the nodes to find all <sys> node */
      nSystems = 0;
      node     = node->xmlChildrenNode;
      do {
         xml_onlyNodes( node );
         if ( !xml_isNode( node, "sys" ) )
            continue;

         /* Display "name" property from "sys" node and increment number of
          * systems found */
         nSystems++;
      } while ( xml_nextNode( node ) );

      /* If the map is a regular one, then load it into the list */
      if ( nSystems > 0 ) {
         newMapItem             = &array_grow( &mapList );
         newMapItem->numSystems = nSystems;
         newMapItem->fileName   = strdup( map_files[i] );
         newMapItem->mapName    = strdup( name );
         newMapItem->description =
            strdup( ( description != NULL ) ? description : "" );
         newMapItem->gfx_store =
            strdup( ( gfx_store != NULL ) ? gfx_store : "" );
         newMapItem->price  = price;
         newMapItem->rarity = rarity;
      }

      /* Clean up. */
      free( name );
      free( file );
      free( description );
      free( gfx_store );
   }

   /* Clean up. */
   PHYSFS_freeList( map_files );

   return 0;
}

/**
 * @brief Frees the loaded map.
 */
static void mapsList_free( void )
{
   unsigned int n = array_size( mapList );
   for ( unsigned int i = 0; i < n; i++ ) {
      free( mapList[i].fileName );
      free( mapList[i].mapName );
      free( mapList[i].description );
      free( mapList[i].gfx_store );
   }
   array_free( mapList );
   mapList = NULL;

   free( mapedit_sLoadMapName );
   mapedit_sLoadMapName = NULL;
}

/**
 * @brief Saves selected systems as a map outfit file.
 *
 *    @return 0 on success.
 */
static int mapedit_saveMap( StarSystem **uniedit_sys, mapOutfitsList_t *ns )
{
   xmlDocPtr        doc;
   xmlTextWriterPtr writer;
   char            *file;

   /* Create the writer. */
   writer = xmlNewTextWriterDoc( &doc, 0 );
   if ( writer == NULL ) {
      WARN( _( "testXmlwriterDoc: Error creating the xml writer" ) );
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start writer. */
   xmlw_start( writer );
   xmlw_startElem( writer, "outfit" );

   /* Attributes. */
   xmlw_attr( writer, "name", "%s", ns->mapName );

   /* General. */
   xmlw_startElem( writer, "general" );
   xmlw_elem( writer, "rarity", "%d", ns->rarity );
   xmlw_elem( writer, "mass", "%d", 0 );
   xmlw_elem( writer, "price", "%" CREDITS_PRI, ns->price );
   xmlw_elem( writer, "description", "%s", ns->description );
   xmlw_elem( writer, "gfx_store", "%s", ns->gfx_store );
   xmlw_endElem( writer ); /* "general" */

   xmlw_startElem( writer, "specific" );
   xmlw_attr( writer, "type", "map" );

   /* Iterate over all selected systems. Save said systems and any NORMAL jumps
    * they might share. */
   for ( int i = 0; i < ns->numSystems; i++ ) {
      StarSystem *s = uniedit_sys[i];
      xmlw_startElem( writer, "sys" );
      xmlw_attr( writer, "name", "%s", s->name );

      /* Iterate jumps and see if they lead to any other systems in our array.
       */
      for ( int j = 0; j < array_size( s->jumps ); j++ ) {
         /* Ignore hidden and exit-only jumps. */
         if ( jp_isFlag( &s->jumps[j], JP_EXITONLY ) )
            continue;
         if ( !mapedit_hidden && jp_isFlag( &s->jumps[j], JP_HIDDEN ) )
            continue;
         /* This is a normal jump. */
         for ( int k = 0; k < ns->numSystems; k++ ) {
            if ( s->jumps[j].target == uniedit_sys[k] ) {
               xmlw_elem( writer, "jump", "%s", uniedit_sys[k]->name );
               break;
            }
         }
      }

      /* Iterate spobs and add them */
      if ( !mapedit_nospob )
         for ( int j = 0; j < array_size( s->spobs ); j++ )
            xmlw_elem( writer, "spob", "%s", s->spobs[j]->name );

      xmlw_endElem( writer ); /* "sys" */
   }

   xmlw_endElem( writer ); /* "specific" */
   xmlw_endElem( writer ); /* "outfit" */
   xmlw_done( writer );

   /* No need for writer anymore. */
   xmlFreeTextWriter( writer );

   /* Actually write data */
   SDL_asprintf( &file, "%s/%s", conf.dev_save_map, ns->fileName );
   if ( xmlSaveFileEnc( file, doc, "UTF-8" ) < 0 )
      WARN( _( "Failed writing '%s'!" ), file );
   free( file );

   /* Clean up. */
   xmlFreeDoc( doc );

   return 0;
}
