use anyhow::Result;
use std::fs::File;
use std::io::Write;
use std::sync::atomic::AtomicU32;
use std::sync::{LazyLock, Mutex};

const WARN_MAX: u32 = 1000;

pub mod version;

pub use formatx;
pub use gettext;
pub use log::*;
pub use semver;

#[cfg(unix)]
pub use nix;

// TODO
// * Remove log if empty (no warnings) on drop
// * Maybe add colour for warning / whatever in console
// * Maybe keep a copy of the last warning before moving over

enum Output {
    Buffer(String),
    File(File),
}
impl Output {
    fn write(&mut self, msg: &str, level: log::Level) -> Result<()> {
        // Write to buffer
        if level <= log::Level::Warn {
            eprintln!("{msg}");
        } else {
            println!("{msg}");
        }
        // Print on screen
        match self {
            Self::Buffer(b) => {
                b.push('\n');
                b.push_str(msg);
            }
            Self::File(f) => {
                f.write(b"\n")?;
                f.write(msg.as_bytes())?;
            }
        }
        Ok(())
    }
}

struct Logger {
    warn_num: AtomicU32,
    output: Mutex<Output>,
}
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
        let level = record.level();
        let msg = match level {
            log::Level::Error => {
                let bt = std::backtrace::Backtrace::force_capture();
                format!("{}[E] {}", bt, record.args())
            }
            log::Level::Warn => {
                let nw = self
                    .warn_num
                    .fetch_add(1, std::sync::atomic::Ordering::SeqCst);
                let msg = if nw <= WARN_MAX {
                    let bt = std::backtrace::Backtrace::force_capture();
                    format!("{}[W] {}", bt, record.args())
                } else if nw == WARN_MAX {
                    format!(
                        "[w] {}",
                        gettext::gettext("TOO MANY WARNINGS, NO LONGER DISPLAYING WARNINGS")
                    )
                } else {
                    String::new()
                };
                #[cfg(unix)]
                if naevc::config::DEBUG_PARANOID {
                    nix::sys::signal::raise(nix::sys::signal::Signal::SIGINT).unwrap();
                }
                msg
            }
            log::Level::Info => format!("[I] {}", record.args()),
            log::Level::Debug => format!("[D] {}", record.args()),
            log::Level::Trace => {
                return;
            }
        };
        let mut o = self.output.lock().unwrap();
        let _ = o.write(&msg, level);
    }

    fn flush(&self) {
        let mut o = self.output.lock().unwrap();
        match &mut *o {
            Output::File(f) => {
                let _ = f.flush();
            }
            _ => (),
        }
    }
}
impl Logger {
    fn new() -> Self {
        Self {
            warn_num: AtomicU32::new(0),
            output: Mutex::new(Output::Buffer(String::new())),
        }
    }

    fn log_to_file(&self, path: &str) -> Result<()> {
        let mut f = File::create(path)?;
        let mut o = self.output.lock().unwrap();
        match &*o {
            Output::Buffer(b) => {
                f.write(&b.as_bytes())?;
            }
            Output::File(_) => {
                anyhow::bail!("already logging to file");
            }
        }
        *o = Output::File(f);
        Ok(())
    }
}
static LOGGER: LazyLock<Logger> = LazyLock::new(|| Logger::new());

pub fn init() -> Result<()> {
    match log::set_logger(&*LOGGER) {
        Ok(_) => (),
        Err(_) => anyhow::bail!("unable to set_logger"),
    }
    if cfg!(debug_assertions) {
        log::set_max_level(log::LevelFilter::Debug);
    } else {
        log::set_max_level(log::LevelFilter::Info);
    }
    Ok(())
}

pub fn set_log_file(path: &str) -> Result<()> {
    LOGGER.log_to_file(path)
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
