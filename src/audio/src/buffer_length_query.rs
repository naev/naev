//! AL_SOFT_buffer_length_query
use std::ffi::CStr;

const AL_SOFT_BUFFER_LENGTH_QUERY_NAME: &CStr = c"AL_SOFT_buffer_length_query";

pub mod consts {
    use crate::openal::al_types::*;
    pub const AL_BYTE_LENGTH_SOFT: ALenum = 0x2009;
    pub const AL_SAMPLE_LENGTH_SOFT: ALenum = 0x200A;
    pub const AL_SEC_LENGTH_SOFT: ALenum = 0x200B;
}

pub fn supported() -> bool {
    crate::openal::is_extension_present(AL_SOFT_BUFFER_LENGTH_QUERY_NAME)
}
