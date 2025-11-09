use anyhow::Result;
use gettext::gettext;
use log::{debug, warn, warn_err};
use pluginmgr::plugin::Plugin;
use std::sync::LazyLock;

static PLUGINS: LazyLock<Vec<Plugin>> = LazyLock::new(|| {
    let mut plugins = match pluginmgr::local_plugins_dir() {
        Ok(path) => match pluginmgr::discover_local_plugins(path) {
            Ok(local) => local,
            Err(e) => {
                warn_err!(e);
                Vec::new()
            }
        },
        Err(e) => {
            warn_err!(e);
            Vec::new()
        }
    };
    plugins.sort_by(|a, b| a.priority.cmp(&b.priority));
    plugins
});

pub fn mount() -> Result<()> {
    debug!("{}", gettext("Loaded plugins:"));
    // reverse as we prepend
    for plugin in PLUGINS.iter().rev() {
        if let Some(mountpoint) = &plugin.mountpoint {
            if let Err(e) = ndata::physfs::mount(mountpoint, false) {
                warn_err!(e);
            }
        } else {
            warn!("Plugin '{}' is missing a mountpoint.", plugin.name);
        }
    }
    Ok(())
}

use crate::array::Array;
use std::ffi::{CString, c_char, c_int};

#[unsafe(no_mangle)]
pub extern "C" fn plugin_check() -> c_int {
    let mut i = 0;
    for plugin in PLUGINS.iter() {
        if !plugin.compatible {
            warn!(
                "Plugin '{}' does not support Naev version '{}'.",
                plugin.name,
                *log::version::VERSION
            );
            i += 1;
        }
    }
    i
}

static PLUGIN_DIR: LazyLock<CString> = LazyLock::new(|| {
    let dir = match pluginmgr::local_plugins_dir() {
        Ok(dir) => dir,
        Err(e) => {
            warn_err!(e);
            return Default::default();
        }
    };
    CString::new(dir.as_os_str().as_encoded_bytes()).unwrap()
});
#[unsafe(no_mangle)]
pub extern "C" fn plugin_dir() -> *const c_char {
    PLUGIN_DIR.as_ptr()
}

#[allow(dead_code)]
struct CPlugin(naevc::plugin_t);
unsafe impl Sync for CPlugin {}
unsafe impl Send for CPlugin {}
static PLUGIN_LIST: LazyLock<Array<CPlugin>> = LazyLock::new(|| {
    let mut out = Vec::new();
    for plugin in PLUGINS.iter() {
        let mp = if let Some(mp) = &plugin.mountpoint {
            CString::new(mp.as_os_str().as_encoded_bytes())
                .unwrap()
                .into_raw()
        } else {
            std::ptr::null_mut()
        };
        out.push(CPlugin(naevc::plugin_t {
            name: CString::new(plugin.name.as_str()).unwrap().into_raw(),
            author: CString::new(plugin.author.as_str()).unwrap().into_raw(),
            version: CString::new(plugin.version.to_string()).unwrap().into_raw(),
            description: CString::new(
                plugin
                    .description
                    .as_ref()
                    .map_or(plugin.r#abstract.as_str(), |s| s.as_str()),
            )
            .unwrap()
            .into_raw(),
            mountpoint: mp,
            total_conversion: plugin.total_conversion as c_int,
            compatible: plugin.compatible as c_int,
            priority: plugin.priority as c_int,
        }))
    }
    Array::new(&out).unwrap()
});
#[unsafe(no_mangle)]
pub extern "C" fn plugin_list() -> *const naevc::plugin_t {
    PLUGIN_LIST.as_ptr() as *const naevc::plugin_t
}

#[unsafe(no_mangle)]
pub extern "C" fn plugin_name(plg: *const naevc::plugin_t) -> *const c_char {
    if plg.is_null() {
        return std::ptr::null();
    }
    let plg = unsafe { &*plg };
    if plg.name.is_null() {
        plg.mountpoint
    } else {
        plg.name
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn plugin_manager() -> c_int {
    match pluginmgr_gui::open() {
        Ok(()) => 0,
        Err(e) => {
            warn_err!(e);
            1
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn plugin_manager_is_open() -> c_int {
    0
}
