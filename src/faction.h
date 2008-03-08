/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef FACTION_H
#  define FACTION_H


#define FACTION_PLAYER  0


/* get stuff */
int faction_get( const char* name );
int faction_getAlliance( char* name );
char* faction_name( int f );

/* player stuff */
void faction_modPlayer( int f, int mod );

/* works with only factions */
int areEnemies( int a, int b );
int areAllies( int a, int b );

/* faction + alliance */
int faction_ofAlliance( int f, int a );

/* check */
int faction_isAlliance( int a );
int faction_isFaction( int f );

/* load/free */
int factions_load (void);
void factions_free (void);


#endif /* FACTION_H */
