/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef GLUE_MACOS_H
#  define GLUE_MACOS_H


/** @cond */
#include <stddef.h>
/** @endcond */


int macos_isBundle ();
int macos_resourcesPath ( char *res, size_t n );

int macos_configPath ( char *res, size_t n );
int macos_cachePath ( char *res, size_t n );


#endif /* GLUE_MACOS_H */
