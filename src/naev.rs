use std::ffi::CString;
use std::io::{Error, ErrorKind, Result};
use std::os::raw::{c_char, c_int};

#[link(name = "naev")]
extern "C" {
    /// Main function in C
    pub fn naev_main(argc: c_int, argv: *mut *mut c_char) -> c_int;
}

mod array;
mod env;
mod gettext;
mod linebreak;
mod ndata;
mod ntime;
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
        naevc::log_init();

        let argv0 = CString::new(env::ENV.argv0.clone()).unwrap();
        match naevc::PHYSFS_init(argv0.as_ptr() as *const c_char) {
            0 => {
                let err = ndata::physfs_error_as_io_error();
                println!("{}", err);
                return Err(Error::new(ErrorKind::Other, err));
                /*
                SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                    _( "Naev Critical Error" ), buf,
                    gl_screen.window );
                */
            }
            _ => (),
        }
        naevc::PHYSFS_permitSymbolicLinks(1);

        /* Set up locales. */
        naevc::gettext_init();
        linebreak::init();
    }

    unsafe {
        naev_main(argv.len() as c_int, argv.as_mut_ptr());
    };
    Ok(())
}
