//! The boundary between the Rust side and the C engine.
//!
//! This crate compiles the C engine, generates the bindings to it at build
//! time (see `build.rs`), and holds the pieces the compiled C calls back into.
//! Those have to be linked from underneath the archive, so they live here
//! rather than in a crate above it.

// The C archive this crate builds calls into these.
pub mod array;

// The generated code needs blanket allows that the module above does not, so
// they stop at the module rather than covering the crate.
#[allow(clippy::all)]
#[allow(
   dead_code,
   non_snake_case,
   non_camel_case_types,
   non_upper_case_globals,
   improper_ctypes,
   unnecessary_transmutes
)]
mod generated {
   pub mod config {
      include!(concat!(env!("OUT_DIR"), "/config.rs"));
   }

   include!(concat!(env!("OUT_DIR"), "/naevc.rs"));
}

pub use generated::*;
