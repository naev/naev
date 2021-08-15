/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file msgcat.c
 *
 * @brief Message catalog lookup and plural-form evaluation subroutines.
 *        This implementation comes from musl. See below for details and copyright info.
 */


/* musl as a whole is licensed under the following standard MIT license:
 *
 * ----------------------------------------------------------------------
 * Copyright © 2005-2020 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ----------------------------------------------------------------------
 * */

/** @cond */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
/** @endcond */

#include "msgcat.h"


/* Internal implementations, corresponding to Musl's __pleval and __mo_lookup. */
static uint64_t msgcat_plural_eval( const char *, uint64_t );
static const char* msgcat_mo_lookup( const void *p, size_t size, const char *s );


/* ==================== https://git.musl-libc.org/cgit/musl/tree/src/locale/dcngettext.c ===================== */
/* (The code in this section is heavily modified for Naev's use, but that's its origin.) */

/**
 * @brief Initialize a msgcat_t, given the contents and content-length of a .mo file.
 */
void msgcat_init( msgcat_t* p, const void* map, size_t map_size )
{
   p->map = map;
   p->map_size = map_size;

   const char *rule = "n!=1;";
   uint64_t np = 2;
   const char *r = msgcat_mo_lookup(p->map, p->map_size, "");
   char *z;
   while (r && strncmp(r, "Plural-Forms:", 13)) {
      z = strchr(r, '\n');
      r = z ? z+1 : 0;
   }
   if (r) {
      r += 13;
      while (isspace(*r)) r++;
      if (!strncmp(r, "nplurals=", 9)) {
         np = strtoul(r+9, &z, 10);
         r = z;
      }
      while (*r && *r != ';') r++;
      if (*r) {
         r++;
         while (isspace(*r)) r++;
         if (!strncmp(r, "plural=", 7))
            rule = r+7;
      }
   }
   p->nplurals = np;
   p->plural_rule = rule;
}

/**
 * @brief Return a translation, if present, from the given message catalog.
 *
 * @param p The message catalog.
 * @param msgid1 The English singular form.
 * @param msgid2 The English plural form. (Pass NULL if simply translating \p msgid1.)
 * @param n The number determining the plural form to use. (Pass 1 if simply translating \p msgid1.)
 * @return The translation in the message catalog, if it exists, else NULL.
 *         (\p msgid1 is \em not passed through; the higher-level gettext.c functions handle fallbacks.)
 */
const char* msgcat_ngettext( const msgcat_t* p, const char* msgid1, const char* msgid2, uint64_t n )
{
   const char *trans = msgcat_mo_lookup(p->map, p->map_size, msgid1);
   if (!trans) return NULL;

   /* Non-plural-processing gettext forms pass a null pointer as
    * msgid2 to request that dcngettext suppress plural processing. */

   if (msgid2 && p->nplurals) {
      uint64_t plural = msgcat_plural_eval(p->plural_rule, n);
      if (plural > p->nplurals) return NULL;
      while (plural--) {
         size_t rem = p->map_size - (trans - (char *)p->map);
         size_t l = strnlen(trans, rem);
         if (l+1 >= rem)
            return NULL;
         trans += l+1;
      }
   }
   return trans;
}



/* ===================== https://git.musl-libc.org/cgit/musl/tree/src/locale/__mo_lookup.c =================== */
static inline uint32_t swapc(uint32_t x, int c)
{
	return c ? (x>>24) | (x>>8&0xff00) | (x<<8&0xff0000) | (x<<24) : x;
}

const char *msgcat_mo_lookup(const void *p, size_t size, const char *s)
{
	const uint32_t *mo = p;
	int sw = *mo - 0x950412de;
	uint32_t b = 0, n = swapc(mo[2], sw);
	uint32_t o = swapc(mo[3], sw);
	uint32_t t = swapc(mo[4], sw);
	if (n>=size/4 || o>=size-4*n || t>=size-4*n || ((o|t)%4))
		return 0;
	o/=4;
	t/=4;
	for (;;) {
		uint32_t ol = swapc(mo[o+2*(b+n/2)], sw);
		uint32_t os = swapc(mo[o+2*(b+n/2)+1], sw);
		if (os >= size || ol >= size-os || ((char *)p)[os+ol])
			return 0;
		int sign = strcmp(s, (char *)p + os);
		if (!sign) {
			uint32_t tl = swapc(mo[t+2*(b+n/2)], sw);
			uint32_t ts = swapc(mo[t+2*(b+n/2)+1], sw);
			if (ts >= size || tl >= size-ts || ((char *)p)[ts+tl])
				return 0;
			return (char *)p + ts;
		}
		else if (n == 1) return 0;
		else if (sign < 0)
			n /= 2;
		else {
			b += n/2;
			n -= n/2;
		}
	}
	return 0;
}


/* ===================== https://git.musl-libc.org/cgit/musl/tree/src/locale/pleval.c ======================== */
/*
grammar:

Start = Expr ';'
Expr  = Or | Or '?' Expr ':' Expr
Or    = And | Or '||' And
And   = Eq | And '&&' Eq
Eq    = Rel | Eq '==' Rel | Eq '!=' Rel
Rel   = Add | Rel '<=' Add | Rel '>=' Add | Rel '<' Add | Rel '>' Add
Add   = Mul | Add '+' Mul | Add '-' Mul
Mul   = Prim | Mul '*' Prim | Mul '/' Prim | Mul '%' Prim
Prim  = '(' Expr ')' | '!' Prim | decimal | 'n'

internals:

recursive descent expression evaluator with stack depth limit.
for binary operators an operator-precedence parser is used.
eval* functions store the result of the parsed subexpression
and return a pointer to the next non-space character.
*/

struct st {
	uint64_t r;
	uint64_t n;
	int op;
};

static const char *skipspace(const char *s)
{
	while (isspace(*s)) s++;
	return s;
}

static const char *evalexpr(struct st *st, const char *s, int d);

static const char *evalprim(struct st *st, const char *s, int d)
{
	char *e;
	if (--d < 0) return "";
	s = skipspace(s);
	if (isdigit(*s)) {
		st->r = strtoul(s, &e, 10);
		if (e == s || st->r == UINT64_MAX) return "";
		return skipspace(e);
	}
	if (*s == 'n') {
		st->r = st->n;
		return skipspace(s+1);
	}
	if (*s == '(') {
		s = evalexpr(st, s+1, d);
		if (*s != ')') return "";
		return skipspace(s+1);
	}
	if (*s == '!') {
		s = evalprim(st, s+1, d);
		st->r = !st->r;
		return s;
	}
	return "";
}

static int binop(struct st *st, int op, uint64_t left)
{
	uint64_t a = left, b = st->r;
	switch (op) {
	case 0: st->r = a||b; return 0;
	case 1: st->r = a&&b; return 0;
	case 2: st->r = a==b; return 0;
	case 3: st->r = a!=b; return 0;
	case 4: st->r = a>=b; return 0;
	case 5: st->r = a<=b; return 0;
	case 6: st->r = a>b; return 0;
	case 7: st->r = a<b; return 0;
	case 8: st->r = a+b; return 0;
	case 9: st->r = a-b; return 0;
	case 10: st->r = a*b; return 0;
	case 11: if (b) {st->r = a%b; return 0;} return 1;
	case 12: if (b) {st->r = a/b; return 0;} return 1;
	}
	return 1;
}

static const char *parseop(struct st *st, const char *s)
{
	static const char opch[11] = "|&=!><+-*%/";
	static const char opch2[6] = "|&====";
	int i;
	for (i=0; i<11; i++)
		if (*s == opch[i]) {
			/* note: >,< are accepted with or without = */
			if (i<6 && s[1] == opch2[i]) {
				st->op = i;
				return s+2;
			}
			if (i>=4) {
				st->op = i+2;
				return s+1;
			}
			break;
		}
	st->op = 13;
	return s;
}

static const char *evalbinop(struct st *st, const char *s, int minprec, int d)
{
	static const char prec[14] = {1,2,3,3,4,4,4,4,5,5,6,6,6,0};
	uint64_t left;
	int op;
	d--;
	s = evalprim(st, s, d);
	s = parseop(st, s);
	for (;;) {
		/*
		st->r (left hand side value) and st->op are now set,
		get the right hand side or back out if op has low prec,
		if op was missing then prec[op]==0
		*/
		op = st->op;
		if (prec[op] <= minprec)
			return s;
		left = st->r;
		s = evalbinop(st, s, prec[op], d);
		if (binop(st, op, left))
			return "";
	}
}

static const char *evalexpr(struct st *st, const char *s, int d)
{
	uint64_t a, b;
	if (--d < 0)
		return "";
	s = evalbinop(st, s, 0, d);
	if (*s != '?')
		return s;
	a = st->r;
	s = evalexpr(st, s+1, d);
	if (*s != ':')
		return "";
	b = st->r;
	s = evalexpr(st, s+1, d);
	st->r = a ? b : st->r;
	return s;
}

uint64_t msgcat_plural_eval(const char *s, uint64_t n)
{
	struct st st;
	st.n = n;
	s = evalexpr(&st, s, 100);
	return *s == ';' ? st.r : UINT64_MAX;
}
