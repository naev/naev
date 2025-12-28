use crate::install::Progress;
use anyhow::{Error, Result};
use fs_err as fs;
use iced::task::{Straw, sipper};
use serde::{Deserialize, Serialize, de};
use std::io::Write;
use std::ops::Deref;
use std::path::{Path, PathBuf};

// To not pull in all of gettext
#[allow(non_snake_case)]
pub const fn N_(s: &str) -> &str {
    s
}

/// Error identifier. Plugin identifiers are not allowed to start with this string.
const ID_ERROR: &str = "ERROR";

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
            } else if !inner.chars().all(|c| c.is_ascii_alphanumeric()) {
                Err(de::Error::invalid_value(
                    de::Unexpected::Str(&inner),
                    &"identifier contains non-ascii alphanumeric characters",
                ))
            } else if inner.starts_with(ID_ERROR) {
                Err(de::Error::invalid_value(
                    de::Unexpected::Str(&inner),
                    &"identifier can not start with 'ERROR'",
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
        let data = fs::read(&path)?;
        Self::from_slice(&data)
    }

    pub fn to_plugin(&self) -> Result<Plugin> {
        Plugin::from_url(self.metadata.clone())
    }

    pub async fn to_plugin_async(&self) -> Result<Plugin> {
        Plugin::from_url_async(self.metadata.clone()).await
    }

    pub fn to_plugin_straw(&self) -> impl Straw<Plugin, Progress, Error> {
        sipper(async move |mut sender| {
            let ret = self.to_plugin_async().await;
            sender.send(1.0.into()).await;
            ret
        })
    }
}

/// The status of a plugin
#[derive(Debug, Clone, Copy, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum ReleaseStatus {
    Stable,
    Testing,
    Development,
}
impl ReleaseStatus {
    /// Gets the status as a human interpretable string
    pub const fn as_str(&self) -> &'static str {
        match self {
            ReleaseStatus::Stable => N_("stable"),
            ReleaseStatus::Testing => N_("testing"),
            ReleaseStatus::Development => N_("development"),
        }
    }

    /// Gets the description.
    pub const fn description(&self) -> &'static str {
        match self {
            ReleaseStatus::Stable => N_("The plugin is completely playable."),
            ReleaseStatus::Testing => {
                N_("The plugin is being tested, and may contain bugs or other issues.")
            }
            ReleaseStatus::Development => N_(
                "The plugin is under heavy development and will be full of missing content and bugs.",
            ),
        }
    }
}

/// A source location of a plugin used to update or download them
#[derive(Debug, Clone, Deserialize, Serialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum Source {
    Git(reqwest::Url),
    Download(reqwest::Url),
    Local,
}

#[derive(Debug, Clone, Deserialize, Serialize, PartialEq, Eq)]
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
    #[serde(default = "source_default")]
    pub source: Source,
    #[serde(default = "priority_default")]
    pub priority: i32,
    #[serde(default)]
    pub total_conversion: bool,
    #[serde(default)]
    pub blacklist: Vec<String>,
    #[serde(default)]
    pub whitelist: Vec<String>,
    #[serde(default)]
    pub compatible: bool,
    #[serde(default)]
    pub mountpoint: Option<PathBuf>,
    #[serde(default)]
    pub disabled: bool,
}
const fn release_status_default() -> ReleaseStatus {
    ReleaseStatus::Stable
}
const fn priority_default() -> i32 {
    5
}
const fn source_default() -> Source {
    Source::Local
}

impl Plugin {
    /// Generates a plugin from a path and an error, useful to load plugins that have errors and
    /// visualize them to the player.
    pub fn from_error<P: AsRef<Path>>(path: P, err: anyhow::Error) -> Self {
        let path = path.as_ref();
        let strerr = ID_ERROR.to_string();
        let disabled = Self::is_disabled(path);
        Self {
            identifier: Identifier(if let Some(filename) = path.file_name() {
                format!("{}-{}", &strerr, filename.to_string_lossy())
            } else {
                strerr.clone()
            }),
            name: if let Some(filename) = path.file_name() {
                filename.to_string_lossy().to_string()
            } else {
                (*path.to_string_lossy()).to_string()
            },
            author: (*path.to_string_lossy()).to_string(),
            version: semver::Version::new(0, 0, 0),
            r#abstract: strerr.clone(),
            description: Some(format!("Error:\n{}", &err.to_string())),
            license: None,
            release_status: ReleaseStatus::Development,
            tags: Vec::new(),
            image_url: None,
            depends: Vec::new(),
            recommends: Vec::new(),
            naev_version: semver::VersionReq::STAR,
            source: source_default(),
            priority: priority_default(),
            total_conversion: false,
            blacklist: Vec::new(),
            whitelist: Vec::new(),
            compatible: true,
            mountpoint: Some(path.to_path_buf()),
            disabled,
        }
    }

    /// Checks to see if a plugin located at a specific path is disabled or not.
    pub fn is_disabled<P: AsRef<Path>>(path: P) -> bool {
        let path = path.as_ref();
        if let Some(filename) = path.file_name() {
            path.parent()
                .map(|p| {
                    p.join(format!(".{}.disabled", filename.to_string_lossy()))
                        .exists()
                })
                .unwrap_or(false)
        } else {
            false
        }
    }

    /// Disables or enables a plugin.
    pub fn disable(&self, disable: bool) -> Result<()> {
        if let Some(path) = &self.mountpoint
            && let Some(filename) = path.file_name()
            && let Some(disabled) = path
                .parent()
                .map(|p| p.join(format!(".{}.disabled", filename.to_string_lossy())))
        {
            if disabled.exists() {
                if !disable {
                    fs::remove_file(disabled)?;
                }
            } else if disable {
                let mut file = fs::File::create(disabled)?;
                file.write_all(b"disabled")?;
            }
            Ok(())
        } else {
            anyhow::bail!("failed set disable state");
        }
    }

    /// Checks to see if a plugin is compatible with the current version.
    pub fn check_compatible(&mut self) -> bool {
        self.compatible = self
            .naev_version
            .matches(&nlog::version::VERSION_WITHOUT_PRERELEASE);
        self.compatible
    }

    /// Loads a plugin from a slice, but doesn't set extra data like the mountpoint or whether or
    /// not it is disabled.
    pub fn from_slice(data: &[u8]) -> Result<Self> {
        let mut plugin: Self = toml::from_slice(data)?;
        // Additional validation
        if plugin.r#abstract.len() > 200 {
            anyhow::bail!(format!(
                "plugin '{}' abstract exceeds 200 characters",
                &plugin.name
            ));
        }
        plugin.compatible = plugin
            .naev_version
            .matches(&nlog::version::VERSION_WITHOUT_PRERELEASE);
        Ok(plugin)
    }

    /// Loads a plugin description from a description file at a url.
    pub fn from_url<T: reqwest::IntoUrl>(url: T) -> Result<Self> {
        let response = reqwest::blocking::get(url)?;
        let content = response.bytes()?;
        let mut plugin = Self::from_slice(&content)?;
        plugin.mountpoint = None; // Make sure it's disabled
        plugin.disabled = false;
        Ok(plugin)
    }

    /// Loads a plugin description from a description file at a url asynchronously.
    pub async fn from_url_async<T: reqwest::IntoUrl>(url: T) -> Result<Self> {
        let response = reqwest::get(url).await?;
        let content = response.bytes().await?;
        let mut plugin = Self::from_slice(&content)?;
        plugin.mountpoint = None;
        plugin.disabled = false;
        Ok(plugin)
    }

    /// Tries to load a plugin from a path. Will store the mountpoint and disabled status too.
    pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let path = path.as_ref();
        let disabled = Self::is_disabled(path);

        if path.is_dir() {
            let metadata = path.join("plugin.toml");
            if metadata.exists() {
                let data = fs::read(metadata)?;
                let mut plugin = Self::from_slice(&data)?;
                plugin.mountpoint = Some(path.to_owned());
                plugin.disabled = disabled;
                Ok(plugin)
            } else {
                anyhow::bail!(format!(
                    "plugin directory '{}' without valid 'plugin.toml'",
                    path.display()
                ));
            }
        // Is zip file?
        } else if path
            .extension()
            .and_then(|e| e.to_str())
            .is_some_and(|e| e.eq_ignore_ascii_case("zip"))
        {
            let data = fs::read(path)?;
            let mut plugin = Self::from_slice(&data)?;
            plugin.mountpoint = Some(path.to_owned());
            plugin.disabled = disabled;
            Ok(plugin)
        } else {
            // Assume directly pointing at plugin.toml
            let data = fs::read(path)?;
            let mut plugin = Self::from_slice(&data)?;
            plugin.mountpoint = path.parent().map(|e| e.to_owned());
            plugin.disabled = disabled;
            Ok(plugin)
        }
    }
}
