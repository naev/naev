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
#include "dev_planet.h"
#include "dev_system.h"
#include "unidiff.h"
#include "dialogue.h"
#include "tk/toolkit_priv.h"
#include "ndata.h"
#include "nfile.h"
#include "nstring.h"


#define EDITOR_WDWNAME  "Planet Property Editor"

#define HIDE_DEFAULT_PLANET      0.25 /**< Default hide value for new planets. */

#define BUTTON_WIDTH    90 /**< Map button width. */
#define BUTTON_HEIGHT   30 /**< Map button height. */


#define SYSEDIT_EDIT_WIDTH       500 /**< System editor width. */
#define SYSEDIT_EDIT_HEIGHT      400 /**< System editor height. */


#define SYSEDIT_DRAG_THRESHOLD   300   /**< Drag threshold. */
#define SYSEDIT_MOVE_THRESHOLD   10    /**< Movement threshold. */

#define SYSEDIT_ZOOM_STEP        1.2   /**< Factor to zoom by for each zoom level. */
#define SYSEDIT_ZOOM_MAX         1     /**< Maximum zoom level (close). */
#define SYSEDIT_ZOOM_MIN         -23   /**< Minimum zoom level (far). */


#define PLANET_SPACE_GFX_PATH    "gfx/planet/space" /**< Path to planet space graphics. */
#define PLANET_LAND_GFX_PATH     "gfx/planet/exterior" /**< Path to planet landing graphics. */


#define PLANET_GFX_SPACE      "gfx/planet/space/" /**< Location of planet space graphics. */
#define PLANET_GFX_EXTERIOR   "gfx/planet/exterior/" /**< Location of planet exterior graphics (when landed). */


/*
 * Selection types.
 */
#define SELECT_NONE        0 /**< No selection. */
#define SELECT_PLANET      1 /**< Selection is a planet. */
#define SELECT_JUMPPOINT   2 /**< Selection is a jump point. */


/**
 * @brief Selection generic for stuff in a system.
 */
typedef struct Select_s {
   int type; /**< Type of selection. */
   union {
      int planet;
      int jump;
   } u; /**< Data itself. */
} Select_t;
static Select_t *sysedit_select  = NULL; /**< Current system selection. */
static int sysedit_nselect       = 0; /**< Number of selections in current system. */
static int sysedit_mselect       = 0; /**< Memory allocated for selections. */
static Select_t sysedit_tsel;         /**< Temporary selection. */
static int  sysedit_tadd         = 0; /**< Add to selection. */


/*
 * System editor stuff.
 */
static StarSystem *sysedit_sys = NULL; /**< Currently opened system. */
static unsigned int sysedit_wid = 0; /**< Sysedit wid. */
static unsigned int sysedit_widEdit = 0; /**< Planet editor wid. */
static int sysedit_grid       = 1;  /**< Grid is visible. */
static double sysedit_xpos    = 0.; /**< Viewport X position. */
static double sysedit_ypos    = 0.; /**< Viewport Y position. */
static double sysedit_zoom    = 1.; /**< Viewport zoom level. */
static int sysedit_moved      = 0;  /**< Space moved since mouse down. */
static unsigned int sysedit_dragTime = 0; /**< Tick last started to drag. */
static int sysedit_drag       = 0;  /**< Dragging viewport around. */
static int sysedit_dragSel    = 0;  /**< Dragging system around. */
static double sysedit_mx      = 0.; /**< Cursor X position. */
static double sysedit_my      = 0.; /**< Cursor Y position. */

/* Stored checkbox values. */
static int jp_hidden = 0; /**< Jump point hidden checkbox value. */
static int jp_exit   = 0; /**< Jump point exit only checkbox value. */

/*
 * Property editor stuff.
 */
static glTexture **sysedit_tex = NULL; /**< Planet textures. */
static int sysedit_ntex       = 0; /**< Number of planet textures. */


/*
 * System editor Prototypes.
 */
/* Custom system editor widget. */
static void sysedit_buttonZoom( unsigned int wid, char* str );
static void sysedit_render( double bx, double by, double w, double h, void *data );
static void sysedit_renderBG( double bx, double bw, double w, double h, double x, double y);
static void sysedit_renderSprite( glTexture *gfx, double bx, double by, double x, double y,
      int sx, int sy, const glColour *c, int selected, const char *caption );
static void sysedit_renderOverlay( double bx, double by, double bw, double bh, void* data );
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data );
/* Button functions. */
static void sysedit_close( unsigned int wid, char *wgt );
static void sysedit_btnNew( unsigned int wid_unused, char *unused );
static void sysedit_btnRename( unsigned int wid_unused, char *unused );
static void sysedit_btnRemove( unsigned int wid_unused, char *unused );
static void sysedit_btnReset( unsigned int wid_unused, char *unused );
static void sysedit_btnScale( unsigned int wid_unused, char *unused );
static void sysedit_btnGrid( unsigned int wid_unused, char *unused );
static void sysedit_btnEdit( unsigned int wid_unused, char *unused );
/* Planet editing. */
static void sysedit_editPnt( void );
static void sysedit_editPntClose( unsigned int wid, char *unused );
static void sysedit_planetDesc( unsigned int wid, char *unused );
static void sysedit_planetDescClose( unsigned int wid, char *unused );
static void sysedit_genServicesList( unsigned int wid );
static void sysedit_btnTechEdit( unsigned int wid, char *unused );
static void sysedit_genTechList( unsigned int wid );
static void sysedit_btnAddTech( unsigned int wid, char *unused );
static void sysedit_btnRmTech( unsigned int wid, char *unused );
static void sysedit_btnAddService( unsigned int wid, char *unused );
static void sysedit_btnRmService( unsigned int wid, char *unused );
static void sysedit_planetGFX( unsigned int wid_unused, char *unused );
static void sysedit_btnGFXClose( unsigned int wid, char *wgt );
static void sysedit_btnGFXApply( unsigned int wid, char *wgt );
static void sysedit_btnFaction( unsigned int wid_unused, char *unused );
static void sysedit_btnFactionSet( unsigned int wid, char *unused );
/* Jump editing */
static void sysedit_editJump( void );
/* Keybindings handling. */
static int sysedit_keys( unsigned int wid, SDLKey key, SDLMod mod );
/* Selection. */
static int sysedit_selectCmp( Select_t *a, Select_t *b );
static void sysedit_checkButtons (void);
static void sysedit_deselect (void);
static void sysedit_selectAdd( Select_t *sel );
static void sysedit_selectRm( Select_t *sel );


/**
 * @brief Opens the system editor interface.
 */
void sysedit_open( StarSystem *sys )
{
   unsigned int wid;
   char buf[PATH_MAX];
   int i;

   /* Reconstructs the jumps - just in case. */
   systems_reconstructJumps();

   /* Reset some variables. */
   sysedit_sys    = sys;
   sysedit_drag   = 0;
   sysedit_zoom   = pow(SYSEDIT_ZOOM_STEP, SYSEDIT_ZOOM_MIN);
   sysedit_xpos   = 0.;
   sysedit_ypos   = 0.;

   /* Load graphics. */
   space_gfxLoad( sysedit_sys );

   /* Create the window. */
   nsnprintf( buf, sizeof(buf), "%s - Star System Editor", sys->name );
   wid = window_create( buf, -1, -1, -1, -1 );
   window_handleKeys( wid, sysedit_keys );
   sysedit_wid = wid;

   /* Close button. */
   window_addButton( wid, -15, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", sysedit_close );
   i = 1;

   /* Scale. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnScale", "Scale", sysedit_btnScale );
   i += 1;

   /* Reset. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnReset", "Reset Jumps", sysedit_btnReset );
   i += 1;

   /* Editing. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnEdit", "Edit", sysedit_btnEdit );
   i += 1;

   /* Remove. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRemove", "Remove", sysedit_btnRemove );
   i += 1;

   /* Rename. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRename", "Rename", sysedit_btnRename );
   i += 1;

   /* New system. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", "New Planet", sysedit_btnNew );
   i += 2;

   /* Toggle Grid. */
   window_addButton( wid, -15, 20+(BUTTON_HEIGHT+20)*i, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnGrid", "Grid", sysedit_btnGrid );

   /* Zoom buttons */
   window_addButton( wid, 40, 20, 30, 30, "btnZoomIn", "+", sysedit_buttonZoom );
   window_addButton( wid, 80, 20, 30, 30, "btnZoomOut", "-", sysedit_buttonZoom );

   /* Selected text. */
   nsnprintf( buf, sizeof(buf), "Radius: %.0f", sys->radius );
   window_addText( wid, 140, 10, SCREEN_W - 80 - 30 - 30 - BUTTON_WIDTH - 20, 30, 0,
         "txtSelected", &gl_smallFont, &cBlack, buf );

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

      default:
         return 0;
   }
}


/**
 * @brief Closes the system editor widget.
 */
static void sysedit_close( unsigned int wid, char *wgt )
{
   /* Unload graphics. */
   space_gfxLoad( sysedit_sys );

   /* Remove selection. */
   sysedit_deselect();

   /* Set the dominant faction. */
   system_setFaction( sysedit_sys );

   /* Save the system */
   dsys_saveSystem( sysedit_sys );

   /* Close the window. */
   window_close( wid, wgt );
}


/**
 * @brief Closes the planet editor, saving the changes made.
 */
static void sysedit_editPntClose( unsigned int wid, char *unused )
{
   (void) unused;
   Planet *p;
   char *inp;

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];

   /* Remove the old presence. */
   system_addPresence(sysedit_sys, p->faction, -p->presenceAmount, p->presenceRange);

   p->population     = (uint64_t)strtoull( window_getInput( sysedit_widEdit, "inpPop" ), 0, 10);
   p->class          = planetclass_get( window_getInput( sysedit_widEdit, "inpClass" )[0] );
   inp               = window_getInput( sysedit_widEdit, "inpLand" );
   if ((inp == NULL) || (strlen(inp) == 0)) {
      free( p->land_func );
      p->land_func = NULL;
   }
   else
      p->land_func = strdup( inp );
   p->presenceAmount = atof(window_getInput( sysedit_widEdit, "inpPresence" ));
   p->presenceRange  = atoi(window_getInput( sysedit_widEdit, "inpPresenceRange" ));
   p->hide           = pow2( atof(window_getInput( sysedit_widEdit, "inpHide" )) );

   /* Add the new presence. */
   system_addPresence(sysedit_sys, p->faction, p->presenceAmount, p->presenceRange);

   dpl_savePlanet( p );

   window_close( wid, unused );
}

/**
 * @brief Closes the jump editor, saving the changes made.
 */
static void sysedit_editJumpClose( unsigned int wid, char *unused )
{
   (void) unused;
   JumpPoint *j;

   j = &sysedit_sys->jumps[ sysedit_select[0].u.jump ];
   if (jp_hidden == 1) {
      jp_setFlag( j, JP_HIDDEN );
      jp_rmFlag(  j, JP_EXITONLY );
   }
   else if (jp_exit == 1) {
      jp_setFlag( j, JP_EXITONLY );
      jp_rmFlag(  j, JP_HIDDEN );
   }
   else {
      jp_rmFlag( j, JP_HIDDEN );
      jp_rmFlag( j, JP_EXITONLY );
   }
   j->hide  = pow2( atof(window_getInput( sysedit_widEdit, "inpHide" )) );

   window_close( wid, unused );
}

/**
 * @brief Enters the editor in new system mode.
 */
static void sysedit_btnNew( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   Planet *p, *b;
   char *name;

   /* Get new name. */
   name = dialogue_inputRaw( "New Planet Creation", 1, 32, "What do you want to name the new planet?" );
   if (name == NULL)
      return;

   /* Check for collision. */
   if (planet_exists( name )) {
      dialogue_alert( "Planet by the name of \er'%s'\e0 already exists in the \er'%s'\e0 system",
            name, planet_getSystem( name ) );
      free(name);
      sysedit_btnNew( 0, NULL );
      return;
   }

   /* Create the new planet. */
   p        = planet_new();
   p->real  = ASSET_REAL;
   p->name  = name;

   /* Base planet data off another. */
   b                    = planet_get( space_getRndPlanet(0) );
   p->gfx_spacePath     = strdup( b->gfx_spacePath );
   p->gfx_spaceName     = strdup( b->gfx_spaceName );
   p->gfx_exterior      = strdup( b->gfx_exterior );
   p->gfx_exteriorPath  = strdup( b->gfx_exteriorPath );
   p->pos.x             = sysedit_xpos / sysedit_zoom;
   p->pos.y             = sysedit_ypos / sysedit_zoom;
   p->hide              = pow2(HIDE_DEFAULT_PLANET);

   /* Add new planet. */
   system_addPlanet( sysedit_sys, name );

   /* Reload graphics. */
   space_gfxLoad( sysedit_sys );
}


static void sysedit_btnRename( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   int i;
   char *name, *oldName, *newName;
   Select_t *sel;
   Planet *p;
   for (i=0; i<sysedit_nselect; i++) {
      sel = &sysedit_select[i];
      if (sel->type == SELECT_PLANET) {
         p = sysedit_sys[i].planets[ sel->u.planet ];

         /* Get new name. */
         name = dialogue_input( "New Planet Creation", 1, 32,
               "What do you want to rename the planet \er%s\e0?", p->name );
         if (name == NULL)
            continue;

         /* Check for collision. */
         if (planet_exists( name )) {
            dialogue_alert( "Planet by the name of \er'%s'\e0 already exists in the \er'%s'\e0 system",
                  name, planet_getSystem( name ) );
            free(name);
            continue;
         }

         /* Rename. */
         oldName = malloc((16+strlen(p->name))*sizeof(char));
         nsnprintf(oldName,15+strlen(p->name),"dat/assets/%s.xml",p->name);
         newName = malloc((16+strlen(name))*sizeof(char));
         nsnprintf(newName,15+strlen(name),"dat/assets/%s.xml",name);
         nfile_rename(oldName,newName);
         free(oldName);
         free(newName);
         free(p->name);
         p->name = name;
         window_modifyText( sysedit_widEdit, "txtName", p->name );
         dpl_savePlanet( p );
      }
   }
}


/**
 * @brief Removes planets.
 */
static void sysedit_btnRemove( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   Select_t *sel;
   char *file;
   int i;

   if (dialogue_YesNo( "Remove selected planets?", "This can not be undone." )) {
      for (i=0; i<sysedit_nselect; i++) {
         sel = &sysedit_select[i];
         if (sel->type == SELECT_PLANET) {
            file = malloc((16+strlen(sysedit_sys->planets[ sel->u.planet ]->name))*sizeof(char));
            nsnprintf(file,(16+strlen(sysedit_sys->planets[ sel->u.planet ]->name))*sizeof(char),
                           "dat/assets/%s.xml",sysedit_sys->planets[ sel->u.planet ]->name);
            nfile_delete(file);
            system_rmPlanet( sysedit_sys, sysedit_sys->planets[ sel->u.planet ]->name );
         }
      }
   }
}


/**
 * @brief Resets jump points.
 */
static void sysedit_btnReset( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   Select_t *sel;
   int i;
   for (i=0; i<sysedit_nselect; i++) {
      sel = &sysedit_select[i];
      if (sel->type == SELECT_JUMPPOINT)
         sysedit_sys[i].jumps[ sel->u.jump ].flags |= JP_AUTOPOS;
   }

   /* Must reconstruct jumps. */
   systems_reconstructJumps();
}


/**
 * @brief Interface for scaling a system from the system view.
 */
static void sysedit_btnScale( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   char *str;
   double s;
   int i;
   StarSystem *sys;

   /* Prompt scale amount. */
   str = dialogue_inputRaw( "Scale Star System", 1, 32, "By how much do you want to scale the star system?" );
   if (str == NULL)
      return;

   sys   = sysedit_sys; /* Comfort. */
   s     = atof(str);

   /* In case screwed up. */
   if ((s < 0.1) || (s > 10.)) {
      i = dialogue_YesNo( "Scale Star System", "Are you sure you want to scale the star system by %.2f (from %.2f to %.2f)?",
            s, sys->radius, sys->radius*s );
      if (i==0)
         return;
   }

   sysedit_sysScale(sys, s);
}

/**
 * @brief Scales a system.
 */
void sysedit_sysScale( StarSystem *sys, double factor )
{
   char buf[PATH_MAX];
   Planet *p;
   JumpPoint *jp;
   int i;

   /* Scale radius. */
   sys->radius *= factor;
   nsnprintf( buf, sizeof(buf), "Radius: %.0f", sys->radius );
   window_modifyText( sysedit_wid, "txtSelected", buf );

   /* Scale planets. */
   for (i=0; i<sys->nplanets; i++) {
      p     = sys->planets[i];
      vect_cset( &p->pos, p->pos.x*factor, p->pos.y*factor );
   }

   /* Scale jumps. */
   for (i=0; i<sys->njumps; i++) {
      jp    = &sys->jumps[i];
      vect_cset( &jp->pos, jp->pos.x*factor, jp->pos.y*factor );
   }

   /* Must reconstruct jumps. */
   systems_reconstructJumps();
}

/**
 * @brief Toggles the grid.
 */
static void sysedit_btnGrid( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;

   sysedit_grid = !sysedit_grid;
}


/**
 * @brief System editor custom widget rendering.
 */
static void sysedit_render( double bx, double by, double w, double h, void *data )
{
   (void) data;
   int i, j;
   StarSystem *sys;
   Planet *p;
   JumpPoint *jp;
   double x,y, z;
   const glColour *c;
   int selected;
   Select_t sel;

   /* Comfort++. */
   sys   = sysedit_sys;
   z     = sysedit_zoom;

   /* Coordinate translation. */
   x = bx - sysedit_xpos + w/2;
   y = by - sysedit_ypos + h/2;

   /* First render background with lines. */
   sysedit_renderBG( bx, by, w, h, x, y );

   /* Render planets. */
   for (i=0; i<sys->nplanets; i++) {
      p              = sys->planets[i];

      /* Must be real. */
      if (p->real != ASSET_REAL)
         continue;

      /* Check if selected. */
      sel.type       = SELECT_PLANET;
      sel.u.planet   = i;
      selected       = 0;
      for (j=0; j<sysedit_nselect; j++) {
         if (sysedit_selectCmp( &sel, &sysedit_select[j] )) {
            selected = 1;
            break;
         }
      }

      /* Render. */
      sysedit_renderSprite( p->gfx_space, x, y, p->pos.x, p->pos.y, 0, 0, NULL, selected, p->name );
   }

   /* Render jump points. */
   for (i=0; i<sys->njumps; i++) {
      jp    = &sys->jumps[i];

      /* Choose colour. */
      if (jp->flags & JP_AUTOPOS)
         c = &cGreen;
      else
         c = NULL;

      /* Check if selected. */
      sel.type       = SELECT_JUMPPOINT;
      sel.u.planet   = i;
      selected       = 0;
      for (j=0; j<sysedit_nselect; j++) {
         if (sysedit_selectCmp( &sel, &sysedit_select[j] )) {
            selected = 1;
            break;
         }
      }

      /* Render. */
      sysedit_renderSprite( jumppoint_gfx, x, y, jp->pos.x, jp->pos.y,
            jp->sx, jp->sy, c, selected, jp->target->name );
   }

   /* Render cursor position. */
   gl_print( &gl_smallFont, bx + 5., by + 5.,
         &cWhite, "%.2f, %.2f",
         (bx + sysedit_mx - x)/z,
         (by + sysedit_my - y)/z );
}


/**
 * @brief Renders the custom widget background.
 */
static void sysedit_renderBG( double bx, double by, double w, double h, double x, double y )
{
   double z, s;
   double sx, sy, sz;

   /* Render blackness. */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Must have grid activated. */
   if (!sysedit_grid)
      return;

   /* Comfort. */
   z  = sysedit_zoom;
   s  = 1000.;

   /* Draw lines that go through 0,0 */
   gl_renderRect( x-1., by, 3., h, &cLightBlue );
   gl_renderRect( bx, y-1., w, 3., &cLightBlue );

   /* Render lines. */
   sz    = s*z;
   sx    = w/2. - fmod( sysedit_xpos, sz ) - sz*round( w/2. / sz );
   sy    = h/2. - fmod( sysedit_ypos, sz ) - sz*round( h/2. / sz );
   /* Vertical. */
   for (   ; sx<w; sx += sz)
      gl_renderRect( bx+sx, by, 1., h, &cBlue );
   /* Horizontal. */
   for (   ; sy<w; sy += sz)
      gl_renderRect( bx, by+sy, w, 1., &cBlue );
}


/**
 * @brief Renders a sprite for the custom widget.
 */
static void sysedit_renderSprite( glTexture *gfx, double bx, double by, double x, double y,
      int sx, int sy, const glColour *c, int selected, const char *caption )
{
   double tx, ty, z;
   glColour cc;
   const glColour *col;

   /* Comfort. */
   z  = sysedit_zoom;

   /* Translate coords. */
   tx = bx + (x - gfx->sw/2.)*z;
   ty = by + (y - gfx->sh/2.)*z;

   /* Selection graphic. */
   if (selected) {
      cc.r = cFontBlue.r;
      cc.g = cFontBlue.g;
      cc.b = cFontBlue.b;
      cc.a = 0.5;
      gl_drawCircle( bx + x*z, by + y*z, gfx->sw*z*1.1, &cc, 1 );
   }

   /* Blit the planet. */
   gl_blitScaleSprite( gfx, tx, ty, sx, sy, gfx->sw*z, gfx->sh*z, c );

   /* Display caption. */
   if (caption != NULL) {
      if (selected)
         col = &cRed;
      else
         col = c;
      gl_printMidRaw( &gl_smallFont, gfx->sw*z+100,
            tx - 50, ty - gl_smallFont.h - 5, col, caption );
   }
}


/**
 * @brief Renders the overlay.
 */
static void sysedit_renderOverlay( double bx, double by, double bw, double bh, void* data )
{
   (void) bx;
   (void) by;
   (void) bw;
   (void) bh;
   (void) data;
}

/**
 * @brief System editor custom widget mouse handling.
 */
static void sysedit_mouse( unsigned int wid, SDL_Event* event, double mx, double my,
      double w, double h, void *data )
{
   (void) wid;
   (void) data;
   int i, j;
   double x,y, t;
   SDLMod mod;
   StarSystem *sys;
   Planet *p;
   JumpPoint *jp;
   Select_t sel;

   /* Comfort. */
   sys = sysedit_sys;

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


            /* Check planets. */
            for (i=0; i<sys->nplanets; i++) {
               p = sys->planets[i];

               /* Must be real. */
               if (p->real != ASSET_REAL)
                  continue;

               /* Position. */
               x = p->pos.x * sysedit_zoom;
               y = p->pos.y * sysedit_zoom;

               /* Set selection. */
               sel.type       = SELECT_PLANET;
               sel.u.planet   = i;

               /* Threshold. */
               t  = p->gfx_space->sw * p->gfx_space->sh / 4.; /* Radius^2 */
               t *= pow2(2.*sysedit_zoom);

               /* Can select. */
               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* Check if already selected. */
                  for (j=0; j<sysedit_nselect; j++) {
                     if (sysedit_selectCmp( &sel, &sysedit_select[j] )) {
                        sysedit_dragSel   = 1;
                        memcpy( &sysedit_tsel, &sel, sizeof(Select_t) );

                        /* Check modifier. */
                        if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                           sysedit_tadd      = 0;
                        else {
                           /* Detect double click to open planet editor. */
                           if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD*2)
                                 && (sysedit_moved < SYSEDIT_MOVE_THRESHOLD)) {
                              sysedit_editPnt();
                              sysedit_dragSel = 0;
                              return;
                           }
                           sysedit_tadd      = -1;
                        }
                        sysedit_dragTime  = SDL_GetTicks();
                        sysedit_moved     = 0;
                        return;
                     }
                  }

                  /* Add the system if not selected. */
                  if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                     sysedit_selectAdd( &sel );
                  else {
                     sysedit_deselect();
                     sysedit_selectAdd( &sel );
                  }
                  sysedit_tsel.type = SELECT_NONE;

                  /* Start dragging anyway. */
                  sysedit_dragSel   = 1;
                  sysedit_dragTime  = SDL_GetTicks();
                  sysedit_moved     = 0;
                  return;
               }
            }

            /* Check jump points. */
            for (i=0; i<sys->njumps; i++) {
               jp = &sys->jumps[i];

               /* Position. */
               x = jp->pos.x * sysedit_zoom;
               y = jp->pos.y * sysedit_zoom;

               /* Set selection. */
               sel.type       = SELECT_JUMPPOINT;
               sel.u.planet   = i;

               /* Threshold. */
               t  = jumppoint_gfx->sw * jumppoint_gfx->sh / 4.; /* Radius^2 */
               t *= pow2(2.*sysedit_zoom);

               /* Can select. */
               if ((pow2(mx-x)+pow2(my-y)) < t) {

                  /* Check if already selected. */
                  for (j=0; j<sysedit_nselect; j++) {
                     if (sysedit_selectCmp( &sel, &sysedit_select[j] )) {
                        sysedit_dragSel   = 1;
                        memcpy( &sysedit_tsel, &sel, sizeof(Select_t) );

                        /* Check modifier. */
                        if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                           sysedit_tadd      = 0;
                        else {
                           /* Detect double click to open planet editor. */
                           if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD*2)
                                 && (sysedit_moved < SYSEDIT_MOVE_THRESHOLD)) {
                              sysedit_editJump();
                              sysedit_dragSel = 0;
                              return;
                           }
                           sysedit_tadd      = -1;
                        }
                        sysedit_dragTime  = SDL_GetTicks();
                        sysedit_moved     = 0;
                        return;
                     }
                  }

                  /* Add the system if not selected. */
                  if (mod & (KMOD_LCTRL | KMOD_RCTRL))
                     sysedit_selectAdd( &sel );
                  else {
                     sysedit_deselect();
                     sysedit_selectAdd( &sel );
                  }
                  sysedit_tsel.type = SELECT_NONE;

                  /* Start dragging anyway. */
                  sysedit_dragSel   = 1;
                  sysedit_dragTime  = SDL_GetTicks();
                  sysedit_moved     = 0;
                  return;
               }
            }

            /* Start dragging. */
            if (!(mod & (KMOD_LCTRL | KMOD_RCTRL))) {
               sysedit_drag      = 1;
               sysedit_dragTime  = SDL_GetTicks();
               sysedit_moved     = 0;
               sysedit_tsel.type = SELECT_NONE;
            }
            return;
         }
         break;

      case SDL_MOUSEBUTTONUP:
         if (sysedit_drag) {
            if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD) && (sysedit_moved < SYSEDIT_MOVE_THRESHOLD)) {
               if (sysedit_tsel.type == SELECT_NONE)
                  sysedit_deselect();
               else
                  sysedit_selectAdd( &sysedit_tsel );
            }
            sysedit_drag      = 0;
            for (i=0; i<sysedit_nselect; i++) {
               dpl_savePlanet(sysedit_sys->planets[ sysedit_select[i].u.planet ]);
            }
         }
         if (sysedit_dragSel) {
            if ((SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD) &&
                  (sysedit_moved < SYSEDIT_MOVE_THRESHOLD) && (sysedit_tsel.type != SELECT_NONE)) {
               if (sysedit_tadd == 0)
                  sysedit_selectRm( &sysedit_tsel );
               else {
                  sysedit_deselect();
                  sysedit_selectAdd( &sysedit_tsel );
               }
            }
            sysedit_dragSel   = 0;
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

            /* Update mouse movement. */
            sysedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
         }
         /* Dragging selection around. */
         else if (sysedit_dragSel && (sysedit_nselect > 0)) {
            if ((sysedit_moved > SYSEDIT_MOVE_THRESHOLD) || (SDL_GetTicks() - sysedit_dragTime > SYSEDIT_DRAG_THRESHOLD)) {
               for (i=0; i<sysedit_nselect; i++) {

                  /* Planets. */
                  if (sysedit_select[i].type == SELECT_PLANET) {
                     p = sys->planets[ sysedit_select[i].u.planet ];
                     p->pos.x += ((double)event->motion.xrel) / sysedit_zoom;
                     p->pos.y -= ((double)event->motion.yrel) / sysedit_zoom;
                  }
                  /* Jump point. */
                  else if (sysedit_select[i].type == SELECT_JUMPPOINT) {
                     jp         = &sys->jumps[ sysedit_select[i].u.jump ];
                     jp->flags &= ~(JP_AUTOPOS);
                     jp->pos.x += ((double)event->motion.xrel) / sysedit_zoom;
                     jp->pos.y -= ((double)event->motion.yrel) / sysedit_zoom;
                  }
               }
            }

            /* Update mouse movement. */
            sysedit_moved += ABS( event->motion.xrel ) + ABS( event->motion.yrel );
         }
         break;
   }
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
      sysedit_zoom *= SYSEDIT_ZOOM_STEP;
      sysedit_zoom = MIN( pow(SYSEDIT_ZOOM_STEP, SYSEDIT_ZOOM_MAX), sysedit_zoom );
   }
   else if (strcmp(str,"btnZoomOut")==0) {
      sysedit_zoom /= SYSEDIT_ZOOM_STEP;
      sysedit_zoom = MAX( pow(SYSEDIT_ZOOM_STEP, SYSEDIT_ZOOM_MIN), sysedit_zoom );
   }

   /* Transform coords back. */
   sysedit_xpos *= sysedit_zoom;
   sysedit_ypos *= sysedit_zoom;
}


/**
 * @brief Deselects everything.
 */
static void sysedit_deselect (void)
{
   if (sysedit_nselect > 0)
      free( sysedit_select );
   sysedit_select    = NULL;
   sysedit_nselect   = 0;
   sysedit_mselect   = 0;

   /* Button sanity. */
   sysedit_checkButtons();
}


/**
 * @brief Checks to see which buttons should be active and the likes.
 */
static void sysedit_checkButtons (void)
{
   int i, sel_planet, sel_jump;
   Select_t *sel;

   /* See if a planet or jump is selected. */
   sel_planet  = 0;
   sel_jump    = 0;
   for (i=0; i<sysedit_nselect; i++) {
      sel = &sysedit_select[i];
      if (sel->type == SELECT_PLANET)
         sel_planet++;
      else if (sel->type == SELECT_JUMPPOINT)
         sel_jump++;
   }

   /* Planet dependent. */
   if (sel_planet) {
      window_enableButton( sysedit_wid, "btnRemove" );
      window_enableButton( sysedit_wid, "btnRename" );
   }
   else {
      window_disableButton( sysedit_wid, "btnRemove" );
      window_disableButton( sysedit_wid, "btnRename" );
   }

   /* Jump dependent. */
   if (sel_jump)
      window_enableButton( sysedit_wid, "btnReset" );
   else
      window_disableButton( sysedit_wid, "btnReset" );

   /* Editor - just one planet. */
   if (((sel_planet==1) && (sel_jump==0)) || ((sel_planet==0) && (sel_jump==1)))
      window_enableButton( sysedit_wid, "btnEdit" );
   else
      window_disableButton( sysedit_wid, "btnEdit" );
}


/**
 * @brief Adds a system to the selection.
 */
static void sysedit_selectAdd( Select_t *sel )
{
   /* Allocate if needed. */
   if (sysedit_mselect < sysedit_nselect+1) {
      if (sysedit_mselect == 0)
         sysedit_mselect = 1;
      sysedit_mselect  *= 2;
      sysedit_select    = realloc( sysedit_select,
            sizeof(Select_t) * sysedit_mselect );
   }

   /* Add system. */
   memcpy( &sysedit_select[ sysedit_nselect ], sel, sizeof(Select_t) );
   sysedit_nselect++;

   /* Button sanity. */
   sysedit_checkButtons();
}


/**
 * @brief Removes a system from the selection.
 */
static void sysedit_selectRm( Select_t *sel )
{
   int i;
   for (i=0; i<sysedit_nselect; i++) {
      if (sysedit_selectCmp( &sysedit_select[i], sel )) {
         sysedit_nselect--;
         memmove( &sysedit_select[i], &sysedit_select[i+1],
               sizeof(Select_t) * (sysedit_nselect - i) );
         /* Button sanity. */
         sysedit_checkButtons();
         return;
      }
   }
   WARN("Trying to unselect item that is not in selection!");
}


/**
 * @brief Compares two selections to see if they are the same.
 *
 *    @return 1 if both selections are the same.
 */
static int sysedit_selectCmp( Select_t *a, Select_t *b )
{
   return (memcmp(a, b, sizeof(Select_t)) == 0);
}


/**
 * @brief Edits a planet.
 */
static void sysedit_editPnt( void )
{
   unsigned int wid;
   int x, y, w, l, bw;
   char buf[1024], *s;
   Planet *p;

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];

   /* Create the window. */
   wid = window_create( EDITOR_WDWNAME, -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   sysedit_widEdit = wid;

   bw = (SYSEDIT_EDIT_WIDTH - 40 - 15 * 3) / 4.;

   /* Rename button. */
   y = -40;
   nsnprintf( buf, sizeof(buf), "Name: " );
   w = gl_printWidthRaw( NULL, buf );
   window_addText( wid, 20, y, 180, 15, 0, "txtNameLabel", &gl_smallFont, &cDConsole, buf );
   nsnprintf( buf, sizeof(buf), "%s", p->name );
   window_addText( wid, 20 + w, y, 180, 15, 0, "txtName", &gl_smallFont, &cBlack, buf );
   window_addButton( wid, -20, y - gl_defFont.h/2. + BUTTON_HEIGHT/2., bw, BUTTON_HEIGHT, "btnRename",
         "Rename", sysedit_btnRename );
   window_addButton( wid, -20 - 15 - bw, y - gl_defFont.h/2. + BUTTON_HEIGHT/2., bw, BUTTON_HEIGHT, "btnFaction",
         "Faction", sysedit_btnFaction );

   y -= gl_defFont.h + 5;

   window_addText( wid, 20, y, 180, 15, 0, "txtFactionLabel", &gl_smallFont, &cDConsole, "Faction: " );
   nsnprintf( buf, sizeof(buf), "%s", p->faction > 0 ? faction_name( p->faction ) : "None" );
   window_addText( wid, 20 + w, y, 180, 15, 0, "txtFaction", &gl_smallFont, &cBlack, buf );
   y -= gl_defFont.h + 5;

   /* Input widgets and labels. */
   x = 20;
   s = "Population";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPop",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 5, y, 80, 20, "inpPop", 12, 1, NULL );
   window_setInputFilter( wid, "inpPop",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`." );
   x += 80 + 10;

   s = "Class";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtClass",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 5, y, 30, 20, "inpClass", 1, 1, NULL );
   x += 30 + 10;

   s = "Land";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtLand",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 5, y, 150, 20, "inpLand", 20, 1, NULL );
   y -= gl_defFont.h + 15;

   /* Second row. */
   x = 20;
   s = "Presence";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPresence",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 5, y, 60, 20, "inpPresence", 5, 1, NULL );
   window_setInputFilter( wid, "inpPresence",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 60 + 10;

   s = "Range";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPresenceRange",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 5, y, 30, 20, "inpPresenceRange", 1, 1, NULL );
   window_setInputFilter( wid, "inpPresenceRange",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`." );
   x += 30 + 10;

   s = "hide";
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtHide",
         NULL, &cBlack, s );
   window_addInput( wid, x += l + 5, y, 50, 20, "inpHide", 4, 1, NULL );
   window_setInputFilter( wid, "inpHide",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 50 + 10;

   /* Bottom buttons. */
   window_addButton( wid, -20 - bw*3 - 15*3, 35 + BUTTON_HEIGHT, bw, BUTTON_HEIGHT,
         "btnRmService", "Rm Service", sysedit_btnRmService );
   window_addButton( wid, -20 - bw*2 - 15*2, 35 + BUTTON_HEIGHT, bw, BUTTON_HEIGHT,
         "btnAddService", "Add Service", sysedit_btnAddService );
   window_addButton( wid, -20 - bw - 15, 35 + BUTTON_HEIGHT, bw, BUTTON_HEIGHT,
         "btnEditTech", "Edit Tech", sysedit_btnTechEdit );
   window_addButton( wid, -20 - bw*3 - 15*3, 20, bw, BUTTON_HEIGHT,
         "btnDesc", "Description", sysedit_planetDesc );
   window_addButton( wid, -20 - bw*2 - 15*2, 20, bw, BUTTON_HEIGHT,
         "btnLandGFX", "Land GFX", sysedit_planetGFX );
   window_addButton( wid, -20 - bw - 15, 20, bw, BUTTON_HEIGHT,
         "btnSpaceGFX", "Space GFX", sysedit_planetGFX );
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT,
         "btnClose", "Close", sysedit_editPntClose );

   /* Load current values. */
   nsnprintf( buf, sizeof(buf), "%"PRIu64, p->population );
   window_setInput( wid, "inpPop", buf );
   nsnprintf( buf, sizeof(buf), "%c", planet_getClass(p) );
   window_setInput( wid, "inpClass", buf );
   window_setInput( wid, "inpLand", p->land_func );
   nsnprintf( buf, sizeof(buf), "%g", p->presenceAmount );
   window_setInput( wid, "inpPresence", buf );
   nsnprintf( buf, sizeof(buf), "%d", p->presenceRange );
   window_setInput( wid, "inpPresenceRange", buf );
   nsnprintf( buf, sizeof(buf), "%g", sqrt(p->hide) );
   window_setInput( wid, "inpHide", buf );

   /* Generate the list. */
   sysedit_genServicesList( wid );
}

/**
 * @brief Updates the jump point checkboxes.
 */
static void jp_type_check_hidden_update( unsigned int wid, char* str )
{
   (void) str;
   if (jp_hidden == 0) {
      jp_hidden = 1;
      jp_exit   = 0;
   }
   else
      jp_hidden = 0;
   window_checkboxSet( wid, "chkHidden", jp_hidden );
   window_checkboxSet( wid, "chkExit",   jp_exit );
}

/**
 * @brief Updates the jump point checkboxes.
 */
static void jp_type_check_exit_update( unsigned int wid, char* str )
{
   (void) str;
   if (jp_exit == 0) {
      jp_exit   = 1;
      jp_hidden = 0;
   }
   else
      jp_exit = 0;
   window_checkboxSet( wid, "chkHidden", jp_hidden );
   window_checkboxSet( wid, "chkExit",   jp_exit );
}

/**
 * @brief Edits a jump.
 */
static void sysedit_editJump( void )
{
   unsigned int wid;
   int x, y, w, l, bw;
   char buf[1024], *s;
   JumpPoint *j;

   j = &sysedit_sys->jumps[ sysedit_select[0].u.jump ];

   /* Create the window. */
   wid = window_create( EDITOR_WDWNAME, -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   sysedit_widEdit = wid;

   bw = (SYSEDIT_EDIT_WIDTH - 40 - 15 * 3) / 4.;

   /* Target lable. */
   y = -40;
   nsnprintf( buf, sizeof(buf), "Target: " );
   w = gl_printWidthRaw( NULL, buf );
   window_addText( wid, 20, y, 180, 15, 0, "txtTargetLabel", &gl_smallFont, &cDConsole, buf );
   nsnprintf( buf, sizeof(buf), "%s", j->target->name );
   window_addText( wid, 20 + w, y, 180, 15, 0, "txtName", &gl_smallFont, &cBlack, buf );

   y -= gl_defFont.h + 10;

   /* Input widgets and labels. */
   x = 20;

   /* Initial checkbox state */
   jp_hidden = 0;
   jp_exit   = 0;
   if (jp_isFlag( j, JP_HIDDEN ))
      jp_hidden = 1;
   else if (jp_isFlag( j, JP_EXITONLY ))
      jp_exit   = 1;
   /* Create check boxes. */
   window_addCheckbox( wid, x, y, 100, 20,
         "chkHidden", "Hidden", jp_type_check_hidden_update, jp_hidden );
   y -= 20;
   window_addCheckbox( wid, x, y, 100, 20,
         "chkExit", "Exit only", jp_type_check_exit_update, jp_exit );
   y -= 30;

   s = "Hide"; /* TODO: if inpType == 0 disable hide box */
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtHide",
         NULL, &cBlack, s );
   window_addInput( wid, x + l + 8, y, 50, 20, "inpHide", 4, 1, NULL );
   window_setInputFilter( wid, "inpHide",
         "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/\\'\"~<>!@#$%^&|_`" );
   x += 50 + 10;

   /* Bottom buttons. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT,
         "btnClose", "Close", sysedit_editJumpClose );

   /* Load current values. */
   nsnprintf( buf, sizeof(buf), "%g", sqrt(j->hide) );
   window_setInput( wid, "inpHide", buf );
}

/**
 * @brief Displays the planet landing description and bar description in a separate window.
 */
static void sysedit_planetDesc( unsigned int wid, char *unused )
{
   (void) unused;
   int x, y, h, w, bw;
   Planet *p;
   char *desc, *bardesc;

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];

   /* Create the window. */
   wid = window_create( "Planet Information", -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   x = 20;
   y = -40;
   w = SYSEDIT_EDIT_WIDTH - 40;
   h = (SYSEDIT_EDIT_HEIGHT - gl_defFont.h * 2 - 30 - 60 - BUTTON_HEIGHT - 10) / 2.;
   desc    = p->description ? p->description : "None";
   bardesc = p->bar_description ? p->bar_description : "None";
   bw = (SYSEDIT_EDIT_WIDTH - 40 - 15 * 3) / 4.;

   window_addButton( wid, -20 - bw*3 - 15*3, 20, bw, BUTTON_HEIGHT,
         "btnProperties", "Properties", window_close );

   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT,
         "btnClose", "Close", sysedit_planetDescClose );

   /* Description label and text. */
   window_addText( wid, x, y, w, gl_defFont.h, 0, "txtDescriptionLabel", &gl_defFont, &cBlack,
         "Landing Description" );
   y -= gl_defFont.h + 10;
   window_addText( wid, x, y, w, h, 0, "txtDescription", &gl_smallFont, &cBlack,
         desc );
   y -= h + 10;

   /* Bar description label and text. */
   window_addText( wid, x, y, w, gl_defFont.h, 0, "txtBarDescriptionLabel", &gl_defFont, &cBlack,
         "Bar Description" );
   y -= gl_defFont.h + 10;
   window_addText( wid, x, y, w, h, 0, "txtBarDescription", &gl_smallFont, &cBlack,
         bardesc );
}

/**
 * @brief Closes both the planet description window and the properties window.
 */
static void sysedit_planetDescClose( unsigned int wid, char *unused )
{
   window_close( wid, unused );
   sysedit_editPntClose( sysedit_widEdit, unused );
}


/**
 * @brief Generates the planet services list.
 */
static void sysedit_genServicesList( unsigned int wid )
{
   int i, j, n;
   Planet *p;
   char **have, **lack;
   int x, y, w, h, hpos, lpos;

   hpos = lpos = -1;

   /* Destroy if exists. */
   if (widget_exists( wid, "lstServicesHave" ) &&
         widget_exists( wid, "lstServicesLacked" )) {
      hpos = toolkit_getListPos( wid, "lstServicesHave" );
      lpos = toolkit_getListPos( wid, "lstServicesLacked" );
      window_destroyWidget( wid, "lstServicesHave" );
      window_destroyWidget( wid, "lstServicesLacked" );
   }

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   x = 20;
   y = 20 + BUTTON_HEIGHT * 2 + 30;
   w = (SYSEDIT_EDIT_WIDTH - 40 - 15 * 3) / 4.;
   h = SYSEDIT_EDIT_HEIGHT - y - 130;

   /* Get all missing services. */
   n = 0;
   for (i=0; i<8; i++)
      if (!planet_hasService(p, 1<<i) && (1<<i != PLANET_SERVICE_INHABITED))
         n++;

   /* Get all the services the planet has. */
   j = 0;
   if (planet_hasService(p, PLANET_SERVICE_LAND)) {
      have = malloc( sizeof(char*) * 8 );
      for (i=0; i<8; i++)
         if (planet_hasService(p, 1<<i)  && (1<<i != PLANET_SERVICE_INHABITED))
            have[j++] = strdup( planet_getServiceName( 1<<i ) );
   }
   else {
      have = malloc( sizeof(char*) );
      have[j++] = strdup("None");
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstServicesHave", have, j, 0, NULL );
   x += w + 15;

   /* Add list of services the planet lacks. */
   j = 0;
   if (n) {
      lack = malloc( sizeof(char*) * n );
      for (i=0; i<8; i++)
         if (!planet_hasService(p, 1<<i) && (1<<i != PLANET_SERVICE_INHABITED))
            lack[j++] = strdup( planet_getServiceName(1<<i) );
   }
   else {
      lack = malloc( sizeof(char*) );
      lack[j++] = strdup( "None" );
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstServicesLacked", lack, j, 0, NULL );

   /* Restore positions. */
   if (hpos != -1 && lpos != -1) {
      toolkit_setListPos( wid, "lstServicesHave", hpos );
      toolkit_setListPos( wid, "lstServicesLacked", lpos );
   }
}


/**
 * @brief Adds a service to a planet.
 */
static void sysedit_btnAddService( unsigned int wid, char *unused )
{
   (void) unused;
   char *selected;
   Planet *p;

   selected = toolkit_getList( wid, "lstServicesLacked" );
   if ((selected == NULL) || (strcmp(selected,"None")==0))
      return;

   /* Enable the service. All services imply landability. */
   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   p->services |= planet_getService(selected) | PLANET_SERVICE_INHABITED | PLANET_SERVICE_LAND;

   /* Regenerate the list. */
   sysedit_genServicesList( wid );
}


/**
 * @brief Removes a service from a planet.
 */
static void sysedit_btnRmService( unsigned int wid, char *unused )
{
   (void) unused;
   char *selected;
   int i;
   Planet *p;

   selected = toolkit_getList( wid, "lstServicesHave" );
   if ((selected==NULL) || (strcmp(selected,"None")==0))
      return;

   /* Flip the bit. Safe enough, as it's always 1 to start with. */
   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   p->services ^= planet_getService(selected);

   /* If landability was removed, the rest must go, too. */
   if (strcmp(selected,"Land")==0) {
      for (i=0; i<8; i++)
         if (1<<i != PLANET_SERVICE_LAND)
            p->services &= (1<<i) & planet_hasService(p, PLANET_SERVICE_LAND);
   }

   sysedit_genServicesList( wid );
}


/**
 * @brief Edits a planet's tech.
 */
static void sysedit_btnTechEdit( unsigned int wid, char *unused )
{
   (void) unused;
   int y, w, bw;

   /* Create the window. */
   wid = window_create( "Planet Tech Editor", -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   w = (SYSEDIT_EDIT_WIDTH - 40 - 15) / 2.;
   bw = (SYSEDIT_EDIT_WIDTH - 40 - 15 * 3) / 4.;

   /* Close button. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );
   y = 20 + BUTTON_HEIGHT + 15;

   /* Remove button. */
   window_addButton( wid, -20-(w+15), y, w, BUTTON_HEIGHT,
         "btnRm", "Rm Tech", sysedit_btnRmTech );

   /* Add button. */
   window_addButton( wid, -20, y, w, BUTTON_HEIGHT,
         "btnAdd", "Add Tech", sysedit_btnAddTech );

   sysedit_genTechList( wid );
}


/**
 * @brief Generates the planet services list.
 */
static void sysedit_genTechList( unsigned int wid )
{
   Planet *p;
   char **have, **lack, **tmp;
   int i, j, n, x, y, w, h, hpos, lpos;

   hpos = lpos = -1;

   /* Destroy if exists. */
   if (widget_exists( wid, "lstTechsHave" ) &&
         widget_exists( wid, "lstTechsLacked" )) {
      hpos = toolkit_getListPos( wid, "lstTechsHave" );
      lpos = toolkit_getListPos( wid, "lstTechsLacked" );
      window_destroyWidget( wid, "lstTechsHave" );
      window_destroyWidget( wid, "lstTechsLacked" );
   }

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   w = (SYSEDIT_EDIT_WIDTH - 40 - 15) / 2.;
   x = -20 - w - 15;
   y = 20 + BUTTON_HEIGHT * 2 + 30;
   h = SYSEDIT_EDIT_HEIGHT - y - 30;

   /* Get all the techs the planet has. */
   n = 0;
   if (p->tech != NULL)
      have = tech_getItemNames( p->tech, &n );
   else {
      have = malloc( sizeof(char*) );
      have[n++] = strdup("None");
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTechsHave", have, n, 0, NULL );
   x += w + 15;

   /* Omit the techs that the planet already has from the list.  */
   n = 0;
   if (p->tech != NULL) {
      tmp = tech_getAllItemNames( &j );
      for (i=0; i<j; i++)
         if (!tech_hasItem( p->tech, tmp[i] ))
            n++;

      if (!n) {
         lack = malloc( sizeof(char*) );
         lack[n++] = strdup("None");
      }
      else {
         lack = malloc( sizeof(char*) * j );
         n = 0;
         for (i=0; i<j; i++)
            if (!tech_hasItem( p->tech, tmp[i] ))
               lack[n++] = strdup( tmp[i] );
      }
      free(tmp);
   }
   else
      lack = tech_getAllItemNames( &n );

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTechsLacked", lack, n, 0, NULL );

   /* Restore positions. */
   if (hpos != -1 && lpos != -1) {
      toolkit_setListPos( wid, "lstTechsHave", hpos );
      toolkit_setListPos( wid, "lstTechsLacked", lpos );
   }
}


/**
 * @brief Adds a tech to a planet.
 */
static void sysedit_btnAddTech( unsigned int wid, char *unused )
{
   (void) unused;
   char *selected;
   Planet *p;

   selected = toolkit_getList( wid, "lstTechsLacked" );
   if ((selected == NULL) || (strcmp(selected,"None")==0))
      return;

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   if (p->tech == NULL)
      p->tech = tech_groupCreate();
   tech_addItemTech( p->tech, selected );

   /* Regenerate the list. */
   sysedit_genTechList( wid );
}


/**
 * @brief Removes a tech from a planet.
 */
static void sysedit_btnRmTech( unsigned int wid, char *unused )
{
   (void) unused;
   char *selected;
   Planet *p;
   int n;

   selected = toolkit_getList( wid, "lstTechsHave" );
   if ((selected == NULL) || (strcmp(selected,"None")==0))
      return;

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   if (tech_hasItem( p->tech, selected ))
      tech_rmItemTech( p->tech, selected );

   tech_getItemNames( p->tech, &n );
   if (!n)
      p->tech = NULL;

   /* Regenerate the list. */
   sysedit_genTechList( wid );
}


/**
 * @brief Edits a planet's faction.
 */
static void sysedit_btnFaction( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   unsigned int wid;
   int i, j, y, h, n, bw, *factions;
   char **str;

   /* Create the window. */
   wid = window_create( "Modify Faction", -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   /* Generate factions list. */
   factions = faction_getAll( &n );
   str = malloc( sizeof(char*) * (n+1));
   j   = 0;
   for (i=0; i<n; i++)
      str[j++] = strdup( faction_name( factions[i] ) );
   str[j++] = strdup( "None" );

   bw = (SYSEDIT_EDIT_WIDTH - 40 - 15 * 3) / 4.;
   y = 20 + BUTTON_HEIGHT + 15;
   h = SYSEDIT_EDIT_HEIGHT - y - 30;
   window_addList( wid, 20, -40, SYSEDIT_EDIT_WIDTH-40, h,
         "lstFactions", str, j, 0, NULL );

   /* Close button. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT,
         "btnClose", "Close", window_close );

   /* Add button. */
   window_addButton( wid, -20-(bw+15), 20, bw, BUTTON_HEIGHT,
         "btnAdd", "Set", sysedit_btnFactionSet );
}


/**
 * @brief Actually modifies the faction.
 */
static void sysedit_btnFactionSet( unsigned int wid, char *unused )
{
   (void) unused;
   char *selected;
   Planet *p;

   selected = toolkit_getList( wid, "lstFactions" );
   if (selected == NULL)
      return;

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   /* Set the faction. */
   if (strcmp(selected,"None")==0)
      p->faction = -1;
   else
      p->faction = faction_get( selected );

   /* Update the editor window. */
   window_modifyText( sysedit_widEdit, "txtFaction", p->faction > 0 ? faction_name( p->faction ) : "None" );

   window_close( wid, unused );
}


/**
 * @brief Opens the system property editor.
 */
static void sysedit_btnEdit( unsigned int wid_unused, char *unused )
{
   (void) wid_unused;
   (void) unused;
   Select_t *sel;

   sel = &sysedit_select[0];

   if (sel->type==SELECT_PLANET)
      sysedit_editPnt();
   else if (sel->type==SELECT_JUMPPOINT)
      sysedit_editJump();
}


/**
 * @brief Opens the planet landing or space graphic editor.
 */
static void sysedit_planetGFX( unsigned int wid_unused, char *wgt )
{
   (void) wid_unused;
   unsigned int wid;
   uint32_t nfiles, i, j;
   char *path, buf[PATH_MAX];
   char **files, **png_files;
   glTexture **tex, *t;
   int w, h, land;
   Planet *p;
   glColour *bg, c;

   land = !strcmp(wgt,"btnLandGFX");

   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];
   /* Create the window. */
   nsnprintf( buf, sizeof(buf), "%s - Planet Properties", p->name );
   wid = window_create( buf, -1, -1, -1, -1 );
   window_dimWindow( wid, &w, &h );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", sysedit_btnGFXClose );

   /* Apply button. */
   window_addButton( wid, -20, 20+(20+BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT,
         land ? "btnApplyLand" : "btnApplySpace", "Apply", sysedit_btnGFXApply );

   /* Find images first. */
   path           = land ? PLANET_LAND_GFX_PATH : PLANET_SPACE_GFX_PATH;
   files          = ndata_list( path, &nfiles );
   ndata_sortName( files, nfiles );
   png_files      = malloc( sizeof(char*) * nfiles );
   tex            = malloc( sizeof(glTexture*) * nfiles );
   sysedit_tex    = malloc( sizeof(glTexture*) * nfiles );
   bg             = malloc( sizeof(glColour) * nfiles );
   j              = 0;
   for (i=0; i<nfiles; i++) {
      nsnprintf( buf, sizeof(buf), "%s/%s", path, files[i] );
      t              = gl_newImage( buf, OPENGL_TEX_MIPMAPS );
      if (t != NULL) {
         tex[j]         = t;
         sysedit_tex[j] = tex[j];
         png_files[j]   = strdup( files[i] );
         c = strcmp(files[i], land ? p->gfx_exteriorPath : p->gfx_spacePath)==0 ? cOrange : cBlack;
         memcpy( &bg[j], &c, sizeof(glColour) );
         j++;
      }
      free( files[i] );
   }
   free( files );
   sysedit_ntex   = j;

   /* Add image array. */
   window_addImageArray( wid, 20, 20, w-60-BUTTON_WIDTH, h-60, "iarGFX", 128, 128, tex, png_files, j, NULL, NULL );
   toolkit_setImageArray( wid, "iarGFX", path );
   toolkit_setImageArrayBackground( wid, "iarGFX", bg );
}


/**
 * @brief Closes the planet graphic editor.
 */
static void sysedit_btnGFXClose( unsigned int wid, char *wgt )
{
   int i;
   for (i=0; i<sysedit_ntex; i++)
      gl_freeTexture( sysedit_tex[i] );
   if (sysedit_tex != NULL)
      free( sysedit_tex );
   sysedit_tex    = NULL;
   sysedit_ntex   = 0;
   window_close( wid, wgt );
}


/**
 * @brief Apply new graphics.
 */
static void sysedit_btnGFXApply( unsigned int wid, char *wgt )
{
   Planet *p;
   char *str, *path, buf[PATH_MAX];
   int land;

   land = !strcmp(wgt,"btnApplyLand");
   p = sysedit_sys->planets[ sysedit_select[0].u.planet ];

   /* Get output. */
   str = toolkit_getImageArray( wid, "iarGFX" );
   if (str == NULL)
      return;

   /* New path. */
   path = land ? PLANET_LAND_GFX_PATH : PLANET_SPACE_GFX_PATH;
   nsnprintf( buf, sizeof(buf), "%s/%s", path, str );

   if (land) {
      free( p->gfx_exteriorPath );
      nsnprintf( buf, sizeof(buf), PLANET_GFX_EXTERIOR"%s", str );
      p->gfx_exteriorPath = strdup( str );
      p->gfx_exterior = strdup( buf );
   }
   else { /* Free old texture, load new. */
      free( p->gfx_spacePath );
      gl_freeTexture( p->gfx_space );
      p->gfx_space     = gl_newImage( buf, OPENGL_TEX_MIPMAPS );
      p->gfx_spacePath = strdup( str );
   }

   /* For now we close. */
   sysedit_btnGFXClose( wid, wgt );
}
