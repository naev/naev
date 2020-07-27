/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "log.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"
#include "mission.h"
#include "colour.h"
#include "player.h"
#include "faction.h"
#include "dialogue.h"
#include "gui.h"
#include "map_find.h"
#include "array.h"
#include "mapData.h"
#include "nstring.h"
#include "nmath.h"
#include "nmath.h"
#include "nxml.h"
#include "ndata.h"


#define BUTTON_WIDTH    80 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define MAP_LOOP_PROT   1000 /**< Number of iterations max in pathfinding before
                                 aborting. */

/* map decorator stack */
static MapDecorator* decorator_stack = NULL; /**< Contains all the map decorators. */
static int decorator_nstack       = 0; /**< Number of map decorators in the stack. */

static double map_zoom        = 1.; /**< Zoom of the map. */
static double map_xpos        = 0.; /**< Map X position. */
static double map_ypos        = 0.; /**< Map Y position. */
static int map_drag           = 0; /**< Is the user dragging the map? */
static int map_selected       = -1; /**< What system is selected on the map. */
static StarSystem **map_path  = NULL; /**< The path to current selected system. */
glTexture *gl_sysfaction_disk    = NULL; /**< Texture of the disk representing factions. */
static int cur_commod         = -1; /**< Current commodity selected. */
static StarSystem *cur_sys_sel = NULL; /**< Currently selected system */
static int cur_planet_sel = 0; /**< Current planet selected by user (0 = star). */
static Planet *cur_planetObj_sel = NULL;
static int pitch = 0; /**< pitch of planet images. */
/* VBO. */
//static gl_vbo *map_vbo = NULL; /**< Map VBO. */
//static gl_vbo *marker_vbo = NULL;

#define MAP_SYSTEM_WDWNAME "System map"
/*
 * extern
 */
/* space.c */
extern StarSystem *systems_stack;
extern int systems_nstack;
extern int faction_nstack;

/*land.c*/
extern int landed;
extern Planet* land_planet;

/*
 * prototypes
 */
/* Update. */
static void map_system_update( unsigned int wid );
/* Render. */
static void map_system_render( double bx, double by, double w, double h, void *data );
/* Mouse. */
static int map_system_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Misc. */
static glTexture *gl_genSysFactionDisk( int radius );
static int map_system_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod );
static void map_system_selectCur (void);
void map_system_show( int wid, int x, int y, int w, int h);

static void map_system_genOutfitsList( unsigned int wid );


/**
 * @brief Initializes the map subsystem.
 *
 *    @return 0 on success.
 */
int map_system_init (void)
{
   const double beta = M_PI / 9;
   GLfloat vertex[6];

   /* Create the VBO. */
   /*
   map_system_vbo = gl_vboCreateStream( sizeof(GLfloat) * 3*(2+4), NULL );

   vertex[0] = 1;
   vertex[1] = 0;
   vertex[2] = 1 + 3 * cos(beta);
   vertex[3] = 3 * sin(beta);
   vertex[4] = 1 + 3 * cos(beta);
   vertex[5] = -3 * sin(beta);
   marker_vbo = gl_vboCreateStatic( sizeof(GLfloat) * 6, vertex );
   */
   gl_sysfaction_disk = gl_genSysFactionDisk( 150. );
   
   return 0;
}

/**
 * @brief Loads all the map decorators.
 *
 *    @return 0 on success.
 */
int map_system_load (void)
{
  return 0;
}

/**
 * @brief Destroys the map subsystem.
 */
void map_system_exit (void)
{
  //int i;

   /* Destroy the VBO. */
   /*if (map_system_vbo != NULL) {
      gl_vboDestroy(map_system_vbo);
      map_system_vbo = NULL;
      }*/
   if (gl_sysfaction_disk != NULL)
      gl_freeTexture( gl_sysfaction_disk );

   /*if (decorator_stack != NULL) {
      for (i=0; i<decorator_nstack; i++)
         gl_freeTexture( decorator_stack[i].image );
      free( decorator_stack );
      decorator_stack = NULL;
      decorator_nstack = 0;
      }*/
}

/**
 * @brief Close the map window.
 */
void map_system_close( unsigned int wid, char *str ){
   if ( cur_sys_sel != cur_system ){
     printf("Unloading %s gfx\n",cur_sys_sel->name);
     space_gfxUnload( cur_sys_sel );
   }
  window_close(wid,str);

}

/**
 * @brief Handles key input to the map window.
 */
static int map_system_keyHandler( unsigned int wid, SDL_Keycode key, SDL_Keymod mod )
{
  if ((key == SDLK_m)) {
    printf("m pressed\n");
    map_system_close(window_get(MAP_SYSTEM_WDWNAME),NULL);
    //window_destroy(window_get(MAP_SYSTEM_WDWNAME));
    return 1;
  }
  /*   (void) mod;

   if ((key == SDLK_SLASH) || (key == SDLK_f)) {
      map_system_inputFind( wid, NULL );
      return 1;
      }*/

   return 0;
}

/**
 * @brief Opens the map window.
 */
void map_system_open (int sys_selected)
{
   unsigned int wid;
   int w, h, x, y, rw;

   /* Destroy window if exists. */
   wid = window_get(MAP_SYSTEM_WDWNAME);
   if (wid > 0) {
      window_destroy( wid );
      return;
   }

   /* get the selected system. */
   cur_sys_sel = system_getIndex( sys_selected );
   cur_planet_sel = 0;
   /* Set up window size. */
   w = MAX(600, SCREEN_W - 140);
   h = MAX(540, SCREEN_H - 140);

   /* create the window. */
   wid = window_create( MAP_SYSTEM_WDWNAME, -1, -1, w, h );
   window_setCancel( wid, map_system_close );
   window_handleKeys( wid, map_system_keyHandler );

   window_addText( wid, -90 + 80, y, 160, 20, 1, "txtSysname",
         &gl_defFont, &cDConsole, cur_sys_sel->name );
   window_addImage( wid, -90 + 32, y - 32, 0, 0, "imgFaction", NULL, 0 );
   /* Close button */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnClose", _("Close"), map_system_close );

   /* Load the planet gfx if necessary */
   if ( cur_sys_sel != cur_system ){
     printf("Loading %s gfx\n",cur_sys_sel->name);
     space_gfxLoad( cur_sys_sel );
   }
   map_system_show( wid, 20, -40, w-40, h-100);

   map_system_genOutfitsList( wid );

}



/**
 * @brief Checks to see if the map is open.
 *
 *    @return 0 if map is closed, non-zero if it's open.
 */
int map_system_isOpen (void)
{
   return window_exists(MAP_SYSTEM_WDWNAME);
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
   StarSystem *sys;

   /* Set zoom. */
   //map_system_setZoom(zoom);


   window_addCust( wid, x, y, w, h,
         "cstMapSys", 1, map_system_render, map_system_mouse, NULL );
}


/**
 * @brief Renders the custom solar system map widget.
 *
 *    @param bx Base X position to render at.
 *    @param by Base Y position to render at.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 */
static void map_system_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   double x,y,r;
   glColour col;
   int nshow;
   int i,j,loaded;
   double iw, ih;
   StarSystem *sys=cur_sys_sel;
   Planet *p;
   int nknown=0;
   static int phase=0;
   glColour ccol;
   char buf[1000];
   int cnt;
   int nameWidth=0;
   int textw;
   double density;
   double f;
   int hasPresence = 0;
   double unknownPresence = 0;
   char t;
   int txtHeight;
   int planetObjChanged = 0;
   cur_planetObj_sel = NULL;
   phase++;
   if( phase > 150 ) phase = 0;
   /* background */
   gl_renderRect( bx, by, w, h, &cBlack );
   
   for( i=0; i<sys->nplanets; i++){
     if( planet_isKnown(sys->planets[i]) && (sys->planets[i]->real == ASSET_REAL) )
       nknown++;
   }
   nshow = nknown+1;
   pitch=h/nshow;
   if( pitch > w/3 ) pitch = w/3;
   if( cur_planet_sel >= nshow ) cur_planet_sel = nshow-1;
   j=0;
   for( i=0; i<sys->nplanets; i++ ){
     p=sys->planets[i];
     if( planet_isKnown(p) && (p->real == ASSET_REAL) ){
       j++;
       if(p->gfx_space == NULL){
	 printf("No gfx for %s...\n",p->name);
       }else{
	 ih=pitch;
	 iw = ih;
	 if( p->gfx_space->w > p->gfx_space->h )
	   ih = ih * p->gfx_space->h / p->gfx_space->w;
	 else if( p->gfx_space->w < p->gfx_space->h )
	   iw = iw * p->gfx_space->w / p->gfx_space->h;
	 gl_blitScale(p->gfx_space, bx+2, by+(nshow-j-1)*pitch + (pitch-ih)/2, iw, ih, &cWhite);
       }
       textw = gl_printWidthRaw( &gl_smallFont, p->name );
       if( textw > nameWidth ) nameWidth = textw;
       gl_print( &gl_smallFont,bx+5+pitch, by+(nshow-j-0.5)*pitch,cur_planet_sel==j?&cGreen:&cWhite, p->name );
       if ( cur_planet_sel == j ){
          if ( cur_planetObj_sel != p )
             planetObjChanged=1;
          cur_planetObj_sel = p;
       }

       
     }
   }
   /* draw the star */
   textw = gl_printWidthRaw( &gl_smallFont, sys->name );
   if( textw > nameWidth ) nameWidth = textw;
   gl_print( &gl_smallFont,bx+5+pitch, by+(nshow-0.5)*pitch,cur_planet_sel==0?&cGreen:&cWhite, sys->name );
   /* draw marker around currently selected planet */
   ccol.r=0; ccol.g=0.6+0.4*sin(phase/150.*2*M_PI); ccol.b=0; ccol.a=1;
   ih=15;
   iw=3;
   gl_renderRect( bx+1, by+(nshow-cur_planet_sel-1)*pitch, iw, ih, &ccol );
   gl_renderRect( bx+1, by+(nshow-cur_planet_sel)*pitch-ih, iw, ih, &ccol );
   gl_renderRect( bx+pitch+3-iw, by+(nshow-cur_planet_sel-1)*pitch, iw, ih, &ccol );
   gl_renderRect( bx+pitch+3-iw, by+(nshow-cur_planet_sel)*pitch-ih, iw, ih, &ccol );
   gl_renderRect( bx+1, by+(nshow-cur_planet_sel-1)*pitch, ih, iw, &ccol );
   gl_renderRect( bx+1, by+(nshow-cur_planet_sel)*pitch-iw, ih, iw, &ccol );
   gl_renderRect( bx+pitch+3-ih, by+(nshow-cur_planet_sel-1)*pitch, ih, iw, &ccol );
   gl_renderRect( bx+pitch+3-ih, by+(nshow-cur_planet_sel)*pitch-iw, ih, iw, &ccol );
   cnt=0;
   buf[0]='\0';
   if( cur_planet_sel == 0 ){
     /* display sun information */
     //cnt+=nsnprintf(buf,sizeof(buf),"Interference: %f   Nebula density: %f   Nebula volatility: %f\n",sys->interference,sys->nebu_density,sys->nebu_volatility);
     /* Nebula. */
     cnt+=nsnprintf(buf,sizeof(buf),"System: %s\n",sys->name);
     if (sys->nebu_density > 0.) {
       /* Volatility */
       if (sys->nebu_volatility > 700.)
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Nebula: Volatile, "));
       else if (sys->nebu_volatility > 300.)
	 cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Nebula: Dangerous, "));
       else if (sys->nebu_volatility > 0.)
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Nebula: Unstable, "));
       else
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Nebula: Stable, "));
	 
       /* Density */
       if (sys->nebu_density > 700.)
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Dense\n"));
       else if (sys->nebu_density < 300.)
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Light\n"));
       else
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Medium\n"));
     }else
       cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Nebula: None\n"));

     /* Interference. */
     if (sys->interference > 0.) {
       if (sys->interference > 700.)
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Interference: Dense\n"));
       else if (sys->interference < 300.)
         cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Interference: Light\n"));
     }
     /* Asteroids. */
     if (sys->nasteroids > 0) {
       density = 0.;
       for (i=0; i<sys->nasteroids; i++) {
         density += sys->asteroids[i].area * sys->asteroids[i].density;
       }
       cnt += nsnprintf(&buf[cnt], sizeof(buf)-cnt, " Asteroid Field density %g\n",density);
     }
     /* Faction */
     f = -1;
     for (i=0; i<sys->nplanets; i++) {
       if (sys->planets[i]->real != ASSET_REAL)
         continue;
       if (!planet_isKnown(sys->planets[i]))
         continue;

       if ((f==-1) && (sys->planets[i]->faction>0)) {
         f = sys->planets[i]->faction;
       }else if (f != sys->planets[i]->faction && /** @todo more verbosity */
		 (sys->planets[i]->faction>0)) {
         cnt+=nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Faction: Multiple\n") );
         break;
       }
     }
     if (f == -1) {
       cnt+=nsnprintf(&buf[cnt], sizeof(buf)-cnt, _("Faction: N/A\n") );
     }  else {
      if (i==sys->nplanets) /* saw them all and all the same */
	cnt += nsnprintf( &buf[cnt],sizeof(buf)-cnt, "Faction: %s\nStanding: %s\n", faction_longname(f), faction_getStandingText( f ) );

     }
     /* Get presence. */
     hasPresence = 0;
     unknownPresence = 0;
     for (i=0; i < sys->npresence; i++) {
       if (sys->presence[i].value <= 0)
         continue;
       hasPresence = 1;
       if (faction_isKnown( sys->presence[i].faction )) {
         t = faction_getColourChar(sys->presence[i].faction);
         /* Use map grey instead of default neutral colour */
         cnt += nsnprintf( &buf[cnt], sizeof(buf)-cnt, "\a0%s: \a%c%.0f\n",
			   faction_shortname(sys->presence[i].faction),
			   (t=='N')?'M':t, sys->presence[i].value);
       }else
         unknownPresence += sys->presence[i].value;
     }
     if (unknownPresence != 0)
       cnt += nsnprintf( &buf[cnt], sizeof(buf)-cnt, "\a0Unknown: \a%c%.0f\n",
			 'M', unknownPresence);
     if (hasPresence == 0)
       cnt += nsnprintf( &buf[cnt], sizeof(buf)-cnt, "Presence: N/A\n");
     txtHeight=gl_printHeight(&gl_smallFont,(w - nameWidth-pitch-5)/2,buf);
     gl_printText( &gl_smallFont, (w - nameWidth-pitch-5)/2, txtHeight, bx+10+pitch+nameWidth, by + h - 10-txtHeight, &cWhite, buf);

     


   /* Jumps. */
     //JumpPoint *jumps; /**< Jump points in the system */
     //int njumps; /**< number of adjacent jumps */
     //jp_isKnown(j)     jp_isFlag(j,JP_KNOWN) /**< Checks if jump is known. */
     //typedef struct JumpPoint_ {
     //StarSystem *target; /**< Target star system to jump to. */
     //unsigned int flags; /**< Flags related to the jump point's status. */
     //} JumpPoint;


     

   }else{
     /* display planet info */
     p=cur_planetObj_sel;
     cnt+=nsnprintf(&buf[cnt],sizeof(buf)-cnt,"Planet: %s\nPopulation: %ld\n%s",p->name,p->population,p->description);
     
     txtHeight=gl_printHeight(&gl_smallFont,(w - nameWidth-pitch-5)/2,buf);


   /*
     Vector2d pos; //< position in star system 
   double radius; //< Radius of the planet. 

     //Planet details. 
   char *class; //< Planet type. Uses Star Trek classification system (https://stexpanded.fandom.com/wiki/Planet_classifications) 
   int faction; //< planet faction 
   uint64_t population; //< Population of the planet. 

   // Asset details. 
   double presenceAmount; //< The amount of presence this asset exerts. 
   int presenceRange; //< The range of presence exertion of this asset. 
   int real; //< If the asset is tangible or not. 
   double hide; //< The ewarfare hide value for an asset. 

   // Landing details. 
   int land_override; //< Forcibly allows the player to either be able to land or not (+1 is land, -1 is not, 0 otherwise). 
   char *land_func; //< Landing function to execute. 
   int can_land; //< Whether or not the player can land. 
   char *land_msg; //< Message on landing. 
   credits_t bribe_price; //< Cost of bribing. 
   char *bribe_msg; //< Bribe message. 
   char *bribe_ack_msg; //< Bribe ACK message. 
   int bribed; //< If planet has been bribed. 

   // Landed details. 
   char* description; //< planet description 
   char* bar_description; //< spaceport bar description 
   unsigned int services; //< what services they offer 
   Commodity **commodities; //< what commodities they sell 
   CommodityPrice *commodityPrice; //< the base cost of a commodity on this planet 
   int ncommodities; //< the amount they have 
   tech_group_t *tech; //< Planet tech. 
   char *gfx_exterior; //< Don't actually load the texture 
   char *gfx_exteriorPath; //< Name of the gfx_exterior for saving purposes. 

#define PLANET_SERVICE_LAND         (1<<0) //< Can land. 
#define PLANET_SERVICE_INHABITED    (1<<1) //< Planet is inhabited. 
#define PLANET_SERVICE_REFUEL       (1<<2) //< Has refueling. 
#define PLANET_SERVICE_BAR          (1<<3) //< Has bar and thus news. 
#define PLANET_SERVICE_MISSIONS     (1<<4) //< Has mission computer. 
#define PLANET_SERVICE_COMMODITY    (1<<5) //< Can trade commodities. 
#define PLANET_SERVICE_OUTFITS      (1<<6) //< Can trade outfits. 
#define PLANET_SERVICE_SHIPYARD     (1<<7) //< Can trade ships. 
#define PLANET_SERVICE_BLACKMARKET  (1<<8) //< Disables license restrictions on goods. 
#define PLANET_SERVICES_MAX         (PLANET_SERVICE_BLACKMARKET<<1)
#define planet_hasService(p,s)      ((p)->services & s) //< Checks if planet has service. 
   */
   

     gl_printText( &gl_smallFont, (w - nameWidth-pitch-5)/2, txtHeight, bx+10+pitch+nameWidth, by + h - 10-txtHeight, &cWhite, buf);
   }
   if ( planetObjChanged ){
      map_system_genOutfitsList( wid );

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
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i;
   double x,y, t;
   StarSystem *sys;
   switch (event->type) {
   case SDL_MOUSEBUTTONDOWN:
     /* Must be in bounds. */
     printf("mouse down: %g %g\n",mx,my);
     if ((mx < 0.) || (mx > w) || (my < 0.) || (my > h))
            return 0;
     if (mx < pitch && my > 0){
       cur_planet_sel = ( h-my ) / pitch  ;
       return 1;
     }
   }
   return 0;
}

/**
 * @brief Generates a texture to represent factions
 *
 * @param radius radius of the disk
 * @return the texture
 */
static glTexture *gl_genSysFactionDisk( int radius )
{
   int i, j;
   uint8_t *pixels;
   SDL_Surface *sur;
   int dist;
   double alpha;

   /* Calculate parameters. */
   const int w = 2 * radius + 1;
   const int h = 2 * radius + 1;

   /* Create the surface. */
   sur = SDL_CreateRGBSurface( 0, w, h, 32, RGBAMASK );

   pixels = sur->pixels;
   memset(pixels, 0xff, sizeof(uint8_t) * 4 * h * w);

   /* Generate the circle. */
   SDL_LockSurface( sur );

   /* Draw the circle with filter. */
   for (i=0; i<h; i++) {
      for (j=0; j<w; j++) {
         /* Calculate blur. */
         dist = (i - radius) * (i - radius) + (j - radius) * (j - radius);
         alpha = 0.;

         if (dist < radius * radius) {
            /* Computes alpha with an empirically chosen formula.
             * This formula accounts for the fact that the eyes
             * has a logarithmic sensitivity to light */
            alpha = 1. * dist / (radius * radius);
            alpha = (exp(1 / (alpha + 1) - 0.5) - 1) * 0xFF;
         }

         /* Sets the pixel alpha which is the forth byte
          * in the pixel representation. */
         pixels[i*sur->pitch + j*4 + 3] = (uint8_t)alpha;
      }
   }

   SDL_UnlockSurface( sur );

   /* Return texture. */
   return gl_loadImage( sur, OPENGL_TEX_MIPMAPS );
}

void map_system_outfits_update( unsigned int wid, char* str ){
   printf("map_system_outfits_update: %s\n",str);
}
static void map_system_outfits_rmouse( unsigned int wid, char* widget_name )
{
   printf("map_system_outfits_rmouse: %s\n",widget_name);
}


/**
 * @brief Generates the outfit list.
 *
 *    @param wid Window to generate the list on.
 */
#define MAPSYS_OUTFITS "mapSysOutfits"
static void map_system_genOutfitsList( unsigned int wid )
{
   int i, active, owned, len;
   int fx, fy, fw, fh, barw; /* Input filter. */
   Outfit **outfits;
   char **soutfits, **slottype, **quantity;
   glTexture **toutfits;
   int noutfits, moutfits;
   int w, h, iw, ih;
   glColour *bg, blend;
   const glColour *c;
   const char *slotname;
   char *filtertext;
   int no_outfits = 0;

   printf("cur_planet_sel in genOutfitsList: %d %p\n",cur_planet_sel,cur_planetObj_sel);
   /* Widget must not already exist. */
   if (widget_exists( wid, MAPSYS_OUTFITS))
      return;

   /* set up the outfits to buy/sell */
   if ( cur_planetObj_sel == NULL ){
      noutfits = 0;
      outfits = NULL;
   }else{
      outfits = tech_getOutfit( cur_planetObj_sel->tech, &noutfits );
   }

   
   moutfits = MAX( 1, noutfits );
   soutfits = malloc( moutfits * sizeof(char*) );
   toutfits = malloc( moutfits * sizeof(glTexture*) );

   if (noutfits <= 0) { /* No outfits */
      soutfits[0] = strdup(_("None"));
      toutfits[0] = NULL;
      noutfits    = 1;
      no_outfits  = 1;
   }
   else {
      /* Create the outfit arrays. */
      quantity = malloc(sizeof(char*)*noutfits);
      bg       = malloc(sizeof(glColour)*noutfits);
      slottype = malloc(sizeof(char*)*noutfits);
      for (i=0; i<noutfits; i++) {
         soutfits[i] = strdup( outfits[i]->name );
	 toutfits[i] = outfits[i]->gfx_store;
         /* Background colour. */
         c = outfit_slotSizeColour( &outfits[i]->slot );
         if (c == NULL)
            c = &cBlack;
         col_blend( &blend, c, &cGrey70, 0.4 );
         bg[i] = blend;

         /* Quantity. */
         owned = player_outfitOwned(outfits[i]);
         len = owned / 10 + 4;
         if (owned >= 1) {
            quantity[i] = malloc( len );
            nsnprintf( quantity[i], len, "%d", owned );
         }
         else
            quantity[i] = NULL;


         /* Get slot name. */
         slotname = outfit_slotName(outfits[i]);
         if ((strcmp(slotname,"NA") != 0) && (strcmp(slotname,"NULL") != 0)) {
            slottype[i]    = malloc( 2 );
            slottype[i][0] = outfit_slotName(outfits[i])[0];
            slottype[i][1] = '\0';
         }
         else
            slottype[i] = NULL;
      }
   }

   /* Clean up. */
   free(outfits);

   window_addImageArray( wid, 200, 70,
         400, 240, MAPSYS_OUTFITS, 64, 64,
         toutfits, soutfits, noutfits, map_system_outfits_update, map_system_outfits_rmouse );

   if (!no_outfits) {
      toolkit_setImageArrayQuantity( wid, MAPSYS_OUTFITS, quantity );
      toolkit_setImageArraySlotType( wid, MAPSYS_OUTFITS, slottype );
      toolkit_setImageArrayBackground( wid, MAPSYS_OUTFITS, bg );
   }
}
