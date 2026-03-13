use anyhow::Result;
use std::io::Write;
use std::sync::OnceLock;
use std::sync::atomic::{AtomicU32, Ordering};
use tracing_appender::non_blocking;
pub use tracing_appender::non_blocking::WorkerGuard;
use tracing_subscriber::fmt::writer::Tee;
use tracing_subscriber::{
   filter::filter_fn, fmt, fmt::MakeWriter, fmt::writer::OptionalWriter, layer::Layer,
   layer::SubscriberExt, util::SubscriberInitExt,
};

const WARN_MAX: u32 = 1000;
static WARNINGS: AtomicU32 = AtomicU32::new(0);

pub mod version;

pub use formatx;
pub use gettext;
pub use semver;
pub use tracing::log::*;

#[cfg(unix)]
pub use nix;

// TODO
// * Maybe add colour for warning / whatever in console
// * Maybe keep a copy of the last warning before moving over

/*
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
impl logcore::Log for Logger {
   fn enabled(&self, metadata: &logcore::Metadata) -> bool {
      let level = metadata.level();
      let target = metadata.target();
      // TODO we should probably allow warnings from 3rd party libraries, but wgpu is really bad at
      // this sort of thing...
      if level >= logcore::Level::Warn && !WHITELIST.iter().any(|s| target.starts_with(s)) {
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
*/

struct TeeWarn<A, B>(Tee<A, B>);
impl<'a, A, B> MakeWriter<'a> for TeeWarn<A, B>
where
   A: MakeWriter<'a>,
   B: MakeWriter<'a>,
{
   type Writer = TeeWarn<A::Writer, B::Writer>;
   #[inline]
   fn make_writer(&'a self) -> Self::Writer {
      TeeWarn(self.0.make_writer())
   }
   #[inline]
   fn make_writer_for(&'a self, meta: &tracing::Metadata<'_>) -> Self::Writer {
      TeeWarn(self.0.make_writer_for(meta))
   }
}
impl<A: Write, B: Write> Write for TeeWarn<A, B> {
   #[inline]
   fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
      let nw = WARNINGS.fetch_add(1, Ordering::SeqCst);
      let ret = if nw > WARN_MAX {
         0
      } else if nw == WARN_MAX {
         let b = gettext::gettext("TOO MANY WARNINGS, NO LONGER DISPLAYING WARNINGS").as_bytes();
         self.0.write(b)?
      } else {
         self.0.write(buf)?
      };
      #[cfg(unix)]
      if naevc::config::DEBUG_PARANOID {
         nix::sys::signal::raise(nix::sys::signal::Signal::SIGINT).unwrap();
      }
      Ok(ret)
   }
   #[inline]
   fn flush(&mut self) -> std::io::Result<()> {
      self.0.flush()
   }
}

static LOGFILE: OnceLock<(String, non_blocking::NonBlocking)> = OnceLock::new();

fn file_logger() -> OptionalWriter<non_blocking::NonBlocking> {
   match LOGFILE.get() {
      Some(f) => OptionalWriter::some(f.1.clone()),
      None => OptionalWriter::none(),
   }
}

const WHITELIST: [&str; 10] = [
   "audio",
   "collide",
   "core",
   "naev",
   "ndata",
   "nlog",
   "physics",
   "pluginmgr-gui",
   "pluginmgr",
   "renderer",
];
pub fn init() -> Result<()> {
   // Don't have access to logging files yet...
   // TODO store messages somewhere and paste back
   let warn_layer = fmt::layer()
      .pretty()
      .without_time()
      .with_target(true)
      .with_file(true)
      .with_line_number(true)
      .with_thread_ids(true)
      .with_thread_names(true)
      .with_writer(TeeWarn(Tee::new(std::io::stderr, file_logger)))
      //.with_filter(tracing_subscriber::filter::LevelFilter::WARN);
      .with_filter(filter_fn(|meta| {
         let level = *meta.level();
         if let Some(path) = meta.module_path() {
            WHITELIST.iter().any(|s| path.starts_with(s)) && level <= tracing::Level::WARN
         } else {
            false
         }
      }));
   let debug_layer = fmt::layer()
      .compact()
      .without_time()
      .with_target(false)
      .with_file(false)
      .with_line_number(false)
      .with_thread_ids(false)
      .with_thread_names(false)
      .with_writer(Tee::new(std::io::stdout, file_logger))
      .with_filter(filter_fn(|meta| {
         let level = *meta.level();
         if let Some(path) = meta.module_path() {
            WHITELIST.iter().any(|s| path.starts_with(s))
               && level > tracing::Level::WARN
               && level <= tracing::Level::DEBUG
         } else {
            false
         }
      }));

   //let (debug_layer, debug_handle) = reload::Layer::new(debug_layer);
   //debug_handle.modify( |layer| {
   //   *layer.inner_mut().writer_mut() = || std::sync::Arc::new(std::io::stderr()).with_max_level( tracing::Level::WARN );
   //});

   let registry = tracing_subscriber::registry()
      .with(warn_layer)
      .with(debug_layer);
   // Install the Tracy tracing subscriber when built with -Dtracy=true.
   // This must happen before any spans are entered so that early
   // initialisation work is visible in the profiler.
   #[cfg(feature = "tracy")]
   {
      registry.with(tracing_tracy::TracyLayer::default());
   }

   registry.init();

   Ok(())
}

pub fn set_log_file(path: &str) -> Result<non_blocking::WorkerGuard> {
   let file_appender = tracing_appender::rolling::never("", path);
   let (writer, guard) = non_blocking(file_appender);
   let _ = LOGFILE.set((path.to_string(), writer));
   Ok(guard)
}

pub fn close_file(guard: Option<non_blocking::WorkerGuard>) {
   drop(guard);
   let nw = WARNINGS.load(Ordering::Relaxed);
   if nw == 0 {
      if let Some((path, _)) = LOGFILE.get() {
         std::fs::remove_file(&*path).unwrap_or_else(|e| {
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
