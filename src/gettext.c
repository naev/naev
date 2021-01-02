/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file gettext.c
 *
 * @brief PhysicsFS-aware gettext implementation.
 */


/** @cond */
#include <locale.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "gettext.h"

#include "array.h"
#include "env.h"
#include "log.h"
#include "msgcat.h"
#include "ndata.h"
#include "nstring.h"


static char gettext_systemLanguage[64] = "en";  /**< Language set by the environment when we started up. */
static msgcat_t **gettext_msgcatChain = NULL;   /**< Pointer to the Array of message catalogs to try in order. */
static msgcat_t *evil = NULL;                   /**< Initial implementation. */

static void gettext_addCat( const char* path );

/**
 * @brief Initialize the translation system.
 * There's no presumption that PhysicsFS is available, so this doesn't actually load translations.
 * There is no gettext_exit() because invalidating pointers to translated strings would be too dangerous.
 */
void gettext_init()
{
   const char *language;

   evil = array_create( msgcat_t );
   gettext_msgcatChain = &evil;
   setlocale( LC_ALL, "" );
   /* If we don't disable LC_NUMERIC, lots of stuff blows up because 1,000 can be interpreted as
    * 1.0 in certain languages. */
   setlocale( LC_NUMERIC, "C" ); /* Disable numeric locale part. */

   language = getenv( "LANGUAGE" );
   if (language != NULL)
      strncpy( gettext_systemLanguage, language, sizeof(gettext_systemLanguage)-1 );
}

/**
 * @brief Set the translation language.
 *
 * @param lang Language code to use. If NULL, use the system default.
 */
void gettext_setLanguage( const char* lang )
{
   char langbuf[256];

   /* Note: We tried setlocale( LC_ALL, ... ), but it bails if no corresponding system
    * locale exists. That's too restrictive when we only need our own language catalogs. */
   if (lang == NULL)
      lang = gettext_systemLanguage;

   nsetenv( "LANGUAGE", lang, 1 );
   DEBUG( _("Reset language to \"%s\""), lang );
   nsnprintf( langbuf, sizeof(langbuf), GETTEXT_PATH"%s/LC_MESSAGES/"PACKAGE_NAME".mo", lang );
   gettext_addCat( langbuf );
}


/**
 * @brief Open the given ndata path and put that catalog at the end of gettext_msgcatChain.
 */
static void gettext_addCat( const char* path )
{
   const char* map;
   size_t map_size;

   map = ndata_read( path, &map_size );
   if (map != NULL) {
      msgcat_init( &array_grow( gettext_msgcatChain ), map, map_size );
      DEBUG( _("Using translations from %s"), path );
   }
}

/**
 * @brief Return a translated version of the input, using the current language catalogs.
 *
 * @param msgid The English singular form.
 * @param msgid_plural The English plural form. (Pass NULL if simply translating \p msgid1.)
 * @param n The number determining the plural form to use. (Pass 1 if simply translating \p msgid1.)
 * @return The translation in the message catalog, if it exists, else whichever of msgid1 or msgid2 is
 *         appropriate in English. The returned string must not be modified or freed.
 */
const char* gettext_ngettext( const char* msgid, const char* msgid_plural, uint64_t n )
{
   msgcat_t *chain;
   const char* trans;
   int i;

   chain = *gettext_msgcatChain;
   for (i=0; i<array_size(chain); i++) {
      trans = msgcat_ngettext( &chain[i], msgid, msgid_plural, n );
      if (trans != NULL)
         return (char*)trans;
   }

   return n>1 && msgid_plural!=NULL ? msgid_plural : msgid;
}

/**
 * @brief Helper function for p_(): Return _(lookup) with a fallback of msgid rather than lookup.
 */
const char* gettext_pgettext( const char* lookup, const char* msgid )
{
   const char *trans;
   trans = _( lookup );
   return trans==lookup ? msgid : trans;
}
