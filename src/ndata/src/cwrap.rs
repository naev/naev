use anyhow::Result;
use fs_err as fs;
use log::debug;
use std::ffi::{CStr, CString, c_char};
use std::path::PathBuf;

/// Migrates from pre-0.13.0 locations
/// TODO remove in 0.15.0 (or maybe 0.14.0?)
pub fn migrate_pref() -> Result<()> {
    // For historical reasons predating physfs adoption, this case is different.
    let old = crate::physfs::get_pref_dir(
        ".",
        if cfg!(target_os = "macos") {
            "org.naev.Naev"
        } else {
            "naev"
        },
    )?;
    if old.is_dir() {
        let new = crate::pref_dir().to_path_buf();
        if new != old && !new.is_dir() {
            fs::rename(old, new)?;
            debug!("Migrated old preferences.");
        }
    }

    // Migrate configuration over if found
    let mut cconfig: PathBuf = unsafe {
        let cpath = naevc::nfile_configPath();
        CStr::from_ptr(cpath).to_str().unwrap()
    }
    .into();
    cconfig.push("conf.lua");
    if cconfig.is_file() {
        let pref = crate::pref_dir();
        fs::create_dir_all(pref)?;
        let new = pref.join("conf.lua");
        if (cconfig != new) && !new.is_file() {
            fs::rename(cconfig, new)?;
        }
        debug!("Migrated configuration file.");
    }

    Ok(())
}

use std::sync::LazyLock;
pub static CONFIG_FILE: LazyLock<CString> = LazyLock::new(|| {
    let new = crate::pref_dir().join("conf.lua");
    CString::new(&*new.to_string_lossy()).unwrap()
});
/// Config file location for C code
#[unsafe(no_mangle)]
pub extern "C" fn ndata_configFile() -> *const c_char {
    CONFIG_FILE.as_ptr()
}
