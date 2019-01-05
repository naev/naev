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

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <fontconfig/fontconfig.h>

#include "log.h"
#include "array.h"
#include "conf.h"
#include "ndata.h"
#include "nfile.h"
#include "utf8.h"

#define HASH_LUT_SIZE 512 /**< Size of glyph look up table. */
#define MAX_ROWS 128 /**< Max number of rows per texture cache. */
#define DEFAULT_TEXTURE_SIZE 1024 /**< Default size of texture caches for glyphs. */


/**
 * @brief Stores the row information for a font.
 */
typedef struct glFontRow_s {
   int x; /**< Current x offset. */
   int y; /**< Current y offset. */
   int h; /**< Height of the row. */
} glFontRow;


/**
 * @brief Stores a texture stash for fonts.
 */
typedef struct glFontTex_s {
   GLuint id; /**< Texture id. */
   glFontRow rows[ MAX_ROWS ]; /**< Stores font row information. */
} glFontTex;


/**
 * @brief Represents a character in the font.
 */
typedef struct glFontGlyph_s {
   uint32_t codepoint; /**< Real character. */
   GLfloat adv_x; /**< X advancement. */
   GLfloat adv_y; /**< Y advancement. */
   /* Offsets are stored in the VBO and thus not an issue. */
   glFontTex *tex; /**< Might be on different texture. */
   GLushort vbo_id; /**< VBO index to use. */
   int next; /**< Stored as a linked list. */
} glFontGlyph;


/**
 * @brief Stores a font character.
 */
typedef struct font_char_s {
   GLubyte *data; /**< Data of the character. */
   int w; /**< Width. */
   int h; /**< Height. */
   int off_x; /**< X offset when rendering. */
   int off_y; /**< Y offset when rendering. */
   GLfloat adv_x; /**< X advancement. */
   GLfloat adv_y; /**< Y advancement. */
   int tx; /**< Texture x position. */
   int ty; /**< Texture y position. */
   int tw; /**< Texture width. */
   int th; /**< Texture height. */
} font_char_t;


/**
 * @brief Font structure.
 */
typedef struct glFontStash_s {
   int h; /**< Font height. */
   int tw; /**< Width of textures. */
   int th; /**< Height of textures. */
   glFontTex *tex; /**< Textures. */
   gl_vbo *vbo_tex; /**< VBO associated to texture coordinates. */
   gl_vbo *vbo_vert; /**< VBO associated to vertex coordinates. */
   GLfloat *vbo_tex_data; /**< Copy of texture coordinate data. */
   GLshort *vbo_vert_data; /**< Copy of vertex coordinate data. */
   int nvbo; /**< Amount of vbo data. */
   int mvbo; /**< Amount of vbo memory. */
   glFontGlyph *glyphs; /**< Unicode glyphs. */
   int lut[HASH_LUT_SIZE]; /**< Look up table. */

   /* Freetype stuff. */
   char *fontname; /**< Font name. */
   FT_Face face; /**< Face structure. */
   FT_Library library; /**< Library. */
   FT_Byte *fontdata; /**< Font data buffer. */
} glFontStash;

/**
 * Available fonts stashes.
 */
static glFontStash *avail_fonts = NULL;  /**< These are pointed to by the font struct exposed in font.h. */

/* default font */
glFont gl_defFont; /**< Default font. */
glFont gl_smallFont; /**< Small font. */
glFont gl_defFontMono; /**< Default mono font. */


/* Last used colour. */
static const glColour *font_lastCol    = NULL; /**< Stores last colour used (activated by '\a'). */
static int font_restoreLast      = 0; /**< Restore last colour. */


/*
 * prototypes
 */
static size_t font_limitSize( glFontStash *ft_font, int *width,
      const char *text, const int max );
static const glColour* gl_fontGetColour( uint32_t ch );
/* Get unicode glyphs from cache. */
static glFontGlyph* gl_fontGetGlyph( glFontStash *stsh, uint32_t ch );
/* Render.
 * TODO this should be changed to be more like font-stash (https://github.com/akrinke/Font-Stash)
 * In particular, instead of writing char by char, they should be batched up by textures and rendered
 * when gl_fontRenderEnd() is called, saving lots of opengl calls.
 */
static void gl_fontRenderStart( const glFontStash *stsh, double x, double y, const glColour *c );
static int gl_fontRenderGlyph( glFontStash *stsh, uint32_t ch, const glColour *c, int state );
static void gl_fontRenderEnd (void);


/**
 * @brief Gets the font stash corresponding to a font.
 */
static glFontStash *gl_fontGetStash( const glFont *font )
{
   return &avail_fonts[ font->id ];
}


/**
 * @brief Adds a font glyph to the texture stash.
 */
static int gl_fontAddGlyphTex( glFontStash *stsh, font_char_t *ch, glFontGlyph *glyph )
{
   int i, j, n;
   GLubyte *data;
   glFontRow *r, *gr;
   glFontTex *tex;
   GLfloat *vbo_tex;
   GLshort *vbo_vert;
   GLfloat tx, ty, txw, tyh;
   GLfloat fw, fh;
   GLshort vx, vy, vw, vh;

   /* Find free row. */
   gr = NULL;
   for (i=0; i<array_size( stsh->tex ); i++) {
      for (j=0; j<MAX_ROWS; j++) {
         r = &stsh->tex->rows[j];
         /* Fits in current row, so use that. */
         if ((r->h == ch->h) && (r->x+ch->w < stsh->tw)) {
            tex = &stsh->tex[i];
            gr = r;
            break;
         }
         /* If not empty row, skip. */
         if (r->h != 0)
            continue;
         /* See if height fits. */
         if ((j==0) || (stsh->tex->rows[j-1].y+stsh->tex->rows[j-1].h+ch->h < stsh->th)) {
            r->h = ch->h;
            if (j>0)
               r->y = stsh->tex->rows[j-1].y + stsh->tex->rows[j-1].h;
            tex = &stsh->tex[i];
            gr = r;
            break;
         }
      }
   }

   /* Allocate new texture. */
   if (gr == NULL) {
      tex = &array_grow( &stsh->tex );
      memset( stsh->tex, 0, sizeof(glFontTex) );

      glGenTextures( 1, &tex->id );
      glBindTexture( GL_TEXTURE_2D, tex->id );

      /* Shouldn't ever scale - we'll generate appropriate size font. */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      /* Clamp texture .*/
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      /* Initialize size. */
      data = calloc( 2*stsh->tw*stsh->th, sizeof(GLubyte) );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, stsh->tw, stsh->th, 0,
            GL_ALPHA, GL_UNSIGNED_BYTE, data );
      free(data);

      /* Check for errors. */
      gl_checkErr();

      gr = &tex->rows[0];
      gr->h = ch->h;
   }

   /* Upload data. */
   glBindTexture( GL_TEXTURE_2D, tex->id );
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glTexSubImage2D( GL_TEXTURE_2D, 0, gr->x, gr->y, ch->w, ch->h,
         GL_ALPHA, GL_UNSIGNED_BYTE, ch->data );

   /* Check for error. */
   gl_checkErr();

   /* Update VBOs. */
   stsh->nvbo++;
   if (stsh->nvbo > stsh->mvbo) {
      stsh->mvbo *= 2;
      stsh->vbo_tex_data  = realloc( stsh->vbo_tex_data,  8*stsh->mvbo*sizeof(GLfloat) );
      stsh->vbo_vert_data = realloc( stsh->vbo_vert_data, 8*stsh->mvbo*sizeof(GLshort) );
   }
   n = 8*stsh->nvbo;
   vbo_tex  = &stsh->vbo_tex_data[n-8];
   vbo_vert = &stsh->vbo_vert_data[n-8];
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
   fw  = (GLfloat) stsh->tw;
   fh  = (GLfloat) stsh->th;
   tx  = (GLfloat) gr->x / fw;
   ty  = (GLfloat) gr->y / fh;
   txw = (GLfloat) (gr->x + ch->w) / fw;
   tyh = (GLfloat) (gr->y + ch->h) / fh;
   vx  = ch->off_x;
   vy  = ch->off_y - ch->h;
   vw  = ch->w;
   vh  = ch->h;
   /* Texture coords. */
   vbo_tex[ 0 ] = tx;  /* Top left. */
   vbo_tex[ 1 ] = ty;
   vbo_tex[ 2 ] = txw; /* Top right. */
   vbo_tex[ 3 ] = ty;
   vbo_tex[ 4 ] = txw; /* Bottom right. */
   vbo_tex[ 5 ] = tyh;
   vbo_tex[ 6 ] = tx;  /* Bottom left. */
   vbo_tex[ 7 ] = tyh;
   /* Vertex coords. */
   vbo_vert[ 0 ] = vx;    /* Top left. */
   vbo_vert[ 1 ] = vy+vh;
   vbo_vert[ 2 ] = vx+vw; /* Top right. */
   vbo_vert[ 3 ] = vy+vh;
   vbo_vert[ 4 ] = vx+vw; /* Bottom right. */
   vbo_vert[ 5 ] = vy;
   vbo_vert[ 6 ] = vx;    /* Bottom left. */
   vbo_vert[ 7 ] = vy;
   /* Update vbos. */
   gl_vboData( stsh->vbo_tex,  sizeof(GLfloat)*n,  stsh->vbo_tex_data );
   gl_vboData( stsh->vbo_vert, sizeof(GLshort)*n, stsh->vbo_vert_data );

   /* Add space for the new character. */
   gr->x += ch->w;

   /* Save glyph data. */
   glyph->vbo_id = (n-8)/2;
   glyph->tex = tex;

   /* Since the VBOs have possibly changed, we have to reset the data. */
   gl_vboActivateOffset( stsh->vbo_tex,  GL_TEXTURE_COORD_ARRAY, 0, 2, GL_FLOAT, 0 );
   gl_vboActivateOffset( stsh->vbo_vert, GL_VERTEX_ARRAY, 0, 2, GL_SHORT, 0 );

   return 0;
}


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
      if (text[i] != '\a')
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
static size_t font_limitSize( glFontStash *ft_font, int *width,
      const char *text, const int max )
{
   int n;
   size_t i;
   uint32_t ch;
   int adv_x;

   /* Avoid segfaults. */
   if ((text == NULL) || (text[0]=='\0'))
      return 0;

   /* limit size */
   i = 0;
   n = 0;
   while ((ch = u8_nextchar( text, &i ))) {
      /* Ignore escape sequence. */
      if (ch == '\a') {
         if (text[i] != '\0')
            i++;
         continue;
      }

      /* Count length. */
      glFontGlyph *glyph = gl_fontGetGlyph( ft_font, ch );
      adv_x = glyph->adv_x;

      /* See if enough room. */
      n += adv_x;
      if (n > max) {
         u8_dec( text, &i );
         n -= adv_x; /* actual size */
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
   int lastspace;
   size_t i;
   uint32_t ch;
   GLfloat n;

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* limit size per line */
   lastspace = 0; /* last ' ' or '\n' in the text */
   n = 0.; /* current width */
   i = 0; /* current position */
   while ((text[i] != '\n') && (text[i] != '\0')) {
      /* Characters we should ignore. */
      if (text[i] == '\t') {
         i++;
         continue;
      }

      /* Ignore escape sequence. */
      if (text[i] == '\a') {
         if (text[i+1] != '\0')
            i += 2;
         else
            i += 1;
         continue;
      }

      /* Save last space. */
      if (text[i] == ' ')
         lastspace = i;

      ch = u8_nextchar( text, &i );
      /* Unicode. */
      glFontGlyph *glyph = gl_fontGetGlyph( stsh, ch );
      if (glyph!=NULL)
         n += glyph->adv_x;

      /* Check if out of bounds. */
      if (n > (GLfloat)width) {
         if (lastspace > 0)
            return lastspace;
         else {
            u8_dec( text, &i );
            return i;
         }
      }
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
   int s;
   size_t i;
   uint32_t ch;

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* Render it. */
   s = 0;
   i = 0;
   gl_fontRenderStart(stsh, x, y, c);
   while ((ch = u8_nextchar( text, &i )))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
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
   int s;
   size_t ret, i;
   uint32_t ch;

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* Limit size. */
   ret = font_limitSize( stsh, NULL, text, max );

   /* Render it. */
   s = 0;
   gl_fontRenderStart(stsh, x, y, c);
   i = 0;
   while ((ch = u8_nextchar( text, &i )) && (i <= ret))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
   gl_fontRenderEnd();

   return ret;
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
   int n, s;
   size_t ret, i;
   uint32_t ch;

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* limit size */
   ret = font_limitSize( stsh, &n, text, width );
   x += (double)(width - n)/2.;

   /* Render it. */
   s = 0;
   gl_fontRenderStart(stsh, x, y, c);
   i = 0;
   while ((ch = u8_nextchar( text, &i )) && (i <= ret))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
   gl_fontRenderEnd();

   return ret;
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
   int p, s;
   double x,y;
   size_t i, ret;
   uint32_t ch;

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   x = bx;
   y = by + height - (double)stsh->h; /* y is top left corner */

   /* Clears restoration. */
   gl_printRestoreClear();

   i = 0;
   s = 0;
   p = 0; /* where we last drew up to */
   while (y - by > -1e-5) {
      ret = p + gl_printWidthForText( ft_font, &text[p], width );

      /* Must restore stuff. */
      gl_printRestoreLast();

      /* Render it. */
      gl_fontRenderStart(stsh, x, y, c);
      for (i=p; i<ret; ) {
         ch = u8_nextchar( text, &i);
         s = gl_fontRenderGlyph( stsh, ch, c, s );
      }
      gl_fontRenderEnd();

      if (ch == '\0')
         break;
      p = i;
      if ((text[p] == '\n') || (text[p] == ' '))
         p++; /* Skip "empty char". */
      y -= 1.5*(double)stsh->h; /* move position down */
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
   GLfloat n;
   size_t i;
   uint32_t ch;

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   n = 0.;
   i = 0;
   while ((ch = u8_nextchar( text, &i ))) {
      /* Ignore escape sequence. */
      if (ch == '\a') {
         if (text[i] != '\0')
            i++;

         continue;
      }

      /* Increment width. */
      glFontGlyph *glyph = gl_fontGetGlyph( stsh, ch );
      n += glyph->adv_x;
   }

   return (int)n;
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
static int font_makeChar( glFontStash *stsh, font_char_t *c, uint32_t ch )
{
   FT_Bitmap bitmap;
   FT_GlyphSlot slot;
   FT_UInt glyph_index;
   int w,h;

   slot = stsh->face->glyph; /* Small shortcut. */

   /* Get glyph index. */
   glyph_index = FT_Get_Char_Index( stsh->face, ch );

   /* Load the glyph. */
   if (FT_Load_Glyph( stsh->face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL)) {
      WARN(_("FT_Load_Glyph failed."));
      return -1;
   }

   bitmap = slot->bitmap; /* to simplify */
   if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
      WARN(_("Font '%s' not using FT_PIXEL_MODE_GRAY!"), stsh->fontname);

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
   c->adv_x = (GLfloat)slot->advance.x / 64.;
   c->adv_y = (GLfloat)slot->advance.y / 64.;

   return 0;
}


/**
 * @brief Starts the rendering engine.
 */
static void gl_fontRenderStart( const glFontStash* font, double x, double y, const glColour *c )
{
   double a;

   /* Enable textures. */
   glEnable(GL_TEXTURE_2D);

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
static const glColour* gl_fontGetColour( uint32_t ch )
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
 * @brief Hash function for integers.
 *
 * Taken from http://burtleburtle.net/bob/hash/integer.html (Thomas Wang).
 * Meant to only use the low bits, not the high bits.
 */
static uint32_t hashint( uint32_t a )
{
   a += ~(a<<15);
   a ^=  (a>>10);
   a +=  (a<<3);
   a ^=  (a>>6);
   a += ~(a<<11);
   a ^=  (a>>16);
   return a;
}

/**
 * @brief Gets or caches a glyph to render.
 */
static glFontGlyph* gl_fontGetGlyph( glFontStash *stsh, uint32_t ch )
{
   int i;
   unsigned int h;

   /* Use hash table and linked lists to find the glyph. */
   h = hashint(ch) & (HASH_LUT_SIZE-1);
   i = stsh->lut[h];
   while (i != -1) {
      if (stsh->glyphs[i].codepoint == ch)
         return &stsh->glyphs[i];
      i = stsh->glyphs[i].next;
   }

   /* Glyph not found, have to generate. */
   glFontGlyph *glyph;
   font_char_t ft_char;
   int idx;

   /* Load data from freetype. */
   font_makeChar( stsh, &ft_char, ch );

   /* Create new character. */
   glyph = &array_grow( &stsh->glyphs );
   glyph->codepoint = ch;
   glyph->tex   = NULL;
   glyph->adv_x = ft_char.adv_x;
   glyph->adv_y = ft_char.adv_y;
   glyph->next  = -1;
   idx = glyph - stsh->glyphs;

   /* Insert in linked list. */
   i = stsh->lut[h];
   if (i == -1) {
      stsh->lut[h] = idx;
   }
   else {
      while (i != -1) {
         if (stsh->glyphs[i].next == -1) {
            stsh->glyphs[i].next = idx;
            break;
         }
         i = stsh->glyphs[i].next;
      }
   }

   /* Find empty texture and render char. */
   gl_fontAddGlyphTex( stsh, &ft_char, glyph );

   return glyph;
}


/**
 * @brief Renders a character.
 */
static int gl_fontRenderGlyph( glFontStash* stsh, uint32_t ch, const glColour *c, int state )
{
   GLushort ind[6];
   double a;
   const glColour *col;
   int vbo_id;

   /* Handle escape sequences. */
   if (ch == '\a') {/* Start sequence. */
      return 1;
   }
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

   /* Unicode goes here.
    * First try to find the glyph. */
   glFontGlyph *glyph;
   glyph = gl_fontGetGlyph( stsh, ch );
   if (glyph == NULL) {
      WARN(_("Unable to find glyph '%d'!"), ch );
      return -1;
   }

   /* Activate texture. */
   glBindTexture(GL_TEXTURE_2D, glyph->tex->id);

   /* VBO indices. */
   vbo_id = glyph->vbo_id;
   ind[0] = vbo_id + 0;
   ind[1] = vbo_id + 1;
   ind[2] = vbo_id + 3;
   ind[3] = vbo_id + 1;
   ind[4] = vbo_id + 3;
   ind[5] = vbo_id + 2;

   /* Draw the element. */
   glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, ind );

   /* Translate matrix. */
   gl_matrixTranslate( glyph->adv_x, glyph->adv_y );

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
 * @brief Tries to find a system font.
 */
static char *gl_fontFind( const char *fname )
{
#ifdef USE_FONTCONFIG
   FcConfig* config;
   FcPattern *pat, *font;
   FcResult result;
   FcChar8* file;
   char *fontFile;
   
   config = FcInitLoadConfigAndFonts();
   pat = FcNameParse( (const FcChar8*)fname );
   FcConfigSubstitute( config, pat, FcMatchPattern );
   FcDefaultSubstitute(pat);
   font = FcFontMatch(config, pat, &result);
   if (font) {
      file = NULL;
      if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
         fontFile = strdup( (char*)file );
         FcPatternDestroy(pat);
         return fontFile;
      }
   }
   FcPatternDestroy(pat);
#endif
   return NULL;
}


/**
 * @brief Initializes a font.
 *
 *    @param font Font to load (NULL defaults to gl_defFont).
 *    @param fname Name of the font (from inside packfile, NULL defaults to default font).
 *    @param h Height of the font to generate.
 *    @return 0 on success.
 */
int gl_fontInit( glFont* font, const char *fname, const char *fallback, const unsigned int h )
{
   FT_Library library;
   FT_Face face;
   size_t bufsize;
   FT_Byte* buf;
   int i;
   glFontStash *stsh;
   char *used_font;

   /* See if we should override fonts. */
   used_font = NULL;
   if ((strcmp( fallback, FONT_DEFAULT_PATH )==0) &&
      (conf.font_name_default!=NULL)) {
      used_font = strdup( conf.font_name_default );
   }
   else if ((strcmp( fallback, FONT_MONOSPACE_PATH )==0) &&
      (conf.font_name_monospace!=NULL)) {
      used_font = strdup( conf.font_name_monospace );
   }
   if (used_font) {
      buf = (FT_Byte*)nfile_readFile( &bufsize, used_font );
      if (buf==NULL) {
         free(used_font);
         used_font = NULL;
      }
   }

   /* Try to use system font. */
   if (used_font==NULL) {
      used_font = gl_fontFind( fname );
      if (used_font) {
         buf = (FT_Byte*)nfile_readFile( &bufsize, used_font );
         if (buf==NULL) {
            free(used_font);
            used_font = NULL;
         }
      }
   }

   /* Fallback to packaged font. */
   if (used_font==NULL) {
      buf = ndata_read( (fallback!=NULL) ? fallback : FONT_DEFAULT_PATH, &bufsize );
      if (buf == NULL) {
         WARN(_("Unable to read font: %s"), (fallback!=NULL) ? fallback : FONT_DEFAULT_PATH);
         return -1;
      }
      used_font = strdup( fallback );
   }

   /* Get default font if not set. */
   if (font == NULL) {
      font = &gl_defFont;
      DEBUG( _("Using default font '%s'"), used_font );
   }

   /* Get font stash. */
   if (avail_fonts==NULL)
      avail_fonts = array_create( glFontStash );
   stsh = &array_grow( &avail_fonts );
   memset( stsh, 0, sizeof(glFontStash) );
   font->id = stsh - avail_fonts;
   font->h = (int)floor((double)h);

   /* Default sizes. */
   stsh->tw = DEFAULT_TEXTURE_SIZE;
   stsh->th = DEFAULT_TEXTURE_SIZE;
   stsh->h = font->h;

   /* Create a FreeType font library. */
   if (FT_Init_FreeType(&library)) {
      WARN(_("FT_Init_FreeType failed with font %s."),
            (used_font!=NULL) ? used_font : FONT_DEFAULT_PATH );
      return -1;
   }

   /* Object which freetype uses to store font info. */
   if (FT_New_Memory_Face( library, buf, bufsize, 0, &face )) {
      WARN(_("FT_New_Face failed loading library from %s"),
            (used_font!=NULL) ? used_font : FONT_DEFAULT_PATH );
      return -1;
   }

   /* Try to resize. */
   if (FT_IS_SCALABLE(face)) {
      if (FT_Set_Char_Size( face,
               0, /* Same as width. */
               h << 6, /* In 1/64th of a pixel. */
               96, /* Create at 96 DPI */
               96)) /* Create at 96 DPI */
         WARN(_("FT_Set_Char_Size failed."));
   }
   else
      WARN(_("Font isn't resizable!"));

   /* Select the character map. */
   if (FT_Select_Charmap( face, FT_ENCODING_UNICODE ))
      WARN(_("FT_Select_Charmap failed to change character mapping."));

   /* Initialize the unicode support. */
   for (i=0; i<HASH_LUT_SIZE; i++)
      stsh->lut[i] = -1;
   stsh->glyphs = array_create( glFontGlyph );
   stsh->tex    = array_create( glFontTex );

   /* Set up font stuff for next glyphs. */
   stsh->fontname = strdup( (used_font!=NULL) ? used_font : FONT_DEFAULT_PATH );
   stsh->face     = face;
   stsh->library  = library;
   stsh->fontdata = buf;

   /* Set up VBOs. */
   stsh->mvbo = 256;
   stsh->vbo_tex_data  = calloc( 8*stsh->mvbo, sizeof(GLfloat) );
   stsh->vbo_vert_data = calloc( 8*stsh->mvbo, sizeof(GLshort) );
   stsh->vbo_tex  = gl_vboCreateStatic( sizeof(GLfloat)*8*stsh->mvbo,  stsh->vbo_tex_data );
   stsh->vbo_vert = gl_vboCreateStatic( sizeof(GLshort)*8*stsh->mvbo, stsh->vbo_vert_data );

   /* Initializes ASCII. */
   for (i=0; i<128; i++)
      gl_fontGetGlyph( stsh, i );

#if 0
   /* We can now free the face and library */
   FT_Done_Face(face);
   FT_Done_FreeType(library);

   /* Free read buffer. */
   free(buf);
#endif

   return 0;
}

/**
 * @brief Frees a loaded font.
 *
 *    @param font Font to free.
 */
void gl_freeFont( glFont* font )
{
   int i;

   if (font == NULL)
      font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( font );

   free(stsh->fontname);
   FT_Done_Face(stsh->face);
   //FT_Done_FreeType(stsh->library);
   free(stsh->fontdata);

   for (i=0; i<array_size(stsh->tex); i++)
      glDeleteTextures( 1, &stsh->tex->id );
   array_free( stsh->tex );
   stsh->tex = NULL;

   if (stsh->glyphs != NULL)
      array_free( stsh->glyphs );
   stsh->glyphs = NULL;
   if (stsh->vbo_tex != NULL)
      gl_vboDestroy(stsh->vbo_tex);
   stsh->vbo_tex = NULL;
   if (stsh->vbo_vert != NULL)
      gl_vboDestroy(stsh->vbo_vert);
   stsh->vbo_vert = NULL;
}


