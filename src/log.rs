// Logging tools
#![allow(dead_code)]

use crate::gettext::gettext;
use crate::{debug, einfo, info, warn};

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
            #[cfg(debug_assertions)]
            println!("{}",&formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")));
        }
    };
}

use std::sync::atomic::AtomicU32;
pub static WARN_NUM: AtomicU32 = AtomicU32::new(0);

#[macro_export]
macro_rules! warn {
    ($($arg:tt)*) => {
        let nw = $crate::log::WARN_NUM.fetch_add( 1, std::sync::atomic::Ordering::SeqCst );
        if nw <= 1000 {
            eprintln!("{}WARNING {}:{}: {}",
                std::backtrace::Backtrace::force_capture(),
                file!(), line!(),
                &formatx::formatx!($($arg)*).unwrap_or(String::from("Unknown")));
        }
        if nw==1000 {
            eprintln!("{}",gettext("TOO MANY WARNINGS, NO LONGER DISPLAYING TOO WARNINGS"));
        }
        if naevc::config::DEBUG_PARANOID {
            #[cfg(unix)]
            {
                nix::sys::signal::raise(nix::sys::signal::Signal::SIGINT).unwrap();
            }
        }
    };
}
