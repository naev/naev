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
#include "physfs.h"

#include "naev.h"
/** @endcond */

#include "gettext.h"

#include "array.h"
#include "env.h"
#include "log.h"
#include "msgcat.h"
#include "ndata.h"
#include "nstring.h"


typedef struct translation {
   char *language;              /**< Language code (allocated string). */
   msgcat_t *chain;             /**< Array of message catalogs to try in order. */
   struct translation *next;    /**< Next entry in the list of loaded translations. */
} translation_t;


static char gettext_systemLanguage[64] = "en";          /**< Language set by the environment when we started up. */
static translation_t *gettext_translations = NULL;      /**< Linked list of loaded translation chains. */
static translation_t *gettext_activeTranslation = NULL; /**< Active language's code. */


/**
 * @brief Initialize the translation system.
 * There's no presumption that PhysicsFS is available, so this doesn't actually load translations.
 * There is no gettext_exit() because invalidating pointers to translated strings would be too dangerous.
 */
void gettext_init()
{
   const char *language;

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
   translation_t *newtrans, **ptrans;
   char root[256], **paths;
   int i;
   const char* map;
   size_t map_size;

   if (lang == NULL)
      lang = gettext_systemLanguage;
   if (gettext_activeTranslation != NULL && !strcmp( lang, gettext_activeTranslation->language ))
      return;

   /* Set the environment variable used by GNU gettext. This can help logging/messages
    * emitted by libraries appear in the user's native language. (setlocale(LC_ALL, ...) may
    * seem like a better idea, but it checks unnecessarily for system locale definitions. */
   nsetenv( "LANGUAGE", lang, 1 );

   /* Search for the selected language in the loaded translations. */
   for (ptrans = &gettext_translations; *ptrans != NULL; ptrans = &(*ptrans)->next)
      if (!strcmp( lang, (*ptrans)->language )) {
         gettext_activeTranslation = *ptrans;
         return;
      }

   /* Load a new translation chain from ndata, and activate it. */
   newtrans = calloc( 1, sizeof( translation_t ) );
   newtrans->language = strdup( lang );
   newtrans->chain = array_create( msgcat_t );

   /* @TODO This code orders the translations alphabetically by file path.
    * That doesn't make sense, but this is a new use case and it's unclear
    * how we should determine precedence in case multiple .mo files exist. */
   nsnprintf( root, sizeof(root), GETTEXT_PATH"%s", lang );
   paths = ndata_listRecursive( root );
   for (i=0; i<array_size(paths); i++) {
      map = ndata_read( paths[i], &map_size );
      if (map != NULL) {
         msgcat_init( &array_grow( &newtrans->chain ), map, map_size );
         DEBUG( _("Adding translations from %s"), paths[i] );
      }
      free( paths[i] );
   }
   array_free( paths );
   gettext_activeTranslation = *ptrans = newtrans;
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

   if (gettext_activeTranslation != NULL) {
      chain = gettext_activeTranslation->chain;
      for (i=0; i<array_size(chain); i++) {
         trans = msgcat_ngettext( &chain[i], msgid, msgid_plural, n );
         if (trans != NULL)
            return (char*)trans;
      }
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
