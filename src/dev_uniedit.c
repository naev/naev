/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_uniedit.c
 *
 * @brief Handles the star system editor.
 */

#include "dev_uniedit.h"

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


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define UNIEDIT_EDIT_WIDTH       400 /**< System editor width. */
#define UNIEDIT_EDIT_HEIGHT      450 /**< System editor height. */


#define UNIEDIT_DRAG_THRESHOLD   300   /**< Drag threshold. */
#define UNIEDIT_MOVE_THRESHOLD   10    /**< Movement threshold. */

#define UNIEDIT_ZOOM_STEP        1.2   /**< Factor to zoom by for each zoom level. */
#define UNIEDIT_ZOOM_MAX         5     /**< Maximum uniedit zoom level (close). */
#define UNIEDIT_ZOOM_MIN         -5    /**< Minimum uniedit zoom level (far). */

/*
 * The editor modes.
 */
#define UNIEDIT_DEFAULT    0  /**< Default editor mode. */
#define UNIEDIT_JUMP       1  /**< Jump point toggle mode. */
#define UNIEDIT_NEWSYS     2  /**< New system editor mode. */


extern int systems_nstack;


static int uniedit_mode       = UNIEDIT_DEFAULT; /**< Editor mode. */
static unsigned int uniedit_wid = 0; /**< Sysedit wid. */
static unsigned int uniedit_widEdit = 0; /**< Sysedit editor wid. */
static double uniedit_xpos    = 0.; /**< Viewport X position. */
static double uniedit_ypos    = 0.; /**< Viewport Y position. */
static double uniedit_zoom    = 1.; /**< Viewport zoom level. */
static int uniedit_moved      = 0;  /**< Space moved since mouse down. */
static unsigned int uniedit_dragTime = 0; /**< Tick last started to drag. */
static int uniedit_drag       = 0;  /**< Dragging viewport around. */
static int uniedit_dragSys    = 0;  /**< Dragging system around. */
static StarSystem **uniedit_sys = NULL; /**< Selected systems. */
static StarSystem *uniedit_tsys = NULL; /**< Temporarily clicked system. */
static int uniedit_tadd       = 0;  /**< Temporarily clicked system should be added. */
static int uniedit_nsys       = 0;  /**< Number of selected systems. */
static int uniedit_msys       = 0;  /**< Memory allocated for selected systems. */
static double uniedit_mx      = 0.; /**< X mouse position. */
static double uniedit_my      = 0.; /**< Y mouse position. */


/*
 * Universe editor Prototypes.
 */
/* Selection. */
static void uniedit_deselect (void);
static void uniedit_selectAdd( StarSystem *sys );
static void uniedit_selectRm( StarSystem *sys );
/* System editing. */
static void uniedit_editSys (void);
static void uniedit_editSysClose( unsigned int wid, char *name );
static void uniedit_editGenList( unsigned int wid );
static void uniedit_btnEditRename( unsigned int wid, char *unused );
static void uniedit_btnEditRmAsset( unsigned int wid, char *unused );
static void uniedit_btnEditAddAsset( unsigned int wid, char *unused );
static void uniedit_btnEditAddAssetAdd( unsigned int wid, char *unused );
/* System renaming. */
static int uniedit_checkName( char *name );
static void uniedit_renameSys (void);
/* New system. */
static void uniedit_newSys( double x, double y );
/* Jump handling. */
static void uniedit_toggleJump( StarSystem *sys );
static void uniedit_jumpAdd( StarSystem *sys, StarSystem *targ );
static void uniedit_jumpRm( StarSystem *sys, StarSystem *targ );
/* Custom system editor widget. */
static void uniedit_buttonZoom( unsigned int wid, char* str );
static void uniedit_render( double bx, double by, double w, double h, void *data );
static void uniedit_renderOverlay( double bx, double by, double bw, double bh, void* data );
static int uniedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Button functions. */
static void uniedit_close( unsigned int wid, char *wgt );
static void uniedit_save( unsigned int wid_unused, char *unused );
static void uniedit_saveMap( unsigned int wid_unused, char *unused );
static void uniedit_btnJump( unsigned int wid_unused, char *unused );
static void uniedit_btnRename( unsigned int wid_unused, char *unused );
static void uniedit_btnEdit( unsigned int wid_unused, char *unused );
static void uniedit_btnNew( unsigned int wid_unused, char *unused );
static void uniedit_btnOpen( unsigned int wid_unused, char *unused );
/* Keybindings handling. */
static int uniedit_keys( unsigned int wid, SDLKey key, SDLMod mod );


/**
 * @brief Opens the system editor interface.
 */
void uniedit_open( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   unsigned int wid;
   int buttonPos = 0;

   /* Pause. */
   pause_game();

   /* Needed to generate faction disk. */
   map_setZoom( 1. );

   /* Must have no diffs applied. */
   diff_clear();

   /* Reset some variables. */
   uniedit_mode   = UNIEDIT_DEFAULT;
   uniedit_drag   = 0;
   uniedit_dragSys = 0;
   uniedit_tsys   = NULL;
   uniedit_tadd   = 0;
   uniedit_zoom   = 1.;
   uniedit_xpos   = 0.;
   uniedit_ypos   = 0.;

   /* Create the window. */
   wid = window_create( "Universe Editor", -1, -1, -1, -1 );
   window_handleKeys( wid, uniedit_keys );
   uniedit_wid = wid;

   /* Close button. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", uniedit_close );
   buttonPos++;

   /* Save button. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSave", "Save All", uniedit_save );
   buttonPos++;

   /* Save Map button. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSaveMap", "Save Map", uniedit_saveMap );
   buttonPos++;

   /* Jump toggle. */
   buttonPos++;
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnJump", "Jump", uniedit_btnJump );
   buttonPos++;

   /* Rename system. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRename", "Rename", uniedit_btnRename );
   buttonPos++;

   /* Edit system. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnEdit", "Edit", uniedit_btnEdit );
   buttonPos++;

   /* New system. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", "New Sys", uniedit_btnNew );
   buttonPos++;

   /* Open a system. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*buttonPos, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOpen", "Open", uniedit_btnOpen );
   buttonPos++;

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", uniedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", uniedit_buttonZoom );

   /* Presence. */
   window_addText( wid, -20, -140, 90, 20, 0, "txtSPresence",
         &gl_smallFont, &cDConsole, "Presence:" );
   window_addText( wid, -20, -140-gl_smallFont.h-5, 80, 100, 0, "txtPresence",
         &gl_smallFont, &cBlack, "N/A" );

   /* Selected text. */
   window_addText( wid, 140, 10, SCREEN_W - 80 - 30 - 30 - BUTTON_WIDTH - 20, 30, 0,
         "txtSelected", &gl_smallFont, &cBlack, NULL );

   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 150, SCREEN_H - 100,
         "cstSysEdit", 1, uniedit_render, uniedit_mouse, NULL );
   window_custSetOverlay( wid, "cstSysEdit", uniedit_renderOverlay );

   /* Deselect everything. */
   uniedit_deselect();
}


/**
 * @brief Handles keybindings.
 */
static int uniedit_keys( unsigned int wid, SDLKey key, SDLMod mod )
{
   (void) wid;
   (void) mod;

   switch (key) {
      /* Mode changes. */
      case SDLK_n:
         uniedit_mode = UNIEDIT_NEWSYS;
         return 1;
      case SDLK_j:
         if (uniedit_nsys == 0)
            return 0;
         uniedit_mode = UNIEDIT_JUMP;
         return 1;
      case SDLK_r:
         if (uniedit_nsys == 0)
            return 0;
         uniedit_renameSys();
         return 1;
      case SDLK_e:
         if (uniedit_nsys == 0)
            return 0;
         uniedit_editSys();
         return 1;
      case SDLK_ESCAPE:
         uniedit_mode = UNIEDIT_DEFAULT;
         return 1;

      default:
         return 0;
   }
}


/**
 * @brief Closes the system editor widget.
 */
static void uniedit_close( unsigned int wid, char *wgt )
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
static void uniedit_save( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   dsys_saveAll();
   dpl_saveAll();
}


/*
 * @brief Saves a map.
 */
static void uniedit_saveMap( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   dsys_saveMap(uniedit_sys, uniedit_nsys);
}


/**
 * @brief Enters the editor in new jump mode.
 */
static void uniedit_btnJump( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   uniedit_mode = UNIEDIT_JUMP;
}


/**
 * @brief Renames selected systems.
 */
static void uniedit_btnRename( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   uniedit_renameSys();
}


/**
 * @brief Enters the editor in new system mode.
 */
static void uniedit_btnNew( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   uniedit_mode = UNIEDIT_NEWSYS;
}


/**
 * @brief Opens up a system.
 */
static void uniedit_btnOpen( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   if (uniedit_nsys != 1)
      return;

   sysedit_open( uniedit_sys[0] );
}


/**
 * @brief Opens the system property editor.
 */
static void uniedit_btnEdit( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   uniedit_editSys();
}


/**
 * @brief System editor custom widget rendering.
 */
static void uniedit_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;
   StarSystem *sys;
   int i;

   /* Parameters. */
   map_renderParams( bx, by, uniedit_xpos, uniedit_ypos, w, h, uniedit_zoom, &x, &y, &r );

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, w, h, r, 1 );

   /* Render system names. */
   map_renderNames( x, y, 1 );

   /* Render the selected system selections. */
   for (i=0; i<uniedit_nsys; i++) {
      sys = uniedit_sys[i];
      gl_drawCircleInRect( x + sys->pos.x * uniedit_zoom, y + sys->pos.y * uniedit_zoom,
            1.5*r, bx, by, w, h, &cWhite, 0 );
   }
}


/**
 * @brief Renders the overlay.
 */
static void uniedit_renderOverlay( double bx, double by, double bw, double bh, void* data )
{
   double x, y;
   (void) bw;
   (void) bh;
   (void) data;

   x = bx + uniedit_mx;
   y = by + uniedit_my;

   if (uniedit_mode == UNIEDIT_NEWSYS)
      toolkit_drawAltText( x, y, "Click to add a new system");
   else if (uniedit_mode == UNIEDIT_JUMP)
      toolkit_drawAltText( x, y, "Click to toggle jump route");
}

/**
 * @brief System editor custom widget mouse handling.
 */
static int uniedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i;
   double x,y, t;
   StarSystem *sys;
   SDLMod mod;

   t = 15.*15.; /* threshold */

   /* Handle modifiers. */
   mod = SDL_GetModState();

   switch (event->type) {

      case SDL_MOUSEBUTTONDOWN:
         /* Must be in bounds. */
         if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;

         /* Zooming */
         if (event->button.button == SDL_BUTTON_WHEELUP) {
            uniedit_buttonZoom( 0, "btnZoomIn" );
            return 1;
         }
         else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
            uniedit_buttonZoom( 0, "btnZoomOut" );
            return 1;
         }

         /* selecting star system */
         else {
            mx -= w/2 - uniedit_xpos;
            my -= h/2 - uniedit_ypos;

            if (uniedit_mode == UNIEDIT_NEWSYS) {
               uniedit_newSys( mx, my );
               uniedit_mode = UNIEDIT_DEFAULT;
               return 1;
            }

            for (i=0; i<systems_nstack; i++) {
               sys = system_getIndex( i );

               /* get position */
               x = sys->pos.x * uniedit_zoom;
               y = sys->pos.y * uniedit_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* Try to find in selected systems - begin drag move. */
                  for (i=0; i<uniedit_nsys; i++) {
                     /* Must match. */
                     if (uniedit_sys[i] != sys)
                        continue;

                     /* Detect double click to open system. */
                     if ((SDL_GetTicks() - uniedit_dragTime < UNIEDIT_DRAG_THRESHOLD*2)
                           && (uniedit_moved < UNIEDIT_MOVE_THRESHOLD)) {
                        if (uniedit_nsys == 1) {
                           sysedit_open( uniedit_sys[0] );
                           return 1;
                        }
                     }

                     /* Handle normal click. */
                     if (uniedit_mode == UNIEDIT_DEFAULT) {
                        uniedit_dragSys   = 1;
                        uniedit_tsys      = sys;

                        /* Check modifier. */
                        if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                           uniedit_tadd      = 0;
                        else
                           uniedit_tadd      = -1;
                        uniedit_dragTime  = SDL_GetTicks();
                        uniedit_moved     = 0;
                     }
                     return 1;
                  }

                  if (uniedit_mode == UNIEDIT_DEFAULT) {
                     /* Add the system if not selected. */
                     if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                        uniedit_selectAdd( sys );
                     else {
                        uniedit_deselect();
                        uniedit_selectAdd( sys );
                     }
                     uniedit_tsys      = NULL;

                     /* Start dragging anyway. */
                     uniedit_dragSys   = 1;
                     uniedit_dragTime  = SDL_GetTicks();
                     uniedit_moved     = 0;
                  }
                  else if (uniedit_mode == UNIEDIT_JUMP) {
                     uniedit_toggleJump( sys );
                     uniedit_mode = UNIEDIT_DEFAULT;
                  }
                  return 1;
               }
            }

            /* Start dragging. */
            if ((uniedit_mode == UNIEDIT_DEFAULT) && !(mod & (KMOD_LCTRL | KMOD_RCTRL))) {
               uniedit_drag      = 1;
               uniedit_dragTime  = SDL_GetTicks();
               uniedit_moved     = 0;
               uniedit_tsys      = NULL;
            }
            return 1;
         }
         break;

      case SDL_MOUSEBUTTONUP:
         if (uniedit_drag) {
            if ((SDL_GetTicks() - uniedit_dragTime < UNIEDIT_DRAG_THRESHOLD) && (uniedit_moved < UNIEDIT_MOVE_THRESHOLD)) {
               if (uniedit_tsys == NULL)
                  uniedit_deselect();
               else
                  uniedit_selectAdd( uniedit_tsys );
            }
            uniedit_drag      = 0;
         }
         if (uniedit_dragSys) {
            if ((SDL_GetTicks() - uniedit_dragTime < UNIEDIT_DRAG_THRESHOLD) &&
                  (uniedit_moved < UNIEDIT_MOVE_THRESHOLD) && (uniedit_tsys != NULL)) {
               if (uniedit_tadd == 0)
                  uniedit_selectRm( uniedit_tsys );
               else {
                  uniedit_deselect();
                  uniedit_selectAdd( uniedit_tsys );
               }
            }
            uniedit_dragSys   = 0;
            for (i=0; i<uniedit_nsys; i++) {
               dsys_saveSystem(uniedit_sys[i]);
            }
         }
         break;

      case SDL_MOUSEMOTION:
         /* Update mouse positions. */
         uniedit_mx  = mx;
         uniedit_my  = my;

         /* Handle dragging. */
         if (uniedit_drag) {
            /* axis is inverted */
            uniedit_xpos -= event->motion.xrel;
            uniedit_ypos += event->motion.yrel;

            /* Update mouse movement. */
            uniedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
         }
         else if (uniedit_dragSys && (uniedit_nsys > 0)) {
            if ((uniedit_moved > UNIEDIT_MOVE_THRESHOLD) || (SDL_GetTicks() - uniedit_dragTime > UNIEDIT_DRAG_THRESHOLD)) {
               for (i=0; i<uniedit_nsys; i++) {
                  uniedit_sys[i]->pos.x += ((double)event->motion.xrel) / uniedit_zoom;
                  uniedit_sys[i]->pos.y -= ((double)event->motion.yrel) / uniedit_zoom;
               }
            }

            /* Update mouse movement. */
            uniedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
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
static int uniedit_checkName( char *name )
{
   int i;

   /* Avoid name collisions. */
   for (i=0; i<systems_nstack; i++) {
      if (strcmp(name, system_getIndex(i)->name)==0) {
         dialogue_alert( "The Star System '%s' already exists!", name );
         return 1;
      }
   }
   return 0;
}


char *uniedit_nameFilter( char *name )
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
 * @brief Renames all the currently selected systems.
 */
static void uniedit_renameSys (void)
{
   int i, j;
   char *name, *oldName, *newName;
   StarSystem *sys;

   for (i=0; i<uniedit_nsys; i++) {
      sys = uniedit_sys[i];

      /* Get name. */
      name = dialogue_input( "Rename Star System", 1, 32, "What do you want to rename \er%s\e0?", sys->name );

      /* Keep current name. */
      if (name == NULL)
         continue;

      /* Try again. */
      if (uniedit_checkName( name )) {
         free(name);
         i--;
         continue;
      }

      /* Change the name. */
      oldName = malloc((14+strlen(sys->name)));
      nsnprintf(oldName,14+strlen(sys->name),"dat/ssys/%s.xml", uniedit_nameFilter(sys->name) );
      newName = malloc(14+strlen(name));
      nsnprintf(newName,14+strlen(name),"dat/ssys/%s.xml", uniedit_nameFilter(name) );
      nfile_rename(oldName,newName);
      free(oldName);
      free(newName);
      free(sys->name);
      sys->name = name;
      dsys_saveSystem(sys);

      /* Re-save adjacent systems. */
      for (j=0; j<sys->njumps; j++)
         dsys_saveSystem( sys->jumps[j].target );
   }
}


/**
 * @brief Creates a new system.
 */
static void uniedit_newSys( double x, double y )
{
   char *name;
   StarSystem *sys;

   /* Get name. */
   name = dialogue_inputRaw( "New Star System Creation", 1, 32, "What do you want to name the new system?" );

   /* Abort. */
   if (name == NULL) {
      dialogue_alert( "Star System creation aborted!" );
      return;
   }

   /* Make sure there is no collision. */
   if (uniedit_checkName( name )) {
      free(name);
      uniedit_newSys( x, y );
      return;
   }

   /* Transform coordinates back to normal if zoomed */
   x /= uniedit_zoom;
   y /= uniedit_zoom;

   /* Create the system. */
   sys         = system_new();
   sys->name   = name;
   sys->pos.x  = x;
   sys->pos.y  = y;
   sys->stars  = STARS_DENSITY_DEFAULT;
   sys->radius = RADIUS_DEFAULT;

   /* Select new system. */
   uniedit_deselect();
   uniedit_selectAdd( sys );

   dsys_saveSystem( sys );
}


/**
 * @brief Toggles the jump point for the selected systems.
 */
static void uniedit_toggleJump( StarSystem *sys )
{
   int i, j, rm;
   StarSystem *isys, *target;

   for (i=0; i<uniedit_nsys; i++) {
      isys  = uniedit_sys[i];
      rm    = 0;
      for (j=0; j<isys->njumps; j++) {
         target = isys->jumps[j].target;
         /* Target already exists, remove. */
         if (target == sys) {
            uniedit_jumpRm( isys, sys );
            uniedit_jumpRm( sys, isys );
            rm = 1;
            break;
         }
      }
      /* Target doesn't exist, add. */
      if (!rm) {
         uniedit_jumpAdd( isys, sys );
         uniedit_jumpAdd( sys, isys );
      }
   }

   /* Reconstruct jumps just in case. */
   systems_reconstructJumps();

   /* Reconstruct universe presences. */
   space_reconstructPresences();

   dsys_saveSystem( sys );
   dsys_saveSystem( isys );

   /* Update sidebar text. */
   uniedit_selectText();
}


/**
 * @brief Adds a new Star System jump.
 */
static void uniedit_jumpAdd( StarSystem *sys, StarSystem *targ )
{
   JumpPoint *jp;

   /* Add the jump. */
   sys->njumps++;
   sys->jumps  = realloc( sys->jumps, sizeof(JumpPoint) * sys->njumps );
   jp          = &sys->jumps[ sys->njumps-1 ];
   memset( jp, 0, sizeof(JumpPoint) );

   /* Fill it out with basics. */
   jp->target  = targ;
   jp->targetid = targ->id;
   jp->radius  = 200.;
   jp->flags   = JP_AUTOPOS; /* Will automatically create position. */
   jp->hide    = pow2(HIDE_DEFAULT_JUMP);
}


/**
 * @brief Removes a Star System jump.
 */
static void uniedit_jumpRm( StarSystem *sys, StarSystem *targ )
{
   int i;

   /* Find associated jump. */
   for (i=0; i<sys->njumps; i++)
      if (sys->jumps[i].target == targ)
         break;

   /* Not found. */
   if (i >= sys->njumps) {
      WARN("Jump for system '%s' not found in system '%s' for removal.", targ->name, sys->name);
      return;
   }

   /* Remove the jump. */
   sys->njumps--;
   memmove( &sys->jumps[i], &sys->jumps[i+1], sizeof(JumpPoint) * (sys->njumps - i) );
}


/**
 * @brief Deselects selected targets.
 */
static void uniedit_deselect (void)
{
   if (uniedit_nsys > 0)
      free( uniedit_sys );
   uniedit_sys    = NULL;
   uniedit_nsys   = 0;
   uniedit_msys   = 0;

   /* Change window stuff. */
   window_disableButton( uniedit_wid, "btnJump" );
   window_disableButton( uniedit_wid, "btnRename" );
   window_disableButton( uniedit_wid, "btnEdit" );
   window_disableButton( uniedit_wid, "btnOpen" );
   window_modifyText( uniedit_wid, "txtSelected", "No selection" );
   window_modifyText( uniedit_wid, "txtPresence", "N/A" );
}


/**
 * @brief Adds a system to the selection.
 */
static void uniedit_selectAdd( StarSystem *sys )
{
   /* Allocate if needed. */
   if (uniedit_msys < uniedit_nsys+1) {
      if (uniedit_msys == 0)
         uniedit_msys = 1;
      uniedit_msys  *= 2;
      uniedit_sys    = realloc( uniedit_sys, sizeof(StarSystem*) * uniedit_msys );
   }

   /* Add system. */
   uniedit_sys[ uniedit_nsys ] = sys;
   uniedit_nsys++;

   /* Set text again. */
   uniedit_selectText();

   /* Enable buttons again. */
   window_enableButton( uniedit_wid, "btnJump" );
   window_enableButton( uniedit_wid, "btnRename" );
   window_enableButton( uniedit_wid, "btnEdit" );
   if (uniedit_nsys == 1)
      window_enableButton( uniedit_wid, "btnOpen" );
   else
      window_disableButton( uniedit_wid, "btnOpen" );
}


/**
 * @brief Removes a system from the selection.
 */
static void uniedit_selectRm( StarSystem *sys )
{
   int i;
   for (i=0; i<uniedit_nsys; i++) {
      if (uniedit_sys[i] == sys) {
         uniedit_nsys--;
         memmove( &uniedit_sys[i], &uniedit_sys[i+1], sizeof(StarSystem*) * (uniedit_nsys - i) );
         uniedit_selectText();
         if (uniedit_nsys == 1)
            window_enableButton( uniedit_wid, "btnOpen" );
         else
            window_disableButton( uniedit_wid, "btnOpen" );
         return;
      }
   }
   WARN("Trying to remove system '%s' from selection when not selected.", sys->name);
}


/**
 * @brief Sets the selected system text.
 */
void uniedit_selectText (void)
{
   int i, l;
   char buf[1024];
   StarSystem *sys;
   int hasPresence;

   l = 0;
   for (i=0; i<uniedit_nsys; i++) {
      l += nsnprintf( &buf[l], sizeof(buf)-l, "%s%s", uniedit_sys[i]->name,
            (i == uniedit_nsys-1) ? "" : ", " );
   }
   if (l == 0)
      uniedit_deselect();
   else {
      window_modifyText( uniedit_wid, "txtSelected", buf );

      /* Presence text. */
      if (uniedit_nsys == 1) {
         sys         = uniedit_sys[0];
         buf[0]      = '\0';
         hasPresence = 0;
         l           = 0;

         for (i=0; i < sys->npresence ; i++) {

            /* Must have presence. */
            if (sys->presence[i].value <= 0)
               continue;

            hasPresence = 1;
            /* Use map grey instead of default neutral colour */
            l += nsnprintf( &buf[l], sizeof(buf)-l, "%s\e0%s: %.0f",
                  (l==0)?"":"\n", faction_name(sys->presence[i].faction),
                  sys->presence[i].value);
         }
         if (hasPresence == 0)
            nsnprintf( buf, sizeof(buf), "None" );

         window_modifyText( uniedit_wid, "txtPresence", buf );
      }
      else
         window_modifyText( uniedit_wid, "txtPresence", "Multiple selected" );
   }
}


/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void uniedit_buttonZoom( unsigned int wid, char* str )
{
   (void) wid;

   /* Transform coords to normal. */
   uniedit_xpos /= uniedit_zoom;
   uniedit_ypos /= uniedit_zoom;

   /* Apply zoom. */
   if (strcmp(str,"btnZoomIn")==0) {
      uniedit_zoom *= UNIEDIT_ZOOM_STEP;
      uniedit_zoom = MIN(pow(UNIEDIT_ZOOM_STEP, UNIEDIT_ZOOM_MAX), uniedit_zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      uniedit_zoom /= UNIEDIT_ZOOM_STEP;
      uniedit_zoom = MAX(pow(UNIEDIT_ZOOM_STEP, UNIEDIT_ZOOM_MIN), uniedit_zoom);
   }

   /* Hack for the circles to work. */
   map_setZoom(uniedit_zoom);

   /* Transform coords back. */
   uniedit_xpos *= uniedit_zoom;
   uniedit_ypos *= uniedit_zoom;
}


/**
 * @brief Edits an individual system or group of systems.
 */
static void uniedit_editSys (void)
{
   unsigned int wid;
   int x, y, l;
   char buf[128], *s;
   StarSystem *sys;

   /* Must have a system. */
   if (uniedit_nsys == 0)
      return;
   sys   = uniedit_sys[0];

   /* Create the window. */
   wid = window_create( "Star System Property Editor", -1, -1, UNIEDIT_EDIT_WIDTH, UNIEDIT_EDIT_HEIGHT );
   uniedit_widEdit = wid;

   x = 20;

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", uniedit_editSysClose );

   /* Rename button. */
   y = -45;
   nsnprintf( buf, sizeof(buf), "Name: \en%s", (uniedit_nsys > 1) ? "\ervarious" : uniedit_sys[0]->name );
   window_addText( wid, x, y, 180, 15, 0, "txtName", &gl_smallFont, &cDConsole, buf );
   window_addButton( wid, 200, y+3, BUTTON_WIDTH, 21, "btnRename", "Rename", uniedit_btnEditRename );

   /* Add general stats */
   s = "Radius";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtRadius",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 7, y, 80, 20, "inpRadius", 10, 1, NULL );
   window_setInputFilter( wid, "inpRadius",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 80 + 12;
   s = "(Scales asset positions)";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtRadiusComment",
         NULL, &cBlack, s );

   /* New row. */
   x = 20;
   y -= gl_defFont.h + 15;
   
   s = "Stars";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtStars",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpStars", 4, 1, NULL );
   window_setInputFilter( wid, "inpStars",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 50 + 12;

   s = "Interference";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtInterference",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 7, y, 55, 20, "inpInterference", 5, 1, NULL );
   window_setInputFilter( wid, "inpInterference",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );

   /* New row. */
   x = 20;
   y -= gl_defFont.h + 15;

   s = "Nebula";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtNebula",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpNebula", 4, 1, NULL );
   window_setInputFilter( wid, "inpNebula",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 50 + 12;

   s = "Volatility";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtVolatility",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 7, y, 50, 20, "inpVolatility", 4, 1, NULL );
   window_setInputFilter( wid, "inpVolatility",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 50 + 12;

   /* Load values */
   nsnprintf( buf, sizeof(buf), "%g", sys->radius );
   window_setInput( wid, "inpRadius", buf );
   nsnprintf( buf, sizeof(buf), "%d", sys->stars );
   window_setInput( wid, "inpStars", buf );
   nsnprintf( buf, sizeof(buf), "%g", sys->interference );
   window_setInput( wid, "inpInterference", buf );
   nsnprintf( buf, sizeof(buf), "%g", sys->nebu_density );
   window_setInput( wid, "inpNebula", buf );
   nsnprintf( buf, sizeof(buf), "%g", sys->nebu_volatility );
   window_setInput( wid, "inpVolatility", buf );

   /* Generate the list. */
   uniedit_editGenList( wid );
}


/**
 * @brief Generates the virtual asset list.
 */
static void uniedit_editGenList( unsigned int wid )
{
   int i, j, n;
   StarSystem *sys;
   Planet *p;
   char **str;
   int y, h, has_assets;

   /* Destroy if exists. */
   if (widget_exists( wid, "lstAssets" ))
      window_destroyWidget( wid, "lstAssets" );

   y = -175;

   /* Check to see if it actually has virtual assets. */
   sys   = uniedit_sys[0];
   n     = sys->nplanets;
   has_assets = 0;
   for (i=0; i<n; i++) {
      p     = sys->planets[i];
      if (p->real == ASSET_VIRTUAL) {
         has_assets = 1;
         break;
      }
   }

   /* Generate list. */
   j     = 0;
   str   = malloc( sizeof(char*) * (n+1) );
   if (has_assets) {
      /* Virtual asset button. */
      for (i=0; i<n; i++) {
         p     = sys->planets[i];
         if (p->real == ASSET_VIRTUAL)
            str[j++] = strdup( p->name );
      }
   }
   else
      str[j++] = strdup("None");

   /* Add list. */
   h = UNIEDIT_EDIT_HEIGHT+y-20 - 2*(BUTTON_HEIGHT+20);
   window_addList( wid, 20, y, UNIEDIT_EDIT_WIDTH-40, h,
         "lstAssets", str, j, 0, NULL );
   y -= h + 20;

   /* Add buttons if needed. */
   if (!widget_exists( wid, "btnRmAsset" ))
      window_addButton( wid, -20, y+3, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnRmAsset", "Remove", uniedit_btnEditRmAsset );
   if (!widget_exists( wid, "btnAddAsset" ))
      window_addButton( wid, -40-BUTTON_WIDTH, y+3, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnAddAsset", "Add", uniedit_btnEditAddAsset );
}


/**
 * @brief Closes the system property editor, saving the changes made.
 */
static void uniedit_editSysClose( unsigned int wid, char *name )
{
   StarSystem *sys;
   double scale;
   
   /* We already know the system exists because we checked when opening the dialog. */
   sys   = uniedit_sys[0];

   /* Changes in radius need to scale the system asset positions. */
   scale = atof(window_getInput( wid, "inpRadius" )) / sys->radius;
   sysedit_sysScale(sys, scale);

   sys->stars           = atoi(window_getInput( wid, "inpStars" ));
   sys->interference    = atof(window_getInput( wid, "inpInterference" ));
   sys->nebu_density    = atof(window_getInput( wid, "inpNebula" ));
   sys->nebu_volatility = atof(window_getInput( wid, "inpVolatility" ));

   /* Reconstruct universe presences. */
   space_reconstructPresences();

   /* Text might need changing. */
   uniedit_selectText();

   dsys_saveSystem( uniedit_sys[0] );

   /* Close the window. */
   window_close( wid, name );
}


/**
 * @brief Removes a selected asset.
 */
static void uniedit_btnEditRmAsset( unsigned int wid, char *unused )
{
   (void) unused;
   char *selected;
   int ret;

   /* Get selection. */
   selected = toolkit_getList( wid, "lstAssets" );

   /* Make sure it's valid. */
   if ((selected==NULL) || (strcmp(selected,"None")==0))
      return;

   /* Remove the asset. */
   ret = system_rmPlanet( uniedit_sys[0], selected );
   if (ret != 0) {
      dialogue_alert( "Failed to remove planet '%s'!", selected );
      return;
   }

   uniedit_editGenList( wid );
}


/**
 * @brief Adds a new virtual asset.
 */
static void uniedit_btnEditAddAsset( unsigned int parent, char *unused )
{
   (void) parent;
   (void) unused;
   unsigned int wid;
   int i, j, n;
   Planet *p;
   char **str;
   int h;

   /* Get all assets. */
   p  = planet_getAll( &n );
   j  = 0;
   for (i=0; i<n; i++)
      if (p[i].real == ASSET_VIRTUAL)
         j = 1;
   if (j==0) {
      dialogue_alert( "No virtual assets to add! Please add virtual assets to dat/asset.xml first." );
      return;
   }

   /* Create the window. */
   wid = window_create( "Add a Virtual Asset", -1, -1, UNIEDIT_EDIT_WIDTH, UNIEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   /* Add virtual asset list. */
   str   = malloc( sizeof(char*) * n );
   j     = 0;
   for (i=0; i<n; i++)
      if (p[i].real == ASSET_VIRTUAL)
         str[j++] = strdup( p[i].name );
   h = UNIEDIT_EDIT_HEIGHT-60-(BUTTON_HEIGHT+20);
   window_addList( wid, 20, -40, UNIEDIT_EDIT_WIDTH-40, h,
         "lstAssets", str, j, 0, NULL );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Add button. */
   window_addButton( wid, -20-(BUTTON_WIDTH+20), 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnAdd", "Add", uniedit_btnEditAddAssetAdd );
}


/**
 * @brief Actually adds the virtual asset.
 */
static void uniedit_btnEditAddAssetAdd( unsigned int wid, char *unused )
{
   char *selected;
   int ret;

   /* Get selection. */
   selected = toolkit_getList( wid, "lstAssets" );
   if (selected == NULL)
      return;

   /* Add virtual presence. */
   ret = system_addPlanet( uniedit_sys[0], selected );
   if (ret != 0) {
      dialogue_alert( "Failed to add virtual asset '%s'!", selected );
      return;
   }

   /* Regenerate the list. */
   uniedit_editGenList( uniedit_widEdit );

   dsys_saveSystem( uniedit_sys[0] );

   /* Close the window. */
   window_close( wid, unused );
}


/**
 * @brief Renames the systems in the system editor.
 */
static void uniedit_btnEditRename( unsigned int wid, char *unused )
{
   (void) unused;
   char buf[128];

   /* Rename systems. */
   uniedit_renameSys();

   /* Update text. */
   nsnprintf( buf, sizeof(buf), "Name: %s", (uniedit_nsys > 1) ? "\ervarious" : uniedit_sys[0]->name );
   window_modifyText( wid, "txtName", buf );
}



