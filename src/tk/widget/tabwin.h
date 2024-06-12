/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "font.h"

/**
 * @brief The button widget.
 */
typedef struct WidgetTabbedWindowData_ {
   int           ntabs;         /**< Number of tabs. */
   char        **tabnames;      /**< Names of the tabs. */
   unsigned int *windows;       /**< Window IDs. */
   unsigned int  parent_window; /**< Parent window Id. */
   int           active;        /**< Currently active window. */
   int           tabpos;        /**< Where are the tabs placed?
                                     0=bottom, 1=top */
   const glFont *font;          /**< Font to use. */

   /* Internal usage. */
   int *namelen;
   void ( *onChange )( unsigned int, const char *, int, int );
} WidgetTabbedWindowData;

/* Required functions. */
unsigned int *window_addTabbedWindow( unsigned int wid, const int x,
                                      const int y,              /* position */
                                      const int w, const int h, /* size */
                                      const char *name, int ntabs,
                                      const char **tabnames, int tabpos );

int window_tabWinSetActive( unsigned int wid, const char *tab, int active );
int window_tabWinGetActive( unsigned int wid, const char *tab );
int window_tabWinOnChange( unsigned int wid, const char *tab,
                           void ( *onChange )( unsigned int, const char *, int,
                                               int ) );
int window_tabSetFont( unsigned int wid, const char *tab, const glFont *font );
unsigned int *window_tabWinGet( unsigned int wid, const char *tab );
int           window_tabWinGetBarWidth( unsigned int wid, const char *tab );
int window_tabWinSetTabName( unsigned int wid, const char *tab, int id,
                             const char *name );
