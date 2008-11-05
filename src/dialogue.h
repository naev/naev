/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef DIALOGUE_H
#  define DIALOGUE_H


/*
 * popups and alerts
 */
void dialogue_alert( const char *fmt, ... ); /* does not pause execution */
void dialogue_msg( char *caption, const char *fmt, ... );
int dialogue_YesNo( char *caption, const char *fmt, ... ); /* Yes = 1, No = 0 */
char* dialogue_input( char* title, int min, int max, const char *fmt, ... );

/*
 * misc
 */
int dialogue_isOpen (void);


#endif /* DIALOGUE_H */

