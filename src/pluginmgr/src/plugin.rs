use anyhow::Result;
use serde::{Deserialize, Serialize, de};
use std::ops::Deref;
use std::path::{Path, PathBuf};

// To not pull in all of gettext
#[allow(non_snake_case)]
pub const fn N_(s: &str) -> &str {
    s
}

/// Small wrapper for our identifier that has additional deserialization checks
#[derive(Debug, Clone, derive_more::Display, Serialize, PartialEq, Eq, Hash)]
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

    pub fn to_plugin(&self) -> Result<Plugin> {
        Plugin::from_url(self.metadata.clone())
    }
}

#[derive(Debug, Clone, Copy, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum ReleaseStatus {
    Stable,
    Testing,
    Development,
}
impl ReleaseStatus {
    pub const fn as_str(&self) -> &'static str {
        match self {
            ReleaseStatus::Stable => N_("stable"),
            ReleaseStatus::Testing => N_("testing"),
            ReleaseStatus::Development => N_("development"),
        }
    }
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
    pub depends: Vec<String>,
    #[serde(default)]
    pub recommends: Vec<String>,
    pub naev_version: semver::VersionReq,
    pub source: Source,
    #[serde(default = "priority_default")]
    pub priority: i32,
    #[serde(default)]
    pub total_conversion: bool,
    #[serde(default)]
    pub blacklist: Vec<String>,
    #[serde(default)]
    pub whitelist: Vec<String>,
    // Fields below are set after loading
    #[serde(default, skip_deserializing)]
    pub compatible: bool,
    #[serde(default, skip_deserializing)]
    pub mountpoint: Option<PathBuf>,
}
impl PartialEq for Plugin {
    fn eq(&self, other: &Self) -> bool {
        self.identifier == other.identifier
    }
}
impl Eq for Plugin {}
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
        plugin.compatible = plugin
            .naev_version
            .matches(&log::version::VERSION_WITHOUT_PRERELEASE);
        Ok(plugin)
    }

    pub fn from_url<T: reqwest::IntoUrl>(url: T) -> Result<Self> {
        let response = reqwest::blocking::get(url)?;
        let content = response.bytes()?;
        Self::from_slice(&content)
    }

    pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let path = path.as_ref();

        if path.is_dir() {
            let metadata = path.join("plugin.toml");
            if metadata.exists() {
                let data = std::fs::read(metadata)?;
                let mut plugin = Self::from_slice(&data)?;
                plugin.mountpoint = Some(path.to_owned());
                Ok(plugin)
            } else {
                anyhow::bail!("directory without valid 'plugin.toml'");
            }
        // Is zip file?
        } else if path
            .extension()
            .and_then(|e| e.to_str())
            .is_some_and(|e| e.eq_ignore_ascii_case("zip"))
        {
            let data = std::fs::read(path)?;
            let mut plugin = Self::from_slice(&data)?;
            plugin.mountpoint = Some(path.to_owned());
            Ok(plugin)
        } else {
            // Assume directly pointing at plugin.toml
            let data = std::fs::read(path)?;
            let mut plugin = Self::from_slice(&data)?;
            plugin.mountpoint = path.parent().map(|e| e.to_owned());
            Ok(plugin)
        }
    }
}
