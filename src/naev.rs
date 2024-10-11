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
mod log;
mod ndata;
mod ntime;
mod slots;
mod version;

pub fn naev() -> Result<()> {
    /* Load up the argv and argc for the C main. */
    let args: Vec<String> = std::env::args().collect();
    let mut cargs = vec![];
    for a in args {
        cargs.push(CString::new(a).unwrap())
    }
    let mut argv = cargs.into_iter().map(|s| s.into_raw()).collect::<Vec<_>>();
    argv.shrink_to_fit();

    /* Begin logging infrastructure. */
    log::init();

    /* Start up PHYSFS. */
    unsafe {
        let argv0 = CString::new(env::ENV.argv0.clone()).unwrap();
        match naevc::PHYSFS_init(argv0.as_ptr() as *const c_char) {
            0 => {
                let err = ndata::physfs_error_as_io_error();
                println!("{}", err);
                return Err(Error::new(ErrorKind::Other, err));
                /* TODO probably move the error handling to the "real" main, when shit hits the
                 * fan. Below depends on sdl3
                SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                    _( "Naev Critical Error" ), buf,
                    gl_screen.window );
                */
            }
            _ => (),
        }
        naevc::PHYSFS_permitSymbolicLinks(1);
    }

    /* Set up locales. */
    linebreak::init();
    gettext::init();

    /* Print the version */
    let human_version = version::VERSION_HUMAN;
    log::log(&human_version);
    if cfg!(target_os = "linux") {
        match env::ENV.is_appimage {
            true => {
                log::log(format!("AppImage detected. Running from: {}", env::ENV.appdir).as_str())
            }
            false => log::debug("AppImage not detected."),
        }
    }

    unsafe {
        naev_main(argv.len() as c_int, argv.as_mut_ptr());
    };
    Ok(())
}
