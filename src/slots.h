/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "opengl_tex.h"

/* Load/exit. */
int  sp_load( void );
void sp_cleanup( void );

/* Stuff. */
unsigned int     sp_get( const char *name );
const char      *sp_display( unsigned int sp );
const char      *sp_description( unsigned int sp );
int              sp_visible( unsigned int spid );
int              sp_required( unsigned int spid );
int              sp_exclusive( unsigned int spid );
int              sp_locked( unsigned int spid );
const glTexture *sp_icon( unsigned int spid );
