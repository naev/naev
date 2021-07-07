/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef DEBUG_H
#  define DEBUG_H


void debug_sigInit (void);
void debug_sigClose (void);
void debug_enableFPUExcept (void);


#ifdef DEBUGGING
#define debug_isFlag(f)     (debug_flags[f])     /**< Checks if flag f is set. */
#define debug_setFlag(f)    (debug_flags[f] = 1) /**< Sets flag f. */
#define debug_rmFlag(f)     (debug_flags[f] = 0) /**< Removes flag f. */

enum {
   DEBUG_MARK_EMITTER,        /**< Mark the trail emitters with a cross. */

   /* Sentinel. */
   DEBUG_FLAGS_MAX      /**< Maximum number of flags. */
};
typedef char DebugFlags[ DEBUG_FLAGS_MAX ];

/* Initialize debugging flags. */
extern DebugFlags debug_flags;
#endif /* DEBUGGING */


#endif /* DEBUG_H */

