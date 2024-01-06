/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#if HAVE_TRACY
#include "attributes.h"
#include "tracy/TracyC.h"
#define _uninitialized_var(x) x = *(&(x))
#  define NTracingFrameMark            TracyCFrameMark
#  define NTracingFrameMarkStart( name ) ___tracy_emit_frame_mark_start( name )
#  define NTracingFrameMarkEnd( name ) ___tracy_emit_frame_mark_end( name )
#  define NTracingZone( ctx, active )  TracyCZone( ctx, active )
#  define NTracingZoneEnd( ctx )       TracyCZoneEnd( ctx )
#  define NTracingAlloc( ptr, size )   do { _uninitialized_var(ptr); TracyCAlloc( ptr, size ) } while (0)
#  define NTracingFree( ptr )          do { if (ptr!=NULL) { TracyCFree( ptr ); }; } while (0)
ALWAYS_INLINE static inline void *nmalloc(size_t size)
{
   void *ptr = malloc(size);
   NTracingAlloc( ptr, size );
   return ptr;
}
ALWAYS_INLINE static inline void nfree(void *ptr)
{
   NTracingFree( ptr );
   free( ptr );
}
ALWAYS_INLINE static inline void *ncalloc(size_t nmemb, size_t size)
{
   void *ptr = calloc( nmemb, size );
   NTracingAlloc( ptr, nmemb*size );
   return ptr;
}
ALWAYS_INLINE static inline void *nrealloc(void *ptr, size_t size)
{
   NTracingFree( ptr );
   void *newptr = realloc( ptr, size );
   NTracingAlloc( newptr, size );
   return newptr;
}
#else
#  define NTracingFrameMark
#  define NTracingFrameMarkStart( name )
#  define NTracingFrameMarkEnd( name )
#  define NTracingZone( ctx, active )
#  define NTracingZoneEnd( ctx )
#  define NTracingAlloc( ptr, size )
#  define NTracingFree( ptr )
#  define nmalloc(size)      malloc(size)
#  define ncalloc(nmemb,size) calloc(nmemb,size)
#  define nfree(ptr)         free(ptr)
#  define nrealloc(ptr,size) realloc(ptr,size)
#endif /* HAVE_TRACY */
