/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/**
 * @brief The checkbox widget.
 */
typedef struct WidgetCheckboxData_ {
   void ( *fptr )( unsigned int, const char * ); /**< Toggle callback. */
   char *display;                                /**< Displayed text. */
   int   state;    /**< Current checkbox status. */
   int   disabled; /**< The widget is currently disabled. */
} WidgetCheckboxData;

/* Required functions. */
void window_addCheckbox(
   unsigned int wid, const int x, const int y,   /* position */
   const int w, const int h,                     /* size */
   const char *name, const char *display,        /* label name, display name */
   void ( *call )( unsigned int, const char * ), /* toggle function */
   int default_state );                          /* default state. */

/* Misc functions. */
void window_checkboxCaption( unsigned int wid, const char *name,
                             char *display );
int  window_checkboxState( unsigned int wid, const char *name );
int  window_checkboxSet( unsigned int wid, const char *name, int state );
int  window_enableCheckbox( unsigned int wid, const char *name );
int  window_disableCheckbox( unsigned int wid, const char *name );
