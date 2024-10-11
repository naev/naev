use std::env;
use std::ffi::CString;
use std::os::raw::c_char;

#[derive(Clone)]
pub struct AppEnv {
    pub is_appimage: bool,
    pub appimage: String,
    pub appdir: String,
    pub argv0: String,
}

use std::sync::LazyLock;
pub static ENV: LazyLock<AppEnv> = LazyLock::new(detect);

fn detect() -> AppEnv {
    let mut e = AppEnv {
        is_appimage: false,
        appimage: String::default(),
        appdir: String::default(),
        argv0: String::default(),
    };

    match env::var("APPIMAGE") {
        Ok(appimage) => {
            e.appimage = appimage;
            e.is_appimage = true;
            if let Ok(v) = env::var("ARGV0") {
                e.argv0 = v
            }
            if let Ok(v) = env::var("APPDIR") {
                e.appdir = v
            }
        }
        Err(_) => {
            e.is_appimage = false;
            e.argv0 = env::args().next().unwrap();
        }
    }

    /* TODO remove. */
    let cappimage: CString = CString::new(e.appimage.clone()).unwrap();
    let cargv0: CString = CString::new(e.argv0.clone()).unwrap();
    let cappdir: CString = CString::new(e.appdir.clone()).unwrap();
    unsafe {
        naevc::env.isAppImage = if e.is_appimage { 1 } else { 0 };
        naevc::env.appimage = cappimage.as_ptr() as *mut c_char;
        naevc::env.argv0 = cargv0.as_ptr() as *mut c_char;
        naevc::env.appdir = cappdir.as_ptr() as *mut c_char;
    }

    e
}
