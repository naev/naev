use anyhow::Result;
use gettext::gettext;
use log::semver;
use log::{debug, warn, warn_err};
use pluginmgr::plugin::{Identifier, Plugin};
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::LazyLock;
use std::sync::atomic::{AtomicBool, Ordering};

static PLUGINS: LazyLock<Vec<Plugin>> = LazyLock::new(|| {
    let mut plugins = match pluginmgr::local_plugins_dir() {
        Ok(path) => match pluginmgr::discover_local_plugins(path) {
            Ok(mut local) => {
                local.retain(|p| !p.disabled);
                local
            }
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

fn changed() -> Result<bool> {
    fn plugins_to_hashmap(plugins: &[Plugin]) -> HashMap<Identifier, semver::Version> {
        plugins
            .iter()
            .map(|p| (p.identifier.clone(), p.version.clone()))
            .collect()
    }

    let loaded = plugins_to_hashmap(&PLUGINS);
    let installed = plugins_to_hashmap(&{
        let mut plgs = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;
        plgs.retain(|p| !p.disabled);
        plgs
    });

    Ok(loaded != installed)
}

fn blacklist_append(blk: &str) {
    let cstr = CString::new(blk).unwrap();
    unsafe {
        naevc::blacklist_append(cstr.as_ptr());
    }
}

fn whitelist_append(wht: &str) {
    let cstr = CString::new(wht).unwrap();
    unsafe {
        naevc::whitelist_append(cstr.as_ptr());
    }
}

static MOUNTED: AtomicBool = AtomicBool::new(false);
pub fn mount() -> Result<()> {
    fn load_plugin(plugin: &Plugin) -> Result<PathBuf> {
        if let Some(mountpoint) = &plugin.mountpoint {
            for blk in &plugin.blacklist {
                blacklist_append(blk);
            }
            for wht in &plugin.whitelist {
                whitelist_append(wht);
            }

            if plugin.total_conversion {
                for blk in [
                    "^ssys/.*\\.xml",
                    "^spob/.*\\.xml",
                    "^spob_virtual/.*\\.xml",
                    "^factions/.*\\.xml",
                    "^commodities/.*\\.xml",
                    "^ships/.*\\.xml",
                    "^outfits/.*\\.xml",
                    "^missions/.*\\.lua",
                    "^events/.*\\.lua",
                    "^tech/.*\\.xml",
                    "^asteroids/.*\\.xml",
                    "^unidiff/.*\\.xml",
                    "^map_decorator/.*\\.xml",
                    "^naevpedia/.*\\.xml",
                    "^intro",
                ] {
                    blacklist_append(blk);
                }
                //for wht in ["^events/settings.lua"] {
                //    whitelist_append(&wht);
                //}
                whitelist_append("^events/settings.lua");
            }
            Ok(mountpoint.to_path_buf())
        } else {
            anyhow::bail!(format!("Plugin '{}' is missing a mountpoint.", plugin.name));
        }
    }

    if !MOUNTED.load(Ordering::Relaxed) {
        MOUNTED.store(true, Ordering::Relaxed);

        debug!("{}", gettext("Loaded plugins:"));
        // reverse as we prepend
        let mountpoints: Vec<_> = PLUGINS
            .iter()
            .rev()
            .filter_map(|plugin| match load_plugin(plugin) {
                Ok(mp) => {
                    debug!(" * {}", &plugin.name);
                    Some(mp)
                }
                Err(e) => {
                    warn_err!(e);
                    None
                }
            })
            .collect();

        // The blacklist_init stuff is horrible and requires us to append the stuff first, before
        // mounting the plugins...
        unsafe {
            naevc::blacklist_init();
        }

        for mountpoint in mountpoints.iter() {
            match ndata::physfs::mount(mountpoint, false) {
                Ok(_) => (),
                Err(e) => {
                    warn_err!(e);
                }
            }
        }
    }
    Ok(())
}

static MANAGER_OPEN: AtomicBool = AtomicBool::new(false);
pub fn manager() -> Result<()> {
    if MANAGER_OPEN.load(Ordering::SeqCst) {
        anyhow::bail!("plugin manager already open!");
    }

    let title = CString::new(gettext("Plugin Manager"))?;
    let msg = CString::new(gettext("Please close the Plugin Manager to continue."))?;
    let wdw = unsafe {
        let w = 300;
        let h = 60 + naevc::gl_printHeightRaw(std::ptr::null(), w - 40, msg.as_ptr());
        let wdw = naevc::window_create(c"wdwPluginManager".as_ptr(), title.as_ptr(), -1, -1, w, h);
        naevc::window_addText(
            wdw,
            20,
            -40,
            w - 40,
            h,
            0,
            c"txtMsg".as_ptr(),
            std::ptr::null_mut(),
            std::ptr::null(),
            msg.as_ptr(),
        );
        wdw
    };

    let exe = std::env::current_exe()?;
    MANAGER_OPEN.store(true, Ordering::SeqCst);
    std::thread::spawn(move || {
        match cargo_util::ProcessBuilder::new(exe)
            .arg("--pluginmanager")
            .exec()
        {
            Ok(()) => (),
            Err(e) => {
                warn_err!(e);
            }
        };
        // TODO fix this, not actually thread safe...
        unsafe {
            naevc::window_destroy(wdw);
        }

        // Hack for now, TODO should be handled in the options menu in the future... (also not
        // thread safe)
        if let Ok(chg) = changed()
            && chg
        {
            unsafe {
                let mut event = naevc::SDL_Event {
                    user: naevc::SDL_UserEvent {
                        type_: naevc::SDL_NEEDSRESTART,
                        reserved: 0,
                        timestamp: 0,
                        windowID: 0,
                        code: 0,
                        data1: std::ptr::null_mut(),
                        data2: std::ptr::null_mut(),
                    },
                };
                naevc::SDL_PushEvent(&mut event);
                naevc::opt_needRestart();
            }
        }

        MANAGER_OPEN.store(false, Ordering::SeqCst);
    });

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
    match manager() {
        Ok(()) => 0,
        Err(e) => {
            warn_err!(e);
            -1
        }
    }
}
