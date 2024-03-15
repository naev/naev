/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

int  cond_init( void );
void cond_exit( void );
int  cond_compile( const char *cond );
int  cond_check( const char *cond );
int  cond_checkChunk( int chunk, const char *cond );
