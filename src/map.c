/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"

#include <malloc.h>
#include <math.h>
#include <float.h>

#include "log.h"
#include "naev.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"
#include "mission.h"


#define WINDOW_WIDTH    650
#define WINDOW_HEIGHT   540

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
static void map_selectCur (void);


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

   /* mark systems as needed */
   mission_sysMark();

   /* Attempt to select current map if none is selected */
   if (map_selected == -1)
      map_selectCur();

   map_wid = window_create( "Star Map", -1, -1,
         WINDOW_WIDTH, WINDOW_HEIGHT );

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
   window_addText( map_wid, -20, -20, 100, 20, 1, "txtSysname",
         &gl_defFont, &cDConsole, systems_stack[ map_selected ].name );
   /* Faction */
   window_addText( map_wid, -20, -60, 90, 20, 0, "txtSFaction",
         &gl_smallFont, &cDConsole, "Faction:" );
   window_addText( map_wid, -20, -60-gl_smallFont.h-5, 80, 100, 0, "txtFaction",
         &gl_smallFont, &cBlack, NULL );
   /* Standing */
   window_addText( map_wid, -20, -100, 90, 20, 0, "txtSStanding",
         &gl_smallFont, &cDConsole, "Standing:" );
   window_addText( map_wid, -20, -100-gl_smallFont.h-5, 80, 100, 0, "txtStanding",
         &gl_smallFont, &cBlack, NULL );
   /* Planets */
   window_addText( map_wid, -20, -140, 90, 20, 0, "txtSPlanets",
         &gl_smallFont, &cDConsole, "Planets:" );
   window_addText( map_wid, -20, -140-gl_smallFont.h-5, 80, 100, 0, "txtPlanets",
         &gl_smallFont, &cBlack, NULL );
   /* Services */
   window_addText( map_wid, -20, -180, 90, 20, 0, "txtSServices",
         &gl_smallFont, &cDConsole, "Services:" );
   window_addText( map_wid, -20, -180-gl_smallFont.h-5, 80, 100, 0, "txtServices",
         &gl_smallFont, &cBlack, NULL );
   /* Close button */
   window_addButton( map_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnClose", "Close", map_close );

         
   /*
    * The map itself.
    */
   window_addCust( map_wid, 20, -40, MAP_WIDTH, MAP_HEIGHT,
         "cstMap", 1, map_render, map_mouse );

   /*
    * Bottom stuff
    *
    * [+] [-]  Nebulae, Asteroids, Interference
    */
   /* Zoom buttons */
   window_addButton( map_wid, 40, 20, 30, 30, "btnZoomIn", "+", map_buttonZoom );
   window_addButton( map_wid, 80, 20, 30, 30, "btnZoomOut", "-", map_buttonZoom );
   /* Situation text */
   window_addText( map_wid, 140, 10, WINDOW_WIDTH - 80 - 30 - 30, 30, 0,
         "txtSystemStatus", &gl_smallFont, &cBlack, NULL );

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
   int f, y, h, standing, nstanding;
   unsigned int services;
   char buf[128];

   sys = &systems_stack[ map_selected ];
 

   /*
    * Right Text
    */

   if (!sys_isKnown(sys)) { /* System isn't known, erase all */
      window_modifyText( map_wid, "txtSysname", "Unknown" );
      window_modifyText( map_wid, "txtFaction", "Unknown" );
      window_modifyText( map_wid, "txtStanding", "Unknown" );
      window_modifyText( map_wid, "txtPlanets", "Unknown" );
      window_moveWidget( map_wid, "txtSServices", -20, -180 );
      window_moveWidget( map_wid, "txtServices", -20, -180-gl_smallFont.h-5 );
      window_modifyText( map_wid, "txtServices", "Unknown" );
      window_modifyText( map_wid, "txtSystemStatus", NULL );
      return;
   }

   /* System is known */
   window_modifyText( map_wid, "txtSysname", sys->name );

   if (sys->nplanets == 0) { /* no planets -> no factions */
      window_modifyText( map_wid, "txtFaction", "NA" );
      window_modifyText( map_wid, "txtStanding", "NA" );
   }
   else {
      standing = 0;
      nstanding = 0;
      f = -1;
      for (i=0; i<sys->nplanets; i++) {
         if ((f==-1) && (sys->planets[i].faction!=0)) {
            f = sys->planets[i].faction;
            standing += faction_getPlayer( f );
            nstanding++;
         }
         else if (f!= sys->planets[i].faction && /* TODO more verbosity */
               (sys->planets[i].faction!=0)) {
            snprintf( buf, 100, "Multiple" );
         }
      }
      if (i==sys->nplanets) /* saw them all and all the same */
         snprintf( buf, 100, "%s", faction_name(f) );

      /* Modify the text */
      window_modifyText( map_wid, "txtFaction", buf );
      window_modifyText( map_wid, "txtStanding",
            faction_getStanding( standing / nstanding ) );
   }

   /* Get planets */
   if (sys->nplanets == 0)
      window_modifyText( map_wid, "txtPlanets", "None" );
   else {
      buf[0] = '\0';
      if (sys->nplanets > 0)
         strcat( buf, sys->planets[0].name );
      for (i=1; i<sys->nplanets; i++) {
         strcat( buf, ",\n" );
         strcat( buf, sys->planets[i].name );
      }

      window_modifyText( map_wid, "txtPlanets", buf );
   }

   /* Get the services */
   h = gl_printHeight( &gl_smallFont, 80, buf );
   y = -180 - (h - gl_smallFont.h);
   window_moveWidget( map_wid, "txtSServices", -20, y );
   y -= gl_smallFont.h+5;
   window_moveWidget( map_wid, "txtServices", -20, y );
   services = 0;
   for (i=0; i<sys->nplanets; i++)
      services |= sys->planets[i].services;
   buf[0] = '\0';
   if (services & PLANET_SERVICE_COMMODITY)
      strcat(buf, "Commodity\n");
   if (services & PLANET_SERVICE_OUTFITS)
      strcat(buf, "Outfits\n");
   if (services & PLANET_SERVICE_SHIPYARD)
      strcat(buf, "Shipyard\n");
   if (buf[0] == '\0')
      strcat(buf, "None");
   window_modifyText( map_wid, "txtServices", buf );


   /*
    * System Status
    */
   buf[0] = '\0';
   if (sys->nebu_density > 0.) { /* Has nebulae */

      /* Volatility */
      if (sys->nebu_volatility > 700.)
         strcat(buf," Volatile");
      else if (sys->nebu_volatility > 300.)
         strcat(buf," Dangerous");
      else if (sys->nebu_volatility > 0.)
         strcat(buf," Unstable");
      /* Density */
      if (sys->nebu_density > 700.)
         strcat(buf," Dense");
      else if (sys->nebu_density < 300.)
         strcat(buf," Light");
      strcat(buf," Nebulae");
   }
   window_modifyText( map_wid, "txtSystemStatus", buf );
}


/*
 * returns 1 if sys is part of the map_path, 2 if won't have enough fuel
 */
static int map_inPath( StarSystem *sys )
{
   int i, f;

   f = pilot_getJumps(player) - 1;
   for (i=0; i<map_npath; i++)
      if (map_path[i] == sys) {
         if (i > f) {
            return 2;
         }
         else
            return 1;
      }
   return 0;
}


/*
 * renders the map as a custom widget
 */
static void map_render( double bx, double by, double w, double h )
{
   int i,j, n,m;
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

      /* check to make sure system is known or adjacent to known (or marked) */
      if (!sys_isMarked(sys) && !space_sysReachable(sys)) continue;

      /* system colours */
      if (sys==cur_system) COLOUR(cRadar_targ);
      else if (!sys_isKnown(sys) || (sys->nplanets==0)) COLOUR(cInert);
      else if (areEnemies(player->faction, sys->faction)) COLOUR(cRed);
      else COLOUR(cYellow);

      /* draw the system */
      tx = x + sys->pos.x*map_zoom;
      ty = y + sys->pos.y*map_zoom;
      gl_drawCircleInRect( tx, ty, r, bx, by, w, h );

      /* mark the system if needed */
      if (sys_isMarked(sys)) {
         COLOUR(cRed);
         glBegin(GL_TRIANGLES);
            glVertex2d( tx+r+9, ty+r+3 );
            glVertex2d( tx+r+3, ty+r+3 );
            glVertex2d( tx+r+3, ty+r+9 );
         glEnd(); /* GL_TRIANGLES */
      }

      /* draw the system name */
      if (sys_isKnown(sys)) {
         tx = x + 7. + sys->pos.x * map_zoom;
         ty = y - 5. + sys->pos.y * map_zoom;
         gl_print( &gl_smallFont,
               tx + SCREEN_W/2., ty + SCREEN_H/2.,
               &cWhite, sys->name );
      }


      if (!sys_isKnown(sys)) continue; /* we don't draw hyperspace lines */

      /* draw the hyperspace paths */
      glShadeModel(GL_SMOOTH);
      /* cheaply use transparency instead of actually calculating
       * from where to where the line must go :) */  
      for (j=0; j<sys->njumps; j++) {

         n = map_inPath(&systems_stack[ sys->jumps[j]]);
         m = map_inPath(sys);
         /* set the colours */
         /* is the route the current one? */
         if ((hyperspace_target != -1) && 
               ( ((cur_system==sys) && (j==hyperspace_target)) ||
                  ((cur_system==&systems_stack[ sys->jumps[j] ]) &&
                     (sys==&systems_stack[ cur_system->jumps[hyperspace_target] ] ))))
            col = &cGreen;
         /* is the route part of the path? */
         else if ((n > 0) && (m > 0)) {
            if ((n == 2) || (m == 2)) /* out of fuel */
               col = &cRed;
            else
               col = &cYellow;
         }
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
   if (map_selected != -1) {
      sys = &systems_stack[ map_selected ];
      COLOUR(cRed);
      gl_drawCircleInRect( x + sys->pos.x * map_zoom, y + sys->pos.y * map_zoom,
            r+3., bx, by, w, h );
   }
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

               /* must be reachable */
               if (!space_sysReachable(sys))
                  continue;

               /* get position */
               x = sys->pos.x * map_zoom;
               y = sys->pos.y * map_zoom;

               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* select the current system and make a path to it */
                  map_selected = i;
                  if (map_path)
                     free(map_path);
                  map_path = map_getJumpPath( &map_npath,
                        cur_system->name, sys->name, 0 );

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
   map_selectCur();
}


static void map_selectCur (void)
{
   int i;

   if (cur_system != NULL)  {
      for (i=0; i<systems_nstack; i++) {
         if (&systems_stack[i] == cur_system) {
            map_selected = i;
            break;
         }
      }
   }
   else {
      /* will probably segfault now */
      map_selected = -1;
   }
}

/*
 * updates the map after a jump
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

/*
 * A* algorithm for shortest path finding
 */
/* the node struct */
typedef struct SysNode_ {
   struct SysNode_ *next, *gnext;

   struct SysNode_ *parent;
   StarSystem* sys;
   double r; /* ranking */
   int g; /* step */
} SysNode;
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
/* creates a new node linke to star system */
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
static double A_h( StarSystem *n, StarSystem *g )
{
   (void)n;
   (void)g;
   /* Euclidean distance */
   /*return sqrt(pow2(n->pos.x - g->pos.x) + pow2(n->pos.y - g->pos.y))/100.;*/
   return 0.;
}
/* gets the g from a node */
static double A_g( SysNode* n )
{
   return n->g;
}
/* adds a node to the linked list */
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
/* removes a node from a linked list */
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
/* checks to see if node is in linked list */
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
/* returns the lowest ranking node from a linked list of nodes */
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
/* frees a linked list */
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
StarSystem** map_getJumpPath( int* njumps, char* sysstart, char* sysend, int ignore_known )
{
   int i, cost;

   StarSystem *sys, *ssys, *esys, **res;

   SysNode *cur, *neighbour;
   SysNode *open, *closed;
   SysNode *ocost, *ccost;

   A_gc = NULL;

   /* initial and target systems */
   ssys = system_get(sysstart); /* start */
   esys = system_get(sysend); /* goal */

   /* system target must be known */
   if (!ignore_known && !sys_isKnown(esys)) {
      if (space_sysReachable(esys)) { /* can we still reach it? */
         res = malloc(sizeof(StarSystem*));
         (*njumps) = 1;
         res[0] = esys;
         return res;
      }

      /* can't reach - don't make path */
      (*njumps) = 0;
      return NULL;
   }

   /* start the linked lists */
   open = closed =  NULL;
   cur = A_newNode( ssys, NULL );
   open = A_add( open, cur ); /* inital open node is the start system */

   while ((cur = A_lowest(open))->sys != esys) {
      /* get best from open and toss to closed */
      open = A_rm( open, cur->sys );
      closed = A_add( closed, cur );
      cost = A_g(cur) + 1;

      for (i=0; i<cur->sys->njumps; i++) {
         sys = &systems_stack[cur->sys->jumps[i]];

         if (!ignore_known && !sys_isKnown(sys)) continue;

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

   /* build path backwards */
   (*njumps) = A_g(cur);
   res = malloc( sizeof(StarSystem*) * (*njumps) );
   for (i=0; i<(*njumps); i++) {
      res[(*njumps)-i-1] = cur->sys;
      cur = cur->parent;
   }

   /* free the linked lists */
   A_freeList(A_gc);
   return res;
}


/*
 * marks maps around a radius of currenty system as known
 */
int map_map( char* targ_sys, int r )
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
         jsys = &systems_stack[cur->sys->jumps[i]];

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


