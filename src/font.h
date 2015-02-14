/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 * @brief Represents a font in memory.
 */
typedef struct glFont_s {
   int h; /**< Font height. */
   GLuint texture; /**< Font atlas. */
   gl_vbo *vbo_tex; /**< VBO associated to texture coordinates. */
   gl_vbo *vbo_vert; /**< VBO associated to vertex coordinates. */
   glFontChar *chars; /**< Characters in the font. */
} glFont;
extern glFont gl_defFont; /**< Default font. */
extern glFont gl_smallFont; /**< Small font. */
extern glFont gl_defFontMono; /**< Default mono font. */


/**
 * @brief Evil hack to allow restoring, yes it makes me cry myself to sleep.
 */
typedef struct glFontRestore_s {
   const glColour *col; /**< Colour to restore. */
} glFontRestore;


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
      const glColour* c, const char *text );


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
      const glColour* c, const char *fmt, ... );


/* Dimension stuff. */
int gl_printWidthForText( const glFont *ft_font, const char *text,
      const int width );
int gl_printWidthRaw( const glFont *ft_font, const char *text );
int gl_printWidth( const glFont *ft_font, const char *fmt, ... );
int gl_printHeightRaw( const glFont *ft_font, const int width, const char *text );
int gl_printHeight( const glFont *ft_font,
      const int width, const char *fmt, ... );

/* Restore hacks. */
void gl_printRestoreClear (void);
void gl_printRestoreInit( glFontRestore *restore );
void gl_printRestoreLast (void);
void gl_printRestore( const glFontRestore *restore );
void gl_printStoreMax( glFontRestore *restore, const char *text, int max );
void gl_printStore( glFontRestore *restore, const char *text );


#endif /* FONT_H */

