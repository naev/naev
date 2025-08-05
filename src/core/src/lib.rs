pub mod ntime;
pub mod nxml;
pub mod start;
pub mod utils;

pub use log;

pub static APPNAME: &str = "Naev";

#[unsafe(no_mangle)]
pub extern "C" fn debug_logBacktrace() {
    log::info!("{}", std::backtrace::Backtrace::force_capture());
}
