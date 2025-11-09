//pub mod install;
//pub mod mount;
pub mod plugin;

use crate::plugin::{Plugin, PluginStub};
use anyhow::Result;
use camino::Utf8PathBuf;
use directories::BaseDirs;
use log::warn_err;
use std::fs;
use std::path::Path;

pub fn discover_local_plugins<P: AsRef<Path>>(root: P) -> Result<Vec<Plugin>> {
    let mut out = Vec::new();
    if !root.as_ref().exists() {
        return Ok(out);
    }
    for entry in fs::read_dir(&root)? {
        match Plugin::from_path(&entry?.path().as_path()) {
            Ok(plugin) => out.push(plugin),
            Err(e) => {
                warn_err!(e);
                continue;
            }
        }
    }
    Ok(out)
}

pub fn repository<P: AsRef<Path>>(root: P) -> Result<Vec<PluginStub>> {
    let plugins_dir = root.as_ref().join("plugins");
    let mut out = Vec::new();
    if !plugins_dir.exists() {
        return Ok(out);
    }
    for entry in fs::read_dir(plugins_dir)? {
        let entry = entry?;
        let path = entry.path();
        if path.extension().and_then(|s| s.to_str()) != Some("toml") {
            continue;
        }
        match PluginStub::from_path(path.as_path()) {
            Ok(plugin) => out.push(plugin),
            Err(e) => {
                warn_err!(e);
                continue;
            }
        }
    }
    Ok(out)
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

/// Returns the Naev plugins directory for the current platform.
pub fn local_plugins_dir() -> anyhow::Result<Utf8PathBuf> {
    let base = BaseDirs::new().ok_or_else(|| anyhow::anyhow!("No home directory found"))?;

    #[cfg(target_os = "linux")]
    let p = Utf8PathBuf::from_path_buf(base.data_dir().join("naev").join("plugins"))
        .map_err(|_| anyhow::anyhow!("Path is not valid UTF-8"))?;

    #[cfg(target_os = "macos")]
    let p = Utf8PathBuf::from_path_buf(base.data_dir().join("org.naev.Naev").join("plugins"))
        .map_err(|_| anyhow::anyhow!("Path is not valid UTF-8"))?;

    #[cfg(target_os = "windows")]
    let p = Utf8PathBuf::from_path_buf(base.data_dir().join("naev").join("plugins"))
        .map_err(|_| anyhow::anyhow!("Path is not valid UTF-8"))?;

    Ok(p)
}

/*
/// Summary of a plugin update operation.
#[derive(Debug, Clone, Serialize)]
pub struct UpdateSummary {
    pub name: String,
    pub status: String,
}

/// Validation summary result for a plugin.
#[derive(Debug, Clone, Serialize)]
pub struct ValidationSummary {
    pub name: String,
    pub valid: bool,
    pub issues: Vec<String>,
}

/// Checks all installed plugins against the repository and updates them if needed.
/// Works for both Git and ZIP-based plugins.
pub fn update_all(
    repo: &Repository,
    lm: &LocalManager,
    inst: &Installer,
) -> Result<Vec<UpdateSummary>> {
    let repo_plugins = repo.list_plugins()?;
    let local_plugins = lm.list_installed()?;

    let mut summaries = Vec::new();

    for plugin in repo_plugins {
        let local = local_plugins
            .iter()
            .find(|p| p.name.eq_ignore_ascii_case(&plugin.name));

        let mut status = "Up to date".to_string();

        if let Some(local) = local {
            // Determine if update needed
            let needs_update = match (&plugin.version, &local.version) {
                (Some(r), Some(l)) if r > l => true,
                _ if matches!(plugin.source_kind(), Some(SourceKind::Git)) => {
                    inst.check_git_update(&plugin)?.is_some()
                }
                _ => false,
            };

            if needs_update {
                match plugin.source_kind() {
                    Some(SourceKind::Git) => {
                        inst.update_git_plugin(&plugin)?;
                        status = "Updated (git)".to_string();
                    }
                    Some(SourceKind::Link) => {
                        inst.update_zip_plugin(&plugin)?;
                        status = "Updated (zip)".to_string();
                    }
                    None => status = "Unknown source".to_string(),
                }
            }
        } else {
            status = "Not installed".to_string();
        }

        summaries.push(UpdateSummary {
            name: plugin.name.clone(),
            status,
        });
    }

    Ok(summaries)
}

/// Validate all installed plugins (both zip and folder types).
pub fn validate_all(lm: &LocalManager) -> Result<Vec<ValidationSummary>> {
    let local_plugins = lm.list_installed()?;
    let mut results = Vec::new();

    for plugin in local_plugins {
        let path = &plugin.path;
        let mut issues = Vec::new();

        let result = validate_plugin(path);

        match result {
            Ok(report) if report.is_valid() => {
                results.push(ValidationSummary {
                    name: plugin.name.clone(),
                    valid: true,
                    issues,
                });
            }
            Ok(report) => {
                issues.extend(report.errors.clone());
                results.push(ValidationSummary {
                    name: plugin.name.clone(),
                    valid: false,
                    issues,
                });
            }
            Err(e) => {
                results.push(ValidationSummary {
                    name: plugin.name.clone(),
                    valid: false,
                    issues: vec![format!("Validation error: {}", e)],
                });
            }
        }
    }

    Ok(results)
}
*/
