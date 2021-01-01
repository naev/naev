/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file gettext.c
 *
 * @brief Stub: will become a PhysicsFS-aware gettext implementation.
 */


/** @cond */
#include "naev.h"
/** @endcond */

#include "options.h"

#include "env.h"
#include "gettext.h"
#include "log.h"
#include "ndata.h"
#include "nstring.h"


/**
 * @brief Set up locales/environment/globals for translation.
 *
 * @param lang Language code to use. If NULL, use the system default.
 */
void gettext_setLanguage( const char* lang )
{
   char langbuf[256];

   setlocale( LC_ALL, "" );
   /* If we don't disable LC_NUMERIC, lots of stuff blows up because 1,000 can be interpreted as
    * 1.0 in certain languages. */
   setlocale( LC_NUMERIC, "C" ); /* Disable numeric locale part. */
   /* Note: We tried setlocale( LC_ALL, ... ), but it bails if no corresponding system
    * locale exists. That's too restrictive when we only need our own language catalogs. */
   if (lang == NULL)
      nsetenv( "LANGUAGE", "", 0 );
   else {
      nsetenv( "LANGUAGE", lang, 1 );
      DEBUG(_("Reset language to \"%s\""), lang);
   }
   /* HACK: All of our code assumes it's working with UTF-8, so force gettext to return it.
    * Testing under Wine shows it may default to Windows-1252, resulting in glitched translations
    * like "Loading Ships..." -> "Schiffe laden \x85" which our font code will render improperly. */
   nsetenv( "OUTPUT_CHARSET", "utf-8", 1 );
   /* Horrible hack taken from https://www.gnu.org/software/gettext/manual/html_node/gettext-grok.html .
    * Not entirely sure it is necessary, but just in case... */
   {
      extern int  _nl_msg_cat_cntr;
      ++_nl_msg_cat_cntr;
   }
   nsnprintf( langbuf, sizeof(langbuf), "%s/"GETTEXT_PATH, ndata_getPath() );
   bindtextdomain( PACKAGE_NAME, langbuf );
   textdomain( PACKAGE_NAME );
}
