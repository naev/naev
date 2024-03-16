/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nstring.h"
#include "opengl.h"

#define FONT_COLOUR_CODE '#'

#define FONT_FLAG_DONTREUSE                                                    \
   ( 1 << 1 ) /**< Don't reuse the font if it's loaded somewhere else. */

/**
 * @brief Represents a font in memory.
 */
typedef struct glFont_s {
   int id; /**< Font stash id. */
   int h;  /**< Font height. */
} glFont;
extern glFont gl_defFont;     /**< Default font. */
extern glFont gl_smallFont;   /**< Small font. */
extern glFont gl_defFontMono; /**< Default mono font. */

/**
 * @brief Evil hack to allow restoring, yes it makes me cry myself to sleep.
 */
typedef struct glFontRestore_s {
   const glColour *col; /**< Colour to restore. */
} glFontRestore;

/**
 * @brief The state of a line iteration. This matches the process of rendering
 * text into an on-screen box: An empty string produces a zero-width line. Each
 * regular, fitting character expands its line horizontally. A newline or
 * wrapping leads to vertical expansion. Word-wrapping happens at "line break
 * opportunities" (defined by Unicode), or mid-word if there's no other way to
 * fit in the width limit. The iterator honors the width limit if at all
 * possible; the only exception is when a single character is enough to overflow
 * it. The layout calculation is iterative; one may for instance change the
 * width limit between lines. \see gl_printLineIteratorInit,
 * gl_printLineIteratorNext.
 */
typedef struct glPrintLineIterator_s {
   const char   *text;    /**< Text to split. */
   const glFont *ft_font; /**< Font to use. */
   int           width;   /**< Maximum width of a line. */
   int           l_width; /**< The current line's actual width. */
   size_t l_begin, l_end; /**< The current line's location (&text[l_begin],
                             inclusive, to &text[l_end], exclusive). */
   size_t l_next; /**< Starting point for next iteration, i.e., l_end plus any
                     spaces that became a line break. */
   uint8_t dead;  /**< Did we emit a line where the text ends? */
   uint8_t no_soft_breaks; /**< Disable word wrapping, e.g. for one-line input
                              widgets. */
} glPrintLineIterator;

/*
 * glFont loading / freeing
 *
 * if font is NULL it uses the internal default font same with gl_print
 */
int  gl_fontInit( glFont *font, const char *fname, const unsigned int h,
                  const char *prefix, unsigned int flags );
int  gl_fontAddFallback( glFont *font, const char *fname, const char *prefix );
int  gl_fontAddFallbackFont( glFont *font, const glFont *f );
void gl_freeFont( glFont *font );
void gl_fontExit( void );

/*
 * const char printing
 */
void gl_printRaw( const glFont *ft_font, const double x, const double y,
                  const glColour *c, const double outlineR, const char *text );
void gl_printRawH( const glFont *ft_font, const mat4 *H, const glColour *c,
                   const double outlineR, const char *text );
int  gl_printMaxRaw( const glFont *ft_font, const int max, const double x,
                     const double y, const glColour *c, const double outlineR,
                     const char *text );
int  gl_printMidRaw( const glFont *ft_font, const int width, double x,
                     const double y, const glColour *c, const double outlineR,
                     const char *text );
int  gl_printTextRaw( const glFont *ft_font, const int width, const int height,
                      double bx, double by, int line_height, const glColour *c,
                      const double outlineR, const char *text );
void gl_printMarkerRaw( const glFont *ft_font, const double x, const double y,
                        const glColour *c, const char *text );

/*
 * printf style printing.
 */
/* prints text normally */
PRINTF_FORMAT( 5, 6 )
void gl_print( const glFont *ft_font, double x, double y, const glColour *c,
               const char *fmt, ... );
/* prints text to a max length */
PRINTF_FORMAT( 6, 7 )
int gl_printMax( const glFont *ft_font, const int max, double x, double y,
                 const glColour *c, const char *fmt, ... );
/* prints text centered in width at x */
PRINTF_FORMAT( 6, 7 )
int gl_printMid( const glFont *ft_font, const int width, double x, double y,
                 const glColour *c, const char *fmt, ... );
/* respects \n -> bx,by is TOP LEFT POSITION */
PRINTF_FORMAT( 8, 9 )
int gl_printText( const glFont *ft_font, int width, int height, double bx,
                  double by, int line_height, const glColour *c,
                  const char *fmt, ... );

/* Dimension stuff. */
void gl_printLineIteratorInit( glPrintLineIterator *iter, const glFont *ft_font,
                               const char *text, int width );
int  gl_printLineIteratorNext( glPrintLineIterator *iter );
int  gl_printWidthRaw( const glFont *ft_font, const char *text );
PRINTF_FORMAT( 2, 3 )
int gl_printWidth( const glFont *ft_font, const char *fmt, ... );
int gl_printHeightRaw( const glFont *ft_font, int width, const char *text );
PRINTF_FORMAT( 3, 4 )
int gl_printHeight( const glFont *ft_font, int width, const char *fmt, ... );
int gl_printLinesRaw( const glFont *ft_font, int width, const char *text );
PRINTF_FORMAT( 3, 4 )
int gl_printLines( const glFont *ft_font, int width, const char *fmt, ... );

/* Restore hacks. */
void gl_printRestoreClear( void );
void gl_printRestoreInit( glFontRestore *restore );
void gl_printRestoreLast( void );
void gl_printRestore( const glFontRestore *restore );
void gl_printStoreMax( glFontRestore *restore, const char *text, int max );
void gl_printStore( glFontRestore *restore, const char *text );

/* Misc stuff. */
void gl_fontSetFilter( const glFont *ft_font, GLint min, GLint mag );
