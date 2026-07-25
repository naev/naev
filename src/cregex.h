/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct CRegex CRegex;

CRegex *cregex_new( const char *str );
int     cregex_is_match( const CRegex *regex, const char *text );
void    cregex_free( CRegex *regex );
