//!AL_SOFT_direct_channels and AL_SOFT_direct_channels_remix
use crate::openal as al;

#[allow(dead_code)]
pub mod consts {
    use crate::openal::al_types::*;
    pub const AL_DIRECT_CHANNELS_SOFT: ALenum = 0x1033;
    pub const AL_DROP_UNMATCHED_SOFT: ALenum = 0x0001; /* same as AL_TRUE */
    pub const AL_REMIX_UNMATCHED_SOFT: ALenum = 0x0002;
}

pub fn supported() -> bool {
    al::is_extension_present(c"AL_SOFT_direct_channels")
        && al::is_extension_present(c"AL_SOFT_direct_channels_remix")
}
