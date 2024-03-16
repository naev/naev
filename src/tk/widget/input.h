/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "font.h"

/**
 * @brief The input widget data.
 */
typedef struct WidgetInputData_ {
   char  *filter;     /**< Characters to filter. */
   char  *input;      /**< Input buffer. */
   char  *empty_text; /**< Text to display when empty. */
   int    oneline;    /**< Is it a one-liner? no '\n' and friends */
   size_t char_max; /**< Maximum number of code points, including terminal null,
                       in buffer. */
   size_t  byte_max; /**< Byte size of the buffer. */
   size_t  view;     /**< View position. */
   size_t  pos;      /**< Cursor position. */
   glFont *font;     /**< Font to use. */
   void ( *fptr )(
      unsigned int,
      const char * ); /**< Modify callback - triggered on text input. */
} WidgetInputData;

/* Required functions. */
void window_addInput( unsigned int wid, const int x, const int y, /* position */
                      const int w, const int h,                   /* size */
                      const char *name, const int max, const int oneline,
                      glFont *font );

/* Input functions. */
void inp_setEmptyText( unsigned int wid, const char *name, const char *str );

/* Misc functions. */
const char *window_getInput( unsigned int wid, const char *name );
const char *window_setInput( unsigned int wid, const char *name,
                             const char *msg );
void        window_setInputFilter( unsigned int wid, const char *name,
                                   const char *filter );
void        window_setInputCallback( unsigned int wid, const char *name,
                                     void ( *fptr )( unsigned int, const char        *) );

/* Filter constants. */
#define INPUT_FILTER_NUMBER                                                    \
   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/"            \
   "\\'\"~<>!@#$%^&|_`"
/* For inputting resolution, we want numbers and 'x'. */
#define INPUT_FILTER_RESOLUTION                                                \
   "abcdefghijklmnopqrstuvwyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{}()-=*/"             \
   "\\'\"~<>!@#$%^&|_`"
