//! AL_SOFT_buffer_length_query

pub mod consts {
    use crate::openal::alc_types::*;
    pub const ALC_OUTPUT_LIMITER_SOFT: ALCenum = 0x199A;
}

pub fn supported(device: &crate::openal::Device) -> bool {
    device.is_extension_present(c"ALC_SOFT_output_limiter")
}
