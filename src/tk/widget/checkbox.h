/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_CHECKBOX_H
#  define WGT_CHECKBOX_H


/**
 * @brief The checkbox widget.
 */
typedef struct WidgetCheckboxData_ {
   void (*fptr) (unsigned int,char*); /**< Toggle callback. */
   char *display; /**< Displayed text. */
   int state; /**< Current checkbox status. */
} WidgetCheckboxData;


/* Required functions. */
void window_addCheckbox( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, const char* display, /* label name, display name */
      void (*call) (unsigned int,char*), /* toggle function */
      int default_state ); /* default state. */

/* Misc functions. */
void window_checkboxCaption( const unsigned int wid, const char *name, char *display );
int window_checkboxState( const unsigned int wid, const char *name );
int window_checkboxSet( const unsigned int wid, const char *name, int state );


#endif /* WGT_CHECKBOX_H */

