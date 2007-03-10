


#ifndef LOG_H
#   define LOG_H


void printlog( const char *fmt, ... );
void printerr( const int errnum, const char *fmt, ... );
void printwarn( const char *fmt, ... );
void printdebug( const char *fmt, ... );


#endif /* LOG_H */
