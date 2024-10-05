use std::os::raw::{c_int, c_char};

#[link(name = "naev")]
extern "C" {
    pub fn naev_main( argc: c_int, argv: *mut *mut c_char ) -> c_int;
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

    println!("SDL_log(16) is {}", naevc::SDL_log(16) );

    unsafe{
        naev_main( argv.len() as c_int, argv.as_mut_ptr() );
    }
}
