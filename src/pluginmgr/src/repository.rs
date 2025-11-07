use crate::model::PluginInfo;
use anyhow::Result;
use quick_xml::de::from_str;
use std::fs;
use std::path::Path;

/// A simple repository reader that expects a `plugins/` directory of XML files.
#[derive(Debug, Clone)]
pub struct Repository {
    pub root: String, // path OR a checked-out git workdir; future: remote URL support
}

impl Repository {
    pub fn from_local_path<P: AsRef<Path>>(root: P) -> Self {
        Self {
            root: root.as_ref().to_string_lossy().into_owned(),
        }
    }

    /// Lists plugins in `root/plugins/*.xml`.
    pub fn list_plugins(&self) -> Result<Vec<PluginInfo>> {
        let plugins_dir = Path::new(&self.root).join("plugins");
        let mut out = Vec::new();
        if !plugins_dir.exists() {
            return Ok(out);
        }
        for entry in fs::read_dir(plugins_dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.extension().and_then(|s| s.to_str()) != Some("xml") {
                continue;
            }
            let xml = fs::read_to_string(&path)?;
            match from_str::<PluginInfo>(&xml) {
                Ok(mut p) => {
                    if p.name.is_empty() {
                        // In the format, name is an attribute of <plugin>. If missing, try filename.
                        if let Some(stem) = path.file_stem().and_then(|s| s.to_str()) {
                            p.name = stem.to_string();
                        }
                    }
                    out.push(p);
                }
                Err(e) => {
                    tracing::warn!(?path, error = %e, "Failed to parse plugin info XML");
                }
            }
        }
        Ok(out)
    }
}
