#ifndef UTF8_H
#define UTF8_H

#include <stdlib.h>
#include <stdint.h>

/** 
 * @file utf8.h
 * Utilities from cutef8: https://github.com/JeffBezanson/cutef8
 * @note Its current README says, "I now use and recommend utf8proc instead of this library."
 * @note Some unused functions (and supporting polyfills, macro tweaks) have been removed.
 * In the unlikely event more cutef8 features are required, check earlier revisions in git. */

#include "nstring.h"

/* is c the start of a utf8 sequence? */
#define isutf(c) (((c)&0xC0)!=0x80)

#define UEOF ((uint32_t)-1)

/* convert UTF-8 data to wide character */
size_t u8_toucs(uint32_t *dest, size_t sz, const char *src, size_t srcsz);

/* the opposite conversion */
size_t u8_toutf8(char *dest, size_t sz, const uint32_t *src, size_t srcsz);

/* single character to UTF-8, returns # bytes written */
size_t u8_wc_toutf8(char *dest, uint32_t ch);

/* character number to byte offset */
size_t u8_offset(const char *str, size_t charnum);

/* byte offset to character number */
size_t u8_charnum(const char *s, size_t offset);

/* return next character, updating an index variable */
uint32_t u8_nextchar(const char *s, size_t *i);

/* next character without NUL character terminator */
uint32_t u8_nextmemchar(const char *s, size_t *i);

/* move to next character */
void u8_inc(const char *s, size_t *i);

/* move to previous character */
void u8_dec(const char *s, size_t *i);

/* returns length of next utf-8 sequence */
size_t u8_seqlen(const char *s);

/* returns the # of bytes needed to encode a certain character */
size_t u8_charlen(uint32_t ch);

/* computes the # of bytes needed to encode a WC string as UTF-8 */
size_t u8_codingsize(uint32_t *wcstr, size_t n);

char read_escape_control_char(char c);

/* assuming src points to the character after a backslash, read an
   escape sequence, storing the result in dest and returning the number of
   input characters processed */
size_t u8_read_escape_sequence(const char *src, size_t ssz, uint32_t *dest);

/* convert a string "src" containing escape sequences to UTF-8 */
size_t u8_unescape(char *buf, size_t sz, const char *src);

/* utility predicates used by the above */
int octal_digit(char c);
int hex_digit(char c);

/* return a pointer to the first occurrence of ch in s, or NULL if not
   found. character index of found character returned in *charn. */
char *u8_strchr(const char *s, uint32_t ch, size_t *charn);

/* same as the above, but searches a buffer of a given size instead of
   a NUL-terminated string. */
char *u8_memchr(const char *s, uint32_t ch, size_t sz, size_t *charn);

char *u8_memrchr(const char *s, uint32_t ch, size_t sz);

/* count the number of characters in a UTF-8 string */
size_t u8_strlen(const char *s);

/* determine whether a sequence of bytes is valid UTF-8. length is in bytes */
int u8_isvalid(const char *str, size_t length);

/* reverse a UTF-8 string. len is length in bytes. dest and src must both
   be allocated to at least len+1 bytes. returns 1 for error, 0 otherwise */
int u8_reverse(char *dest, char *src, size_t len);

#endif
