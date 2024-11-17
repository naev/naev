/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

/* individual library stuff */
int nlua_loadHook( nlua_env env );

/* Misc. */
int  hookL_getarg( unsigned int hook );
void hookL_unsetarg( unsigned int hook );
