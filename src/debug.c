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
#include <unwind.h>

#define __USE_GNU /* Grrr... */
#include <dlfcn.h>
#undef __USE_GNU
#endif /* DEBUGGING */

#if DEBUGGING && !HAVE_STRSIGNAL
extern const char *strsignal (int); /* From libiberty */
#endif /* DEBUGGING && !HAVE_STRSIGNAL */

#include "naev.h"
/** @endcond */

#include "debug.h"

#include "log.h"

#if DEBUGGING
static bfd *abfd      = NULL;
static asymbol **syms = NULL;
/* Initialize debugging flags. */
DebugFlags debug_flags;
#endif /* DEBUGGING */

#ifdef bfd_get_section_flags
/* We're dealing with a binutils version prior to 2.34 (2020-02-01) and must adapt the API as follows: */
#define bfd_section_flags( section )    bfd_get_section_flags( abfd, section )
#define bfd_section_vma( section )      bfd_get_section_vma( abfd, section )
#define bfd_section_size( section )     bfd_get_section_size( section )
#endif /* bfd_get_section_flags */


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
   return strsignal(sig);
}
#endif /* DEBUGGING */

#if DEBUGGING
/**
 * @brief Translates and displays the address as something humans can enjoy.
 */
static void debug_translateAddress( void *address )
{
   const char *file = NULL, *func = NULL;
   unsigned int line = 0;
   asection *section;
   Dl_info addr = {0};

   (void) dladdr( address, &addr );

   for (section = abfd==NULL?NULL:abfd->sections; section != NULL; section = section->next) {
      if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
         continue;

      bfd_vma vma = bfd_section_vma(section), func_vma = (uintptr_t) address;
      bfd_size_type size = bfd_section_size(section);
      if (func_vma < vma || func_vma >= vma + size)
         func_vma = (bfd_vma) (address - addr.dli_fbase);
      if (func_vma < vma || func_vma >= vma + size)
         continue;

      (void) bfd_find_nearest_line(abfd, section, syms, func_vma - vma, &file, &func, &line);
      break;
   }

   do {
      bfd_vma offset = address - (addr.dli_saddr ? addr.dli_saddr : addr.dli_fbase);
#define TRY( str ) ((str != NULL && str[0]) ? str : "??")
#define OPT( str ) ((str != NULL && str[0]) ? str : "")
      DEBUG( "%s(%s+%#" BFD_VMA_FMT "x) [%p] %s(...):%u %s", TRY(addr.dli_fname), OPT(addr.dli_sname), offset, address, TRY(func), line, TRY(file) );
   } while (section!=NULL && bfd_find_inliner_info(abfd, &file, &func, &line));
}

/**
 * @brief Translates and displays the address as something humans can enjoy.
 */
static _Unwind_Reason_Code debug_unwindTrace( struct _Unwind_Context* ctx, void* data )
{
   (void) data;
   int ip_before_insn = 0;
   uintptr_t ip = _Unwind_GetIPInfo( ctx, &ip_before_insn );

   if (!ip)
      return _URC_END_OF_STACK;
   if (!ip_before_insn)
      ip -= 1;
   debug_translateAddress( (void*) ip );
   return _URC_NO_REASON;
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

   LOG( _("Naev received %s!"),
#if HAVE_SIGACTION
         debug_sigCodeToStr( info->si_signo, info->si_code )
#else /* HAVE_SIGACTION */
         debug_sigCodeToStr( sig, 0 )
#endif /* HAVE_SIGACTION */
	);

   _Unwind_Backtrace( debug_unwindTrace, NULL );
   DEBUG( _("Report this to project maintainer with the backtrace.") );

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
   bfd_init();

   /* Read the executable. We need its full path, which Linux provides via procfs and Darwin/dlfcn-win32 dladdr() happens to provide. */
#if LINUX
   abfd = bfd_openr( "/proc/self/exe", NULL );
#else
   Dl_info addr = {0};
   (void) dladdr( debug_sigInit, &addr );
   abfd = bfd_openr( addr.dli_fname, NULL );
#endif
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
 * @brief Closes the back-tracing signal handler.
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
