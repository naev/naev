use sdl3 as sdl;
use std::ffi::CString;
use std::io::{Read, Result};

pub mod physfs;

pub const GFX_PATH: &str = "gfx/";

pub fn simplify_path(path: &str) -> Result<String> {
    let mut out: Vec<&str> = Vec::new();
    for s in path.split("/") {
        match s {
            "" | "." => continue,
            ".." => {
                out.pop();
            }
            _ => out.push(s),
        }
    }
    Ok(out.join("/"))
}

pub fn read(path: &str) -> Result<Vec<u8>> {
    let mut f = physfs::File::open(path, physfs::Mode::Read)?;
    let mut out: Vec<u8> = vec![0; f.len()? as usize];
    f.read_exact(out.as_mut())?;
    Ok(out)
}

pub fn is_dir(path: &str) -> bool {
    match stat(path) {
        Ok(s) => s.filetype == FileType::Directory,
        Err(_) => false,
    }
}

pub fn is_file(path: &str) -> bool {
    match stat(path) {
        Ok(s) => s.filetype == FileType::Regular,
        Err(_) => false,
    }
}

pub fn read_dir(path: &str) -> Result<Vec<String>> {
    Ok(physfs::read_dir(path)?
        .into_iter()
        .filter_map(|f| match is_dir(&f) {
            true => read_dir(&f).ok(),
            false => match physfs::blacklisted(&f) {
                true => None,
                false => Some(vec![f]),
            },
        })
        .flatten()
        .collect())
}

pub fn iostream(path: &str) -> Result<sdl::iostream::IOStream> {
    physfs::iostream(path, physfs::Mode::Read)
}

pub fn open(path: &str) -> Result<physfs::File> {
    physfs::File::open(path, physfs::Mode::Read)
}

#[derive(PartialEq)]
pub enum FileType {
    Regular,
    Directory,
    Symlink,
    Other,
}

#[allow(dead_code)]
pub struct Stat {
    filesize: i64,
    modtime: i64,
    createtime: i64,
    accesstime: i64,
    filetype: FileType,
    readonly: bool,
}

pub fn stat(filename: &str) -> Result<Stat> {
    let c_filename = CString::new(filename)?;
    let mut st = naevc::PHYSFS_Stat {
        filesize: 0,
        modtime: 0,
        createtime: 0,
        accesstime: 0,
        filetype: 0,
        readonly: 0,
    };
    match unsafe { naevc::PHYSFS_stat(c_filename.as_ptr(), &mut st) } {
        0 => Err(physfs::error_as_io_error_with_file("PHYSFS_stat", filename)),
        _ => Ok(Stat {
            filesize: st.filesize,
            modtime: st.modtime,
            createtime: st.createtime,
            accesstime: st.accesstime,
            filetype: match st.filetype {
                naevc::PHYSFS_FileType_PHYSFS_FILETYPE_REGULAR => FileType::Regular,
                naevc::PHYSFS_FileType_PHYSFS_FILETYPE_DIRECTORY => FileType::Directory,
                naevc::PHYSFS_FileType_PHYSFS_FILETYPE_SYMLINK => FileType::Symlink,
                _ => FileType::Other,
            },
            readonly: st.readonly != 0,
        }),
    }
}
