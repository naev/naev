use std::env;

#[derive(Clone)]
pub struct AppEnv {
    pub is_appimage: bool,
    pub appimage: String,
    pub appdir: String,
    pub argv0: String,
}

use std::sync::LazyLock;
pub static ENV: LazyLock<AppEnv> = LazyLock::new(|| detect());

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
            match env::var("ARGV0") {
                Ok(v) => e.argv0 = v,
                _ => (),
            }
            match env::var("APPDIR") {
                Ok(v) => e.appdir = v,
                _ => (),
            }
        }
        Err(_) => {
            e.is_appimage = false;
            e.argv0 = env::args().nth(0).unwrap();
        }
    }
    e
}
