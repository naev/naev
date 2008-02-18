/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"


#include "log.h"
#include "naev.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"


#define WINDOW_WIDTH    550
#define WINDOW_HEIGHT   440

#define MAP_WIDTH       (WINDOW_WIDTH-150)
#define MAP_HEIGHT      (WINDOW_HEIGHT-100)

#define BUTTON_WIDTH    60
#define BUTTON_HEIGHT   30


static int map_wid = 0;
static double map_zoom = 1.; /* zoom of the map */
static double map_xpos = 0.; /* map position */
static double map_ypos = 0.;
static int map_selected = -1;
static StarSystem **map_path = NULL; /* the path to current selected system */
static int map_npath = 0;

static int map_drag = 0; /* is the user dragging the map? */

/*
 * extern
 */
/* space.c */
extern StarSystem *systems_stack;
extern int systems_nstack;
/* player.c */
extern int planet_target;
extern int hyperspace_target;


/*
 * prototypes
 */
static void map_close( char* str );
static void map_update (void);
static int map_inPath( StarSystem *sys );
static void map_render( double bx, double by, double w, double h );
static void map_mouse( SDL_Event* event, double mx, double my );
static void map_buttonZoom( char* str );


/*
 * opens the map window
 */
void map_open (void)
{
   if (map_wid) {
      map_close(NULL);
      return;
   }

   /* set position to focus on current system */
   map_xpos = cur_system->pos.x;
   map_ypos = cur_system->pos.y;

   map_wid = window_create( "Star Map", -1, -1,
         WINDOW_WIDTH, WINDOW_HEIGHT );

   window_addText( map_wid, -20, -20, 100, 20, 1, "txtSysname",
         &gl_defFont, &cDConsole, systems_stack[ map_selected ].name );
   window_addText( map_wid, -20, -60, 90, 20, 0, "txtSFaction",
         &gl_smallFont, &cDConsole, "Faction:" );
   window_addText( map_wid, -20, -60-gl_smallFont.h-5, 80, 100, 0, "txtFaction",
         &gl_smallFont, &cBlack, NULL );
   window_addText( map_wid, -20, -110, 90, 20, 0, "txtSPlanets",
         &gl_smallFont, &cDConsole, "Planets:" );
   window_addText( map_wid, -20, -110-gl_smallFont.h-5, 80, 100, 0, "txtPlanets",
         &gl_smallFont, &cBlack, NULL );
         

   window_addCust( map_wid, 20, -40, MAP_WIDTH, MAP_HEIGHT,
         "cstMap", 1, map_render, map_mouse );
   window_addButton( map_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", map_close );

   window_addButton( map_wid, 40, 20, 30, 30, "btnZoomIn", "+", map_buttonZoom );
   window_addButton( map_wid, 80, 20, 30, 30, "btnZoomOut", "-", map_buttonZoom );

   map_update();
}
static void map_close( char* str )
{
   (void)str;
   if (map_wid) {
      window_destroy( map_wid );
      map_wid = 0;
   }
}
static void map_update (void)
{
   int i;
   StarSystem* sys;
   int f;
   char buf[100];

   sys = &systems_stack[ map_selected ];

   window_modifyText( map_wid, "txtSysname", sys->name );

   if (sys->nplanets == 0) /* no planets -> no factions */
      snprintf( buf, 100, "NA" );
   else {
      f = -1;
      for (i=0; i<sys->nplanets; i++) {
         if ((f==-1) && (sys->planets[i].faction!=0))
            f = sys->planets[i].faction;
         else if (f!= sys->planets[i].faction && /* TODO more verbosity */
               (sys->planets[i].faction!=0)) {
            snprintf( buf, 100, "Multiple" );
            break;
         }
      }
      if (i==sys->nplanets) /* saw them all and all the same */
         snprintf( buf, 100, "%s", faction_name(f) );
   }
   window_modifyText( map_wid, "txtFaction", buf );

   buf[0] = '\0';
   if (sys->nplanets == 0)
      snprintf( buf, 100, "None" );
   else {
      if (sys->nplanets > 0)
         strcat( buf, sys->planets[0].name );
      for (i=1; i<sys->nplanets; i++) {
         strcat( buf, ",\n" );
         strcat( buf, sys->planets[i].name );
      }
   }
   window_modifyText( map_wid, "txtPlanets", buf );
}


/*
 * returns 1 if sys is part of the map_path
 */
static int map_inPath( StarSystem *sys )
{
   int i;
   for (i=0; i<map_npath; i++)
      if (map_path[i] == sys)
         return 1;
   return 0;
}


/*
 * renders the map as a custom widget
 */
static void map_render( double bx, double by, double w, double h )
{
   int i,j;
   double x,y,r, tx,ty;
   StarSystem* sys;
   glColour* col;

   r = 5.;
   x = (bx - map_xpos + w/2) * 1.;
   y = (by - map_ypos + h/2) * 1.;

   /* background */
   COLOUR(cBlack);
   glBegin(GL_QUADS);
      glVertex2d( bx, by );
      glVertex2d( bx, by+h );
      glVertex2d( bx+w, by+h );
      glVertex2d( bx+w, by );
   glEnd(); /* GL_QUADS */


   /* render the star systems */
   for (i=0; i<systems_nstack; i++) {
      
      sys = &systems_stack[i];

      /* draw the system */
      if (sys==cur_system) COLOUR(cRadar_targ);
      else if (sys->nplanets==0) COLOUR(cInert); /* TODO dependent on planet type */
      else if (areEnemies(player->faction, sys->faction)) COLOUR(cRed);
      else COLOUR(cYellow);
      gl_drawCircleInRect( x + sys->pos.x*map_zoom, y + sys->pos.y*map_zoom,
            r, bx, by, w, h );
      /* draw the system name */
      tx = x + 7. + sys->pos.x * map_zoom;
      ty = y - 5. + sys->pos.y * map_zoom;
      gl_print( &gl_smallFont,
            tx + gl_screen.w/2., ty + gl_screen.h/2.,
            &cWhite, sys->name );

      /* draw the hyperspace paths */
      glShadeModel(GL_SMOOTH);
      /* cheaply use transparency instead of actually calculating
       * from where to where the line must go :) */  
      for (j=0; j<sys->njumps; j++) {
         /* set the colours */
         /* is the route the current one? */
         if (((cur_system==sys) && (j==hyperspace_target)) ||
               ((cur_system==&systems_stack[ sys->jumps[j] ]) &&
                (sys==&systems_stack[ cur_system->jumps[hyperspace_target] ] )))
            col = &cRed;
         /* is the route part of the path? */
         else if (map_inPath(&systems_stack[ sys->jumps[j]]) && map_inPath(sys))
            col = &cGreen;
         else col = &cDarkBlue;

         glBegin(GL_LINE_STRIP);
            ACOLOUR(*col,0.);
            tx = x + sys->pos.x * map_zoom;
            ty = y + sys->pos.y * map_zoom;
            glVertex2d( tx, ty );
            COLOUR(*col);
            tx += (systems_stack[ sys->jumps[j] ].pos.x - sys->pos.x)/2. * map_zoom;
            ty += (systems_stack[ sys->jumps[j] ].pos.y - sys->pos.y)/2. * map_zoom;
            glVertex2d( tx, ty );
            ACOLOUR(*col,0.);
            tx = x + systems_stack[ sys->jumps[j] ].pos.x * map_zoom;
            ty = y + systems_stack[ sys->jumps[j] ].pos.y * map_zoom;
            glVertex2d( tx, ty );
         glEnd(); /* GL_LINE_STRIP */
      }
      glShadeModel(GL_FLAT);
   }

   /* selected planet */
   sys = &systems_stack[ map_selected ];
   COLOUR(cRed);
   gl_drawCircleInRect( x + sys->pos.x * map_zoom, y + sys->pos.y * map_zoom,
         r+3., bx, by, w, h );
}
/*
 * map event handling
 */
static void map_mouse( SDL_Event* event, double mx, double my )
{
   int i, j;
   double x,y, t;
   StarSystem *sys;

   t = 15.*15.; /* threshold */

   mx -= MAP_WIDTH/2 - map_xpos;
   my -= MAP_HEIGHT/2 - map_ypos;

   switch (event->type) {
      
      case SDL_MOUSEBUTTONDOWN:
         /* zooming */
         if (event->button.button == SDL_BUTTON_WHEELUP)
            map_buttonZoom( "btnZoomOut" );
         else if (event->button.button == SDL_BUTTON_WHEELDOWN)
            map_buttonZoom( "btnZoomIn" );

         /* selecting star system */
         else {
            for (i=0; i<systems_nstack; i++) {
               sys = &systems_stack[i];

               /* get position */
               x = sys->pos.x * map_zoom;
               y = sys->pos.y * map_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* select the current system and make a path to it */
                  map_selected = i;
                  if (map_path)
                     free(map_path);
                  map_path = system_getJumpPath( &map_npath,
                        cur_system->name, sys->name );

                  if (map_npath==0)
                     hyperspace_target = -1;
                  else 
                  /* see if it is a valid hyperspace target */
                  for (j=0; j<cur_system->njumps; j++) {
                     if (map_path[0]==&systems_stack[cur_system->jumps[j]]) {
                        planet_target = -1; /* override planet_target */
                        hyperspace_target = j;
                        break;
                     }
                  }
                  map_update();
                  break;
               }
            }
            map_drag = 1;
         }
         break;

      case SDL_MOUSEBUTTONUP:
         if (map_drag) map_drag = 0;
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
static void map_buttonZoom( char* str )
{
   if (strcmp(str,"btnZoomIn")==0) {
      map_zoom += (map_zoom >= 1.) ? 0.5 : 0.25;
      map_zoom = MIN(2.5, map_zoom);
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      map_zoom -= (map_zoom > 1.) ? 0.5 : 0.25;
      map_zoom = MAX(0.5, map_zoom);
   }
}


/*
 * sets the map to sane defaults
 */
void map_clear (void)
{
   int i;

   map_zoom = 1.;
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
   if (cur_system != NULL)
      for (i=0; i<systems_nstack; i++)
         if (&systems_stack[i] == cur_system) {
            map_selected = i;
            break;
         }
   else
      map_selected = 0;
}


/*
 * updates the map after a jump
 */
void map_jump (void)
{
   int j;

   map_xpos = cur_system->pos.x;
   map_ypos = cur_system->pos.y;

   /* update path if set */
   if (map_path != NULL) {
      map_npath--;
      if (map_npath == 0) { /* path is empty */
         free (map_path);
         map_path = NULL;
      }
      else { /* get rid of bottom of the path */
         memcpy( &map_path[0], &map_path[1], sizeof(StarSystem*) * map_npath );
         map_path = realloc( map_path, sizeof(StarSystem*) * map_npath );

         /* set the next jump to be to the next in path */
         for (j=0; j<cur_system->njumps; j++) {
            if (map_path[0]==&systems_stack[cur_system->jumps[j]]) {
               planet_target = -1; /* override planet_target */
               hyperspace_target = j;
               break;
            }
         }

      }
   }
}



