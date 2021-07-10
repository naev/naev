/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef NLUA_SAFELANES_H
#  define NLUA_SAFELANES_H

#include "nlua.h"

#define SAFELANES_METATABLE      "safelanes" /**< Module's metatable identifier. */

/* Library loading */
int nlua_loadSafelanes( nlua_env env );

#endif /* NLUA_SAFELANES_H */
