/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file dev_sysedit.c
 *
 * @brief Handles the star system editor.
 */
/** @cond */
#include "physfs.h"
#include <SDL3/SDL_events.h>

#include "naev.h"
/** @endcond */

#include <libgen.h>

#include "dev_sysedit.h"

#include "array.h"
#include "conf.h"
#include "dev_spob.h"
#include "dev_system.h"
#include "dev_uniedit.h"
#include "dialogue.h"
#include "economy.h"
#include "ndata.h"
#include "nstring.h"
#include "opengl.h"
#include "opengl_render.h"
#include "safelanes.h"
#include "space.h"
#include "toolkit.h"

#define BUTTON_WIDTH 100 /**< Map button width. */
#define BUTTON_HEIGHT 30 /**< Map button height. */

#define SYSEDIT_EDIT_WIDTH 500  /**< System editor width. */
#define SYSEDIT_EDIT_HEIGHT 400 /**< System editor height. */

#define SYSEDIT_DRAG_THRESHOLD 300 /**< Drag threshold. */
#define SYSEDIT_MOVE_THRESHOLD 10  /**< Movement threshold. */

#define SYSEDIT_ZOOM_STEP 1.2 /**< Factor to zoom by for each zoom level. */
#define SYSEDIT_ZOOM_MAX 1    /**< Maximum zoom level (close). */
#define SYSEDIT_ZOOM_MIN -23  /**< Minimum zoom level (far). */

/*
 * Selection types.
 */
enum {
   SELECT_NONE,       /**< No selection. */
   SELECT_SPOB,       /**< Selection is a spob. */
   SELECT_JUMPPOINT,  /**< Selection is a jump point. */
   SELECT_ASTEROID,   /**< Selection is an asteroid. */
   SELECT_ASTEXCLUDE, /**< Selection is an asteroid exclusion zone. */
};

/**
 * @brief Selection generic for stuff in a system.
 */
typedef struct Select_s {
   int type; /**< Type of selection. */
   union {
      int spob;
      int jump;
      int asteroid;
      int astexclude;
   } u; /**< Data itself. */
} Select_t;
static Select_t *sysedit_select = NULL; /**< Current system selection. */
static int sysedit_nselect = 0; /**< Number of selections in current system. */
static int sysedit_mselect = 0; /**< Memory allocated for selections. */
static Select_t sysedit_tsel;   /**< Temporary selection. */
static int      sysedit_tadd     = 0;    /**< Add to selection. */
static char   **sysedit_tagslist = NULL; /**< List of existing tags. */

/*
 * System editor stuff.
 */
static StarSystem  *sysedit_sys      = NULL; /**< Currently opened system. */
static unsigned int sysedit_wid      = 0;    /**< Sysedit wid. */
static unsigned int sysedit_widEdit  = 0;    /**< Spob editor wid. */
static int          sysedit_grid     = 1;    /**< Grid is visible. */
static double       sysedit_xpos     = 0.;   /**< Viewport X position. */
static double       sysedit_ypos     = 0.;   /**< Viewport Y position. */
static double       sysedit_zoom     = 1.;   /**< Viewport zoom level. */
static int          sysedit_moved    = 0;  /**< Space moved since mouse down. */
static unsigned int sysedit_dragTime = 0;  /**< Tick last started to drag. */
static int          sysedit_drag     = 0;  /**< Dragging viewport around. */
static int          sysedit_dragSel  = 0;  /**< Dragging system around. */
static double       sysedit_mx       = 0.; /**< Cursor X position. */
static double       sysedit_my       = 0.; /**< Cursor Y position. */

/* Stored checkbox values. */
static int jp_hidden = 0; /**< Jump point hidden checkbox value. */
static int jp_exit   = 0; /**< Jump point exit only checkbox value. */

/*
 * System editor Prototypes.
 */
/* Custom system editor widget. */
static void sysedit_buttonZoom( unsigned int wid, const char *str );
static void sysedit_render( double bx, double by, double w, double h,
                            void *data );
static void sysedit_renderAsteroidsField( double bx, double by,
                                          const AsteroidAnchor *ast,
                                          int                   selected );
static void sysedit_renderAsteroidExclusion( double bx, double by,
                                             const AsteroidExclusion *aexcl,
                                             int selected );
static void sysedit_renderBG( double bx, double bw, double w, double h,
                              double x, double y );
static void sysedit_renderSprite( const glTexture *gfx, double bx, double by,
                                  double x, double y, int sx, int sy,
                                  const glColour *c, int selected,
                                  const char *caption );
static void sysedit_focusLose( unsigned int wid, const char *wgtname );
static int  sysedit_mouseTrySelect( const Select_t *sel, double x, double y,
                                    double t, double mx, double my,
                                    SDL_Keymod mod, void ( *func )( void ) );
static int  sysedit_mouse( unsigned int wid, const SDL_Event *event, double mx,
                           double my, double w, double h, double xr, double yr,
                           void *data );
/* Button functions. */
static void sysedit_close( unsigned int wid, const char *wgt );
static void sysedit_btnNewSpob( unsigned int wid_unused, const char *unused );
static void sysedit_btnNewAsteroids( unsigned int wid_unused,
                                     const char  *unused );
static UniAttribute_t *sysedit_asteroidsAttr( const AsteroidAnchor *ast );
static void sysedit_btnRename( unsigned int wid_unused, const char *unused );
static void sysedit_btnRemove( unsigned int wid_unused, const char *unused );
static void sysedit_btnReset( unsigned int wid_unused, const char *unused );
static void sysedit_btnScale( unsigned int wid_unused, const char *unused );
static void sysedit_btnGrid( unsigned int wid_unused, const char *unused );
static void sysedit_btnEdit( unsigned int wid_unused, const char *unused );
/* Spob editing. */
static void sysedit_editPnt( void );
static void sysedit_editPntClose( unsigned int wid, const char *unused );
static void sysedit_spobDesc( unsigned int wid, const char *unused );
static void sysedit_spobDescReturn( unsigned int wid, const char *unused );
static void sysedit_spobDescClose( unsigned int wid, const char *unused );
static void sysedit_genServicesList( unsigned int wid );
static void sysedit_btnTechEdit( unsigned int wid, const char *unused );
static void sysedit_genTechList( unsigned int wid );
static void sysedit_btnAddTech( unsigned int wid, const char *unused );
static void sysedit_btnRmTech( unsigned int wid, const char *unused );
static void sysedit_btnTagsEdit( unsigned int wid, const char *unused );
static void sysedit_genTagsList( unsigned int wid );
static void sysedit_btnAddTag( unsigned int wid, const char *unused );
static void sysedit_btnRmTag( unsigned int wid, const char *unused );
static void sysedit_btnNewTag( unsigned int wid, const char *unused );
static void sysedit_btnTagsClose( unsigned int wid, const char *unused );
static void sysedit_btnAddService( unsigned int wid, const char *unused );
static void sysedit_btnRmService( unsigned int wid, const char *unused );
static void sysedit_spobGFX( unsigned int wid_unused, const char *wgt );
static void sysedit_btnGFXClose( unsigned int wid, const char *wgt );
static void sysedit_btnGFXApply( unsigned int wid, const char *wgt );
static void sysedit_btnFaction( unsigned int wid_unused, const char *unused );
static void sysedit_btnFactionSet( unsigned int wid, const char *unused );
/* Jump editing */
static void sysedit_editJump( void );
static void sysedit_editJumpClose( unsigned int wid, const char *unused );
/* Asteroid editing. */
static void sysedit_editAsteroids( void );
static void sysedit_editAsteroidsClose( unsigned int wid, const char *unused );
static void sysedit_genAsteroidsList( unsigned int wid );
static void sysedit_btnAsteroidsDelete( unsigned int wid, const char *unused );
static void sysedit_btnRmAsteroid( unsigned int wid, const char *unused );
static void sysedit_btnAddAsteroid( unsigned int wid, const char *unused );
/* Exclusion zone editing. */
static void sysedit_editExclusion( void );
static void sysedit_editExclusionClose( unsigned int wid, const char *unused );
static void sysedit_btnExclusionDelete( unsigned int wid, const char *unused );
/* Keybindings handling. */
static int sysedit_keys( unsigned int wid, SDL_Keycode key, SDL_Keymod mod,
                         int isrepeat );
/* Selection. */
static int  sysedit_selectCmp( const Select_t *a, const Select_t *b );
static int  sysedit_isSelected( const Select_t *s );
static void sysedit_checkButtons( void );
static void sysedit_deselect( void );
static void sysedit_selectAdd( const Select_t *sel );
static void sysedit_selectRm( const Select_t *sel );
/* Diffs. */
static void sysedit_diffCreateSpobStr( const Spob *spb, UniHunkType_t type,
                                       char *str );
static void sysedit_diffCreateSpobInt( const Spob *spb, UniHunkType_t type,
                                       int data );
static void sysedit_diffCreateSpobFloat( const Spob *spb, UniHunkType_t type,
                                         double fdata );

/**
 * @brief Opens the system editor interface.
 */
void sysedit_open( StarSystem *sys )
{
   unsigned int   wid;
   char           buf[128];
   int            i;
   const glColour cBG = { 0., 0., 0., 0.95 };

   /* Reconstructs the jumps - just in case. */
   systems_reconstructJumps();

   /* Reset some variables. */
   sysedit_sys  = sys;
   sysedit_drag = 0;
   sysedit_zoom = pow( SYSEDIT_ZOOM_STEP, SYSEDIT_ZOOM_MIN );
   sysedit_xpos = 0.;
   sysedit_ypos = 0.;

   /* Load graphics. */
   space_gfxLoad( sysedit_sys );

   /* Create the window. */
   snprintf( buf, sizeof( buf ), _( "%s - Star System Editor" ), sys->name );
   wid = window_create( "wdwSysEdit", buf, -1, -1, -1, -1 );
   window_setDynamic( wid, 1 );
   window_handleKeys( wid, sysedit_keys );
   window_setBorder( wid, 0 );
   sysedit_wid = wid;

   window_setAccept( wid, sysedit_close );

   /* Actual viewport, at the bottom. */
   window_addCust( wid, 0, 0, SCREEN_W, SCREEN_H, "cstSysEdit", 1,
                   sysedit_render, sysedit_mouse, NULL, sysedit_focusLose,
                   NULL );

   /* Overlay background. */
   window_addRect( wid, SCREEN_W - 130, 0, 130, SCREEN_H, "rctRCol", &cBG, 0 );
   window_addRect( wid, 0, 0, SCREEN_W, 60, "rctBBar", &cBG, 0 );

   /* Close button. */
   window_addButtonKey( wid, -15, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                        _( "Exit" ), sysedit_close, SDLK_X );
   i = 1;

   /* Scale. */
   window_addButton( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnScale", _( "Scale" ),
                     sysedit_btnScale );
   i += 1;

   /* Reset. */
   window_addButtonKey( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnReset", _( "Reset Jumps" ),
                        sysedit_btnReset, SDLK_R );
   i += 1;

   /* Editing. */
   window_addButtonKey( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnEdit", _( "Edit" ), sysedit_btnEdit,
                        SDLK_E );
   i += 1;

   /* Remove. */
   window_addButton( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                     BUTTON_HEIGHT, "btnRemove", _( "Remove" ),
                     sysedit_btnRemove );
   i += 1;

   /* New spob. */
   window_addButtonKey( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnNewSpob", _( "New Spob" ),
                        sysedit_btnNewSpob, SDLK_N );
   i += 1;

   /* New asteroids. */
   window_addButtonKey( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnNewAsteroids", _( "New Asteroids" ),
                        sysedit_btnNewAsteroids, SDLK_A );
   i += 1;

   /* Toggle Grid. */
   window_addButtonKey( wid, -15, 20 + ( BUTTON_HEIGHT + 20 ) * i, BUTTON_WIDTH,
                        BUTTON_HEIGHT, "btnGrid", _( "Grid" ), sysedit_btnGrid,
                        SDLK_G );

   /* Options button. */
   window_addButton( wid, -15 - BUTTON_WIDTH - 20, 20, BUTTON_HEIGHT,
                     BUTTON_HEIGHT, "btnOptions", NULL, uniedit_options );
   window_buttonCustomRender( wid, "btnOptions",
                              window_buttonCustomRenderGear );

   /* Zoom buttons */
   window_addButton( wid, 20, 20, 30, 30, "btnZoomIn", p_( "zoomin", "+" ),
                     sysedit_buttonZoom );
   window_addButton( wid, 60, 20, 30, 30, "btnZoomOut", p_( "zoomout", "-" ),
                     sysedit_buttonZoom );

   /* Selected text. */
   snprintf( buf, sizeof( buf ), _( "Radius: %.0f" ), sys->radius );
   window_addText( wid, 100, 10, SCREEN_W / 2 - 140, 30, 0, "txtSelected",
                   &gl_smallFont, NULL, buf );

   /* Deselect everything. */
   sysedit_deselect();
}

/**
 * @brief Handles keybindings.
 */
static int sysedit_keys( unsigned int wid, SDL_Keycode key, SDL_Keymod mod,
                         int isrepeat )
{
   (void)wid;
   (void)mod;
   (void)isrepeat;

   switch ( key ) {

   default:
      return 0;
   }
}

/**
 * @brief Closes the system editor widget.
 */
static void sysedit_close( unsigned int wid, const char *wgt )
{
   /* Unload graphics. */
   space_gfxLoad( sysedit_sys );

   /* Remove selection. */
   sysedit_deselect();

   /* Set the dominant faction. */
   system_setFaction( sysedit_sys );

   /* Update asteroid info. */
   system_updateAsteroids( sysedit_sys );

   /* Save the system */
   if ( conf.devautosave ) {
      if ( dsys_saveSystem( sysedit_sys ) )
         uniedit_saveError();
   }

   /* Reconstruct universe presences. */
   space_reconstructPresences();
   safelanes_recalculate();

   /* Close the window. */
   window_close( wid, wgt );

   /* Update the universe editor's sidebar text. */
   uniedit_selectText();

   /* Unset. */
   sysedit_wid = 0;
}

/**
 * @brief Closes the spob editor, saving the changes made.
 */
static void sysedit_editPntClose( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *inp;
   Spob       *p = sysedit_sys->spobs[sysedit_select[0].u.spob];
   int         data;
   double      fdata;

   fdata = atof( window_getInput( sysedit_widEdit, "inpPop" ) );
   if ( fabs( fdata - p->population ) > 0. ) {
      if ( uniedit_diffMode )
         sysedit_diffCreateSpobFloat( p, HUNK_TYPE_SPOB_POPULATION, fdata );
      else
         p->population = fdata;
   }

   inp = window_getInput( sysedit_widEdit, "inpClass" );
   if ( strcmp( p->class, inp ) != 0 ) {
      if ( uniedit_diffMode ) {
         sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_CLASS, strdup( inp ) );
      } else {
         free( p->class );
         if ( inp[0] == '\0' )
            p->class = NULL;
         else
            p->class = strdup( inp );
      }
   }

   inp = window_getInput( sysedit_widEdit, "inpLua" );
   if ( ( inp != p->lua_file ) ||
        ( ( p->lua_file != NULL ) && ( inp != NULL ) &&
          strcmp( p->lua_file, inp ) != 0 ) ) {
      if ( uniedit_diffMode ) {
         if ( ( inp == NULL ) || ( strlen( inp ) == 0 ) )
            sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_LUA, NULL );
         else
            sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_LUA, strdup( inp ) );
      } else {
         free( p->lua_file );
         if ( ( inp == NULL ) || ( strlen( inp ) == 0 ) )
            p->lua_file = NULL;
         else
            p->lua_file = strdup( inp );
      }
   }

   fdata = atof( window_getInput( sysedit_widEdit, "inpPresenceBase" ) );
   if ( fabs( p->presence.base - fdata ) > 1e-5 ) {
      if ( uniedit_diffMode )
         sysedit_diffCreateSpobFloat( p, HUNK_TYPE_SPOB_PRESENCE_BASE, fdata );
      else
         p->presence.base = fdata;
   }
   fdata = atof( window_getInput( sysedit_widEdit, "inpPresenceBonus" ) );
   if ( fabs( p->presence.bonus - fdata ) > 1e-5 ) {
      if ( uniedit_diffMode )
         sysedit_diffCreateSpobFloat( p, HUNK_TYPE_SPOB_PRESENCE_BONUS, fdata );
      else
         p->presence.bonus = fdata;
   }
   data = atoi( window_getInput( sysedit_widEdit, "inpPresenceRange" ) );
   if ( data != p->presence.range ) {
      if ( uniedit_diffMode )
         sysedit_diffCreateSpobInt( p, HUNK_TYPE_SPOB_PRESENCE_RANGE, data );
      else
         p->presence.range = data;
   }
   fdata = atof( window_getInput( sysedit_widEdit, "inpHide" ) );
   if ( fabs( p->hide - fdata ) > 1e-5 ) {
      if ( uniedit_diffMode )
         sysedit_diffCreateSpobFloat( p, HUNK_TYPE_SPOB_HIDE, fdata );
      else
         p->hide = fdata;
   }

   for ( int i = 0; i < array_size( sysedit_tagslist ); i++ )
      free( sysedit_tagslist[i] );
   array_free( sysedit_tagslist );
   sysedit_tagslist = NULL;

   /* Have to recompute presences if stuff changed. */
   space_reconstructPresences();

   if ( !uniedit_diffMode ) {
      if ( conf.devautosave ) {
         if ( dpl_saveSpob( p ) )
            uniedit_saveError();
      }

      /* Clean up presences. */
      space_reconstructPresences();
      safelanes_recalculate();
   }

   window_close( wid, unused );
}

/**
 * @brief Enters the editor in new spob mode.
 */
static void sysedit_btnNewSpob( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   Spob *p, *b;
   char *name;
   int   good;

   if ( uniedit_diffMode ) {
      /* TODO let select the spob from a list. */
      dialogue_alertRaw(
         ( "Adding new spobs is not supported in diff mode!" ) );
      return;
   }

   /* Get new name. */
   name = dialogue_inputRaw( _( "New Spob Creation" ), 1, 32,
                             _( "What do you want to name the new spob?" ) );
   if ( name == NULL )
      return;

   /* Check for collision. */
   if ( spob_exists( name ) ) {
      dialogue_alert( _( "Space object by the name of #r'%s'#0 already exists "
                         "in the #r'%s'#0 system" ),
                      name, spob_getSystemName( name ) );
      free( name );
      sysedit_btnNewSpob( 0, NULL );
      return;
   }

   /* Create the new spob. */
   p       = spob_new();
   p->name = name;

   /* Set filename. */
   char *cleanname = uniedit_nameFilter( p->name );
   SDL_asprintf( &p->filename, "spob/%s.xml", cleanname );
   free( cleanname );

   /* Base spob data off another. */
   good = 0;
   while ( !good ) {
      b    = spob_get( space_getRndSpob( 0, 0, NULL ) );
      good = !( ( b->class == NULL ) || ( b->gfx_spacePath == NULL ) ||
                ( b->gfx_spaceName == NULL ) || ( b->gfx_exterior == NULL ) ||
                ( b->gfx_exteriorPath == NULL ) );
   }
   p->class            = strdup( b->class );
   p->gfx_spacePath    = strdup( b->gfx_spacePath );
   p->gfx_spaceName    = strdup( b->gfx_spaceName );
   p->gfx_exterior     = strdup( b->gfx_exterior );
   p->gfx_exteriorPath = strdup( b->gfx_exteriorPath );
   p->pos.x            = sysedit_xpos / sysedit_zoom;
   p->pos.y            = sysedit_ypos / sysedit_zoom;
   p->hide             = HIDE_DEFAULT_SPOB;
   p->radius           = b->radius;

   /* Add new spob. */
   system_addSpob( sysedit_sys, name );

   /* Run galaxy modifications. */
   space_reconstructPresences();
   economy_execQueued();

   if ( conf.devautosave ) {
      int ret = dsys_saveSystem( sysedit_sys );
      ret |= dpl_saveSpob( p );
      if ( ret )
         uniedit_saveError();
   }

   /* Reload graphics. */
   space_gfxLoad( sysedit_sys );
}

/**
 * @brief Enters the editor in new spob mode.
 */
static void sysedit_btnNewAsteroids( unsigned int wid_unused,
                                     const char  *unused )
{
   (void)wid_unused;
   (void)unused;
   const char *title, *caption;
   char       *ret;
   const char *opts[] = {
      _( "Asteroid Field" ),
      _( "Exclusion Zone" ),
      _( "Cancel" ),
   };

   /* See if we want to make a field or exclusion zone. */
   title   = _( "Add asteroid field or exclusion zone?" );
   caption = _( "Do you wish to add an asteroid field or an asteroid exclusion "
                "zone that will remove all asteroids that will appear in it?" );
   dialogue_makeChoice( title, caption, 3 );
   dialogue_addChoice( title, caption, opts[0] );
   dialogue_addChoice( title, caption, opts[1] );
   dialogue_addChoice( title, caption, opts[2] );
   ret = dialogue_runChoice();
   if ( ret == NULL )
      ret = strdup( opts[0] );

   if ( strcmp( ret, opts[0] ) == 0 ) {
      free( ret );
      AsteroidAnchor *ast = &array_grow( &sysedit_sys->asteroids );

      if ( uniedit_diffMode ) {
         char *label = dialogue_inputRaw(
            _( "New Asteroid Field Creation" ), 1, 32,
            _( "What do you want to label the new asteroid field? This is used "
               "when referencing the asteroid field and is not displayed to "
               "the "
               "player." ) );
         if ( label == NULL )
            return;
         for ( int i = 0; i < array_size( sysedit_sys->asteroids ); i++ ) {
            if ( sysedit_sys->asteroids[i].label &&
                 ( strcmp( sysedit_sys->asteroids[i].label, label ) == 0 ) ) {
               dialogue_alert( _( "Asteroid field with label '%s' already "
                                  "exists in system '%s'!" ),
                               label, sysedit_sys->name );
               return;
            }
         }

         /* Create the new unidiff hunk. */
         uniedit_diffCreateSysStr( sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_ADD,
                                   label );
         return;
      }

      asteroid_initAnchor( ast );
      ast->pos.x = sysedit_xpos / sysedit_zoom;
      ast->pos.y = sysedit_ypos / sysedit_zoom;
      asteroids_computeInternals( ast );
   } else if ( strcmp( ret, opts[1] ) == 0 ) {
      free( ret );
      AsteroidExclusion *exc = &array_grow( &sysedit_sys->astexclude );
      if ( uniedit_diffMode ) {
         /* TODO. */
         return;
      }

      memset( exc, 0, sizeof( AsteroidExclusion ) );
      exc->radius = 1000.;
      exc->pos.x  = sysedit_xpos / sysedit_zoom;
      exc->pos.y  = sysedit_ypos / sysedit_zoom;
   } else {
      free( ret );
      return;
   }

   if ( conf.devautosave ) {
      if ( dsys_saveSystem( sysedit_sys ) )
         uniedit_saveError();
   }
}

static UniAttribute_t *sysedit_asteroidsAttr( const AsteroidAnchor *ast )
{
   UniAttribute_t  attr;
   UniAttribute_t *attr_list = array_create( UniAttribute_t );
   attr.name                 = strdup( "label" );
   attr.value                = strdup( ast->label );
   array_push_back( &attr_list, attr );
   return attr_list;
}

static void sysedit_btnRename( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   for ( int i = 0; i < sysedit_nselect; i++ ) {
      Select_t   *sel = &sysedit_select[i];
      const char *prompt;

      if ( sel->type != SELECT_SPOB )
         continue;

      char *name, *oldName, *newName, *filtered;
      Spob *p = sysedit_sys[i].spobs[sel->u.spob];

      /* Get new name. */
      if ( uniedit_diffMode )
         prompt =
            _( "What do you want to rename the spob #r%s#0?\n\n#rNote:#0 this "
               "will only change the display name of the space object." );
      else
         prompt =
            _( "What do you want to rename the spob #r%s#0?\n\n#rNote:#0 this "
               "will rename and copy the space object data file." );
      name = dialogue_input( _( "Rename Spob" ), 1, 32, prompt, p->name );
      if ( name == NULL )
         continue;

      if ( uniedit_diffMode ) {
         sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_DISPLAYNAME,
                                    name ); /* Name is already allocated. */
      } else {
         /* Check for collision. */
         if ( spob_exists( name ) ) {
            dialogue_alert( _( "Space object by the name of #r'%s'#0 already "
                               "exists in the #r'%s'#0 system" ),
                            name, spob_getSystemName( name ) );
            free( name );
            i--;
            continue;
         }

         /* Rename. */
         filtered = strdup( p->filename );
         SDL_asprintf( &oldName, "%s/spob/%s", conf.dev_data_dir,
                       basename( filtered ) );
         free( filtered );

         filtered = uniedit_nameFilter( name );
         SDL_asprintf( &newName, "%s/spob/%s.xml", conf.dev_data_dir,
                       filtered );
         free( filtered );

         if ( rename( oldName, newName ) )
            WARN( _( "Failed to rename '%s' to '%s'!" ), oldName, newName );

         /* Clean up. */
         free( oldName );
         // free( p->filename );

         /* Replace name in stack. */
         spob_rename( p, name );
         // p->filename = newName;

         dsys_saveSystem( sysedit_sys );
         dpl_saveSpob( p );
      }

      /* Rename input if called from edit window. */
      if ( window_existsID( sysedit_widEdit ) )
         window_modifyText( sysedit_widEdit, "txtName", p->name );
   }
}

/**
 * @brief Removes spobs.
 */
static void sysedit_btnRemove( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   char *file;

   if ( uniedit_diffMode ) {
      if ( dialogue_YesNo(
              _( "Remove selected objects (excluding jumps)?" ),
              _( "Objects will be removed through the current diff." ) ) ) {
         for ( int i = 0; i < sysedit_nselect; i++ ) {
            Select_t *sel = &sysedit_select[i];
            if ( sel->type == SELECT_SPOB ) {
               const Spob *sp = sysedit_sys->spobs[sel->u.spob];
               uniedit_diffCreateSysStr( sysedit_sys, HUNK_TYPE_SPOB_REMOVE,
                                         strdup( sp->name ) );
            } else if ( sel->type == SELECT_ASTEROID ) {
               AsteroidAnchor *ast = &sysedit_sys->asteroids[sel->u.asteroid];
               if ( ( ast->label == NULL ) ||
                    ( strcmp( ast->label, "" ) == 0 ) ) {
                  dialogue_alertRaw( _( "Modifying asteroid fields in diff "
                                        "mode is only supported when "
                                        "they "
                                        "have labels. Please set a label via "
                                        "the property editor." ) );
                  return;
               }
               uniedit_diffCreateSysStr(
                  sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_REMOVE, ast->label );
            } /* TODO remove asteroid exclusions. */
            /*else if ( sel->type == SELECT_ASTEXCLUDE ) {
               AsteroidExclusion *exc =
                  &sysedit_sys->astexclude[sel->u.astexclude];
               array_erase( &sysedit_sys->astexclude, exc, exc + 1 );
            }*/
         }
      }
   } else {
      if ( dialogue_YesNo( _( "Remove selected objects (excluding jumps)?" ),
                           _( "This can not be undone." ) ) ) {
         for ( int i = 0; i < sysedit_nselect; i++ ) {
            const Select_t *sel = &sysedit_select[i];
            if ( sel->type == SELECT_SPOB ) {
               const Spob *sp       = sysedit_sys->spobs[sel->u.spob];
               char       *filtered = uniedit_nameFilter( sp->name );
               SDL_asprintf( &file, "%s/spob/%s.xml", conf.dev_data_dir,
                             filtered );
               remove( file );

               free( filtered );
               free( file );

               system_rmSpob( sysedit_sys, sp->name );
            } else if ( sel->type == SELECT_ASTEROID ) {
               AsteroidAnchor *ast = &sysedit_sys->asteroids[sel->u.asteroid];
               asteroid_freeAnchor( ast );
               array_erase( &sysedit_sys->asteroids, ast, ast + 1 );
            } else if ( sel->type == SELECT_ASTEXCLUDE ) {
               AsteroidExclusion *exc =
                  &sysedit_sys->astexclude[sel->u.astexclude];

               array_erase( &sysedit_sys->astexclude, exc, exc + 1 );
            }
         }

         /* Run galaxy modifications. */
         space_reconstructPresences();
         economy_execQueued();
      }
   }
}

/**
 * @brief Resets jump points.
 */
static void sysedit_btnReset( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   if ( uniedit_diffMode ) {
      dialogue_alertRaw(
         _( "Reseting systems is not supported in diff mode!" ) );
      return;
   }

   for ( int i = 0; i < sysedit_nselect; i++ ) {
      Select_t *sel = &sysedit_select[i];
      if ( sel->type == SELECT_JUMPPOINT )
         sysedit_sys[i].jumps[sel->u.jump].flags |= JP_AUTOPOS;
   }

   /* Must reconstruct jumps. */
   systems_reconstructJumps();
}

/**
 * @brief Interface for scaling a system from the system view.
 */
static void sysedit_btnScale( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   char       *str;
   double      s;
   StarSystem *sys;

   if ( uniedit_diffMode ) {
      dialogue_alertRaw(
         _( "Scaling systems is not supported in diff mode!" ) );
      return;
   }

   /* Prompt scale amount. */
   str = dialogue_inputRaw(
      _( "Scale Star System" ), 1, 32,
      _( "By how much do you want to scale the star system?" ) );
   if ( str == NULL )
      return;

   sys = sysedit_sys; /* Comfort. */
   s   = atof( str );
   free( str );

   /* In case screwed up. */
   if ( ( s < 0.1 ) || ( s > 10. ) ) {
      int i = dialogue_YesNo( _( "Scale Star System" ),
                              _( "Are you sure you want to scale the star "
                                 "system by %.2f (from %.2f to %.2f)?" ),
                              s, sys->radius, sys->radius * s );
      if ( i == 0 )
         return;
   }

   sysedit_sysScale( sys, s );
}

/**
 * @brief Scales a system.
 */
void sysedit_sysScale( StarSystem *sys, double factor )
{
   char buf[STRMAX];

   /* Ignore trivial scaling. */
   if ( fabs( factor - 1.0 ) < 1e-5 )
      return;

   /* Scale radius. */
   sys->radius *= factor;
   snprintf( buf, sizeof( buf ), _( "Radius: %.0f" ), sys->radius );
   if ( sysedit_wid > 0 )
      window_modifyText( sysedit_wid, "txtSelected", buf );

   /* Scale spobs. */
   for ( int i = 0; i < array_size( sys->spobs ); i++ ) {
      Spob *p = sys->spobs[i];
      vec2_cset( &p->pos, p->pos.x * factor, p->pos.y * factor );
   }

   /* Scale jumps. */
   for ( int i = 0; i < array_size( sys->jumps ); i++ ) {
      JumpPoint *jp = &sys->jumps[i];
      vec2_cset( &jp->pos, jp->pos.x * factor, jp->pos.y * factor );
   }

   /* Scale asteroids. */
   for ( int i = 0; i < array_size( sys->asteroids ); i++ ) {
      AsteroidAnchor *ast = &sys->asteroids[i];
      vec2_cset( &ast->pos, ast->pos.x * factor, ast->pos.y * factor );
      ast->radius *= factor;
   }
   for ( int i = 0; i < array_size( sys->astexclude ); i++ ) {
      AsteroidExclusion *exc = &sys->astexclude[i];
      vec2_cset( &exc->pos, exc->pos.x * factor, exc->pos.y * factor );
      exc->radius *= factor;
   }

   /* Must reconstruct jumps. */
   systems_reconstructJumps();
}

/**
 * @brief Toggles the grid.
 */
static void sysedit_btnGrid( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;

   sysedit_grid = !sysedit_grid;
}

/**
 * @brief System editor custom widget rendering.
 */
static void sysedit_render( double bx, double by, double w, double h,
                            void *data )
{
   (void)data;
   StarSystem *sys;
   double      x, y, z;

   /* Comfort++. */
   sys = sysedit_sys;
   z   = sysedit_zoom;

   /* Coordinate translation. */
   x = bx - sysedit_xpos + w / 2;
   y = by - sysedit_ypos + h / 2;

   /* First render background with lines. */
   sysedit_renderBG( bx, by, w, h, x, y );

   /* Render spobs. */
   for ( int i = 0; i < array_size( sys->spobs ); i++ ) {
      Spob          *p   = sys->spobs[i];
      const Select_t sel = {
         .type   = SELECT_SPOB,
         .u.spob = i,
      };
      int selected = sysedit_isSelected( &sel );
      /* TODO handle non-sprite rendering. */
      if ( p->gfx_space != NULL )
         sysedit_renderSprite( p->gfx_space, x, y, p->pos.x, p->pos.y, 0, 0,
                               NULL, selected, p->name );
   }

   /* Render jump points. */
   for ( int i = 0; i < array_size( sys->jumps ); i++ ) {
      const glColour *c;
      JumpPoint      *jp  = &sys->jumps[i];
      const Select_t  sel = {
          .type   = SELECT_JUMPPOINT,
          .u.jump = i,
      };
      int selected = sysedit_isSelected( &sel );

      /* Choose colour. */
      c = ( jp->flags & JP_AUTOPOS ) ? &cGreen : NULL;

      /* Render. */
      sysedit_renderSprite( jumppoint_gfx, x, y, jp->pos.x, jp->pos.y, jp->sx,
                            jp->sy, c, selected, jp->target->name );
   }

   /* Render asteroids */
   for ( int i = 0; i < array_size( sys->asteroids ); i++ ) {
      const AsteroidAnchor *ast = &sys->asteroids[i];
      const Select_t        sel = {
                .type       = SELECT_ASTEROID,
                .u.asteroid = i,
      };
      int selected = sysedit_isSelected( &sel );
      sysedit_renderAsteroidsField( x, y, ast, selected );
   }

   /* Render asteroid exclusions */
   for ( int i = 0; i < array_size( sys->astexclude ); i++ ) {
      const AsteroidExclusion *aexcl = &sys->astexclude[i];
      const Select_t           sel   = {
                     .type         = SELECT_ASTEXCLUDE,
                     .u.astexclude = i,
      };
      int selected = sysedit_isSelected( &sel );
      sysedit_renderAsteroidExclusion( x, y, aexcl, selected );
   }

   /* Render safe lanes. */
   SafeLane *safelanes = safelanes_get( -1, 0, sys );
   for ( int i = 0; i < array_size( safelanes ); i++ ) {
      vec2           *posns[2];
      Spob           *pnt;
      JumpPoint      *njp;
      glColour        col;
      const SafeLane *sf = &safelanes[i];

      for ( int j = 0; j < 2; j++ ) {
         switch ( sf->point_type[j] ) {
         case SAFELANE_LOC_SPOB:
            pnt      = spob_getIndex( sf->point_id[j] );
            posns[j] = &pnt->pos;
            break;
         case SAFELANE_LOC_DEST_SYS:
            njp = jump_getTarget( system_getIndex( sf->point_id[j] ), sys );
            posns[j] = &njp->pos;
            break;
         default:
            WARN( _( "Invalid vertex type." ) );
            continue;
         }
      }

      col   = *faction_colour( sf->faction );
      col.a = 0.3;

      /* Get positions and stuff. */
      double x1, y1, x2, y2, ry, rx, r, rw, rh;
      x1 = x + posns[0]->x * z;
      y1 = y + posns[0]->y * z;
      x2 = x + posns[1]->x * z;
      y2 = y + posns[1]->y * z;
      rx = x2 - x1;
      ry = y2 - y1;
      r  = atan2( ry, rx );
      rw = MOD( rx, ry ) / 2.;
      rh = 9.;

      /* Render. */
      glUseProgram( shaders.safelane.program );
      gl_renderShader( ( x1 + x2 ) / 2., ( y1 + y2 ) / 2., rw, rh, r,
                       &shaders.safelane, &col, 1 );
   }
   array_free( safelanes );

   /* Render cursor position. */
   gl_print( &gl_defFontMono, bx + 5., by + 65., &cWhite, "% 9.2f x % 9.2f",
             ( bx + sysedit_mx - x ) / z, ( by + sysedit_my - y ) / z );
}

/**
 * @brief Draws an asteroid field on the map.
 *
 */
static void sysedit_renderAsteroidsField( double bx, double by,
                                          const AsteroidAnchor *ast,
                                          int                   selected )
{
   double tx, ty, z;

   /* Inits. */
   z = sysedit_zoom;

   /* Translate asteroid field center's coords. */
   tx = bx + ast->pos.x * z;
   ty = by + ast->pos.y * z;

   if ( selected ) {
      const glColour csel = COL_ALPHA( cFontBlue, 0.5 );
      gl_renderCircle( tx, ty, ast->radius * sysedit_zoom, &csel, 1 );
   }

   gl_renderCircle( tx, ty, ast->radius * sysedit_zoom, &cOrange, 0 );
   gl_printMidRaw( &gl_smallFont, 200, tx - 100, ty - gl_smallFont.h / 2.,
                   ( selected ) ? &cRed : NULL, -1., _( "Asteroid Field" ) );
}

/**
 * @brief Draws an asteroid exclusion zone on the map.
 *
 */
static void sysedit_renderAsteroidExclusion( double bx, double by,
                                             const AsteroidExclusion *aexcl,
                                             int                      selected )
{
   double          tx, ty, z, r, rr;
   const glColour *col;

   /* Inits. */
   z = sysedit_zoom;

   /* Translate asteroid field center's coords. */
   tx = bx + aexcl->pos.x * z;
   ty = by + aexcl->pos.y * z;
   r  = aexcl->radius * sysedit_zoom;
   rr = r * sin( M_PI / 4. );

   if ( selected ) {
      const glColour csel = COL_ALPHA( cFontBlue, 0.5 );
      gl_renderCircle( tx, ty, r, &csel, 1 );
   }

   col = ( selected ) ? &cWhite : &cRed;

   gl_renderCircle( tx, ty, r, col, 0 );
   gl_renderCross( tx, ty, r, col );
   gl_renderRectEmpty( tx - rr, ty - rr, rr * 2, rr * 2, col );
}

/**
 * @brief Renders the custom widget background.
 */
static void sysedit_renderBG( double bx, double by, double w, double h,
                              double x, double y )
{
   /* Comfort. */
   const double z = sysedit_zoom;
   const double s = 1000.;

   /* Vars */
   double startx, starty, spacing;
   int    nx, ny;

   /* Render blackness. */
   gl_renderRect( bx, by, w, h, &cBlack );

   /* Must have grid activated. */
   if ( !sysedit_grid )
      return;

   /* Draw lines that go through 0,0 */
   gl_renderRect( x - 1., by, 3., h, &cLightBlue );
   gl_renderRect( bx, y - 1., w, 3., &cLightBlue );

   /* Render lines. */
   spacing = s * z;
   startx  = bx + fmod( x - bx, spacing );
   starty  = by + fmod( y - by, spacing );

   nx = lround( w / spacing );
   ny = lround( h / spacing );

   /* Vertical. */
   for ( int i = 0; i < nx; i++ ) {
      double d = startx + ( i * spacing );
      gl_renderLine( d, by, d, by + h, &cBlue );
   }
   /* Horizontal. */
   for ( int i = 0; i < ny; i++ ) {
      double d = starty + ( i * spacing );
      gl_renderLine( bx, d, bx + w, d, &cBlue );
   }

   gl_renderCircle( x, y, sysedit_sys->radius * z, &cLightBlue, 0 );
}

/**
 * @brief Renders a sprite for the custom widget.
 */
static void sysedit_renderSprite( const glTexture *gfx, double bx, double by,
                                  double x, double y, int sx, int sy,
                                  const glColour *c, int selected,
                                  const char *caption )
{
   double tx, ty, z;

   /* Comfort. */
   z = sysedit_zoom;

   /* Selection graphic. */
   if ( selected ) {
      const glColour csel = COL_ALPHA( cFontBlue, 0.5 );
      gl_renderCircle( bx + x * z, by + y * z, tex_sw( gfx ) * z * 1.1, &csel,
                       1 );
   }

   /* Translate coords. */
   tx = bx + ( x - tex_sw( gfx ) / 2. ) * z;
   ty = by + ( y - tex_sh( gfx ) / 2. ) * z;
   /* Blit the spob. */
   gl_renderScaleSprite( gfx, tx, ty, sx, sy, tex_sw( gfx ) * z,
                         tex_sh( gfx ) * z, c );

   /* Display caption. */
   if ( caption != NULL ) {
      const glColour *col;
      if ( selected )
         col = &cRed;
      else
         col = c;
      gl_printMidRaw( &gl_smallFont, tex_sw( gfx ) * z + 100, tx - 50,
                      ty - gl_smallFont.h - 5, col, -1., caption );
   }
}

/**
 * @brief Called when it's de-focused.
 */
static void sysedit_focusLose( unsigned int wid, const char *wgtname )
{
   (void)wid;
   (void)wgtname;
   sysedit_drag = sysedit_dragSel = 0;
}

static int sysedit_mouseTrySelect( const Select_t *sel, double x, double y,
                                   double t, double mx, double my,
                                   SDL_Keymod mod, void ( *func )( void ) )
{
   x *= sysedit_zoom;
   y *= sysedit_zoom;

   if ( ( pow2( mx - x ) + pow2( my - y ) ) > t )
      return 0;

   /* Check if already selected. */
   for ( int j = 0; j < sysedit_nselect; j++ ) {
      if ( !sysedit_selectCmp( sel, &sysedit_select[j] ) )
         continue;

      sysedit_dragSel = 1;
      sysedit_tsel    = *sel;

      /* Check modifier. */
      if ( mod & ( SDL_KMOD_LCTRL | SDL_KMOD_RCTRL ) )
         sysedit_tadd = 0;
      else {
         /* Detect double click to open spob editor. */
         if ( ( SDL_GetTicks() - sysedit_dragTime <
                SYSEDIT_DRAG_THRESHOLD * 2 ) &&
              ( sysedit_moved < SYSEDIT_MOVE_THRESHOLD ) ) {
            if ( func != NULL )
               func();
            sysedit_dragSel = 0;
            return 1;
         }
         sysedit_tadd = -1;
      }
      sysedit_dragTime = SDL_GetTicks();
      sysedit_moved    = 0;
      return 1;
   }

   /* Add the system if not selected. */
   if ( mod & ( SDL_KMOD_LCTRL | SDL_KMOD_RCTRL ) )
      sysedit_selectAdd( sel );
   else {
      sysedit_deselect();
      sysedit_selectAdd( sel );
   }
   sysedit_tsel.type = SELECT_NONE;

   /* Start dragging anyway. */
   sysedit_dragSel  = 1;
   sysedit_dragTime = SDL_GetTicks();
   sysedit_moved    = 0;
   return 1;
}

/**
 * @brief System editor custom widget mouse handling.
 */
static int sysedit_mouse( unsigned int wid, const SDL_Event *event, double mx,
                          double my, double w, double h, double xr, double yr,
                          void *data )
{
   (void)data;
   StarSystem *sys = sysedit_sys;
   SDL_Keymod  mod = SDL_GetModState();

   switch ( event->type ) {

   case SDL_EVENT_MOUSE_WHEEL:
      /* Must be in bounds. */
      if ( ( mx < 0. ) || ( mx > w ) || ( my < 0. ) || ( my > h ) )
         return 0;

      if ( event->wheel.y > 0 )
         sysedit_buttonZoom( 0, "btnZoomIn" );
      else if ( event->wheel.y < 0 )
         sysedit_buttonZoom( 0, "btnZoomOut" );

      return 1;

   case SDL_EVENT_MOUSE_BUTTON_DOWN:
      /* Must be in bounds. */
      if ( ( mx < 0. ) || ( mx > w ) || ( my < 0. ) || ( my > h ) )
         return 0;
      window_setFocus( wid, "cstSysEdit" );

      /* selecting star system */
      mx -= w / 2 - sysedit_xpos;
      my -= h / 2 - sysedit_ypos;

      /* Check spobs. */
      for ( int i = 0; i < array_size( sys->spobs ); i++ ) {
         Spob          *p   = sys->spobs[i];
         const Select_t sel = {
            .type   = SELECT_SPOB,
            .u.spob = i,
         };

         /* Threshold. */
         double t = pow2( p->radius ); /* Radius^2 */
         t *= pow2( 2. * sysedit_zoom );

         /* Try to select. */
         if ( sysedit_mouseTrySelect( &sel, p->pos.x, p->pos.y, t, mx, my, mod,
                                      sysedit_editPnt ) )
            return 1;
      }

      /* Check jump points. */
      for ( int i = 0; i < array_size( sys->jumps ); i++ ) {
         JumpPoint     *jp  = &sys->jumps[i];
         const Select_t sel = {
            .type   = SELECT_JUMPPOINT,
            .u.jump = i,
         };

         /* Threshold. */
         double t = tex_sw( jumppoint_gfx ) * tex_sh( jumppoint_gfx ) /
                    4.; /* Radius^2 */
         t *= pow2( 2. * sysedit_zoom );

         /* Try to select. */
         if ( sysedit_mouseTrySelect( &sel, jp->pos.x, jp->pos.y, t, mx, my,
                                      mod, sysedit_editJump ) )
            return 1;
      }

      /* Check asteroids exclusions. */
      for ( int i = 0; i < array_size( sys->astexclude ); i++ ) {
         AsteroidExclusion *exc = &sys->astexclude[i];
         const Select_t     sel = {
                .type         = SELECT_ASTEXCLUDE,
                .u.astexclude = i,
         };
         double t = pow2( exc->radius * sysedit_zoom );

         /* Try to select. */
         if ( sysedit_mouseTrySelect( &sel, exc->pos.x, exc->pos.y, t, mx, my,
                                      mod, sysedit_editExclusion ) )
            return 1;
      }

      /* Check asteroids. */
      for ( int i = 0; i < array_size( sys->asteroids ); i++ ) {
         AsteroidAnchor *ast = &sys->asteroids[i];
         const Select_t  sel = {
             .type       = SELECT_ASTEROID,
             .u.asteroid = i,
         };
         double t = pow2( ast->radius * sysedit_zoom );

         /* Try to select. */
         if ( sysedit_mouseTrySelect( &sel, ast->pos.x, ast->pos.y, t, mx, my,
                                      mod, sysedit_editAsteroids ) )
            return 1;
      }

      /* Start dragging. */
      if ( !( mod & ( SDL_KMOD_LCTRL | SDL_KMOD_RCTRL ) ) ) {
         sysedit_drag      = 1;
         sysedit_dragTime  = SDL_GetTicks();
         sysedit_moved     = 0;
         sysedit_tsel.type = SELECT_NONE;
      }
      return 1;

   case SDL_EVENT_MOUSE_BUTTON_UP:
      if ( sysedit_drag ) {
         if ( ( SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD ) &&
              ( sysedit_moved < SYSEDIT_MOVE_THRESHOLD ) ) {
            if ( sysedit_tsel.type == SELECT_NONE )
               sysedit_deselect();
            else
               sysedit_selectAdd( &sysedit_tsel );
         }
         sysedit_drag = 0;

         if ( conf.devautosave ) {
            int ret = dsys_saveSystem( sysedit_sys );
            for ( int i = 0; i < sysedit_nselect; i++ )
               if ( sysedit_select[i].type == SELECT_SPOB )
                  ret |= dpl_saveSpob( sys->spobs[sysedit_select[i].u.spob] );
            if ( ret )
               uniedit_saveError();
         }
      }
      if ( sysedit_dragSel ) {
         if ( ( SDL_GetTicks() - sysedit_dragTime < SYSEDIT_DRAG_THRESHOLD ) &&
              ( sysedit_moved < SYSEDIT_MOVE_THRESHOLD ) &&
              ( sysedit_tsel.type != SELECT_NONE ) ) {
            if ( sysedit_tadd == 0 )
               sysedit_selectRm( &sysedit_tsel );
            else {
               sysedit_deselect();
               sysedit_selectAdd( &sysedit_tsel );
            }
         }
         sysedit_dragSel = 0;

         /* Save all spobs in our selection - their positions might have
          * changed. */
         if ( conf.devautosave ) {
            int ret = dsys_saveSystem( sysedit_sys );
            for ( int i = 0; i < sysedit_nselect; i++ )
               if ( sysedit_select[i].type == SELECT_SPOB )
                  ret |= dpl_saveSpob( sys->spobs[sysedit_select[i].u.spob] );
            if ( ret )
               uniedit_saveError();
         }
      }
      break;

   case SDL_EVENT_MOUSE_MOTION:
      /* Update mouse positions. */
      sysedit_mx = mx;
      sysedit_my = my;

      /* Handle dragging. */
      if ( sysedit_drag ) {
         /* axis is inverted */
         sysedit_xpos -= xr;
         sysedit_ypos += yr;

         /* Update mouse movement. */
         sysedit_moved += ABS( xr ) + ABS( yr );
      }
      /* Dragging selection around. */
      else if ( sysedit_dragSel && ( sysedit_nselect > 0 ) ) {
         if ( ( sysedit_moved > SYSEDIT_MOVE_THRESHOLD ) ||
              ( SDL_GetTicks() - sysedit_dragTime > SYSEDIT_DRAG_THRESHOLD ) ) {
            double xmove = xr / sysedit_zoom;
            double ymove = -yr / sysedit_zoom;
            for ( int i = 0; i < sysedit_nselect; i++ ) {
               Spob              *p;
               JumpPoint         *jp;
               AsteroidAnchor    *ast;
               AsteroidExclusion *exc;
               Select_t          *sel = &sysedit_select[i];

               switch ( sel->type ) {
               case SELECT_SPOB:
                  p = sys->spobs[sel->u.spob];
                  if ( uniedit_diffMode ) {
                     sysedit_diffCreateSpobFloat( p, HUNK_TYPE_SPOB_POS_X,
                                                  p->pos.x + xmove );
                     sysedit_diffCreateSpobFloat( p, HUNK_TYPE_SPOB_POS_Y,
                                                  p->pos.y + ymove );
                  } else {
                     p->pos.x += xmove;
                     p->pos.y += ymove;
                  }
                  break;

               case SELECT_JUMPPOINT:
                  jp = &sys->jumps[sel->u.jump];
                  if ( uniedit_diffMode ) {
                     /* TODO diff. */
                     dialogue_alertRaw( _( "Editing jump points is not yet "
                                           "supported in diff mode." ) );
                  } else {
                     jp->flags &= ~( JP_AUTOPOS );
                     jp->pos.x += xmove;
                     jp->pos.y += ymove;
                  }
                  break;

               case SELECT_ASTEROID:
                  ast = &sys->asteroids[sel->u.asteroid];
                  if ( uniedit_diffMode ) {
                     if ( ( ast->label == NULL ) ||
                          ( strcmp( ast->label, "" ) == 0 ) ) {
                        dialogue_alertRaw(
                           _( "Modifying asteroid fields in diff mode is only "
                              "supported when "
                              "they "
                              "have labels. Please set a label via the "
                              "property editor." ) );
                     } else {
                        uniedit_diffCreateSysFloatAttr(
                           sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_POS_X,
                           ast->pos.x + xmove, sysedit_asteroidsAttr( ast ) );
                        uniedit_diffCreateSysFloatAttr(
                           sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_POS_Y,
                           ast->pos.y + ymove, sysedit_asteroidsAttr( ast ) );
                     }
                  } else {
                     ast->pos.x += xmove;
                     ast->pos.y += ymove;
                  }
                  break;

               case SELECT_ASTEXCLUDE:
                  exc = &sys->astexclude[sel->u.astexclude];
                  if ( uniedit_diffMode ) {
                     /* TODO diff. */
                  } else {
                     exc->pos.x += xmove;
                     exc->pos.y += ymove;
                  }
                  break;
               }
            }
         }

         /* Update mouse movement. */
         sysedit_moved += ABS( xr ) + ABS( yr );
      }
      break;
   }

   return 0;
}

/**
 * @brief Handles the button zoom clicks.
 *
 *    @param wid Unused.
 *    @param str Name of the button creating the event.
 */
static void sysedit_buttonZoom( unsigned int wid, const char *str )
{
   (void)wid;

   /* Transform coords to normal. */
   sysedit_xpos /= sysedit_zoom;
   sysedit_ypos /= sysedit_zoom;

   /* Apply zoom. */
   if ( strcmp( str, "btnZoomIn" ) == 0 ) {
      sysedit_zoom *= SYSEDIT_ZOOM_STEP;
      sysedit_zoom =
         MIN( pow( SYSEDIT_ZOOM_STEP, SYSEDIT_ZOOM_MAX ), sysedit_zoom );
   } else if ( strcmp( str, "btnZoomOut" ) == 0 ) {
      sysedit_zoom /= SYSEDIT_ZOOM_STEP;
      sysedit_zoom =
         MAX( pow( SYSEDIT_ZOOM_STEP, SYSEDIT_ZOOM_MIN ), sysedit_zoom );
   }

   /* Transform coords back. */
   sysedit_xpos *= sysedit_zoom;
   sysedit_ypos *= sysedit_zoom;
}

/**
 * @brief Deselects everything.
 */
static void sysedit_deselect( void )
{
   if ( sysedit_nselect > 0 )
      free( sysedit_select );
   sysedit_select  = NULL;
   sysedit_nselect = 0;
   sysedit_mselect = 0;

   /* Button check. */
   sysedit_checkButtons();
}

/**
 * @brief Checks to see which buttons should be active and the likes.
 */
static void sysedit_checkButtons( void )
{
   int sel_spob, sel_jump, sel_asteroid, sel_exclusion;

   /* See if a spob or jump is selected. */
   sel_spob      = 0;
   sel_jump      = 0;
   sel_asteroid  = 0;
   sel_exclusion = 0;
   for ( int i = 0; i < sysedit_nselect; i++ ) {
      const Select_t *sel = &sysedit_select[i];
      switch ( sel->type ) {
      case SELECT_SPOB:
         sel_spob++;
         break;
      case SELECT_JUMPPOINT:
         sel_spob++;
         break;
      case SELECT_ASTEROID:
         sel_asteroid++;
         break;
      case SELECT_ASTEXCLUDE:
         sel_exclusion++;
         break;
      }
   }

   /* Spob dependent. */
   if ( sel_spob || sel_asteroid || sel_exclusion )
      window_enableButton( sysedit_wid, "btnRemove" );
   else
      window_disableButton( sysedit_wid, "btnRemove" );

   /* Jump dependent. */
   if ( sel_jump )
      window_enableButton( sysedit_wid, "btnReset" );
   else
      window_disableButton( sysedit_wid, "btnReset" );

   /* Editor - just one spob. */
   if ( sysedit_nselect == 1 )
      window_enableButton( sysedit_wid, "btnEdit" );
   else
      window_disableButton( sysedit_wid, "btnEdit" );
}

/**
 * @brief Adds a system to the selection.
 */
static void sysedit_selectAdd( const Select_t *sel )
{
   /* Allocate if needed. */
   if ( sysedit_mselect < sysedit_nselect + 1 ) {
      if ( sysedit_mselect == 0 )
         sysedit_mselect = 1;
      sysedit_mselect *= 2;
      sysedit_select =
         realloc( sysedit_select, sizeof( Select_t ) * sysedit_mselect );
   }

   /* Add system. */
   sysedit_select[sysedit_nselect] = *sel;
   sysedit_nselect++;

   /* Button check. */
   sysedit_checkButtons();
}

/**
 * @brief Removes a system from the selection.
 */
static void sysedit_selectRm( const Select_t *sel )
{
   for ( int i = 0; i < sysedit_nselect; i++ ) {
      if ( sysedit_selectCmp( &sysedit_select[i], sel ) ) {
         sysedit_nselect--;
         memmove( &sysedit_select[i], &sysedit_select[i + 1],
                  sizeof( Select_t ) * ( sysedit_nselect - i ) );
         /* Button check. */
         sysedit_checkButtons();
         return;
      }
   }
   WARN( _( "Trying to deselect item that is not in selection!" ) );
}

/**
 * @brief Compares two selections to see if they are the same.
 *
 *    @return 1 if both selections are the same.
 */
static int sysedit_selectCmp( const Select_t *a, const Select_t *b )
{
   return ( memcmp( a, b, sizeof( Select_t ) ) == 0 );
}

/**
 * @brief Check to see if something is selected.
 */
static int sysedit_isSelected( const Select_t *sel )
{
   for ( int i = 0; i < sysedit_nselect; i++ )
      if ( sysedit_selectCmp( sel, &sysedit_select[i] ) )
         return 1;
   return 0;
}

/**
 * @brief Edits a spob.
 */
static void sysedit_editPnt( void )
{
   unsigned int wid;
   int          x, y, w, l, bw;
   char         buf[STRMAX_SHORT], title[128];
   const char  *s;
   Spob        *p;

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   /* Create the window. */
   snprintf( title, sizeof( title ), _( "Space Object Property Editor - %s" ),
             p->name );
   wid = window_create( "wdwSysEditPnt", title, -1, -1, SYSEDIT_EDIT_WIDTH,
                        SYSEDIT_EDIT_HEIGHT );
   sysedit_widEdit = wid;

   window_setCancel( wid, sysedit_editPntClose );

   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   /* Rename button. */
   y = -40;
   snprintf( buf, sizeof( buf ), "%s ", _( "Name:" ) );
   w = gl_printWidthRaw( NULL, buf );
   window_addText( wid, 20, y, 180, 15, 0, "txtNameLabel", &gl_smallFont, NULL,
                   buf );
   snprintf( buf, sizeof( buf ), "%s", p->name );
   window_addText( wid, 20 + w, y, 180, 15, 0, "txtName", &gl_smallFont, NULL,
                   buf );
   window_addButton( wid, -20, y - gl_defFont.h / 2. + BUTTON_HEIGHT / 2., bw,
                     BUTTON_HEIGHT, "btnRename", _( "Rename" ),
                     sysedit_btnRename );
   window_addButton(
      wid, -20 - 15 - bw, y - gl_defFont.h / 2. + BUTTON_HEIGHT / 2., bw,
      BUTTON_HEIGHT, "btnFaction", _( "Faction" ), sysedit_btnFaction );

   y -= gl_defFont.h + 5;

   snprintf( buf, sizeof( buf ), "%s ", _( "Faction:" ) );
   w = gl_printWidthRaw( NULL, buf );
   window_addText( wid, 20, y, 180, 15, 0, "txtFactionLabel", &gl_smallFont,
                   NULL, buf );
   snprintf( buf, sizeof( buf ), "%s",
             p->presence.faction >= 0 ? faction_name( p->presence.faction )
                                      : _( "None" ) );
   window_addText( wid, 20 + w, y, 180, 15, 0, "txtFaction", &gl_smallFont,
                   NULL, buf );
   y -= gl_defFont.h + 5;

   /* Input widgets and labels. */
   x = 20;
   s = _( "Population" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPop", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 80, 20, "inpPop", 12, 1, NULL );
   window_setInputFilter( wid, "inpPop", INPUT_FILTER_NUMBER );
   x += 80 + 10;

   s = _( "Class" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtClass", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 30, 20, "inpClass", 16, 1, NULL );
   x += 30 + 10;

   s = _( "Lua" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtLua", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 180, 20, "inpLua", 50, 1, NULL );
   y -= gl_defFont.h + 15;

   /* Second row. */
   x = 20;
   s = _( "Base Presence" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPresenceBase", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 50, 20, "inpPresenceBase", 5, 1, NULL );
   window_setInputFilter( wid, "inpPresenceBase", INPUT_FILTER_NUMBER );
   x += 50 + 10;

   s = _( "Bonus Presence" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPresenceBonus", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 50, 20, "inpPresenceBonus", 5, 1,
                    NULL );
   window_setInputFilter( wid, "inpPresenceBonus", INPUT_FILTER_NUMBER );
   x += 50 + 10;

   s = p_( "sysedit", "Range" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtPresenceRange", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 30, 20, "inpPresenceRange", 1, 1,
                    NULL );
   window_setInputFilter( wid, "inpPresenceRange", INPUT_FILTER_NUMBER );
   // x += 30 + 10;

   x = 250;
   y -= gl_defFont.h + 15;
   s = _( "hide" );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtHide", NULL, NULL, s );
   window_addInput( wid, x += l + 5, y, 50, 20, "inpHide", 4, 1, NULL );
   window_setInputFilter( wid, "inpHide", INPUT_FILTER_NUMBER );
   // x += 50 + 10;

   /* Tags. */
   x = 250;
   y -= gl_defFont.h + 20;
   l = scnprintf( buf, sizeof( buf ), "#n%s#0", _( "Tags:" ) );
   for ( int i = 0; i < array_size( p->tags ); i++ )
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s %s",
                      ( ( i > 0 ) ? "," : "" ), p->tags[i] );
   window_addText( wid, x, y, 240, 100, 0, "txtTags", NULL, NULL, buf );

   /* Bottom buttons. */
   window_addButton( wid, -20 - bw * 3 - 15 * 3, 35 + BUTTON_HEIGHT, bw,
                     BUTTON_HEIGHT, "btnRmService", _( "Rm Service" ),
                     sysedit_btnRmService );
   window_addButton( wid, -20 - bw * 2 - 15 * 2, 35 + BUTTON_HEIGHT, bw,
                     BUTTON_HEIGHT, "btnAddService", _( "Add Service" ),
                     sysedit_btnAddService );
   window_addButton( wid, -20 - bw - 15, 35 + BUTTON_HEIGHT, bw, BUTTON_HEIGHT,
                     "btnEditTech", _( "Edit Tech" ), sysedit_btnTechEdit );
   window_addButton( wid, -20, 35 + BUTTON_HEIGHT, bw, BUTTON_HEIGHT,
                     "btnEditTags", _( "Edit Tags" ), sysedit_btnTagsEdit );
   window_addButton( wid, -20 - bw * 3 - 15 * 3, 20, bw, BUTTON_HEIGHT,
                     "btnDesc", _( "Description" ), sysedit_spobDesc );
   window_addButton( wid, -20 - bw * 2 - 15 * 2, 20, bw, BUTTON_HEIGHT,
                     "btnLandGFX", _( "Land GFX" ), sysedit_spobGFX );
   window_addButton( wid, -20 - bw - 15, 20, bw, BUTTON_HEIGHT, "btnSpaceGFX",
                     _( "Space GFX" ), sysedit_spobGFX );
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     sysedit_editPntClose );

   /* Load current values. */
   snprintf( buf, sizeof( buf ), "%lf", p->population );
   window_setInput( wid, "inpPop", buf );
   snprintf( buf, sizeof( buf ), "%s", p->class );
   window_setInput( wid, "inpClass", buf );
   window_setInput( wid, "inpLua", p->lua_file );
   snprintf( buf, sizeof( buf ), "%g", p->presence.base );
   window_setInput( wid, "inpPresenceBase", buf );
   snprintf( buf, sizeof( buf ), "%g", p->presence.bonus );
   window_setInput( wid, "inpPresenceBonus", buf );
   snprintf( buf, sizeof( buf ), "%d", p->presence.range );
   window_setInput( wid, "inpPresenceRange", buf );
   snprintf( buf, sizeof( buf ), "%g", p->hide );
   window_setInput( wid, "inpHide", buf );

   /* Generate the list. */
   sysedit_genServicesList( wid );
}

/**
 * @brief Updates the jump point checkboxes.
 */
static void jp_type_check_hidden_update( unsigned int wid, const char *str )
{
   (void)str;
   if ( jp_hidden == 0 ) {
      jp_hidden = 1;
      jp_exit   = 0;
   } else
      jp_hidden = 0;
   window_checkboxSet( wid, "chkHidden", jp_hidden );
   window_checkboxSet( wid, "chkExit", jp_exit );
}

/**
 * @brief Updates the jump point checkboxes.
 */
static void jp_type_check_exit_update( unsigned int wid, const char *str )
{
   (void)str;
   if ( jp_exit == 0 ) {
      jp_exit   = 1;
      jp_hidden = 0;
   } else
      jp_exit = 0;
   window_checkboxSet( wid, "chkHidden", jp_hidden );
   window_checkboxSet( wid, "chkExit", jp_exit );
}

/**
 * @brief Updates the jump point checkboxes.
 */
static void jp_type_check_nolanes_update( unsigned int wid, const char *str )
{
   int        s = window_checkboxState( wid, str );
   JumpPoint *j = &sysedit_sys->jumps[sysedit_select[0].u.jump];
   if ( s )
      jp_setFlag( j, JP_NOLANES );
   else
      jp_rmFlag( j, JP_NOLANES );
}

/**
 * @brief Edits a jump.
 */
static void sysedit_editJump( void )
{
   unsigned int wid;
   int          x, y, w, l, bw;
   char         buf[STRMAX_SHORT];
   const char  *s;
   JumpPoint   *j = &sysedit_sys->jumps[sysedit_select[0].u.jump];

   /* Create the window. */
   wid = window_create( "wdwJumpPointEditor", _( "Jump Point Editor" ), -1, -1,
                        SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   sysedit_widEdit = wid;

   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   /* Target lable. */
   y = -40;
   snprintf( buf, sizeof( buf ), _( "Target: " ) );
   w = gl_printWidthRaw( NULL, buf );
   window_addText( wid, 20, y, 180, 15, 0, "txtTargetLabel", &gl_smallFont,
                   NULL, buf );
   snprintf( buf, sizeof( buf ), "%s", j->target->name );
   window_addText( wid, 20 + w, y, 180, 15, 0, "txtName", &gl_smallFont, NULL,
                   buf );

   y -= gl_defFont.h + 10;

   /* Input widgets and labels. */
   x = 20;

   /* Initial checkbox state */
   jp_hidden = 0;
   jp_exit   = 0;
   if ( jp_isFlag( j, JP_HIDDEN ) )
      jp_hidden = 1;
   else if ( jp_isFlag( j, JP_EXITONLY ) )
      jp_exit = 1;
   /* Create check boxes. */
   window_addCheckbox( wid, x, y, 100, 20, "chkHidden", _( "Hidden" ),
                       jp_type_check_hidden_update, jp_hidden );
   y -= 20;
   window_addCheckbox( wid, x, y, 100, 20, "chkExit", _( "Exit only" ),
                       jp_type_check_exit_update, jp_exit );
   y -= 20;
   window_addCheckbox( wid, x, y, 100, 20, "chkNolanes", _( "No lanes" ),
                       jp_type_check_nolanes_update,
                       jp_isFlag( j, JP_NOLANES ) );
   y -= 30;

   s = _( "Hide" ); /* TODO: if inpType == 0 disable hide box */
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtHide", NULL, NULL, s );
   window_addInput( wid, x + l + 8, y, 50, 20, "inpHide", 4, 1, NULL );
   window_setInputFilter( wid, "inpHide", INPUT_FILTER_NUMBER );
   // x += 50 + 10;

   /* Bottom buttons. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     sysedit_editJumpClose );

   /* Load current values. */
   snprintf( buf, sizeof( buf ), "%g", j->hide );
   window_setInput( wid, "inpHide", buf );
}

/**
 * @brief Closes the jump editor, saving the changes made.
 */
static void sysedit_editJumpClose( unsigned int wid, const char *unused )
{
   (void)unused;
   JumpPoint *j = &sysedit_sys->jumps[sysedit_select[0].u.jump];

   if ( uniedit_diffMode ) {
      /* TODO implement. */
      dialogue_alertRaw(
         _( "Editing jump points is not yet supported in diff mode." ) );
      window_close( wid, unused );
      return;
   }

   /* TODO add diff support. */
   if ( jp_hidden == 1 ) {
      jp_setFlag( j, JP_HIDDEN );
      jp_rmFlag( j, JP_EXITONLY );
   } else if ( jp_exit == 1 ) {
      jp_setFlag( j, JP_EXITONLY );
      jp_rmFlag( j, JP_HIDDEN );
   } else {
      jp_rmFlag( j, JP_HIDDEN );
      jp_rmFlag( j, JP_EXITONLY );
   }
   j->hide = atof( window_getInput( sysedit_widEdit, "inpHide" ) );

   window_close( wid, unused );
}

/**
 * @brief Opens the asteroid editor.
 */
static void sysedit_editAsteroids( void )
{
   unsigned int          wid;
   int                   x, y, l, bw;
   char                  buf[STRMAX_SHORT];
   const char           *s;
   const AsteroidAnchor *ast =
      &sysedit_sys->asteroids[sysedit_select[0].u.asteroid];

   /* Create the window. */
   wid = window_create( "wdwAsteroidsEditor", _( "Asteroid Field Editor" ), -1,
                        -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   sysedit_widEdit = wid;
   window_setCancel( wid, sysedit_editAsteroidsClose );

   /* Input widgets and labels. */
   x = 20;

   /* Add some inputs. */
   y = -40;
   s = _( "Label: " );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtLabel", NULL, NULL, s );
   if ( uniedit_diffMode )
      window_addText( wid, x + l + 8, y, 80, 20, 1, "txtInpLabel", NULL, NULL,
                      ast->label );
   else
      window_addInput( wid, x + l + 8, y, 80, 20, "inpLabel", 10, 1, NULL );
   x += l + 20 + 80 + 8;
   s = _( "Density: " );
   l = gl_printWidthRaw( NULL, s );
   if ( x + l > SYSEDIT_EDIT_WIDTH - 20 ) {
      x = 20;
      y -= 30;
   }
   window_addText( wid, x, y, l, 20, 1, "txtDensity", NULL, NULL, s );
   window_addInput( wid, x + l + 8, y, 80, 20, "inpDensity", 10, 1, NULL );
   window_setInputFilter( wid, "inpDensity", INPUT_FILTER_NUMBER );
   x += l + 20 + 80 + 8;
   s = _( "Radius: " );
   l = gl_printWidthRaw( NULL, s );
   if ( x + l > SYSEDIT_EDIT_WIDTH - 20 ) {
      x = 20;
      y -= 30;
   }
   window_addText( wid, x, y, l, 20, 1, "txtInput", NULL, NULL, s );
   window_addInput( wid, x + l + 8, y, 80, 20, "inpRadius", 10, 1, NULL );
   window_setInputFilter( wid, "inpRadius", INPUT_FILTER_NUMBER );
   x += l + 20 + 80 + 8;
   s = _( "Max Speed: " );
   l = gl_printWidthRaw( NULL, s );
   if ( x + l > SYSEDIT_EDIT_WIDTH - 20 ) {
      x = 20;
      y -= 30;
   }
   window_addText( wid, x, y, l, 20, 1, "txtMaxspeed", NULL, NULL, s );
   window_addInput( wid, x + l + 8, y, 80, 20, "inpMaxspeed", 10, 1, NULL );
   window_setInputFilter( wid, "inpMaxspeed", INPUT_FILTER_NUMBER );
   x += l + 20 + 80 + 8;
   s = _( "Accel: " );
   l = gl_printWidthRaw( NULL, s );
   if ( x + l > SYSEDIT_EDIT_WIDTH - 20 ) {
      x = 20;
      y -= 30;
   }
   window_addText( wid, x, y, l, 20, 1, "txtAccel", NULL, NULL, s );
   window_addInput( wid, x + l + 8, y, 80, 20, "inpAccel", 10, 1, NULL );
   window_setInputFilter( wid, "inpAccel", INPUT_FILTER_NUMBER );
   y -= 40;

   /* Button width. */
   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 2.;

   /* List Captions. */
   window_addText( wid, 20, y, bw, gl_smallFont.h, 1, "txtAsteroidsHave", NULL,
                   NULL, _( "Asteroids" ) );
   window_addText( wid, 20 + bw + 15, y, bw, gl_smallFont.h, 1,
                   "txtAsteroidsAvailable", NULL, NULL, _( "Available" ) );

   /* Button width. */
   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   /* Bottom buttons. */
   window_addButton( wid, 20, 20, bw, BUTTON_HEIGHT, "btnRmAsteroid",
                     _( "Rm Asteroid" ), sysedit_btnRmAsteroid );
   window_addButton( wid, 20 + bw + 15, 20, bw, BUTTON_HEIGHT, "btnAddAsteroid",
                     _( "Add Asteroid" ), sysedit_btnAddAsteroid );
   window_addButton( wid, 20 + 2 * ( bw + 15 ), 20, bw, BUTTON_HEIGHT,
                     "btnDelete", _( "Delete" ), sysedit_btnAsteroidsDelete );
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     sysedit_editAsteroidsClose );

   /* List to choose the different asteroids that appear. Has to be created
    * after buttons. */
   sysedit_genAsteroidsList( wid );

   /* Load current values. */
   if ( !uniedit_diffMode && ( ast->label != NULL ) )
      window_setInput( wid, "inpLabel", ast->label );
   snprintf( buf, sizeof( buf ), "%g", ast->density );
   window_setInput( wid, "inpDensity", buf );
   snprintf( buf, sizeof( buf ), "%g", ast->radius );
   window_setInput( wid, "inpRadius", buf );
   snprintf( buf, sizeof( buf ), "%g", ast->maxspeed );
   window_setInput( wid, "inpMaxspeed", buf );
   snprintf( buf, sizeof( buf ), "%g", ast->accel );
   window_setInput( wid, "inpAccel", buf );
}

static void sysedit_genAsteroidsList( unsigned int wid )
{
   int                   hpos, apos, nhave, navail;
   int                   x, y, w, h;
   const AsteroidAnchor *ast =
      &sysedit_sys->asteroids[sysedit_select[0].u.asteroid];
   const AsteroidTypeGroup *astgroups;
   char                   **have, **available;
   int                      wx, wy, ww, wh;

   window_posWidget( wid, "txtAsteroidsHave", &wx, &wy );
   window_dimWidget( wid, "txtAsteroidsHave", &ww, &wh );

   hpos = apos = -1;
   if ( widget_exists( wid, "lstAsteroidsHave" ) &&
        widget_exists( wid, "lstAsteroidsAvailable" ) ) {
      hpos = toolkit_getListPos( wid, "lstAsteroidsHave" );
      apos = toolkit_getListPos( wid, "lstAsteroidsAvailable" );
      window_destroyWidget( wid, "lstAsteroidsHave" );
      window_destroyWidget( wid, "lstAsteroidsAvailable" );
   }

   /* Set up positions. */
   w = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 2.;
   h = 200;
   x = 20;
   y = wy + wh - 15 - h;

   /* Find and add used asteroids. */
   nhave = array_size( ast->groups );
   if ( nhave > 0 ) {
      have = malloc( sizeof( char * ) * nhave );
      for ( int i = 0; i < nhave; i++ )
         have[i] = strdup( ast->groups[i]->name );
      window_enableButton( wid, "btnRmAsteroid" );
   } else {
      have = NULL;
      window_disableButton( wid, "btnRmAsteroid" );
   }
   window_addList( wid, x, y, w, h, "lstAsteroidsHave", have, nhave, 0, NULL,
                   sysedit_btnRmAsteroid );
   x += w + 15;

   /* Load all asteroid types. */
   astgroups = astgroup_getAll();
   navail    = array_size( astgroups );
   if ( navail > 0 ) {
      available = malloc( sizeof( char * ) * navail );
      for ( int i = 0; i < navail; i++ )
         available[i] = strdup( astgroups[i].name );
      qsort( available, navail, sizeof( char * ), strsort );
      window_enableButton( wid, "btnAddAsteroid" );
   } else {
      available = NULL;
      window_disableButton( wid, "btnAddAsteroid" );
   }
   window_addList( wid, x, y, w, h, "lstAsteroidsAvailable", available, navail,
                   0, NULL, sysedit_btnAddAsteroid );

   /* Restore positions. */
   if ( hpos != -1 && apos != -1 ) {
      toolkit_setListPos( wid, "lstAsteroidsHave", hpos );
      toolkit_setListPos( wid, "lstAsteroidsAvailable", apos );
   }
}

static void sysedit_btnRmAsteroid( unsigned int wid, const char *unused )
{
   (void)unused;
   int             pos = toolkit_getListPos( wid, "lstAsteroidsHave" );
   AsteroidAnchor *ast;
   if ( pos < 0 )
      return;
   if ( array_size( sysedit_sys->asteroids ) <= 0 )
      return;
   ast = &sysedit_sys->asteroids[sysedit_select[0].u.asteroid];

   if ( uniedit_diffMode ) {
      if ( ( ast->label == NULL ) || ( strcmp( ast->label, "" ) == 0 ) ) {
         dialogue_alertRaw(
            _( "Modifying asteroid fields in diff mode is only supported when "
               "they "
               "have labels. Please set a label via the property editor." ) );
         return;
      }
      uniedit_diffCreateSysStrAttr(
         sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_REMOVE_TYPE,
         ast->groups[pos]->name, sysedit_asteroidsAttr( ast ) );
   } else {
      if ( array_size( ast->groups ) > 0 )
         array_erase( &ast->groups, &ast->groups[pos], &ast->groups[pos + 1] );
   }

   sysedit_genAsteroidsList( wid );
}

static void sysedit_btnAddAsteroid( unsigned int wid, const char *unused )
{
   (void)unused;
   const char     *selected = toolkit_getList( wid, "lstAsteroidsAvailable" );
   AsteroidAnchor *ast = &sysedit_sys->asteroids[sysedit_select[0].u.asteroid];
   AsteroidTypeGroup *grp = astgroup_getName( selected );
   /* Failed to add. */
   if ( grp == NULL )
      return;
   if ( uniedit_diffMode ) {
      if ( ( ast->label == NULL ) || ( strcmp( ast->label, "" ) == 0 ) ) {
         dialogue_alertRaw(
            _( "Modifying asteroid fields in diff mode is only supported when "
               "they "
               "have labels. Please set a label via the property editor." ) );
         return;
      }
      uniedit_diffCreateSysStrAttr( sysedit_sys,
                                    HUNK_TYPE_SSYS_ASTEROIDS_ADD_TYPE,
                                    grp->name, sysedit_asteroidsAttr( ast ) );
   } else {
      array_push_back( &ast->groups, grp );
   }

   sysedit_genAsteroidsList( wid );
}

static void sysedit_btnAsteroidsDelete( unsigned int wid, const char *unused )
{
   int i = dialogue_YesNo(
      _( "Remove Asteroid Field" ),
      _( "Are you sure you want to remove this asteroid field?" ) );
   if ( i == 0 )
      return;

   AsteroidAnchor *ast = &sysedit_sys->asteroids[sysedit_select[0].u.asteroid];

   if ( uniedit_diffMode ) {
      if ( ( ast->label == NULL ) || ( strcmp( ast->label, "" ) == 0 ) ) {
         dialogue_alertRaw(
            _( "Modifying asteroid fields in diff mode is only supported when "
               "they "
               "have labels. Please set a label via the property editor." ) );
         return;
      }
      uniedit_diffCreateSysStr( sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_REMOVE,
                                ast->label );
      window_close( wid, unused );
      return;
   }

   asteroid_freeAnchor( ast );
   array_erase( &sysedit_sys->asteroids, ast, ast + 1 );

   if ( conf.devautosave ) {
      if ( dsys_saveSystem( sysedit_sys ) )
         uniedit_saveError();
   }

   window_close( wid, unused );
}

static void sysedit_editAsteroidsClose( unsigned int wid, const char *unused )
{
   AsteroidAnchor *ast = &sysedit_sys->asteroids[sysedit_select[0].u.asteroid];

   if ( uniedit_diffMode ) {
      double density, radius, maxspeed, accel;
      if ( ( ast->label == NULL ) || ( strcmp( ast->label, "" ) == 0 ) ) {
         dialogue_alertRaw(
            _( "Editing asteroids in diff mode is only supported when they "
               "have labels. Please set a label via the property editor." ) );
         window_close( wid, unused );
      }
      density = atof( window_getInput( sysedit_widEdit, "inpDensity" ) );
      if ( ast->density - density > DOUBLE_TOL )
         uniedit_diffCreateSysFloatAttr(
            sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_DENSITY, density,
            sysedit_asteroidsAttr( ast ) );
      radius = atof( window_getInput( sysedit_widEdit, "inpRadius" ) );
      if ( ast->radius - radius > DOUBLE_TOL )
         uniedit_diffCreateSysFloatAttr(
            sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_RADIUS, density,
            sysedit_asteroidsAttr( ast ) );
      maxspeed = atof( window_getInput( sysedit_widEdit, "inpMaxspeed" ) );
      if ( ast->maxspeed - maxspeed > DOUBLE_TOL )
         uniedit_diffCreateSysFloatAttr(
            sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_MAXSPEED, density,
            sysedit_asteroidsAttr( ast ) );
      accel = atof( window_getInput( sysedit_widEdit, "inpAccel" ) );
      if ( ast->accel - accel > DOUBLE_TOL )
         uniedit_diffCreateSysFloatAttr(
            sysedit_sys, HUNK_TYPE_SSYS_ASTEROIDS_ACCEL, density,
            sysedit_asteroidsAttr( ast ) );
      return;
   }
   const char *label = window_getInput( sysedit_widEdit, "inpLabel" );
   free( ast->label );
   ast->label = NULL;
   if ( ( label != NULL ) && ( strcmp( label, "" ) != 0 ) ) {
      int found = 0;
      for ( int i = 0; i < array_size( sysedit_sys->asteroids ); i++ ) {
         if ( sysedit_sys->asteroids[i].label &&
              ( strcmp( sysedit_sys->asteroids[i].label, label ) == 0 ) ) {
            dialogue_alert( _( "Asteroid field with label '%s' already "
                               "exists in system '%s'!" ),
                            label, sysedit_sys->name );
            found = 1;
            break;
         }
      }
      if ( !found )
         ast->label = strdup( label );
   }
   ast->density  = atof( window_getInput( sysedit_widEdit, "inpDensity" ) );
   ast->radius   = atof( window_getInput( sysedit_widEdit, "inpRadius" ) );
   ast->maxspeed = atof( window_getInput( sysedit_widEdit, "inpMaxspeed" ) );
   ast->accel    = atof( window_getInput( sysedit_widEdit, "inpAccel" ) );

   /* Need to update some internals based on new values. */
   asteroids_computeInternals( ast );

   if ( conf.devautosave ) {
      if ( dsys_saveSystem( sysedit_sys ) )
         uniedit_saveError();
   }

   window_close( wid, unused );
}

static void sysedit_editExclusion( void )
{
   unsigned int             wid;
   int                      x, y, l, bw;
   char                     buf[STRMAX_SHORT];
   const char              *s;
   const AsteroidExclusion *exc =
      &sysedit_sys->astexclude[sysedit_select[0].u.astexclude];

   /* Create the window. */
   wid             = window_create( "wdwExclusionEditor",
                                    _( "Asteroid Exclusion Zone Editor" ), -1, -1,
                                    SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   sysedit_widEdit = wid;
   window_setCancel( wid, sysedit_editExclusionClose );

   /* Add some inputs. */
   x = 20;
   y = -40;
   s = _( "Label: " );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtLabel", NULL, NULL, s );
   if ( uniedit_diffMode )
      window_addText( wid, x + l + 8, y, 80, 20, 1, "txtInpLabel", NULL, NULL,
                      exc->label );
   else
      window_addInput( wid, x + l + 8, y, 80, 20, "inpLabel", 10, 1, NULL );
   x += l + 20 + 80 + 8;
   s = _( "Radius: " );
   l = gl_printWidthRaw( NULL, s );
   window_addText( wid, x, y, l, 20, 1, "txtInput", NULL, NULL, s );
   window_addInput( wid, x + l + 8, y, 80, 20, "inpRadius", 10, 1, NULL );
   window_setInputFilter( wid, "inpRadius", INPUT_FILTER_NUMBER );

   /* Bottom buttons. */
   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;
   window_addButton( wid, -20 - 15 - bw, 20, bw, BUTTON_HEIGHT, "btnDelete",
                     _( "Delete" ), sysedit_btnExclusionDelete );
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     sysedit_editExclusionClose );

   /* Load current values. */
   if ( !uniedit_diffMode && ( exc->label != NULL ) )
      window_setInput( wid, "inpLabel", exc->label );
   snprintf( buf, sizeof( buf ), "%g", exc->radius );
   window_setInput( wid, "inpRadius", buf );
}

static void sysedit_btnExclusionDelete( unsigned int wid, const char *unused )
{
   int i = dialogue_YesNo(
      _( "Remove Asteroid Exclusion Zone" ),
      _( "Are you sure you want to remove this asteroid exclusion zone?" ) );
   if ( i == 0 )
      return;

   AsteroidExclusion *exc =
      &sysedit_sys->astexclude[sysedit_select[0].u.astexclude];

   array_erase( &sysedit_sys->astexclude, exc, exc + 1 );

   if ( conf.devautosave ) {
      if ( dsys_saveSystem( sysedit_sys ) )
         uniedit_saveError();
   }

   window_close( wid, unused );
}

static void sysedit_editExclusionClose( unsigned int wid, const char *unused )
{
   AsteroidExclusion *exc =
      &sysedit_sys->astexclude[sysedit_select[0].u.astexclude];

   const char *label = window_getInput( sysedit_widEdit, "inpLabel" );
   free( exc->label );
   exc->label = NULL;
   if ( ( label != NULL ) && ( strcmp( label, "" ) != 0 ) ) {
      int found = 0;
      for ( int i = 0; i < array_size( sysedit_sys->astexclude ); i++ ) {
         if ( sysedit_sys->astexclude[i].label &&
              ( strcmp( sysedit_sys->astexclude[i].label, label ) == 0 ) ) {
            dialogue_alert( _( "Asteroid exclusion with label '%s' already "
                               "exists in system '%s'!" ),
                            label, sysedit_sys->name );
            found = 1;
            break;
         }
      }
      if ( !found )
         exc->label = strdup( label );
   }
   exc->radius = atof( window_getInput( sysedit_widEdit, "inpRadius" ) );

   if ( conf.devautosave ) {
      if ( dsys_saveSystem( sysedit_sys ) )
         uniedit_saveError();
   }

   window_close( wid, unused );
}

/**
 * @brief Displays the spob landing description and bar description in a
 * separate window.
 */
static void sysedit_spobDesc( unsigned int wid, const char *unused )
{
   (void)unused;
   int         x, y, h, w, bw;
   Spob       *p;
   const char *desc, *bardesc;
   char        title[128];

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   /* Create the window. */
   snprintf( title, sizeof( title ), _( "Space Object Information - %s" ),
             p->name );
   wid = window_create( "wdwSpobDesc", title, -1, -1, SYSEDIT_EDIT_WIDTH,
                        SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   x = 20;
   y = -40;
   w = SYSEDIT_EDIT_WIDTH - 40;
   h = ( SYSEDIT_EDIT_HEIGHT - gl_defFont.h * 2 - 30 - 60 - BUTTON_HEIGHT -
         10 ) /
       2.;
   desc    = p->description ? p->description : _( "None" );
   bardesc = p->bar_description ? p->bar_description : _( "None" );
   bw      = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   window_addButton( wid, -20 - bw * 3 - 15 * 3, 20, bw, BUTTON_HEIGHT,
                     "btnProperties", _( "Properties" ),
                     sysedit_spobDescReturn );

   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     sysedit_spobDescClose );

   /* Description label and text. */
   window_addText( wid, x, y, w, gl_defFont.h, 0, "txtDescriptionLabel",
                   &gl_defFont, NULL, _( "Landing Description" ) );
   y -= gl_defFont.h + 10;
   window_addInput( wid, x, y, w, h, "txtDescription", 1024, 0, NULL );
   window_setInputFilter( wid, "txtDescription", "[]{}~<>@#$^|_" );
   y -= h + 10;
   /* Load current values. */
   window_setInput( wid, "txtDescription", desc );

   /* Bar description label and text. */
   window_addText( wid, x, y, w, gl_defFont.h, 0, "txtBarDescriptionLabel",
                   &gl_defFont, NULL, _( "Bar Description" ) );
   y -= gl_defFont.h + 10;
   window_addInput( wid, x, y, w, h, "txtBarDescription", 1024, 0, NULL );
   window_setInputFilter( wid, "txtBarDescription", "[]{}~<>@#$^|_" );
   /* Load current values. */
   window_setInput( wid, "txtBarDescription", bardesc );
}

/**
 * @brief Closes the spob description window and returns to the properties
 * window.
 */
static void sysedit_spobDescReturn( unsigned int wid, const char *unused )
{
   Spob       *p         = sysedit_sys->spobs[sysedit_select[0].u.spob];
   const char *mydesc    = window_getInput( wid, "txtDescription" );
   const char *mybardesc = window_getInput( wid, "txtBarDescription" );

   if ( ( mydesc != NULL ) && ( ( p->description == NULL ) ||
                                ( strcmp( mydesc, p->description ) != 0 ) ) ) {
      if ( uniedit_diffMode ) {
         sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_DESCRIPTION,
                                    strdup( mydesc ) );
      } else {
         free( p->description );
         p->description = strdup( mydesc );
      }
   }
   if ( ( mybardesc != NULL ) &&
        ( ( p->bar_description == NULL ) ||
          ( strcmp( mybardesc, p->bar_description ) != 0 ) ) ) {
      if ( uniedit_diffMode ) {
         sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_BAR,
                                    strdup( mybardesc ) );
      } else {
         free( p->bar_description );
         p->bar_description = strdup( mybardesc );
      }
   }

   window_close( wid, unused );
}

/**
 * @brief Closes both the spob description window and the properties window.
 */
static void sysedit_spobDescClose( unsigned int wid, const char *unused )
{
   sysedit_spobDescReturn( wid, unused );
   sysedit_editPntClose( sysedit_widEdit, unused );
}

/**
 * @brief Generates the spob services list.
 */
static void sysedit_genServicesList( unsigned int wid )
{
   int    j, n, nservices;
   Spob  *p;
   char **have, **lack;
   int    x, y, w, h, hpos, lpos;

   hpos = lpos = -1;

   /* Destroy if exists. */
   if ( widget_exists( wid, "lstServicesHave" ) &&
        widget_exists( wid, "lstServicesLacked" ) ) {
      hpos = toolkit_getListPos( wid, "lstServicesHave" );
      lpos = toolkit_getListPos( wid, "lstServicesLacked" );
      window_destroyWidget( wid, "lstServicesHave" );
      window_destroyWidget( wid, "lstServicesLacked" );
   }

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];
   x = 20;
   y = 20 + BUTTON_HEIGHT * 2 + 30;
   w = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;
   h = SYSEDIT_EDIT_HEIGHT - y - 130;

   /* Get all missing services. */
   n = nservices = 0;
   for ( int i = 1; i < SPOB_SERVICES_MAX; i <<= 1 ) {
      if ( !spob_hasService( p, i ) && ( i != SPOB_SERVICE_INHABITED ) )
         n++;
      nservices++; /* Cheaply track all service types. */
   }

   /* Get all the services the spob has. */
   j    = 0;
   have = malloc( sizeof( char * ) * MAX( nservices - n, 1 ) );
   if ( nservices == n )
      have[j++] = strdup( _( "None" ) );
   else
      for ( int i = 1; i < SPOB_SERVICES_MAX; i <<= 1 )
         if ( spob_hasService( p, i ) && ( i != SPOB_SERVICE_INHABITED ) )
            have[j++] = strdup( spob_getServiceName( i ) );

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstServicesHave", have, j, 0, NULL,
                   sysedit_btnRmService );
   x += w + 15;

   /* Add list of services the spob lacks. */
   j    = 0;
   lack = malloc( sizeof( char * ) * MAX( 1, n ) );
   if ( !n )
      lack[j++] = strdup( _( "None" ) );
   else
      for ( int i = 1; i < SPOB_SERVICES_MAX; i <<= 1 )
         if ( !spob_hasService( p, i ) && ( i != SPOB_SERVICE_INHABITED ) )
            lack[j++] = strdup( spob_getServiceName( i ) );

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstServicesLacked", lack, j, 0, NULL,
                   sysedit_btnAddService );

   /* Restore positions. */
   if ( hpos != -1 && lpos != -1 ) {
      toolkit_setListPos( wid, "lstServicesHave", hpos );
      toolkit_setListPos( wid, "lstServicesLacked", lpos );
   }
}

/**
 * @brief Adds a service to a spob.
 */
static void sysedit_btnAddService( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected = toolkit_getList( wid, "lstServicesLacked" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   /* Enable the service. All services imply landability. */
   if ( uniedit_diffMode ) {
      if ( !spob_hasService( p, SPOB_SERVICE_LAND ) )
         sysedit_diffCreateSpobStr(
            p, HUNK_TYPE_SPOB_SERVICE_ADD,
            strdup( spob_getServiceName( SPOB_SERVICE_LAND ) ) );
      if ( !spob_hasService( p, SPOB_SERVICE_INHABITED ) )
         sysedit_diffCreateSpobStr(
            p, HUNK_TYPE_SPOB_SERVICE_ADD,
            strdup( spob_getServiceName( SPOB_SERVICE_INHABITED ) ) );
      sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_SERVICE_ADD,
                                 strdup( selected ) );
   } else {
      p->services |= spob_getService( selected ) | SPOB_SERVICE_INHABITED |
                     SPOB_SERVICE_LAND;
   }

   /* Regenerate the list. */
   sysedit_genServicesList( wid );
}

/**
 * @brief Removes a service from a spob.
 */
static void sysedit_btnRmService( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected = toolkit_getList( wid, "lstServicesHave" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   /* Flip the bit. Safe enough, as it's always 1 to start with. */
   if ( uniedit_diffMode ) {
      if ( strcmp( selected, "Land" ) == 0 ) {
         /* Remove them all. */
         for ( int i = 1; i < SPOB_SERVICES_MAX; i <<= 1 ) {
            if ( !spob_hasService( p, i ) )
               sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_SERVICE_ADD,
                                          strdup( spob_getServiceName( i ) ) );
         }
      } else
         sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_SERVICE_REMOVE,
                                    strdup( selected ) );
   } else {
      /* If landability was removed, the rest must go, too. */
      if ( strcmp( selected, "Land" ) == 0 )
         p->services = 0;
      else
         p->services ^= spob_getService( selected );
   }

   sysedit_genServicesList( wid );
}

/**
 * @brief Edits a spob's tech.
 */
static void sysedit_btnTechEdit( unsigned int wid, const char *unused )
{
   (void)unused;
   int y, w, bw;

   /* Create the window. */
   wid = window_create( "wdwSpobTechEditor", _( "Space Object Tech Editor" ),
                        -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   w  = ( SYSEDIT_EDIT_WIDTH - 40 - 15 ) / 2.;
   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   /* Close button. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     window_close );
   y = 20 + BUTTON_HEIGHT + 15;

   /* Remove button. */
   window_addButton( wid, -20 - ( w + 15 ), y, w, BUTTON_HEIGHT, "btnRm",
                     _( "Rm Tech" ), sysedit_btnRmTech );

   /* Add button. */
   window_addButton( wid, -20, y, w, BUTTON_HEIGHT, "btnAdd", _( "Add Tech" ),
                     sysedit_btnAddTech );

   sysedit_genTechList( wid );
}

/**
 * @brief Generates the spob tech list.
 */
static void sysedit_genTechList( unsigned int wid )
{
   Spob  *p;
   char **have, **lack;
   int    j, n, x, y, w, h, hpos, lpos;

   hpos = lpos = -1;

   /* Destroy if exists. */
   if ( widget_exists( wid, "lstTechsHave" ) &&
        widget_exists( wid, "lstTechsLacked" ) ) {
      hpos = toolkit_getListPos( wid, "lstTechsHave" );
      lpos = toolkit_getListPos( wid, "lstTechsLacked" );
      window_destroyWidget( wid, "lstTechsHave" );
      window_destroyWidget( wid, "lstTechsLacked" );
   }

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];
   w = ( SYSEDIT_EDIT_WIDTH - 40 - 15 ) / 2.;
   x = -20 - w - 15;
   y = 20 + BUTTON_HEIGHT * 2 + 30;
   h = SYSEDIT_EDIT_HEIGHT - y - 30;

   /* Get all the techs the spob has. */
   n = 0;
   if ( p->tech != NULL )
      have = tech_getItemNames( p->tech, &n );
   else {
      have      = malloc( sizeof( char * ) );
      have[n++] = strdup( _( "None" ) );
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTechsHave", have, n, 0, NULL,
                   sysedit_btnRmTech );
   x += w + 15;

   /* Omit the techs that the spob already has from the list.  */
   n = 0;
   if ( p->tech != NULL ) {
      char **tmp = tech_getAllItemNames( &j );
      for ( int i = 0; i < j; i++ )
         if ( !tech_hasItem( p->tech, tmp[i] ) )
            n++;

      if ( !n ) {
         lack      = malloc( sizeof( char * ) );
         lack[n++] = strdup( _( "None" ) );
      } else {
         lack = malloc( sizeof( char * ) * j );
         n    = 0;
         for ( int i = 0; i < j; i++ )
            if ( !tech_hasItem( p->tech, tmp[i] ) )
               lack[n++] = strdup( tmp[i] );
      }

      /* Clean up. */
      for ( int i = 0; i < j; i++ )
         free( tmp[i] );

      free( tmp );
   } else
      lack = tech_getAllItemNames( &n );

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTechsLacked", lack, n, 0, NULL,
                   sysedit_btnAddTech );

   /* Restore positions. */
   if ( hpos != -1 && lpos != -1 ) {
      toolkit_setListPos( wid, "lstTechsHave", hpos );
      toolkit_setListPos( wid, "lstTechsLacked", lpos );
   }
}

/**
 * @brief Adds a tech to a spob.
 */
static void sysedit_btnAddTech( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected = toolkit_getList( wid, "lstTechsLacked" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   if ( uniedit_diffMode ) {
      sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_TECH_ADD,
                                 strdup( selected ) );
   } else {
      if ( p->tech == NULL )
         p->tech = tech_groupCreate();
      tech_addItemTech( p->tech, selected );
   }

   /* Regenerate the list. */
   sysedit_genTechList( wid );
}

/**
 * @brief Removes a tech from a spob.
 */
static void sysedit_btnRmTech( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected = toolkit_getList( wid, "lstTechsHave" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   if ( uniedit_diffMode ) {
      if ( tech_hasItem( p->tech, selected ) )
         sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_TECH_REMOVE,
                                    strdup( selected ) );
   } else {
      int n;
      if ( tech_hasItem( p->tech, selected ) )
         tech_rmItemTech( p->tech, selected );
      n = tech_getItemCount( p->tech );
      if ( !n )
         p->tech = NULL;
   }

   /* Regenerate the list. */
   sysedit_genTechList( wid );
}

/**
 * @brief Edits a spob's tags.
 */
static void sysedit_btnTagsEdit( unsigned int wid, const char *unused )
{
   (void)unused;
   int y, w, bw;

   /* Create the window. */
   wid = window_create( "wdwSpobTagsEditor", _( "Space Object Tags Editor" ),
                        -1, -1, SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, sysedit_btnTagsClose );

   w  = ( SYSEDIT_EDIT_WIDTH - 40 - 15 ) / 2.;
   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;

   /* Close button. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     sysedit_btnTagsClose );
   y = 20 + BUTTON_HEIGHT + 15;

   /* Remove button. */
   window_addButton( wid, -20 - ( w + 15 ), y, w, BUTTON_HEIGHT, "btnRm",
                     _( "Rm Tag" ), sysedit_btnRmTag );

   /* Add button. */
   window_addButton( wid, -20, y, w, BUTTON_HEIGHT, "btnAdd", _( "Add Tag" ),
                     sysedit_btnAddTag );

   /* New tag. */
   window_addButton( wid, -20 - ( w + 15 ), 20, w, BUTTON_HEIGHT, "btnNew",
                     _( "New Tag" ), sysedit_btnNewTag );

   /* Generate list of tags. */
   if ( sysedit_tagslist == NULL ) {
      Spob *spob_all   = spob_getAll();
      sysedit_tagslist = array_create( char * );
      for ( int i = 0; i < array_size( spob_all ); i++ ) {
         Spob *s = &spob_all[i];
         for ( int j = 0; j < array_size( s->tags ); j++ ) {
            const char *t     = s->tags[j];
            int         found = 0;
            for ( int k = 0; k < array_size( sysedit_tagslist ); k++ )
               if ( strcmp( sysedit_tagslist[k], t ) == 0 ) {
                  found = 1;
                  break;
               }
            if ( !found )
               array_push_back( &sysedit_tagslist, strdup( t ) );
         }
      }
      qsort( sysedit_tagslist, array_size( sysedit_tagslist ), sizeof( char * ),
             strsort );
   }

   sysedit_genTagsList( wid );
}

/*
 * Tags are closed so update tags.
 */
static void sysedit_btnTagsClose( unsigned int wid, const char *unused )
{
   char  buf[STRMAX_SHORT];
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];
   int   l = scnprintf( buf, sizeof( buf ), "#n%s#0", _( "Tags:" ) );
   for ( int i = 0; i < array_size( p->tags ); i++ )
      l += scnprintf( &buf[l], sizeof( buf ) - l, "%s %s",
                      ( ( i > 0 ) ? "," : "" ), p->tags[i] );
   window_modifyText( sysedit_widEdit, "txtTags", buf );

   window_close( wid, unused );
}

/**
 * @brief Generates the spob tech list.
 */
static void sysedit_genTagsList( unsigned int wid )
{
   Spob  *p;
   char **have, **lack;
   int    n, x, y, w, h, hpos, lpos, empty;

   hpos = lpos = -1;

   /* Destroy if exists. */
   if ( widget_exists( wid, "lstTagsHave" ) &&
        widget_exists( wid, "lstTagsLacked" ) ) {
      hpos = toolkit_getListPos( wid, "lstTagsHave" );
      lpos = toolkit_getListPos( wid, "lstTagsLacked" );
      window_destroyWidget( wid, "lstTagsHave" );
      window_destroyWidget( wid, "lstTagsLacked" );
   }

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];
   w = ( SYSEDIT_EDIT_WIDTH - 40 - 15 ) / 2.;
   x = -20 - w - 15;
   y = 20 + BUTTON_HEIGHT * 2 + 30;
   h = SYSEDIT_EDIT_HEIGHT - y - 30;

   /* Get all the techs the spob has. */
   n = array_size( p->tags );
   if ( n > 0 ) {
      have = malloc( n * sizeof( char * ) );
      for ( int i = 0; i < n; i++ )
         have[i] = strdup( p->tags[i] );
      empty = 0;
   } else {
      have      = malloc( sizeof( char * ) );
      have[n++] = strdup( _( "None" ) );
      empty     = 1;
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTagsHave", have, n, 0, NULL,
                   sysedit_btnRmTag );
   x += w + 15;

   /* Omit the techs that the spob already has from the list.  */
   n    = 0;
   lack = malloc( array_size( sysedit_tagslist ) * sizeof( char * ) );
   for ( int i = 0; i < array_size( sysedit_tagslist ); i++ ) {
      const char *t = sysedit_tagslist[i];
      if ( empty )
         lack[n++] = strdup( t );
      else {
         int found = 0;
         for ( int j = 0; j < array_size( p->tags ); j++ )
            if ( strcmp( p->tags[j], t ) == 0 ) {
               found = 1;
               break;
            }
         if ( !found )
            lack[n++] = strdup( t );
      }
   }

   /* Add list. */
   window_addList( wid, x, y, w, h, "lstTagsLacked", lack, n, 0, NULL,
                   sysedit_btnAddTag );

   /* Restore positions. */
   if ( hpos != -1 && lpos != -1 ) {
      toolkit_setListPos( wid, "lstTagsHave", hpos );
      toolkit_setListPos( wid, "lstTagsLacked", lpos );
   }
}

/**
 * @brief Adds a tech to a spob.
 */
static void sysedit_btnAddTag( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected = toolkit_getList( wid, "lstTagsLacked" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   if ( uniedit_diffMode ) {
      sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_TAG_ADD,
                                 strdup( selected ) );
   } else {
      if ( p->tags == NULL )
         p->tags = array_create( char * );
      array_push_back( &p->tags, strdup( selected ) );
   }

   /* Regenerate the list. */
   sysedit_genTagsList( wid );
}

/**
 * @brief Removes a tech from a spob.
 */
static void sysedit_btnRmTag( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected;

   selected = toolkit_getList( wid, "lstTagsHave" );
   if ( ( selected == NULL ) || ( strcmp( selected, _( "None" ) ) == 0 ) )
      return;

   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   if ( uniedit_diffMode ) {
      sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_TAG_REMOVE,
                                 strdup( selected ) );
   } else {
      int i;
      for ( i = 0; i < array_size( p->tags ); i++ )
         if ( strcmp( selected, p->tags[i] ) == 0 )
            break;
      if ( i >= array_size( p->tags ) )
         return;
      free( p->tags[i] );
      array_erase( &p->tags, &p->tags[i], &p->tags[i + 1] );
   }

   /* Regenerate the list. */
   sysedit_genTagsList( wid );
}

/**
 * @brief Adds a tech to a spob.
 */
static void sysedit_btnNewTag( unsigned int wid, const char *unused )
{
   (void)unused;
   char *tag =
      dialogue_input( _( "Add New Spob Tag" ), 1, 128,
                      _( "Please write the new tag to add to the spob." ) );
   if ( tag == NULL )
      return;

   Spob *s = sysedit_sys->spobs[sysedit_select[0].u.spob];
   if ( uniedit_diffMode ) {
      sysedit_diffCreateSpobStr( s, HUNK_TYPE_SPOB_TAG_ADD, tag );
   } else {
      if ( s->tags == NULL )
         s->tags = array_create( char * );
      array_push_back( &s->tags, tag ); /* gets freed later */

      /* Also add to list of all tags. */
      array_push_back( &sysedit_tagslist, strdup( tag ) );
   }

   /* Regenerate the list. */
   sysedit_genTagsList( wid );
}

/**
 * @brief Edits a spob's faction.
 */
static void sysedit_btnFaction( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   unsigned int wid;
   int          pos, j, y, h, bw, *factions;
   char       **str;
   Spob        *p;

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   /* Create the window. */
   wid = window_create( "wdwModifyFaction", _( "Modify Faction" ), -1, -1,
                        SYSEDIT_EDIT_WIDTH, SYSEDIT_EDIT_HEIGHT );
   window_setCancel( wid, window_close );

   /* Generate factions list. */
   factions = faction_getAll();
   str      = malloc( sizeof( char * ) * ( array_size( factions ) + 1 ) );
   str[0]   = strdup( _( "None" ) );
   j        = 1;
   for ( int i = 0; i < array_size( factions ); i++ )
      str[j++] = strdup( faction_name(
         factions[i] ) ); /* Not translating so we can use faction_get */
   qsort( &str[1], j - 1, sizeof( char * ), strsort );

   /* Get current faction. */
   if ( p->presence.faction >= 0 ) {
      const char *s = faction_name( p->presence.faction );
      pos           = 0;
      for ( int i = 0; i < j; i++ )
         if ( strcmp( s, str[i] ) == 0 )
            pos = i;
   } else
      pos = -1;

   bw = ( SYSEDIT_EDIT_WIDTH - 40 - 15 * 3 ) / 4.;
   y  = 20 + BUTTON_HEIGHT + 15;
   h  = SYSEDIT_EDIT_HEIGHT - y - 30;
   window_addList( wid, 20, -40, SYSEDIT_EDIT_WIDTH - 40, h, "lstFactions", str,
                   j, pos, NULL, sysedit_btnFactionSet );

   /* Close button. */
   window_addButton( wid, -20, 20, bw, BUTTON_HEIGHT, "btnClose", _( "Close" ),
                     window_close );

   /* Add button. */
   window_addButton( wid, -20 - ( bw + 15 ), 20, bw, BUTTON_HEIGHT, "btnAdd",
                     _( "Set" ), sysedit_btnFactionSet );

   array_free( factions );
}

/**
 * @brief Actually modifies the faction.
 */
static void sysedit_btnFactionSet( unsigned int wid, const char *unused )
{
   (void)unused;
   const char *selected = toolkit_getList( wid, "lstFactions" );
   if ( selected == NULL )
      return;
   Spob *p = sysedit_sys->spobs[sysedit_select[0].u.spob];

   if ( uniedit_diffMode ) {
      sysedit_diffCreateSpobStr( p, HUNK_TYPE_SPOB_FACTION,
                                 strdup( selected ) );
   } else {
      /* "None" case. */
      if ( toolkit_getListPos( wid, "lstFactions" ) == 0 ) {
         p->presence.faction = -1;
      } else {
         /* Set the faction. */
         p->presence.faction = faction_get( selected );
      }
   }

   /* Update the editor window. */
   window_modifyText( sysedit_widEdit, "txtFaction",
                      p->presence.faction >= 0
                         ? faction_name( p->presence.faction )
                         : _( "None" ) );

   window_close( wid, unused );
}

/**
 * @brief Opens the system property editor.
 */
static void sysedit_btnEdit( unsigned int wid_unused, const char *unused )
{
   (void)wid_unused;
   (void)unused;
   const Select_t *sel = &sysedit_select[0];
   if ( sel->type == SELECT_SPOB )
      sysedit_editPnt();
   else if ( sel->type == SELECT_JUMPPOINT )
      sysedit_editJump();
   else if ( sel->type == SELECT_ASTEROID )
      sysedit_editAsteroids();
   else if ( sel->type == SELECT_ASTEXCLUDE )
      sysedit_editExclusion();
}

/**
 * @brief Opens the spob landing or space graphic editor.
 */
static void sysedit_spobGFX( unsigned int wid_unused, const char *wgt )
{
   (void)wid_unused;
   unsigned int    wid;
   size_t          nfiles, j;
   char           *path, buf[STRMAX_SHORT];
   char          **files;
   glTexture      *t;
   ImageArrayCell *cells;
   int             w, h, land;
   Spob           *p;
   glColour        c;

   land = ( strcmp( wgt, "btnLandGFX" ) == 0 );

   p = sysedit_sys->spobs[sysedit_select[0].u.spob];
   /* Create the window. */
   snprintf( buf, sizeof( buf ), _( "%s - Space Object Properties" ), p->name );
   wid = window_create( "wdwSpobGFX", buf, -1, -1, -1, -1 );
   window_dimWindow( wid, &w, &h );

   window_setCancel( wid, sysedit_btnGFXClose );
   window_setAccept( wid, sysedit_btnGFXApply );

   /* Close button. */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "btnClose",
                     _( "Close" ), sysedit_btnGFXClose );

   /* Apply button. */
   window_addButton( wid, -20, 20 + ( 20 + BUTTON_HEIGHT ), BUTTON_WIDTH,
                     BUTTON_HEIGHT, land ? "btnApplyLand" : "btnApplySpace",
                     _( "Apply" ), sysedit_btnGFXApply );

   /* Find images first. */
   path  = land ? SPOB_GFX_EXTERIOR_PATH : SPOB_GFX_SPACE_PATH;
   files = PHYSFS_enumerateFiles( path );
   for ( nfiles = 0; files[nfiles]; nfiles++ ) {
   }
   cells = calloc( nfiles, sizeof( ImageArrayCell ) );

   j = 0;
   for ( size_t i = 0; i < nfiles; i++ ) {
      PHYSFS_Stat path_stat;
      const char *filepath;
      snprintf( buf, sizeof( buf ), "%s/%s", path, files[i] );
      /* Ignore directories. */
      if ( !PHYSFS_stat( buf, &path_stat ) ) {
         WARN( _( "Unable to check file type for '%s'!" ), buf );
         continue;
      }
      if ( path_stat.filetype != PHYSFS_FILETYPE_REGULAR )
         continue;

      t = gl_newImage( buf, OPENGL_TEX_MIPMAPS );
      if ( t == NULL )
         continue;
      cells[j].image   = t;
      cells[j].caption = strdup( files[i] );
      filepath         = ( land ? p->gfx_exteriorPath : p->gfx_spacePath );
      c = ( ( filepath == NULL ) || strcmp( files[i], filepath ) != 0 )
             ? cBlack
             : cOrange;
      memcpy( &cells[j].bg, &c, sizeof( glColour ) );
      j++;
   }
   PHYSFS_freeList( files );

   /* Add image array. */
   window_addImageArray( wid, 20, 20, w - 60 - BUTTON_WIDTH, h - 60, "iarGFX",
                         128, 128, cells, j, NULL, NULL, NULL );
   toolkit_setImageArray( wid, "iarGFX", path );
}

/**
 * @brief Closes the spob graphic editor.
 */
static void sysedit_btnGFXClose( unsigned int wid, const char *wgt )
{
   window_close( wid, wgt );
}

/**
 * @brief Apply new graphics.
 */
static void sysedit_btnGFXApply( unsigned int wid, const char *wgt )
{
   Spob       *p;
   const char *str;
   char       *path, buf[PATH_MAX];
   int         land;

   land = ( strcmp( wgt, "btnApplyLand" ) == 0 );
   p    = sysedit_sys->spobs[sysedit_select[0].u.spob];

   /* Get output. */
   str = toolkit_getImageArray( wid, "iarGFX" );
   if ( str == NULL )
      return;

   /* New path. */
   path = land ? SPOB_GFX_EXTERIOR_PATH : SPOB_GFX_SPACE_PATH;
   snprintf( buf, sizeof( buf ), "%s/%s", path, str );

   if ( land ) {
      free( p->gfx_exteriorPath );
      free( p->gfx_exterior );
      snprintf( buf, sizeof( buf ), SPOB_GFX_EXTERIOR_PATH "%s", str );
      p->gfx_exteriorPath = strdup( str );
      p->gfx_exterior     = strdup( buf );
   } else { /* Free old texture, load new. */
      free( p->gfx_spaceName );
      free( p->gfx_spacePath );
      p->gfx_spaceName = strdup( buf );
      p->gfx_spacePath = strdup( str );
      gl_freeTexture( p->gfx_space );
      p->gfx_space = NULL;
      p->radius    = -1.; /* Reset radius. */
      spob_gfxLoad( p );
   }

   /* For now we close. */
   sysedit_btnGFXClose( wid, wgt );
}

static void sysedit_diffCreateSpobStr( const Spob *spb, UniHunkType_t type,
                                       char *str )
{
   UniHunk_t hunk;
   memset( &hunk, 0, sizeof( hunk ) );
   hunk.target.type   = HUNK_TARGET_SPOB;
   hunk.target.u.name = strdup( spb->name );
   hunk.type          = type;
   hunk.dtype         = HUNK_DATA_STRING;
   hunk.u.name        = str;
   uniedit_diffAdd( &hunk );
}
static void sysedit_diffCreateSpobInt( const Spob *spb, UniHunkType_t type,
                                       int data )
{
   UniHunk_t hunk;
   memset( &hunk, 0, sizeof( hunk ) );
   hunk.target.type   = HUNK_TARGET_SPOB;
   hunk.target.u.name = strdup( spb->name );
   hunk.type          = type;
   hunk.dtype         = HUNK_DATA_INT;
   hunk.u.data        = data;
   uniedit_diffAdd( &hunk );
}
static void sysedit_diffCreateSpobFloat( const Spob *spb, UniHunkType_t type,
                                         double fdata )
{
   UniHunk_t hunk;
   memset( &hunk, 0, sizeof( hunk ) );
   hunk.target.type   = HUNK_TARGET_SPOB;
   hunk.target.u.name = strdup( spb->name );
   hunk.type          = type;
   hunk.dtype         = HUNK_DATA_FLOAT;
   hunk.u.fdata       = fdata;
   uniedit_diffAdd( &hunk );
}
