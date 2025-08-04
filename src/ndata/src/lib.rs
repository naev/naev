use log::{info, warn, warn_err};
use sdl3 as sdl;
use std::ffi::{c_char, CStr, CString};
use std::io::{Read, Result};
use std::path::Path;

use log::formatx::formatx;
use log::gettext::gettext;
use log::{semver, version};

pub mod physfs;

pub const GFX_PATH: &str = "gfx/";

/// Whether or not the data has likely been found
fn found() -> bool {
    physfs::exists("VERSION") && physfs::exists("start.xml")
}

/// See if version is OK.
fn test_version() -> anyhow::Result<()> {
    let version_buf = &read("VERSION")?;
    let version_str = String::from_utf8_lossy(version_buf);
    let version = semver::Version::parse(&version_str)?;
    let diff = version::compare_versions(&version::VERSION, &version);
    if diff != 0 {
        let err_str = formatx!(gettext("ndata_version inconsistency with this version of Naev!\nExpected ndata version {} got {}."), &*version::VERSION_HUMAN, &version_str)?;
        warn!(&err_str);
        if diff.abs() > 2 {
            anyhow::bail!(err_str);
        } else if diff.abs() > 1 {
            info!("Naev will probably crash now as the versions are probably not compatible.");
        }
    }
    Ok(())
}

/// Initializes the ndata, has to be called first.
/// Will only return an Err when it is not recoverable.
pub fn setup() -> anyhow::Result<()> {
    // Global override takes preference if applicable
    unsafe {
        if !naevc::conf.datapath.is_null() {
            let datapath = CStr::from_ptr(naevc::conf.datapath);
            physfs::set_write_dir(&datapath.to_string_lossy()).unwrap_or_else(|e| {
                warn_err!(e);
            });
            return Ok(());
        }
    }

    // For historical reasons predating physfs adoption, this case is different.
    let app = if cfg!(target_os = "macos") {
        "org.naev.Naev"
    } else {
        "naev"
    };
    match physfs::get_pref_dir(".", app) {
        Ok(pref) => match physfs::set_write_dir(&pref) {
            Ok(_) => (),
            Err(e) => {
                warn_err!(e);
                warn!("Cannot determine data path, using current directory");
                physfs::set_write_dir("./naev/").unwrap_or_else(|e| {
                    warn_err!(e);
                });
            }
        },
        Err(e) => {
            warn_err!(e);
        }
    };

    // We need the write directories set up so we can start logging
    // TODO fix logging
    unsafe {
        naevc::log_redirect();
    }

    // Load conf
    if unsafe { !naevc::conf.ndata.is_null() } {
        let path = unsafe { CStr::from_ptr(naevc::conf.ndata).to_string_lossy() };
        match physfs::mount(&path, true) {
            Err(e) => {
                warn_err!(e);
            }
            Ok(()) => {
                info!("Added datapath from conf.lua file: {}", &path);
            }
        }
    }

    // If not found, try other places
    for s in [&physfs::get_base_dir(), naevc::config::PKGDATADIR] {
        if !found() {
            let path = Path::new(s).join("dat");
            match physfs::mount(&path.to_string_lossy(), true) {
                Err(e) => {
                    warn_err!(e);
                }
                Ok(()) => {
                    info!("Trying default datapath : {}", &path.to_string_lossy());
                }
            }
        }
    }

    if cfg!(target_os = "macos") {
        if unsafe { naevc::macos_isBundle() } != 0 {
            const PATH_MAX: usize = naevc::PATH_MAX as usize;
            let mut buf: [c_char; PATH_MAX] = [0; PATH_MAX];
            unsafe {
                naevc::macos_resourcesPath(buf.as_mut_ptr(), PATH_MAX - 4);
            }
            let buf = unsafe { CStr::from_ptr(buf.as_ptr()) };
            let path = Path::new(&*buf.to_string_lossy()).join("dat");
            match physfs::mount(&path.to_string_lossy(), true) {
                Err(e) => {
                    warn_err!(e);
                }
                Ok(()) => {
                    info!("Trying default datapath : {}", &path.to_string_lossy());
                }
            }
        }
    } else if cfg!(target_os = "linux") && unsafe { naevc::env.isAppImage } != 0 {
        let buf = unsafe { CStr::from_ptr(naevc::env.appdir) };
        let path = Path::new(&*buf.to_string_lossy())
            .join(naevc::config::PKGDATADIR)
            .join("dat");
        match physfs::mount(&path.to_string_lossy(), true) {
            Err(e) => {
                warn_err!(e);
            }
            Ok(()) => {
                info!("Trying default datapath : {}", &path.to_string_lossy());
            }
        }
    }

    // Finally, we mount the write directory also as read
    physfs::mount(&physfs::get_write_dir(), false).unwrap_or_else(|e| {
        warn_err!(e);
    });

    // Plugin initialization before checking the data for consistency
    unsafe {
        naevc::plugin_init();
    }

    // If data is not found, we error.
    if !found() {
        anyhow::bail!(formatx!(gettext("Unable to find game data. You may need to install, specify a datapath, or run using {} (if developing)."), "naev.py" )?);
    }

    test_version()
}

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
