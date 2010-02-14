/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_sysedit.c
 *
 * @brief Handles the star system editor.
 */

#include "dev_sysedit.h"

#include "naev.h"

#include "SDL.h"

#include "space.h"
#include "toolkit.h"
#include "opengl.h"
#include "map.h"
#include "dev_system.h"
#include "unidiff.h"
#include "dialogue.h"
#include "tk/toolkit_priv.h"


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define SYSEDIT_DRAG_THRESHOLD   300   /**< Drag threshold. */
#define SYSEDIT_MOVE_THRESHOLD   10    /**< Movement threshold. */


/*
 * The editor modes.
 */
#define SYSEDIT_DEFAULT    0  /**< Default editor mode. */
#define SYSEDIT_JUMP       1  /**< Jump point toggle mode. */
#define SYSEDIT_NEWSYS     2  /**< New system editor mode. */


extern int systems_nstack;


static int sysedit_mode       = SYSEDIT_DEFAULT; /**< Editor mode. */
static unsigned int sysedit_wid = 0; /**< Sysedit wid. */
static double sysedit_xpos    = 0.; /**< Viewport X position. */
static double sysedit_ypos    = 0.; /**< Viewport Y position. */
static double sysedit_zoom    = 1.; /**< Viewport zoom level. */
static int sysedit_moved      = 0;  /**< Space moved since mouse down. */
static unsigned int sysedit_dragTime = 0; /**< Tick last started to drag. */
static int sysedit_drag       = 0;  /**< Dragging viewport around. */
static int sysedit_dragSys    = 0;  /**< Dragging system around. */
static StarSystem **sysedit_sys = NULL; /**< Selected systems. */
static StarSystem *sysedit_tsys = NULL; /**< Temporarily clicked system. */
static int sysedit_tadd       = 0;  /**< Temporarily clicked system should be added. */
static int sysedit_nsys       = 0;  /**< Number of selected systems. */
static int sysedit_msys       = 0;  /**< Memory allocated for selected systems. */
static double sysedit_mx      = 0.; /**< X mouse position. */
static double sysedit_my      = 0.; /**< Y mouse position. */


/*
 * Prototypes.
 */
/* Selection. */
static void sysedit_deselect (void);
static void sysedit_selectAdd( StarSystem *sys );
static void sysedit_selectRm( StarSystem *sys );
static void sysedit_selectText (void);
/* Misc modes. */
static int sysedit_checkName( char *name );
static void sysedit_renameSys (void);
static void sysedit_newSys( double x, double y );
static void sysedit_toggleJump( StarSystem *sys );
static void sysedit_jumpAdd( StarSystem *sys, StarSystem *targ );
static void sysedit_jumpRm( StarSystem *sys, StarSystem *targ );
/* Custom system editor widget. */
static void sysedit_buttonZoom( unsigned int wid, char* str );
static void sysedit_render( double bx, double by, double w, double h, void *data );
static void sysedit_renderOverlay( double bx, double by, double bw, double bh, void* data );
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Button functions. */
static void sysedit_close( unsigned int wid, char *wgt );
static void sysedit_save( unsigned int wid_unused, char *unused );
static void sysedit_btnJump( unsigned int wid_unused, char *unused );
static void sysedit_btnRename( unsigned int wid_unused, char *unused );
static void sysedit_btnNew( unsigned int wid_unused, char *unused );
/* Keybindings handling. */
static int sysedit_keys( unsigned int wid, SDLKey key, SDLMod mod );


/**
 * @brief Opens the system editor interface.
 */
void sysedit_open( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   unsigned int wid;

   /* Needed to generate faction disk. */
   map_setZoom( 1. );

   /* Must have no diffs applied. */
   diff_clear();

   /* Reset some variables. */
   sysedit_mode   = SYSEDIT_DEFAULT;
   sysedit_drag   = 0;
   sysedit_dragSys = 0;
   sysedit_tsys   = NULL;
   sysedit_tadd   = 0;
   sysedit_zoom   = 1.;
   sysedit_xpos   = 0.;
   sysedit_ypos   = 0.;

   /* Create the window. */
   wid = window_create( "System Editor", -1, -1, -1, -1 );
   window_handleKeys( wid, sysedit_keys );
   sysedit_wid = wid;

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", sysedit_close );

   /*Save button. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*1, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSave", "Save", sysedit_save );

   /* Jump toggle. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*3, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnJump", "Jump", sysedit_btnJump );

   /* Rename system. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*4, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRename", "Rename", sysedit_btnRename );

   /* New system. */
   window_addButton( wid, -20, 20+(BUTTON_HEIGHT+20)*5, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", "New Sys", sysedit_btnNew );

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", sysedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", sysedit_buttonZoom );

   /* Selected text. */
   window_addText( wid, 140, 10, SCREEN_W - 80 - 30 - 30 - BUTTON_WIDTH - 20, 30, 0,
         "txtSelected", &gl_smallFont, &cBlack, NULL );

   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 150, SCREEN_H - 100,
         "cstSysEdit", 1, sysedit_render, sysedit_mouse, NULL );
   window_custSetOverlay( wid, "cstSysEdit", sysedit_renderOverlay );

   /* Deselect everything. */
   sysedit_deselect();
}


/**
 * @brief Handles keybindings.
 */
static int sysedit_keys( unsigned int wid, SDLKey key, SDLMod mod )
{
   (void) wid;
   (void) mod;

   switch (key) {
      /* Mode changes. */
      case SDLK_n:
         sysedit_mode = SYSEDIT_NEWSYS;
         return 1;
      case SDLK_j:
         sysedit_mode = SYSEDIT_JUMP;
         return 1;
      case SDLK_ESCAPE:
         sysedit_mode = SYSEDIT_DEFAULT;
         return 1;

      default:
         return 0;
   }
}


/**
 * @brief Closes the system editor widget.
 */
static void sysedit_close( unsigned int wid, char *wgt )
{
   /* Frees some memory. */
   sysedit_deselect();

   /* Reconstruct jumps. */
   systems_reconstructJumps();

   /* Close the window. */
   window_close( wid, wgt );
}


/**
 * @brief Saves the systems.
 */
static void sysedit_save( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   dsys_saveAll();
}


/**
 * @brief Enters the editor in new jump mode.
 */
static void sysedit_btnJump( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   sysedit_mode = SYSEDIT_JUMP;
}


/**
 * @brief Renames selected systems.
 */
static void sysedit_btnRename( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   sysedit_renameSys();
}


/**
 * @brief Enters the editor in new system mode.
 */
static void sysedit_btnNew( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   sysedit_mode = SYSEDIT_NEWSYS;
}


/**
 * @brief System editor custom widget rendering.
 */
static void sysedit_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;
   StarSystem *sys;
   int i;

   /* Parameters. */
   map_renderParams( bx, by, sysedit_xpos, sysedit_ypos, w, h, sysedit_zoom, &x, &y, &r );

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, w, h, r, 1 );

   /* Render system names. */
   map_renderNames( x, y, 1 );

   /* Render the selected system selections. */
   for (i=0; i<sysedit_nsys; i++) {
      sys = sysedit_sys[i];
      gl_drawCircleInRect( x + sys->pos.x * sysedit_zoom, y + sys->pos.y * sysedit_zoom,
            1.5*r, bx, by, w, h, &cWhite, 0 );
   }
}


/**
 * @brief Renders the overlay.
 */
static void sysedit_renderOverlay( double bx, double by, double bw, double bh, void* data )
{
   double x, y;
   (void) bw;
   (void) bh;
   (void) data;

   x = bx + sysedit_mx;
   y = by + sysedit_my;

   if (sysedit_mode == SYSEDIT_NEWSYS)
      toolkit_drawAltText( x, y, "Click to add a new system");
   else if (sysedit_mode == SYSEDIT_JUMP)
      toolkit_drawAltText( x, y, "Click to toggle jump route");
}

/**
 * @brief System editor custom widget mouse handling.
 */
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
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
            return;

         /* Zooming */
         if (event->button.button == SDL_BUTTON_WHEELUP)
            sysedit_buttonZoom( 0, "btnZoomIn" );
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            sysedit_buttonZoom( 0, "btnZoomOut" );

         /* selecting star system */
         else {
            mx -= w/2 - sysedit_xpos;
            my -= h/2 - sysedit_ypos;

            if (sysedit_mode == SYSEDIT_NEWSYS) {
               sysedit_newSys( mx, my );
               sysedit_mode = SYSEDIT_DEFAULT;
               return;
            }

            for (i=0; i<systems_nstack; i++) {
               sys = system_getIndex( i );

               /* get position */
               x = sys->pos.x * sysedit_zoom;
               y = sys->pos.y * sysedit_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* Try to find in selected systems - begin drag move. */
                  for (i=0; i<sysedit_nsys; i++) {
                     if (sysedit_sys[i] == sys) {
                        if (sysedit_mode == SYSEDIT_DEFAULT) {
                           sysedit_dragSys   = 1;
                           sysedit_tsys      = sys;

                           /* Check modifier. */
                           if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                              sysedit_tadd      = 0;
                           else
                              sysedit_tadd      = -1;
                           sysedit_dragTime  = SDL_GetTicks();
                           sysedit_moved     = 0;
                        }
                        return;
                     }
                  }

                  if (sysedit_mode == SYSEDIT_DEFAULT) {
                     /* Add the system if not selected. */
                     if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                        sysedit_selectAdd( sys );
                     else {
                        sysedit_deselect();
                        sysedit_selectAdd( sys );
                     }
                     sysedit_tsys      = NULL;

                     /* Start dragging anyway. */
                     sysedit_dragSys   = 1;
                     sysedit_dragTime  = SDL_GetTicks();
                     sysedit_moved     = 0;
                  }
                  else if (sysedit_mode == SYSEDIT_JUMP) {
                     sysedit_toggleJump( sys );
                     sysedit_mode = SYSEDIT_DEFAULT;
                  }
                  return;
               }
            }

            /* Start dragging. */
            if ((sysedit_mode == SYSEDIT_DEFAULT) && !(mod & (KMOD_LCTRL | KMOD_RCTRL))) {
               sysedit_drag      = 1;
               sysedit_dragTime  = SDL_GetTicks();
               sysedit_moved     = 0;
               sysedit_tsys      = NULL;
            }
            return;
         }
         break;

      case SDL_MOUSEBUTTONUP:
         if (sysedit_drag) {
            if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD) && (sysedit_moved < SYSEDIT_MOVE_THRESHOLD)) {
               if (sysedit_tsys == NULL)
                  sysedit_deselect();
               else
                  sysedit_selectAdd( sysedit_tsys );
            }
            sysedit_drag      = 0;
         }
         if (sysedit_dragSys) {
            if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD) &&
                  (sysedit_moved < SYSEDIT_MOVE_THRESHOLD) && (sysedit_tsys != NULL)) {
               if (sysedit_tadd == 0)
                  sysedit_selectRm( sysedit_tsys );
               else {
                  sysedit_deselect();
                  sysedit_selectAdd( sysedit_tsys );
               }
            }
            sysedit_dragSys   = 0;
         }
         break;

      case SDL_MOUSEMOTION:
         /* Update mouse positions. */
         sysedit_mx  = mx;
         sysedit_my  = my;

         /* Handle dragging. */
         if (sysedit_drag) {
            /* axis is inverted */
            sysedit_xpos -= event->motion.xrel;
            sysedit_ypos += event->motion.yrel;

            /* Update mousemovement. */
            sysedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
         }
         else if (sysedit_dragSys && (sysedit_nsys > 0)) {
            if ((sysedit_moved > SYSEDIT_MOVE_THRESHOLD) || (SDL_GetTicks() - sysedit_dragTime > SYSEDIT_DRAG_THRESHOLD)) {
               for (i=0; i<sysedit_nsys; i++) {
                  sysedit_sys[i]->pos.x += ((double)event->motion.xrel) / sysedit_zoom;
                  sysedit_sys[i]->pos.y -= ((double)event->motion.yrel) / sysedit_zoom;
               }
            }

            /* Update mousemovement. */
            sysedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
         }
         break;
   }
}


/**
 * @brief Checks to see if a system name is already in use.
 *
 *    @return 1 if system name is already in use.
 */
static int sysedit_checkName( char *name )
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


/**
 * @brief Renames all the currently selected systems.
 */
static void sysedit_renameSys (void)
{
   int i;
   char *name;
   StarSystem *sys;

   for (i=0; i<sysedit_nsys; i++) {
      sys = sysedit_sys[i];

      /* Get name. */
      name = dialogue_input( "Rename Star System", 1, 32, "What do you want to rename \er%s\e0?", sys->name );

      /* Keep current name. */
      if (name == NULL)
         continue;
  
      /* Try again. */
      if (sysedit_checkName( name )) {
         i--;
         continue;
      }

      /* Change the name. */
      free(sys->name);
      sys->name = name;
   }
}


/**
 * @brief Creates a new system.
 */
static void sysedit_newSys( double x, double y )
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
   if (sysedit_checkName( name )) {
      free(name);
      sysedit_newSys( x, y );
      return;
   }

   /* Create the system. */
   sys         = system_new();
   sys->name   = name;
   sys->pos.x  = x;
   sys->pos.y  = y;
   sys->stars  = 400;

   /* Select new system. */
   sysedit_deselect();
   sysedit_selectAdd( sys );
}


/**
 * @brief Toggles the jump point for the selected systems.
 */
static void sysedit_toggleJump( StarSystem *sys )
{
   int i, j, rm;
   StarSystem *isys, *target;

   for (i=0; i<sysedit_nsys; i++) {
      isys  = sysedit_sys[i];
      rm    = 0;
      for (j=0; j<isys->njumps; j++) {
         target = isys->jumps[j].target;
         /* Target already exists, remove. */
         if (target == sys) {
            sysedit_jumpRm( isys, sys );
            sysedit_jumpRm( sys, isys );
            rm = 1;
            break;
         }
      }
      /* Target doesn't exist, add. */
      if (!rm) {
         sysedit_jumpAdd( isys, sys );
         sysedit_jumpAdd( sys, isys );
      }
   }

   /* Reconstruct jumps just in case. */
   systems_reconstructJumps();
}


/**
 * @brief Adds a new Star System jump.
 */
static void sysedit_jumpAdd( StarSystem *sys, StarSystem *targ )
{
   JumpPoint *jp;

   /* Add the jump. */
   sys->njumps++;
   sys->jumps  = realloc( sys->jumps, sizeof(JumpPoint) * sys->njumps );
   jp          = &sys->jumps[ sys->njumps-1 ];

   /* Fill it out with basics. */
   jp->target  = targ;
   jp->targetid = targ->id;
   jp->radius  = 200.;
   jp->flags   = JP_AUTOPOS; /* Will automaticalyl create position. */

}


/**
 * @brief Removes a Star System jump.
 */
static void sysedit_jumpRm( StarSystem *sys, StarSystem *targ )
{
   int i;

   /* Find assosciated jump. */
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
static void sysedit_deselect (void)
{
   if (sysedit_nsys > 0)
      free( sysedit_sys );
   sysedit_sys    = NULL;
   sysedit_nsys   = 0;
   sysedit_msys   = 0;
   window_modifyText( sysedit_wid, "txtSelected", "No selection" );
}


/**
 * @brief Adds a system to the selection.
 */
static void sysedit_selectAdd( StarSystem *sys )
{
   /* Allocate if needed. */
   if (sysedit_msys < sysedit_nsys+1) {
      if (sysedit_msys == 0)
         sysedit_msys = 1;
      sysedit_msys  *= 2;
      sysedit_sys    = realloc( sysedit_sys, sizeof(StarSystem*) * sysedit_msys );
   }

   /* Add system. */
   sysedit_sys[ sysedit_nsys ] = sys; 
   sysedit_nsys++;

   /* Set text again. */
   sysedit_selectText();
}


/**
 * @brief Removes a system from the selection.
 */
static void sysedit_selectRm( StarSystem *sys )
{
   int i;
   for (i=0; i<sysedit_nsys; i++) {
      if (sysedit_sys[i] == sys) {
         sysedit_nsys--;
         memmove( &sysedit_sys[i], &sysedit_sys[i+1], sizeof(StarSystem*) * (sysedit_nsys - i) );
         sysedit_selectText();
         return;
      }
   }
   WARN("Trying to remove system '%s' from selection when not selected.", sys->name);
}


/**
 * @brief Sets the selected system text.
 */
static void sysedit_selectText (void)
{
   int i, l;
   char buf[1024];
   l = 0;
   for (i=0; i<sysedit_nsys; i++) {
      l += snprintf( &buf[l], sizeof(buf)-l, "%s%s", sysedit_sys[i]->name,
            (i == sysedit_nsys-1) ? "" : ", " );
   }
   if (l == 0)
      sysedit_deselect();
   else
      window_modifyText( sysedit_wid, "txtSelected", buf );
}


/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void sysedit_buttonZoom( unsigned int wid, char* str )
{
   (void) wid;

   /* Transform coords to normal. */
   sysedit_xpos /= sysedit_zoom;
   sysedit_ypos /= sysedit_zoom;

   /* Apply zoom. */
   if (strcmp(str,"btnZoomIn")==0) {
      sysedit_zoom += (sysedit_zoom >= 1.) ? 0.5 : 0.25;
      sysedit_zoom = MIN(2.5, sysedit_zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      sysedit_zoom -= (sysedit_zoom > 1.) ? 0.5 : 0.25;
      sysedit_zoom = MAX(0.5, sysedit_zoom);
   }

   /* Hack for the circles to work. */
   map_setZoom(sysedit_zoom);

   /* Transform coords back. */
   sysedit_xpos *= sysedit_zoom;
   sysedit_ypos *= sysedit_zoom;
}

