/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef WGT_LIST_H
#  define WGT_LIST_H


/**
 * @brief The list widget data.
 */
typedef struct WidgetListData_ {
   char **options; /**< Pointer to the options. */
   int noptions; /**< Total number of options. */
   int selected; /**< Which option is currently selected. */
   int pos; /** Current topmost option (in view). */
   void (*onSelect) (unsigned int,char*); /**< Modify callback - triggered on selection. */
   void (*onActivate) (unsigned int,char*); /**< Activate callback - triggered on double-click. */
   int height; /**< Real height. */
} WidgetListData;


/* Required functions. */
void window_addList( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, char **items, int nitems, int defitem,
      void (*onSelect) (unsigned int, char*),
      void (*onActivate) (unsigned int, char*) );

/* Misc functions. */
char* toolkit_getList( const unsigned int wid, const char* name );
int toolkit_getListPos( const unsigned int wid, const char* name );
char* toolkit_setList( const unsigned int wid, const char* name, char* value );
char* toolkit_setListPos( const unsigned int wid, const char* name, int pos );
int toolkit_getListOffset( const unsigned int wid, const char* name );
int toolkit_setListOffset( const unsigned int wid, const char* name, int off );


#endif /* WGT_LIST_H */

