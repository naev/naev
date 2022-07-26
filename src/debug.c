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

#include "naev.h"

#if LINUX && HAS_BFD && DEBUGGING
#include <execinfo.h>
#include <bfd.h>
#endif /* LINUX && HAS_BFD && DEBUGGING */
/** @endcond */

#include "log.h"

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
      DEBUG( _("Unable to set up %s signal handler."), "SIGSEGV" );
   sigaction(SIGFPE, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up %s signal handler."), "SIGFPE" );
   sigaction(SIGABRT, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up %s signal handler."), "SIGABRT" );
   sigaction(SIGTRAP, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up %s signal handler."), "SIGTRAP" );
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
   abfd = NULL;
   signal( SIGSEGV, SIG_DFL );
   signal( SIGFPE,  SIG_DFL );
   signal( SIGABRT, SIG_DFL );
   signal( SIGTRAP, SIG_DFL );
#endif /* LINUX && HAS_BFD && DEBUGGING */
}
