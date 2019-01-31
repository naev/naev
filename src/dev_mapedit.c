/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_mapedit.c
 *
 * @brief Handles the star system editor.
 */

#include "dev_mapedit.h"

#include "naev.h"

#include "SDL.h"

#include "space.h"
#include "toolkit.h"
#include "opengl.h"
#include "map.h"
#include "dev_system.h"
#include "dev_planet.h"
#include "unidiff.h"
#include "dialogue.h"
#include "tk/toolkit_priv.h"
#include "dev_sysedit.h"
#include "pause.h"
#include "nfile.h"
#include "nstring.h"

/* Extra includes for loading the data from the map outfits */
#include "load.h"
#include "ndata.h"
#include "array.h"
#include "mapData.h"
#include "outfit.h"


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define MAPEDIT_EDIT_WIDTH       400 /**< System editor width. */
#define MAPEDIT_EDIT_HEIGHT      450 /**< System editor height. */


#define MAPEDIT_DRAG_THRESHOLD   300   /**< Drag threshold. */
#define MAPEDIT_MOVE_THRESHOLD   10    /**< Movement threshold. */

#define MAPEDIT_ZOOM_STEP        1.2   /**< Factor to zoom by for each zoom level. */
#define MAPEDIT_ZOOM_MAX         5     /**< Maximum mapedit zoom level (close). */
#define MAPEDIT_ZOOM_MIN         -5    /**< Minimum mapedit zoom level (far). */

#define MAPEDIT_OPEN_WIDTH       800   /**< Open window width. */
#define MAPEDIT_OPEN_HEIGHT      500   /**< Open window height. */
#define MAPEDIT_OPEN_TXT_WIDTH   300   /**< Text width. */

#define MAPEDIT_SAVE_WIDTH       800   /**< Open window width. */
#define MAPEDIT_SAVE_HEIGHT      500   /**< Open window height. */
#define MAPEDIT_SAVE_TXT_WIDTH   300   /**< Text width. */

typedef struct mapOutfitsList_s {
   char sFileName[MAPEDIT_FILENAME_MAX];
   char sMapName[MAPEDIT_NAME_MAX];
   char sDescription[MAPEDIT_DESCRIPTION_MAX];
   int iNumSystems;
} mapOutfitsList_t;

extern int systems_nstack;

static mapOutfitsList_t *mapList = NULL; /* Array of map outfits for displaying in the Open window. */

static unsigned int mapedit_wid = 0; /**< Sysedit wid. */
static double mapedit_xpos    = 0.; /**< Viewport X position. */
static double mapedit_ypos    = 0.; /**< Viewport Y position. */
static double mapedit_zoom    = 1.; /**< Viewport zoom level. */
static int mapedit_moved      = 0;  /**< Space moved since mouse down. */
static unsigned int mapedit_dragTime = 0; /**< Tick last started to drag. */
static int mapedit_drag       = 0;  /**< Dragging viewport around. */
static StarSystem **mapedit_sys = NULL; /**< Selected systems. */
static int mapedit_iLastClickedSystem = 0; /**< Last clicked system (use with system_getIndex). */
static int mapedit_tadd       = 0;  /**< Temporarily clicked system should be added. */
static int mapedit_nsys       = 0;  /**< Number of selected systems. */
static int mapedit_msys       = 0;  /**< Memory allocated for selected systems. */
static double mapedit_mx      = 0.; /**< X mouse position. */
static double mapedit_my      = 0.; /**< Y mouse position. */
static unsigned int mapedit_widLoad = 0; /**< Load Map Outfit wid. */
static unsigned int mapedit_widSave = 0; /**< Save Map Outfit wid. */
static int  mapedit_nLoadSys   = 0;                            /**< Last loaded file number of systems. */
static char mapedit_sLoadFileName[MAPEDIT_FILENAME_MAX];       /**< Last loaded map filename. */
static char mapedit_sLoadMapName[MAPEDIT_NAME_MAX];            /**< Last loaded map filename. */
static char mapedit_sLoadDescription[MAPEDIT_DESCRIPTION_MAX]; /**< Last loaded map filename. */


/*
 * Universe editor Prototypes.
 */
/* Selection. */
static void mapedit_deselect (void);
static void mapedit_selectAdd( StarSystem *sys );
static void mapedit_selectRm( StarSystem *sys );
/* Custom system editor widget. */
static void mapedit_buttonZoom( unsigned int wid, char* str );
static void mapedit_render( double bx, double by, double w, double h, void *data );
/*static void mapedit_renderOverlay( double bx, double by, double bw, double bh, void* data );*/
static int mapedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Button functions. */
static void mapedit_close( unsigned int wid, char *wgt );
static void mapedit_btnOpen( unsigned int wid_unused, char *unused );
static void mapedit_btnSaveMapAs( unsigned int wid_unused, char *unused );
static void mapedit_clear( unsigned int wid_unused, char *unused );
/* Keybindings handling. */
static int mapedit_keys( unsigned int wid, SDLKey key, SDLMod mod );
/* Loading of Map files. */
static void mapedit_loadMapMenu_open (void);
static void mapedit_loadMapMenu_close( unsigned int wdw, char *str );
static void mapedit_loadMapMenu_update( unsigned int wdw, char *str );
static void mapedit_loadMapMenu_load( unsigned int wdw, char *str );
/* Saving of Map files. */
void mapedit_saveMapMenu_open (void);
static void mapedit_saveMapMenu_save( unsigned int wdw, char *str );
static void mapedit_saveMapMenu_update( unsigned int wdw, char *str );
/* Management of last loaded/saved Map file. */
void mapedit_setGlobalLoadedInfos (int nSys, char * sFileName, char * sMapName, char * sDescription);
/* Management of Map files list. */
static int  mapedit_mapsList_refresh (void);
static mapOutfitsList_t *mapedit_mapsList_getList ( int *n );


/**
 * @brief Opens the system editor interface.
 */
void mapedit_open( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   unsigned int wid;
   int buttonHPos = 0;
   int buttonVPos = 1;
   int textPos = 0;
   int linesPos = 0;
   int curLines = 0;

   /* Pause. */
   pause_game();

   /* Needed to generate faction disk. */
   map_setZoom( 1. );

   /* Must have no diffs applied. */
   diff_clear();

   /* Reset some variables. */
   mapedit_drag   = 0;
   mapedit_tadd   = 0;
   mapedit_zoom   = 1.;
   mapedit_xpos   = 0.;
   mapedit_ypos   = 0.;

   /* Create the window. */
   wid = window_create( "Map Outfit Editor", -1, -1, -1, -1 );
   window_handleKeys( wid, mapedit_keys );
   mapedit_wid = wid;

   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 350, SCREEN_H - 100,
         "cstSysEdit", 1, mapedit_render, mapedit_mouse, NULL );
   /*window_custSetOverlay( wid, "cstSysEdit", mapedit_renderOverlay );*/

   /* Button : reset the current map. */
   buttonHPos = 2;
   window_addButtonKey( wid, -20-(BUTTON_WIDTH+20)*buttonHPos, 20+(BUTTON_HEIGHT+20)*buttonVPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClear", "Reset", mapedit_clear, SDLK_r );
   buttonHPos--;

   /* Button : open map file. */
   window_addButtonKey( wid, -20-(BUTTON_WIDTH+20)*buttonHPos, 20+(BUTTON_HEIGHT+20)*buttonVPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOpen", "Open", mapedit_btnOpen, SDLK_o );
   buttonHPos--;

   /* Button : save current map to file. */
   window_addButtonKey( wid, -20-(BUTTON_WIDTH+20)*buttonHPos, 20+(BUTTON_HEIGHT+20)*buttonVPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSaveAs", "Save", mapedit_btnSaveMapAs, SDLK_s );
   buttonHPos = 0;
   buttonVPos--;

   /* Button : exit editor. */
   window_addButtonKey( wid, -20-(BUTTON_WIDTH+20)*buttonHPos, 20+(BUTTON_HEIGHT+20)*buttonVPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Exit", mapedit_close, SDLK_x );

   /* Main title. */
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290, 20, 0, "txtSLastLoaded",
         NULL, &cDConsole, "Last loaded Map" );
   textPos++;
   linesPos++;

   /* Filename. */
   curLines = 1;
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, gl_smallFont.h+5, 0, "txtSFileName",
         &gl_smallFont, &cDConsole, "File Name:" );
   window_addText( wid, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtFileName",
         &gl_smallFont, &cBlack, "N/A" );
   textPos++;
   linesPos+=curLines+1;

   /* Map name. */
   curLines = 3;
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, gl_smallFont.h+5, 0, "txtSMapName",
         &gl_smallFont, &cDConsole, "Map Name:" );
   window_addText( wid, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtMapName",
         &gl_smallFont, &cBlack, "N/A" );
   textPos++;
   linesPos+=curLines+1;

   /* Map description. */
   curLines = 5;
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, gl_smallFont.h+5, 0, "txtSDescription",
         &gl_smallFont, &cDConsole, "Description:" );
   window_addText( wid, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtDescription",
         &gl_smallFont, &cBlack, "N/A" );
   textPos++;
   linesPos+=(curLines+1);

   /* Loaded Map # of systems. */
   curLines = 1;
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, 20, 0, "txtSLoadedNumSystems",
         &gl_smallFont, &cDConsole, "Number of Systems (limited to 100):" );
   window_addText( wid, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtLoadedNumSystems",
         &gl_smallFont, &cBlack, "N/A" );
   textPos++;
   linesPos+=curLines+1;

   /* Main title */
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290, 20, 0, "txtSCurentMap",
         NULL, &cDConsole, "Current Map" );
   textPos++;
   linesPos++;

   /* Current Map # of systems. */
   curLines = 1;
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, 20, 0, "txtSCurrentNumSystems",
         &gl_smallFont, &cDConsole, "Number of Systems (limited to 100):" );
   window_addText( wid, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtCurrentNumSystems",
         &gl_smallFont, &cBlack, "N/A" );
   textPos++;
   linesPos+=curLines+1;

   /* Presence. */
   curLines = 5;
   window_addText( wid, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, 20, 0, "txtSPresence",
         &gl_smallFont, &cDConsole, "Presence:" );
   window_addText( wid, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtPresence",
         &gl_smallFont, &cBlack, "No selection" );
   textPos++;
   linesPos+=curLines+1;

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", mapedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", mapedit_buttonZoom );

   /* Selected text. */
   window_addText( wid, 140, 10, SCREEN_W - 350 - 30 - 30 - BUTTON_WIDTH - 20, 30, 0,
         "txtSelected", &gl_smallFont, &cBlack, NULL );

   /* Deselect everything. */
   mapedit_deselect();
}


/**
 * @brief Handles keybindings.
 */
static int mapedit_keys( unsigned int wid, SDLKey key, SDLMod mod )
{
   (void) mod;

   switch (key) {
      /* Mode changes. */
      case SDLK_ESCAPE:
         mapedit_close(wid, "Close");
         return 1;

      default:
         return 0;
   }
}


/**
 * @brief Closes the system editor widget.
 */
static void mapedit_close( unsigned int wid, char *wgt )
{
   /* Frees some memory. */
   mapedit_deselect();
   if (mapList == NULL) {
      //WARN("mapList does not exist.");
   } else {
      //WARN("mapList exists.");
      array_free(mapList);
   }

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
static void mapedit_clear( unsigned int wid, char *wgt )
{
   (void) wid;
   (void) wgt;

   /* Clear the map. */
   mapedit_deselect();
}

/**
 * @brief Opens up a map file.
 */
static void mapedit_btnOpen( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   
   mapedit_loadMapMenu_open();

}


/**
 * @brief Saves current map to file.
 */
static void mapedit_btnSaveMapAs( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   
   mapedit_saveMapMenu_open();

}


/**
 * @brief System editor custom widget rendering.
 */
static void mapedit_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;
   StarSystem *sys;
   int i;

   /* Parameters. */
   map_renderParams( bx, by, mapedit_xpos, mapedit_ypos, w, h, mapedit_zoom, &x, &y, &r );

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render faction disks. */
   map_renderFactionDisks( x, y, 1 );

   /* Render jump paths. */
   map_renderJumps( x, y, 1 );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, w, h, r, 1 );

   /* Render system names. */
   map_renderNames( bx, by, x, y, w, h, 1 );

   /* Render the selected system selections. */
   for (i=0; i<mapedit_nsys; i++) {
      sys = mapedit_sys[i];
      gl_drawCircleInRect( x + sys->pos.x * mapedit_zoom, y + sys->pos.y * mapedit_zoom,
            1.8*r, bx, by, w, h, &cRed, 0 );
      gl_drawCircleInRect( x + sys->pos.x * mapedit_zoom, y + sys->pos.y * mapedit_zoom,
            2.0*r, bx, by, w, h, &cRed, 0 );
   }
   
   /* Render last clicked system */
   if (mapedit_iLastClickedSystem != 0) {
      sys = system_getIndex( mapedit_iLastClickedSystem );
      gl_drawCircleInRect( x + sys->pos.x * mapedit_zoom, y + sys->pos.y * mapedit_zoom,
            2.4*r, bx, by, w, h, &cBlue, 0 );
      gl_drawCircleInRect( x + sys->pos.x * mapedit_zoom, y + sys->pos.y * mapedit_zoom,
            2.6*r, bx, by, w, h, &cBlue, 0 );
      gl_drawCircleInRect( x + sys->pos.x * mapedit_zoom, y + sys->pos.y * mapedit_zoom,
            2.8*r, bx, by, w, h, &cBlue, 0 );
   }
}


/**
 * @brief Renders the overlay.
 */
/*static void mapedit_renderOverlay( double bx, double by, double bw, double bh, void* data )
{
   double x, y;
   (void) bw;
   (void) bh;
   (void) data;

   x = bx + mapedit_mx;
   y = by + mapedit_my;

   toolkit_drawAltText( x, y, "Click on a system toggles it from the map");
}*/

/**
 * @brief System editor custom widget mouse handling.
 */
static int mapedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i;
   int found;
   double x,y, t;
   StarSystem *sys;
   /*SDLMod mod;*/

   /* Debug log */
   /*WARN("Entering function.");*/

   t = 15.*15.; /* threshold */

   /* Handle modifiers. */
   /*mod = SDL_GetModState();*/

   switch (event->type) {

      case SDL_MOUSEBUTTONDOWN:
         /* Debug log */
         /*WARN("\tCase SDL_MOUSEBUTTONDOWN");*/

         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;

         /* Zooming */
         if (event->button.button == SDL_BUTTON_WHEELUP) {
            mapedit_buttonZoom( 0, "btnZoomIn" );
            return 1;
         }
         else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
            mapedit_buttonZoom( 0, "btnZoomOut" );
            return 1;
         }

         /* selecting star system */
         else {
            mx -= w/2 - mapedit_xpos;
            my -= h/2 - mapedit_ypos;

            for (i=0; i<systems_nstack; i++) {
               sys = system_getIndex( i );

               /* get position */
               x = sys->pos.x * mapedit_zoom;
               y = sys->pos.y * mapedit_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* Debug log */
                  /*WARN("\tMouse on a system");*/

                  /* Set last clicked system */
                  mapedit_iLastClickedSystem = i;

                  /* Try to find in selected systems. */
                  found = 0;
                  for (i=0; i<mapedit_nsys; i++) {
                     /* Must match. */
                     if (mapedit_sys[i] == sys) {
                        found = 1;
                        break;
                     } else {
                        continue;
                     }
                  }

                  /* Toggle system selection. */
                  if (found) {
                     mapedit_selectRm( sys );
                  } else {
                     mapedit_selectAdd( sys );
                     /* Debug log */
                     /*WARN("\tAfter mapedit_selectAdd");*/
                  }
                  /*WARN("\tAbort case SDL_MOUSEBUTTONDOWN");*/
                  /*WARN("Exiting function with return 1.");*/
                  return 1;
               }
            }

            /* Start dragging the viewport. */
            mapedit_drag      = 1;
            mapedit_dragTime  = SDL_GetTicks();
            mapedit_moved     = 0;

            /* Debug log */
            /*WARN("\tEnd case SDL_MOUSEBUTTONDOWN");*/
         }
         break;

      case SDL_MOUSEBUTTONUP:
         /* Handles dragging viewport around. */
         if (mapedit_drag) {
            mapedit_drag      = 0;
         }
         break;

      case SDL_MOUSEMOTION:
         /* Update mouse positions. */
         mapedit_mx  = mx;
         mapedit_my  = my;

         /* Handle dragging. */
         if (mapedit_drag) {
            /* axis is inverted */
            mapedit_xpos -= event->motion.xrel;
            mapedit_ypos += event->motion.yrel;

            /* Update mouse movement. */
            mapedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
         }
         break;
   }

   /* Debug log */
   /*WARN("Exiting function.");*/

   return 0;
}


char *mapedit_nameFilter( char *name )
{
   int i, pos;
   char *out;

   out = calloc( 1, (strlen(name)+1)  );
   pos = 0;
   for (i=0; i<(int)strlen(name); i++) {
      if (!ispunct(name[i])) {
         if (name[i] == ' ')
            out[pos] = '_';
         else
            out[pos] = tolower(name[i]);
         pos++;
      }
   }

   return out;
}


/**
 * @brief Deselects selected targets.
 */
static void mapedit_deselect (void)
{
   if (mapedit_nsys > 0)
      free( mapedit_sys );
   mapedit_sys    = NULL;
   mapedit_nsys   = 0;
   mapedit_msys   = 0;
   
   /* Change window stuff. */
   window_modifyText( mapedit_wid, "txtSelected", "No selection" );
   window_modifyText( mapedit_wid, "txtCurrentNumSystems", "0" );
}


/**
 * @brief Adds a system to the selection.
 */
static void mapedit_selectAdd( StarSystem *sys )
{
   /* Debug log */
   /*WARN("Entering function.");*/

   /*WARN("\tSystems before       : %i", mapedit_nsys);*/
   /*WARN("\tmapedit_msys before  : %i", mapedit_msys);*/
   /*WARN("\t\tMemory allocated   : %i", sizeof(StarSystem*) * mapedit_msys);*/
   
   /* Workaround for BUG found in memory allocation */
   if (mapedit_nsys == 100) {
      /*WARN("\tLimit set to preventing memory allocation BUG : only up to 100 systems in a map");*/
      /*WARN("Exiting function.");*/
      return;
   }

   /* Allocate if needed. */
   if (mapedit_msys < mapedit_nsys+1) {
      if (mapedit_msys == 0)
         mapedit_msys = 1;
      mapedit_msys  *= 2;
      /*WARN("\tmapedit_msys++ : %i", mapedit_msys);*/
      /*WARN("\tBefore realloc");*/
      mapedit_sys    = realloc( mapedit_sys, sizeof(StarSystem*) * mapedit_msys );
      /*WARN("\tAfter realloc");*/
   }

   /* Add system. */
   /*WARN("\tBefore setting mapedit_sys[ mapedit_nsys ]");*/
   mapedit_sys[ mapedit_nsys ] = sys;
   /*WARN("\tAfter setting mapedit_sys[ mapedit_nsys ]");*/
   mapedit_nsys++;

   /* Set text again. */
   /*WARN("\tBefore calling mapedit_selectText");*/
   mapedit_selectText();
   /*WARN("\tAfter calling mapedit_selectText");*/

   /*WARN("\tSystems after        : %i", mapedit_nsys);*/
   /*WARN("\tmapedit_msys after   : %i", mapedit_msys);*/

   /* Debug log */
   /*WARN("Exiting function.");*/
}


/**
 * @brief Removes a system from the selection.
 */
static void mapedit_selectRm( StarSystem *sys )
{
   int i;
   for (i=0; i<mapedit_nsys; i++) {
      if (mapedit_sys[i] == sys) {
         mapedit_nsys--;
         memmove( &mapedit_sys[i], &mapedit_sys[i+1], sizeof(StarSystem*) * (mapedit_nsys - i) );
         mapedit_selectText();
         return;
      }
   }
   //WARN("Trying to remove system '%s' from selection when not selected.", sys->name);
}


/**
 * @brief Sets the selected system text.
 */
void mapedit_selectText (void)
{
   int i, l;
   char buf[1024];
   StarSystem *sys;
   int hasPresence;

   /* Built list of all selected systems names */
   l = 0;
   for (i=0; i<mapedit_nsys; i++) {
      l += nsnprintf( &buf[l], sizeof(buf)-l, "%s%s", mapedit_sys[i]->name,
            (i == mapedit_nsys-1) ? "" : ", " );
   }

   if (l == 0)
      /* Change display to reflect that no system is selected */
      mapedit_deselect();
   else {
      /* Display list of selected systems */
      window_modifyText( mapedit_wid, "txtSelected", buf );

      /* Display number of selected systems */
      buf[0]      = '\0';
      nsnprintf( &buf[0], 4, "%i", mapedit_nsys);
      window_modifyText( mapedit_wid, "txtCurrentNumSystems", buf );

      /* Compute and display presence text. */
      if (mapedit_iLastClickedSystem != 0) {
         sys = system_getIndex( mapedit_iLastClickedSystem );
         buf[0]      = '\0';
         hasPresence = 0;
         l           = 0;

         for (i=sys->npresence-1; i >= 0 ; i--) {

            /* Must have presence. */
            if (sys->presence[i].value <= 0)
               continue;

            hasPresence = 1;
            /* Use map grey instead of default neutral colour */
            l += nsnprintf( &buf[l], sizeof(buf)-l, "%s\e0%s: %.0f",
                  (l==0)?"":"\n", faction_name(sys->presence[i].faction),
                  sys->presence[i].value);
         }
         if (hasPresence == 0) {
            nsnprintf( buf, sizeof(buf), "None" );
         }

         window_modifyText( mapedit_wid, "txtPresence", buf );
         buf[0]      = '\0';
         nsnprintf( &buf[0], sizeof(buf), "Presence (%s)", sys->name );
         window_modifyText( mapedit_wid, "txtSPresence", buf );
      } else {
         window_modifyText( mapedit_wid, "txtSPresence", "Presence" );
         window_modifyText( mapedit_wid, "txtPresence", "No system yet clicked" );
      }
   }
}


/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void mapedit_buttonZoom( unsigned int wid, char* str )
{
   (void) wid;

   /* Transform coords to normal. */
   mapedit_xpos /= mapedit_zoom;
   mapedit_ypos /= mapedit_zoom;

   /* Apply zoom. */
   if (strcmp(str,"btnZoomIn")==0) {
      mapedit_zoom *= MAPEDIT_ZOOM_STEP;
      mapedit_zoom = MIN(pow(MAPEDIT_ZOOM_STEP, MAPEDIT_ZOOM_MAX), mapedit_zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      mapedit_zoom /= MAPEDIT_ZOOM_STEP;
      mapedit_zoom = MAX(pow(MAPEDIT_ZOOM_STEP, MAPEDIT_ZOOM_MIN), mapedit_zoom);
   }

   /* Hack for the circles to work. */
   map_setZoom(mapedit_zoom);

   /* Transform coords back. */
   mapedit_xpos *= mapedit_zoom;
   mapedit_ypos *= mapedit_zoom;
}


/**
 * @brief Opens the save map outfit menu.
 */
void mapedit_saveMapMenu_open (void)
{
   unsigned int wid;
   char **names;
   mapOutfitsList_t *nslist, *ns;
   int i, n, iCurrent;
   /*int len;*/
   int textPos = 0;
   int linesPos = 0;
   int curLines = 0;
   
   /* Debug log */
   //WARN("Entering function.");

   /* window */
   wid = window_create( "Save to Map Outfit", -1, -1, MAPEDIT_SAVE_WIDTH, MAPEDIT_SAVE_HEIGHT );
   mapedit_widSave = wid;

   /* Default actions */
   window_setAccept( mapedit_widSave, mapedit_saveMapMenu_save );
   window_setCancel( mapedit_widSave, mapedit_loadMapMenu_close );

   /* Load list of map outfits */
   mapedit_mapsList_refresh();

   /* load the maps */
   iCurrent = 0;
   nslist = mapedit_mapsList_getList( &n );
   if (n > 0) {
      names = malloc( sizeof(char*)*n );
      for (i=0; i<n; i++) {
         ns       = &nslist[i];
         /*len      = strlen(ns->sMapName);*/
         names[i] = strdup(ns->sMapName);
          if (!strcmp(mapedit_sLoadMapName, ns->sMapName)) {
            iCurrent = i;
          }
      }
   } else {
      /* case there are no files */
      names = malloc(sizeof(char*));
      names[0] = strdup("None");
      n     = 1;
   }

   /* Debug Log */
   //WARN("List[0] : %s", names[0]);
   //WARN("WID     : %i", mapedit_widSave);

   /* Filename. */
   curLines = 1;
   window_addText( mapedit_widSave, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, gl_smallFont.h+5, 0, "txtSFileName",
         &gl_smallFont, &cDConsole, "File Name (.xml):" );
   window_addInput ( mapedit_widSave, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, 5+curLines*(gl_smallFont.h+5), "inpFileName", MAPEDIT_FILENAME_MAX, 1, NULL );
   textPos++;
   linesPos+=curLines+1;

   /* Map name. */
   curLines = 3;
   window_addText( mapedit_widSave, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, gl_smallFont.h+5, 0, "txtSMapName",
         &gl_smallFont, &cDConsole, "Map Name:" );
   window_addInput ( mapedit_widSave, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, 5+curLines*(gl_smallFont.h+5), "inpMapName", MAPEDIT_NAME_MAX, 0, NULL );
   textPos++;
   linesPos+=curLines+1;

   /* Map description. */
   curLines = 5;
   window_addText( mapedit_widSave, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, gl_smallFont.h+5, 0, "txtSDescription",
         &gl_smallFont, &cDConsole, "Description:" );
   window_addInput( mapedit_widSave, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, 5+curLines*(gl_smallFont.h+5), "inpDescription", MAPEDIT_DESCRIPTION_MAX, 0, NULL );
   textPos++;
   linesPos+=(curLines+1);

   /* Loaded Map # of systems. */
   curLines = 1;
   window_addText( mapedit_widSave, -20, -40-textPos*20-linesPos*(gl_smallFont.h+5), 290-10, 20, 0, "txtSLoadedNumSystems",
         &gl_smallFont, &cDConsole, "Number of Systems (limited to 100):" );
   window_addText( mapedit_widSave, -20, -40-textPos*20-(linesPos+1)*(gl_smallFont.h+5), 290-20, curLines*(gl_smallFont.h+5), 0, "txtLoadedNumSystems",
         &gl_smallFont, &cBlack, "N/A" );
   textPos++;
   linesPos+=curLines+1;

   /* Debug log */
   //WARN("Creating list.");

   /* Create list : must be created after inpFileName, inpMapName, inpDescription and txtLoadedNumSystems */
   window_addList( mapedit_widSave, 20, -50,
         MAPEDIT_SAVE_WIDTH-MAPEDIT_SAVE_TXT_WIDTH-60, MAPEDIT_SAVE_HEIGHT-40-40,
         "lstMapOutfits", names, n, iCurrent, mapedit_saveMapMenu_update );

   /* Debug log */
   //WARN("Creating buttons.");

   /* Buttons */
   window_addButtonKey( mapedit_widSave, -20, 20 + BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSave", "Save", mapedit_saveMapMenu_save, SDLK_s );
   window_addButtonKey( mapedit_widSave, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", "Exit", mapedit_loadMapMenu_close, SDLK_x );

   /* Update all widgets */
   mapedit_saveMapMenu_update( mapedit_widSave, "" );

   /* Debug log */
   //WARN("Exiting function.");
}


/**
 * @brief Opens the load map outfit menu.
 */
void mapedit_loadMapMenu_open (void)
{
   unsigned int wid;
   char **names;
   mapOutfitsList_t *nslist, *ns;
   int i, n;
   /*int len;*/
   
   /* Debug log */
   //WARN("Entering function.");

   /* window */
   wid = window_create( "Open Map Outfit", -1, -1, MAPEDIT_OPEN_WIDTH, MAPEDIT_OPEN_HEIGHT );
   mapedit_widLoad = wid;

   /* Default actions */
   window_setAccept( mapedit_widLoad, mapedit_loadMapMenu_load );
   window_setCancel( mapedit_widLoad, mapedit_loadMapMenu_close );

   /* Load list of map outfits */
   mapedit_mapsList_refresh();

   /* load the maps */
   nslist = mapedit_mapsList_getList( &n );
   if (n > 0) {
      names = malloc( sizeof(char*)*n );
      for (i=0; i<n; i++) {
         ns       = &nslist[i];
         /*len      = strlen(ns->sMapName);*/
         names[i] = strdup(ns->sMapName);
      }
   }
   /* case there are no files */
   else {
      names = malloc(sizeof(char*));
      names[0] = strdup("None");
      n     = 1;
   }

   /* Debug Log */
   //WARN("List[0] : %s", names[0]);
   //WARN("WID     : %i", mapedit_widLoad);

   /* Debug log */
   //WARN("Creating list.");

   /* Map info text. */
   window_addText( mapedit_widLoad, -20, -40, MAPEDIT_OPEN_TXT_WIDTH, MAPEDIT_OPEN_HEIGHT-40-20-2*(BUTTON_HEIGHT+20),
         0, "txtMapInfo", NULL, &cBlack, NULL );

   window_addList( mapedit_widLoad, 20, -50,
         MAPEDIT_OPEN_WIDTH-MAPEDIT_OPEN_TXT_WIDTH-60, MAPEDIT_OPEN_HEIGHT-110,
         "lstMapOutfits", names, n, 0, mapedit_loadMapMenu_update );

   /* Debug log */
   //WARN("Creating buttons.");

   /* Buttons */
   window_addButtonKey( mapedit_widLoad, -20, 20 + BUTTON_HEIGHT+20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", "Load", mapedit_loadMapMenu_load, SDLK_l );
   window_addButton( mapedit_widLoad, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnBack", "Back", mapedit_loadMapMenu_close );
   window_addButton( mapedit_widLoad, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnDelete", "Del", mapedit_loadMapMenu_close );

   /* Debug log */
   //WARN("Exiting function.");
}


/**
 * @brief Updates the load menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void mapedit_loadMapMenu_update( unsigned int wdw, char *str )
{
   (void) str;
   int pos;
   int n;
   mapOutfitsList_t *ns;
   char *save;
   char buf[256];

    /* Debug log */
   //WARN("Entering function.");

   /* Make sure list is ok. */
   save = toolkit_getList( wdw, "lstMapOutfits" );
   if (strcmp(save,"None") == 0) {
      //WARN("No list.");
      return;
   }

   /* Get position. */
   //WARN("Getting current position in list.");
   pos = toolkit_getListPos( wdw, "lstMapOutfits" );
   ns  = mapedit_mapsList_getList( &n );
   ns  = &ns[pos];
   //WARN("\tPosition is : %i.", pos);

   /* Display text. */
   //WARN("Formatting text to display :");
   //WARN("\tFile Name   : \"%s\"", ns->sFileName);
   //WARN("\tMap name    : \"%s\"", ns->sMapName);
   //WARN("\tSystems     : %i", ns->iNumSystems);
   nsnprintf( buf, sizeof(buf),
         "File Name:\n"
         "   %s\n"
         "Map name:\n"
         "   %s\n"
         "Description:\n"
         "   %s\n"
         "Systems:\n"
         "   %i",
         ns->sFileName, ns->sMapName, ns->sDescription, ns->iNumSystems
   );

   /*WARN("Setting text into widget.");*/
   window_modifyText( wdw, "txtMapInfo", buf );

    /* Debug log */
   //WARN("Exiting function.");
}


/**
 * @brief Closes the load map outfit menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void mapedit_loadMapMenu_close( unsigned int wdw, char *str )
{
   (void)str;

   /* Debug log */
   /*WARN("Entering function.");*/

   window_destroy( wdw );

   /* Debug log */
   /*WARN("Exiting function.");*/
}


/**
 * @brief Load the selected Map.
 */
static void mapedit_loadMapMenu_load( unsigned int wdw, char *str )
{
   (void)str;
   int pos, n, len, compareLimit, i, found;
   size_t bufsize;
   mapOutfitsList_t *ns;
   char *save, *buf, *file, *name, *systemName;
   xmlNodePtr node;
   xmlDocPtr doc;
   StarSystem *sys;

   /* Debug log */
   //WARN("Entering function.");

   /* Make sure list is ok. */
   save = toolkit_getList( wdw, "lstMapOutfits" );
   if (strcmp(save,"None") == 0) {
      //WARN("No list.");
      return;
   }

   /* Get position. */
   /*WARN("Getting current position in list.");*/
   pos = toolkit_getListPos( wdw, "lstMapOutfits" );
   ns  = mapedit_mapsList_getList( &n );
   ns  = &ns[pos];
   /*WARN("\tPosition is : %i.", pos);*/

   /* Display text. */
   /*WARN("Getting file to load :");*/
   //WARN("\tFile Name   : \"%s\"", ns->sFileName);
   //WARN("\tMap name    : \"%s\"", ns->sMapName);
   //WARN("\tDescription : \"%s\"", ns->sDescription);
   //WARN("\tSystems     : %i", ns->iNumSystems);

   /*WARN("\tGetting full file path");*/
   len  = strlen(MAP_DATA_PATH)+strlen(ns->sFileName)+2;
   file = malloc( len );
   nsnprintf( file, len, "%s%s", MAP_DATA_PATH, ns->sFileName );
   /*WARN("\tFile Path   : \"%s\"", file);*/

   /*WARN("\tReading file into buffer");*/
   buf = ndata_read( file, &bufsize );
   doc = xmlParseMemory( buf, bufsize );

   /* Get first node, normally "outfit" */
   node = doc->xmlChildrenNode;
   if (node == NULL) {
      //WARN("\t\tMalformed file \"%s\" : does not contain any elements", ns->sFileName);
      free(file);
      xmlFreeDoc(doc);
      free(buf);
	  return;
   } else {
      /*WARN("\t\tFile form OK");*/
   }

   if (!xml_isNode(node,"outfit")) {
      //WARN("\t\tMalformed file \"%s\" : no <outfit> elements", ns->sFileName);
      free(file);
      xmlFreeDoc(doc);
      free(buf);
	  return;
   } else {
      /*WARN("\t\t<outfit> element found");*/
   }

   /* Get "name" property from the "outfit" node */
   name = xml_nodeProp( node,"name" );
   /*WARN("\t\tName = \"%s\"", name);*/
   compareLimit = MAPEDIT_FILENAME_MAX;
   if (!strncmp(ns->sMapName, name, compareLimit)==0) {
      //WARN("\t\tInconsistent names between list and file : list=\"%s\", file=\"%s\"", ns->sMapName, name);
	  return;
   } else {
      /*WARN("\t\tNames found OK between list and file : \"%s\"", name);*/
   }

   /* Loop on the nodes to find <specific> node */
   node = node->xmlChildrenNode;
   /*WARN("\t\tBeginning loop searching for <specific> node");*/
   do {
      xml_onlyNodes(node);

      if (!xml_isNode(node,"specific")) {
         /*WARN("\t\tFile \"%s\" has unknown node \"%s\"", map_files[i], node->name);*/
         continue;
      }
      /*WARN("\t\t\t<specific> element found");*/

      /* Break out of the loop, either with a correct outfitType or not */
      break;
   } while (xml_nextNode(node));
   /*WARN("\t\tEnding loop searching for <specific> node");*/
   
   /*WARN("\t\tUnselecting all previously selected systems");*/
   mapedit_deselect();

   /* Loop on the nodes to find all <sys> node */
   node = node->xmlChildrenNode;
   /*WARN("\t\tBeginning loop on <sys> nodes");*/
   do {
      systemName = "";
      xml_onlyNodes(node);

      if (!xml_isNode(node,"sys")) {
         /*WARN("\t\tFile \"%s\" has unknown node \"%s\"", map_files[i], node->name);*/
         continue;
      }
      /*WARN("\t\t\t<sys> element found");*/

      /* Display "name" property from "sys" node and increment number of systems found */
      systemName = xml_nodeProp( node,"name" );
      /*WARN("\t\t\t\tSystem name = \"%s\"", systemName);*/
	  
	  /* Find system */
	  found  = 0;
      for (i=0; i<systems_nstack; i++) {
         sys = system_getIndex( i );
         compareLimit = strlen(systemName);
         if (strncmp(systemName, sys->name, compareLimit)==0) {
		    found = 1;
			break;
		 }
	  }

	  /* If system exists, select it */
	  if (found) {
         /*WARN("\t\t\t\tSystem \"%s\" found in stack, i=%i", systemName, i);*/
	     mapedit_selectAdd( sys );
	  } else {
         /*WARN("\t\t\t\tSystem \"%s\" not found in stack", systemName);*/
	  }
   } while (xml_nextNode(node));
   /*WARN("\t\tEnding loop on <sys> nodes");*/

   //WARN("\tSetting global display variables");
   //WARN("\tFile Name   : \"%s\"", ns->sFileName);
   //WARN("\tMap name    : \"%s\"", ns->sMapName);
   //WARN("\tDescription : \"%s\"", ns->sDescription);
   //WARN("\tSystems     : %i", ns->iNumSystems);
   mapedit_setGlobalLoadedInfos ( mapedit_nsys, ns->sFileName, ns->sMapName, ns->sDescription);
   
   /*WARN("\tCleaning up memory");*/
   free(file);
   xmlFreeDoc(doc);
   free(buf);

   /*WARN("\tDestroying window");*/
   window_destroy( wdw );

   /* Debug log */
   //WARN("Exiting function.");
}


/**
 * @brief Updates the save menu.
 *    @param wdw Window triggering function.
 *    @param str Unused.
 */
static void mapedit_saveMapMenu_update( unsigned int wdw, char *str )
{
   (void) str;
   int pos, n, len;
   mapOutfitsList_t *ns;
   char *save;
   char buf[25], sFileName[MAPEDIT_FILENAME_MAX];

    /* Debug log */
   //WARN("Entering function.");

   /* Make sure list is ok. */
   save = toolkit_getList( wdw, "lstMapOutfits" );
   if (strcmp(save,"None") == 0) {
      //WARN("No list.");
      return;
   }
   
   /* Get position. */
   //WARN("Getting current position in list.");
   pos = toolkit_getListPos( wdw, "lstMapOutfits" );
   ns  = mapedit_mapsList_getList( &n );
   ns  = &ns[pos];
   //WARN("\tPosition is : %i.", pos);

   /* Getting file nae without extension */
   len = strlen( ns->sFileName );
   nsnprintf( sFileName, len-3, "%s", ns->sFileName );

   /* Display text. */
   //WARN("Formatting text to display :");
   //WARN("\tFile Name   : \"%s\"", sFileName);
   //WARN("\tMap name    : \"%s\"", ns->sMapName);
   //WARN("\tSystems     : %i", ns->iNumSystems);
   nsnprintf( buf, sizeof(buf), "Selected: %i; File: %i", mapedit_nsys, ns->iNumSystems );

   /* Displaying info strings */
   window_setInput  ( wdw, "inpFileName", sFileName );
   window_setInput  ( wdw, "inpMapName", ns->sMapName );
   window_setInput  ( wdw, "inpDescription", ns->sDescription );
   window_modifyText( wdw, "txtLoadedNumSystems", buf );

    /* Debug log */
   //WARN("Exiting function.");
}


/**
 * @brief Save the current Map to selected file.
 */
static void mapedit_saveMapMenu_save( unsigned int wdw, char *str )
{
   (void)str;
   int pos, n, len;
   mapOutfitsList_t *ns;
   char *save;
   char sFileName[MAPEDIT_FILENAME_MAX], sMapName[MAPEDIT_NAME_MAX], sDescription[MAPEDIT_DESCRIPTION_MAX];

   /* Debug log */
   //WARN("Entering function.");
   //WARN("\tWndow ID wid=%i", wdw);

   /* Make sure list is ok. */
   save = toolkit_getList( wdw, "lstMapOutfits" );
   if (strcmp(save,"None") == 0) {
      //WARN("No list.");
      return;
   }

   /* Get position. */
   /*WARN("Getting current position in list.");*/
   pos = toolkit_getListPos( wdw, "lstMapOutfits" );
   ns  = mapedit_mapsList_getList( &n );
   ns  = &ns[pos];
   /*WARN("\tPosition is : %i.", pos);*/

   /* Getting file name without extension from input box */
   save = window_getInput( wdw, "inpFileName" );
   len = strlen( save );
   nsnprintf( sFileName, len+1, "%s", save );

   /* Getting map name from input box */
   save = window_getInput( wdw, "inpMapName" );
   len = strlen( save );
   nsnprintf( sMapName, len+1, "%s", save );

   /* Getting description from input box */
   save = window_getInput( wdw, "inpDescription" );
   len = strlen( save );
   nsnprintf( sDescription, len+1, "%s", save );

   /* Display text. */
   //WARN("Getting file to save :");
   //WARN("\tFile Name   : \"%s\"", sFileName);
   //WARN("\tMap name    : \"%s\"", sMapName);
   //WARN("\tDescription : \"%s\"", sDescription);
   //WARN("\tSystems     : %i", mapedit_nsys);

   //WARN("\tSaving the file");
   dsys_saveMap(mapedit_sys, mapedit_nsys, sFileName, sMapName, sDescription);

   //WARN("\tSetting global display variables");
   //WARN("\tFile Name   : \"%s\"", sFileName);
   //WARN("\tMap name    : \"%s\"", sMapName);
   //WARN("\tDescription : \"%s\"", sDescription);
   //WARN("\tSystems     : %i", mapedit_nsys);
   mapedit_setGlobalLoadedInfos ( mapedit_nsys, sFileName, sMapName, sDescription);

   //WARN("\tCleaning up memory");
   /*free(save);*/

   //WARN("\tDestroying window");
   window_destroy( wdw );

   /* Debug log */
   //WARN("Exiting function.");
}

/**
 * @brief Set and display the global variables describing last loaded/saved file
 */
void mapedit_setGlobalLoadedInfos (int nSys, char * sFileName, char * sMapName, char * sDescription) {
   char buf[8];

   /* Setting number of systems */
   mapedit_nLoadSys   = nSys;

   /* Clearing info strings */
   mapedit_sLoadFileName[0] = '\0';
   mapedit_sLoadMapName[0] = '\0';
   mapedit_sLoadDescription[0] = '\0';

   /* Setting info strings */
   nsnprintf( mapedit_sLoadFileName, MAPEDIT_FILENAME_MAX, "%s", sFileName );
   nsnprintf( mapedit_sLoadMapName, MAPEDIT_NAME_MAX, "%s", sMapName );
   nsnprintf( mapedit_sLoadDescription, MAPEDIT_DESCRIPTION_MAX, "%s", sDescription );
   nsnprintf( buf, 8, "%i", mapedit_nLoadSys );

   /* Displaying info strings */
   window_modifyText( mapedit_wid, "txtFileName", mapedit_sLoadFileName );
   window_modifyText( mapedit_wid, "txtMapName", mapedit_sLoadMapName );
   window_modifyText( mapedit_wid, "txtDescription", mapedit_sLoadDescription );
   window_modifyText( mapedit_wid, "txtLoadedNumSystems", buf );
}


/**
 * @brief Gets the list of all the maps names.
 *   from outfit_mapParse()
 *
 */
static int mapedit_mapsList_refresh (void)
{
   int i, len, compareLimit, nSystems;
   size_t nfiles, bufsize;
   char *buf;
   xmlNodePtr node, cur;
   xmlDocPtr doc;
   char **map_files;
   char *file, *name, *description, *outfitType;
   /*char *systemName;*/
   mapOutfitsList_t *newMapItem;

   /* Debug log */
   //WARN("Entering function.");

   if (mapList == NULL) {
      //WARN("mapList does not exist.");
   } else {
      //WARN("mapList exists.");
      array_free(mapList);
   }
   mapList = array_create( mapOutfitsList_t );

   map_files = ndata_list( MAP_DATA_PATH, &nfiles );
   newMapItem = NULL;
   for (i=0; i<(int)nfiles; i++) {

      //WARN("\tNew file found");
      len  = strlen(MAP_DATA_PATH)+strlen(map_files[i])+2;
      file = malloc( len );
      nsnprintf( file, len, "%s%s", MAP_DATA_PATH, map_files[i] );

      buf = ndata_read( file, &bufsize );
      doc = xmlParseMemory( buf, bufsize );

      /* Get first node, normally "outfit" */
      node = doc->xmlChildrenNode;
      if (node == NULL) {
         //WARN("\t\tMalformed file \"%s\" : does not contain any elements", map_files[i]);
         free(file);
         xmlFreeDoc(doc);
         free(buf);
         return -1;
      } else {
         //WARN("\t\tFile form OK");
      }

      if (!xml_isNode(node,"outfit")) {
         //WARN("\t\tMalformed file \"%s\" : no <outfit> elements", map_files[i]);
         free(file);
         xmlFreeDoc(doc);
         free(buf);
         return -1;
      } else {
         //WARN("\t\t<outfit> element found");
      }

      /* Get "name" property from the "outfit" node */
      name = xml_nodeProp( node,"name" );
      //WARN("\t\tName = \"%s\"", name);

      /* Loop on the nodes to find <specific> node */
      node = node->xmlChildrenNode;
      //WARN("\t\tBeginning loop searching for <specific> node");
      do {
         outfitType = "";
         xml_onlyNodes(node);

         if (!xml_isNode(node,"specific")) {
            if (xml_isNode(node,"general")) {
               description = NULL;
               //WARN("\t\t\t<general> element found");
               //WARN("\t\t\t\tReading subnode \"description\"");
               cur = node->children;
               do {
                  xml_onlyNodes(cur);
                  xmlr_strd(cur,"description",description);
               } while (xml_nextNode(cur));
               //WARN("\t\t\t\tFile \"%s\" has description \"%s\"", map_files[i], description);
            } else {
               //WARN("\t\t\tFile \"%s\" has unknown node \"%s\"", map_files[i], node->name);
			}
            continue;
         }

         //WARN("\t\t\t<specific> element found");

         /* Get the "type" property from "specific" node */
         outfitType = xml_nodeProp( node,"type" );
         //WARN("\t\t\t\tOutfit type = \"%s\"", outfitType);

         /* Break out of the loop, either with a correct outfitType or not */
         break;
      } while (xml_nextNode(node));
      //WARN("\t\tEnding loop searching for <specific> node");

      /* If its not a map, we don't care. */
      compareLimit = 3;
      if (!strncmp(outfitType, "map", compareLimit)==0) {
         //WARN("\t\tFile is not a regular map");
         free(file);
         xmlFreeDoc(doc);
         free(buf);
         continue;
      }

      /* Loop on the nodes to find all <sys> node */
      nSystems = 0;
      node = node->xmlChildrenNode;
      //WARN("\t\tBeginning loop on <sys> nodes");
      do {
         /*systemName = "";*/
         xml_onlyNodes(node);

         if (!xml_isNode(node,"sys")) {
            //WARN("\t\tFile \"%s\" has unknown node \"%s\"", map_files[i], node->name);
            continue;
         }
         /*WARN("\t\t\t<sys> element found");*/

         /* Display "name" property from "sys" node and increment number of systems found */
         nSystems++;
         /*systemName = xml_nodeProp( node,"name" );*/
         /*WARN("\t\t\t\tSystem name = \"%s\"", systemName);*/
      } while (xml_nextNode(node));
      //WARN("\t\tEnding loop on <sys> nodes");
      //WARN("\t\t%i systems found", nSystems);

      /* If the map is a regular one, then load it into the list */
	  if (nSystems > 0) {
         newMapItem = &array_grow( &mapList );
         //WARN("\t\tLoading map into list :");
         newMapItem->iNumSystems   = nSystems;
         len  = strlen(map_files[i])+1;
		 nsnprintf( newMapItem->sFileName, len, "%s", map_files[i] );
         len  = strlen(name)+1;
		 nsnprintf( newMapItem->sMapName, len, "%s", name );
         len  = strlen(description)+1;
		 nsnprintf( newMapItem->sDescription, len, "%s", description );
         //WARN("\t\t\tnewMapItem->iNumSystems  = %i",     newMapItem->iNumSystems);
         //WARN("\t\t\tnewMapItem->sFileName    = \"%s\"", newMapItem->sFileName);
         //WARN("\t\t\tnewMapItem->sMapName     = \"%s\"", newMapItem->sMapName);
         //WARN("\t\t\tnewMapItem->sDescription = \"%s\"", newMapItem->sDescription);
	  } else {
         //WARN("\t\tFile contains no stellar system");
	  }

      /* Clean up. */
      free(name);
      free(file);
      free(buf);
      //WARN("\tNew file end");
  }

   /* Clean up. */
   for (i=0; i<(int)nfiles; i++)
      free( map_files[i] );
   free( map_files );

    /* Debug log */
   /*WARN("Exiting function.");*/

   return 0;
}

/**
 * @brief Gets the list of loaded maps.
 */
mapOutfitsList_t *mapedit_mapsList_getList( int *n )
{
   if (mapList == NULL) {
      *n = 0;
      return NULL;
   }

   *n = array_size( mapList );
   return mapList;
}

