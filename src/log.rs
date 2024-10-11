// Logging tools
#![allow(dead_code)]

pub fn init() {
    unsafe {
        naevc::log_init();
    };
}

pub fn log(msg: &str) {
    println!("{}", msg);
}

pub fn debug(msg: &str) {
    println!("{}", msg);
}

pub fn warn(msg: &str) {
    println!("{}", msg);
}

#[macro_export]
macro_rules! nlog {
    ($($arg:tt)*) => {
        println!("{}",&formatx!($($arg)*).unwrap())
    };
}

#[macro_export]
macro_rules! ndebug {
    ($($arg:tt)*) => {
        if naevc::config::DEBUG {
            println!("{}",&formatx!($($arg)*).unwrap());
        }
    };
}

#[macro_export]
macro_rules! nwarn {
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
