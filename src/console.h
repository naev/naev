/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef CONSOLE_H
#  define CONSOLE_H


/*
 * Init/exit.
 */
int cli_init (void);
void cli_exit (void);

/*
 * Misc.
 */
void cli_open (void);
void cli_addMessage( const char *msg );
void cli_addMessageMax( const char *msg, const int l );


#endif /* CONSOLE_H */


