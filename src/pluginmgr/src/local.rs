use crate::mount::PluginSource;
use crate::{
    model::{InstalledPluginInfo, LocalPlugin},
    naev_plugins_dir,
};
use anyhow::Result;
use quick_xml::de::from_str;
use std::fs;
use std::path::PathBuf;

pub struct LocalManager {
    pub root: PathBuf,
}

impl LocalManager {
    pub fn discover() -> Result<Self> {
        let p = naev_plugins_dir()?;
        Ok(Self {
            root: PathBuf::from(p.as_str()),
        })
    }

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

    pub fn list_installed(&self) -> Result<Vec<LocalPlugin>> {
        let mut out = Vec::new();
        if !self.root.exists() {
            return Ok(out);
        }

        for entry in fs::read_dir(&self.root)? {
            let entry = entry?;
            let path = entry.path();

            // Handle both folders and zip files
            let is_zip = path
                .extension()
                .and_then(|e| e.to_str())
                .map_or(false, |e| e.eq_ignore_ascii_case("zip"));

            let mut name = path
                .file_stem()
                .unwrap_or_default()
                .to_string_lossy()
                .to_string();
            let mut author = None;
            let mut version = None;
            let mut description = None;
            let mut source = None;

            // --- Read plugin.xml from either directory or zip ---
            if is_zip {
                // Mount ZIP and read plugin.xml inside it
                let file = fs::File::open(&path)?;
                let mut zip = zip::ZipArchive::new(file)?;

                if zip.file_names().any(|n| n == "plugin.xml") {
                    let mut xml_data = String::new();
                    {
                        // Limit the lifetime of the borrow
                        use std::io::Read;
                        let mut f = zip.by_name("plugin.xml")?;
                        f.read_to_string(&mut xml_data)?;
                    }

                    if let Ok(info) = from_str::<InstalledPluginInfo>(&xml_data) {
                        name = info.name;
                        if !info.author.is_empty() {
                            author = Some(info.author);
                        }
                        version = info.version;
                        description = info.description;
                        source = info.source;
                    }
                }
            } else if path.is_dir() {
                // Check for plugin.xml in directory
                let plugin_xml = path.join("plugin.xml");
                if plugin_xml.exists() {
                    if let Ok(xml) = fs::read_to_string(&plugin_xml) {
                        if let Ok(info) = from_str::<InstalledPluginInfo>(&xml) {
                            name = info.name;
                            if !info.author.is_empty() {
                                author = Some(info.author);
                            }
                            version = info.version;
                            description = info.description;
                            source = info.source;
                        }
                    }
                }
            } else {
                continue;
            }

            out.push(LocalPlugin {
                name,
                author,
                version,
                description,
                source,
                path: path.to_string_lossy().to_string(),
            });
        }

        Ok(out)
    }

    pub fn ensure_root(&self) -> Result<()> {
        fs::create_dir_all(&self.root)?;
        Ok(())
    }
}
