/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef NSTD_H
#  define NSTD_H


#include <ctype.h>


/**
 * @brief Checks to see if k is in ascii area.
 */
#define nstd_checkascii(k)       ((k & 0xff) == k)


/**
 * @brief Checks to see if a key is alpha.
 */
#define nstd_isalpha(k) (nstd_checkascii(k) ? isalpha(k) : 0)
/**
 * @brief Checks to see if a key is alphanumeric.
 */
#define nstd_isalnum(k) (nstd_checkascii(k) ? isalnum(k) : 0)
/**
 * @brief Checks to see if a key is a control character.
 */
#define nstd_iscntrl(k) (nstd_checkascii(k) ? iscntrl(k) : 0)


#endif /* NSTD_H */
