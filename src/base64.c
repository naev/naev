

#include <malloc.h>
#include <string.h>
#include <stdint.h>



/* encode table */
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/* decode table */
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";


/*
 * encodes src of sz length storing the new length in len
 */
char* base64_encode( int *len, char *src, size_t sz )
{
   char *r;
   size_t i, c;
   uint32_t n;
   uint8_t ch[4], pad;

   /* create r */
   c = sz * 4 / 3 + sz % 3;
   c += c / 76;
   (*len) = c;
   r = malloc( (*len) * sizeof(char) );

   /* setup padding */
   pad = sz % 3;
   
   /* time to do the bulk work */
   i = 0;
   for (c=0; c<sz; c+=3) {

      /* specification wants newline after every 76 characters */
      if ((c > 0) && ((c / 3 * 4) % 76 == 0))
         r[i++] = '\n';

      /* n is 24 bits */
      n =   (src[c] << 16) +
            (c+1<sz) ? (src[c+1] << 8) : 0 + /* may be out of range */
            (c+2<sz) ? src[c+2] : 0; /* may be out of range */
     
      /* ch[0-3] are 6 bits each */
      ch[0] = (n >> 18) & 63;
      ch[1] = (n >> 12) & 63;
      ch[2] = (n >> 6) & 63;
      ch[3] = n & 63;

      /* add to str */
      r[i++] = cb64[ ch[0] ];
      r[i++] = cb64[ ch[1] ];
      r[i++] = cb64[ ch[2] ];
      r[i++] = cb64[ ch[3] ];
   }

   for (c=0; c<pad; c++)
      r[i++] = '=';
   r[i] = '\0';

   return r;
}


inline int dec_valid( char inp )
{
   if ((inp < 43) || (inp > 122) || cd64[ (int)inp-43 ] == '$')
      return 0;
   return 1;
}
inline char dec_ch( char inp )
{
   return cd64[ (int)inp-43 ] - 61;
}
char* base64_decode( int *len, char *src, size_t sz )
{
   char *r, *dat;
   size_t c,i,j;
   uint32_t n;

   /* allocate r */
   c = sz * 3 / 4;
   r = malloc( c * sizeof(char) );

   /* create a clean version of the text */
   dat = malloc( sz * sizeof(char) );
   j = 0;
   for (i=0; i<sz; i++) {
      if (src[i] == '=')
         dat[j++] = '\0';
      else if (dec_valid( src[i] ))
         dat[j++] = src[i];
   }

   /* fill r */
   i = 0;
   for (c=0; c<j; c+=4) {

      n = dec_ch( dat[c+0] ) << 18;
      n = dec_ch( dat[c+1] ) << 12;
      n = dec_ch( dat[c+2] ) << 6;
      n = dec_ch( dat[c+3] ) << 0;

      r[i++] = (n >> 16) & 0xFF;
      r[i++] = (n >> 8) & 0xFF;
      r[i++] = (n >> 0) & 0xFF;
   }

   (*len) = i;
   return r;
}



