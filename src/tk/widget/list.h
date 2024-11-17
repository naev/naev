/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/**
 * @brief The list widget data.
 */
typedef struct WidgetListData_ {
   char **options;  /**< Pointer to the options. */
   int    noptions; /**< Total number of options. */
   int    selected; /**< Which option is currently selected. */
   int    pos;      /** Current topmost option (in view). */
   void ( *onSelect )(
      unsigned int,
      const char * ); /**< Modify callback - triggered on selection. */
   void ( *onActivate )(
      unsigned int,
      const char * ); /**< Activate callback - triggered on double-click. */
   int height;        /**< Real height. */
   /* Alt text stuff. */
   int    alt;     /**< Alt text. */
   int    altx;    /**< X position of alt text. */
   int    alty;    /**< Y position of alt text. */
   char **alttext; /**< Alt text to display when hovering over options. */
} WidgetListData;

/* Required functions. */
void window_addList( unsigned int wid, const int x, const int y, /* position */
                     const int w, const int h,                   /* size */
                     const char *name, char **items, int nitems, int defitem,
                     void ( *onSelect )( unsigned int, const char * ),
                     void ( *onActivate )( unsigned int, const char * ) );

/* Misc functions. */
const char *toolkit_getList( unsigned int wid, const char *name );
int         toolkit_getListPos( unsigned int wid, const char *name );
const char *toolkit_setList( unsigned int wid, const char *name,
                             const char *value );
const char *toolkit_setListPos( unsigned int wid, const char *name, int pos );
int         toolkit_getListOffset( unsigned int wid, const char *name );
int toolkit_setListOffset( unsigned int wid, const char *name, int off );
int toolkit_setListAltText( unsigned int wid, const char *name,
                            char **alttext );
