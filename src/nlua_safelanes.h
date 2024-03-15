/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

#define SAFELANES_METATABLE "safelanes" /**< Module's metatable identifier. */

/* Library loading */
int nlua_loadSafelanes( nlua_env env );
