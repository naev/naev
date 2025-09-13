//! AL_SOFT_buffer_length_query
use crate::openal::al_types::*;
use std::ffi::CStr;

pub const AL_SOFT_BUFFER_LENGTH_QUERY_NAME: &CStr = c"AL_SOFT_buffer_length_query";

pub const AL_BYTE_LENGTH_SOFT: ALenum = 0x2009;
pub const AL_SAMPLE_LENGTH_SOFT: ALenum = 0x200A;
pub const AL_SEC_LENGTH_SOFT: ALenum = 0x200B;
