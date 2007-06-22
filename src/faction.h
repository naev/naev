

#ifndef FACTION_H
#  define FACTION_H


typedef struct Faction {

	char* name;

	struct Faction** enemies;
	struct Faction** allies;

} Faction;


Faction* get_faction( const char* name );

int areEnemeis( Faction* a, Faction* b );
int areAllies( Faction* a, Faction* b );

int factions_load (void);
void factions_free (void);


#endif /* FACTION_H */
