/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef GETTEXT_H
#define GETTEXT_H

/** @cond */
#include <stdint.h>
/** @endcond */

void gettext_init();
void gettext_setLanguage( const char* lang );

char* gettext_ngettext( const char* msgid, const char* msgid_plural, uint64_t n );
const char* gettext_pgettext( const char* lookup, const char* msgid );

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
 * if the message catalogs in ndata provide it. Otherwise, it returns its
 * input. The returned string must not be modified or freed.
 */
#define _( msgid )                    gettext_ngettext( msgid, NULL, 1 )

/** \see gettext_noop */
#define N_( msgid )                   gettext_noop( msgid )

/** \see gettext_ngettext() */
#define n_( msgid, msgid_plural, n )  gettext_ngettext( msgid, msgid_plural, n )

/** The separator between msgctxt and msgid in a .mo file.  */
#define GETTEXT_CONTEXT_GLUE "\004"

/** Pseudo function call, taking a \p msgctxt and a \p msgid instead of just a
 * \p msgid. \p msgctxt and \p msgid must be string literals. \p msgctxt should be
 * short and rarely need to change.
 * The letter 'p' stands for 'particular' or 'special'.
 */
#define p_( msgctxt, msgid )          gettext_pgettext( msgctxt GETTEXT_CONTEXT_GLUE msgid, msgid )

#endif /* GETTEXT_H */
