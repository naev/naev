/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "mission.h"
#include "opengl.h"

/*
 * Adding.
 */
unsigned int npc_add_mission( unsigned int mid, const char *func,
                              const char *name, int priority,
                              glTexture *portrait, const char *desc,
                              glTexture *background );
unsigned int npc_add_event( unsigned int evt, const char *func,
                            const char *name, int priority, glTexture *portrait,
                            const char *desc, glTexture *background );

/*
 * Removing.
 */
int npc_rm_event( unsigned int id, unsigned int evt );
int npc_rm_mission( unsigned int id, unsigned int mid );
int npc_rm_parentEvent( unsigned int id );
int npc_rm_parentMission( unsigned int mid );

/*
 * Control.
 */
void npc_sort( void );
void npc_generateMissions( void );
void npc_patchMission( Mission *misn );
void npc_clear( void );

/*
 * Land image array stuff.
 */
int         npc_getArraySize( void );
const char *npc_getName( int i );
glTexture  *npc_getBackground( int i );
glTexture  *npc_getTexture( int i );
const char *npc_getDesc( int i );
int         npc_isImportant( int i );
int         npc_approach( int i );
