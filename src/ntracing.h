/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#if HAVE_TRACY
#include "tracy/TracyC.h"
#define _uninitialized_var(x) x = *(&(x))
#  define NTracingFrameMark            TracyCFrameMark
#  define NTracingFrameMarkStart( name ) ___tracy_emit_frame_mark_start( name )
#  define NTracingFrameMarkEnd( name ) ___tracy_emit_frame_mark_end( name )
#  define NTracingZone( ctx, active )  TracyCZone( ctx, active )
#  define NTracingZoneEnd( ctx )       TracyCZoneEnd( ctx )
#  define NTracingAlloc( ptr, size )   do { _uninitialized_var(ptr); TracyCAlloc( ptr, size ) } while (0)
#  define NTracingFree( ptr )          do { if (ptr!=NULL) { TracyCFree( ptr ); }; } while (0)
#else
#  define NTracingFrameMark
#  define NTracingFrameMarkStart( name )
#  define NTracingFrameMarkEnd( name )
#  define NTracingZone( ctx, active )
#  define NTracingZoneEnd( ctx )
#  define NTracingAlloc( ptr, size )
#  define NTracingFree( ptr )
#endif /* HAVE_TRACY */
