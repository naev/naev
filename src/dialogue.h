/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DIALOGUE_H
#  define DIALOGUE_H


#include "attributes.h"


/*
 * popups and alerts
 */
FORMAT( printf, 1, 2 ) void dialogue_alert( const char *fmt, ... ); /* does not pause execution */
FORMAT( printf, 2, 3 ) void dialogue_msg( const char *caption, const char *fmt, ... );
void dialogue_msgRaw( const char *caption, const char *msg );
FORMAT( printf, 3, 4 ) void dialogue_msgImg( const char *caption, const char *img, const char *fmt, ... );
void dialogue_msgImgRaw( const char *caption, const char *msg, const char *img, int width, int height );
FORMAT( printf, 2, 3 ) int dialogue_YesNo( const char *caption, const char *fmt, ... ); /* Yes = 1, No = 0 */
int dialogue_YesNoRaw( const char *caption, const char *msg );
void dialogue_makeChoice( const char *caption, const char *msg, int opts );
void dialogue_addChoice( const char *caption, const char *msg, const char *opt );
char *dialogue_runChoice (void);
FORMAT( printf, 4, 5 ) char* dialogue_input( const char* title, int min, int max, const char *fmt, ... );
char* dialogue_inputRaw( const char* title, int min, int max, const char *msg  );
FORMAT( printf, 4, 5 ) int dialogue_list( const char* title, char **items, int nitems, const char *fmt, ... );
int dialogue_listRaw( const char* title, char **items, int nitems, const char *msg );
FORMAT( printf, 8, 9 ) int dialogue_listPanel ( const char* title, char **items, int nitems, int extrawidth,
      int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
      void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
      const char *fmt, ... );
int dialogue_listPanelRaw( const char* title, char **items, int nitems, int extrawidth,
      int minheight, void (*add_widgets) (unsigned int wid, int x, int y, int w, int h),
      void (*select_call) (unsigned int wid, char* wgtname, int x, int y, int w, int h),
      const char *msg );

/*
 * misc
 */
int dialogue_isOpen (void);


#endif /* DIALOGUE_H */

