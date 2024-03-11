/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdint.h>
#include <stddef.h>
/** @endcond */

typedef struct msgcat {
   const void *map;             /**< .mo file contents, which we'd mmap() but for PhysicsFS. */
   size_t map_size;             /**< .mo file size. */
   const char *plural_rule;     /**< .mo "Plural-Forms" expression (RHS of "plural="), used by ngettext. */
   uint64_t nplurals;           /**< .mo "Plural-Forms" expression (RHS of "nplurals="), used by ngettext. */
} msgcat_t;

void msgcat_init( msgcat_t* p, const void* map, size_t map_size );
const char* msgcat_ngettext( const msgcat_t* p, const char* msgid1, const char* msgid2, uint64_t n );
uint32_t msgcat_nstringsFromHeader( const char buf[12] );
