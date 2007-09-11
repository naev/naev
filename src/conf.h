/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef CONF_H
#  define CONF_H


/*
 * loading
 */
void conf_setDefaults (void);
int conf_loadConfig( const char* file );
void conf_parseCLI( int argc, char** argv );

/*
 * saving
 */
int conf_saveConfig (void);


#endif /* CONF_H */
