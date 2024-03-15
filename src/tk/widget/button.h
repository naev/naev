/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"

/**
 * @brief The button widget.
 */
typedef struct WidgetButtonData_ {
   void ( *fptr )( unsigned int, const char * ); /**< Activate callback. */
   char *display;                                /**< Displayed text. */
   int   disabled;    /**< 1 if button is disabled, 0 if enabled. */
   int   softdisable; /**< Whether the function should still run if disabled. */
   SDL_Keycode key;   /**< Keybinding key. */
   void ( *cst_render )( double x, double y, double w, double h,
                         const glColour *c ); /**< Custom render function. */
} WidgetButtonData;

/* Required functions. */
void window_addButtonKey(
   unsigned int wid, const int x, const int y, /* position */
   const int w, const int h,                   /* size */
   const char *name, const char *display,      /* label name, display name */
   void ( *call )( unsigned int,
                   const char * ), /* function to call when clicked */
   SDL_Keycode
      key ); /* Hotkey for using the button without it being focused. */

void window_addButton(
   unsigned int wid, const int x, const int y, /* position */
   const int w, const int h,                   /* size */
   const char *name, const char *display,      /* label name, display name */
   void ( *call )( unsigned int,
                   const char * ) ); /* function to call when clicked */

/* Misc functions. */
void window_disableButton( unsigned int wid, const char *name );
void window_disableButtonSoft( unsigned int wid, const char *name );
void window_enableButton( unsigned int wid, const char *name );
void window_buttonCaption( unsigned int wid, const char *name,
                           const char *display );
void window_buttonCustomRender( unsigned int wid, const char *name,
                                void ( *func )( double, double, double, double,
                                                const glColour * ) );
void window_buttonCustomRenderGear( double x, double y, double w, double h,
                                    const glColour *c );
