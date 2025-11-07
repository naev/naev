use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum SourceKind {
    Git,
    Link,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename = "plugin")]
pub struct PluginInfo {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "author", default)]
    pub author: String,
    #[serde(rename = "license", default)]
    pub license: Option<String>,
    #[serde(rename = "website", default)]
    pub website: Option<String>,
    #[serde(rename = "git", default)]
    pub git: Option<String>,
    #[serde(rename = "link", default)]
    pub link: Option<String>,
    #[serde(rename = "version", default)]
    pub version: Option<String>,
}

impl PluginInfo {
    pub fn source_kind(&self) -> Option<SourceKind> {
        match (&self.git, &self.link) {
            (Some(_), _) => Some(SourceKind::Git),
            (None, Some(_)) => Some(SourceKind::Link),
            _ => None,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename = "plugin")]
pub struct InstalledPluginInfo {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "author", default)]
    pub author: String,
    #[serde(rename = "version", default)]
    pub version: Option<String>,
    #[serde(rename = "naev_version", default)]
    pub naev_version: Option<String>,
    #[serde(rename = "description", default)]
    pub description: Option<String>,
    #[serde(rename = "source", default)]
    pub source: Option<String>,
}

#[derive(Debug, Clone, Serialize)]
pub struct LocalPlugin {
    pub name: String,
    pub author: Option<String>,
    pub version: Option<String>,
    pub description: Option<String>,
    pub source: Option<String>,
    pub path: String,
}
