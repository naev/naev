/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file input.c
 *
 * @brief Input widget.
 */


#include "tk/toolkit_priv.h"

#include "nstd.h"


static void inp_render( Widget* inp, double bx, double by );
static int inp_key( Widget* inp, SDLKey key, SDLMod mod );
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
 */
void window_addInput( const unsigned int wid,
                      const int x, const int y, /* position */
                      const int w, const int h, /* size */
                      char* name, const int max, const int oneline )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw);                             
                                                                    
   /* generic */
   wgt->type   = WIDGET_INPUT;
   wgt->name   = strdup(name);
   wgt->wdw    = wid;

   /* specific */
   wgt->render          = inp_render;
   wgt->cleanup         = inp_cleanup;
   wgt_setFlag(wgt, WGT_FLAG_CANFOCUS);
   wgt->keyevent        = inp_key;
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

   x = bx + inp->x;
   y = by + inp->y;

   /* main background */
   toolkit_drawRect( x, y, inp->w, inp->h, &cWhite, NULL );

   /* center vertically */
   if (inp->dat.inp.oneline) ty = y - (inp->h - gl_smallFont.h)/2.;

   gl_printTextRaw( &gl_smallFont, inp->w-10., inp->h,
         x+5. + SCREEN_W/2., ty  + SCREEN_H/2.,
         &cBlack, inp->dat.inp.input + inp->dat.inp.view );

   /* inner outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outter outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 1.,
         toolkit_colDark, NULL );
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
   int n;
   
   /*
    * Handle arrow keys.
    * @todo finish implementing, no cursor makes it complicated to see where you are.
    */
#if 0
   if ((type == SDL_KEYDOWN) &&
         ((key == SDLK_LEFT) || (key == SDLK_RETURN))) {
      /* Move pointer. */                                           
      if (key == SDLK_LEFT) {                                       
         if (inp->dat.inp.pos > 0)
            inp->dat.inp.pos -= 1;
      }
      else if (key == SDLK_RIGHT) {
         if ((inp->dat.inp.pos < inp->dat.inp.max-1) &&
               (inp->dat.inp.input[inp->dat.inp.pos+1] != '\0'))
            inp->dat.inp.pos += 1;
      }

      return 1;
   }
#endif

   /* Only catch some keys. */
   if (!nstd_isalnum(key) && (key != SDLK_BACKSPACE) &&
         (key != SDLK_SPACE) && (key != SDLK_RETURN))
      return 0;

   if (inp->dat.inp.oneline) {

      /* backspace -> delete text */
      if ((key == SDLK_BACKSPACE) && (inp->dat.inp.pos > 0)) {
         inp->dat.inp.input[ --inp->dat.inp.pos ] = '\0';

         if (inp->dat.inp.view > 0) {
            n = gl_printWidthRaw( &gl_smallFont,
                  inp->dat.inp.input + inp->dat.inp.view - 1 );
            if (n+10 < inp->w)
               inp->dat.inp.view--;
         }
      }

      /* in limits. */
      else if ((inp->dat.inp.pos < inp->dat.inp.max-1)) {

         if ((key==SDLK_RETURN) && !inp->dat.inp.oneline)
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = '\n';
                                                                    
         /* upper case characters */
         else if (nstd_isalpha(key) && (mod & (KMOD_LSHIFT | KMOD_RSHIFT)))
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = nstd_toupper(key);

         /* rest */
         else if (!nstd_iscntrl(key))
            inp->dat.inp.input[ inp->dat.inp.pos++ ] = key;

         /* didn't get a useful key */
         else return 0;

         n = gl_printWidthRaw( &gl_smallFont, inp->dat.inp.input+inp->dat.inp.view );
         if (n+10 > inp->w) inp->dat.inp.view++;
         return 1;
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
   return (wgt) ? wgt->dat.inp.input : NULL;
}

