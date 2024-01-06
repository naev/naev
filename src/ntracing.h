/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#ifdef HAVE_TRACY
#include "tracy/TracyC.h"
#  define NTracingFrameMark            TracyCFrameMark
#  define NTracingFrameMarkStart( name ) ___tracy_emit_frame_mark_start( name )
#  define NTracingFrameMarkEnd( name ) ___tracy_emit_frame_mark_end( name )
#  define NTracingZone( ctx, active )  TracyCZone( ctx, active )
#  define NTracingZoneEnd( ctx )       TracyCZoneEnd( ctx )
#else
#  define NTracingFrameMark
#  define NTracingFrameMarkStart( name )
#  define NTracingFrameMarkEnd( name )
#  define NTracingZone( ctx, active )
#  define NTracingZoneEnd( ctx )
#endif /* HAVE_TRACY */
