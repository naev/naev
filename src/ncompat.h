/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/* Standard specific. */
/**
 * @brief Whether or not the system follows unix standards like $HOME.
 *
 * @note macOS does not define these macros, but does follow unix somewhat.
 */
#if (defined(__unix__) || defined(__unix) || __MACOS__)
#define HAS_UNIX 1
#else
#define HAS_UNIX 0
#endif
/**
 * @brief Whether or not the system is compliant to POSIX.1.
 *
 * @note Most systems don't actually follow it fully so they don't declare that
 *       they support it.  We consider unix good enough.
 */
#if HAS_UNIX
#define HAS_POSIX 1
#else
#define HAS_POSIX 0
#endif
