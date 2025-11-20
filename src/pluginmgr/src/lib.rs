pub mod git;
pub mod install;
pub mod plugin;

use crate::install::Progress;
use crate::plugin::{Plugin, PluginStub};
use anyhow::{Context, Error, Result};
use formatx::formatx;
use iced::task::{Sipper, Straw, sipper};
use log::gettext::pgettext;
use log::warn_err;
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

pub fn discover_local_plugins<P: AsRef<Path>>(root: P) -> Result<Vec<Plugin>> {
    if !root.as_ref().exists() {
        return Ok(Vec::new());
    }
    Ok(fs::read_dir(&root)?
        .filter_map(|entry| {
            let entry = match entry {
                Ok(entry) => entry,
                Err(e) => {
                    warn_err!(e);
                    return None;
                }
            };
            let path = entry.path();
            match Plugin::from_path(&path) {
                Ok(plugin) => Some(plugin),
                Err(e) => {
                    warn_err!(e);
                    Some(Plugin::from_error(&path, e))
                }
            }
        })
        .collect())
}

pub fn discover_remote_plugins<T: reqwest::IntoUrl>(
    url: T,
    branch: &str,
) -> impl Straw<Vec<Plugin>, Progress, Error> {
    sipper(async move |mut sender| {
        use base64::{Engine as _, engine::general_purpose::URL_SAFE};
        let repo_hash = URL_SAFE.encode(format!("{}:{}", url.as_str(), branch));
        let repo_path = cache_dir()?.join("plugins-repo");
        fs::create_dir_all(&repo_path)?;
        let repo_path = repo_path.join(repo_hash);
        let urlstr = url.as_str().to_string();

        let sendmap = |mut prog: Progress| -> Progress {
            prog.value *= 0.2;
            prog
        };
        let repo = if repo_path.exists() {
            sender
                .send(Progress {
                    message: formatx!(pgettext("plugins", "Updating plugin remote {}"), &urlstr)
                        .ok(),
                    value: 0.0,
                })
                .await;
            git::pull(&repo_path).with(sendmap).run(&sender).await?
        } else {
            sender
                .send(Progress {
                    message: formatx!(pgettext("plugins", "Cloning plugin remote {}"), &urlstr)
                        .ok(),
                    value: 0.0,
                })
                .await;
            git::clone(&repo_path, url)
                .with(sendmap)
                .run(&sender)
                .await?
        };
        let workdir = repo.workdir().context("naev-plugins directory is bare")?;
        sender
            .send(Progress {
                message: formatx!(pgettext("plugins", "Updating plugins found in {}"), &urlstr)
                    .ok(),
                value: 0.2,
            })
            .await;

        fn load_stub(stub: &PluginStub) -> impl Straw<Plugin, f32, Error> {
            sipper(async move |mut sender| {
                let ret = stub.to_plugin_async().await;
                sender.send(1.0.into()).await;
                ret
            })
        }
        use futures::StreamExt;
        let repo = repository(workdir)?;
        let inc = 0.8 / (repo.len() as f32);
        let progress = Arc::new(Mutex::new(0.2));
        Ok(futures::stream::iter(repo)
            .filter_map(|stub| {
                let value = sender.clone();
                let progress = progress.clone();
                async move {
                    match load_stub(&stub)
                        .with(|val| {
                            let mut lock = progress.lock().unwrap();
                            *lock += val * inc;
                            (*lock).into()
                        })
                        .run(&value)
                        .await
                    {
                        Ok(plugin) => Some(plugin),
                        Err(e) => {
                            warn_err!(e);
                            None
                        }
                    }
                }
            })
            .collect()
            .await)
    })
}

pub fn repository<P: AsRef<Path>>(root: P) -> Result<Vec<PluginStub>> {
    let plugins_dir = root.as_ref().join("plugins");
    if !plugins_dir.exists() {
        return Ok(Vec::new());
    }
    Ok(fs::read_dir(&plugins_dir)?
        .filter_map(|entry| {
            let entry = match entry {
                Ok(entry) => entry,
                Err(e) => {
                    warn_err!(e);
                    return None;
                }
            };
            let path = entry.path();
            if path.extension().and_then(|s| s.to_str()) != Some("toml") {
                return None;
            }
            match PluginStub::from_path(path.as_path()) {
                Ok(plugin) => Some(plugin),
                Err(e) => {
                    warn_err!(e);
                    None
                }
            }
        })
        .collect())
}
/*
    /// Opens a mounted view of a local plugin (directory or zip).
    pub fn mount_plugin(&self, name: &str) -> Result<PluginSource> {
        let plugin_path = self.root.join(name);

        // Some plugins are stored as zips; some as directories
        let zip_path = plugin_path.with_extension("zip");

        if zip_path.exists() {
            Ok(PluginSource::open(zip_path)?)
        } else {
            Ok(PluginSource::open(plugin_path)?)
        }
    }

    pub fn ensure_root(&self) -> Result<()> {
        fs::create_dir_all(&self.root)?;
        Ok(())
    }
*/

pub fn cache_dir() -> Result<PathBuf> {
    let proj_dirs = directories::ProjectDirs::from("org", "naev", "naev")
        .context("getting project directorios")?;
    Ok(proj_dirs.cache_dir().to_path_buf())
}

pub fn write_dir() -> Result<PathBuf> {
    /*
    use directories::BaseDirs;
    let base = BaseDirs::new().ok_or_else(|| anyhow::anyhow!("No home directory found"))?;

    #[cfg(target_os = "linux")]
    let p = base.data_dir().join("naev");

    #[cfg(target_os = "macos")]
    let p = base.data_dir().join("org.naev.Naev");

    #[cfg(target_os = "windows")]
    let p = base.data_dir().join("naev");
    Ok(p)
    */
    Ok(Path::new(&ndata::physfs::get_write_dir()).to_path_buf())
}

/// Returns the Naev plugins directory for the current platform.
pub fn local_plugins_dir() -> Result<PathBuf> {
    Ok(write_dir()?.join("plugins"))
}

pub fn local_plugins_disabled_dir() -> Result<PathBuf> {
    Ok(write_dir()?.join("plugins-disabled"))
}
