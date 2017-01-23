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
#include <assert.h>

#include "nstd.h"
#include "nstring.h"


static void inp_render( Widget* inp, double bx, double by );
static int inp_isBreaker(char c);
static int inp_key( Widget* inp, SDLKey key, SDLMod mod );
static int inp_text( Widget* inp, const char *buf );
static int inp_addKey( Widget* inp, SDLKey key );
static void inp_clampView( Widget *inp );
static void inp_cleanup( Widget* inp );
static void inp_focusGain( Widget* inp );
static void inp_focusLose( Widget* inp );


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
   wgt->focusGain       = inp_focusGain;
   wgt->focusLose       = inp_focusLose;
   wgt->dat.inp.font    = (font != NULL) ? font : &gl_smallFont;
   wgt->dat.inp.max     = max+1;
   wgt->dat.inp.oneline = oneline;
   wgt->dat.inp.pos     = 0;
   wgt->dat.inp.view    = 0;
   wgt->dat.inp.input   = calloc( wgt->dat.inp.max, 1 );
   wgt->dat.inp.fptr    = NULL;

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
   char buf[ 512 ], *str;
   int w, m, p, s;
   int lines;
   char c;

   x = bx + inp->x;
   y = by + inp->y;

   /* main background */
   toolkit_drawRect( x, y, inp->w, inp->h, &cWhite, NULL );

   if (inp->dat.inp.oneline)
      /* center vertically */
      ty = y - (inp->h - gl_smallFont.h)/2.;
   else {
      /* Align top-left. */
      ty = y - gl_smallFont.h / 2.;
   }

   /* Draw text. */
   gl_printTextRaw( inp->dat.inp.font, inp->w-10., inp->h,
         x+5., ty, &cBlack, &inp->dat.inp.input[ inp->dat.inp.view ] );

   /* Draw cursor. */
   if (wgt_isFlag( inp, WGT_FLAG_FOCUSED )) {
      if (inp->dat.inp.oneline) {
         m = MIN( inp->dat.inp.pos - inp->dat.inp.view, (int)sizeof(buf)-1 );
         strncpy( buf, &inp->dat.inp.input[ inp->dat.inp.view ], m );
         buf[ m ] = '\0';
         w = gl_printWidthRaw( inp->dat.inp.font, buf );
         toolkit_drawRect( x + 5. + w, y + (inp->h - inp->dat.inp.font->h - 4.)/2.,
               1., inp->dat.inp.font->h + 4., &cBlack, &cBlack );
      }
      else {
         /* Wrap the cursor around if the text is longer than the width of the widget. */
         str   = inp->dat.inp.input;
         w     = 0;
         p     = 0;
         lines = 0;
         s     = 0;
         do {
            p     += w;
            if ((s != 0) && ((str[p] == '\n') || (str[p] == ' ')))
               p++;
            s      = 1;
            w      = gl_printWidthForText( inp->dat.inp.font, &str[p], inp->w-10 );
            lines += 1;
            if (str[p+w] == '\0')
               break;
         } while (p+w < inp->dat.inp.pos);

         /* Hack because we have to avoid wraps when counting lines, so here we
          * handle the last line partially. */
         c = str[ inp->dat.inp.pos ];
         str[ inp->dat.inp.pos ] = '\0';
         w = gl_printWidthRaw( inp->dat.inp.font, &str[p] );
         str[ inp->dat.inp.pos ] = c;

         /* Get the actual width now. */
         toolkit_drawRect( x + 5. + w, y + inp->h - lines * (inp->dat.inp.font->h + 5) - 3.,
               1., inp->dat.inp.font->h + 4., &cBlack, &cBlack );
      }
   }

   /* inner outline */
   toolkit_drawOutline( x, y, inp->w, inp->h, 0.,
         toolkit_colLight, toolkit_col );
   /* outer outline */
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

   if (ret && inp->dat.inp.fptr != NULL)
      inp->dat.inp.fptr( inp->wdw, inp->name );

   return ret;
}


/**
 * @brief Adds a single key to the input.
 *
 *    @param inp Input widget receiving the key.
 *    @param key Key to receive.
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

   /* Check to see if is in filter to ignore. */
   if (inp->dat.inp.filter != NULL)
      for (i=0; inp->dat.inp.filter[i] != '\0'; i++)
         if (inp->dat.inp.filter[i] == c)
            return 1; /* Ignored. */

   /* Make sure it's not full. */
   if (strlen(inp->dat.inp.input) >= (size_t)inp->dat.inp.max-1)
      return 1;

   /* Add key. */
   memmove( &inp->dat.inp.input[ inp->dat.inp.pos+1 ],
         &inp->dat.inp.input[ inp->dat.inp.pos ],
         inp->dat.inp.max - inp->dat.inp.pos - 2 );
   inp->dat.inp.input[ inp->dat.inp.pos++ ] = c;
   inp->dat.inp.input[ inp->dat.inp.max-1 ] = '\0';

   if (inp->dat.inp.oneline) {
      /* We can't wrap the text, so we need to scroll it out. */
      n = gl_printWidthRaw( inp->dat.inp.font, inp->dat.inp.input+inp->dat.inp.view );
      if (n+10 > inp->w)
         inp->dat.inp.view++;
   }

   return 1;
}

/**
 * @brief Checks if a character is a breaker character (for editing purposes)
 *
 *    @param c character to check.
 *    @return 1 if the char is a breaker, 0 if it isn't.
 */
static int inp_isBreaker(char c)
{
   char* breakers = ";:.-_ \n";
   int i;

   for (i = 0; i < (int)strlen(breakers); i++) {
      if (breakers[i] == c)
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
   int n, curpos, prevpos, curchars, prevchars, charsfromleft, lines;
   int len;
   char* str;

   /*
    * Handle arrow keys.
    */
    if ((key == SDLK_LEFT) ||
         (key == SDLK_RIGHT) ||
         (key == SDLK_UP) ||
         (key == SDLK_DOWN)) {
      /* Move pointer. */
      if (key == SDLK_LEFT) {
         if (inp->dat.inp.pos > 0) {
            if (mod & KMOD_CTRL) {
               /* We want to position the cursor at the start of the previous or current word. */
               /* Begin by skipping all breakers. */
               while (inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1]) && inp->dat.inp.pos > 0) {
                  inp->dat.inp.pos--;
               }
               /* Now skip until we encounter a breaker (or SOL). */
               while (!inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1]) && inp->dat.inp.pos > 0) {
                  inp->dat.inp.pos--;
               }
            }
            else
               inp->dat.inp.pos -= 1;

            inp_clampView( inp );
         }
      }
      else if (key == SDLK_RIGHT) {
         len = (int)strlen(inp->dat.inp.input);
         if (inp->dat.inp.pos < len) {
            if (mod & KMOD_CTRL) {
               /* We want to position the cursor at the start of the next word. */
               /* Begin by skipping all non-breakers. */
               while (!inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos])
                     && (inp->dat.inp.pos < len)) {
                  inp->dat.inp.pos++;
               }
               /* Now skip until we encounter a non-breaker (or EOL). */
               while (inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos])
                     && (inp->dat.inp.pos < len)) {
                  inp->dat.inp.pos++;
               }
            }
            else
               inp->dat.inp.pos += 1;

            inp_clampView( inp );
         }
      }
      else if (key == SDLK_UP) {
         if (inp->dat.inp.oneline)
            return 0;

         str      = inp->dat.inp.input;
         curpos   = 0;
         prevpos  = 0;
         curchars = 0;
         lines    = 0;

         if (inp->dat.inp.pos == 0) /* We can't move beyond the current line, as it is the first one. */
            return 1;

         /* Keep not-printing the lines until the current pos is smaller than the virtual pos.
          * At this point, we've arrived at the line the cursor is on. */
         while (inp->dat.inp.pos > curpos) {
            prevpos   = curpos;
            prevchars = curchars;

            curchars  = gl_printWidthForText( inp->dat.inp.font, &str[curpos], inp->w-10 );
            curpos   += curchars;
            /* Handle newlines. */
            if (str[curpos] == '\n') {
               curchars++;
               curpos++;
            }

            lines++;
         }

         /* Set the pos to the same number of characters from the left hand
          * edge, on the previous line (unless there aren't that many chars).
          * This is more or less equal to going up a line. */
         charsfromleft     = inp->dat.inp.pos - prevpos;
         /* Hack for moving up to the first line. */
         if (lines == 2)
            charsfromleft--;

         inp->dat.inp.pos  = prevpos - prevchars;
         inp->dat.inp.pos += MIN(charsfromleft, prevchars);
      }
      else if (key == SDLK_DOWN) {
         if (inp->dat.inp.oneline)
            return 0;

         str      = inp->dat.inp.input;
         curpos   = 0;
         prevpos  = 0;
         curchars = 0;
         lines    = 0;

         /* We can't move beyond the current line, as it is the last one. */
         if (inp->dat.inp.pos == (int)strlen(inp->dat.inp.input))
            return 1;

         /* Keep not-printing the lines until the current pos is smaller than the virtual pos.
          * At this point, we've arrived at the line the cursor is on. */
         while (inp->dat.inp.pos >= curpos) {
            prevpos   = curpos;
            prevchars = curchars;

            curchars  = gl_printWidthForText( inp->dat.inp.font, &str[curpos], inp->w-10 );
            curpos   += curchars;
            /* Handle newlines. */
            if (str[curpos] == '\n') {
               curchars++;
               curpos++;
            }
            lines++;
         }

         /* Take note how many chars from the left we have. */
         charsfromleft = inp->dat.inp.pos - prevpos;
         /* Hack for moving down from the first line. */
         if (lines == 1)
            charsfromleft++;

         /* Now not-print one more line. This is the line we want to move the cursor to. */
         prevpos   = curpos;
         prevchars = curchars;
         curchars  = gl_printWidthForText( inp->dat.inp.font, &str[curpos], inp->w-10 );
         curpos   += curchars;

         /* Set the pos to the same number of characters from the left hand
          * edge, on this line (unless there aren't that many chars).
          * This is more or less equal to going down a line.
          * But make sure never to go past the end of the string. */
         inp->dat.inp.pos  = prevpos;
         inp->dat.inp.pos += MIN(charsfromleft, curchars);
         inp->dat.inp.pos  = MIN(inp->dat.inp.pos, (int)strlen(inp->dat.inp.input));
      }

      return 1;
   }


#if SDL_VERSION_ATLEAST(2,0,0)
   /* Don't use, but don't eat, either. */
   if ((key == SDLK_TAB) || (key == SDLK_ESCAPE))
      return 0;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* Eat everything else that isn't usable. Om nom. */
   /* Only catch some keys. */
   if ((key != SDLK_BACKSPACE) &&
         (key != SDLK_DELETE) &&
         (key != SDLK_RETURN) &&
         (key != SDLK_KP_ENTER) &&
         (key != SDLK_HOME) &&
         (key != SDLK_END) &&
         (key != SDLK_PAGEUP) &&
         (key != SDLK_PAGEDOWN))
#if SDL_VERSION_ATLEAST(2,0,0)
      return 1; /* SDL2 uses TextInput and should eat most keys. Om nom. */
#else /* SDL_VERSION_ATLEAST(2,0,0) */
      return 0;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* backspace -> delete text */
   if (key == SDLK_BACKSPACE) {
      if (inp->dat.inp.pos <= 0)
         return 1; /* We still catch the event. */

      /* We want to move inp->dat.inp.pos backward and delete all characters caught between it and curpos at the end. */
      curpos = inp->dat.inp.pos;
      if (inp->dat.inp.pos > 0) {
         if (mod & KMOD_CTRL) {
            /* We want to delete up to the start of the previous or current word. */
            /* Begin by skipping all breakers. */
            while (inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1]) && inp->dat.inp.pos > 0) {
               inp->dat.inp.pos--;
            }
            /* Now skip until we encounter a breaker (or SOL). */
            while (!inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1]) && inp->dat.inp.pos > 0) {
               inp->dat.inp.pos--;
            }
         }
         else {
            inp->dat.inp.pos--;
         }
      }
      /* Actually delete the chars. */
      memmove( &inp->dat.inp.input[ inp->dat.inp.pos ],
            &inp->dat.inp.input[ curpos ],
            (inp->dat.inp.max - curpos) );
      inp->dat.inp.input[ inp->dat.inp.max - curpos + inp->dat.inp.pos ] = '\0';

      if (inp->dat.inp.oneline && inp->dat.inp.view > 0) {
         n = gl_printWidthRaw( &gl_smallFont,
               inp->dat.inp.input + inp->dat.inp.view - 1 );
         if (n+10 < inp->w)
            inp->dat.inp.view--;
      }

      if (inp->dat.inp.fptr != NULL)
         inp->dat.inp.fptr( inp->wdw, inp->name );

      inp_clampView( inp );
      return 1;
   }
   /* delete -> delete text */
   else if (key == SDLK_DELETE) {
      len = (int)strlen(inp->dat.inp.input);
      /* We want to move curpos forward and delete all characters caught between it and inp->dat.inp.pos at the end. */
      curpos = inp->dat.inp.pos;
      if (inp->dat.inp.pos < len) {
         if (mod & KMOD_CTRL) {
            /* We want to delete up until the start of the next word. */
            /* Begin by skipping all non-breakers. */
            while (!inp_isBreaker(inp->dat.inp.input[curpos])
                  && (curpos < len)) {
               curpos++;
            }
            /* Now skip until we encounter a non-breaker (or EOL). */
            while (inp_isBreaker(inp->dat.inp.input[curpos])
                  && (curpos < len)) {
               curpos++;
            }
         }
         else {
            curpos++;
         }
      }
      /* Actually delete the chars. */
      memmove( &inp->dat.inp.input[ inp->dat.inp.pos ],
            &inp->dat.inp.input[ curpos ],
            (inp->dat.inp.max - curpos) );
      inp->dat.inp.input[ inp->dat.inp.max - curpos + inp->dat.inp.pos ] = '\0';

      if (inp->dat.inp.fptr != NULL)
         inp->dat.inp.fptr( inp->wdw, inp->name );

      return 1;
   }
   /* home -> move to start */
   else if (key == SDLK_HOME) {
      inp->dat.inp.pos = 0;
      inp_clampView( inp );

      return 1;
   }
   /* end -> move to end */
   else if (key == SDLK_END) {
      inp->dat.inp.pos = strlen(inp->dat.inp.input);
      inp_clampView( inp );

      return 1;
   }

   /* in limits. */
   else if (key==SDLK_RETURN || key==SDLK_KP_ENTER) {
      if (inp->dat.inp.oneline)
         return 0; /* Enter does not work in one-liners. */
      /* Empty. */
      if ((inp->dat.inp.pos >= inp->dat.inp.max-1))
         return 1;

      memmove( &inp->dat.inp.input[ inp->dat.inp.pos+1 ],
            &inp->dat.inp.input[ inp->dat.inp.pos ],
            inp->dat.inp.max - inp->dat.inp.pos - 2 );
      inp->dat.inp.input[ inp->dat.inp.pos++ ] = '\n';
      inp->dat.inp.input[ inp->dat.inp.max-1 ] = '\0'; /* Make sure it's NUL terminated. */

      if (inp->dat.inp.fptr != NULL)
         inp->dat.inp.fptr( inp->wdw, inp->name );

      return 1;
   }

   /* Nothing caught anything so just return. */
   return 0;
}


/*
 * @brief Keeps the input widget's view in sync with its cursor
 *
 *    @param inp Input widget to operate on.
 */
static void inp_clampView( Widget *inp )
{
   int visible;

   /* @todo Handle multiline input widgets. */
   if (!inp->dat.inp.oneline)
      return;

   /* If the cursor is behind the view, shift the view backwards. */
   if (inp->dat.inp.view > inp->dat.inp.pos) {
      inp->dat.inp.view = inp->dat.inp.pos;
      return;
   }

   visible = gl_printWidthForText( inp->dat.inp.font,
         &inp->dat.inp.input[ inp->dat.inp.view ], inp->w - 10 );

   /* Shift the view right until the cursor is visible. */
   while (inp->dat.inp.view + visible < inp->dat.inp.pos)
      visible = gl_printWidthForText( inp->dat.inp.font,
            &inp->dat.inp.input[ inp->dat.inp.view++ ], inp->w - 10 );
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
      memset( wgt->dat.inp.input, 0, wgt->dat.inp.max );
      wgt->dat.inp.pos     = 0;
      wgt->dat.inp.view    = 0;
   }
   else {
      strncpy( wgt->dat.inp.input, msg, wgt->dat.inp.max );
      wgt->dat.inp.input[ wgt->dat.inp.max-1 ] = '\0';
      wgt->dat.inp.pos = strlen( wgt->dat.inp.input );
   }

   /* Get the value. */
   if (wgt->dat.inp.fptr != NULL)
      wgt->dat.inp.fptr( wid, name );

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

/**
 * @brief Sets the callback used when the input's text is modified.
 *
 *    @param wid Window to which input widget belongs.
 *    @param name Input widget to set callback for.
 *    @param fptr Function to trigger when the input's text is modified.
 */
void window_setInputCallback( const unsigned int wid, char* name, void (*fptr)(unsigned int, char*) )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid, name);
   if (wgt == NULL)
      return;

   /* Check the type. */
   if (wgt->type != WIDGET_INPUT) {
      WARN("Trying to set callback on non-input widget '%s'.", name);
      return;
   }

   wgt->dat.inp.fptr = fptr;
}

/**
 * @brief Input widget gains focus.
 *
 *    @param inp Widget gaining the focus.
 */
static void inp_focusGain( Widget* inp )
{
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_Rect input_pos;

   input_pos.x = (int)inp->x;
   input_pos.y = (int)inp->y;
   input_pos.w = (int)inp->w;
   input_pos.h = (int)inp->h;

   SDL_StartTextInput();
   SDL_SetTextInputRect( &input_pos );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   (void) inp;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
}

/**
 * @brief Input widget loses focus.
 *
 *    @param inp Widget losing the focus.
 */
static void inp_focusLose( Widget* inp )
{
   (void) inp;
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_StopTextInput();
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
}


