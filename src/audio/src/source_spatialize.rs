//! AL_SOFT_source_spatialize extension
#![allow(clippy::upper_case_acronyms)]
use crate::openal::al_types::*;
use std::ffi::CStr;
use std::sync::atomic::AtomicBool;

pub const AL_SOFT_SOURCE_SPATIALIZE: &CStr = c"AL_SOFT_source_spatialize";

pub const AL_SOURCE_SPATIALIZE_SOFT: ALenum = 0x1214;
pub const AL_AUTO_SOFT: ALenum = 0x0002;

pub static HAS_AL_SOFT_SOURCE_SPATIALIZE: AtomicBool = AtomicBool::new(false);
