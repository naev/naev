

#ifndef BASE64_H
#  define BASE64_H


char* base64_encode( int *len, char *src, size_t sz );
char* base64_decode( int *len, char *src, size_t sz );


#endif /* BASE64_H */
