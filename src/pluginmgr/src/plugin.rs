use anyhow::Result;
use serde::{Deserialize, Serialize, de};
use std::ops::Deref;
use std::path::{Path, PathBuf};

/// Small wrapper for our identifier that has additional deserialization checks
#[derive(Debug, Clone, Serialize)]
pub struct Identifier(String);
impl Deref for Identifier {
    type Target = String;
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl<'de> de::Deserialize<'de> for Identifier {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: de::Deserializer<'de>,
    {
        <String as de::Deserialize>::deserialize(deserializer).and_then(|inner| {
            if inner.len() > 25 {
                Err(de::Error::invalid_length(
                    inner.len(),
                    &"identifier exceeds maximum of 25 characters",
                ))
            } else if !inner.is_ascii() {
                Err(de::Error::invalid_value(
                    de::Unexpected::Str(&inner),
                    &"identifier contains non-ascii characters",
                ))
            } else if inner.contains(char::is_whitespace) {
                Err(de::Error::invalid_value(
                    de::Unexpected::Str(&inner),
                    &"identifier contains whitespace",
                ))
            } else {
                Ok(Self(inner))
            }
        })
    }
}

/// Represents a plugin stub, which is the minimum information necessary to look up and find the plugin
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct PluginStub {
    pub identifier: Identifier,
    pub name: String,
    pub(crate) source: Source,
    pub(crate) metadata: reqwest::Url,
}
impl PluginStub {
    pub fn from_slice(data: &[u8]) -> Result<Self> {
        let stub: Self = toml::from_slice(data)?;
        // Additional validation
        if !stub.identifier.is_ascii() {
            anyhow::bail!("identifier is not ascii");
        }
        Ok(stub)
    }

    pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let data = std::fs::read(&path)?;
        Self::from_slice(&data)
    }
}

#[derive(Debug, Clone, Copy, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum ReleaseStatus {
    Stable,
    Testing,
    Development,
}

#[derive(Debug, Clone, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Source {
    Git(reqwest::Url),
    Download(reqwest::Url),
    Local,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Plugin {
    pub identifier: Identifier,
    pub name: String,
    pub author: String,
    pub version: semver::Version,
    pub r#abstract: String,
    pub description: Option<String>,
    pub license: Option<String>,
    #[serde(default = "release_status_default")]
    pub release_status: ReleaseStatus,
    #[serde(default)]
    pub tags: Vec<String>,
    pub image_url: Option<reqwest::Url>,
    #[serde(default)]
    depends: Vec<String>,
    #[serde(default)]
    recommends: Vec<String>,
    naev_version: semver::VersionReq,
    source: Source,
    #[serde(default = "priority_default")]
    priority: i32,
    #[serde(default)]
    pub total_conversion: bool,
    // Fields below are set after loading
    #[serde(default, skip_deserializing)]
    pub compatible: bool,
    #[serde(default, skip_deserializing)]
    mountpoint: Option<PathBuf>,
}
fn release_status_default() -> ReleaseStatus {
    ReleaseStatus::Stable
}
fn priority_default() -> i32 {
    5
}

impl Plugin {
    pub fn from_slice(data: &[u8]) -> Result<Self> {
        let mut plugin: Self = toml::from_slice(data)?;
        // Additional validation
        if plugin.r#abstract.len() > 200 {
            anyhow::bail!("abstract exceeds 200 characters");
        }
        let naev = semver::Version::parse("0.13.0")?;
        plugin.compatible = plugin.naev_version.matches(&naev);
        Ok(plugin)
    }

    pub fn from_url<T: reqwest::IntoUrl>(url: T) -> Result<Self> {
        let response = reqwest::blocking::get(url)?;
        let content = response.bytes()?;
        Self::from_slice(&content)
    }

    pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let path = path.as_ref();
        let is_zip = path
            .extension()
            .and_then(|e| e.to_str())
            .map_or(false, |e| e.eq_ignore_ascii_case("zip"));

        if is_zip {
            let data = std::fs::read(&path)?;
            let mut plugin = Self::from_slice(&data)?;
            plugin.mountpoint = Some(path.to_owned());
            Ok(plugin)
        } else if path.is_dir() {
            let metadata = path.join("plugin.toml");
            if metadata.exists() {
                let data = std::fs::read(&path)?;
                let mut plugin = Self::from_slice(&data)?;
                plugin.mountpoint = Some(path.to_owned());
                Ok(plugin)
            } else {
                anyhow::bail!("directory without valid 'plugin.toml'");
            }
        } else {
            // Assume directly pointing at plugin.toml
            let data = std::fs::read(&path)?;
            let mut plugin = Self::from_slice(&data)?;
            plugin.mountpoint = path.parent().and_then(|e| Some(e.to_owned()));
            Ok(plugin)
        }
    }
}
