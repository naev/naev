

#ifndef PLAYER_H
#  define PLAYER_H


#include "SDL.h"


/*
 * flags
 */
#define PLAYER_FLAG_NULL		(1<<0)
#define PLAYER_FLAG_MOV_LEFT	(1<<1)
#define PLAYER_FLAG_MOV_RIGHT	(1<<2)
#define PLAYER_FLAG_MOV_ACC	(1<<3)
int player_isFlag( unsigned int flag );
void player_setFlag( unsigned int flag );
void player_rmFlag( unsigned int flag );


/*
 * input
 */
void handle_input( SDL_Event* event );



#endif /* PLAYER_H */
