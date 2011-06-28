/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NSTD_H
#  define NSTD_H


#include <ctype.h>

#include "SDL.h"


/**
 * @brief Checks to see if k is in ascii area.
 */
#define nstd_checkascii(k)       ((k & 0xff) == k)


/**
 * @brief Checks to see if a key is alpha.
 */
#define nstd_isalpha(k)    (nstd_checkascii(k) ? isalpha(k) : 0)
/**
 * @brief Checks to see if a key is alphanumeric.
 */
#define nstd_isalnum(k)    (nstd_checkascii(k) ? isalnum(k) : 0)
/**
 * @brief Checks to see if a key is a control character.
 */
#define nstd_iscntrl(k)    (nstd_checkascii(k) ? iscntrl(k) : 0)
/**
 * @brief Checks to see if a key is a space character.
 */
#define nstd_isspace(k)    (nstd_checkascii(k) ? isspace(k) : 0)
/**
 * @brief Checks to see if a character is printable.
 */
#define nstd_isgraph(k)    (nstd_checkascii(k) ? isgraph(k) : 0)


/**
 * @brief Converts a key to lowercase if applicable.
 */
#define nstd_tolower(k)    (nstd_checkascii(k) ? (SDLKey)tolower(k) : k)
/**
 * @brief Converts a key to uppercase if applicable.
 */
#define nstd_toupper(k)    (nstd_checkascii(k) ? (SDLKey)toupper(k) : k)


/* Adds STRCASECMP. */
#if HAS_WIN32
#include <shlwapi.h>
#define STRCASECMP      lstrcmpiA
#else /* HAS_WIN32 */
#include <strings.h>
#define STRCASECMP      strcasecmp
#endif /* HAS_WIN32 */


#endif /* NSTD_H */
