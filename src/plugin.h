/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct plugin_t {
   char *name;             /**< Name of the plugin. */
   char *author;           /**< Author(s) of the plugin. */
   char *version;          /**< Version of the plugin. */
   char *description;      /**< Decscription of the plugin. */
   char *mountpoint;       /**< Where it is mounted. */
   int   total_conversion; /**< Whether or not it is a total conversion. */
   int   compatible;
   int   priority;
} plugin_t;

/* Plugin subsystem. */
const char     *plugin_dir( void );
int             plugin_check( void );
const plugin_t *plugin_list( void );
const char     *plugin_name( const plugin_t *plg );
