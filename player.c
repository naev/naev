

#include "player.h"

#include "all.h"
#include "pilot.h"
#include "log.h"


static unsigned int player_flags = PLAYER_FLAG_NULL;


/*
 * used in pilot.c
 */
void player_think( Pilot* player, const FP dt )
{
	player->solid->dir_vel = 0.;
	if (player_isFlag(PLAYER_FLAG_MOV_LEFT))
		player->solid->dir_vel += player->ship->turn;
	if (player_isFlag(PLAYER_FLAG_MOV_RIGHT))
		player->solid->dir_vel -= player->ship->turn;

	player->solid->force = (player_isFlag(PLAYER_FLAG_MOV_ACC))?player->ship->thrust:0.;
}


/*
 * flag manipulation
 */
int player_isFlag( unsigned int flag )
{
	return player_flags & flag;
}
void player_setFlag( unsigned int flag )
{
	if (!player_isFlag(flag))
		player_flags |= flag;
}
void player_rmFlag( unsigned int flag )
{
	if (player_isFlag(flag))
		player_flags ^= flag;
}


