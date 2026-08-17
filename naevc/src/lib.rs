//! Raw FFI bindings to the Naev C engine.
//!
//! The contents are generated at build time; see `build.rs` for the header
//! list and the bindgen configuration.

#![allow(clippy::all)]
#![allow(
   non_snake_case,
   non_camel_case_types,
   non_upper_case_globals,
   improper_ctypes,
   unnecessary_transmutes
)]

#[allow(dead_code)]
pub mod config {
   include!(concat!(env!("OUT_DIR"), "/config.rs"));
}

include!(concat!(env!("OUT_DIR"), "/naevc.rs"));
