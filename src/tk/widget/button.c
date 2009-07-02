/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file button.c
 *
 * @brief Button widget.
 */


#include "tk/toolkit_priv.h"


static int btn_key( Widget* btn, SDLKey key, SDLMod mod );
static void btn_render( Widget* btn, double bx, double by );
static void btn_cleanup( Widget* btn );


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
                       char* name, char* display,
                       void (*call) (unsigned int wgt, char* wdwname) )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);

   /* generic */
   wgt->type = WIDGET_BUTTON;
   wgt->name = strdup(name);
   wgt->wdw  = wid;
   
   /* specific */
   wgt->keyevent           = btn_key;
   wgt->render             = btn_render;
   wgt->cleanup            = btn_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->dat.btn.display    = strdup(display);
   wgt->dat.btn.disabled   = 0; /* initially enabled */
   wgt->dat.btn.fptr       = call;

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );

   if (wgt->dat.btn.fptr == NULL) { /* Disable if function is NULL. */
      wgt->dat.btn.disabled = 1;
      wgt_rmFlag(wgt, WGT_FLAG_CANFOCUS);
   }

   if (wdw->focus == -1) /* initialize the focus */
      toolkit_nextFocus();
}


/**
 * @brief Disables a button.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to disable.
 */
void window_disableButton( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check type. */
   if (wgt->type != WIDGET_BUTTON) {
      DEBUG("Trying to disable a non-button widget '%s'", name);
      return;
   }

   /* Disable button. */
   wgt->dat.btn.disabled = 1;
   wgt_rmFlag(wgt, WGT_FLAG_CANFOCUS);
}


/**
 * @brief Enables a button.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the button to enable.
 */
void window_enableButton( const unsigned int wid, char *name )
{
   Widget *wgt;
  
   /* Get widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check type. */
   if (wgt->type != WIDGET_BUTTON) {
      DEBUG("Trying to enable a non-button widget '%s'", name);
      return;
   }

   /* Enable button. */
   wgt->dat.btn.disabled = 0;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
}


/**
 * @brief Handles input for an button widget.
 *
 *    @param btn Button widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int btn_key( Widget* btn, SDLKey key, SDLMod mod )
{
   (void) mod;

   if (key == SDLK_RETURN)
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
   glColour *c, *dc, *lc;
   double x, y;

   x = bx + btn->x;
   y = by + btn->y;

   /* set the colours */
   if (btn->dat.btn.disabled==1) {
      lc = &cGrey60;
      c = &cGrey20;
      dc = &cGrey40;
   }
   else {
      switch (btn->status) {
         case WIDGET_STATUS_NORMAL:
            lc = &cGrey80;
            c = &cGrey60;
            dc = &cGrey40;
            break;
         case WIDGET_STATUS_MOUSEOVER:
            lc = &cWhite;
            c = &cGrey80;
            dc = &cGrey60;
            break;
         case WIDGET_STATUS_MOUSEDOWN:
            lc = &cGreen;
            c = &cGreen;
            dc = &cGrey40;
            break;
         default:
            break;
      }
   }


   /* shaded base */
   if (btn->dat.btn.disabled==1) {
      toolkit_drawRect( x, y,            btn->w, 0.4*btn->h, dc, NULL );
      toolkit_drawRect( x, y+0.4*btn->h, btn->w, 0.6*btn->h, dc, c );
   }
   else {
      toolkit_drawRect( x, y,            btn->w, 0.6*btn->h, dc, c );
      toolkit_drawRect( x, y+0.6*btn->h, btn->w, 0.4*btn->h, c, NULL );
   }
   
   /* inner outline */
   toolkit_drawOutline( x, y, btn->w, btn->h, 0., lc, c );
   /* outter outline */
   toolkit_drawOutline( x, y, btn->w, btn->h, 1., &cBlack, NULL );

   gl_printMidRaw( NULL, (int)btn->w,
         bx + (double)SCREEN_W/2. + btn->x,
         by + (double)SCREEN_H/2. + btn->y + (btn->h - gl_defFont.h)/2.,
         &cDarkRed, btn->dat.btn.display );
}


/**
 * @brief Clean up function for the button widget.
 *
 *    @param btn Button to clean up.
 */
static void btn_cleanup( Widget *btn )
{
   if (btn->dat.btn.display != NULL)
      free(btn->dat.btn.display);
}
