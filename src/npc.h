/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NPC_H
#  define NPC_H


#include "opengl.h"
#include "mission.h"


/*
 * Adding.
 */
unsigned int npc_add_mission( Mission *misn, const char *func, const char *name,
      int priority, const char *portrait, const char *desc );
unsigned int npc_add_event( unsigned int evt, const char *func, const char *name,
      int priority, const char *portrait, const char *desc );

/*
 * Removing.
 */
int npc_rm_event( unsigned int id, unsigned int evt );
int npc_rm_mission( unsigned int id, Mission *misn );
int npc_rm_parentEvent( unsigned int id );
int npc_rm_parentMission( Mission *misn );

/*
 * Control.
 */
void npc_sort (void);
void npc_generate (void);
void npc_patchMission( Mission *misn );
void npc_clear (void);
void npc_freeAll (void);

/*
 * Land image array stuff.
 */
int npc_getArraySize (void);
int npc_getNameArray( char **names, int n );
int npc_getTextureArray( glTexture **tex, int n );
const char *npc_getName( int i );
glTexture *npc_getTexture( int i );
const char *npc_getDesc( int i );
int npc_approach( int i );


#endif /* NPC_H */
