/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NPC_H
#  define NPC_H


/*
 * Adding.
 */
unsigned int npc_add_mission( char *misn, char *func, char *name,
      int priority, char *portrait );
unsigned int npc_add_event( char *evt, char *func, char *name,
      int priority, char *portrait );

/*
 * Removing.
 */
int npc_rm_event( unsigned int id, const char *evt );
int npc_rm_mission( unsigned int id, const char *misn );

/*
 * Control.
 */
void npc_generate (void);
void npc_clear (void);
void npc_freeAll (void);



#endif /* NPC_H */
