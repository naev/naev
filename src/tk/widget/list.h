/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LIST_H
#  define LIST_H


/**
 * @brief The list widget data.
 */
typedef struct WidgetListData_ { /* WIDGET_LIST */
   char **options; /**< Pointer to the options. */
   int noptions; /**< Total number of options. */
   int selected; /**< Which option is currently selected. */
   int pos; /** Current topmost option (in view). */
   void (*fptr) (unsigned int,char*); /**< Modify callback - triggered on selection. */
   int height; /**< Real height. */
} WidgetListData;


/* Required functions. */
void window_addList( const unsigned int wid,
      const int x, const int y, /* position */
      const int w, const int h, /* size */
      char* name, char **items, int nitems, int defitem,            
      void (*call) (unsigned int,char*) );

/* Misc functions. */
char* toolkit_getList( const unsigned int wid, char* name );
int toolkit_getListPos( const unsigned int wid, char* name );


#endif /* LIST_H */

