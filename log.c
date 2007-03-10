
#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void printlog( const char *fmt, ... )
{
	va_list ap;

	fprintf( stdout, " " );
	va_start( ap, fmt );
	vfprintf( stdout, fmt, ap );
	va_end( ap );
	fprintf( stdout, "\n" );
}

void printerr( const int errnum, const char *fmt, ... )
{       
	va_list ap;

	fprintf( stderr, "ERROR: " );                             
	va_start( ap, fmt );
	vfprintf( stderr, fmt, ap );
	va_end( ap );
	fprintf( stderr, "\n" );

	exit( errnum );
}

void printwarn( const char *fmt, ... )
{  
	va_list ap;

	fprintf( stderr, "WARNING: " );
	va_start( ap, fmt );
	vfprintf( stderr, fmt, ap );
	va_end( ap );
	fprintf( stderr, "\n" );
}

void printdebug( const char *fmt, ... )
{
#ifdef DEBUG
	va_list ap;

	fprintf( stderr, "DEBUG: " );
	va_start( ap, fmt );
	vfprintf( stderr, fmt, ap );
	va_end( ap );
	fprintf( stderr, "\n" );
#endif /* DEBUG */
}

