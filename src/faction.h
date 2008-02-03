/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef FACTION_H
#  define FACTION_H


int faction_get( const char* name );
char* faction_name( int f );

int areEnemies( int a, int b );
int areAllies( int a, int b );

int factions_load (void);
void factions_free (void);


#endif /* FACTION_H */
