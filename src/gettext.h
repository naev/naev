/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stddef.h>
#include <stdint.h>
/** @endcond */

#include "attributes.h"

typedef struct LanguageOption_ {
   char  *language; /**< Language code (allocated string). */
   double coverage; /**< The fraction of strings which have a translation. */
} LanguageOption;

void            gettext_init( void );
void            gettext_exit( void );
const char     *gettext_getSystemLanguage( void );
const char     *gettext_getLanguage( void );
void            gettext_setLanguage( const char *lang );
LanguageOption *gettext_languageOptions( void );
double          gettext_languageCoverage( const char *lang );

const char *gettext_ngettext( const char *msgid, const char *msgid_plural,
                              uint64_t n );
FORMAT_ARG( 2 )
const char *gettext_pgettext_impl( const char *lookup, const char *msgid );

/** A pseudo function call that serves as a marker for the automated
 * extraction of messages, but does not call gettext(). The run-time
 * translation is done at a different place in the code.
 * The argument, String, should be a literal string. Concatenated strings
 * and other string expressions won't work.
 * The macro's expansion is not parenthesized, so that it is suitable as
 * initializer for static 'char[]' or 'const char[]' variables.
 */
#define gettext_noop( String ) String

/** A gettext() implementation that will return the input's translation,
 * if the message catalogues in ndata provide it. Otherwise, it returns its
 * input. The returned string must not be modified or freed.
 */
FORMAT_ARG( 1 ) static inline const char *_( const char *msgid )
{
   return gettext_ngettext( msgid, NULL, 1 );
}
/* Helper for rust. */
const char *gettext_rust( const char *msgid );

/** \see gettext_noop */
#define N_( msgid ) gettext_noop( msgid )

/** \see gettext_ngettext() */
FORMAT_ARG( 1 )
static inline const char *n_( const char *msgid, const char *msgid_plural,
                              uint64_t n )
{
   return gettext_ngettext( msgid, msgid_plural, n );
}

/** The separator between msgctxt and msgid in a .mo file.  */
#define GETTEXT_CONTEXT_GLUE "\004"

/** Pseudo function call, taking a \p msgctxt and a \p msgid instead of just a
 * \p msgid. \p msgctxt and \p msgid must be string literals. \p msgctxt should
 * be short and rarely need to change. The letter 'p' stands for 'particular' or
 * 'special'.
 */
#define p_( msgctxt, msgid )                                                   \
   gettext_pgettext_impl( msgctxt GETTEXT_CONTEXT_GLUE msgid, msgid )

/** The function is almost the same as p_() but \p msgctxt and \p msgid can be
 * string variables.
 */
const char *pgettext_var( const char *msgctxt, const char *msgid );
