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


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define SYSEDIT_DRAG_THRESHOLD   300   /**< Drag threshold. */
#define SYSEDIT_MOVE_THRESHOLD   10    /**< Movement threshold. */


extern int systems_nstack;


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
static int sysedit_msys       = 1;  /**< Memory allocated for selected systems. */


/*
 * Prototypes.
 */
/* Selection. */
static void sysedit_deselect (void);
static void sysedit_selectAdd( StarSystem *sys );
static void sysedit_selectRm( StarSystem *sys );
static void sysedit_selectText (void);
/* Custom system editor widget. */
static void sysedit_buttonZoom( unsigned int wid, char* str );
static void sysedit_render( double bx, double by, double w, double h, void *data );
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Button functions. */
static void sysedit_save( unsigned int wid_unused, char *unused );


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
   sysedit_zoom = 1.;

   /* Create the window. */
   wid = window_create( "System Editor", -1, -1, -1, -1 );
   sysedit_wid = wid;

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /*Save button. */
   window_addButton( wid, -20, 20+(BUTTON_WIDTH+20)*1, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSave", "Save", sysedit_save );

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", sysedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", sysedit_buttonZoom );

   /* Selected text. */
   window_addText( wid, 140, 10, SCREEN_W - 80 - 30 - 30 - BUTTON_WIDTH - 20, 30, 0,
         "txtSelected", &gl_smallFont, &cBlack, NULL );

   /* Actual viewport. */
   window_addCust( wid, 20, -40, SCREEN_W - 150, SCREEN_H - 100,
         "cstSysEdit", 1, sysedit_render, sysedit_mouse, NULL );

   /* Deselect everything. */
   sysedit_deselect();
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
 * @brief System editor custom widget rendering.
 */
static void sysedit_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;

   /* Parameters. */
   map_renderParams( bx, by, sysedit_xpos, sysedit_ypos, w, h, sysedit_zoom, &x, &y, &r );

   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Render systems. */
   map_renderSystems( bx, by, x, y, w, h, r, 1 );

   /* Render system names. */
   map_renderNames( x, y, 1 );
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

            for (i=0; i<systems_nstack; i++) {
               sys = system_getIndex( i );

               /* get position */
               x = sys->pos.x * sysedit_zoom;
               y = sys->pos.y * sysedit_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* Try to find in selected systems - begin drag move. */
                  for (i=0; i<sysedit_nsys; i++) {
                     if (sysedit_sys[i] == sys) {
                        sysedit_dragSys   = 1;
                        sysedit_tsys      = sys;

                        /* Check modifier. */
                        if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                           sysedit_tadd      = 0;
                        else
                           sysedit_tadd      = -1;
                        sysedit_dragTime  = SDL_GetTicks();
                        sysedit_moved     = 0;
                        return;
                     }
                  }

                  /* Add the system if not selected. */
                  if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                     sysedit_selectAdd( sys );
                  else {
                     sysedit_deselect();
                     sysedit_selectAdd( sys );
                  }
                  sysedit_tsys      = NULL;
                  return;
               }
            }

            /* Start dragging. */
            sysedit_drag      = 1;
            sysedit_dragTime  = SDL_GetTicks();
            sysedit_moved     = 0;
            sysedit_tsys      = NULL;
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
            if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD) && (sysedit_moved < SYSEDIT_MOVE_THRESHOLD)) {
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
      window_modifyText( sysedit_wid, "txtSelected", "No selection" );
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

