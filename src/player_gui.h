/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PLAYER_GUI_H
#  define PLAYER_GUI_H


/* Clean up. */
void player_guiCleanup (void);

/* Manipulation. */
int player_guiAdd( char* name );
void player_guiRm( char* name );
int player_guiCheck( char* name );

/* High level. */
char** player_guiList (void);


#endif /* PLAYER_GUI_H */

