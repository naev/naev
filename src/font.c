/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file font.c
 *
 * @brief OpenGL font rendering routines.
 *
 * We use distance fields [1] to render high quality fonts with the help of
 * some shaders. Characters are generated on demand using a texture atlas.
 *
 * [1]: https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
 */
/** @cond */
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_MODULE_H
#include <wctype.h>
#include "linebreak.h"
#include "linebreakdef.h"

#include "naev.h"
/** @endcond */

#include "font.h"

#include "array.h"
#include "conf.h"
#include "distance_field.h"
#include "log.h"
#include "ndata.h"
#include "nfile.h"
#include "utf8.h"
#include "ntracing.h"

#define MAX_EFFECT_RADIUS 4 /**< Maximum pixel distance from glyph to outline/shadow/etc. */
#define FONT_DISTANCE_FIELD_SIZE   55 /**< Size to render the fonts at. */
#define HASH_LUT_SIZE 512 /**< Size of glyph look up table. */
#define DEFAULT_TEXTURE_SIZE 1024 /**< Default size of texture caches for glyphs. */
#define MAX_ROWS 64 /**< Max number of rows per texture cache. */

/**
 * OpenGL rendering stuff. Since we can't actually render with multiple threads
 * we can be lazy and use global variables.
 */
static mat4 font_projection_mat; /**< Projection matrix. */
static FT_Library font_library = NULL; /**< Global FreeType library. */
static FT_UInt    prev_glyph_index; /**< Index of last character drawn (for kerning). */
static int        prev_glyph_ft_index; /**< HACK: Index into which stsh->ft[_].face? */

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
   GLfloat adv_x; /**< X advancement on the screen. */
   GLfloat m; /**< Number of distance units corresponding to 1 "pixel". */
   int ft_index; /**< HACK: Index into the array of fallback fonts. */
   int tex_index; /**< Might be on different texture. */
   GLushort vbo_id; /**< VBO index to use. */
   int next; /**< Stored as a linked list. */
} glFontGlyph;

/**
 * @brief Stores a font character.
 */
typedef struct font_char_s {
   GLubyte *data; /**< Data of the character. */
   GLfloat *dataf; /**< Float data of the character. */
   int w; /**< Width. */
   int h; /**< Height. */
   int ft_index; /**< HACK: Index into the array of fallback fonts. */
   int off_x; /**< X offset when rendering. */
   int off_y; /**< Y offset when rendering. */
   GLfloat adv_x; /**< X advancement on the screen. */
   GLfloat m; /**< Number of distance units corresponding to 1 "pixel". */
   int tx; /**< Texture x position. */
   int ty; /**< Texture y position. */
   int tw; /**< Texture width. */
   int th; /**< Texture height. */
} font_char_t;

/**
 * @brief Stores a font file. May be referenced by multiple glFonts for size or fallback reasons.
 */
typedef struct glFontFile_s {
   char *name; /**< Font file name. */
   int refcount; /**< Reference counting. */
   FT_Byte *data; /**< Font data buffer. */
   size_t datasize; /**< Font data size. */
} glFontFile;

/**
 * @brief Freetype Font structure.
 */
typedef struct glFontStashFreetype_s {
   glFontFile *file; /**< Font file. */
   FT_Face face; /**< Face structure. */
} glFontStashFreetype;

/**
 * @brief Font structure.
 */
typedef struct glFontStash_s {
   /* Core values (determine font). */
   char *fname; /**< Font list name. */

   /* Generated values. */
   GLint magfilter; /**< Magnification filter. */
   GLint minfilter; /**< Minification filter. */
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
   glFontStashFreetype *ft;

   int refcount; /**< Reference counting. */
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
static const glColour *font_lastCol    = NULL; /**< Stores last colour used (activated by FONT_COLOUR_CODE). */
static int font_restoreLast      = 0; /**< Restore last colour. */

/*
 * prototypes
 */
static int gl_fontstashAddFallback( glFontStash* stsh, const char *fname, unsigned int h );
static size_t font_limitSize( glFontStash *stsh, int *width, const char *text, const int max );
static const glColour* gl_fontGetColour( uint32_t ch );
static uint32_t font_nextChar( const char *s, size_t *i );
/* Get unicode glyphs from cache. */
static glFontGlyph* gl_fontGetGlyph( glFontStash *stsh, uint32_t ch );
/* Render.
 * TODO this should be changed to be more like font-stash (https://github.com/akrinke/Font-Stash)
 * In particular, instead of writing char by char, they should be batched up by textures and rendered
 * when gl_fontRenderEnd() is called, saving lots of opengl calls.
 */
static void gl_fontRenderStart( const glFontStash *stsh, double x, double y, const glColour *c, double outlineR );
static void gl_fontRenderStartH( const glFontStash* stsh, const mat4 *H, const glColour *c, double outlineR );
static int gl_fontRenderGlyph( glFontStash *stsh, uint32_t ch, const glColour *c, int state );
static void gl_fontRenderEnd (void);
/* Fussy layout concerns. */
static void gl_fontKernStart (void);
static int gl_fontKernGlyph( glFontStash* stsh, uint32_t ch, glFontGlyph* glyph );
static void gl_fontstashftDestroy( glFontStashFreetype *ft );

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
   int n;
   glFontRow *gr, *lr;
   glFontTex *tex;
   GLfloat *vbo_tex;
   GLshort *vbo_vert;
   GLfloat tx, ty, txw, tyh;
   GLfloat fw, fh;
   GLshort vx, vy, vw, vh;
   double mx, my;

   /* Find free row. */
   tex = NULL;
   gr = NULL;
   for (int i=0; i<array_size( stsh->tex ); i++) {
      for (int j=0; j<MAX_ROWS; j++) {
         glFontRow *r = &stsh->tex->rows[j];
         /* Not empty row and doesn't fit. */
         if ((r->h != 0) && (r->h != ch->h))
            continue;
         if (r->h == ch->h) {
            /* Fits in current row, so use that. */
            if (r->x + ch->w <= stsh->tw) {
               tex = &stsh->tex[i];
               gr = r;
               break;
            }
            else
               continue; /* Didn't fit so continue looking. */
         }
         if (r->h != 0)
            continue;
         /* First row. */
         if (j==0) {
            assert( ch->h <= stsh->th ); /* Would be ridiculously large character... */
            r->h = ch->h;
            tex = &stsh->tex[i];
            gr = r;
            break;
         }
         /* See if height fits to create a new row. */
         lr = &stsh->tex->rows[j-1];
         if (lr->y + lr->h + ch->h <= stsh->th) {
            r->h = ch->h;
            r->y = lr->y + lr->h;
            tex = &stsh->tex[i];
            gr = r;
         }
         break; /* Have to break here because either we added a new row or texture is full. */
      }
      if (gr != NULL)
         break;
   }

   /* Didn't fit so allocate new texture. */
   if (gr == NULL) {
      tex = &array_grow( &stsh->tex );
      memset( tex, 0, sizeof(glFontTex) );

      /* Create new texture. */
      glGenTextures( 1, &tex->id );
      glBindTexture( GL_TEXTURE_2D, tex->id );

      /* Set a sane default minification and magnification filter. */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, stsh->magfilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, stsh->minfilter);

      /* Clamp texture .*/
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      /* Initialize size. */
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, stsh->tw, stsh->th, 0,
            GL_RED, GL_UNSIGNED_BYTE, NULL );

      /* Check for errors. */
      gl_checkErr();

      /* Create a new entry at the beginning of the first row with our target height. */
      gr = &tex->rows[0];
      gr->h = ch->h;
   }

   /* Upload data. */
   glBindTexture( GL_TEXTURE_2D, tex->id );
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   if (ch->dataf != NULL)
      glTexSubImage2D( GL_TEXTURE_2D, 0, gr->x, gr->y, ch->w, ch->h,
            GL_RED, GL_FLOAT, ch->dataf );
   else
      glTexSubImage2D( GL_TEXTURE_2D, 0, gr->x, gr->y, ch->w, ch->h,
            GL_RED, GL_UNSIGNED_BYTE, ch->data );
   glPixelStorei(GL_UNPACK_ALIGNMENT,4);

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
   mx  = 1. / fw;
   my  = 1. / fh;
   tx  = (GLfloat) (gr->x+1.) / fw;
   ty  = (GLfloat) (gr->y+1.) / fh;
   txw = (GLfloat) (gr->x + ch->w-1.) / fw;
   tyh = (GLfloat) (gr->y + ch->h-1.) / fh;
   vx  = ch->off_x + mx;
   vy  = ch->off_y - ch->h + mx;
   vw  = ch->w - mx;
   vh  = ch->h - my;
   /* Texture coords. */
   vbo_tex[ 0 ] = tx;  /* Top left. */
   vbo_tex[ 1 ] = ty;
   vbo_tex[ 2 ] = txw; /* Top right. */
   vbo_tex[ 3 ] = ty;
   vbo_tex[ 4 ] = tx;  /* Bottom left. */
   vbo_tex[ 5 ] = tyh;
   vbo_tex[ 6 ] = txw; /* Bottom right. */
   vbo_tex[ 7 ] = tyh;
   /* Vertex coords. */
   vbo_vert[ 0 ] = vx;    /* Top left. */
   vbo_vert[ 1 ] = vy+vh;
   vbo_vert[ 2 ] = vx+vw; /* Top right. */
   vbo_vert[ 3 ] = vy+vh;
   vbo_vert[ 4 ] = vx;    /* Bottom left. */
   vbo_vert[ 5 ] = vy;
   vbo_vert[ 6 ] = vx+vw; /* Bottom right. */
   vbo_vert[ 7 ] = vy;
   /* Update vbos. */
   gl_vboData( stsh->vbo_tex,  sizeof(GLfloat)*n,  stsh->vbo_tex_data );
   gl_vboData( stsh->vbo_vert, sizeof(GLshort)*n, stsh->vbo_vert_data );

   /* Add space for the new character. */
   gr->x += ch->w;

   /* Save glyph data. */
   glyph->vbo_id = (n-8)/2;
   glyph->tex_index = tex - stsh->tex;

   /* Since the VBOs have possibly changed, we have to reset the data. */
   gl_vboActivateAttribOffset( stsh->vbo_vert, shaders.font.vertex, 0, 2, GL_SHORT, 0 );
   gl_vboActivateAttribOffset( stsh->vbo_tex, shaders.font.tex_coord, 0, 2, GL_FLOAT, 0 );

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
   const glColour *col = restore->col; /* Use whatever is there. */
   for (int i=0; (text[i]!='\0') && (i<=max); i++) {
      /* Only want escape sequences. */
      if (text[i] != FONT_COLOUR_CODE)
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
 *    @param stsh Font stash to calculate width with.
 *    @param width Actual width it takes up.
 *    @param text Text to parse.
 *    @param max Max to look for.
 *    @return Number of characters that fit.
 */
static size_t font_limitSize( glFontStash *stsh, int *width, const char *text, const int max )
{
   GLfloat n;
   size_t i;
   uint32_t ch;

   /* Avoid segfaults. */
   if ((text == NULL) || (text[0]=='\0'))
      return 0;

   /* limit size */
   gl_fontKernStart();
   i = 0;
   n = 0.;
   while ((ch = u8_nextchar( text, &i ))) {
      int adv_x;

      /* Ignore escape sequence. */
      if (ch == FONT_COLOUR_CODE) {
         if (text[i] != '\0')
            i++;
         continue;
      }

      /* Count length. */
      glFontGlyph *glyph = gl_fontGetGlyph( stsh, ch );
      adv_x = gl_fontKernGlyph( stsh, ch, glyph ) + glyph->adv_x;

      /* See if enough room. */
      n += adv_x;
      if ((int)round(n) > max) {
         u8_dec( text, &i );
         n -= adv_x; /* actual size */
         break;
      }
   }

   if (width != NULL)
      (*width) = (int)round(n);
   return i;
}

/**
 * @brief Initialize an iterator object for breaking text into lines.
 *
 *    @param iter New glPrintLineIterator.
 *    @param text Text to split.
 *    @param ft_font Font to use.
 *    @param width Maximum width of a line.
 */
void gl_printLineIteratorInit( glPrintLineIterator *iter, const glFont *ft_font, const char *text, int width )
{
   memset( iter, 0, sizeof(glPrintLineIterator) );
   iter->text = text;
   iter->ft_font = (ft_font==NULL ? &gl_defFont : ft_font);
   iter->width = width;
}

typedef struct _linepos_t_ {
   size_t i;    /**< Byte index of the current char */
   uint32_t ch; /**< Current code point */
   GLfloat w;   /**< Current width (line start to left side of current character) */
} _linepos_t;

/**
 * @brief Updates \p iter with the next line's information.
 *
 *    @param iter An iterator returned by \ref gl_printLineIteratorInit.
 *    @return nonzero if there's a line.
 */
int gl_printLineIteratorNext( glPrintLineIterator* iter )
{
   glFontStash *stsh = gl_fontGetStash( iter->ft_font );
   int brk, can_break, can_fit, any_char_fit = 0, any_word_fit;
   size_t char_end = iter->l_next;
   struct LineBreakContext lbc;

   if (iter->dead)
      return 0;

   /* limit size per line */
   gl_fontKernStart();

   /* Initialize line break stuff. */
   iter->l_begin = iter->l_next;
   iter->l_end = iter->l_begin;
   _linepos_t pos = { .i = char_end, .w = 0. };
   pos.ch = font_nextChar( iter->text, &char_end );
   lb_init_break_context( &lbc, pos.ch, gettext_getLanguage() );

   while (pos.ch != '\0') {
      glFontGlyph *glyph = gl_fontGetGlyph( stsh, pos.ch );
      GLfloat glyph_w = glyph==NULL ? 0 : gl_fontKernGlyph( stsh, pos.ch, glyph ) + glyph->adv_x;
      _linepos_t nextpos = { .i = char_end, .w = pos.w + glyph_w };
      nextpos.ch = font_nextChar( iter->text, &char_end );
      brk = lb_process_next_char( &lbc, nextpos.ch );
      can_break = (brk == LINEBREAK_ALLOWBREAK && !iter->no_soft_breaks) || brk == LINEBREAK_MUSTBREAK;
      can_fit = (iter->width >= (int)round(nextpos.w));
      any_word_fit = (iter->l_end != iter->l_begin);
      /* Emergency situations: */
      can_break |= !can_fit && !any_word_fit;
      can_fit |= !any_char_fit;

      if (can_break && iswspace( pos.ch )) {
         iter->l_width = (int)round(pos.w);
         /* IMPORTANT: when eating a space, we can't backtrack to a previous position, because there might be a skipped font markup sequence in between. */
         iter->l_end = iter->l_next = nextpos.i;
         u8_dec( iter->text, &iter->l_end );
      }
      else if (can_break && can_fit) {
         iter->l_width = (int)round(nextpos.w);
         iter->l_end = iter->l_next = nextpos.i;
      }
      else if (!can_fit && !any_word_fit) {
         iter->l_width = (int)round(pos.w);
         iter->l_end = iter->l_next = pos.i;
      }

      if (!can_fit || brk == LINEBREAK_MUSTBREAK)
         return 1;

      any_char_fit = 1;
      pos = nextpos;
   }

   /* Ran out of text. */
   iter->l_width = (int)round(pos.w);
   iter->l_end = iter->l_next = char_end;
   iter->dead = 1;
   return 1;
}

/** @brief Reads the next utf-8 sequence out of a string, updating an index. Skips font markup directives.
 * @TODO For now, this enforces font.c's inability to handle tabs.
 */
static uint32_t font_nextChar( const char *s, size_t *i )
{
   uint32_t ch = s[*i]; /* To be corrected: the character starting at byte *i. Whether it's zero or not is already correct. */
   while (ch != 0) {
      ch = u8_nextchar(s, i);
      if (ch != FONT_COLOUR_CODE)
         return ch;
      ch = u8_nextchar(s, i); /* Skip the operand and try again. */
      if (ch == FONT_COLOUR_CODE)
         return ch; /* Doubled escape char represents the escape char itself. */
   }
   return 0;
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
 *    @param outlineR Radius in px of outline (-1 for default, 0 for none)
 *    @param text String to display.
 */
void gl_printRaw( const glFont *ft_font, double x, double y, const glColour* c,
      double outlineR, const char *text )
{
   int s;
   size_t i;
   uint32_t ch;
   NTracingZone( _ctx, 1 );

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* Render it. */
   s = 0;
   i = 0;
   gl_fontRenderStart( stsh, x, y, c, outlineR );
   while ((ch = u8_nextchar( text, &i )))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
   gl_fontRenderEnd();

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Prints text on screen using a transformation matrix.
 *
 * Defaults ft_font to gl_defFont if NULL.
 *
 *    @param ft_font Font to use
 *    @param H Transformation matrix to use.
 *    @param c Colour to use (uses white if NULL)
 *    @param outlineR Radius in px of outline (-1 for default, 0 for none)
 *    @param text String to display.
 */
void gl_printRawH( const glFont *ft_font, const mat4 *H,
      const glColour* c, const double outlineR , const char *text )
{
   int s;
   size_t i;
   uint32_t ch;
   NTracingZone( _ctx, 1 );

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* Render it. */
   s = 0;
   i = 0;
   gl_fontRenderStartH( stsh, H, c, outlineR );
   while ((ch = u8_nextchar( text, &i )))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
   gl_fontRenderEnd();

   NTracingZoneEnd( _ctx);
}

/**
 * @brief Wrapper for gl_printRaw for map overlay markers
 *
 * See gl_printRaw params (minus outlineR)
 */
void gl_printMarkerRaw( const glFont *ft_font,
      double x, double y,
      const glColour* c, const char *text)
{
   gl_printRaw( ft_font, x, y, c, 1, text );
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
   if (fmt == NULL)
      return;

   char text[STRMAX_SHORT]; /* holds the string */
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, sizeof(text), fmt, ap);
   va_end(ap);

   gl_printRaw( ft_font, x, y, c, -1., text );


}

/**
 * @brief Behaves like gl_printRaw but stops displaying text after a certain distance.
 *
 *    @param ft_font Font to use.
 *    @param max Maximum length to reach.
 *    @param x X position to display text at.
 *    @param y Y position to display text at.
 *    @param c Colour to use (NULL defaults to white).
 *    @param outlineR Radius in px of outline (-1 for default, 0 for none)
 *    @param text String to display.
 *    @return The number of characters it had to suppress.
 */
int gl_printMaxRaw( const glFont *ft_font, const int max, double x, double y,
      const glColour* c, double outlineR, const char *text)
{
   int s;
   size_t ret, i;
   uint32_t ch;
   NTracingZone( _ctx, 1 );

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* Limit size. */
   ret = font_limitSize( stsh, NULL, text, max );

   /* Render it. */
   s = 0;
   gl_fontRenderStart( stsh, x, y, c, outlineR );
   i = 0;
   while ((ch = u8_nextchar( text, &i )) && (i <= ret))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
   gl_fontRenderEnd();

   NTracingZoneEnd( _ctx );
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
int gl_printMax( const glFont *ft_font, const int max, double x, double y,
      const glColour* c, const char *fmt, ... )
{
   if (fmt == NULL)
      return -1;

   char text[STRMAX_SHORT]; /* holds the string */
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, sizeof(text), fmt, ap);
   va_end(ap);

   return gl_printMaxRaw( ft_font, max, x, y, c, -1., text );
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
 *    @param outlineR Radius in px of outline (-1 for default, 0 for none)
 *    @param text String to display.
 *    @return The number of characters it had to truncate.
 */
int gl_printMidRaw(
      const glFont *ft_font,
      int width,
      double x,
      double y,
      const glColour* c,
      double outlineR,
      const char *text
      )
{
   int n, s;
   size_t ret, i;
   uint32_t ch;
   NTracingZone( _ctx, 1 );

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   /* limit size */
   n = 0;
   ret = font_limitSize( stsh, &n, text, width );
   x += (double)(width - n)/2.;

   /* Render it. */
   s = 0;
   gl_fontRenderStart( stsh, x, y, c, outlineR );
   i = 0;
   while ((ch = u8_nextchar( text, &i )) && (i <= ret))
      s = gl_fontRenderGlyph( stsh, ch, c, s );
   gl_fontRenderEnd();

   NTracingZoneEnd( _ctx );
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
      double x, double y,
      const glColour* c, const char *fmt, ... )
{
   if (fmt == NULL)
      return -1;

   char text[STRMAX_SHORT]; /* holds the string */
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, sizeof(text), fmt, ap);
   va_end(ap);

   return gl_printMidRaw( ft_font, width, x, y, c, -1., text );
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
 *    @param line_height Height of each line to print.
 *    @param c Colour to use (NULL defaults to white).
 *    @param outlineR Radius in px of outline (-1 for default, 0 for none)
 *    @param text String to display.
 *    @return 0 on success.
 * prints text with line breaks included to a maximum width and height preset
 */
int gl_printTextRaw( const glFont *ft_font,
      const int width, const int height,
      double bx, double by, int line_height,
      const glColour* c,
      double outlineR,
      const char *text
    )
{
   glPrintLineIterator iter;
   int s;
   double x,y;
   uint32_t ch;
   NTracingZone( _ctx, 1 );

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   x = bx;
   y = by + height - (double)ft_font->h; /* y is top left corner */

   /* Default to 1.5 line height. */
   if (line_height == 0)
      line_height = 1.5*(double)ft_font->h;

   /* Clears restoration. */
   gl_printRestoreClear();

   s = 0;
   gl_printLineIteratorInit( &iter, ft_font, text, width );
   while ((y - by > -1e-5) && gl_printLineIteratorNext( &iter )) {
      /* Must restore stuff. */
      gl_printRestoreLast();

      /* Render it. */
      gl_fontRenderStart( stsh, x, y, c, outlineR );
      for (size_t i = iter.l_begin; i < iter.l_end; ) {
         ch = u8_nextchar( text, &i );
         s = gl_fontRenderGlyph( stsh, ch, c, s );
      }
      gl_fontRenderEnd();

      y -= line_height; /* move position down */
   }

   NTracingZoneEnd( _ctx );
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
 *    @param line_height Height of each line to print.
 *    @param c Colour to use (NULL defaults to white).
 *    @param fmt Text to display formatted like printf.
 *    @return 0 on success.
 * prints text with line breaks included to a maximum width and height preset
 */
int gl_printText( const glFont *ft_font,
      const int width, const int height,
      double bx, double by, int line_height,
      const glColour* c, const char *fmt, ... )
{
   char text[STRMAX]; /* holds the string */

   if (fmt == NULL)
      return -1;
   else { /* convert the symbols to text */
      va_list ap;
      va_start(ap, fmt);
      vsnprintf(text, sizeof(text), fmt, ap);
      va_end(ap);
   }

   return gl_printTextRaw( ft_font, width, height, bx, by, line_height, c, -1., text );
}

/**
 * @brief Gets the width that it would take to print some text.
 *
 * Does not display text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param text Text to calculate the length of.
 *    @return The length of the text in pixels.
 */
int gl_printWidthRaw( const glFont *ft_font, const char *text )
{
   GLfloat n, nmax;
   size_t i;
   uint32_t ch;
   NTracingZone( _ctx, 1 );

   if (ft_font == NULL)
      ft_font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( ft_font );

   gl_fontKernStart();
   nmax = n = 0.;
   i = 0;
   while ((ch = font_nextChar( text, &i ))) {
      /* Newline. */
      if (ch == '\n') {
         gl_fontKernStart();
         nmax = MAX( nmax, n );
         n = 0.;
         continue;
      }

      /* Increment width. */
      glFontGlyph *glyph = gl_fontGetGlyph( stsh, ch );
      n += gl_fontKernGlyph( stsh, ch, glyph ) + glyph->adv_x;
   }
   nmax = MAX( nmax, n );

   NTracingZoneEnd( _ctx );
   return (int)round(nmax);
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
   if (fmt == NULL)
      return 0;

   char text[STRMAX_SHORT]; /* holds the string */
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, sizeof(text), fmt, ap);
   va_end(ap);

   return gl_printWidthRaw( ft_font, text );
}

/**
 * @brief Gets the height of a non-formatted string.
 *
 * Does not display the text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param width Width to jump to next line once reached.
 *    @param text Text to get the height of.
 *    @return The height of the text.
 */
int gl_printHeightRaw( const glFont *ft_font,
      const int width, const char *text )
{
   glPrintLineIterator iter;
   int line_height;
   double y = 0.;

   /* Check 0 length strings. */
   if (text[0] == '\0')
      return 0;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   line_height = 1.5*(double)ft_font->h;
   gl_printLineIteratorInit( &iter, ft_font, text, width );
   while (gl_printLineIteratorNext( &iter ))
      y += line_height;

   return (int)y - line_height + ft_font->h + 1;
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
   if (fmt == NULL)
      return -1;

   char text[STRMAX_SHORT]; /* holds the string */
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, sizeof(text), fmt, ap);
   va_end(ap);

   return gl_printHeightRaw( ft_font, width, text );
}

/**
 * @brief Gets the number of lines of a non-formatted string.
 *
 * Does not display the text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param width Width to jump to next line once reached.
 *    @param text Text to get the height of.
 *    @return The number of lines of the text.
 */
int gl_printLinesRaw( const glFont *ft_font,
      const int width, const char *text )
{
   glPrintLineIterator iter;
   int n = 0;

   /* Check 0 length strings. */
   if (text[0] == '\0')
      return 0;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   gl_printLineIteratorInit( &iter, ft_font, text, width );
   while (gl_printLineIteratorNext( &iter ))
      n++;

   return n;
}

/**
 * @brief Gets the number of lines of the text if it were printed.
 *
 * Does not display the text on screen.
 *
 *    @param ft_font Font to use (NULL defaults to gl_defFont).
 *    @param width Width to jump to next line once reached.
 *    @param fmt Text to get the height of in printf format.
 *    @return The number of lines of he text.
 */
int gl_printLines( const glFont *ft_font,
      const int width, const char *fmt, ... )
{
   if (fmt == NULL)
      return -1;

   char text[STRMAX_SHORT]; /* holds the string */
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(text, sizeof(text), fmt, ap);
   va_end(ap);

   return gl_printLinesRaw( ft_font, width, text );
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
   int len = array_size(stsh->ft);
   for (int i=0; i<len; i++) {
      FT_UInt glyph_index;
      int w,h, rw,rh, b;
      double vmax;
      FT_Bitmap bitmap;
      FT_GlyphSlot slot;
      glFontStashFreetype *ft = &stsh->ft[i];

      /* Get glyph index. */
      glyph_index = FT_Get_Char_Index( ft->face, ch );
      /* Skip missing unless last font. */
      if (glyph_index==0) {
         if (i<len-1)
            continue;
         else {
            WARN(_("Font '%s' unicode character '%#x' not found in font! Using missing glyph."), ft->file->name, ch);
            ft = &stsh->ft[0]; /* Fallback to first font. */
         }
      }

      /* Load the glyph. */
      if (FT_Load_Glyph( ft->face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL)) {
         WARN(_("FT_Load_Glyph failed."));
         return -1;
      }

      slot = ft->face->glyph; /* Small shortcut. */
      bitmap = slot->bitmap; /* to simplify */
      if (bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
         WARN(_("Font '%s' not using FT_PIXEL_MODE_GRAY!"), ft->file->name);

      w = bitmap.width;
      h = bitmap.rows;

      /* Store data. */
      c->data = NULL;
      c->dataf = NULL;
      if (bitmap.buffer == NULL) {
         /* Space characters tend to have no buffer. */
         b = 0;
         rw = w;
         rh = h;
         c->data = malloc( sizeof(GLubyte) * w*h );
         memset( c->data, 0, sizeof(GLubyte) * w*h );
         vmax = 1.0; /* arbitrary */
      }
      else {
         GLubyte *buffer;
         /* Create a larger image using an extra border and center glyph. */
         b = 1 + ((MAX_EFFECT_RADIUS+1) * FONT_DISTANCE_FIELD_SIZE - 1) / stsh->h;
         rw = w+b*2;
         rh = h+b*2;
         buffer = calloc( rw*rh, sizeof(GLubyte) );
         for (int v=0; v<h; v++)
            for (int u=0; u<w; u++)
               buffer[ (b+v)*rw+(b+u) ] = bitmap.buffer[ v*w+u ];
         /* Compute signed fdistance field with buffered glyph. */
         c->dataf = make_distance_mapbf( buffer, rw, rh, &vmax );
         free( buffer );
      }
      c->w     = rw;
      c->h     = rh;
      c->m     = (2. * vmax * stsh->h) / FONT_DISTANCE_FIELD_SIZE;
      c->off_x = slot->bitmap_left-b;
      c->off_y = slot->bitmap_top +b;
      c->adv_x = (GLfloat)slot->metrics.horiAdvance / 64.;
      c->ft_index = i;

      return 0;
   }
   WARN(_("Unable to load character '%#x'!"), ch);
   return -1;
}

/**
 * @brief Starts the rendering engine.
 */
static void gl_fontRenderStart( const glFontStash* stsh, double x, double y, const glColour *c, double outlineR )
{
   /* OpenGL has pixel centers at 0.5 offset. */
   mat4 H = gl_view_matrix;
   mat4_translate( &H, x+0.5*gl_screen.wscale, y+0.5*gl_screen.hscale, 0 );
   gl_fontRenderStartH( stsh, &H, c, outlineR );
}
static void gl_fontRenderStartH( const glFontStash* stsh, const mat4 *H, const glColour *c, double outlineR )
{
   double a, scale;
   const glColour *col;

   outlineR = (outlineR==-1) ? 1 : MAX( outlineR, 0 );

   /* Handle colour. */
   a = (c==NULL) ? 1. : c->a;
   if (font_restoreLast)
      col = font_lastCol;
   else if (c==NULL)
      col = &cWhite;
   else
      col = c;

   glUseProgram(shaders.font.program);
   gl_uniformAColour(shaders.font.colour, col, a);
   if (outlineR == 0.)
      gl_uniformAColour(shaders.font.outline_colour, col, 0.);
   else
      gl_uniformAColour(shaders.font.outline_colour, &cGrey10, a);

   scale = (double)stsh->h / FONT_DISTANCE_FIELD_SIZE;
   font_projection_mat = *H;
   mat4_scale( &font_projection_mat, scale, scale, 1 );

   font_restoreLast = 0;
   gl_fontKernStart();

   /* Activate the appropriate VBOs. */
   glEnableVertexAttribArray( shaders.font.vertex );
   gl_vboActivateAttribOffset( stsh->vbo_vert, shaders.font.vertex, 0, 2, GL_SHORT, 0 );
   glEnableVertexAttribArray( shaders.font.tex_coord );
   gl_vboActivateAttribOffset( stsh->vbo_tex, shaders.font.tex_coord, 0, 2, GL_FLOAT, 0 );

   /* Depth testing is used to draw the outline under the glyph. */
   if (outlineR > 0.)
      glEnable( GL_DEPTH_TEST );
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
      case 'o': col = &cFontOrange; break;
      case 'y': col = &cFontYellow; break;
      case 'w': col = &cFontWhite; break;
      case 'p': col = &cFontPurple; break;
      case 'n': col = &cFontGrey; break;
      /* Fancy states. */
      case 'F': col = &cFriend; break; /**< Friendly */
      case 'H': col = &cHostile; break; /**< Hostile */
      case 'N': col = &cNeutral; break; /**< Neutral */
      case 'I': col = &cInert; break; /**< Inert */
      case 'R': col = &cRestricted; break; /**< Restricted */
      case 'C': col = &cFontGreen; break; /**< Console */
      case '0': col = NULL; break;
      default:
         WARN("Unknown font escape code '%c'", ch);
         col = NULL;
         break;
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
   if (font_makeChar( stsh, &ft_char, ch ))
      return NULL;

   /* Create new character. */
   glyph = &array_grow( &stsh->glyphs );
   glyph->codepoint = ch;
   glyph->adv_x = ft_char.adv_x;
   glyph->m = ft_char.m;
   glyph->ft_index = ft_char.ft_index;
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

   free(ft_char.data);
   free(ft_char.dataf);

   return glyph;
}

/**
 * @brief Call at the start of a string/line.
 */
static void gl_fontKernStart (void)
{
   prev_glyph_index = 0;
}

/**
 * @brief Return the signed advance (same units as adv_x) ahead of the current char.
 */
static int gl_fontKernGlyph( glFontStash* stsh, uint32_t ch, glFontGlyph* glyph )
{
   FT_Face ft_face;
   FT_UInt ft_glyph_index;
   int kern_adv_x = 0;

   ft_face = stsh->ft[glyph->ft_index].face;
   ft_glyph_index = FT_Get_Char_Index( ft_face, ch );
   if (prev_glyph_index && prev_glyph_ft_index == glyph->ft_index) {
      FT_Vector kerning;
      FT_Get_Kerning( ft_face, prev_glyph_index, ft_glyph_index, FT_KERNING_DEFAULT, &kerning );
      kern_adv_x = kerning.x / 64;
   }
   prev_glyph_index = ft_glyph_index;
   prev_glyph_ft_index = glyph->ft_index;
   return kern_adv_x;
}

/**
 * @brief Renders a character.
 */
static int gl_fontRenderGlyph( glFontStash* stsh, uint32_t ch, const glColour *c, int state )
{
   double scale;
   int kern_adv_x;
   glFontGlyph *glyph;

   /* Handle escape sequences. */
   if ((ch == FONT_COLOUR_CODE) && (state==0)) { /* Start sequence. */
      return 1;
   }
   if ((state == 1) && (ch != FONT_COLOUR_CODE)) {
      const glColour *col = gl_fontGetColour( ch );
      double a = (c==NULL) ? 1. : c->a;
      if (col != NULL)
         gl_uniformAColour(shaders.font.colour, col, a );
      else if (c==NULL)
         gl_uniformColour(shaders.font.colour, &cWhite);
      else
         gl_uniformColour(shaders.font.colour, c);
      font_lastCol = col;
      return 0;
   }

   /* Unicode goes here.
    * First try to find the glyph. */
   glyph = gl_fontGetGlyph( stsh, ch );
   if (glyph == NULL) {
      WARN(_("Unable to find glyph '%d'!"), ch );
      return -1;
   }

   /* Kern if possible. */
   scale = (double)stsh->h / FONT_DISTANCE_FIELD_SIZE;
   kern_adv_x = gl_fontKernGlyph( stsh, ch, glyph );
   if (kern_adv_x) {
      mat4_translate( &font_projection_mat,
            kern_adv_x/scale, 0, 0 );
   }

   /* Activate texture. */
   glBindTexture(GL_TEXTURE_2D, stsh->tex[glyph->tex_index].id);

   glUniform1f(shaders.font.m, glyph->m);
   gl_uniformMat4(shaders.font.projection, &font_projection_mat);

   /* Draw the element. */
   glDrawArrays( GL_TRIANGLE_STRIP, glyph->vbo_id, 4 );

   /* Translate matrix. */
   mat4_translate( &font_projection_mat,
         glyph->adv_x/scale, 0, 0 );

   return 0;
}

/**
 * @brief Ends the rendering engine.
 */
static void gl_fontRenderEnd (void)
{
   glDisableVertexAttribArray( shaders.font.vertex );
   glDisableVertexAttribArray( shaders.font.tex_coord );
   glUseProgram(0);

   glDisable( GL_DEPTH_TEST );

   /* Check for errors. */
   gl_checkErr();
}

/**
 * @brief Sets the minification and magnification filters for a font.
 *
 *    @param ft_font Font to set filters of.
 *    @param min Minification filter (GL_LINEAR on GL_NEAREST).
 *    @param mag Magnification filter (GL_LINEAR on GL_NEAREST).
 */
void gl_fontSetFilter( const glFont *ft_font, GLint min, GLint mag )
{
   glFontStash *stsh = gl_fontGetStash( ft_font );
   stsh->minfilter = min;
   stsh->magfilter = mag;

   for (int i=0; i<array_size(stsh->tex); i++) {
      glBindTexture( GL_TEXTURE_2D, stsh->tex[i].id );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, stsh->magfilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, stsh->minfilter);
   }

   gl_checkErr();
}

/**
 * @brief Initializes a font.
 *
 *    @param font Font to load (NULL defaults to gl_defFont).
 *    @param fname Name of the font (from inside packfile).
 *    @param h Height of the font to generate.
 *    @param prefix Prefix to look for the font.
 *    @param flags Flags to use when generating the font.
 *    @return 0 on success.
 */
int gl_fontInit( glFont* font, const char *fname, const unsigned int h, const char *prefix, unsigned int flags )
{
   size_t len, plen;
   glFontStash *stsh, *reusable_stsh_slot;
   int ch;
   char fullname[PATH_MAX];

   /* Initialize FreeType. */
   if (font_library == NULL) {
      if (FT_Init_FreeType( &font_library )) {
         WARN(_("FT_Init_FreeType failed with font %s."), fname);
         return -1;
      }
   }

   /* Replace name if NULL. */
   if (fname == NULL)
      fname = FONT_DEFAULT_PATH;

   /* Get font stash. */
   if (avail_fonts==NULL)
      avail_fonts = array_create( glFontStash );

   /* Check if available. */
   reusable_stsh_slot = NULL;
   if (!(flags & FONT_FLAG_DONTREUSE)) {
      for (int i=0; i<array_size(avail_fonts); i++) {
         stsh = &avail_fonts[i];
         if (stsh->fname == NULL) {
            /* This glFontStash must have been zeroed by gl_freeFont after its refcount dropped to zero. */
            reusable_stsh_slot = stsh;
            continue;
         }
         if (strcmp(stsh->fname,fname)!=0 || stsh->h != (int)h)
            continue;
         /* Found a match! */
         stsh->refcount++;
         font->id = stsh - avail_fonts;
         font->h = h;
         return 0;
      }
   }

   /* Create new font. */
   if (reusable_stsh_slot != NULL)
      stsh = reusable_stsh_slot;
   else
      stsh = &array_grow( &avail_fonts );
   memset( stsh, 0, sizeof(glFontStash) );
   stsh->refcount = 1; /* Initialize refcount. */
   stsh->fname = strdup(fname);
   font->id = stsh - avail_fonts;
   font->h = h;

   /* Default stuff. */
   stsh->magfilter = GL_LINEAR;
   stsh->minfilter = GL_LINEAR;
   stsh->tw = DEFAULT_TEXTURE_SIZE;
   stsh->th = DEFAULT_TEXTURE_SIZE;
   stsh->h = h;

   /* Set up font stuff for next glyphs. */
   stsh->ft = array_create( glFontStashFreetype );
   ch = 0;
   len = strlen(fname);
   plen = strlen(prefix);
   for (size_t i=0; i<=len; i++) {
      if ((fname[i]=='\0') || (fname[i]==',')) {
         strncpy( fullname, prefix, sizeof(fullname)-1 );
         strncat( fullname, &fname[ch], MIN( sizeof(fullname)-1-plen, i-ch ) );
         gl_fontstashAddFallback( stsh, fullname, h );
         ch = i;
         if (fname[i]==',')
            ch++;
      }
   }

   /* Initialize the unicode support. */
   for (int i=0; i<HASH_LUT_SIZE; i++)
      stsh->lut[i] = -1;
   stsh->glyphs = array_create( glFontGlyph );
   stsh->tex    = array_create( glFontTex );

   /* Set up VBOs. */
   stsh->mvbo = 256;
   stsh->vbo_tex_data  = calloc( 8*stsh->mvbo, sizeof(GLfloat) );
   stsh->vbo_vert_data = calloc( 8*stsh->mvbo, sizeof(GLshort) );
   stsh->vbo_tex  = gl_vboCreateStatic( sizeof(GLfloat)*8*stsh->mvbo,  stsh->vbo_tex_data );
   stsh->vbo_vert = gl_vboCreateStatic( sizeof(GLshort)*8*stsh->mvbo, stsh->vbo_vert_data );

   return 0;
}

/**
 * @brief Adds a fallback font to a font.
 *
 *    @param font Font to add fallback to.
 *    @param fname Name of the fallback to add.
 *    @param prefix Prefix to use.
 *    @return 0 on success.
 */
int gl_fontAddFallback( glFont* font, const char *fname, const char *prefix )
{
   size_t len, plen;
   int ch, ret;
   glFontStash *stsh = gl_fontGetStash( font );

   ret = 0;
   ch = 0;
   len = strlen(fname);
   plen = strlen(prefix);
   for (size_t i=0; i<=len; i++) {
      if ((fname[i]=='\0') || (fname[i]==',')) {
         char fullname[PATH_MAX];
         strncpy( fullname, prefix, sizeof(fullname)-1 );
         strncat( fullname, &fname[ch], MIN( sizeof(fullname)-1-plen, i-ch ) );
         ret |= gl_fontstashAddFallback( stsh, fullname, font->h );
         ch = i;
         if (fname[i]==',')
            ch++;
      }
   }

   return ret;
}

/**
 * @brief Adds a fallback font to a stash.
 *
 *    @param stsh Stash to add fallback to.
 *    @param fname Name of the fallback to add.
 *    @param h Height to use for the font.
 *    @return 0 on success.
 */
static int gl_fontstashAddFallback( glFontStash* stsh, const char *fname, unsigned int h )
{
   glFontStashFreetype ft = {.file=NULL, .face=NULL};

   /* Set up file data. Reference a loaded copy if we have one. */
   for (int i=0; i<array_size(avail_fonts); i++) {
      if (avail_fonts[i].ft == NULL)
         continue;
      for (int j=0; j<array_size(avail_fonts[i].ft); j++)
         if (!strcmp( fname, avail_fonts[i].ft[j].file->name ))
            ft.file = avail_fonts[i].ft[j].file;
      if (ft.file != NULL) {
         ft.file->refcount++;
         break;
      }
   }

   if (ft.file == NULL) {
      /* Read font file. */
      ft.file = malloc( sizeof( glFontFile ) );
      ft.file->name = strdup( fname );
      ft.file->refcount = 1;
      ft.file->data = (FT_Byte*) ndata_read( fname, &ft.file->datasize );
      if (ft.file->data == NULL) {
         WARN(_("Unable to read font: %s"), fname );
         gl_fontstashftDestroy( &ft );
         return -1;
      }
   }

   /* Object which freetype uses to store font info. */
   if (FT_New_Memory_Face( font_library, ft.file->data, ft.file->datasize, 0, &ft.face )) {
      WARN(_("FT_New_Memory_Face failed loading library from %s"), fname);
      gl_fontstashftDestroy( &ft );
      return -1;
   }

   /* Try to resize. */
   if (FT_IS_SCALABLE(ft.face)) {
      FT_Matrix scale;
      if (FT_Set_Char_Size( ft.face,
               0, /* Same as width. */
               h * 64,
               96, /* Create at 96 DPI */
               96)) /* Create at 96 DPI */
         WARN(_("FT_Set_Char_Size failed."));
      scale.xx = scale.yy = (FT_Fixed)FONT_DISTANCE_FIELD_SIZE*0x10000/h;
      scale.xy = scale.yx = 0;
      FT_Set_Transform( ft.face, &scale, NULL );
   }
   else
      WARN(_("Font isn't resizable!"));

   /* Select the character map. */
   if (FT_Select_Charmap( ft.face, FT_ENCODING_UNICODE ))
      WARN(_("FT_Select_Charmap failed to change character mapping."));

   /* Save stuff. */
   array_push_back( &stsh->ft, ft );

   /* Success. */
   return 0;
}

/**
 * @brief Frees a loaded font.
 *        Caution: its glFontStash still has a slot in avail_fonts.
 *        At the time of writing, it's enough to zero it so it cannot
 *        match a future font request.
 *
 *    @param font Font to free.
 */
void gl_freeFont( glFont* font )
{
   if (font == NULL)
      font = &gl_defFont;
   glFontStash *stsh = gl_fontGetStash( font );

   /* Check references. */
   stsh->refcount--;
   if (stsh->refcount > 0)
      return;
   /* Not references and must eliminate. */

   for (int i=0; i<array_size(stsh->ft); i++)
      gl_fontstashftDestroy( &stsh->ft[i] );
   array_free( stsh->ft );

   free( stsh->fname );
   for (int i=0; i<array_size(stsh->tex); i++)
      glDeleteTextures( 1, &stsh->tex->id );
   array_free( stsh->tex );

   array_free( stsh->glyphs );
   gl_vboDestroy(stsh->vbo_tex);
   gl_vboDestroy(stsh->vbo_vert);
   free(stsh->vbo_tex_data);
   free(stsh->vbo_vert_data);

   memset( stsh, 0, sizeof(glFontStash) );
   /* Font stash will get reused when possible, and we can't erase because IDs
    * will get messed up. */
}

/**
 * @brief Frees resources referenced by a glFontStashFreetype struct.
 */
static void gl_fontstashftDestroy( glFontStashFreetype *ft )
{
   if (--ft->file->refcount == 0) {
      free(ft->file->name);
      free(ft->file->data);
      free(ft->file);
   }
   FT_Done_Face(ft->face);
}

/**
 * @brief Frees all resources associated with the font system.
 *        This also resets font ID tracking, so there's no going back.
 */
void gl_fontExit (void)
{
   FT_Done_FreeType( font_library );
   font_library = NULL;
   array_free( avail_fonts );
   avail_fonts = NULL;
}
