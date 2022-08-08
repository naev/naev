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
#include <bfd.h>

#if HAVE_DLADDR
#define __USE_GNU /* Grrr... */
#include <dlfcn.h>
#undef __USE_GNU
#endif /* HAVE_DLADDR */

#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif /* HAVE_EXECINFO_H */
#endif /* DEBUGGING */

#include "naev.h"
/** @endcond */

#include "log.h"

#if DEBUGGING
static bfd *abfd      = NULL;
static asymbol **syms = NULL;
#endif /* DEBUGGING */

#ifdef bfd_get_section_flags
/* We're dealing with a binutils version prior to 2.34 (2020-02-01) and must adapt the API as follows: */
#define bfd_section_flags( section )    bfd_get_section_flags( abfd, section )
#define bfd_section_vma( section )      bfd_get_section_vma( abfd, section )
#define bfd_section_size( section )     bfd_get_section_size( section )
#endif /* bfd_get_section_flags */


#ifdef DEBUGGING
/* Initialize debugging flags. */
#include "debug.h"
DebugFlags debug_flags;
#endif /* DEBUGGING */


#if DEBUGGING
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

#if DEBUGGING && HAVE_EXECINFO_H
/**
 * @brief Translates and displays the address as something humans can enjoy.
 */
static void debug_translateAddress( const char *symbol, void *address )
{
   const char *file, *func;
   unsigned int line;
   asection *section;

   for (section = abfd->sections; section != NULL; section = section->next) {
      bfd_vma func_vma = (uintptr_t) address, base_vma = 0;
#if HAVE_DLADDR
      Dl_info addr;
      if (dladdr( address, &addr ))
         base_vma = (uintptr_t) addr.dli_fbase;
#endif /* HAVE_DLADDR */

      if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
         continue;

      bfd_vma vma = bfd_section_vma(section);
      bfd_size_type size = bfd_section_size(section);
      if (func_vma < vma || func_vma >= vma + size) {
         func_vma -= base_vma;
         if (func_vma < vma || func_vma >= vma + size)
            continue;
      }

      if (!bfd_find_nearest_line(abfd, section, syms, func_vma - vma,
            &file, &func, &line))
         continue;

      do {
         if (func == NULL || func[0] == '\0')
            func = "??";
         DEBUG("%s %s(...):%u %s", symbol, func, line, file);
      } while (bfd_find_inliner_info(abfd, &file, &func, &line));

      return;
   }

   DEBUG("%s %s(...):%u %s", symbol, "??", 0, "??");
}
#endif /* DEBUGGING && HAVE_EXECINFO_H */

#if DEBUGGING
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

   LOG( _("Naev received %s!"),
#if HAVE_SIGACTION
         debug_sigCodeToStr( info->si_signo, info->si_code )
#else /* HAVE_SIGACTION */
         debug_sigCodeToStr( sig, 0 )
#endif /* HAVE_SIGACTION */
	);

#if HAVE_EXECINFO_H
   int num;
   void *buf[64];
   char **symbols;

   num      = backtrace(buf, 64);
   symbols  = backtrace_symbols(buf, num);
   for (int i=0; i<num; i++) {
      if (abfd != NULL) {
         debug_translateAddress( symbols[i], buf[i] );
	 continue;
      }
      DEBUG("   %s", symbols[i]);
   }
   DEBUG( _("Report this to project maintainer with the backtrace.") );
#endif /* HAVE_EXECINFO_H */

   /* Always exit. */
   exit(1);
}
#endif /* DEBUGGING */

/**
 * @brief Sets up the SignalHandler for Linux.
 */
void debug_sigInit (void)
{
#if DEBUGGING
   bfd_init();

   /* Read the executable. TODO: in case libbfd exists on platforms without procfs, try env.argv0 from "env.h"? */
   abfd = bfd_openr("/proc/self/exe", NULL);
   if (abfd != NULL) {
      char **matching;
      bfd_check_format_matches(abfd, bfd_object, &matching);

      /* Read symbols */
      if (bfd_get_file_flags(abfd) & HAS_SYMS) {
         unsigned int size;
         long symcount = bfd_read_minisymbols( abfd, /*dynamic:*/ FALSE, (void **)&syms, &size );
         if ( symcount == 0 && abfd != NULL )
            symcount = bfd_read_minisymbols( abfd, /*dynamic:*/ TRUE, (void **)&syms, &size );
         assert(symcount >= 0);
      }
   }

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
 * @brief Closes the SignalHandler for Linux.
 */
void debug_sigClose (void)
{
#if DEBUGGING
   bfd_close( abfd );
   abfd = NULL;
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
