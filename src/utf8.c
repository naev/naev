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

/** @cond */
#if HAVE_ALLOCA_H 
#   include <alloca.h> /* Not available in windows, necessary for linux. */
#endif /* HAVE_ALLOCA_H */
#include <assert.h>
#if HAVE_MALLOC_H
#   include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include <stdio.h>
#include <string.h>
/** @endcond */

#include "utf8.h"

#include "ncompat.h"

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
        return '#';
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
