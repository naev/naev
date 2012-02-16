/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file font.c
 *
 * @brief OpenGL font rendering routines.
 *
 * Use a displaylist to store ASCII chars rendered with freefont
 * There are several drawing methods depending on whether you want
 * print it all, print to a max width, print centered or print a
 * block of text.
 *
 * There are hard-coded size limits.  256 characters for all routines
 * except gl_printText which has a 1024 limit.
 *
 * @todo check if length is too long
 */


#include "font.h"

#include "naev.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "log.h"
#include "ndata.h"


#define FONT_DEF  "dat/font.ttf" /**< Default font path. */


/**
 * @brief Stores a font character.
 */
typedef struct font_char_s {
   GLubyte *data; /**< Data of the character. */
   int w; /**< Width. */
   int h; /**< Height. */
   int off_x; /**< X offset when rendering. */
   int off_y; /**< Y offset when rendering. */
   int adv_x; /**< X advancement. */
   int adv_y; /**< Y advancement. */
   int tx; /**< Texture x position. */
   int ty; /**< Texture y position. */
   int tw; /**< Texture width. */
   int th; /**< Texture height. */
} font_char_t;


/* default font */
glFont gl_defFont; /**< Default font. */
glFont gl_smallFont; /**< Small font. */


/* Last used colour. */
static const glColour *font_lastCol    = NULL; /**< Stores last colour used (activated by '\e'). */
static int font_restoreLast      = 0; /**< Restore last colour. */


/*
 * prototypes
 */
static int font_limitSize( const glFont *ft_font, int *width,
      const char *text, const int max );
static const glColour* gl_fontGetColour( int ch );
/* Render. */
static void gl_fontRenderStart( const glFont* font, double x, double y, const glColour *c );
static int gl_fontRenderCharacter( const glFont* font, int ch, const glColour *c, int state );
static void gl_fontRenderEnd (void);


/**
 * @brief Clears the restoration.
 */
void gl_printRestoreClear (void)
{
   font_lastCol = NULL;
}


/**
 * @brief Restores last colour.
 */
void gl_printRestoreLast (void)
{
   if (font_lastCol != NULL)
      font_restoreLast = 1;
}


/**
 * @brief Initializes a restore structure.
 *    @param restore Structure to initialize.
 */
void gl_printRestoreInit( glFontRestore *restore )
{
   memset( restore, 0, sizeof(glFontRestore) );
}


/**
 * @brief Restores last colour from a restore structure.
 *    @param restore Structure to restore.
 */
void gl_printRestore( const glFontRestore *restore )
{
   if (restore->col != NULL) {
      font_lastCol = restore->col;
      font_restoreLast = 1;
   }
}


/**
 * @brief Stores the colour information from a piece of text limited to max characters.
 *    @param restore Structure to save colour information to.
 *    @param text Text to extract colour information from.
 *    @param max Maximum number of characters to process.
 */
void gl_printStoreMax( glFontRestore *restore, const char *text, int max )
{
   int i;
   const glColour *col;

   col = restore->col; /* Use whatever is there. */
   for (i=0; (text[i]!='\0') && (i<=max); i++) {
      /* Only want escape sequences. */
      if (text[i] != '\e')
         continue;

      /* Get colour. */
      if ((i+1<=max) && (text[i+1]!='\0')) {
         col = gl_fontGetColour( text[i+1] );
         i += 1;
      }
   }

   restore->col = col;
}


/**
 * @brief Stores the colour information from a piece of text.
 *    @param restore Structure to save colour information to.
 *    @param text Text to extract colour information from.
 */
void gl_printStore( glFontRestore *restore, const char *text )
{
   gl_printStoreMax( restore, text, INT_MAX );
}


/**
 * @brief Limits the text to max.
 *
 *    @param ft_font Font to calculate width with.
 *    @param width Actual width it takes up.
 *    @param text Text to parse.
 *    @param max Max to look for.
 *    @return Number of characters that fit.
 */
static int font_limitSize( const glFont *ft_font, int *width,
      const char *text, const int max )
{
   int n, i;

   /* Avoid segfaults. */
   if (text == NULL)
      return 0;

   /* limit size */
   n = 0;
   for (i=0; text[i] != '\0'; i++) {
      /* Ignore escape sequence. */
      if (text[i] == '\e') {
         if (text[i+1] != '\0')
            i += 1;
         continue;
      }

      /* Count length. */
      n += ft_font->chars[ (int)text[i] ].adv_x;
      if (n > max) {
         n -= ft_font->chars[ (int)text[i] ].adv_x; /* actual size */
         break;
      }
   }

   if (width != NULL)
      (*width) = n;
   return i;
}


/**
 * @brief Gets the number of characters in text that fit into width.
 *
 *    @param ft_font Font to use.
 *    @param text Text to check.
 *    @param width Width to match.
 *    @return Number of characters that fit.
 */
int gl_printWidthForText( const glFont *ft_font, const char *text,
      const int width )
{
   int i, n, lastspace;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   /* limit size per line */
   lastspace = 0; /* last ' ' or '\n' in the text */
   n = 0; /* current width */
   i = 0; /* current position */
   while ((text[i] != '\n') && (text[i] != '\0')) {

      /* Characters we should ignore. */
      if (text[i] == '\t') {
         i++;
         continue;
      }

      /* Ignore escape sequence. */
      if (text[i] == '\e') {
         if (text[i+1] != '\0')
            i += 2;
         else
            i += 1;
         continue;
      }

      /* Increase size. */
      n += ft_font->chars[ (int)text[i] ].adv_x;

      /* Save last space. */
      if (text[i] == ' ')
         lastspace = i;

      /* Check if out of bounds. */
      if (n > width) {
         if (lastspace > 0)
            return lastspace;
         else
            return i-1;
      }

      /* Check next character. */
      i++;
   }

   return i;
}


/**
 * @brief Prints text on screen.
 *
 * Defaults ft_font to gl_defFont if NULL.
 *
 *    @param ft_font Font to use
 *    @param x X position to put text at.
 *    @param y Y position to put text at.
 *    @param c Colour to use (uses white if NULL)
 *    @param str String to display.
 */
void gl_printRaw( const glFont *ft_font,
      const double x, const double y,
      const glColour* c, const char *text )
{
   int i, s;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   /* Render it. */
   s = 0;
   gl_fontRenderStart(ft_font, x, y, c);
   for (i=0; text[i] != '\0'; i++)
      s = gl_fontRenderCharacter( ft_font, text[i], c, s );
   gl_fontRenderEnd();
}


/**
 * @brief Prints text on screen like printf.
 *
 * Defaults ft_font to gl_defFont if NULL.
 *
 *    @param ft_font Font to use (NULL means gl_defFont)
 *    @param x X position to put text at.
 *    @param y Y position to put text at.
 *    @param c Colour to use (uses white if NULL)
 *    @param fmt String formatted like printf to print.
 */
void gl_print( const glFont *ft_font,
      const double x, const double y,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[256]; /* holds the string */
   va_list ap;

   if (fmt == NULL) return;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsnprintf(text, 256, fmt, ap);
      va_end(ap);
   }

   gl_printRaw( ft_font, x, y, c, text );
}


/**
 * @brief Behaves like gl_printRaw but stops displaying text after a certain distance.
 *
 *    @param ft_font Font to use.
 *    @param max Maximum length to reach.
 *    @param x X position to display text at.
 *    @param y Y position to display text at.
 *    @param c Colour to use (NULL defaults to white).
 *    @param fmt String to display formatted like printf.
 *    @return The number of characters it had to suppress.
 */
int gl_printMaxRaw( const glFont *ft_font, const int max,
      const double x, const double y,
      const glColour* c, const char *text )
{
   int ret, i, s;

   ret = 0; /* default return value */

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   /* Limit size. */
   ret = font_limitSize( ft_font, NULL, text, max );

   /* Render it. */
   s = 0;
   gl_fontRenderStart(ft_font, x, y, c);
   for (i=0; i < ret; i++)
      s = gl_fontRenderCharacter( ft_font, text[i], c, s );
   gl_fontRenderEnd();

   return 0;
}
/**
 * @brief Behaves like gl_print but stops displaying text after reaching a certain length.
 *
 *    @param ft_font Font to use (NULL means use gl_defFont).
 *    @param max Maximum length to reach.
 *    @param x X position to display text at.
 *    @param y Y position to display text at.
 *    @param c Colour to use (NULL defaults to white).
 *    @param fmt String to display formatted like printf.
 *    @return The number of characters it had to suppress.
 */
int gl_printMax( const glFont *ft_font, const int max,
      const double x, const double y,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[256]; /* holds the string */
   va_list ap;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsnprintf(text, 256, fmt, ap);
      va_end(ap);
   }

   return gl_printMaxRaw( ft_font, max, x, y, c, text );
}


/**
 * @brief Displays text centered in position and width.
 *
 * Will truncate if text is too long.
 *
 *    @param ft_font Font to use.
 *    @param width Width of area to center in.
 *    @param x X position to display text at.
 *    @param y Y position to display text at.
 *    @param c Colour to use for text (NULL defaults to white).
 *    @param fmt Text to display formatted like printf.
 *    @return The number of characters it had to truncate.
 */
int gl_printMidRaw( const glFont *ft_font, const int width,
      double x, const double y,
      const glColour* c, const char *text )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   int n, ret, i, s;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   ret = 0; /* default return value */

   /* limit size */
   ret = font_limitSize( ft_font, &n, text, width );
   x += (double)(width - n)/2.;

   /* Render it. */
   s = 0;
   gl_fontRenderStart(ft_font, x, y, c);
   for (i=0; i < ret; i++)
      s = gl_fontRenderCharacter( ft_font, text[i], c, s );
   gl_fontRenderEnd();

   return 0;
}
/**
 * @brief Displays text centered in position and width.
 *
 * Will truncate if text is too long.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont)
 *    @param width Width of area to center in.
 *    @param x X position to display text at.
 *    @param y Y position to display text at.
 *    @param c Colour to use for text (NULL defaults to white).
 *    @param fmt Text to display formatted like printf.
 *    @return The number of characters it had to truncate.
 */
int gl_printMid( const glFont *ft_font, const int width,
      double x, const double y,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[256]; /* holds the string */
   va_list ap;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsnprintf(text, 256, fmt, ap);
      va_end(ap);
   }

   return gl_printMidRaw( ft_font, width, x, y, c, text );
}


/**
 * @brief Prints a block of text that fits in the dimensions given.
 *
 * Positions are based on origin being top-left.
 *
 *    @param ft_font Font to use.
 *    @param width Maximum width to print to.
 *    @param height Maximum height to print to.
 *    @param bx X position to display text at.
 *    @param by Y position to display text at.
 *    @param c Colour to use (NULL defaults to white).
 *    @param fmt Text to display formatted like printf.
 *    @return 0 on success.
 * prints text with line breaks included to a maximum width and height preset
 */
int gl_printTextRaw( const glFont *ft_font,
      const int width, const int height,
      double bx, double by,
      const glColour* c, const char *text )
{
   int ret, i, p, s;
   double x,y;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   x = bx;
   y = by + height - (double)ft_font->h; /* y is top left corner */

   /* Clears restoration. */
   gl_printRestoreClear();

   s = 0;
   p = 0; /* where we last drew up to */
   while (y - by > -1e-5) {
      ret = gl_printWidthForText( ft_font, &text[p], width );

      /* Must restore stuff. */
      gl_printRestoreLast();

      /* Render it. */
      gl_fontRenderStart(ft_font, x, y, c);
      for (i=0; i < ret; i++)
         s = gl_fontRenderCharacter( ft_font, text[p+i], c, s );
      gl_fontRenderEnd();

      if (text[p+i] == '\0')
         break;
      p += i;
      if ((text[p] == '\n') || (text[p] == ' '))
         p++; /* Skip "empty char". */
      y -= 1.5*(double)ft_font->h; /* move position down */
   }


   return 0;
}


/**
 * @brief Prints a block of text that fits in the dimensions given.
 *
 * Positions are based on origin being top-left.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param width Maximum width to print to.
 *    @param height Maximum height to print to.
 *    @param bx X position to display text at.
 *    @param by Y position to display text at.
 *    @param c Colour to use (NULL defaults to white).
 *    @param fmt Text to display formatted like printf.
 *    @return 0 on success.
 * prints text with line breaks included to a maximum width and height preset
 */
int gl_printText( const glFont *ft_font,
      const int width, const int height,
      double bx, double by,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[4096]; /* holds the string */
   va_list ap;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsnprintf(text, 4096, fmt, ap);
      va_end(ap);
   }

   return gl_printTextRaw( ft_font, width, height, bx, by, c, text );
}


/**
 * @brief Gets the width that it would take to print some text.
 *
 * Does not display text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param fmt Text to calculate the length of.
 *    @return The length of the text in pixels.
 */
int gl_printWidthRaw( const glFont *ft_font, const char *text )
{
   int i, n;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   for (n=0,i=0; i<(int)strlen(text); i++) {
      /* Ignore escape sequence. */
      if (text[i] == '\e') {
         if (text[i+1] != '\0')
            i += 2;
         else
            i += 1;
         continue;
      }

      /* Increment width. */
      n += ft_font->chars[ (int)text[i] ].adv_x;
   }

   return n;
}


/**
 * @brief Gets the width that it would take to print some text.
 *
 * Does not display text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param fmt Text to calculate the length of.
 *    @return The length of the text in pixels.
 */
int gl_printWidth( const glFont *ft_font, const char *fmt, ... )
{
   char text[256]; /* holds the string */
   va_list ap;

   if (fmt == NULL) return 0;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsnprintf(text, 256, fmt, ap);
      va_end(ap);
   }

   return gl_printWidthRaw( ft_font, text );
}


/**
 * @brief Gets the height of a non-formatted string.
 *
 * Does not display the text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param width Width to jump to next line once reached.
 *    @param fmt Text to get the height of in printf format.
 *    @return The height of the text.
 */
int gl_printHeightRaw( const glFont *ft_font,
      const int width, const char *text )
{
   int i, p;
   double y;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   /* Check 0 length strings. */
   if (text[0] == '\0')
      return 0;

   y = 0.;
   p = 0;
   do {
      i = gl_printWidthForText( ft_font, &text[p], width );
      p += i + 1;
      y += 1.5*(double)ft_font->h; /* move position down */
   } while (text[p-1] != '\0');

   return (int) (y - 0.5*(double)ft_font->h);
}

/**
 * @brief Gets the height of the text if it were printed.
 *
 * Does not display the text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param width Width to jump to next line once reached.
 *    @param fmt Text to get the height of in printf format.
 *    @return The height of the text.
 */
int gl_printHeight( const glFont *ft_font,
      const int width, const char *fmt, ... )
{
   char text[1024]; /* holds the string */
   va_list ap;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsnprintf(text, 1024, fmt, ap);
      va_end(ap);
   }

   return gl_printHeightRaw( ft_font, width, text );
}


/*
 *
 * G L _ F O N T
 *
 */
/**
 */
static int font_makeChar( font_char_t *c, FT_Face face, char ch )
{
   FT_Bitmap bitmap;
   FT_GlyphSlot slot;
   int w,h;

   slot = face->glyph; /* Small shortcut. */

   /* Load the glyph. */
   if (FT_Load_Char( face, ch, FT_LOAD_RENDER )) {
      WARN("FT_Load_Char failed.");
      return -1;
   }

   bitmap = slot->bitmap; /* to simplify */

   /* need the POT wrapping for opengl */
   w = bitmap.width;
   h = bitmap.rows;

   /* Store data. */
   c->data = malloc( sizeof(GLubyte) * w*h );
   memcpy( c->data, bitmap.buffer, sizeof(GLubyte) * w*h );
   c->w     = w;
   c->h     = h;
   c->off_x = slot->bitmap_left;
   c->off_y = slot->bitmap_top;
   c->adv_x = slot->advance.x >> 6;
   c->adv_y = slot->advance.y >> 6;
   return 0;
}


/**
 * @brief Generates the font's texture atlas.
 */
static int font_genTextureAtlas( glFont* font, FT_Face face )
{
   font_char_t chars[128];
   int i, n;
   int x, y, x_off, y_off;
   int total_w;
   int w, h, max_h;
   int offset;
   GLubyte *data;
   GLfloat *vbo_tex;
   GLshort *vbo_vert;
   GLfloat tx, ty, txw, tyh;
   GLfloat fw, fh;
   GLshort vx, vy, vw, vh;

   /* Render characters into software. */
   total_w  = 0;
   max_h    = 0;
   for (i=0; i<128; i++) {
      font_makeChar( &chars[i], face, i );
      total_w += chars[i].w;
      if (chars[i].h > max_h)
         max_h = chars[i].h;
   }

   /* Calculate how to fit them.
    * rows * Hmax = Wtotal / rows
    * rows^2 = Wtotal / Hmax
    * rows = sqrt( Wtotal / Hmax )
    */
   n = ceil( sqrt( (double)total_w / (double)max_h ) );
   w = ceil( total_w / n ) + 1;
   h = ceil( max_h * n ) + 1;

   /* Check if need to be POT. */
   if (1) { /*gl_needPOT()) { */ /** @TODO fix this stuff. */
      w = gl_pot(w);
      h = gl_pot(h);
   }

   /* Test fit - formula isn't perfect. */
   x_off = 0;
   y_off = 0;
   for (i=0; i<128; i++) {
      if (x_off + chars[i].w >= w) {
         x_off  = 0;
         y_off += max_h;

         /* Check for overflow. */
         if (y_off + max_h >= h) {
            h += max_h;

            /* POT needs even more. */
            if (1) /*gl_needPOT()) */ /** @TODO fix this stuff. */
               h = gl_pot(h);
         }
      }

      /* Displace offset. */
      x_off += chars[i].w;
   }

   /* Generate the texture. */
   data  = calloc( w*h*2, 1 );
   x_off = 0;
   y_off = 0;
   for (i=0; i<128; i++) {
      /* Check if need to skip to newline. */
      if (x_off + chars[i].w >= w) {
         x_off  = 0;
         y_off += max_h;

         if (y_off + max_h >= h)
            WARN("Font is still too small - something went wrong.");
      }

      /* Render character. */
      for (y=0; y<chars[i].h; y++) {
         for (x=0; x<chars[i].w; x++) {
            offset  = (y_off + y) * w;
            offset += x_off + x;
            data[ offset*2     ] = 0xcf; /* Constant luminance. */
            data[ offset*2 + 1 ] = chars[i].data[ y*chars[i].w + x ];
         }
      }

      /* Store character information. */
      font->chars[i].adv_x = chars[i].adv_x;
      font->chars[i].adv_y = chars[i].adv_y;

      /* Store temporary information. */
      chars[i].tx = x_off;
      chars[i].ty = y_off;
      chars[i].tw = chars[i].w;
      chars[i].th = chars[i].h;

      /* Displace offset. */
      x_off += chars[i].w;

      /* Free memory. */
      free(chars[i].data);
   }

   /* Create the font texture. */
   glGenTextures( 1, &font->texture );
   glBindTexture( GL_TEXTURE_2D, font->texture );

   /* Shouldn't ever scale - we'll generate appropriate size font. */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   /* Clamp texture .*/
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   /* Upload data. */
   glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0,
         GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data );

   /* Check for errors. */
   gl_checkErr();

   /* Create the VBOs. */
   n           = 8 * 128;
   vbo_tex     = malloc(sizeof(GLfloat) * n);
   vbo_vert    = malloc(sizeof(GLshort) * n);
   for (i=0; i<128; i++) {
      /* We do something like the following for vertex coordinates.
       *
       *
       *  +----------------- top reference   \  <------- font->h
       *  |                                  |
       *  |                                  | --- off_y
       *  +----------------- glyph top       /
       *  |
       *  |
       *  +----------------- glyph bottom
       *  |
       *  v   y
       *
       *
       *  +----+------------->  x
       *  |    |
       *  |    glyph start
       *  |
       *  side reference
       *
       *  \----/
       *   off_x
       */
      /* Temporary variables. */
      fw  = (GLfloat) w;
      fh  = (GLfloat) h;
      tx  = (GLfloat)chars[i].tx / fw;
      ty  = (GLfloat)chars[i].ty / fh;
      txw = (GLfloat)(chars[i].tx + chars[i].tw) / fw;
      tyh = (GLfloat)(chars[i].ty + chars[i].th) / fh;
      vx  = chars[i].off_x;
      vy  = chars[i].off_y - chars[i].h;
      vw  = chars[i].w;
      vh  = chars[i].h;
      /* Texture coords. */
      vbo_tex[  8*i + 0 ] = tx;  /* Top left. */
      vbo_tex[  8*i + 1 ] = ty;
      vbo_tex[  8*i + 2 ] = txw; /* Top right. */
      vbo_tex[  8*i + 3 ] = ty;
      vbo_tex[  8*i + 4 ] = txw; /* Bottom right. */
      vbo_tex[  8*i + 5 ] = tyh;
      vbo_tex[  8*i + 6 ] = tx;  /* Bottom left. */
      vbo_tex[  8*i + 7 ] = tyh;
      /* Vertex coords. */
      vbo_vert[ 8*i + 0 ] = vx;    /* Top left. */
      vbo_vert[ 8*i + 1 ] = vy+vh;
      vbo_vert[ 8*i + 2 ] = vx+vw; /* Top right. */
      vbo_vert[ 8*i + 3 ] = vy+vh;
      vbo_vert[ 8*i + 4 ] = vx+vw; /* Bottom right. */
      vbo_vert[ 8*i + 5 ] = vy;
      vbo_vert[ 8*i + 6 ] = vx;    /* Bottom left. */
      vbo_vert[ 8*i + 7 ] = vy;
   }
   font->vbo_tex  = gl_vboCreateStatic( sizeof(GLfloat)*n, vbo_tex );
   font->vbo_vert = gl_vboCreateStatic( sizeof(GLshort)*n, vbo_vert );

   /* Free the data. */
   free(data);
   free(vbo_tex);
   free(vbo_vert);

   return 0;
}


/**
 * @brief Starts the rendering engine.
 */
static void gl_fontRenderStart( const glFont* font, double x, double y, const glColour *c )
{
   double a;

   /* Enable textures. */
   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, font->texture);

   /* Set up matrix. */
   gl_matrixMode(GL_MODELVIEW);
   gl_matrixPush();
      gl_matrixTranslate( round(x), round(y) );

   /* Handle colour. */
   if (font_restoreLast) {
      a   = (c==NULL) ? 1. : c->a;
      ACOLOUR(*font_lastCol,a);
   }
   else {
      if (c==NULL)
         glColor4d( 1., 1., 1., 1. );
      else
         COLOUR(*c);
   }
   font_restoreLast = 0;

   /* Activate the appropriate VBOs. */
   gl_vboActivateOffset( font->vbo_tex,  GL_TEXTURE_COORD_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( font->vbo_vert, GL_VERTEX_ARRAY, 0, 2, GL_SHORT, 0 );
}


/**
 * @brief Gets the colour from a character.
 */
static const glColour* gl_fontGetColour( int ch )
{
   const glColour *col;
   switch (ch) {
      /* TOP SECRET COLOUR CONVENTION
       * FOR YOUR EYES ONLY
       *
       * Lowercase characters represent base colours.
       * Uppercase characters reperesent fancy game related colours.
       * Digits represent states.
       */
      /* Colours. */
      case 'r': col = &cFontRed; break;
      case 'g': col = &cFontGreen; break;
      case 'b': col = &cFontBlue; break;
      case 'y': col = &cFontYellow; break;
      case 'w': col = &cFontWhite; break;
      case 'p': col = &cFontPurple; break;
      case 'n': col = &cBlack; break;
      /* Fancy states. */
      case 'F': col = &cFriend; break;
      case 'H': col = &cHostile; break;
      case 'N': col = &cNeutral; break;
      case 'I': col = &cInert; break;
      case 'R': col = &cRestricted; break;
      case 'S': col = &cDRestricted; break;
      case 'M': col = &cMapNeutral; break;
      case 'C': col = &cConsole; break;
      case 'D': col = &cDConsole; break;
      case '0': col = NULL; break;
      default: col = NULL; break;
   }
   return col;
}


/**
 * @brief Renders a character.
 */
static int gl_fontRenderCharacter( const glFont* font, int ch, const glColour *c, int state )
{
   GLushort ind[6];
   double a;
   const glColour *col;

   /* Handle escape sequences. */
   if (ch == '\e') /* Start sequence. */
      return 1;
   if (state == 1) {
      col = gl_fontGetColour( ch );
      a   = (c==NULL) ? 1. : c->a;
      if (col == NULL) {
         if (c==NULL)
            glColor4d( 1., 1., 1., 1. );
         else
            COLOUR(*c);
      }
      else
         ACOLOUR(*col,a);
      font_lastCol = col;
      return 0;
   }

   if (!isspace(ch)) {
      /*
       * Global  Local
       * 0--1      0--1 4
       * | /|  =>  | / /|
       * |/ |      |/ / |
       * 3--2      2 3--5
       */
      ind[0] = 4*ch + 0;
      ind[1] = 4*ch + 1;
      ind[2] = 4*ch + 3;
      ind[3] = 4*ch + 1;
      ind[4] = 4*ch + 3;
      ind[5] = 4*ch + 2;

      /* Draw the element. */
      glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, ind );
   }

   /* Translate matrix. */
   gl_matrixTranslate( font->chars[ch].adv_x, font->chars[ch].adv_y );

   return 0;
}


/**
 * @brief Ends the rendering engine.
 */
static void gl_fontRenderEnd (void)
{
   gl_vboDeactivate();
   gl_matrixPop();
   gl_matrixMode( GL_PROJECTION );
   glDisable(GL_TEXTURE_2D);

   /* Check for errors. */
   gl_checkErr();
}


/**
 * @brief Initializes a font.
 *
 *    @param font Font to load (NULL defaults to gl_defFont).
 *    @param fname Name of the font (from inside packfile, NULL defaults to default font).
 *    @param h Height of the font to generate.
 */
void gl_fontInit( glFont* font, const char *fname, const unsigned int h )
{
   FT_Library library;
   FT_Face face;
   uint32_t bufsize;
   FT_Byte* buf;

   /* Get default font if not set. */
   if (font == NULL)
      font = &gl_defFont;

   /* Read the font. */
   buf = ndata_read( (fname!=NULL) ? fname : FONT_DEF, &bufsize );
   if (buf == NULL) {
      WARN("Unable to read font: %s", (fname!=NULL) ? fname : FONT_DEF);
      return;
   }

   /* Allocage. */
   font->chars = malloc(sizeof(glFontChar)*128);
   font->h = (int)floor((double)h * gl_screen.scale);
   if (font->chars==NULL) {
      WARN("Out of memory!");
      return;
   }

   /* create a FreeType font library */
   if (FT_Init_FreeType(&library)) {
      WARN("FT_Init_FreeType failed with font %s.",
            (fname!=NULL) ? fname : FONT_DEF );
      return;
   }

   /* object which freetype uses to store font info */
   if (FT_New_Memory_Face( library, buf, bufsize, 0, &face )) {
      WARN("FT_New_Face failed loading library from %s",
            (fname!=NULL) ? fname : FONT_DEF );
      return;
   }

   /* Try to resize. */
   if (FT_IS_SCALABLE(face)) {
      if (FT_Set_Char_Size( face,
               0, /* Same as width. */
               h << 6, /* In 1/64th of a pixel. */
               96, /* Create at 96 DPI */
               96)) /* Create at 96 DPI */
         WARN("FT_Set_Char_Size failed.");
   }
   else
      WARN("Font isn't resizable!");

   /* Select the character map. */
   if (FT_Select_Charmap( face, FT_ENCODING_UNICODE ))
      WARN("FT_Select_Charmap failed to change character mapping.");

   /* Generate the font atlas. */
   font_genTextureAtlas( font, face );

   /* we can now free the face and library */
   FT_Done_Face(face);
   FT_Done_FreeType(library);
   free(buf);
}

/**
 * @brief Frees a loaded font.
 *
 *    @param font Font to free.
 */
void gl_freeFont( glFont* font )
{
   if (font == NULL)
      font = &gl_defFont;
   glDeleteTextures(1,&font->texture);
   if (font->chars != NULL)
      free(font->chars);
   font->chars = NULL;
   if (font->vbo_tex != NULL)
      gl_vboDestroy(font->vbo_tex);
   font->vbo_tex = NULL;
   if (font->vbo_vert != NULL)
      gl_vboDestroy(font->vbo_vert);
   font->vbo_vert = NULL;
}
