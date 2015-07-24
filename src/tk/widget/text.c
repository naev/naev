/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file text.c
 *
 * @brief Text widget.
 */


#include "tk/toolkit_priv.h"

#include <stdlib.h>
#include "nstring.h"


static void txt_render( Widget* txt, double bx, double by );
static void txt_cleanup( Widget* txt );


/**
 * @brief Adds a text widget to the window.
 *
 * Position origin is 0,0 at bottom left.  If you use negative X or Y
 *  positions.  They actually count from the opposite side in.
 *
 *    @param wid ID of the window to add the widget to.
 *    @param x X position within the window to use.
 *    @param y Y position within the window to use.
 *    @param w Maximum width of the text.
 *    @param h Maximum height of the text.
 *    @param centered Whether text should be centered.
 *    @param name Name of the widget to use internally.
 *    @param font Font to use (NULL is default).
 *    @param colour Colour to use (NULL is default).
 *    @param string Text to display.
 */
void window_addText( const unsigned int wid,
                     const int x, const int y,
                     const int w, const int h,
                     const int centered, const char* name,
                     glFont* font, const glColour* colour, const char* string )
{
   Window *wdw = window_wget(wid);
   Widget *wgt = window_newWidget(wdw, name);
   if (wgt == NULL)
      return;

   /* generic */
   wgt->type = WIDGET_TEXT;

   /* specific */
   wgt->render             = txt_render;
   wgt->cleanup            = txt_cleanup;
   wgt->dat.txt.font       = (font==NULL) ? &gl_defFont : font;
   wgt->dat.txt.colour     = (colour==NULL) ? cBlack : *colour;
   wgt->dat.txt.centered   = centered;
   wgt->dat.txt.text       = (string==NULL) ? NULL : strdup(string);

   /* position/size */
   wgt->w = (double) w;
   wgt->h = (double) h;
   toolkit_setPos( wdw, wgt, x, y );
}


/**
 * @brief Renders a text widget.
 *
 *    @param txt Text widget to render.
 *    @param bx Base X position.
 *    @param by Base Y position.
 */
static void txt_render( Widget* txt, double bx, double by )
{
   /* Must have text to display. */
   if (txt->dat.txt.text==NULL)
      return;

   if (txt->dat.txt.centered)
      gl_printMidRaw( txt->dat.txt.font, txt->w,
            bx + txt->x,
            by + txt->y + (txt->h - txt->dat.txt.font->h)/2.,
            &txt->dat.txt.colour, txt->dat.txt.text );
   else
      gl_printTextRaw( txt->dat.txt.font, txt->w, txt->h,
            bx + txt->x, by + txt->y,
            &txt->dat.txt.colour, txt->dat.txt.text );
}


/**
 * @brief Clean up function for the text widget.
 *
 *    @param txt Text widget to clean up.
 */
static void txt_cleanup( Widget* txt )
{
   if (txt->dat.txt.text != NULL)
      free(txt->dat.txt.text);
}


/**
 * @brief Modifies an existing text widget.
 *
 *    @param wid Window to which the text widget belongs.
 *    @param name Name of the text widget.
 *    @param newstring String to set for the text widget.
 */
void window_modifyText( const unsigned int wid,
      const char* name, const char* newstring )
{
   Widget *wgt;

   /* Get the widget. */
   wgt = window_getwgt(wid,name);
   if (wgt == NULL)
      return;

   /* Check type. */
   if (wgt->type != WIDGET_TEXT) {
      WARN("Not modifying text on non-text widget '%s'.", name);
      return;
   }

   /* Set text. */
   if (wgt->dat.txt.text)
      free(wgt->dat.txt.text);
   wgt->dat.txt.text = (newstring) ?  strdup(newstring) : NULL;
}

