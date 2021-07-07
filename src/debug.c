/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file debug.c
 *
 * @brief Handles low-level debugging hooks.
 */

/** @cond */
#include "naev.h"

#if LINUX && HAS_BFD && DEBUGGING
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <bfd.h>
#include <assert.h>
#endif /* LINUX && HAS_BFD && DEBUGGING */
/** @endcond */

#include "log.h"

#if HAVE_FENV_H && DEBUGGING
/* This module uses GNU extensions to enable FPU exceptions. */
#define _GNU_SOURCE
#include <fenv.h>
#endif /* HAVE_FENV_H && DEBUGGING */


#if LINUX && HAS_BFD && DEBUGGING
static bfd *abfd      = NULL;
static asymbol **syms = NULL;
#endif /* LINUX && HAS_BFD && DEBUGGING */


#ifdef DEBUGGING
/* Initialize debugging flags. */
#include "debug.h"
DebugFlags debug_flags;
#endif /* DEBUGGING */


#if LINUX && HAS_BFD && DEBUGGING
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
         case SI_USER: return _("SIGFPE (raised by program)");
         case FPE_INTDIV: return _("SIGFPE (integer divide by zero)");
         case FPE_INTOVF: return _("SIGFPE (integer overflow)");
         case FPE_FLTDIV: return _("SIGFPE (floating-point divide by zero)");
         case FPE_FLTOVF: return _("SIGFPE (floating-point overflow)");
         case FPE_FLTUND: return _("SIGFPE (floating-point underflow)");
         case FPE_FLTRES: return _("SIGFPE (floating-point inexact result)");
         case FPE_FLTINV: return _("SIGFPE (floating-point invalid operation)");
         case FPE_FLTSUB: return _("SIGFPE (subscript out of range)");
         default: return _("SIGFPE");
      }
   else if (sig == SIGSEGV)
      switch (sig_code) {
         case SI_USER: return _("SIGSEGV (raised by program)");
         case SEGV_MAPERR: return _("SIGSEGV (address not mapped to object)");
         case SEGV_ACCERR: return _("SIGSEGV (invalid permissions for mapped object)");
         default: return _("SIGSEGV");
      }
   else if (sig == SIGABRT)
      switch (sig_code) {
         case SI_USER: return _("SIGABRT (raised by program)");
         default: return _("SIGABRT");
      }

   /* No suitable code found. */
   return strsignal(sig);
}

/**
 * @brief Translates and displays the address as something humans can enjoy.
 *
 * @TODO Remove the conditional defines which are to support old BFD (namely Ubuntu 16.04).
 */
static void debug_translateAddress( const char *symbol, bfd_vma address )
{
   const char *file, *func;
   unsigned int line;
   asection *section;

   for (section = abfd->sections; section != NULL; section = section->next) {
#ifdef bfd_get_section_flags
      if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
#else /* bfd_get_section_flags */
      if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
#endif /* bfd_get_section_flags */
         continue;

#ifdef bfd_get_section_vma
      bfd_vma vma = bfd_get_section_vma(abfd, section);
#else /* bfd_get_section_vma */
      bfd_vma vma = bfd_section_vma(section);
#endif /* bfd_get_section_vma */

#ifdef bfd_get_section_size
      bfd_size_type size = bfd_get_section_size(section);
#else /* bfd_get_section_size */
      bfd_size_type size = bfd_section_size(section);
#endif /* bfd_get_section_size */
      if (address < vma || address >= vma + size)
         continue;

      if (!bfd_find_nearest_line(abfd, section, syms, address - vma,
            &file, &func, &line))
         continue;

      do {
         if (func == NULL || func[0] == '\0')
            func = "??";
         if (file == NULL || file[0] == '\0')
            file = "??";
         DEBUG("%s %s(...):%u %s", symbol, func, line, file);
      } while (bfd_find_inliner_info(abfd, &file, &func, &line));

      return;
   }

   DEBUG("%s %s(...):%u %s", symbol, "??", 0, "??");
}


/**
 * @brief Backtrace signal handler for Linux.
 *
 *    @param sig Signal.
 *    @param info Signal information.
 *    @param unused Unused.
 */
static void debug_sigHandler( int sig, siginfo_t *info, void *unused )
{
   (void)sig;
   (void)unused;
   int i, num;
   void *buf[64];
   char **symbols;

   num      = backtrace(buf, 64);
   symbols  = backtrace_symbols(buf, num);

   DEBUG( _("Naev received %s!"),
         debug_sigCodeToStr(info->si_signo, info->si_code) );
   for (i=0; i<num; i++) {
      if (abfd != NULL)
         debug_translateAddress(symbols[i], (bfd_vma) (bfd_hostptr_t) buf[i]);
      else
         DEBUG("   %s", symbols[i]);
   }
   DEBUG( _("Report this to project maintainer with the backtrace.") );

   /* Always exit. */
   exit(1);
}
#endif /* LINUX && HAS_BFD && DEBUGGING */


/**
 * @brief Sets up the SignalHandler for Linux.
 */
void debug_sigInit (void)
{
#if LINUX && HAS_BFD && DEBUGGING
   char **matching;
   struct sigaction sa, so;
   long symcount;
   unsigned int size;

   bfd_init();

   /* Read the executable */
   abfd = bfd_openr("/proc/self/exe", NULL);
   if (abfd != NULL) {
      bfd_check_format_matches(abfd, bfd_object, &matching);

      /* Read symbols */
      if (bfd_get_file_flags(abfd) & HAS_SYMS) {

         /* static */
         symcount = bfd_read_minisymbols (abfd, FALSE, (void **)&syms, &size);
         if ( symcount == 0 && abfd != NULL ) /* dynamic */
            symcount = bfd_read_minisymbols (abfd, TRUE, (void **)&syms, &size);
         assert(symcount >= 0);
      }
   }

   /* Set up handler. */
   sa.sa_handler   = NULL;
   sa.sa_sigaction = debug_sigHandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags     = SA_SIGINFO;

   /* Attach signals. */
   sigaction(SIGSEGV, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up SIGSEGV signal handler.") );
   sigaction(SIGFPE, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up SIGFPE signal handler.") );
   sigaction(SIGABRT, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up SIGABRT signal handler.") );
   DEBUG( _("BFD backtrace catching enabled.") );
#endif /* LINUX && HAS_BFD && DEBUGGING */
}


/**
 * @brief Closes the SignalHandler for Linux.
 */
void debug_sigClose (void)
{
#if LINUX && HAS_BFD && DEBUGGING
   bfd_close( abfd );
#endif /* LINUX && HAS_BFD && DEBUGGING */
}


/**
 * @brief Enables FPU exceptions. Artificially limited to Linux until link issues are figured out.
 */
void debug_enableFPUExcept (void)
{
#if HAVE_FEENABLEEXCEPT && DEBUGGING
      feenableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
#endif /* HAVE_FEENABLEEXCEPT && DEBUGGING */
}
