use sdl2 as sdl;
use std::io::{Read, Result};

use crate::physfs;

pub const GFX_PATH: &str = "gfx/";

pub fn read(path: &str) -> Result<Vec<u8>> {
    let mut f = physfs::File::open(path, physfs::Mode::Read)?;
    let mut out: Vec<u8> = vec![0; f.len()? as usize];
    f.read_exact(out.as_mut())?;
    Ok(out)
}

pub fn read_dir(path: &str) -> Result<Vec<String>> {
    physfs::read_dir(path)
}

pub fn rwops(path: &str) -> Result<sdl::rwops::RWops> {
    physfs::rwops(path, physfs::Mode::Read)
}

pub fn open(path: &str) -> Result<physfs::File> {
    physfs::File::open(path, physfs::Mode::Read)
}
