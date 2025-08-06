// Logging tools
use anyhow::Result;
use std::sync::atomic::AtomicU32;

pub static WARN_NUM: AtomicU32 = AtomicU32::new(0);
pub const WARN_MAX: u32 = 1000;

pub mod version;

pub use formatx;
pub use gettext;
pub use log::*;
pub use semver;

#[cfg(unix)]
pub use nix;

struct Logger {}
impl log::Log for Logger {
    fn enabled(&self, metadata: &log::Metadata) -> bool {
        if cfg!(debug_assertions) {
            metadata.level() <= log::Level::Debug
        } else {
            metadata.level() <= log::Level::Info
        }
    }

    fn log(&self, record: &log::Record) {
        if !self.enabled(record.metadata()) {
            return;
        }
        match record.level() {
            log::Level::Error => {
                eprintln!("[E] {}:{}: {}", file!(), line!(), record.args());
            }
            log::Level::Warn => {
                let nw = WARN_NUM.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                if nw <= WARN_MAX {
                    eprintln!("[W] {}:{}: {}", file!(), line!(), record.args());
                }
                if nw == WARN_MAX {
                    eprintln!(
                        "{}",
                        gettext::gettext("TOO MANY WARNINGS, NO LONGER DISPLAYING TOO WARNINGS")
                    );
                }
                if naevc::config::DEBUG_PARANOID {
                    #[cfg(unix)]
                    {
                        nix::sys::signal::raise(nix::sys::signal::Signal::SIGINT).unwrap();
                    }
                }
            }
            log::Level::Info => {
                println!("[I] {}", record.args());
            }
            log::Level::Debug => {
                println!("[D] {}", record.args());
            }
            log::Level::Trace => (),
        }
    }

    fn flush(&self) {}
}
impl Logger {
    const fn new() -> Self {
        Logger {}
    }
}
static LOGGER: Logger = Logger::new();

pub fn init() -> Result<()> {
    match log::set_logger(&LOGGER) {
        Ok(_) => (),
        Err(_) => anyhow::bail!("unable to set_logger"),
    }
    if cfg!(debug_assertions) {
        log::set_max_level(log::LevelFilter::Debug);
    } else {
        log::set_max_level(log::LevelFilter::Info);
    }
    unsafe {
        naevc::log_init();
    };
    Ok(())
}

pub fn info(msg: &str) {
    info!("{msg}");
}

pub fn debug(msg: &str) {
    debug!("{msg}");
}

pub fn warn(msg: &str) {
    warn!("{msg}");
}

pub fn warn_err(err: anyhow::Error) {
    warn_err!(err);
}

#[macro_export]
macro_rules! infox {
    ($($arg:tt)*) => {
        $crate::info!("{}",&$crate::formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")))
    };
}

#[macro_export]
macro_rules! debugx {
    ($($arg:tt)*) => {
        $crate::debug!("{}",&$crate::formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")));
    };
}

#[macro_export]
macro_rules! warnx {
    ($($arg:tt)*) => {
        $crate::warn!("{}",&$crate::formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")));
    };
}

#[macro_export]
macro_rules! warn_err {
    ($err:ident) => {
        $crate::warn!("{:?}", $err);
    };
}

// Some simple C API
use std::ffi::{c_char, CStr};
macro_rules! log_c {
    ($funcname: ident, $macro: ident) => {
        #[unsafe(no_mangle)]
        pub extern "C" fn $funcname(msg: *const c_char) {
            let msg = unsafe { CStr::from_ptr(msg) };
            $macro!("{}", msg.to_string_lossy());
        }
    };
}
log_c!(debug_rust, debug);
log_c!(info_rust, info);
log_c!(warn_rust, warn);
