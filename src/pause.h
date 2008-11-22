/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef PAUSE_H
#  define PAUSE_H


extern int paused;
extern double dt_mod;

void pause_setSpeed( double mod );

void pause_game (void);
void unpause_game (void);

void pause_delay( unsigned int delay );


#endif /* PAUSE_H */
