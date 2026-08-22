use anyhow::Result;
use fs_err as fs;
use iced::widget;
use itertools::Itertools;
use nlog::gettext::{N_, pgettext};
use nlog::warn_err;
use pluginmgr::plugin::{Identifier, Plugin};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::io::Write;
use std::ops::Deref;
use std::path::{Path, PathBuf};
use std::sync::{LazyLock, Mutex};

/// A remote plugin repository.
#[derive(Debug, Clone, Deserialize, Serialize, PartialEq)]
pub struct Remote {
   pub url: reqwest::Url,
   pub mirror: Option<reqwest::Url>,
   pub branch: String,
}

static REMOTES_DEFAULT: LazyLock<Vec<Remote>> = LazyLock::new(|| {
   vec![Remote {
      url: reqwest::Url::parse("https://codeberg.org/naev/naev-plugins").unwrap(),
      mirror: Some(reqwest::Url::parse("https://github.com/naev/naev-plugins").unwrap()),
      branch: "main".to_string(),
   }]
});

/// Location of the plugins directory.
pub fn local_plugins_dir() -> PathBuf {
   pluginmgr::local_plugins_dir().unwrap()
}

/// Location of the cache directory for storing information about plugins.
pub fn catalog_cache_dir() -> PathBuf {
   ndata::cache_dir().join("pluginmanager")
}

pub static CONFIG_FILE: LazyLock<PathBuf> =
   LazyLock::new(|| ndata::pref_dir().join("pluginmanager.toml"));

/// To skip serializing if default.
pub fn skip_remotes(remotes: &Vec<Remote>) -> bool {
   REMOTES_DEFAULT.deref() == remotes
}
/// To set the default remotes if not found.
pub fn default_remotes() -> Vec<Remote> {
   REMOTES_DEFAULT.clone()
}
const REFRESH_INTERVAL_DEFAULT: chrono::TimeDelta = chrono::TimeDelta::days(1);
pub fn skip_refresh_interval(interval: &chrono::TimeDelta) -> bool {
   *interval == REFRESH_INTERVAL_DEFAULT
}
pub fn default_refresh_interval() -> chrono::TimeDelta {
   REFRESH_INTERVAL_DEFAULT
}

/// Plugin manager configuration.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Conf {
   #[serde(skip_serializing_if = "skip_remotes", default = "default_remotes")]
   pub remotes: Vec<Remote>,
   #[serde(
      skip_serializing_if = "skip_refresh_interval",
      default = "default_refresh_interval"
   )]
   pub refresh_interval: chrono::TimeDelta,
   #[serde(skip, default = "local_plugins_dir")]
   pub install_path: PathBuf,
   #[serde(skip, default = "catalog_cache_dir")]
   pub catalog_cache: PathBuf,
}
impl Conf {
   pub fn new() -> Result<Self> {
      Ok(Self {
         remotes: REMOTES_DEFAULT.clone(),
         install_path: pluginmgr::local_plugins_dir()?,
         catalog_cache: ndata::cache_dir().join("pluginmanager"),
         refresh_interval: default_refresh_interval(),
      })
   }
}

/// Different potential plugin states.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
#[serde(rename_all = "lowercase")]
pub enum PluginState {
   Installed,
   Disabled,
   Available,
}
impl PluginState {
   pub const fn as_str(&self) -> &'static str {
      match self {
         PluginState::Installed => N_("installed"),
         PluginState::Disabled => N_("disabled"),
         PluginState::Available => N_("available"),
      }
   }
}

/// A wrapper containing local and remote information about plugins.
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct PluginWrap {
   pub identifier: Identifier,
   pub local: Option<Plugin>,
   pub remote: Option<Plugin>,
   pub state: PluginState,
   #[serde(skip, default)]
   pub image: Option<iced::advanced::image::Handle>,
   #[serde(skip, default)]
   pub description_md: Option<Vec<widget::markdown::Item>>,
}
impl PluginWrap {
   pub fn new_local(plugin: &Plugin, state: PluginState) -> Self {
      PluginWrap {
         identifier: plugin.identifier.clone(),
         local: Some(plugin.clone()),
         remote: None,
         state,
         image: None,
         description_md: plugin
            .description
            .as_ref()
            .map(|desc| widget::markdown::parse(desc).collect()),
      }
   }

   pub fn new_remote(plugin: &Plugin) -> Self {
      PluginWrap {
         identifier: plugin.identifier.clone(),
         local: None,
         remote: Some(plugin.clone()),
         state: PluginState::Available,
         image: None,
         description_md: plugin
            .description
            .as_ref()
            .map(|desc| widget::markdown::parse(desc).collect()),
      }
   }

   pub fn update_description(&mut self) {
      self.description_md = self
         .plugin()
         .description
         .as_ref()
         .map(|desc| widget::markdown::parse(desc).collect());
   }

   pub fn update_remote_if_newer(&mut self, remote: &Plugin) {
      if let Some(dest) = &self.remote {
         if dest.version <= remote.version {
            self.remote = Some(remote.clone());
            self.update_description();
         }
      } else {
         self.remote = Some(remote.clone());
         self.update_description();
      }
   }

   pub fn plugin(&self) -> &Plugin {
      if let Some(local) = &self.local
         && let Some(remote) = &self.remote
      {
         if local.version <= remote.version {
            remote
         } else {
            local
         }
      } else {
         self.plugin_prefer_local()
      }
   }

   pub fn plugin_prefer_local(&self) -> &Plugin {
      if let Some(local) = &self.local {
         local
      } else if let Some(remote) = &self.remote {
         remote
      } else {
         unreachable!();
      }
   }

   pub fn has_update(&self) -> bool {
      if let Some(local) = &self.local
         && let Some(remote) = &self.remote
         && local.version < remote.version
      {
         true
      } else {
         false
      }
   }

   pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
      let data = fs::read(path)?;
      let mut wrap: Self = toml::from_slice(&data)?;
      if let Some(local) = &mut wrap.local {
         local.check_compatible();
      }
      if let Some(remote) = &mut wrap.remote {
         remote.check_compatible();
      }
      wrap.update_description();
      Ok(wrap)
   }

   pub fn image_path_url<P: AsRef<Path>>(&self, dir: P) -> Option<(PathBuf, reqwest::Url)> {
      let plugin = self.plugin();
      if let Some(url) = &plugin.image_url
         && let Some(ext) = Path::new(url.path()).extension().and_then(|e| e.to_str())
      {
         let path = dir.as_ref().join(format!("{}.{}", plugin.identifier, ext));
         Some((path.to_path_buf(), url.clone()))
      } else {
         None
      }
   }

   pub fn missing_image<P: AsRef<Path>>(&self, dir: P) -> Option<(PathBuf, reqwest::Url)> {
      if let Some((path, url)) = self.image_path_url(dir)
         && !fs::exists(&path).ok()?
      {
         Some((path, url))
      } else {
         None
      }
   }

   pub fn load_image<P: AsRef<Path>>(&mut self, dir: P) -> Result<()> {
      if let Some((path, _)) = self.image_path_url(dir)
         && fs::exists(&path)?
      {
         self.image = Some(iced::advanced::image::Handle::from_path(&path));
      }
      Ok(())
   }
}

#[derive(Debug, Copy, Clone, Serialize, Deserialize)]
pub struct Metadata {
   pub last_updated: chrono::DateTime<chrono::Utc>,
}
impl Metadata {
   pub fn new() -> Self {
      Self {
         last_updated: chrono::DateTime::<chrono::Utc>::MIN_UTC,
      }
   }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Catalog {
   pub meta: Mutex<Metadata>,
   pub conf: Conf,
   /// Contains the reference data of all the plugins
   #[serde(skip, default)]
   pub data: Mutex<HashMap<Identifier, PluginWrap>>,
}
impl Catalog {
   pub fn new(conf: Conf) -> Self {
      Self {
         meta: Mutex::new(Metadata::new()),
         conf,
         data: Mutex::new(HashMap::new()),
      }
   }

   pub fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
      let data = fs::read(&path)?;
      Ok(toml::from_slice(&data)?)
   }

   pub fn save_to_cache(&self) -> Result<()> {
      for plugin in self.data.lock().unwrap().values() {
         let data = match toml::to_string(&plugin) {
            Ok(data) => data,
            Err(e) => {
               warn_err!(e);
               continue;
            }
         };
         let mut file = fs::File::create(
            self
               .conf
               .catalog_cache
               .join(format!("{}.toml", plugin.identifier)),
         )?;
         file.write_all(data.as_bytes())?;
      }
      // Write metadata
      let data = toml::to_string(self)?;
      let mut file = fs::File::create(&*CONFIG_FILE)?;
      file.write_all(data.as_bytes())?;
      Ok(())
   }

   pub async fn load_from_cache(&self) -> Result<()> {
      *self.data.lock().unwrap() = fs::read_dir(&self.conf.catalog_cache)?
         .filter_map(|entry| {
            let entry = match entry {
               Ok(entry) => entry,
               Err(e) => {
                  warn_err!(e);
                  return None;
               }
            };
            let wrap = match PluginWrap::from_path(entry.path().as_path()) {
               Ok(wrap) => wrap,
               Err(_) => {
                  return None;
               }
            };
            Some((wrap.identifier.clone(), wrap))
         })
         .collect();
      Ok(())
   }

   pub fn check_issues(&self) -> Result<(Vec<String>, Vec<Identifier>)> {
      let lock = self.data.lock().unwrap();
      let plugins: Vec<&Plugin> = lock
         .values()
         .filter_map(|pw| {
            if pw.state == PluginState::Installed
               && let Some(p) = &pw.local
            {
               Some(p)
            } else {
               None
            }
         })
         .sorted_by_key(|p| &p.identifier)
         .collect();
      let mut issues = Vec::new();

      // Generic test function.
      fn test<'a, F>(
         plugins: &'a [&'a Plugin],
         issues: &mut Vec<String>,
         f: F,
         msg: &'a str,
      ) -> Vec<&'a Plugin>
      where
         F: FnMut(&&Plugin) -> bool,
      {
         let plugs: Vec<_> = plugins.iter().cloned().filter(f).collect();
         if !plugs.is_empty() {
            issues.push(format!(
               "{msg}:\n * {}",
               plugs
                  .iter()
                  .map(|p| p.name.as_str())
                  .collect::<Vec<_>>()
                  .join("\n * ")
            ));
         }
         plugs
      }

      // Multiple TC conflict
      let tc: Vec<_> = plugins
         .iter()
         .cloned()
         .filter(|p| p.total_conversion)
         .collect();
      if tc.len() > 1 {
         issues.push( format!("Multiple total conversions detected! You should only use one total conversion at a time. The following are in conflict:\n * {}",
               tc.iter().map(|p|p.name.as_str()).collect::<Vec<_>>().join("\n * " )));
      }

      // TC are incompatible with other outfits unless they depend
      let tc_depends = if tc.len() == 1
         && let Some(tc) = tc.first()
      {
         test(
            &plugins,
            &mut issues,
            |p: &&Plugin| {
               p != tc
                  && !p.depends.contains(&tc.identifier)
                  && !p.recommends.contains(&tc.identifier)
            },
            pgettext(
               "plugins",
               "The following plugins may not be compatible with the current total conversion:",
            ),
         )
      } else {
         Vec::new()
      };

      // Incompatible versions conflict
      let incompat = test(
         &plugins,
         &mut issues,
         |p: &&Plugin| !p.compatible,
         pgettext(
            "plugins",
            "The following plugins are incompatible with the current Naev version:",
         ),
      );

      // Missing dependencies
      let depends = test(
         &plugins,
         &mut issues,
         |p: &&Plugin| p.depends.iter().any(|d| lock.get(d).is_none()),
         pgettext("plugins", "The following plugins are missing dependencies:"),
      );

      // Conflicts
      let conflicts = test(
         &plugins,
         &mut issues,
         |p: &&Plugin| p.conflicts.iter().any(|d| lock.get(d).is_some()),
         pgettext(
            "plugins",
            "The following plugins conflict with installed plugins:\n * {}",
         ),
      );

      // Collect them up
      let badplugs = tc
         .into_iter()
         .chain(tc_depends)
         .chain(incompat)
         .chain(depends)
         .chain(conflicts)
         .map(|p| p.identifier.clone())
         .unique()
         .collect::<Vec<_>>();
      Ok((issues, badplugs))
   }
}
