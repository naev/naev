/*
 * See Licensing and Copyright notice in naev.h
 */

#pragma once

#ifndef __has_attribute
   #define __has_attribute( x ) 0
#endif

// Nullability
#if __has_attribute( nonnull )
   #define NONNULL( ... ) __attribute__( ( nonnull( __VA_ARGS__ ) ) )
#else
   #define NONNULL( ... )
#endif

#if __has_attribute( returns_nonnull )
   #define RETURNS_NONNULL __attribute__( ( returns_nonnull ) )
#else
   #define RETURNS_NONNULL
#endif

// Function attributes
#if __has_attribute( sentinel )
   #define SENTINEL( n ) __attribute__( ( sentinel( n ) ) )
#else
   #define SENTINEL( n )
#endif

#if __has_attribute( noreturn )
   #define NORETURN __attribute__( ( noreturn ) )
#else
   #define NORETURN
#endif

// User defined diagnosis
#if __has_attribute( diagnose_if )
   #define WARN_IF( c, m ) __attribute__( ( diagnose_if( c, m, "warning" ) ) )
   #define ERR_IF( c, m )  __attribute__( ( diagnose_if( c, m, "error" ) ) )
#else
   #define WARN_IF( c, m )
   #define ERR_IF( c, m )
#endif

// Statement attributes
#if __has_attribute( fallthrough )
   #define FALLTHROUGH __attribute__( ( fallthrough ) )
#else
   #define FALLTHROUGH (void)0
#endif