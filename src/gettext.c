/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file gettext.c
 *
 * @brief PhysicsFS-aware gettext implementation.
 */
/** @cond */
#include <ctype.h>
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

typedef struct translation {
   char *language;              /**< Language code (allocated string). */
   msgcat_t *chain;             /**< Array of message catalogs to try in order. */
   char **chain_lang;           /**< Array of those catalogs' names. */
   struct translation *next;    /**< Next entry in the list of loaded translations. */
} translation_t;

static char *gettext_systemLanguage = NULL;             /**< Language, or :-delimited list of them, from the system at startup. */
static translation_t *gettext_translations = NULL;      /**< Linked list of loaded translation chains. */
static translation_t *gettext_activeTranslation = NULL; /**< Active language's code. */
static uint32_t gettext_nstrings = 0;                   /**< Number of translatable strings in the game. */

static void gettext_readStats (void);
static const char* gettext_matchLanguage( const char* lang, size_t lang_len, char*const* available );

/**
 * @brief Initialize the translation system.
 * There's no presumption that PhysicsFS is available, so this doesn't actually load translations.
 * We loosely follow: https://www.gnu.org/software/gettext/manual/html_node/Locale-Environment-Variables.html
 */
void gettext_init (void)
{
   const char *env_vars[] = {"LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"};

   setlocale( LC_ALL, "" );
   /* If we don't disable LC_NUMERIC, lots of stuff blows up because 1,000 can be interpreted as
    * 1.0 in certain languages. */
   setlocale( LC_NUMERIC, "C" ); /* Disable numeric locale part. */

   /* Try to get info from SDL. */
   SDL_Locale *locales = SDL_GetPreferredLocales();
   if (locales != NULL) {
      if (locales[0].language != NULL) {
         gettext_systemLanguage = strdup( locales[0].language );
         SDL_free( locales );
         return;
      }
      SDL_free( locales );
   }

   free( gettext_systemLanguage );
   for (size_t i=0; i < sizeof(env_vars)/sizeof(env_vars[0]); i++) {
      const char *language = SDL_getenv( env_vars[i] );
      if (language != NULL && *language != 0) {
         gettext_systemLanguage = strdup( language );
         return; /* The first env var with language settings wins. */
      }
   }
   gettext_systemLanguage = strdup( "" );
}

/**
 * @brief Free resources associated with the translation system.
 * This invalidates previously returned pointers to translated strings; be sure to actually exit after calling!
 */
void gettext_exit (void)
{
   free( gettext_systemLanguage );
   gettext_systemLanguage = NULL;
   while (gettext_translations != NULL) {
      for (int i = 0; i < array_size(gettext_translations->chain_lang); i++)
         free( gettext_translations->chain_lang[i] );
      array_free( gettext_translations->chain_lang );
      for (int i = 0; i < array_size(gettext_translations->chain); i++)
         free( (void*) gettext_translations->chain[i].map );
      array_free( gettext_translations->chain );
      free( gettext_translations->language );
      gettext_activeTranslation = gettext_translations->next;
      free( gettext_translations );
      gettext_translations = gettext_activeTranslation;
   }
}

/**
 * @brief Gets the current system language as detected by Naev.
 */
const char* gettext_getSystemLanguage (void)
{
   return gettext_systemLanguage;
}

/**
 * @brief Gets the active (primary) translation language. Even in case of a complex locale, this will be the name of
 *        the first message catalog to be checked (or the "en" language code for untranslated English).
 *        The purpose is to provide a simple answer to things like libunibreak which ask which language we're using.
 */
const char* gettext_getLanguage (void)
{
   if (array_size( gettext_activeTranslation->chain_lang ))
      return gettext_activeTranslation->chain_lang[0];
   else
      return "en";
}

/**
 * @brief Set the translation language.
 *
 * @param lang Language code to use. If NULL, use the system default.
 */
void gettext_setLanguage( const char* lang )
{
   translation_t *newtrans;
   char root[256], **paths, **available_langs;

   if (lang == NULL)
      lang = gettext_systemLanguage;
   if (gettext_activeTranslation != NULL && !strcmp( lang, gettext_activeTranslation->language ))
      return;

   /* Search for the selected language in the loaded translations. */
   for (translation_t *ptrans = gettext_translations; ptrans != NULL; ptrans = ptrans->next)
      if (!strcmp( lang, ptrans->language )) {
         gettext_activeTranslation = ptrans;
         return;
      }

   /* Load a new translation chain from ndata, and activate it. */
   newtrans = calloc( 1, sizeof( translation_t ) );
   newtrans->language = strdup( lang );
   newtrans->chain = array_create( msgcat_t );
   newtrans->chain_lang = array_create( char* );
   newtrans->next = gettext_translations;
   gettext_translations = newtrans;

   available_langs = PHYSFS_enumerateFiles( GETTEXT_PATH );

   /* @TODO This code orders the translations alphabetically by file path.
    * That doesn't make sense, but this is a new use case and it's unclear
    * how we should determine precedence in case multiple .mo files exist. */
   while (lang != NULL && *lang != 0) {
      size_t map_size, lang_part_len;
      const char *lang_part = lang, *lang_match;
      lang = strchr(lang, ':');
      if (lang == NULL)
         lang_part_len = strlen(lang_part);
      else {
         lang_part_len = (size_t)(lang-lang_part);
         lang++;
      }
      lang_match = gettext_matchLanguage( lang_part, lang_part_len, available_langs );
      if (lang_match == NULL)
         continue;
      snprintf( root, sizeof(root), GETTEXT_PATH"%s", lang_match );
      paths = ndata_listRecursive( root );
      for (int i=0; i<array_size(paths); i++) {
         const char *map = ndata_read( paths[i], &map_size );
         if (map != NULL) {
            msgcat_init( &array_grow( &newtrans->chain ), map, map_size );
            array_push_back( &newtrans->chain_lang, strdup( lang_match ) );
            DEBUG( _("Adding translations from %s"), paths[i] );
         }
         free( paths[i] );
      }
      array_free( paths );
   }
   PHYSFS_freeList( available_langs );
   gettext_activeTranslation = newtrans;
}

/**
 * @brief Pick the best match from "available" (a physfs listing) for the string-slice with address lang, length lang_len.
 *
 * @return The best match, if any, else NULL.
 */
static const char* gettext_matchLanguage( const char* lang, size_t lang_len, char*const* available )
{
   const char *best = NULL;

   if (lang_len == 0)
      return NULL;

   /* Good enough for now: Return the greatest (thus longest) string matching up to their common length. */
   for (size_t i=0; available[i]!=NULL; i++) {
      int c = strncmp( lang, available[i], MIN( lang_len, strlen(available[i]) ) );
      if (c < 0)
         break;
      else if (c == 0)
         best = available[i];
   }
   return best;
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
   if (gettext_activeTranslation != NULL) {
      msgcat_t *chain = gettext_activeTranslation->chain;
      for (int i=0; i<array_size(chain); i++) {
         const char *trans = msgcat_ngettext( &chain[i], msgid, msgid_plural, n );
         if (trans != NULL)
            return (char*)trans;
      }
   }

   return n>1 && msgid_plural!=NULL ? msgid_plural : msgid;
}

/**
 * @brief Helper function for p_(): Return _(lookup) with a fallback of msgid rather than lookup.
 */
const char* gettext_pgettext_impl( const char* lookup, const char* msgid )
{
   const char *trans = _( lookup );
   return trans==lookup ? msgid : trans;
}

/**
 * @brief Read the GETTEXT_STATS_PATH data and compute gettext_nstrings.
 * (Common case: just a "naev.txt" file with one number. But mods pulled in via PhysicsFS can have their own string counts.)
 */
static void gettext_readStats (void)
{
   char **paths = ndata_listRecursive( GETTEXT_STATS_PATH );

   for (int i=0; i<array_size(paths); i++) {
      size_t size;
      char *text = ndata_read( paths[i], &size );
      if (text != NULL)
         gettext_nstrings += strtoul( text, NULL, 10 );
      free( text );
      free( paths[i] );
   }
   array_free( paths );
}

/**
 * @brief List the available languages, with completeness statistics.
 * @return Array (array.h) of LanguageOptions.
 */
LanguageOption* gettext_languageOptions (void)
{
   LanguageOption *opts = array_create( LanguageOption );
   LanguageOption en = { .language = strdup("en"), .coverage = 1. };
   char **dirs = PHYSFS_enumerateFiles( GETTEXT_PATH );

   array_push_back( &opts, en );
   for (size_t i=0; dirs[i]!=NULL; i++) {
      LanguageOption *opt = &array_grow( &opts );
      opt->language = strdup( dirs[i] );
      opt->coverage = gettext_languageCoverage( dirs[i] );
   }
   PHYSFS_freeList( dirs );

   return opts;
}

/**
 * @brief Return the fraction of strings which have a translation into the given language.
 *
 * @param lang A single language code.
 */
double gettext_languageCoverage( const char* lang )
{
   uint32_t translated = 0;
   char **paths, dirpath[PATH_MAX], buf[12];

   /* We nail 100% of the translations we don't have to do. :) */
   if (!strcmp( lang, "en" ))
      return 1.;

   /* Initialize gettext_nstrings, if needed. */
   if (gettext_nstrings == 0)
      gettext_readStats();

   snprintf( dirpath, sizeof(dirpath), GETTEXT_PATH"%s", lang );
   paths = ndata_listRecursive( dirpath );
   for (int j=0; j<array_size(paths); j++) {
      PHYSFS_file *file;
      PHYSFS_sint64 size;
      file = PHYSFS_openRead( paths[j] );
      free( paths[j] );
      if (file == NULL)
         continue;
      size = PHYSFS_readBytes( file, buf, 12 );
      if (size < 12)
         continue;
      translated += msgcat_nstringsFromHeader( buf );
   }
   array_free( paths );
   return (double)translated / gettext_nstrings;
}

/* The function is almost the same as p_() but msgctxt and msgid can be string variables.
 */
const char* pgettext_var( const char* msgctxt, const char* msgid )
{
   char *lookup = NULL;
   const char* trans;
   SDL_asprintf( &lookup, "%s" GETTEXT_CONTEXT_GLUE "%s", msgctxt, msgid );
   trans = gettext_pgettext_impl( lookup, msgid );
   free( lookup );
   return trans;
}
