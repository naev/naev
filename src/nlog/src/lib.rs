use anyhow::Result;
use log as logcore;
use std::fs::File;
use std::io::Write;
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::{LazyLock, Mutex};

const WARN_MAX: u32 = 1000;

pub mod version;

pub use formatx;
pub use gettext;
pub use logcore::*;
pub use semver;

#[cfg(unix)]
pub use nix;

// TODO
// * Maybe add colour for warning / whatever in console
// * Maybe keep a copy of the last warning before moving over

struct LogFile {
   name: String,
   file: File,
}

enum Output {
   Buffer(String),
   File(LogFile),
}
impl Output {
   fn write(&mut self, msg: &str, level: logcore::Level) -> Result<()> {
      if msg.is_empty() {
         return Ok(());
      }

      // Write to buffer
      if level <= logcore::Level::Warn {
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
         Self::File(lf) => {
            lf.file.write_all(b"\n")?;
            lf.file.write_all(msg.as_bytes())?;
         }
      }
      Ok(())
   }
}

struct Logger {
   warn_num: AtomicU32,
   output: Mutex<Output>,
}
const WHITELIST: [&str; 9] = [
   "naev",
   "nlog",
   "renderer",
   "pluginmgr",
   "pluginmgr-gui",
   "core",
   "audio",
   "ndata",
   "physics",
];
impl logcore::Log for Logger {
   fn enabled(&self, metadata: &logcore::Metadata) -> bool {
      let level = metadata.level();
      let target = metadata.target();
      if level > logcore::Level::Warn && !WHITELIST.iter().any(|s| target.starts_with(s)) {
         return false;
      }
      if cfg!(debug_assertions) {
         level <= logcore::Level::Debug
      } else {
         level <= logcore::Level::Info
      }
   }

   fn log(&self, record: &logcore::Record) {
      if !self.enabled(record.metadata()) {
         return;
      }
      let level = record.level();
      let msg = match level {
         logcore::Level::Error => {
            // Mark it as having at least one warning on error
            let _ = self
               .warn_num
               .compare_exchange(0, 1, Ordering::SeqCst, Ordering::Relaxed);
            let bt = std::backtrace::Backtrace::force_capture();
            format!("{}[E] {}", bt, record.args())
         }
         logcore::Level::Warn => {
            let nw = self.warn_num.fetch_add(1, Ordering::SeqCst);
            let msg = if nw < WARN_MAX {
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
         logcore::Level::Info => format!("[I] {}", record.args()),
         logcore::Level::Debug => format!("[D] {}", record.args()),
         logcore::Level::Trace => {
            return;
         }
      };
      let _ = self.output.lock().unwrap().write(&msg, level);
   }

   fn flush(&self) {
      let mut o = self.output.lock().unwrap();
      if let Output::File(lf) = &mut *o {
         let _ = lf.file.flush();
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
            f.write_all(b.as_bytes())?;
         }
         Output::File(_) => {
            anyhow::bail!("already logging to file");
         }
      }
      *o = Output::File(LogFile {
         name: String::from(path),
         file: f,
      });
      Ok(())
   }
}

static LOGGER: LazyLock<Logger> = LazyLock::new(Logger::new);

pub fn init() -> Result<()> {
   match logcore::set_logger(&*LOGGER) {
      Ok(_) => (),
      Err(_) => anyhow::bail!("unable to set_logger"),
   }
   if cfg!(debug_assertions) {
      logcore::set_max_level(logcore::LevelFilter::Debug);
   } else {
      logcore::set_max_level(logcore::LevelFilter::Info);
   }
   Ok(())
}

pub fn set_log_file(path: &str) -> Result<()> {
   LOGGER.log_to_file(path)
}

pub fn close_file() {
   let nw = LOGGER.warn_num.load(Ordering::Relaxed);
   if nw == 0 {
      let o = &mut *LOGGER.output.lock().unwrap();
      let filename = match o {
         Output::File(lf) => Some(lf.name.clone()),
         _ => None,
      };
      if let Some(filename) = filename {
         *o = Output::Buffer(String::new());
         std::fs::remove_file(filename).unwrap_or_else(|e| {
            eprintln!("{}", e);
         });
      }
   }
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
   ($err:expr) => {
      $crate::warn!("{:?}", $err);
   };
}

// Some simple C API
use std::ffi::{CStr, c_char};
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
