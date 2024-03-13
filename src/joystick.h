/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/*
 * gets the joystick index number based on its name
 */
int joystick_get( const char *namjoystick );

/*
 * sets the game to use the joystick of index indjoystick
 */
int joystick_use( int indjoystick );

/*
 * init/exit functions
 */
int  joystick_init( void );
void joystick_exit( void );
