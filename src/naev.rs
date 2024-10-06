use std::ffi::CString;
use std::io::Result;
use std::os::raw::{c_char, c_int};

#[link(name = "naev")]
extern "C" {
    /// Main function in C
    pub fn naev_main(argc: c_int, argv: *mut *mut c_char) -> c_int;
}

mod array;
mod ndata;
mod slots;

#[allow(dead_code)]
pub fn naev() -> Result<()> {
    let args: Vec<String> = std::env::args().collect();
    let mut cargs = vec![];
    for a in args {
        cargs.push(CString::new(a).unwrap())
    }
    let mut argv = cargs.into_iter().map(|s| s.into_raw()).collect::<Vec<_>>();
    argv.shrink_to_fit();

    unsafe {
        naev_main(argv.len() as c_int, argv.as_mut_ptr());
    };
    Ok(())
}
