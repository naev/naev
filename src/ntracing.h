/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#ifdef TRACY_ENABLE

#include "tracy/TracyC.h"

#define NTracingFrameMark TracyCFrameMark
#define NTracingZone( ctx, active ) TracyCZone( ctx, active )
#define NTracingZoneEnd( ctx ) TracyCZoneEnd( ctx )

#else

#define NTracingFrameMark
#define NTracingZone( ctx, active )
#define NTracingZoneEnd( ctx )

#endif
