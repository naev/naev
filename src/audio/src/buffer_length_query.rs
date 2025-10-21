//! AL_SOFT_buffer_length_query
#![allow(dead_code)]

pub mod consts {
    use crate::openal::al_types::*;
    pub const AL_BYTE_LENGTH_SOFT: ALenum = 0x2009;
    pub const AL_SAMPLE_LENGTH_SOFT: ALenum = 0x200A;
    pub const AL_SEC_LENGTH_SOFT: ALenum = 0x200B;
}

pub fn supported() -> bool {
    crate::openal::is_extension_present(c"AL_SOFT_buffer_length_query")
}
