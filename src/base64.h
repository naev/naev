/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef BASE64_H
#  define BASE64_H


#include <stddef.h>


char* base64_encode( size_t *len, const char *src, size_t sz );
char* base64_decode( size_t *len, const char *src, size_t sz );
char* base64_encode_to_cstr( const char *src, size_t sz );
char* base64_decode_cstr( size_t *len, const char *src );


#endif /* BASE64_H */
