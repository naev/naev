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
        if cfg!(debug_assertions) {
            println!("{}",&formatx!($($arg)*).unwrap());
        }
    };
}

#[macro_export]
macro_rules! nwarn {
    ($($arg:tt)*) => {
        println!("{}", std::backtrace::Backtrace::force_capture());
        println!("{}",&formatx!($($arg)*).unwrap());
    };
}
