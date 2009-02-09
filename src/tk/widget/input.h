/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef INPUT_H
#  define INPUT_H


/**
 * @brief The input widget data.
 */
typedef struct WidgetInputData_ { /* WIDGET_INPUT */
   char *input; /**< Input buffer. */
   int max; /**< Maximum length. */
   int oneline; /**< Is it a one-liner? no '\n' and friends */
   int view; /**< View position. */
   int pos; /**< Cursor position. */
} WidgetInputData;


/* Required functions. */
void window_addInput( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const int max, const int oneline );

/* Misc functions. */
char* window_getInput( const unsigned int wid, char* name );


#endif /* INPUT_H */

