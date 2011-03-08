/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef FONT_H
#  define FONT_H


#include "opengl.h"


/**
 * @brief Represents a character in the font.
 */
typedef struct glFontChar_s {
   double adv_x; /**< X advancement. */
   double adv_y; /**< Y advancement. */
} glFontChar;


/**
 * @struct glFont
 *
 * @brief Represents a font in memory.
 */
typedef struct glFont_s {
   int h; /**< Font height. */
   GLuint texture; /**< Font atlas. */
   gl_vbo *vbo_tex; /**< VBO assosciated to texture coordinates. */
   gl_vbo *vbo_vert; /**< VBO assosciated to vertex coordinates. */
   glFontChar *chars; /**< Characters in the font. */
} glFont;
extern glFont gl_defFont; /**< default font */
extern glFont gl_smallFont; /**< small font */


/*
 * glFont loading / freeing
 *
 * if font is NULL it uses the internal default font same with gl_print
 */
void gl_fontInit( glFont* font, const char *fname, const unsigned int h );
void gl_freeFont( glFont* font );


/*
 * const char printing
 */
void gl_printRaw( const glFont *ft_font,
      const double x, const double y,
      const glColour* c, const char *text );
int gl_printMaxRaw( const glFont *ft_font, const int max,
      const double x, const double y,
      const glColour* c, const char *text );
int gl_printMidRaw( const glFont *ft_font, const int width,
      double x, const double y,
      const glColour* c, const char *text );
int gl_printTextRaw( const glFont *ft_font,
      const int width, const int height,
      double bx, double by,
      glColour* c, const char *text );


/*
 * printf style printing.
 */
/* prints text normally */
void gl_print( const glFont *ft_font, const double x, const double y,
      const glColour *c, const char *fmt, ... );
/* prints text to a max length */
int gl_printMax( const glFont *ft_font, const int max,
      const double x, const double y,
      const glColour *c, const char *fmt, ... );
/* prints text centered in width at x */
int gl_printMid( const glFont *ft_font, const int width,
      double x, const double y,
      const glColour* c, const char *fmt, ... );
/* respects \n -> bx,by is TOP LEFT POSITION */
int gl_printText( const glFont *ft_font,
      const int width, const int height,
      double bx, double by,
      glColour* c, const char *fmt, ... );


/* Dimension stuff. */
int gl_printWidthForText( const glFont *ft_font, const char *text,
      const int width );
int gl_printWidthRaw( const glFont *ft_font, const char *text );
int gl_printWidth( const glFont *ft_font, const char *fmt, ... );
int gl_printHeightRaw( const glFont *ft_font, const int width, const char *text );
int gl_printHeight( const glFont *ft_font,
      const int width, const char *fmt, ... );

/* Misc stuff. */
void gl_printRestoreLast (void);


#endif /* FONT_H */

