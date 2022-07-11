/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct plugin_s {
   char *name;          /**< Name of the mod. */
   char *author;        /**< Author(s) of the mod. */
   char *version;       /**< Version of the mod. */
   char *compatibility; /**< Compatibility with Naev versions. */
   char *mountpoint;    /**< Where it is mounted. */
} plugin_t;

int plugin_init (void);
const plugin_t *plugin_list (void);
