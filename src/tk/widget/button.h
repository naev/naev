/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_BUTTON_H
#  define WGT_BUTTON_H


/**
 * @brief The button widget.
 */
typedef struct WidgetButtonData_ {
   void (*fptr) (unsigned int,char*); /**< Activate callback. */
   char *display; /**< Displayed text. */
   int disabled; /**< 1 if button is disabled, 0 if enabled. */
   SDLKey key;
} WidgetButtonData;


/* Required functions. */
void window_addButtonKey( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, char* display, /* label name, display name */
      void (*call) (unsigned int,char*), /* function to call when clicked */
      SDLKey key ); /* Hotkey for using the button without it being focused. */

void window_addButton( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, char* display, /* label name, display name */
      void (*call) (unsigned int,char*) ); /* function to call when clicked */

/* Misc functions. */
void window_disableButton( const unsigned int wid, char* name );
void window_enableButton( const unsigned int wid, char *name );
void window_buttonCaption( const unsigned int wid, char *name, char *display );


#endif /* WGT_BUTTON_H */

