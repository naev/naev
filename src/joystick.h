

#ifndef JOYSTICK_H
#  define JOYSTICK_H


/*
 * gets the joystick index number based on its name
 */
int joystick_get( char* namjoystick );

/*
 * sets the game to use the joystick of index indjoystick
 */
int joystick_use( int indjoystick );

/*
 * init/exit functions
 */
int joystick_init();
void joystick_exit();


#endif /* JOYSTICK_H */
