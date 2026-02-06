pub mod constants;
pub mod lua;
pub mod ntime;
pub mod nxml;
pub mod start;

pub use nlog;

pub static APPNAME: &str = "Naev";

#[unsafe(no_mangle)]
pub extern "C" fn debug_logBacktrace() {
   nlog::info!("{}", std::backtrace::Backtrace::force_capture());
}
