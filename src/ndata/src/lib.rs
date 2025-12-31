use directories::ProjectDirs;
use nlog::{debug, info, warn, warn_err};
use sdl3 as sdl;
use std::ffi::{CStr, CString, c_char};
use std::io::{Read, Result};
use std::path::{Path, PathBuf};
use std::sync::LazyLock;

use nlog::formatx::formatx;
use nlog::gettext::gettext;
use nlog::{infox, semver, version};

pub mod cwrap;
pub mod data;
pub mod env;
pub mod luafile;
pub mod physfs;

/// Whether or not the data has likely been found
fn found() -> bool {
    exists("VERSION") && (exists("start.xml") || exists("start.toml"))
}

/// Wrapper for directories, which lets us use different fallbacks and overrides
struct Directories {
    pref: PathBuf,
    cache: PathBuf,
}
impl Directories {
    fn new() -> Self {
        // Global override takes preference if applicable
        if unsafe { !naevc::conf.datapath.is_null() } {
            let datapath = unsafe { CStr::from_ptr(naevc::conf.datapath) }
                .to_string_lossy()
                .to_string();
            let path: PathBuf = datapath.into();
            return Directories {
                cache: path.join("cache/"),
                pref: path,
            };
        }

        // We would want to use ProjectDirs for everything, but there is no equivalent to the
        // sdl3::filesystem::get_pref_dir and instead data_dir() is a subdir so we frankenstein it
        let pref = match sdl::filesystem::get_pref_path(".", "naev") {
            Ok(path) => path,
            Err(e) => {
                warn_err!(e);
                ProjectDirs::from("", "", "naev")
                    .unwrap()
                    .data_dir()
                    .to_path_buf()
            }
        };
        let cache = match ProjectDirs::from("", "", "naev") {
            Some(pd) => pd.cache_dir().to_path_buf(),
            None => pref.join("cache/"),
        };
        Self { pref, cache }
    }
}

/// The local project directories that get cached on init
static PROJECT_DIRS: LazyLock<Directories> = LazyLock::new(Directories::new);

/// Gets the configuration directory
pub fn pref_dir() -> &'static Path {
    PROJECT_DIRS.pref.as_path()
}

/// Gets the cache directory used by the project
pub fn cache_dir() -> &'static Path {
    PROJECT_DIRS.cache.as_path()
}

/// Initializes the ndata, has to be called first.
/// Will only return an Err when it is not recoverable.
pub fn setup() -> anyhow::Result<()> {
    match physfs::set_write_dir(pref_dir()) {
        Ok(_) => (),
        Err(e) => {
            warn_err!(e);
            info!("Cannot determine data path, using current directory");
            physfs::set_write_dir("./naev/").unwrap_or_else(|e| {
                warn_err!(e);
            });
        }
    }

    // Redirect the log after we set up the write directory.
    let logpath = physfs::get_write_dir().join("logs");
    let _ = std::fs::create_dir_all(&logpath);
    let logfile = logpath.join(
        chrono::Local::now()
            .format("%Y-%m-%d_%H-%M-%S.txt")
            .to_string(),
    );
    infox!(gettext("Logging to {}"), logfile.to_string_lossy());
    nlog::set_log_file(&logfile.to_string_lossy()).unwrap_or_else(|e| {
        warn_err!(e);
    });

    // Load conf
    if unsafe { !naevc::conf.ndata.is_null() } {
        let path = unsafe { CStr::from_ptr(naevc::conf.ndata).to_string_lossy() };
        match physfs::mount(&*path, true) {
            Err(e) => {
                warn_err!(e);
            }
            Ok(()) => {
                info!("Added datapath from conf.lua file: {}", &path);
            }
        }
    }

    // If the path is absolute, .join will replace the path, so we have to make relative
    let pkgdatadir = {
        let mut path = Path::new(&naevc::config::PKGDATADIR);
        if path.is_absolute() {
            path = match path.strip_prefix("/").ok() {
                Some(p) => p,
                None => Path::new(""),
            };
        }
        path.to_string_lossy()
    };

    if cfg!(target_os = "macos") {
        if unsafe { naevc::macos_isBundle() } != 0 {
            const PATH_MAX: usize = naevc::PATH_MAX as usize;
            let mut buf: [c_char; PATH_MAX] = [0; PATH_MAX];
            unsafe {
                naevc::macos_resourcesPath(buf.as_mut_ptr(), PATH_MAX - 4);
            }
            let buf = unsafe { CStr::from_ptr(buf.as_ptr()) };
            let path = Path::new(&*buf.to_string_lossy()).join("dat");
            match physfs::mount(&path, true) {
                Err(e) => {
                    warn_err!(e);
                }
                Ok(()) => {
                    info!("Trying default datapath : {}", &path.to_string_lossy());
                }
            }
        }
    } else if cfg!(target_os = "linux") && env::ENV.is_appimage {
        let path = Path::new(&env::ENV.appdir).join(&*pkgdatadir).join("dat");
        match physfs::mount(&path, true) {
            Err(e) => {
                warn_err!(e);
            }
            Ok(()) => {
                info!("Trying default datapath : {}", &path.to_string_lossy());
            }
        }
    }

    // If not found, try other places
    for s in [
        &pkgdatadir,
        naevc::config::PKGDATADIR,
        physfs::get_base_dir().to_string_lossy().as_ref(),
    ] {
        if found() {
            break;
        }
        let path = Path::new(s).join("dat");
        match physfs::mount(&path, true) {
            Err(_) => {
                //warn_err!(e);
                debug!("Failed to mount path '{}'", &path.to_string_lossy());
            }
            Ok(()) => {
                info!("Trying default datapath: '{}'", &path.to_string_lossy());
            }
        }
    }

    // Finally, we mount the write directory also as read
    physfs::mount(physfs::get_write_dir(), false).unwrap_or_else(|e| {
        warn_err!(e);
    });
    Ok(())
}

/// Makes sure the ndata was loaded properly.
pub fn check_version() -> anyhow::Result<()> {
    // If data is not found, we error.
    if !found() {
        anyhow::bail!(formatx!(
            gettext(
                "Unable to find game data. You may need to install, specify a datapath, or run using {} (if developing)."
            ),
            "naev.py"
        )?);
    }

    // Verify the version
    let version_buf = &read("VERSION")?;
    let version_str = String::from_utf8_lossy(version_buf);
    let version = semver::Version::parse(&version_str)?;
    let diff = version::compare_versions(&version::VERSION, &version);
    if diff != 0 {
        let err_str = formatx!(
            gettext(
                "ndata_version inconsistency with this version of Naev!\nExpected ndata version {} got {}."
            ),
            &*version::VERSION_HUMAN,
            &version_str
        )?;
        warn!("{}", &err_str);
        if diff.abs() > 2 {
            anyhow::bail!(err_str);
        } else if diff.abs() > 1 {
            info!("Naev will probably crash now as the versions are probably not compatible.");
        }
    }
    Ok(())
}

/// Simplifies a path, using the separator '/' and removing duplicate or unnecessary redirections.
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

/// Slurps an entire file
pub fn read<P: AsRef<Path>>(path: P) -> Result<Vec<u8>> {
    let mut f = physfs::File::open(path, physfs::Mode::Read)?;
    let mut out: Vec<u8> = vec![0; f.len()? as usize];
    f.read_exact(out.as_mut())?;
    Ok(out)
}

/// Same as read, but converts to string
pub fn read_to_string<P: AsRef<Path>>(path: P) -> Result<String> {
    let mut f = physfs::File::open(path, physfs::Mode::Read)?;
    let mut out = String::new();
    f.read_to_string(&mut out)?;
    Ok(out)
}

/// Checks to see if a path is a directory
pub fn is_dir<P: AsRef<Path>>(path: P) -> bool {
    match stat(path) {
        Ok(s) => s.filetype == FileType::Directory,
        Err(_) => false,
    }
}

/// Checks to see if a path is a file
pub fn is_file<P: AsRef<Path>>(path: P) -> bool {
    match stat(path) {
        Ok(s) => s.filetype == FileType::Regular,
        Err(_) => false,
    }
}

/// Checks to see if a path exists (can be file or directory.
pub fn exists<P: AsRef<Path>>(path: P) -> bool {
    physfs::exists(path)
}

/// Recursively lists all the files in a directory.
pub fn read_dir<P: AsRef<Path>>(path: P) -> Result<Vec<PathBuf>> {
    let path = path.as_ref();
    Ok(physfs::read_dir(path)?
        .into_iter()
        .filter_map(|f| {
            let full = path.join(&f);
            match is_dir(&full) {
                true => read_dir(&full).ok().map(|v| {
                    let base: PathBuf = f.into();
                    v.iter().map(|file| base.join(file)).collect()
                }),
                false => match physfs::blacklisted(&full) {
                    true => None,
                    false => Some(vec![f.into()]),
                },
            }
        })
        .flatten()
        .collect())
}

/// Allows applying a filter
pub fn read_dir_filter<P: AsRef<Path>>(
    path: P,
    predicate: impl Fn(&Path) -> bool,
) -> Result<Vec<PathBuf>> {
    let path = path.as_ref();
    Ok(physfs::read_dir(path)?
        .into_iter()
        .filter_map(|f| {
            let full = path.join(&f);
            match is_dir(&full) {
                true => read_dir_filter(&full, &predicate).ok().map(|v| {
                    let base: PathBuf = f.into();
                    v.iter().map(|file| base.join(file)).collect()
                }),
                false => match physfs::blacklisted(&full) {
                    true => None,
                    false => match predicate(Path::new(&f)) {
                        true => Some(vec![f.into()]),
                        false => None,
                    },
                },
            }
        })
        .flatten()
        .collect())
}

/// Gets an SDL IOStream from a file if exists
pub fn iostream<P: AsRef<Path>>(path: P) -> Result<sdl::iostream::IOStream<'static>> {
    physfs::iostream(path, physfs::Mode::Read)
}

/// Opens a file for reading
pub fn open<P: AsRef<Path>>(path: P) -> Result<physfs::File> {
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

/// Gets information about a file or directory
pub fn stat<P: AsRef<Path>>(filename: P) -> Result<Stat> {
    let c_filename = CString::new(filename.as_ref().as_os_str().as_encoded_bytes()).unwrap();
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

/*
/// Like fs::canonicalize but doesn't require the path to exist.
/// For String use simplify_path.
fn normalize_path<P: AsRef<Path>>(path: P) -> Option<PathBuf> {
    use std::path::Component;
    let mut test_path = PathBuf::new();
    for component in path.as_ref().components() {
        match component {
            Component::ParentDir => {
                if !test_path.pop() {
                    return None;
                }
            }
           Component::CurDir => {}
            _ => test_path.push(component.as_os_str()),
        }
    }
    Some(test_path)
}

/// Checks to see if a path is within the base path.
fn is_path_within_base<P: AsRef<Path>>(path: P, base: P) -> bool {
    if let (Some(norm_path), Some(norm_base)) = (normalize_path(path), normalize_path(base)) {
        norm_path.starts_with(norm_base)
    } else {
        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn normalize_path_test() {
        assert_eq!(normalize_path(Path::new(".")), Some(PathBuf::from("")));
        assert_eq!(normalize_path(Path::new("a")), Some(PathBuf::from("a")));
        assert_eq!(normalize_path(Path::new("./././a/..")), Some(PathBuf::from("")));
        assert_eq!(normalize_path(Path::new("a/..")), Some(PathBuf::from("")));
        assert_eq!(normalize_path(Path::new("a/../b")), Some(PathBuf::from("b")));
        assert_eq!(normalize_path(Path::new("..")), None);
        assert_eq!(normalize_path(Path::new("a/../..")), None);
    }

    #[test]
    fn is_path_within_base_test() {
        assert!(is_path_within_base(Path::new("a/b/c"), Path::new("a")));
        assert!(is_path_within_base(Path::new("a/b/c"), Path::new("a/b")));
        assert!(is_path_within_base(Path::new("a/b/c"), Path::new("a/b/c")));
        assert!(is_path_within_base(Path::new(""), Path::new("")));
        assert!(is_path_within_base(Path::new("a/."), Path::new("a/")));
        assert!(!is_path_within_base(Path::new("a/b/c"), Path::new("a/b/c/d")));
        assert!(!is_path_within_base(Path::new(""), Path::new("a/b/c/d")));
        assert!(!is_path_within_base(Path::new("a/.."), Path::new("a/")));
    }
}
*/
