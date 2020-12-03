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
#include "utf8.h"


static void inp_render( Widget* inp, double bx, double by );
static int inp_isBreaker(char c);
static int inp_key( Widget* inp, SDL_Keycode key, SDL_Keymod mod );
static int inp_text( Widget* inp, const char *buf );
static int inp_addKey( Widget* inp, uint32_t ch );
static int inp_rangeToWidth( Widget *inp, int start_pos, int end_pos );
static int inp_rangeFromWidth( Widget *inp, int start_pos, int width );
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
   wgt->dat.inp.char_max= max+1;
   wgt->dat.inp.byte_max= 4*max+1;
   wgt->dat.inp.oneline = oneline;
   wgt->dat.inp.pos     = 0;
   wgt->dat.inp.view    = 0;
   wgt->dat.inp.input   = calloc( wgt->dat.inp.byte_max, 1 ); /* Maximum length of a unicode character is 4 bytes. */
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
   char *str;
   int s;
   size_t p, w;
   int lines;

   x = bx + inp->x;
   y = by + inp->y;

   /* main background */
   toolkit_drawRect( x - 4, y - 4, inp->w + 8, inp->h + 8, &cBlack, NULL );

   /* Draw text. */
   if (inp->dat.inp.oneline) {
      /* center vertically, print whatever text fits regardless of word boundaries. */
      ty = y + (inp->h - inp->dat.inp.font->h)/2.;
      gl_printMaxRaw( inp->dat.inp.font, inp->w-10.,
            x+5., ty, &cGreen, -1., &inp->dat.inp.input[ inp->dat.inp.view ] );
   }
   else {
      /* Align top-left, print with word wrapping. */
      ty = y - inp->dat.inp.font->h / 2.;
      gl_printTextRaw( inp->dat.inp.font, inp->w-10., inp->h,
            x+5., ty, &cGreen, -1., &inp->dat.inp.input[ inp->dat.inp.view ] );
   }

   /* Draw cursor. */
   if (wgt_isFlag( inp, WGT_FLAG_FOCUSED )) {
      if (inp->dat.inp.oneline) {
         w = inp_rangeToWidth( inp, inp->dat.inp.view, inp->dat.inp.pos );
         toolkit_drawRect( x + 5. + w, y + (inp->h - inp->dat.inp.font->h - 4.)/2.,
               1., inp->dat.inp.font->h + 4., &cGreen, &cGreen );
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
            w      = inp_rangeFromWidth( inp, p, -1 );
            lines += 1;
            if (str[p+w] == '\0')
               break;
         } while (p+w < inp->dat.inp.pos);

         /* On the final line, no word-wrap is possible. */
         w = inp_rangeToWidth( inp, p, inp->dat.inp.pos );

         /* Get the actual width now. */
         toolkit_drawRect( x + 5. + w, y + inp->h - lines * (inp->dat.inp.font->h + 5) - 3.,
               1., inp->dat.inp.font->h + 4., &cGreen, &cGreen );
      }
   }

   /* inner outline */
   /* toolkit_drawOutline( x, y, inp->w, inp->h, 0.,
         toolkit_colLight, NULL ); */
   /* outer outline */
   toolkit_drawOutline( x-2, y-2, inp->w + 4, inp->h + 4, 1.,
         &cGrey20, NULL );
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
   size_t i;
   int ret;
   uint32_t ch;

   i = 0;
   ret = 0;
   while ((ch = u8_nextchar( buf, &i )))
      ret |= inp_addKey( inp, ch );

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
static int inp_addKey( Widget* inp, uint32_t ch )
{
   size_t i, len;
   int n;
   uint32_t c;
   char buf[8];

   /* Check to see if is in filter to ignore. */
   if (inp->dat.inp.filter != NULL) {
      i = 0;
      while ((c = u8_nextchar( inp->dat.inp.filter, &i )))
         if (c == ch)
            return 1; /* Ignored. */
   }

   /* Make sure it's not full. */
   if (u8_strlen(inp->dat.inp.input) >= inp->dat.inp.char_max-1)
      return 1;

   /* Render back to utf8. */
   len = u8_toutf8( buf, sizeof(buf), &ch, 1 );

   /* Add key. */
   memmove( &inp->dat.inp.input[ inp->dat.inp.pos+len ],
         &inp->dat.inp.input[ inp->dat.inp.pos ],
         inp->dat.inp.byte_max-1 - inp->dat.inp.pos - len );
   for (i=0; i<len; i++)
      inp->dat.inp.input[ inp->dat.inp.pos++ ] = buf[i];
   assert(inp->dat.inp.input[ inp->dat.inp.byte_max-1 ] == '\0');

   while (inp->dat.inp.oneline) {
      /* We can't wrap the text, so we need to scroll it out. */
      n = inp_rangeToWidth( inp, inp->dat.inp.view, -1 );
      if (n+10 <= inp->w)
         break;
      u8_inc( inp->dat.inp.input, &inp->dat.inp.view );
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
   return strchr(";:.-_ \n", c) != NULL;
}


/**
 * @brief Handles input for an input widget.
 *
 *    @param inp Input widget to handle event.
 *    @param key Key being handled.
 *    @param mod Mods when key is being pressed.
 *    @return 1 if the event was used, 0 if it wasn't.
 */
static int inp_key( Widget* inp, SDL_Keycode key, SDL_Keymod mod )
{
   (void) mod;
   int n, w;
   size_t curpos, len, prev_line_start, curr_line_start, line_end, next_line_start;

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
               while (inp->dat.inp.pos > 0 && inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1])) {
                  u8_dec( inp->dat.inp.input, &inp->dat.inp.pos );
               }
               /* Now skip until we encounter a breaker (or SOL). */
               while (inp->dat.inp.pos > 0 && !inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1])) {
                  u8_dec( inp->dat.inp.input, &inp->dat.inp.pos );
               }
            }
            else
               u8_dec( inp->dat.inp.input, &inp->dat.inp.pos );
         }
      }
      else if (key == SDLK_RIGHT) {
         len = strlen(inp->dat.inp.input);
         if (inp->dat.inp.pos < len) {
            if (mod & KMOD_CTRL) {
               /* We want to position the cursor at the start of the next word. */
               /* Begin by skipping all non-breakers. */
               while (!inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos])
                     && (inp->dat.inp.pos < len)) {
                  u8_inc( inp->dat.inp.input, &inp->dat.inp.pos );
               }
               /* Now skip until we encounter a non-breaker (or EOL). */
               while (inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos])
                     && (inp->dat.inp.pos < len)) {
                  u8_inc( inp->dat.inp.input, &inp->dat.inp.pos );
               }
            }
            else
               u8_inc( inp->dat.inp.input, &inp->dat.inp.pos );
         }
      }
      else if (key == SDLK_UP || key == SDLK_DOWN) {
         if (inp->dat.inp.oneline)
            return 0;

         /* Keep a running list of the 3 most recent line-start positions found. SIZE_MAX is a sentinel. */
         curr_line_start = SIZE_MAX;
         next_line_start = 0;
         do {
            prev_line_start = curr_line_start;
            curr_line_start = next_line_start;
            line_end = curr_line_start + inp_rangeFromWidth( inp, curr_line_start, -1 );
            if (inp->dat.inp.input[line_end] == '\0')
               next_line_start = SIZE_MAX;
            else if (isspace(inp->dat.inp.input[line_end]))
               next_line_start = line_end + 1;
            else
               next_line_start = line_end;
         } while (line_end < inp->dat.inp.pos);

         w = inp_rangeToWidth( inp, curr_line_start, inp->dat.inp.pos );

         /* Extreme cases: moving to start/end of the whole input. */
         if (key == SDLK_UP && curr_line_start == 0)
            inp->dat.inp.pos = 0;
         else if (key == SDLK_DOWN && next_line_start == SIZE_MAX)
            inp->dat.inp.pos = strlen(inp->dat.inp.input);
         /* Main cases: aim for the same width into the target line. ISSUE: this logic skews left. */
         else if (key == SDLK_UP)
            inp->dat.inp.pos = prev_line_start + inp_rangeFromWidth( inp, prev_line_start, w );
         else
            inp->dat.inp.pos = next_line_start + inp_rangeFromWidth( inp, next_line_start, w );
      }

      inp_clampView( inp );
      return 1;
   }


   /* Don't use, but don't eat, either. */
   if ((key == SDLK_TAB) || (key == SDLK_ESCAPE))
      return 0;

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
      return 1; /* SDL2 uses TextInput and should eat most keys. Om nom. */

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
            while (inp->dat.inp.pos > 0 && inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1])) {
               u8_dec( inp->dat.inp.input, &inp->dat.inp.pos );
            }
            /* Now skip until we encounter a breaker (or SOL). */
            while (inp->dat.inp.pos > 0 && !inp_isBreaker(inp->dat.inp.input[inp->dat.inp.pos-1])) {
               u8_dec( inp->dat.inp.input, &inp->dat.inp.pos );
            }
         }
         else {
            u8_dec( inp->dat.inp.input, &inp->dat.inp.pos );
         }
      }
      /* Actually delete the chars. */
      memmove( &inp->dat.inp.input[ inp->dat.inp.pos ],
            &inp->dat.inp.input[ curpos ],
            inp->dat.inp.byte_max - curpos );
      assert(inp->dat.inp.input[ inp->dat.inp.byte_max-1 - curpos + inp->dat.inp.pos ] == '\0');

      while (inp->dat.inp.oneline && inp->dat.inp.view > 0) {
         n = inp_rangeToWidth( inp, inp->dat.inp.view - 1, -1 );
         if (n+10 >= inp->w)
            break;
	 inp->dat.inp.view -= MIN( curpos - inp->dat.inp.pos, inp->dat.inp.view );
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
               u8_inc( inp->dat.inp.input, &curpos );
            }
            /* Now skip until we encounter a non-breaker (or EOL). */
            while (inp_isBreaker(inp->dat.inp.input[curpos])
                  && (curpos < len)) {
               u8_inc( inp->dat.inp.input, &curpos );
            }
         }
         else {
            u8_inc( inp->dat.inp.input, &curpos );
         }
      }
      /* Actually delete the chars. */
      memmove( &inp->dat.inp.input[ inp->dat.inp.pos ],
            &inp->dat.inp.input[ curpos ],
            inp->dat.inp.byte_max - curpos );
      assert(inp->dat.inp.input[ inp->dat.inp.byte_max-1 - curpos + inp->dat.inp.pos ] == '\0');

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
      if (u8_strlen(inp->dat.inp.input) >= (size_t)inp->dat.inp.char_max-2)
         return 1;

      memmove( &inp->dat.inp.input[ inp->dat.inp.pos+1 ],
            &inp->dat.inp.input[ inp->dat.inp.pos ],
            inp->dat.inp.byte_max-1 - inp->dat.inp.pos - 1 );
      inp->dat.inp.input[ inp->dat.inp.pos++ ] = '\n';
      assert(inp->dat.inp.input[ inp->dat.inp.byte_max-1 ] == '\0');

      if (inp->dat.inp.fptr != NULL)
         inp->dat.inp.fptr( inp->wdw, inp->name );

      return 1;
   }

   /* Nothing caught anything so just return. */
   return 0;
}


/*
 * @brief Returns the width required to fit the text from byte positions \p start_pos to \p end_pos.
 *
 *    @param inp Input widget to operate on.
 *    @param start_pos Starting byte position (inclusive).
 *    @param start_pos Ending byte position (exclusive), or -1 for the end of the string.
 */
static int inp_rangeToWidth( Widget *inp, int start_pos, int end_pos )
{
   char c;
   int w;

   if (end_pos >= 0) {
      if (end_pos <= start_pos)
         return 0;
      c = inp->dat.inp.input[ inp->dat.inp.pos ];
      inp->dat.inp.input[ end_pos ] = '\0';
   }
   w = gl_printWidthRaw( inp->dat.inp.font, &inp->dat.inp.input[start_pos] );
   if (end_pos >= 0)
      inp->dat.inp.input[ end_pos ] = c;
   return w;
}


/*
 * @brief Returns the byte-size of the text we can fit within \p width starting at \p start_pos.
 *        Note: "for convenience" this function accounts for word-wrap if the widget is word-wrapping
 *        and width==-1; otherwise, it assumes we're measuring the characters within a line.
 *
 *    @param inp Input widget to operate on.
 *    @param start_pos Starting byte position.
 *    @param width Amount of horizontal space, or -1 for the width of the widget's text area.
 */
static int inp_rangeFromWidth( Widget *inp, int start_pos, int width )
{
   int tw, oneline;
   tw = width>=0 ? width : inp->w-10;
   oneline = width>=0 || inp->dat.inp.oneline;
   if (oneline)
      return gl_printWidthForTextLine( inp->dat.inp.font, &inp->dat.inp.input[start_pos], tw );
   return gl_printWidthForText( inp->dat.inp.font, &inp->dat.inp.input[start_pos], tw );
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

   visible = inp_rangeFromWidth( inp, inp->dat.inp.view, -1 );

   /* Shift the view right until the cursor is visible. */
   while (inp->dat.inp.view + visible < inp->dat.inp.pos) {
      visible = inp_rangeFromWidth( inp, inp->dat.inp.view, -1 );
      u8_inc( inp->dat.inp.input, &inp->dat.inp.view );
   }
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
      memset( wgt->dat.inp.input, 0, wgt->dat.inp.byte_max );
      wgt->dat.inp.pos     = 0;
      wgt->dat.inp.view    = 0;
   }
   else {
      strncpy( wgt->dat.inp.input, msg, wgt->dat.inp.byte_max );
      wgt->dat.inp.input[ wgt->dat.inp.byte_max-1 ] = '\0';
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
   SDL_Rect input_pos;
   Window *w;

   w = window_wget( inp->wdw );
   gl_screenToWindowPos( &input_pos.x, &input_pos.y, w->x + inp->x + 5., w->y + inp->y );
   input_pos.y -= inp->h;
   input_pos.w = (int)inp->w;
   input_pos.h = (int)inp->h;

   SDL_StartTextInput();
   SDL_SetTextInputRect( &input_pos );
}

/**
 * @brief Input widget loses focus.
 *
 *    @param inp Widget losing the focus.
 */
static void inp_focusLose( Widget* inp )
{
   (void) inp;
   SDL_StopTextInput();
}


