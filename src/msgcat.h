/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef PLURALFORM_H
#define PLURALFORM_H

/** @cond */
#include <stdint.h>
/** @endcond */

typedef struct msgcat {
   const void *map;             /**< .mo file contents, which we'd mmap() but for PhysicsFS. */
   size_t map_size;             /**< .mo file size. */
   const char *plural_rule;     /**< .mo "Plural-Forms" expression (RHS of "plural="), used by ngettext. */
   int nplurals;                /**< .mo "Plural-Forms" expression (RHS of "nplurals="), used by ngettext. */
} msgcat_t;

void msgcat_init( msgcat_t* p, const void* map, size_t map_size );
const char* msgcat_ngettext( const msgcat_t* p, const char* msgid1, const char* msgid2, uint64_t n );

#endif
