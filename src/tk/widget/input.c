/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file input.c
 *
 * @brief Input widget.
 */


#include "tk/toolkit_priv.h"

#include <stdlib.h>
#include <string.h>

#include "nstd.h"


static void inp_render( Widget* inp, double bx, double by );
static int inp_key( Widget* inp, SDLKey key, SDLMod mod );
static int inp_text( Widget* inp, const char *buf );
static int inp_addKey( Widget* inp, SDLKey key );
static void inp_cleanup( Widget* inp );


/**
 * @brief Adds an input widget to a window.
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
 *    @param max Max amount of characters that can be written.
 *    @param oneline Whether widget should only be one line long.
 *    @param font Font to use.
 */
void window_addInput( const unsigned int wid,
                      const int x, const int y, /* position */
                      const int w, const int h, /* size */
                      char* name, const int max, const int oneline,
                      glFont *font )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type   = WIDGET_INPUT;

   /* specific */
   wgt->render          = inp_render;
   wgt->cleanup         = inp_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->keyevent        = inp_key;
   wgt->textevent       = inp_text;
   /*wgt->keyevent        = inp_key;*/
   wgt->dat.inp.font    = (font != NULL) ? font : &gl_smallFont;
   wgt->dat.inp.max     = max+1;
   wgt->dat.inp.oneline = oneline;
   wgt->dat.inp.pos     = 0;
   wgt->dat.inp.view    = 0;
   wgt->dat.inp.input   = malloc(sizeof(char)*wgt->dat.inp.max);
   memset(wgt->dat.inp.input, 0, wgt->dat.inp.max*sizeof(char));

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Renders a input widget.
 *
 *    @param inp Input widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void inp_render( Widget* inp, double bx, double by )
{
   double x, y, ty;
   char buf[ PATH_MAX ];
   int w;
   int m;

   x = bx + inp->x;
   y = by + inp->y;

   /* main background */
   toolkit_drawRect( x, y, inp->w, inp->h, &cWhite, NULL );

   /* center vertically */
   if (inp->dat.inp.oneline)
      ty = y - (inp->h - gl_smallFont.h)/2.;
   else {
      WARN("Multi-line input widgets unsupported atm.");
      return;
   }

   /* Draw text. */
   gl_printTextRaw( inp->dat.inp.font, inp->w-10., inp->h,
         x+5., ty, &cBlack, &inp->dat.inp.input[ inp->dat.inp.view ] );

   /* Draw cursor. */
   if (wgt_isFlag( inp, WGT_FLAG_FOCUSED )) {
      m = MIN( inp->dat.inp.pos - inp->dat.inp.view, PATH_MAX-1 );
      strncpy( buf, &inp->dat.inp.input[ inp->dat.inp.view ], m );
      buf[ m ] = '\0';
      w = gl_printWidthRaw( inp->dat.inp.font, buf );
      toolkit_drawRect( x+5.+w, y + (inp->h - inp->dat.inp.font->h - 4.)/2.,
            1., inp->dat.inp.font->h + 4., &cBlack, &cBlack );
   }

   /* inner outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outter outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 1.,
         toolkit_colDark, NULL );
}


/**
 * @brief Handles input text.
 *
 *    @param inp Input widget to handle event.
 *    @param buf Text to handle.
 *    @return 1 if text was used;
 */
static int inp_text( Widget* inp, const char *buf )
{
   int i;
   int ret;

   i = 0;
   ret = 0;
   while (buf[i] != '\0') {
      ret |= inp_addKey( inp, buf[i] );
      i++;
   }
   return ret;
}


/**
 * @brief Adds a single key to the input.
 *
 *    @param inp Input widget recieving the key.
 *    @param key Key to recieve.
 *    @return 1 if key was used.
 */
static int inp_addKey( Widget* inp, SDLKey key )
{
   int i;
   int n;
   char c;

   /*
    * Handle arrow keys.
    * @todo finish implementing, no cursor makes it complicated to see where you are.
    */
   /* Only catch some keys. */
   if (!nstd_isgraph(key) && (key != ' '))
      return 0;

   /* No sense to use SDLKey below this. */
   c = key;

   if (inp->dat.inp.oneline) {

      /* Make sure it's not full. */
      if (strlen(inp->dat.inp.input) >= (size_t)inp->dat.inp.max-1)
         return 0;

      /* Check to see if is in filter to ignore. */
      if (inp->dat.inp.filter != NULL)
         for (i=0; inp->dat.inp.filter[i] != '\0'; i++)
            if (inp->dat.inp.filter[i] == c)
               return 0; /* Ignored. */

      /* Add key. */
      memmove( &inp->dat.inp.input[ inp->dat.inp.pos+1 ],
            &inp->dat.inp.input[ inp->dat.inp.pos ],
            inp->dat.inp.max - inp->dat.inp.pos - 2 );
      inp->dat.inp.input[ inp->dat.inp.pos++ ] = c;

      n = gl_printWidthRaw( inp->dat.inp.font, inp->dat.inp.input+inp->dat.inp.view );
      if (n+10 > inp->w)
         inp->dat.inp.view++;
      return 1;
   }
   return 0;
}


/**
 * @brief Handles input for an input widget.
 *
 *    @param inp Input widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int inp_key( Widget* inp, SDLKey key, SDLMod mod )
{
   (void) mod;
   int n;

   /*
    * Handle arrow keys.
    */
    if ((key == SDLK_LEFT) || (key == SDLK_RIGHT)) {
      /* Move pointer. */
      if (key == SDLK_LEFT) {
         if (inp->dat.inp.pos > 0)
            inp->dat.inp.pos -= 1;
      }
      else if (key == SDLK_RIGHT) {
         if ((inp->dat.inp.pos < inp->dat.inp.max-1) &&
               (inp->dat.inp.input[inp->dat.inp.pos] != '\0'))
            inp->dat.inp.pos += 1;
      }

      return 1;
   }

   /* Only catch some keys. */
   if ((key != SDLK_BACKSPACE) && (key != SDLK_RETURN) && (key != SDLK_KP_ENTER))
      return 0;

   if (inp->dat.inp.oneline) {

      /* backspace -> delete text */
      if ((key == SDLK_BACKSPACE) && (inp->dat.inp.pos > 0)) {
         inp->dat.inp.pos--;
         memmove( &inp->dat.inp.input[ inp->dat.inp.pos ],
               &inp->dat.inp.input[ inp->dat.inp.pos+1 ],
               sizeof(char)*(inp->dat.inp.max - inp->dat.inp.pos - 1) );
         inp->dat.inp.input[ inp->dat.inp.max - 1 ] = '\0';

         if (inp->dat.inp.view > 0) {
            n = gl_printWidthRaw( &gl_smallFont,
                  inp->dat.inp.input + inp->dat.inp.view - 1 );
            if (n+10 < inp->w)
               inp->dat.inp.view--;
         }
         return 1;
      }

      /* in limits. */
      else if ((inp->dat.inp.pos < inp->dat.inp.max-1)) {

         if ((key==SDLK_RETURN || key==SDLK_KP_ENTER) && !inp->dat.inp.oneline) {
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = '\n';
            return 1;
         }
      }
   }
   return 0;
}



/**
 * @brief Clean up function for the input widget.
 *
 *    @param inp Input widget to clean up.
 */
static void inp_cleanup( Widget* inp )
{
   /* Free filter if needed. */
   if (inp->dat.inp.filter != NULL)
      free(inp->dat.inp.filter);

   free(inp->dat.inp.input); /* frees the input buffer */
}


/**
 * @brief Gets the input from an input widget.
 *
 *    @param wid ID of the window to get widget from.
 *    @param name Name of the widget.
 */
char* window_getInput( const unsigned int wid, char* name )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Check the type. */
   if (wgt->type != WIDGET_INPUT) {
      WARN("Trying to get input from non-input widget '%s'.", name);
      return NULL;
   }

   /* Get the value. */
   return wgt->dat.inp.input;
}

/**
 * @brief Sets the input for an input widget.
 *
 *    @param wid Window to which the widget belongs.
 *    @param name Name of the widget to modify.
 *    @param msg Message to set for the input box or NULL to clear.
 * @return The message actually set (can be truncated).
 */
char* window_setInput( const unsigned int wid, char* name, const char *msg )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return NULL;

   /* Check the type. */
   if (wgt->type != WIDGET_INPUT) {
      WARN("Trying to set input on non-input widget '%s'.", name);
      return NULL;
   }

   /* Set the message. */
   if (msg == NULL) {
      memset( wgt->dat.inp.input, 0, wgt->dat.inp.max*sizeof(char) );
      wgt->dat.inp.pos     = 0;
      wgt->dat.inp.view    = 0;
   }
   else {
      strncpy( wgt->dat.inp.input, msg, wgt->dat.inp.max );
      wgt->dat.inp.pos = strlen( wgt->dat.inp.input );
   }

   /* Get the value. */
   return wgt->dat.inp.input;
}


/**
 * @brief Sets the input filter.
 *
 * This is a list of characters which won't be accepted as input.
 *
 *    @param wid Window to which input widget belongs.
 *    @param name Input widget to set filter on.
 *    @param filter '\0' terminated list of characters to filter.
 */
void window_setInputFilter( const unsigned int wid, char* name, const char *filter )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_INPUT) {
      WARN("Trying to set input filter on non-input widget '%s'.", name);
      return;
   }

   /* Free if already exists. */
   if (wgt->dat.inp.filter != NULL)
      free(wgt->dat.inp.filter);

   /* Copy filter over. */
   wgt->dat.inp.filter = strdup( filter );
}

