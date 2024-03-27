/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file button.c
 *
 * @brief Button widget.
 */

/** @cond */
#include <stdlib.h>
/** @endcond */

#include "nstring.h"
#include "tk/toolkit_priv.h"

static Widget *chk_getWgt( unsigned int wid, const char *name );
static int     chk_key( Widget *chk, SDL_Keycode key, SDL_Keymod mod,
                        int isrepeat );
static int     chk_mclick( Widget *chk, int button, int x, int y );
static void    chk_render( Widget *chk, double bx, double by );
static void    chk_cleanup( Widget *chk );
static void    chk_toggleState( Widget *chk );

/**
 * @brief Adds a button widget to a window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Width of the widget.
 *    @param h Height of the widget.
 *    @param name Name of the widget to use internally.
 *    @param display Text displayed on the button (centered).
 *    @param call Function to call when checkbox is toggled Parameter passed
 *                is the name of the checkbox.
 */
void window_addCheckbox(
   unsigned int wid, const int x, const int y,   /* position */
   const int w, const int h,                     /* size */
   const char *name, const char *display,        /* label name, display name */
   void ( *call )( unsigned int, const char * ), /* toggle function */
   int default_state )                           /* default state. */
{
   Window *wdw = window_wget( wid );
   Widget *wgt = window_newWidget( wdw, name );
   if ( wgt == NULL )
      return;

   /* generic */
   wgt->type = WIDGET_CHECKBOX;

   /* specific */
   wgt->keyevent         = chk_key;
   wgt->mclickevent      = chk_mclick;
   wgt->render           = chk_render;
   wgt->cleanup          = chk_cleanup;
   wgt->dat.chk.display  = ( display == NULL ) ? NULL : strdup( display );
   wgt->dat.chk.fptr     = call;
   wgt->dat.chk.state    = default_state;
   wgt->dat.chk.disabled = 0;

   /* position/size */
   wgt->w = (double)w;
   wgt->h = (double)h;
   toolkit_setPos( wdw, wgt, x, y );
}

/**
 * @brief Gets a widget.
 */
static Widget *chk_getWgt( unsigned int wid, const char *name )
{
   Widget *wgt = window_getwgt( wid, name );
   if ( wgt == NULL )
      return NULL;

   /* Check type. */
   if ( wgt->type != WIDGET_CHECKBOX ) {
      DEBUG( "Calling checkbox function on non-checkbox widget '%s'", name );
      return NULL;
   }

   return wgt;
}

/**
 * @brief Changes the checkbox caption.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to change caption.
 *    @param display New caption to display.
 */
void window_checkboxCaption( unsigned int wid, const char *name, char *display )
{
   Widget *wgt = chk_getWgt( wid, name );
   if ( wgt == NULL )
      return;

   free( wgt->dat.chk.display );
   wgt->dat.chk.display = strdup( display );
}

/**
 * @brief Gets the state of a checkbox.
 *
 *    @return 1 = pressed, 0 = not pressed, -1 = error.
 */
int window_checkboxState( unsigned int wid, const char *name )
{
   Widget *wgt = chk_getWgt( wid, name );
   if ( wgt == NULL )
      return -1;

   return wgt->dat.chk.state;
}

/**
 * @brief Sets the checkbox state.
 *
 *    @param state State to set checkbox to.
 *    @return The checkbox state or -1 on error.
 */
int window_checkboxSet( unsigned int wid, const char *name, int state )
{
   Widget *wgt = chk_getWgt( wid, name );
   if ( wgt == NULL )
      return -1;

   wgt->dat.chk.state = state;
   return wgt->dat.chk.state;
}

/**
 * @brief Toggles the checkbox state.
 */
static void chk_toggleState( Widget *chk )
{
   chk->dat.chk.state = !chk->dat.chk.state;
   if ( chk->dat.chk.fptr != NULL )
      chk->dat.chk.fptr( chk->wdw, chk->name );
}

/**
 * @brief Handles input for an button widget.
 *
 *    @param chk Button widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @param isrepeat Whether or not the key is repeating.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int chk_key( Widget *chk, SDL_Keycode key, SDL_Keymod mod, int isrepeat )
{
   (void)mod;
   (void)isrepeat;

   if ( chk->dat.chk.disabled )
      return 0;

   if ( key == SDLK_SPACE )
      chk_toggleState( chk );

   return 0;
}

/**
 * @brief Handles checkbox mouse clicks.
 */
static int chk_mclick( Widget *chk, int button, int x, int y )
{
   (void)button;
   (void)y;
   if ( chk->dat.chk.disabled )
      return 0;
   if ( ( x > 0 ) && ( x <= chk->w ) && ( y > 0 ) && ( y <= chk->h ) )
      chk_toggleState( chk );
   return 1;
}

/**
 * @brief Renders a button widget.
 *
 *    @param chk WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void chk_render( Widget *chk, double bx, double by )
{
   const glColour *cbkg, *cfg;
   double          x, y;

   x = bx + chk->x;
   y = by + chk->y;

   /* set the colours */
   if ( chk->dat.chk.disabled ) {
      cbkg = toolkit_col;
      cfg  = &cFontGrey;
   } else {
      cbkg = toolkit_colLight;
      cfg  = &cFontWhite;
   }

   /* Draw rect. */
   gl_renderPane( x - 1, y - 1 + ( chk->h - 10. ) / 2., 12., 12.,
                  toolkit_colDark );
   gl_renderPane( x, y + ( chk->h - 10. ) / 2., 10., 10., cbkg );
   if ( chk->dat.chk.state )
      gl_renderPane( x + 2., y + 2. + ( chk->h - 10. ) / 2., 6., 6.,
                     toolkit_colDark );

   /* Draw the txt. */
   gl_printMaxRaw( NULL, chk->w - 20, bx + chk->x + 15,
                   by + chk->y + ( chk->h - gl_defFont.h ) / 2., cfg, -1.,
                   chk->dat.chk.display );
}

/**
 * @brief Clean up function for the button widget.
 *
 *    @param chk Button to clean up.
 */
static void chk_cleanup( Widget *chk )
{
   if ( chk->dat.chk.display != NULL )
      free( chk->dat.chk.display );
}

/**
 * @brief Disables a checkbox.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the checkbox to disable.
 */
int window_enableCheckbox( unsigned int wid, const char *name )
{
   Widget *wgt = chk_getWgt( wid, name );
   if ( wgt == NULL )
      return -1;
   wgt->dat.chk.disabled = 0;
   return 0;
}

/**
 * @brief Enables a checkbox.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the checkbox to enable.
 */
int window_disableCheckbox( unsigned int wid, const char *name )
{
   Widget *wgt = chk_getWgt( wid, name );
   if ( wgt == NULL )
      return -1;
   wgt->dat.chk.disabled = 1;
   return 0;
}
