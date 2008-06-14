/*
 * See Licensing and Copyright notice in naev.h
 */



#include "font.h"

#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "naev.h"
#include "log.h"
#include "pack.h"


#define FONT_DEF  "dat/font.ttf"


/*
 *
 *    OpenGL font rendering routines
 *
 * Use a displaylist to store ASCII chars rendered with freefont
 * There are several drawing methods depending on whether you want
 * print it all, print to a max width, print centered or print a
 * block of text.
 *
 * There are hardcoded size limits.  256 characters for all routines
 * except gl_printText which has a 1024 limit.
 *
 * TODO check if length is too long
 */



/* default font */
glFont gl_defFont;
glFont gl_smallFont;


/* 
 * prototypes
 */
static void glFontMakeDList( FT_Face face, char ch,
      GLuint list_base, GLuint *tex_base, int *width_base );
static int pot( int n );


/*
 * gets the closest power of two
 */
static int pot( int n )
{
   int i = 1;
   while (i < n)
      i <<= 1;
   return i;
}


/*
 * prints text on screen like printf
 *
 * defaults ft_font to gl_defFont if NULL
 */
void gl_print( const glFont *ft_font,
      const double x, const double y,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[256]; /* holds the string */
   va_list ap;

   if (ft_font == NULL) ft_font = &gl_defFont;                            

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
/*
 * behaves exactly like gl_print but prints to a maximum length of max    
 * returns how many characters it had to suppress
 */
int gl_printMax( const glFont *ft_font, const int max,
      const double x, const double y,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[256]; /* holds the string */
   va_list ap;
   int i, n, len, ret;

   ret = 0; /* default return value */

   if (ft_font == NULL) ft_font = &gl_defFont;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }


   /* limit size */
   len = (int)strlen(text);
   for (n=0,i=0; i<len; i++) {
      n += ft_font->w[ (int)text[i] ];
      if (n > max) {
         ret = len - i; /* difference */
         text[i] = '\0';
         break;
      }
   }

   /* display the text */
   glEnable(GL_TEXTURE_2D);

   glListBase(ft_font->list_base);


   glMatrixMode(GL_MODELVIEW); /* using MODELVIEW, PROJECTION gets full fast */
   glPushMatrix(); /* translation matrix */
      glTranslated( x-(double)SCREEN_W/2., y-(double)SCREEN_H/2., 0);

   if (c==NULL) glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);
   glCallLists(i, GL_UNSIGNED_BYTE, &text);

   glPopMatrix(); /* translation matrix */
   glDisable(GL_TEXTURE_2D);

   gl_checkErr();

   return ret;
}
/*
 * functions like gl_printMax but centers the text in the width
 */
int gl_printMid( const glFont *ft_font, const int width,
      double x, const double y,
      const glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[256]; /* holds the string */
   va_list ap;
   int i, n, len, ret;

   ret = 0; /* default return value */

   if (ft_font == NULL) ft_font = &gl_defFont;

   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   }

   /* limit size */
   len = (int)strlen(text);
   for (n=0,i=0; i<len; i++) {
      n += ft_font->w[ (int)text[i] ];
      if (n > width) {
         ret = len - i; /* difference */
         n -= ft_font->w[ (int)text[i] ]; /* actual size */
         text[i] = '\0';
         break;
      }
   }

   x += (double)(width - n)/2.;

   /* display the text */
   glEnable(GL_TEXTURE_2D);

   glListBase(ft_font->list_base);

   glMatrixMode(GL_MODELVIEW); /* using MODELVIEW, PROJECTION gets full fast */
   glPushMatrix(); /* translation matrix */
      glTranslated( x-(double)SCREEN_W/2., y-(double)SCREEN_H/2., 0);

   if (c==NULL) glColor4d( 1., 1., 1., 1. );
   else COLOUR(*c);
   glCallLists(i, GL_UNSIGNED_BYTE, &text);

   glPopMatrix(); /* translation matrix */
   glDisable(GL_TEXTURE_2D);

   gl_checkErr();

   return ret;
}
/*
 * prints text with line breaks included to a maximum width and height preset
 */
int gl_printText( const glFont *ft_font,
      const int width, const int height,
      double bx, double by,
      glColour* c, const char *fmt, ... )
{
   /*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
   char text[4096]; /* holds the string */
   char buf[256];
   va_list ap;
   int p, i, j, n, m, len, ret, lastspace;
   double x,y;

   ret = 0; /* default return value */

   if (ft_font == NULL) ft_font = &gl_defFont;

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

   len = (int)strlen(text);
   /* limit size per line */
   lastspace = -1; /* last ' ' or '\n' in the text */
   n = 0; /* current width */
   i = 0; /* current position */
   p = -1; /* where we last drew up to */
   while (i<len+1) {
      
      if (by - y > (double)height) return len-lastspace; /* past height */

      n += ft_font->w[ (int)text[i] ];
      
      if ((text[i]==' ') || (text[i]=='\n') || (text[i]=='\0')) lastspace = i;

      if (((n > width) && (p!=lastspace))
            || (text[i]=='\n') || (text[i]=='\0')) {

         /* time to draw the line */
         m = 0;
         if (lastspace==-1) lastspace = 0;
         for (j=0; j<(lastspace-p-1); j++) {
            if (text[p+j+1]=='\t') {
               p++;
               continue;
            }
            m += ft_font->w[ (int)text[p+j+1] ];
            if (m > width) break;
            buf[j] = text[p+j+1];
         }
         /* no need for NUL termination */

         glMatrixMode(GL_MODELVIEW); /* using MODELVIEW, PROJECTION gets full fast */
         glPushMatrix(); /* translation matrix */                               
            glTranslated( x, y, 0);

         glCallLists(j, GL_UNSIGNED_BYTE, &buf); /* the actual displaying */

         glPopMatrix(); /* translation matrx */

         p = lastspace;
         n = 0;
         i = lastspace;
         y -= 1.5*(double)ft_font->h; /* move position down */
      }
      i++;
   }

   glDisable(GL_TEXTURE_2D);

   gl_checkErr();

   return ret;
}
/*
 * gets the width of the text about to be printed
 */
int gl_printWidth( const glFont *ft_font, const char *fmt, ... )
{  
   int i, n;
   char text[256]; /* holds the string */
   va_list ap;                                                            

   if (ft_font == NULL) ft_font = &gl_defFont;

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

/*
 * prints the height of the text about to be printed
 */
int gl_printHeight( const glFont *ft_font,
      const int width, const char *fmt, ... )
{
   char text[1024]; /* holds the string */
   va_list ap;
   int p, i, n, len, lastspace;
   double y;
   
   if (ft_font == NULL) ft_font = &gl_defFont;
   
   if (fmt == NULL) return -1;
   else { /* convert the symbols to text */
      va_start(ap, fmt);
      vsprintf(text, fmt, ap);
      va_end(ap);
   } 
   y = 0.;
   
   len = (int)strlen(text);
   /* limit size per line */
   lastspace = -1; /* last ' ' or '\n' in the text */
   n = 0; /* current width */
   i = 0; /* current position */
   p = -1; /* where we last drew up to */
   while (i<len+1) {

      n += ft_font->w[ (int)text[i] ];

      if ((text[i]==' ') || (text[i]=='\n') || (text[i]=='\0')) lastspace = i;

      if (((n > width) && (p!=lastspace))
            || (text[i]=='\n') || (text[i]=='\0')) {
         p = lastspace;
         n = 0;
         i = lastspace;
         y += 1.5*(double)ft_font->h; /* move position down */
      }
      i++;
   }
   
   return (int) y;
}


/*
 *
 * G L _ F O N T
 *
 */
/*
 * basically taken from NeHe lesson 43
 * http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=43
 */
static void glFontMakeDList( FT_Face face, char ch,
      GLuint list_base, GLuint *tex_base, int* width_base )
{  
   FT_Glyph glyph;
   FT_Bitmap bitmap;
   GLubyte* expanded_data;
   int w,h;
   int i,j;
   double x,y;

   if (FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ),
      FT_LOAD_FORCE_AUTOHINT)) /* FT_LOAD_DEFAULT )) - looks much better then default */
      WARN("FT_Load_Glyph failed");                                       

   if (FT_Get_Glyph( face->glyph, &glyph ))
      WARN("FT_Get_Glyph failed");

   /* converting our glyph to a bitmap */
   FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
   FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;

   bitmap = bitmap_glyph->bitmap; /* to simplify */

   /* need the POT wrapping for opengl */
   w = pot(bitmap.width);
   h = pot(bitmap.rows);

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
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
         GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

   free(expanded_data); /* no use for this anymore */

   /* creating of the display list */
   glNewList(list_base+ch,GL_COMPILE);

   /* corrects a spacing flaw between letters and
    * downwards correction for letters like g or y */
   glPushMatrix();                                                        
      glTranslated( bitmap_glyph->left, bitmap_glyph->top-bitmap.rows, 0);


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
   glTranslated( face->glyph->advance.x >> 6, 0, 0);
   width_base[(int)ch] = (int)(face->glyph->advance.x >> 6);

   /* end of display list */
   glEndList();

   FT_Done_Glyph(glyph);

   gl_checkErr();
}
void gl_fontInit( glFont* font, const char *fname, const unsigned int h )
{
   uint32_t bufsize;
   int i;

   if (font == NULL) font = &gl_defFont;

   FT_Byte* buf = pack_readfile( DATA, (fname) ? fname : FONT_DEF, &bufsize );

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
   FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);

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
void gl_freeFont( glFont* font )
{
   if (font == NULL) font = &gl_defFont;
   glDeleteLists(font->list_base,128);
   glDeleteTextures(128,font->textures);
   free(font->textures);
   free(font->w);
}
