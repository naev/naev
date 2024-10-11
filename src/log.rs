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
