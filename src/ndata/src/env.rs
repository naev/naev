use std::env;
use std::ffi::CString;
use std::os::raw::c_char;

#[derive(Clone, Default)]
pub struct AppEnv {
    pub is_appimage: bool,
    pub appimage: String,
    pub appdir: String,
    pub argv0: String,

    // These static CStrings are used to provide valid pointers to the C FFI struct
    // for the lifetime of the program. They are set once at startup and never changed.
    // This ensures the C side always sees valid, non-dangling pointers.
    cappimage: CString,
    cappdir: CString,
    cargv0: CString,
}

use std::sync::LazyLock;
pub static ENV: LazyLock<AppEnv> = LazyLock::new(detect);

fn detect() -> AppEnv {
    let mut e = AppEnv {
        is_appimage: false,
        ..Default::default()
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
            e.argv0 = env::args().next().unwrap_or(String::from("Unknown"));
        }
    }

    // TODO remove when we don't need it anymore.
    e.cappimage = CString::new(&*e.appimage).ok().unwrap();
    e.cargv0 = CString::new(&*e.argv0).ok().unwrap();
    e.cappdir = CString::new(&*e.appdir).ok().unwrap();
    unsafe {
        naevc::env.isAppImage = if e.is_appimage { 1 } else { 0 };
        naevc::env.appimage = e.cappimage.as_ptr() as *mut c_char;
        naevc::env.argv0 = e.cargv0.as_ptr() as *mut c_char;
        naevc::env.appdir = e.cappdir.as_ptr() as *mut c_char;
    }
    e
}
