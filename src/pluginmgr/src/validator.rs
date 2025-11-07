use crate::model::InstalledPluginInfo;
use crate::mount::PluginSource;
use anyhow::{Context, Result};
use quick_xml::de::from_str;
use serde::Serialize;
use std::path::Path;

/// Result of a plugin validation operation.
#[derive(Debug, Clone, Serialize)]
pub struct PluginValidationResult {
    pub name: String,
    pub valid: bool,
    pub errors: Vec<String>,
}

impl PluginValidationResult {
    pub fn is_valid(&self) -> bool {
        self.valid && self.errors.is_empty()
    }
}

/// Validates a mounted plugin by checking required files and metadata.
pub fn validate_plugin<P: AsRef<Path>>(plugin_path: P) -> Result<PluginValidationResult> {
    let path = plugin_path.as_ref();
    let mut mount = PluginSource::open(path)?;

    let mut errors = Vec::new();
    let mut valid = true;
    let mut name = path
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("unknown")
        .to_string();

    // --- Step 1: Ensure plugin.xml exists ---
    let files = mount.list_files()?;
    if !files.iter().any(|f| f.ends_with("plugin.xml")) {
        errors.push("Missing plugin.xml".into());
        valid = false;
        return Ok(PluginValidationResult {
            name,
            valid,
            errors,
        });
    }

    // --- Step 2: Try to parse plugin.xml ---
    let xml_bytes = mount
        .read_file("plugin.xml")
        .context("Failed to read plugin.xml")?;
    let xml_text = String::from_utf8_lossy(&xml_bytes);
    match from_str::<InstalledPluginInfo>(&xml_text) {
        Ok(info) => {
            name = info.name.clone();

            if info.author.trim().is_empty() {
                errors.push("Missing <author>".into());
            }
            if info.version.is_none() {
                errors.push("Missing <version>".into());
            }
            if info.source.is_none() {
                errors.push("Missing <source>".into());
            }
        }
        Err(e) => {
            errors.push(format!("Invalid XML: {e}"));
        }
    }

    if !errors.is_empty() {
        valid = false;
    }

    Ok(PluginValidationResult {
        name,
        valid,
        errors,
    })
}
