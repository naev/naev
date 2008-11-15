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
 * There are hardcoded size limits.  256 characters for all routines
 * except gl_printText which has a 1024 limit.
 *
 * @todo check if length is too long
 */


#include "font.h"

#include "ft2build.h"
#include FT_FREETYPE_H 
#include FT_GLYPH_H 

#include "naev.h"
#include "log.h"
#include "pack.h"


#define FONT_DEF  "dat/font.ttf" /**< Default font path. */


/* default font */
glFont gl_defFont; /**< Default font. */
glFont gl_smallFont; /**< Small font. */


/* 
 * prototypes
 */
static void glFontMakeDList( FT_Face face, char ch,
      GLuint list_base, GLuint *tex_base, int *width_base );
static int font_limitSize( const glFont *ft_font, int *width,
      char *text, const int max );


/**
 * @brief Limits the text to max.
 *
 *    @param ft_font Font to calculate width with.
 *    @param width Actual width it takes up.
 *    @param text Text to parse.
 *    @param max Max to look for.
 */
static int font_limitSize( const glFont *ft_font, int *width,
      char *text, const int max )
{
   int n, len, i;

   /* limit size */
   len = (int)strlen(text);
   n = 0;
   for (i=0; i<len; i++) {
      n += ft_font->w[ (int)text[i] ];
      if (n > max) {
         n -= ft_font->w[ (int)text[i] ]; /* actual size */
         text[i] = '\0';
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
int gl_printWidthForText( const glFont *ft_font, char *text,
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

      /* Increase size. */
      n += ft_font->w[ (int)text[i] ];

      /* Save last space. */
      if (text[i] == ' ')
         lastspace = i;

      /* Check if out of bounds. */
      if (n > width)
         return lastspace;   

      /* Check next character. */
      i++;
   }

   return i;
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

   if (ft_font == NULL)
      ft_font = &gl_defFont;                            

   if (fmt == NULL) return;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }

   glEnable(GL_TEXTURE_2D);

   glListBase(ft_font->list_base);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix(); /* translation matrix */
      glTranslated( x-(double)SCREEN_W/2., y-(double)SCREEN_H/2., 0);

   if (c==NULL) glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);
   glCallLists(strlen(text), GL_UNSIGNED_BYTE, &text);

   glPopMatrix(); /* translation matrix */
   glDisable(GL_TEXTURE_2D);

   gl_checkErr();
}
/**
 * @brief Behavise like gl_print but stops displaying text after reaching a certain length.
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
   int ret;

   ret = 0; /* default return value */

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }

   /* limit size */
   ret = font_limitSize( ft_font, NULL, text, max );

   /* display the text */
   glEnable(GL_TEXTURE_2D);

   glListBase(ft_font->list_base);

   glMatrixMode(GL_MODELVIEW); /* using MODELVIEW, PROJECTION gets full fast */
   glPushMatrix(); /* translation matrix */
      glTranslated( x-(double)SCREEN_W/2., y-(double)SCREEN_H/2., 0);

   if (c==NULL) glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);
   glCallLists(ret, GL_UNSIGNED_BYTE, &text);

   glPopMatrix(); /* translation matrix */
   glDisable(GL_TEXTURE_2D);

   gl_checkErr();

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
   int n, ret;

   ret = 0; /* default return value */

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }

   /* limit size */
   ret = font_limitSize( ft_font, &n, text, width );
   x += (double)(width - n)/2.;

   /* display the text */
   glEnable(GL_TEXTURE_2D);

   glListBase(ft_font->list_base);

   glMatrixMode(GL_MODELVIEW); /* using MODELVIEW, PROJECTION gets full fast */
   glPushMatrix(); /* translation matrix */
      glTranslated( x-(double)SCREEN_W/2., y-(double)SCREEN_H/2., 0);

   if (c==NULL) glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);
   glCallLists(ret, GL_UNSIGNED_BYTE, &text);

   glPopMatrix(); /* translation matrix */
   glDisable(GL_TEXTURE_2D);

   gl_checkErr();

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
      glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[4096]; /* holds the string */
   va_list ap;
   int i, p;
   double x,y;

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }
   bx -= (double)SCREEN_W/2.;
   by -= (double)SCREEN_H/2.;
   x = bx;
   y = by + height - (double)ft_font->h; /* y is top left corner */

   /* prepare ze opengl */
   glEnable(GL_TEXTURE_2D);
   glListBase(ft_font->list_base);
   if (c==NULL) glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);

   p = 0; /* where we last drew up to */
   while (y - by > -1e-5) {
      i = gl_printWidthForText( ft_font, &text[p], width );

      glMatrixMode(GL_MODELVIEW); /* using MODELVIEW, PROJECTION gets full fast */
      glPushMatrix(); /* translation matrix */                               
         glTranslated( x, y, 0);

      glCallLists(i, GL_UNSIGNED_BYTE, &text[p]); /* the actual displaying */

      glPopMatrix(); /* translation matrix */

      if (text[p+i] == '\0')
         break;
      p += i + 1;
      y -= 1.5*(double)ft_font->h; /* move position down */
   }

   glDisable(GL_TEXTURE_2D);

   gl_checkErr();

   return 0;
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
   int i, n;
   char text[256]; /* holds the string */
   va_list ap;                                                            

   if (ft_font == NULL)
      ft_font = &gl_defFont;

   if (fmt == NULL) return 0;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }

   for (n=0,i=0; i<(int)strlen(text); i++)
      n += ft_font->w[ (int)text[i] ];

   return n;
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
   int i, p;
   double y;
   
   if (ft_font == NULL) 
      ft_font = &gl_defFont;
   
   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   } 

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


/*
 *
 * G L _ F O N T
 *
 */
/**
 * @brief Makes the font display list.
 *
 * Basically taken from NeHe lesson 43
 * http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=43
 */
static void glFontMakeDList( FT_Face face, char ch,
      GLuint list_base, GLuint *tex_base, int* width_base )
{  
   FT_Bitmap bitmap;
   GLubyte* expanded_data;
   FT_GlyphSlot slot;
   int w,h;
   int i,j;
   double x,y;

   slot = face->glyph; /* Small shortcut. */

   /* Load the glyph. */
   if (FT_Load_Char( face, ch, FT_LOAD_RENDER ))
      WARN("FT_Load_Char failed.");                                       

   bitmap = slot->bitmap; /* to simplify */

   /* need the POT wrapping for opengl */
   w = gl_pot(bitmap.width);
   h = gl_pot(bitmap.rows);

   /* memory for textured data
    * bitmap is using two channels, one for luminosity and one for alpha */
   expanded_data = (GLubyte*) malloc(sizeof(GLubyte)*2* w*h + 1);
   for (j=0; j < h; j++) {
      for (i=0; i < w; i++ ) {
         expanded_data[2*(i+j*w)]= expanded_data[2*(i+j*w)+1] =
            (i>=bitmap.width || j>=bitmap.rows) ?
            0 : bitmap.buffer[i + bitmap.width*j];
      }
   }

   /* creating the opengl texture */
   glBindTexture( GL_TEXTURE_2D, tex_base[(int)ch]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
         GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

   free(expanded_data); /* no use for this anymore */

   /* creating of the display list */
   glNewList(list_base+ch,GL_COMPILE);

   /* corrects a spacing flaw between letters and
    * downwards correction for letters like g or y */
   glPushMatrix();                                                        
      glTranslated( slot->bitmap_left, slot->bitmap_top - bitmap.rows, 0);


   /* take into account opengl POT wrapping */
   x = (double)bitmap.width/(double)w;
   y = (double)bitmap.rows/(double)h;

   /* draw the texture mapped QUAD */
   glBindTexture(GL_TEXTURE_2D,tex_base[(int)ch]);
   glBegin( GL_QUADS );

      glTexCoord2d( 0, 0 );
         glVertex2d( 0, bitmap.rows );
      
      glTexCoord2d( x, 0);
         glVertex2d( bitmap.width, bitmap.rows );

      glTexCoord2d( x, y );
         glVertex2d( bitmap.width, 0 );
      
      glTexCoord2d( 0, y );
         glVertex2d( 0, 0 );

   glEnd(); /* GL_QUADS */

   glPopMatrix(); /* translation matrix */
   glTranslated( slot->advance.x >> 6, slot->advance.y >> 6, 0);
   width_base[(int)ch] = slot->advance.x >> 6;

   /* end of display list */
   glEndList();

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
   uint32_t bufsize;
   int i;

   if (font == NULL) font = &gl_defFont;

   FT_Byte* buf = pack_readfile( DATA, (fname!=NULL) ? fname : FONT_DEF, &bufsize );

   /* allocage */
   font->textures = malloc(sizeof(GLuint)*128);
   font->w = malloc(sizeof(int)*128);
   font->h = (int)h;
   if ((font->textures==NULL) || (font->w==NULL)) {
      WARN("Out of memory!");
      return;
   }

   /* create a FreeType font library */
   FT_Library library;
   if (FT_Init_FreeType(&library))
      WARN("FT_Init_FreeType failed");

   /* object which freetype uses to store font info */
   FT_Face face;
   if (FT_New_Memory_Face( library, buf, bufsize, 0, &face ))
      WARN("FT_New_Face failed loading library from %s", fname );

   /* FreeType is cool and measures using 1/64 of a pixel, therefore expand */
   FT_Set_Char_Size( face,
         0, /* Same as width. */
         h << 6, /* In 1/64th of a pixel. */
         96,
         96);

   /* Select the character map. */
   if (FT_Select_Charmap( face, FT_ENCODING_UNICODE ))
      WARN("FT_Select_Charmap failed to change character mapping.");

   /* have OpenGL allocate space for the textures / display list */
   font->list_base = glGenLists(128);
   glGenTextures( 128, font->textures );


   /* create each of the font display lists */
   for (i=0; i<128; i++)
      glFontMakeDList( face, i, font->list_base, font->textures, font->w );

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
   if (font == NULL) font = &gl_defFont;
   glDeleteLists(font->list_base,128);
   glDeleteTextures(128,font->textures);
   free(font->textures);
   free(font->w);
}
