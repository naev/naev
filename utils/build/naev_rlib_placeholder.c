/* Meson needs a library() for the executable to link against, but the Rust code
 * arrives as libnaev.rlib rather than as a source, so that link gets no inputs
 * at all. cctools ld64 emits an empty __text section for such a link anyway.
 * ld64.lld emits a Mach-O with no sections, and rcodesign will not sign one. */
int naev_rlib_placeholder( void );
int naev_rlib_placeholder( void )
{
   return 0;
}
