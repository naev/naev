/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DIALOGUE_H
#  define DIALOGUE_H


/*
 * popups and alerts
 */
void dialogue_alert( const char *fmt, ... ); /* does not pause execution */
void dialogue_msg( const char *caption, const char *fmt, ... );
void dialogue_msgRaw( const char *caption, const char *msg );
int dialogue_YesNo( const char *caption, const char *fmt, ... ); /* Yes = 1, No = 0 */
int dialogue_YesNoRaw( const char *caption, const char *msg );
void dialogue_makeChoice( const char *caption, const char *msg, int opts );
void dialogue_addChoice( const char *caption, const char *msg, const char *opt );
char *dialogue_runChoice (void);
char* dialogue_input( const char* title, int min, int max, const char *fmt, ... );
char* dialogue_inputRaw( const char* title, int min, int max, const char *msg  );
int dialogue_list( const char* title, char **items, int nitems, const char *fmt, ... );
int dialogue_listRaw( const char* title, char **items, int nitems, const char *msg );

/*
 * misc
 */
int dialogue_isOpen (void);


#endif /* DIALOGUE_H */

