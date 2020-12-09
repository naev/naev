/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DIALOGUE_H
#  define DIALOGUE_H


#include "nstring.h"


/*
 * popups and alerts
 */
/* Alert. */
PRINTF_FORMAT( 1, 2 ) void dialogue_alert( const char *fmt, ... ); /* does not pause execution */
/* Message. */
PRINTF_FORMAT( 2, 3 ) void dialogue_msg( const char *caption, const char *fmt, ... );
void dialogue_msgRaw( const char *caption, const char *msg );
/* Image. */
PRINTF_FORMAT( 3, 4 ) void dialogue_msgImg( const char *caption, const char *img, const char *fmt, ... );
void dialogue_msgImgRaw( const char *caption, const char *msg, const char *img, int width, int height );
/* YesNo. */
PRINTF_FORMAT( 2, 3 ) int dialogue_YesNo( const char *caption, const char *fmt, ... ); /* Yes = 1, No = 0 */
int dialogue_YesNoRaw( const char *caption, const char *msg );
/* Input. */
PRINTF_FORMAT( 4, 5 ) char* dialogue_input( const char* title, int min, int max, const char *fmt, ... );
char* dialogue_inputRaw( const char* title, int min, int max, const char *msg  );

/*
 * Choice dialogues.
 */
void dialogue_makeChoice( const char *caption, const char *msg, int opts );
void dialogue_addChoice( const char *caption, const char *msg, const char *opt );
char *dialogue_runChoice (void);

/*
 * Lists.
 */
PRINTF_FORMAT( 4, 5 ) int dialogue_list( const char* title, char **items, int nitems, const char *fmt, ... );
int dialogue_listRaw( const char* title, char **items, int nitems, const char *msg );
PRINTF_FORMAT( 8, 9 ) int dialogue_listPanel ( const char* title, char **items, int nitems, int extrawidth,
      int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
      void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
      const char *fmt, ... );
int dialogue_listPanelRaw( const char* title, char **items, int nitems, int extrawidth,
      int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
      void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
      const char *msg );

/*
 * Custom.
 */
void dialogue_custom( const char* caption, int width, int height,
      int (*update) (double dt, void* data),
      void (*render) (double x, double y, double w, double h, void* data),
      int (*keyboard) (unsigned int wid, SDL_Keycode key, SDL_Keymod mod, void* data),
      int (*mouse) (unsigned int wid, SDL_Event* event, double x, double y, double w, double h, double rx, double ry, void* data),
      void *data );


/*
 * misc
 */
int dialogue_isOpen (void);


#endif /* DIALOGUE_H */

