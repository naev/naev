/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file debug.c
 *
 * @brief Handles low-level debugging hooks.
 */

/** @cond */
#include <assert.h>
#include <signal.h>

#if DEBUGGING
#include <backtrace.h>

#define __USE_GNU /* Grrr... */
#include <dlfcn.h>
#undef __USE_GNU
#endif /* DEBUGGING */

#include "naev.h"
/** @endcond */

#include "debug.h"

#include "log.h"

#if DEBUGGING
static struct backtrace_state *debug_bs = NULL;
DebugFlags debug_flags;

/**
 * @brief Gets the string related to the signal code.
 *
 *    @param sig Signal to which code belongs.
 *    @param sig_code Signal code to get string of.
 *    @return String of signal code.
 */
const char* debug_sigCodeToStr( int sig, int sig_code )
{
   if (sig == SIGFPE)
      switch (sig_code) {
#ifdef SI_USER
         case SI_USER: return _("SIGFPE (raised by program)");
#endif /* SI_USER */
#ifdef FPE_INTDIV
         case FPE_INTDIV: return _("SIGFPE (integer divide by zero)");
#endif /* FPE_INTDIV */
#ifdef FPE_INTOVF
         case FPE_INTOVF: return _("SIGFPE (integer overflow)");
#endif /* FPE_INTOVF */
#ifdef FPE_FLTDIV
         case FPE_FLTDIV: return _("SIGFPE (floating-point divide by zero)");
#endif /* FPE_FLTDIV */
#ifdef FPE_FLTOVF
         case FPE_FLTOVF: return _("SIGFPE (floating-point overflow)");
#endif /* FPE_FLTOVF */
#ifdef FPE_FLTUND
         case FPE_FLTUND: return _("SIGFPE (floating-point underflow)");
#endif /* FPE_FLTUND */
#ifdef FPE_FLTRES
         case FPE_FLTRES: return _("SIGFPE (floating-point inexact result)");
#endif /* FPE_FLTRES */
#ifdef FPE_FLTINV
         case FPE_FLTINV: return _("SIGFPE (floating-point invalid operation)");
#endif /* FPE_FLTINV */
#ifdef FPE_FLTSUB
         case FPE_FLTSUB: return _("SIGFPE (subscript out of range)");
#endif /* FPE_FLTSUB */
         default: return _("SIGFPE");
      }
   else if (sig == SIGSEGV)
      switch (sig_code) {
#ifdef SI_USER
         case SI_USER: return _("SIGSEGV (raised by program)");
#endif /* SI_USER */
#ifdef SEGV_MAPERR
         case SEGV_MAPERR: return _("SIGSEGV (address not mapped to object)");
#endif /* SEGV_MAPERR */
#ifdef SEGV_ACCERR
         case SEGV_ACCERR: return _("SIGSEGV (invalid permissions for mapped object)");
#endif /* SEGV_ACCERR */
         default: return _("SIGSEGV");
      }
   else if (sig == SIGABRT)
      switch (sig_code) {
#ifdef SI_USER
         case SI_USER: return _("SIGABRT (raised by program)");
#endif /* SI_USER */
         default: return _("SIGABRT");
      }

   /* No suitable code found. */
#if HAVE_STRSIGNAL
   return strsignal(sig);
#else /* HAVE_STRSIGNAL */
   {
      static char buf[128];
      snprintf( buf, sizeof(buf), _("signal %d"), sig );
      return buf;
   }
#endif /* HAVE_STRSIGNAL */
}
#endif /* DEBUGGING */

#if DEBUGGING
typedef struct { void* data; uintptr_t pc; const char* file; int line; const char* func; } FrameInfo;

/**
 * @brief Callback for handling one frame of a backtrace, after function lookup has succeeded (or it failed but we supplied null data).
 */
static void debug_backtrace_syminfo_callback( void* data, uintptr_t pc, const char* symname, uintptr_t symval, uintptr_t symsize )
{
   (void) symsize;
   FrameInfo *fi = data;
   Dl_info addr = {0};
   dladdr( (void*) pc, &addr );
   uintptr_t offset = pc - (symval ? symval : (uintptr_t)addr.dli_fbase);
   pc -= (uintptr_t) addr.dli_fbase;
   symname = symname ? symname : "??";
   fi->func = fi->func ? fi->func : symname;
   fi->file = fi->file ? fi->file : "??";
   addr.dli_fname = addr.dli_fname ? addr.dli_fname : "??";
   int width = snprintf( NULL, 0, "%s at %s:%u", fi->func, fi->file, fi->line );
   int pad = MAX( 0, 80 - width );
   LOGERR( "[%#14"PRIxPTR"] %s at %s:%u %*s| %s(%s+%#"PRIxPTR")", pc, fi->func, fi->file, fi->line, pad, "", addr.dli_fname, symval ? symname : "", offset );
}


/**
 * @brief Callback for handling one frame of a backtrace, after function lookup has failed.
 */
static void debug_backtrace_error_callback( void* data, const char* msg, int errnum )
{
   FrameInfo *fi = data;
   (void) msg;
   (void) errnum;
   debug_backtrace_syminfo_callback( data, fi->pc, "??", 0, 0 );
}

/**
 * @brief Callback for handling one frame of a backtrace.
 */
static int debug_backtrace_full_callback( void* data, uintptr_t pc, const char* file, int line, const char* func )
{
   FrameInfo fi = { .data = data, .pc = pc, .file = file, .line = line, .func = func };
   if (pc != 0 && ~pc != 0)
      backtrace_syminfo( debug_bs, pc, debug_backtrace_syminfo_callback, debug_backtrace_error_callback, &fi );

   return 0;
}

/**
 * @brief Logs a backtrace to stderr.
 */
void debug_logBacktrace (void) {
   backtrace_full( debug_bs, 1, debug_backtrace_full_callback, NULL, NULL );
}

#if HAVE_SIGACTION
static void debug_sigHandler( int sig, siginfo_t *info, void *unused )
#else /* HAVE_SIGACTION */
static void debug_sigHandler( int sig )
#endif /* HAVE_SIGACTION */
{
   (void) sig;
#if HAVE_SIGACTION
   (void) unused;
#endif /* HAVE_SIGACTION */

#if HAVE_SIGACTION
   LOGERR( _("Naev received %s!"), debug_sigCodeToStr( info->si_signo, info->si_code ) );
#else /* HAVE_SIGACTION */
   LOGERR( _("Naev received %s!"), debug_sigCodeToStr( sig, 0 ) );
#endif /* HAVE_SIGACTION */

   debug_logBacktrace();
   LOGERR( _("Report this to project maintainer with the backtrace.") );

   /* Always exit. */
   exit(1);
}
#endif /* DEBUGGING */

/**
 * @brief Sets up the back-tracing signal handler.
 */
void debug_sigInit (void)
{
#if DEBUGGING
   Dl_info addr = {0};
#if WIN32
   dladdr( debug_sigInit, &addr );  /* Get the filename using dlfcn-win32; libbacktrace fucks this up (as of 2022-08-18). */
#endif /* WIN32 */

   debug_bs = backtrace_create_state( addr.dli_fname, /*threaded:*/ 1, NULL, NULL );

   /* Set up handler. */
#if HAVE_SIGACTION
   const char *str = _("Unable to set up %s signal handler.");
   struct sigaction so, sa = { .sa_handler = NULL, .sa_flags = SA_SIGINFO };
   sa.sa_sigaction = debug_sigHandler;
   sigemptyset(&sa.sa_mask);

   sigaction(SIGSEGV, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( str, "SIGSEGV" );
   sigaction(SIGFPE, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( str, "SIGFPE" );
   sigaction(SIGABRT, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( str, "SIGABRT" );
#else /* HAVE_SIGACTION */
   signal( SIGSEGV, debug_sigHandler );
   signal( SIGFPE,  debug_sigHandler );
   signal( SIGABRT, debug_sigHandler );
#endif /* HAVE_SIGACTION */
#endif /* DEBUGGING */
}


/**
 * @brief Closes the back-tracing signal handler.
 */
void debug_sigClose (void)
{
#if DEBUGGING
   signal( SIGSEGV, SIG_DFL );
   signal( SIGFPE,  SIG_DFL );
   signal( SIGABRT, SIG_DFL );
#endif /* DEBUGGING */
}


/**
 * @brief Does nothing. Calling this tells our debug scripts to stop tracing.
 */
void debug_enableLeakSanitizer (void)
{
}
