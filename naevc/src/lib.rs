// The C archive calls into these, so they can't live in a crate above it.
pub mod array;

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
