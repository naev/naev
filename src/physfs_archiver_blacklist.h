/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

int  blacklist_append( const char *path );
int  whitelist_append( const char *path );
int  blacklist_init( void );
void blacklist_exit( void );
