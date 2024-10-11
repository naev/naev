// Logging tools
#![allow(dead_code)]

use crate::{debug, nlog, warn};
use formatx::formatx;

pub fn init() {
    unsafe {
        naevc::log_init();
    };
}

pub fn log(msg: &str) {
    nlog!(msg);
}

pub fn debug(msg: &str) {
    debug!(msg);
}

pub fn warn(msg: &str) {
    warn!(msg);
}

#[macro_export]
macro_rules! nlog {
    ($($arg:tt)*) => {
        println!("{}",&formatx!($($arg)*).unwrap())
    };
}

#[macro_export]
macro_rules! debug {
    ($($arg:tt)*) => {
        if naevc::config::DEBUG {
            println!("{}",&formatx!($($arg)*).unwrap());
        }
    };
}

#[macro_export]
macro_rules! warn {
    ($($arg:tt)*) => {
        eprintln!("{}WARNING {}:{}: {}",
            std::backtrace::Backtrace::force_capture(),
            file!(), line!(),
            &formatx!($($arg)*).unwrap());
        if naevc::config::DEBUG_PARANOID {
            if cfg!(unix) {
                nix::sys::signal::raise( nix::sys::signal::Signal::SIGINT ).unwrap();
            }
        }
    };
}
