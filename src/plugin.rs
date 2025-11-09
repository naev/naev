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

use std::ffi::{c_char, c_int};

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

#[unsafe(no_mangle)]
pub extern "C" fn plugin_dir() -> *const c_char {
    std::ptr::null()
}

#[unsafe(no_mangle)]
pub extern "C" fn plugin_list() -> *const naevc::plugin_t {
    std::ptr::null()
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
