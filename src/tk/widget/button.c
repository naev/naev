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


static int btn_mclick( Widget* btn, int button, int x, int y );
static int btn_key( Widget* btn, SDL_Keycode key, SDL_Keymod mod );
static void btn_render( Widget* btn, double bx, double by );
static void btn_cleanup( Widget* btn );
static Widget* btn_get( const unsigned int wid, const char* name );
static void btn_updateHotkey( Widget *btn );


/**
 * @brief Adds a button widget to a window, with a hotkey that enables the button to be activated with that key.
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
 *    @param call Function to call when button is pressed. Parameter passed
 *                is the name of the button.
 *    @param key Hotkey for using the button without it being focused.
 */
void window_addButtonKey( const unsigned int wid,
                       const int x, const int y,
                       const int w, const int h,
                       const char* name, const char* display,
                       void (*call) (unsigned int wgt, char* wdwname),
                       SDL_Keycode key )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type = WIDGET_BUTTON;

   /* specific */
   wgt->keyevent           = btn_key;
   wgt->render             = btn_render;
   wgt->cleanup            = btn_cleanup;
   wgt->mclickevent        = btn_mclick;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->dat.btn.display    = strdup(display);
   wgt->dat.btn.disabled   = 0; /* initially enabled */
   wgt->dat.btn.fptr       = call;
   if (key != 0) {
      wgt->dat.btn.key = key;
      btn_updateHotkey(wgt);
   }

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wgt->dat.btn.fptr == NULL) { /* Disable if function is NULL. */
      wgt->dat.btn.disabled = 1;
      wgt_rmFlag(wgt, WGT_FLAG_CANFOCUS);
   }

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus( wdw );
}


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
 *    @param call Function to call when button is pressed. Parameter passed
 *                is the name of the button.
 */
void window_addButton( const unsigned int wid,
                       const int x, const int y,
                       const int w, const int h,
                       const char* name, const char* display,
                       void (*call) (unsigned int wgt, char* wdwname) )
{
   window_addButtonKey( wid, x, y, w, h, name, display, call, 0 );
}


/**
 * @brief Gets a button widget.
 */
static Widget* btn_get( const unsigned int wid, const char* name )
{
   Widget *wgt;

   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Check type. */
   if (wgt->type != WIDGET_BUTTON) {
      DEBUG("Widget '%s' isn't a button", name);
      return NULL;
   }

   return wgt;
}


/**
 * @brief Disables a button.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to disable.
 */
void window_disableButton( const unsigned int wid, const char* name )
{
   Widget *wgt;
   Window *wdw;

   /* Get the widget. */
   wgt = btn_get( wid, name );
   if (wgt == NULL)
      return;

   /* Disable button. */
   wgt->dat.btn.disabled = 1;

   /* Sanitize focus. */
   wdw = window_wget(wid);
   toolkit_focusSanitize(wdw);
}


/**
 * @brief Disables a button, while still running the button's function.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to disable.
 */
void window_disableButtonSoft( const unsigned int wid, const char *name )
{
   Widget *wgt;

   /* Get the widget. */
      wgt = btn_get( wid, name );
      if (wgt == NULL)
         return;

   wgt->dat.btn.softdisable = 1;
   window_disableButton( wid, name );
}


/**
 * @brief Enables a button.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to enable.
 */
void window_enableButton( const unsigned int wid, const char *name )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = btn_get( wid, name );
   if (wgt == NULL)
      return;

   /* Enable button. */
   wgt->dat.btn.disabled = 0;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
}


/**
 * @brief Changes the button caption.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to change caption.
 *    @param display New caption to display.
 */
void window_buttonCaption( const unsigned int wid, const char *name, const char *display )
{

   Widget *wgt;

   /* Get the widget. */
   wgt = btn_get( wid, name );
   if (wgt == NULL)
      return;

   free(wgt->dat.btn.display);
   wgt->dat.btn.display = strdup(display);

   if (wgt->dat.btn.key != 0)
      btn_updateHotkey(wgt);
}


/**
 * @brief Checks a button's hotkey against its label and highlights the hotkey, if present.
 */
static void btn_updateHotkey( Widget *btn )
{
   char buf[PATH_MAX], *display, target;
   const char *keyname;
   size_t i;
   int match;

   keyname = SDL_GetKeyName(btn->dat.btn.key);
   if (strlen(keyname) != 1) /* Only interested in single chars. */
      return;

   target = keyname[0];
   if (!isalnum(target)) /* We filter to alpha numeric characters. */
      return;
   target = tolower(target);

   /* Find first occurence in string. */
   display  = btn->dat.btn.display;
   match    = -1;
   for (i=0; i<strlen(display); i++) {
      if (tolower(display[i])==target) {
         match = i;
         break;
      }
   }
   if (match < 0)
      return;
   target         = display[match]; /* Store character, can be uppercase. */
   display[match] = '\0'; /* Cuts the string into two. */

   /* Copy both parts and insert the character in the middle. */
   nsnprintf( buf, sizeof(buf), "%s\aw%c\a0%s", display, target, &display[match+1] );

   free(btn->dat.btn.display);
   btn->dat.btn.display = strdup(buf);
}


/**
 * @brief Handles input for an button widget.
 *
 *    @param btn Button widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int btn_key( Widget* btn, SDL_Keycode key, SDL_Keymod mod )
{
   (void) mod;

   /* Don't grab disabled events. Soft-disabling falls through. */
   if ((btn->dat.btn.disabled) && (!btn->dat.btn.softdisable))
      return 0;

   if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
      if (btn->dat.btn.fptr != NULL) {
         (*btn->dat.btn.fptr)(btn->wdw, btn->name);
         return 1;
      }
   return 0;
}


/**
 * @brief Renders a button widget.
 *
 *    @param btn WIDGET_BUTTON widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void btn_render( Widget* btn, double bx, double by )
{
   const glColour *c, *fc, *outline;
   double x, y;

   x = bx + btn->x;
   y = by + btn->y;

   /* set the colours */
   if (btn->dat.btn.disabled) {
      c  = &cGrey20;
      fc = &cFontGrey;
      outline = &cGrey20;
   }
   else {
      fc = &cFontGrey;
      outline = &cGrey15;
      switch (btn->status) {
         case WIDGET_STATUS_MOUSEOVER:
            c  = &cGrey30;
            break;
         case WIDGET_STATUS_MOUSEDOWN:
            c  = &cGrey35;
            break;
         case WIDGET_STATUS_NORMAL:
         default:
            c  = &cGrey25;
      }
   }


   toolkit_drawRect( x, y, btn->w, btn->h, c, NULL );

   /* inner outline */
   // toolkit_drawOutline( x, y, btn->w, btn->h, 0., outline, NULL );
   /* outer outline */
   toolkit_drawOutlineThick( x, y, btn->w, btn->h, 1., 2, outline, NULL );

   gl_printMidRaw( &gl_smallFont, (int)btn->w,
         bx + btn->x,
         by + btn->y + (btn->h - gl_smallFont.h)/2.,
         fc, -1., btn->dat.btn.display );
}


/**
 * @brief Clean up function for the button widget.
 *
 *    @param btn Button to clean up.
 */
static void btn_cleanup( Widget *btn )
{
   free(btn->dat.btn.display);
}


/**
 * @brief Basically traps click events.
 */
static int btn_mclick( Widget* btn, int button, int x, int y )
{
   (void) btn;
   (void) button;
   (void) x;
   (void) y;
   return 1;
}


