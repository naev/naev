

#ifndef CONF_H
#  define CONF_H


void conf_setDefaults (void);
int conf_loadConfig( const char* file );
void conf_parseCLI( int argc, char** argv );


#endif /* CONF_H */
