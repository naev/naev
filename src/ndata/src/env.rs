use std::env;
use std::ffi::CString;
use std::os::raw::c_char;
use std::sync::OnceLock;

#[derive(Clone)]
pub struct AppEnv {
    pub is_appimage: bool,
    pub appimage: String,
    pub appdir: String,
    pub argv0: String,
}

use std::sync::LazyLock;
pub static ENV: LazyLock<AppEnv> = LazyLock::new(detect);

/*
 These static CStrings are used to provide valid pointers to the C FFI struct
 for the lifetime of the program. They are set once at startup and never changed.
 This ensures the C side always sees valid, non-dangling pointers.
*/
static CAPPIMAGE: OnceLock<CString> = OnceLock::new();
static CARGV0: OnceLock<CString> = OnceLock::new();
static CAPPDIR: OnceLock<CString> = OnceLock::new();

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
            e.argv0 = env::args().next().unwrap_or(String::from("Unknown"));
        }
    }

    /*
     TODO remove when we don't need it anymore.
     OnceLock guarantees only one initialization and shared access is safe.
    */
    CAPPIMAGE
        .set(CString::new(e.appimage.clone()).unwrap())
        .ok();
    CARGV0.set(CString::new(e.argv0.clone()).unwrap()).ok();
    CAPPDIR.set(CString::new(e.appdir.clone()).unwrap()).ok();

    unsafe {
        naevc::env.isAppImage = if e.is_appimage { 1 } else { 0 };
        naevc::env.appimage = CAPPIMAGE.get().unwrap().as_ptr() as *mut c_char;
        naevc::env.argv0 = CARGV0.get().unwrap().as_ptr() as *mut c_char;
        naevc::env.appdir = CAPPDIR.get().unwrap().as_ptr() as *mut c_char;
    }
    e
}
