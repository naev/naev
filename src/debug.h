/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

void debug_sigInit (void);
void debug_sigClose (void);
void debug_enableFPUExcept (void);
void debug_disableFPUExcept (void);
void debug_enableLeakSanitizer (void);

enum {
   DEBUG_MARK_EMITTER,  /**< Mark the trail emitters with a cross. */
   /* Sentinel. */
   DEBUG_FLAGS_MAX      /**< Maximum number of flags. */
};

#if DEBUGGING
#define debug_isFlag(f)     (debug_flags[f])     /**< Checks if flag f is set. */
#define debug_setFlag(f)    (debug_flags[f] = 1) /**< Sets flag f. */
#define debug_rmFlag(f)     (debug_flags[f] = 0) /**< Removes flag f. */

typedef char DebugFlags[ DEBUG_FLAGS_MAX ];

/* Initialize debugging flags. */
extern DebugFlags debug_flags;

void debug_logBacktrace (void);
#else /* DEBUGGING */
#define debug_isFlag(f)     0                    /**< Checks if flag f is set. */
#define debug_rmFlag(f)     do {} while(0)       /**< Removes flag f. */
#endif /* DEBUGGING */
