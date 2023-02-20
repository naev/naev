/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file debug_fpu.c
 *
 * @brief Implements the debug_enableFPUExcept function where supported (GNU systems).
 * This is separated into its own file because defining _GNU_SOURCE -- the only way
 * to get access to feenableexcept() -- does frightening things.
 */

/** @cond */
#if HAVE_FENV_H && DEBUGGING
/* This module uses GNU extensions to enable FPU exceptions. */
#define _GNU_SOURCE
#include <fenv.h>
#endif /* HAVE_FENV_H && DEBUGGING */
/** @endcond */

#include "debug.h"

/**
 * @brief Enables FPU exceptions. Artificially limited to Linux until link issues are figured out.
 */
void debug_enableFPUExcept (void)
{
#if HAVE_FEENABLEEXCEPT && DEBUGGING
   feenableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
#endif /* HAVE_FEENABLEEXCEPT && DEBUGGING */
}

/**
 * @brief Disables FPU exceptions.
 */
void debug_disableFPUExcept (void)
{
#if HAVE_FEENABLEEXCEPT && DEBUGGING
   fedisableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
#endif /* HAVE_FEENABLEEXCEPT && DEBUGGING */
}
