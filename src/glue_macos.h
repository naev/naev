/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stddef.h>
/** @endcond */

int macos_isBundle ( void );
int macos_resourcesPath ( char *res, size_t n );

int macos_configPath ( char *res, size_t n );
int macos_cachePath ( char *res, size_t n );
