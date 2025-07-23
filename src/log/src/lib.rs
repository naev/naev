// Logging tools
use std::sync::atomic::AtomicU32;
pub static WARN_NUM: AtomicU32 = AtomicU32::new(0);
pub const WARN_MAX: u32 = 1000;

#[cfg(unix)]
pub use nix;

// Re-export so this doesn't depend on anything.
pub use gettext;

pub fn init() {
    unsafe {
        naevc::log_init();
    };
}

pub fn einfo(msg: &str) {
    einfo!(msg);
}

pub fn info(msg: &str) {
    info!(msg);
}

pub fn debug(msg: &str) {
    debug!(msg);
}

pub fn warn(msg: &str) {
    warn!(msg);
}

pub fn warn_err(err: anyhow::Error) {
    warn_err!(err);
}

#[macro_export]
macro_rules! info {
    ($($arg:tt)*) => {
        println!("{}",&formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")))
    };
}

#[macro_export]
macro_rules! einfo {
    ($($arg:tt)*) => {
        eprintln!("{}",&formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")))
    };
}

#[macro_export]
macro_rules! debug {
    ($($arg:tt)*) => {
        if naevc::config::DEBUG {
            println!("{}",&formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")));
        }
    };
}

#[macro_export]
macro_rules! warn {
    ($($arg:tt)*) => {
        let nw = $crate::WARN_NUM.fetch_add( 1, std::sync::atomic::Ordering::SeqCst );
        if nw <= $crate::WARN_MAX {
            eprintln!("{}WARNING {}:{}: {}",
                std::backtrace::Backtrace::force_capture(),
                file!(), line!(),
                &formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")));
        }
        if nw==$crate::WARN_MAX {
            eprintln!("{}",$crate::gettext::gettext("TOO MANY WARNINGS, NO LONGER DISPLAYING TOO WARNINGS"));
        }
        if naevc::config::DEBUG_PARANOID {
            #[cfg(unix)]
            {
                $crate::nix::sys::signal::raise($crate::nix::sys::signal::Signal::SIGINT).unwrap();
            }
        }
    };
}

#[macro_export]
macro_rules! warn_err {
    ($err:ident) => {
        let nw = $crate::WARN_NUM.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        if nw <= $crate::WARN_MAX {
            eprintln!("WARNING: {:?}", $err);
        }
        if nw == $crate::WARN_MAX {
            eprintln!(
                "{}",
                $crate::gettext::gettext("TOO MANY WARNINGS, NO LONGER DISPLAYING TOO WARNINGS")
            );
        }
        if naevc::config::DEBUG_PARANOID {
            #[cfg(unix)]
            {
                $crate::nix::sys::signal::raise($crate::nix::sys::signal::Signal::SIGINT).unwrap();
            }
        }
    };
}
