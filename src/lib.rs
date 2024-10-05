#[link(name = "naev")]
extern "C" {
    pub fn naev_main(
        argc: ::std::os::raw::c_int,
        argv: *mut *mut ::std::os::raw::c_char,
    ) -> ::std::os::raw::c_int;
}

use std::env;
use std::ffi::CString;

#[allow(dead_code)]
pub fn naev() {
    let args: Vec<String> = env::args().collect();
    let mut cargs = vec![];
    for a in args {
        cargs.push( CString::new(a).unwrap() )
    }
    let mut argv = cargs
        .into_iter()
        .map(|s| s.into_raw() )
        .collect::<Vec<_>>();
    argv.shrink_to_fit();

    unsafe{
        naev_main( argv.len() as ::std::os::raw::c_int, argv.as_mut_ptr() );
    }
}
