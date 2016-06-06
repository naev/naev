/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef JOYSTICK_H
#  define JOYSTICK_H


/*
 * gets the joystick index number based on its name
 */
int joystick_get( const char* namjoystick );

/*
 * sets the game to use the joystick of index indjoystick
 */
int joystick_use( int indjoystick );

/*
 * init/exit functions
 */
int joystick_init (void);
void joystick_exit (void);


#endif /* JOYSTICK_H */
