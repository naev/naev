/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct plugin_s {
   char *name;          /**< Name of the plugin. */
   char *author;        /**< Author(s) of the plugin. */
   char *version;       /**< Version of the plugin. */
   char *description;   /**< Decscription of the plugin. */
   char *compatibility; /**< Compatibility with Naev versions. */
   char *source;        /**< Source location of the plugin. */
   char *mountpoint;    /**< Where it is mounted. */
   int   priority; /**< Loading priority of the mod. Lower is better, defaults
                      to 5. */
   int compatible; /**< Whether or not it is compatible with the current version
                      of Naev. */
   int total_conversion; /**< Whether or not it is a total conversion. */
} plugin_t;

int             plugin_init( void );
void            plugin_exit( void );
int             plugin_check( void );
const plugin_t *plugin_list( void );
const char     *plugin_name( const plugin_t *plg );
