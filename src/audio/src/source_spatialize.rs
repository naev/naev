//! AL_SOFT_source_spatialize extension
#![allow(clippy::upper_case_acronyms, dead_code)]
use std::sync::atomic::{AtomicBool, Ordering};

pub mod consts {
    use crate::openal::al_types::*;
    pub const AL_SOURCE_SPATIALIZE_SOFT: ALenum = 0x1214;
    pub const AL_AUTO_SOFT: ALenum = 0x0002;
}

pub static HAS_AL_SOFT_SOURCE_SPATIALIZE: AtomicBool = AtomicBool::new(false);

pub fn supported() -> bool {
    let has_source_spatialize = crate::openal::is_extension_present(c"AL_SOFT_source_spatialize");
    HAS_AL_SOFT_SOURCE_SPATIALIZE.store(has_source_spatialize, Ordering::Relaxed);
    has_source_spatialize
}
