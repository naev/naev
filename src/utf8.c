/*
  Basic UTF-8 manipulation routines
  by Jeff Bezanson
  placed in the public domain Fall 2005

  This code is designed to provide the utilities you need to manipulate
  UTF-8 as an internal string encoding. These functions do not perform the
  error checking normally needed when handling UTF-8 data, so if you happen
  to be from the Unicode Consortium you will want to flay me alive.
  I do this because error checking can be performed at the boundaries (I/O),
  with these routines reserved for higher performance on data known to be
  valid.
  A UTF-8 validation routine is included.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <wchar.h>
#include <wctype.h>

#if HAS_WIN32
#include <malloc.h>
#endif /* HAS_WIN32 */
#include <assert.h>

#include "utf8.h"

#include "ncompat.h"
#include "nstring.h"

#if HAS_WIN32
/*
 * This is an implementation of wcwidth() and wcswidth() (defined in
 * IEEE Std 1002.1-2001) for Unicode.
 *
 * http://www.opengroup.org/onlinepubs/007904975/functions/wcwidth.html
 * http://www.opengroup.org/onlinepubs/007904975/functions/wcswidth.html
 *
 * In fixed-width output devices, Latin characters all occupy a single
 * "cell" position of equal width, whereas ideographic CJK characters
 * occupy two such cells. Interoperability between terminal-line
 * applications and (teletype-style) character terminals using the
 * UTF-8 encoding requires agreement on which character should advance
 * the cursor by how many cell positions. No established formal
 * standards exist at present on which Unicode character shall occupy
 * how many cell positions on character terminals. These routines are
 * a first attempt of defining such behavior based on simple rules
 * applied to data provided by the Unicode Consortium.
 *
 * For some graphical characters, the Unicode standard explicitly
 * defines a character-cell width via the definition of the East Asian
 * FullWidth (F), Wide (W), Half-width (H), and Narrow (Na) classes.
 * In all these cases, there is no ambiguity about which width a
 * terminal shall use. For characters in the East Asian Ambiguous (A)
 * class, the width choice depends purely on a preference of backward
 * compatibility with either historic CJK or Western practice.
 * Choosing single-width for these characters is easy to justify as
 * the appropriate long-term solution, as the CJK practice of
 * displaying these characters as double-width comes from historic
 * implementation simplicity (8-bit encoded characters were displayed
 * single-width and 16-bit ones double-width, even for Greek,
 * Cyrillic, etc.) and not any typographic considerations.
 *
 * Much less clear is the choice of width for the Not East Asian
 * (Neutral) class. Existing practice does not dictate a width for any
 * of these characters. It would nevertheless make sense
 * typographically to allocate two character cells to characters such
 * as for instance EM SPACE or VOLUME INTEGRAL, which cannot be
 * represented adequately with a single-width glyph. The following
 * routines at present merely assign a single-cell width to all
 * neutral characters, in the interest of simplicity. This is not
 * entirely satisfactory and should be reconsidered before
 * establishing a formal standard in this area. At the moment, the
 * decision which Not East Asian (Neutral) characters should be
 * represented by double-width glyphs cannot yet be answered by
 * applying a simple rule from the Unicode database content. Setting
 * up a proper standard for the behavior of UTF-8 character terminals
 * will require a careful analysis not only of each Unicode character,
 * but also of each presentation form, something the author of these
 * routines has avoided to do so far.
 *
 * http://www.unicode.org/unicode/reports/tr11/
 *
 * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

#include <wchar.h>

struct interval {
   int first;
   int last;
};

/* auxiliary function for binary search in interval table */
static int bisearch(wchar_t ucs, const struct interval *table, int max) {
   int min = 0;
   int mid;

   if (ucs < table[0].first || ucs > table[max].last)
      return 0;
   while (max >= min) {
      mid = (min + max) / 2;
      if (ucs > table[mid].last)
         min = mid + 1;
      else if (ucs < table[mid].first)
         max = mid - 1;
      else
         return 1;
   }

   return 0;
}


/* The following two functions define the column width of an ISO 10646
 * character as follows:
 *
 *    - The null character (U+0000) has a column width of 0.
 *
 *    - Other C0/C1 control characters and DEL will lead to a return
 *      value of -1.
 *
 *    - Non-spacing and enclosing combining characters (general
 *      category code Mn or Me in the Unicode database) have a
 *      column width of 0.
 *
 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
 *
 *    - Other format characters (general category code Cf in the Unicode
 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
 *
 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
 *      have a column width of 0.
 *
 *    - Spacing characters in the East Asian Wide (W) or East Asian
 *      Full-width (F) category as defined in Unicode Technical
 *      Report #11 have a column width of 2.
 *
 *    - All remaining characters (including all printable
 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
 *      etc.) have a column width of 1.
 *
 * This implementation assumes that wchar_t characters are encoded
 * in ISO 10646.
 */

static int wcwidth(wchar_t ucs)
{
   /* sorted list of non-overlapping intervals of non-spacing characters */
   /* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
   static const struct interval combining[] = {
      { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
      { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
      { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
      { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
      { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
      { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
      { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
      { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
      { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
      { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
      { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
      { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
      { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
      { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
      { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
      { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
      { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
      { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
      { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
      { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
      { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
      { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
      { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
      { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
      { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
      { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
      { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
      { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
      { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
      { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
      { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
      { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
      { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
      { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
      { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
      { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
      { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
      { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
      { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
      { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
      { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
      { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
      { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
      { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
      { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
      { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
      { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
      { 0xE0100, 0xE01EF }
   };

   /* test for 8-bit control characters */
   if (ucs == 0)
      return 0;
   if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
      return -1;

   /* binary search in table of non-spacing characters */
   if (bisearch(ucs, combining,
            sizeof(combining) / sizeof(struct interval) - 1))
      return 0;

   /* if we arrive here, ucs is not a combining or C0/C1 control character */

   return 1 + 
      (ucs >= 0x1100 &&
       (ucs <= 0x115f ||                    /* Hangul Jamo init. consonants */
        ucs == 0x2329 || ucs == 0x232a ||
        (ucs >= 0x2e80 && ucs <= 0xa4cf &&
         ucs != 0x303f) ||                  /* CJK ... Yi */
        (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
        (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
        (ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms */
        (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
        (ucs >= 0xff00 && ucs <= 0xff60) || /* Fullwidth Forms */
        (ucs >= 0xffe0 && ucs <= 0xffe6) ||
        (ucs >= 0x20000 && ucs <= 0x2fffd) ||
        (ucs >= 0x30000 && ucs <= 0x3fffd)));
}
#endif /* HAS_WIN32 */

static const uint32_t offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/* returns length of next utf-8 sequence */
size_t u8_seqlen(const char *s)
{
    return trailingBytesForUTF8[(unsigned int)(unsigned char)s[0]] + 1;
}

/* returns the # of bytes needed to encode a certain character
   0 means the character cannot (or should not) be encoded. */
size_t u8_charlen(uint32_t ch)
{
    if (ch < 0x80)
        return 1;
    else if (ch < 0x800)
        return 2;
    else if (ch < 0x10000)
        return 3;
    else if (ch < 0x110000)
        return 4;
    return 0;
}

size_t u8_codingsize(uint32_t *wcstr, size_t n)
{
    size_t i, c=0;

    for (i=0; i < n; i++)
        c += u8_charlen(wcstr[i]);
    return c;
}

/* conversions without error checking
   only works for valid UTF-8, i.e. no 5- or 6-byte sequences
   srcsz = source size in bytes
   sz = dest size in # of wide characters

   returns # characters converted
   if sz == srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space.
*/
size_t u8_toucs(uint32_t *dest, size_t sz, const char *src, size_t srcsz)
{
    uint32_t ch;
    const char *src_end = src + srcsz;
    size_t nb;
    size_t i=0;

    if (sz == 0 || srcsz == 0)
        return 0;

    while (i < sz) {
        if (!isutf(*src)) {     // invalid sequence
            dest[i++] = 0xFFFD;
            src++;
            if (src >= src_end) break;
            continue;
        }
        nb = trailingBytesForUTF8[(unsigned char)*src];
        if (src + nb >= src_end)
            break;
        ch = 0;
        switch (nb) {
            /* these fall through deliberately */
        case 5: ch += (unsigned char)*src++; ch <<= 6;
            /* Falls through. */
        case 4: ch += (unsigned char)*src++; ch <<= 6;
            /* Falls through. */
        case 3: ch += (unsigned char)*src++; ch <<= 6;
            /* Falls through. */
        case 2: ch += (unsigned char)*src++; ch <<= 6;
            /* Falls through. */
        case 1: ch += (unsigned char)*src++; ch <<= 6;
            /* Falls through. */
        case 0: ch += (unsigned char)*src++;
        }
        ch -= offsetsFromUTF8[nb];
        dest[i++] = ch;
    }
    return i;
}

/* srcsz = number of source characters
   sz = size of dest buffer in bytes

   returns # bytes stored in dest
   the destination string will never be bigger than the source string.
*/
size_t u8_toutf8(char *dest, size_t sz, const uint32_t *src, size_t srcsz)
{
    uint32_t ch;
    size_t i = 0;
    char *dest0 = dest;
    char *dest_end = dest + sz;

    while (i < srcsz) {
        ch = src[i];
        if (ch < 0x80) {
            if (dest >= dest_end)
                break;
            *dest++ = (char)ch;
        }
        else if (ch < 0x800) {
            if (dest >= dest_end-1)
                break;
            *dest++ = (ch>>6) | 0xC0;
            *dest++ = (ch & 0x3F) | 0x80;
        }
        else if (ch < 0x10000) {
            if (dest >= dest_end-2)
                break;
            *dest++ = (ch>>12) | 0xE0;
            *dest++ = ((ch>>6) & 0x3F) | 0x80;
            *dest++ = (ch & 0x3F) | 0x80;
        }
        else if (ch < 0x110000) {
            if (dest >= dest_end-3)
                break;
            *dest++ = (ch>>18) | 0xF0;
            *dest++ = ((ch>>12) & 0x3F) | 0x80;
            *dest++ = ((ch>>6) & 0x3F) | 0x80;
            *dest++ = (ch & 0x3F) | 0x80;
        }
        i++;
    }
    return (dest-dest0);
}

size_t u8_wc_toutf8(char *dest, uint32_t ch)
{
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch>>6) | 0xC0;
        dest[1] = (ch & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch>>12) | 0xE0;
        dest[1] = ((ch>>6) & 0x3F) | 0x80;
        dest[2] = (ch & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch>>18) | 0xF0;
        dest[1] = ((ch>>12) & 0x3F) | 0x80;
        dest[2] = ((ch>>6) & 0x3F) | 0x80;
        dest[3] = (ch & 0x3F) | 0x80;
        return 4;
    }
    return 0;
}

/* charnum => byte offset */
size_t u8_offset(const char *s, size_t charnum)
{
    size_t i=0;

    while (charnum > 0) {
        if (s[i++] & 0x80) {
            (void)(isutf(s[++i]) || isutf(s[++i]) || ++i);
        }
        charnum--;
    }
    return i;
}

/* byte offset => charnum */
size_t u8_charnum(const char *s, size_t offset)
{
    size_t charnum = 0, i=0;

    while (i < offset) {
        if (s[i++] & 0x80) {
            (void)(isutf(s[++i]) || isutf(s[++i]) || ++i);
        }
        charnum++;
    }
    return charnum;
}

/* number of characters in NUL-terminated string */
size_t u8_strlen(const char *s)
{
    size_t count = 0;
    size_t i = 0, lasti;

    while (1) {
        lasti = i;
        while (s[i] > 0)
            i++;
        count += (i-lasti);
        if (s[i++]==0) break;
        (void)(isutf(s[++i]) || isutf(s[++i]) || ++i);
        count++;
    }
    return count;
}

size_t u8_strwidth(const char *s)
{
    uint32_t ch;
    size_t nb, tot=0;
    int w;
    signed char sc;

    while ((sc = (signed char)*s) != 0) {
        if (sc >= 0) {
            s++;
            if (sc) tot++;
        }
        else {
            if (!isutf(sc)) { tot++; s++; continue; }
            nb = trailingBytesForUTF8[(unsigned char)sc];
            ch = 0;
            switch (nb) {
                /* these fall through deliberately */
            case 5: ch += (unsigned char)*s++; ch <<= 6;
               /* Falls through. */
            case 4: ch += (unsigned char)*s++; ch <<= 6;
               /* Falls through. */
            case 3: ch += (unsigned char)*s++; ch <<= 6;
               /* Falls through. */
            case 2: ch += (unsigned char)*s++; ch <<= 6;
               /* Falls through. */
            case 1: ch += (unsigned char)*s++; ch <<= 6;
               /* Falls through. */
            case 0: ch += (unsigned char)*s++;
            }
            ch -= offsetsFromUTF8[nb];
            w = wcwidth(ch);  // might return -1
            if (w > 0) tot += w;
        }
    }
    return tot;
}

/* reads the next utf-8 sequence out of a string, updating an index */
uint32_t u8_nextchar(const char *s, size_t *i)
{
    uint32_t ch = 0;
    size_t sz = 0;

    do {
        ch <<= 6;
        ch += (unsigned char)s[(*i)];
        sz++;
    } while (s[*i] && (++(*i)) && !isutf(s[*i]));
    ch -= offsetsFromUTF8[sz-1];

    return ch;
}

/* next character without NUL character terminator */
uint32_t u8_nextmemchar(const char *s, size_t *i)
{
    uint32_t ch = 0;
    size_t sz = 0;

    do {
        ch <<= 6;
        ch += (unsigned char)s[(*i)++];
        sz++;
    } while (!isutf(s[*i]));
    ch -= offsetsFromUTF8[sz-1];

    return ch;
}

void u8_inc(const char *s, size_t *i)
{
    (void)(isutf(s[++(*i)]) || isutf(s[++(*i)]) || isutf(s[++(*i)]) || ++(*i));
}

void u8_dec(const char *s, size_t *i)
{
    (void)(isutf(s[--(*i)]) || isutf(s[--(*i)]) || isutf(s[--(*i)]) || --(*i));
}

int octal_digit(char c)
{
    return (c >= '0' && c <= '7');
}

int hex_digit(char c)
{
    return ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f'));
}

char read_escape_control_char(char c)
{
    if (c == 'n')
        return '\n';
    else if (c == 't')
        return '\t';
    else if (c == 'r')
        return '\r';
    else if (c == 'e')
        return 033; // '\e'
    else if (c == 'b')
        return '\b';
    else if (c == 'f')
        return '\f';
    else if (c == 'v')
        return '\v';
    else if (c == 'a')
        return '\a';
    return c;
}

/* assumes that src points to the character after a backslash
   returns number of input characters processed, 0 if error */
size_t u8_read_escape_sequence(const char *str, size_t ssz, uint32_t *dest)
{
    uint32_t ch;
    char digs[10];
    int dno=0, ndig;
    size_t i=1;
    char c0 = str[0];
    assert(ssz > 0);

    if (octal_digit(c0)) {
        i = 0;
        do {
            digs[dno++] = str[i++];
        } while (i<ssz && octal_digit(str[i]) && dno<3);
        digs[dno] = '\0';
        ch = strtol(digs, NULL, 8);
    }
    else if ((c0=='x' && (ndig=2)) ||
             (c0=='u' && (ndig=4)) ||
             (c0=='U' && (ndig=8))) {
        while (i<ssz && hex_digit(str[i]) && dno<ndig) {
            digs[dno++] = str[i++];
        }
        if (dno == 0) return 0;
        digs[dno] = '\0';
        ch = strtol(digs, NULL, 16);
    }
    else {
        ch = (uint32_t)read_escape_control_char(c0);
    }
    *dest = ch;

    return i;
}

/* convert a string with literal \uxxxx or \Uxxxxxxxx characters to UTF-8
   example: u8_unescape(mybuf, 256, "hello\\u220e")
   note the double backslash is needed if called on a C string literal */
size_t u8_unescape(char *buf, size_t sz, const char *src)
{
    size_t c=0, amt;
    uint32_t ch = 0;
    char temp[4];

    while (*src && c < sz) {
        if (*src == '\\') {
            src++;
            amt = u8_read_escape_sequence(src, 1000, &ch);
        }
        else {
            ch = (uint32_t)*src;
            amt = 1;
        }
        src += amt;
        amt = u8_wc_toutf8(temp, ch);
        if (amt > sz-c)
            break;
        memcpy(&buf[c], temp, amt);
        c += amt;
    }
    if (c < sz)
        buf[c] = '\0';
    return c;
}

static int buf_put2c(char *buf, const char *src)
{
    buf[0] = src[0];
    buf[1] = src[1];
    buf[2] = '\0';
    return 2;
}

int u8_escape_wchar(char *buf, size_t sz, uint32_t ch)
{
    assert(sz > 2);
    if (ch == L'\n')
        return buf_put2c(buf, "\\n");
    else if (ch == L'\t')
        return buf_put2c(buf, "\\t");
    else if (ch == L'\r')
        return buf_put2c(buf, "\\r");
    else if (ch == 033) // L'\e'
        return buf_put2c(buf, "\\a");
    else if (ch == L'\b')
        return buf_put2c(buf, "\\b");
    else if (ch == L'\f')
        return buf_put2c(buf, "\\f");
    else if (ch == L'\v')
        return buf_put2c(buf, "\\v");
    else if (ch == L'\a')
        return buf_put2c(buf, "\\a");
    else if (ch == L'\\')
        return buf_put2c(buf, "\\\\");
    else if (ch < 32 || ch == 0x7f)
        return nsnprintf(buf, sz, "\\x%.2hhx", (unsigned char)ch);
    else if (ch > 0xFFFF)
        return nsnprintf(buf, sz, "\\U%.8x", (uint32_t)ch);
    else if (ch >= 0x80)
        return nsnprintf(buf, sz, "\\u%.4hx", (unsigned short)ch);

    buf[0] = (char)ch;
    buf[1] = '\0';
    return 1;
}

size_t u8_escape(char *buf, size_t sz, const char *src, size_t *pi, size_t end,
                 int escape_quotes, int ascii)
{
    size_t i = *pi, i0;
    uint32_t ch;
    char *start = buf;
    char *blim = start + sz-11;
    assert(sz > 11);

    while (i<end && buf<blim) {
        // sz-11: leaves room for longest escape sequence
        if (escape_quotes && src[i] == '"') {
            buf += buf_put2c(buf, "\\\"");
            i++;
        }
        else if (src[i] == '\\') {
            buf += buf_put2c(buf, "\\\\");
            i++;
        }
        else {
            i0 = i;
            ch = u8_nextmemchar(src, &i);
            if (ascii || !iswprint((wint_t)ch)) {
                buf += u8_escape_wchar(buf, sz - (buf-start), ch);
            }
            else {
                i = i0;
                do {
                    *buf++ = src[i++];
                } while (!isutf(src[i]));
            }
        }
    }
    *buf++ = '\0';
    *pi = i;
    return (buf-start);
}

char *u8_strchr(const char *s, uint32_t ch, size_t *charn)
{
    size_t i = 0, lasti=0;
    uint32_t c;

    *charn = 0;
    while (s[i]) {
        c = u8_nextchar(s, &i);
        if (c == ch) {
            /* it's const for us, but not necessarily the caller */
            return (char*)&s[lasti];
        }
        lasti = i;
        (*charn)++;
    }
    return NULL;
}

char *u8_memchr(const char *s, uint32_t ch, size_t sz, size_t *charn)
{
    size_t i = 0, lasti=0;
    uint32_t c;
    int csz;

    *charn = 0;
    while (i < sz) {
        c = csz = 0;
        do {
            c <<= 6;
            c += (unsigned char)s[i++];
            csz++;
        } while (i < sz && !isutf(s[i]));
        c -= offsetsFromUTF8[csz-1];

        if (c == ch) {
            return (char*)&s[lasti];
        }
        lasti = i;
        (*charn)++;
    }
    return NULL;
}

char *u8_memrchr(const char *s, uint32_t ch, size_t sz)
{
    size_t i = sz-1, tempi=0;
    uint32_t c;

    if (sz == 0) return NULL;

    while (i && !isutf(s[i])) i--;

    while (1) {
        tempi = i;
        c = u8_nextmemchar(s, &tempi);
        if (c == ch) {
            return (char*)&s[i];
        }
        if (i == 0)
            break;
        tempi = i;
        u8_dec(s, &i);
        if (i > tempi)
            break;
    }
    return NULL;
}

int u8_is_locale_utf8(const char *locale)
{
    /* this code based on libutf8 */
    const char* cp = locale;

    if (locale == NULL) return 0;

    for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++) {
        if (*cp == '.') {
            const char* encoding = ++cp;
            for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; cp++)
                ;
            if ((cp-encoding == 5 && !strncmp(encoding, "UTF-8", 5))
                || (cp-encoding == 4 && !strncmp(encoding, "utf8", 4)))
                return 1; /* it's UTF-8 */
            break;
        }
    }
    return 0;
}

size_t u8_vprintf(const char *fmt, va_list ap)
{
    int cnt, sz=0, nc, needfree=0;
    char *buf;
    uint32_t *wcs;

    sz = 512;
    buf = (char*)alloca(sz);
    cnt = vsnprintf(buf, sz, fmt, ap);
    if (cnt < 0)
        return 0;
    if (cnt >= sz) {
        buf = (char*)malloc(cnt + 1);
        needfree = 1;
        vsnprintf(buf, cnt+1, fmt, ap);
    }
    wcs = (uint32_t*)alloca((cnt+1) * sizeof(uint32_t));
    nc = u8_toucs(wcs, (size_t)cnt+1, buf, cnt);
    wcs[nc] = 0;
    printf("%ls", (wchar_t*)wcs);
    if (needfree) free(buf);
    return nc;
}

size_t u8_printf(const char *fmt, ...)
{
    size_t cnt;
    va_list args;

    va_start(args, fmt);

    cnt = u8_vprintf(fmt, args);

    va_end(args);
    return cnt;
}

/* based on the valid_utf8 routine from the PCRE library by Philip Hazel

   length is in bytes, since without knowing whether the string is valid
   it's hard to know how many characters there are! */
int u8_isvalid(const char *str, size_t length)
{
    const unsigned char *p, *pend = (unsigned char*)str + length;
    unsigned char c;
    int ret = 1; /* ASCII */
    size_t ab;

    for (p = (unsigned char*)str; p < pend; p++) {
        c = *p;
        if (c < 128)
            continue;
        ret = 2; /* non-ASCII UTF-8 */
        if ((c & 0xc0) != 0xc0)
            return 0;
        ab = trailingBytesForUTF8[c];
        if (length < ab)
            return 0;
        length -= ab;

        p++;
        /* Check top bits in the second byte */
        if ((*p & 0xc0) != 0x80)
            return 0;

        /* Check for overlong sequences for each different length */
        switch (ab) {
            /* Check for xx00 000x */
        case 1:
            if ((c & 0x3e) == 0) return 0;
            continue;   /* We know there aren't any more bytes to check */

            /* Check for 1110 0000, xx0x xxxx */
        case 2:
            if (c == 0xe0 && (*p & 0x20) == 0) return 0;
            break;

            /* Check for 1111 0000, xx00 xxxx */
        case 3:
            if (c == 0xf0 && (*p & 0x30) == 0) return 0;
            break;

            /* Check for 1111 1000, xx00 0xxx */
        case 4:
            if (c == 0xf8 && (*p & 0x38) == 0) return 0;
            break;

            /* Check for leading 0xfe or 0xff,
               and then for 1111 1100, xx00 00xx */
        case 5:
            if (c == 0xfe || c == 0xff ||
                (c == 0xfc && (*p & 0x3c) == 0)) return 0;
            break;
        }

        /* Check for valid bytes after the 2nd, if any; all must start 10 */
        while (--ab > 0) {
            if ((*(++p) & 0xc0) != 0x80) return 0;
        }
    }

    return ret;
}

int u8_reverse(char *dest, char * src, size_t len)
{
    size_t si=0, di=len;
    unsigned char c;

    dest[di] = '\0';
    while (si < len) {
        c = (unsigned char)src[si];
        if ((~c) & 0x80) {
            di--;
            dest[di] = c;
            si++;
        }
        else {
            switch (c>>4) {
            case 0xC:
            case 0xD:
                di -= 2;
                memcpy(&dest[di], &src[si], sizeof(int16_t));
                si += 2;
                break;
            case 0xE:
                di -= 3;
                dest[di] = src[si];
                memcpy(&dest[di+1], &src[si+1], sizeof(int16_t));
                si += 3;
                break;
            case 0xF:
                di -= 4;
                memcpy(&dest[di], &src[si], sizeof(int32_t));
                si += 4;
                break;
            default:
                return 1;
            }
        }
    }
    return 0;
}
