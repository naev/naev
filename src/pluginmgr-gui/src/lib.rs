use anyhow::{Error, Result};
use formatx::formatx;
use fs_err as fs;
use iced::task::{Sipper, Straw, sipper};
use iced::{Task, widget};
use log::gettext::{N_, gettext, pgettext};
use log::warn_err;
use pluginmgr::install;
use pluginmgr::install::Installer;
use pluginmgr::plugin::{Identifier, Plugin, ReleaseStatus};
use serde::{Deserialize, Serialize};
use std::borrow::Cow;
use std::collections::HashMap;
use std::io::Write;
use std::ops::Deref;
use std::path::{Path, PathBuf};
use std::sync::{Arc, LazyLock, Mutex};

/// A remote plugin repository.
#[derive(Debug, Clone, Deserialize, Serialize, PartialEq)]
struct Remote {
    url: reqwest::Url,
    mirror: Option<reqwest::Url>,
    branch: String,
}

static REMOTES_DEFAULT: LazyLock<Vec<Remote>> = LazyLock::new(|| {
    vec![Remote {
        url: reqwest::Url::parse("https://codeberg.org/naev/naev-plugins").unwrap(),
        mirror: Some(reqwest::Url::parse("https://github.com/naev/naev-plugins").unwrap()),
        branch: "main".to_string(),
    }]
});

/// Location of the plugins directory.
fn local_plugins_dir() -> PathBuf {
    pluginmgr::local_plugins_dir().unwrap()
}

/// Location of the cache directory for storing information about plugins.
fn catalog_cache_dir() -> PathBuf {
    ndata::cache_dir().join("pluginmanager")
}

static CONFIG_FILE: LazyLock<PathBuf> =
    LazyLock::new(|| ndata::pref_dir().join("pluginmanager.toml"));

/// To skip serializing if default.
fn skip_remotes(remotes: &Vec<Remote>) -> bool {
    REMOTES_DEFAULT.deref() == remotes
}
/// To set the default remotes if not found.
fn default_remotes() -> Vec<Remote> {
    REMOTES_DEFAULT.clone()
}
const REFRESH_INTERVAL_DEFAULT: chrono::TimeDelta = chrono::TimeDelta::days(1);
fn skip_refresh_interval(interval: &chrono::TimeDelta) -> bool {
    *interval == REFRESH_INTERVAL_DEFAULT
}
fn default_refresh_interval() -> chrono::TimeDelta {
    REFRESH_INTERVAL_DEFAULT
}

/// Plugin manager configuration.
#[derive(Debug, Clone, Serialize, Deserialize)]
struct Conf {
    #[serde(skip_serializing_if = "skip_remotes", default = "default_remotes")]
    remotes: Vec<Remote>,
    #[serde(
        skip_serializing_if = "skip_refresh_interval",
        default = "default_refresh_interval"
    )]
    refresh_interval: chrono::TimeDelta,
    #[serde(skip, default = "local_plugins_dir")]
    install_path: PathBuf,
    #[serde(skip, default = "catalog_cache_dir")]
    catalog_cache: PathBuf,
}
impl Conf {
    fn new() -> Result<Self> {
        Ok(Self {
            remotes: REMOTES_DEFAULT.clone(),
            install_path: pluginmgr::local_plugins_dir()?,
            catalog_cache: ndata::cache_dir().join("pluginmanager"),
            refresh_interval: default_refresh_interval(),
        })
    }
}

const THEME: iced::Theme = iced::Theme::Dark;
const SHADOW: iced::Shadow = iced::Shadow {
    color: iced::Color::BLACK,
    offset: iced::Vector { x: 0.0, y: 0.0 },
    blur_radius: 5.0,
};

/// Opens the plugin manager. Requires a different process if using OpenGL / Vulkan.
pub fn open() -> Result<()> {
    let icon = iced::window::icon::from_file_data(App::ICON, None).ok();

    // Load the fonts the same way Naev does
    let fonts: Vec<_> = gettext("Cabin-SemiBold.otf,NanumBarunGothicBold.ttf,SourceCodePro-Semibold.ttf,IBMPlexSansJP-Medium.otf")
        .split(',')
        .filter_map(|f| {
            let path = format!("fonts/{f}");
            match ndata::read(&path) {
                Ok(data) => Some(Cow::from(data)),
                Err(e) => {
                    warn_err!(e);
                    None
                }
            }
        })
        .collect();

    Ok(iced::application(App::run, App::update, App::view)
        .title(gettext("Naev Plugin Manager"))
        .window(iced::window::Settings {
            icon,
            ..Default::default()
        })
        .settings(iced::Settings {
            fonts,
            ..Default::default()
        })
        .theme(THEME)
        .centered()
        .run()?)
}

#[derive(Debug, Clone, PartialEq)]
enum DropDownAction {
    Update,
    Refresh,
    ClearCache,
}
impl std::fmt::Display for DropDownAction {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            Self::Update => pgettext("plugins", "Update All"),
            Self::Refresh => pgettext("plugins", "Force Refresh"),
            Self::ClearCache => pgettext("plugins", "Clear Cache"),
        })
    }
}

/// Application internal messages.
#[derive(Debug, Clone)]
enum Message {
    Startup,
    UpdateView(Result<(), LogEntry>),
    Selected(usize),
    Install(Plugin),
    Enable(Plugin),
    Update(Plugin),
    Disable(Plugin),
    Uninstall(Plugin),
    LinkClicked(widget::markdown::Uri),
    ProgressNew(Progress),
    Progress(install::Progress),
    LogResult(Result<(), LogEntry>),
    LogToggle,
    Action(DropDownAction),
    RefreshLocal(Result<(), LogEntry>),
    FilterChange(String),
}
impl Message {
    fn update_view(result: Result<()>) -> Self {
        Message::UpdateView(result.map_err(|e| e.into()))
    }

    fn refresh_local(result: Result<()>) -> Self {
        Message::RefreshLocal(result.map_err(|e| e.into()))
    }

    fn log_result(result: Result<()>) -> Self {
        Message::LogResult(result.map_err(|e| e.into()))
    }
}

/// Different potential plugin states.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
#[serde(rename_all = "lowercase")]
enum PluginState {
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
struct PluginWrap {
    identifier: Identifier,
    local: Option<Plugin>,
    remote: Option<Plugin>,
    state: PluginState,
    #[serde(skip, default)]
    image: Option<iced::advanced::image::Handle>,
    #[serde(skip, default)]
    description_md: Option<Vec<widget::markdown::Item>>,
}
impl PluginWrap {
    fn new_local(plugin: &Plugin, state: PluginState) -> Self {
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

    fn new_remote(plugin: &Plugin) -> Self {
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

    fn update_description(&mut self) {
        self.description_md = self
            .plugin()
            .description
            .as_ref()
            .map(|desc| widget::markdown::parse(desc).collect());
    }

    fn update_remote_if_newer(&mut self, remote: &Plugin) {
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

    fn plugin(&self) -> &Plugin {
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

    fn plugin_prefer_local(&self) -> &Plugin {
        if let Some(local) = &self.local {
            local
        } else if let Some(remote) = &self.remote {
            remote
        } else {
            unreachable!();
        }
    }

    fn has_update(&self) -> bool {
        if let Some(local) = &self.local
            && let Some(remote) = &self.remote
            && local.version < remote.version
        {
            true
        } else {
            false
        }
    }

    fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let data = fs::read(path)?;
        let mut wrap: Self = toml::from_slice(&data)?;
        if let Some(local) = &mut wrap.local {
            local.check_compatible();
        }
        if let Some(remote) = &mut wrap.remote {
            remote.check_compatible();
        }
        Ok(wrap)
    }

    fn image_path_url<P: AsRef<Path>>(&self, dir: P) -> Option<(PathBuf, reqwest::Url)> {
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

    fn missing_image<P: AsRef<Path>>(&self, dir: P) -> Option<(PathBuf, reqwest::Url)> {
        if let Some((path, url)) = self.image_path_url(dir)
            && !fs::exists(&path).ok()?
        {
            Some((path, url))
        } else {
            None
        }
    }

    fn load_image<P: AsRef<Path>>(&mut self, dir: P) -> Result<()> {
        if let Some((path, _)) = self.image_path_url(dir)
            && fs::exists(&path)?
        {
            self.image = Some(iced::advanced::image::Handle::from_path(&path));
        }
        Ok(())
    }
}

#[derive(Debug, Copy, Clone, Serialize, Deserialize)]
struct Metadata {
    last_updated: chrono::DateTime<chrono::Utc>,
}
impl Metadata {
    fn new() -> Self {
        Self {
            last_updated: chrono::DateTime::<chrono::Utc>::MIN_UTC,
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
struct Catalog {
    meta: Mutex<Metadata>,
    conf: Conf,
    /// Contains the reference data of all the plugins
    #[serde(skip, default)]
    data: Mutex<HashMap<Identifier, PluginWrap>>,
}
impl Catalog {
    fn new(conf: Conf) -> Self {
        Self {
            meta: Mutex::new(Metadata::new()),
            conf,
            data: Mutex::new(HashMap::new()),
        }
    }

    fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let data = fs::read(&path)?;
        Ok(toml::from_slice(&data)?)
    }

    fn save_to_cache(&self) -> Result<()> {
        for (_, plugin) in self.data.lock().unwrap().iter() {
            let data = match toml::to_string(&plugin) {
                Ok(data) => data,
                Err(e) => {
                    warn_err!(e);
                    continue;
                }
            };
            let mut file = fs::File::create(
                self.conf
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

    async fn load_from_cache(&self) -> Result<()> {
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
}

#[derive(Debug, Clone)]
enum LogType {
    Info,
    Error,
}

#[derive(Debug, Clone)]
struct LogEntry {
    ltype: LogType,
    message: String,
}
impl From<Error> for LogEntry {
    fn from(e: Error) -> Self {
        LogEntry {
            ltype: LogType::Error,
            message: format!("Error: {}", e),
        }
    }
}
impl LogEntry {
    fn info(message: String) -> Self {
        LogEntry {
            ltype: LogType::Info,
            message,
        }
    }
}

#[derive(Debug, Clone)]
struct Progress {
    title: String,
    message: String,
    /// [0.0, 1.0] value
    value: f32,
}

#[derive(Debug)]
struct App {
    catalog: Arc<Catalog>,
    view: Vec<PluginWrap>,
    has_update: bool,
    selected: Option<(usize, Identifier)>,
    progress: Option<Progress>,
    drop_action: bool,
    log: Vec<LogEntry>,
    log_open: bool,
    filter: String,
    // Some useful data
    default_logo: iced::advanced::image::Handle,
}

impl App {
    const ICON: &[u8] = include_bytes!("../../../extras/logos/logo64.png");

    fn run() -> (Self, Task<Message>) {
        let app = Self::new().unwrap();
        (app, Task::done(Message::Startup))
    }

    fn new() -> Result<Self> {
        let conf = Conf::new()?;
        fs::create_dir_all(&conf.install_path)?;
        fs::create_dir_all(&conf.catalog_cache)?;

        // We'll hardcode a logo into the source code for now
        use iced::advanced::image;
        let default_logo = image::Handle::from_bytes(Self::ICON);

        Ok(App {
            catalog: Arc::new(Catalog::new(conf)),
            view: Vec::new(),
            selected: None,
            has_update: false,
            progress: None,
            drop_action: false,
            log: Vec::new(),
            log_open: false,
            filter: "".to_string(),
            default_logo,
        })
    }

    fn load_from_cache_or_refresh_task(&mut self) -> Task<Message> {
        fn wrap(c: Arc<Catalog>) -> impl Straw<(), install::Progress, Error> {
            sipper(async move |sender| {
                let last_updated = c.meta.lock().unwrap().last_updated;
                let refresh = match c.load_from_cache().await {
                    Ok(()) => {
                        chrono::Local::now().signed_duration_since(last_updated)
                            >= c.conf.refresh_interval
                    }

                    Err(e) => {
                        warn_err!(e);
                        true
                    }
                };
                if refresh && let Err(e) = App::refresh_straw(c.clone()).run(&sender).await {
                    warn_err!(e);
                } else {
                    // refresh_straw will already run refresh_local_straw
                    App::refresh_local_straw(c).run(&sender).await?;
                }
                Ok(())
            })
        }
        if let Ok(metacatalog) = Catalog::from_path(&*CONFIG_FILE) {
            self.catalog = Arc::new(metacatalog);
        }
        Task::done(Message::ProgressNew(Progress {
            title: pgettext("plugins", "Starting Up").to_string(),
            message: "".to_string(),
            value: 0.0,
        }))
        .chain(Task::sip(
            wrap(self.catalog.clone()),
            Message::Progress,
            Message::update_view,
        ))
    }

    fn refresh_straw(c: Arc<Catalog>) -> impl Straw<(), install::Progress, Error> {
        sipper(async move |mut sender| {
            sender
                .send(install::Progress {
                    message: Some(
                        pgettext("plugins", "Refreshing remote repositories").to_string(),
                    ),
                    value: 0.0,
                })
                .await;
            let mut hm: HashMap<Identifier, Plugin> = HashMap::new();
            let progress = Arc::new(Mutex::new(0.0));
            let inc = 0.9 / (c.conf.remotes.len() as f32);
            for remote in &c.conf.remotes {
                let plugins = {
                    match pluginmgr::discover_remote_plugins(remote.url.clone(), &remote.branch)
                        .with(|v| {
                            let lock = progress.lock().unwrap();
                            (*lock + v.value * inc).into()
                        })
                        .run(&sender)
                        .await
                    {
                        Ok(plugins) => Ok(plugins),
                        Err(e) => {
                            if let Some(mirror) = &remote.mirror {
                                pluginmgr::discover_remote_plugins(mirror.clone(), &remote.branch)
                                    .with(|v| {
                                        let lock = progress.lock().unwrap();
                                        (*lock + v.value * inc).into()
                                    })
                                    .run(&sender)
                                    .await
                            } else {
                                Err(e)
                            }
                        }
                    }
                }?;
                for plugin in plugins {
                    hm.entry(plugin.identifier.clone())
                        .and_modify(|e| {
                            if e.version < plugin.version {
                                *e = plugin.clone()
                            }
                        })
                        .or_insert(plugin);
                }
                let val = {
                    let mut lock = progress.lock().unwrap();
                    *lock += inc;
                    *lock
                };
                sender.send(val.into()).await;
            }
            {
                let mut data = c.data.lock().unwrap();
                for (id, remote) in hm.iter() {
                    if let Some(wrap) = data.get_mut(id) {
                        wrap.update_remote_if_newer(remote);
                    } else {
                        data.insert(id.clone(), PluginWrap::new_remote(remote));
                    }
                }
                c.meta.lock().unwrap().last_updated = chrono::Local::now().into();
            }
            Self::refresh_local_straw(c)
                .with(|mut v| {
                    v.value = 0.9 + 0.1 * v.value;
                    v
                })
                .run(&sender)
                .await
        })
    }

    fn refresh_local_straw(c: Arc<Catalog>) -> impl Straw<(), install::Progress, Error> {
        sipper(async move |mut sender| {
            sender
                .send(install::Progress {
                    message: Some(
                        pgettext("plugins", "Refreshing local and remote repositories").to_string(),
                    ),
                    value: 0.0,
                })
                .await;
            let images = {
                let mut data = c.data.lock().unwrap();
                for (_, wrap) in data.iter_mut() {
                    wrap.local = None;
                    wrap.state = PluginState::Available;
                }
                for plugin in pluginmgr::discover_local_plugins(&c.conf.install_path)? {
                    let state = match plugin.disabled {
                        true => PluginState::Disabled,
                        false => PluginState::Installed,
                    };
                    if let Some(wrap) = data.get_mut(&plugin.identifier) {
                        wrap.local = Some(plugin.clone());
                        wrap.state = state;
                    } else {
                        data.insert(
                            plugin.identifier.clone(),
                            PluginWrap::new_local(&plugin, state),
                        );
                    }
                }
                data.retain(|_, wrap| wrap.local.is_some() || wrap.remote.is_some());

                let images: Vec<(PathBuf, reqwest::Url)> = data
                    .iter()
                    .filter_map(|(_, wrap)| wrap.missing_image(&c.conf.catalog_cache))
                    .collect();
                images
            };

            async fn download_image<P: AsRef<Path>, T: reqwest::IntoUrl>(
                path: P,
                url: T,
            ) -> Result<()> {
                let response = reqwest::get(url).await?;
                let content = response.bytes().await?;
                let mut file = fs::File::create(path.as_ref())?;
                file.write_all(&content)?;
                Ok(())
            }
            use futures::StreamExt;
            futures::stream::iter(images)
                .for_each(async |(path, url)| {
                    if let Err(e) = download_image(path, url).await {
                        warn_err!(e);
                    }
                })
                .await;

            for (_, wrap) in c.data.lock().unwrap().iter_mut() {
                if let Err(e) = wrap.load_image(&c.conf.catalog_cache) {
                    warn_err!(e);
                }
            }

            c.save_to_cache()
        })
    }

    fn start_task(title: &str) -> Task<Message> {
        Task::done(Message::ProgressNew(Progress {
            title: title.to_string(),
            message: "".to_string(),
            value: 0.0,
        }))
    }

    fn refresh_task(&self) -> Task<Message> {
        Self::start_task(pgettext("plugins", "Refreshing")).chain(Task::sip(
            Self::refresh_straw(self.catalog.clone()),
            Message::Progress,
            Message::update_view,
        ))
    }

    fn refresh_local_task(&self) -> Task<Message> {
        Self::start_task(pgettext("plugins", "Refreshing")).chain(Task::sip(
            Self::refresh_local_straw(self.catalog.clone()),
            Message::Progress,
            Message::update_view,
        ))
    }

    fn install_task(&self, plugin: &Plugin) -> Task<Message> {
        Self::start_task(pgettext("plugins", "Installing")).chain(Task::sip(
            Installer::new(&self.catalog.conf.install_path, plugin).install(),
            Message::Progress,
            Message::refresh_local,
        ))
    }

    fn update_task(&self, plugin: &Plugin) -> Task<Message> {
        Self::start_task(pgettext("plugins", "Updating")).chain(Task::sip(
            Installer::new(&self.catalog.conf.install_path, plugin).update(),
            Message::Progress,
            Message::refresh_local,
        ))
    }

    fn uninstall_task(&self, plugin: &Plugin, path: &PathBuf) -> Task<Message> {
        Self::start_task(pgettext("plugins", "Removing")).chain(Task::sip(
            Installer::new(path, plugin).uninstall(),
            Message::Progress,
            Message::refresh_local,
        ))
    }

    /// Updates the view if applicable
    fn update_view(&mut self) -> Task<Message> {
        self.view = self
            .catalog
            .data
            .lock()
            .unwrap()
            .values()
            .filter(|p| {
                if self.filter.is_empty() {
                    return true;
                }
                let plg = p.plugin();
                let filter = self.filter.to_lowercase();
                plg.name.to_lowercase().contains(&filter)
                    || plg.r#abstract.to_lowercase().contains(&filter)
                    || plg.tags.iter().any(|t| t.to_lowercase().contains(&filter))
                    || plg.release_status.as_str().contains(&filter)
            })
            .cloned()
            .collect();
        self.view.sort_by(|a, b| {
            let ord = a.state.cmp(&b.state);
            if ord == std::cmp::Ordering::Equal {
                a.plugin().identifier.cmp(&b.plugin().identifier)
            } else {
                ord
            }
        });
        self.has_update = self.view.iter().any(|wrap| wrap.has_update());

        fn recover_selected(
            view: &[PluginWrap],
            identifier: &Identifier,
        ) -> Option<(usize, Identifier)> {
            for (id, wrap) in view.iter().enumerate() {
                if wrap.identifier == *identifier {
                    return Some((id, identifier.clone()));
                }
            }
            None
        }
        // Try to recover selection if it is not matched anymore
        if let Some((id, identifier)) = &self.selected {
            if let Some(sel) = self.view.get(*id) {
                if sel.identifier != *identifier {
                    self.selected = recover_selected(&self.view, identifier);
                }
            } else {
                self.selected = recover_selected(&self.view, identifier);
            }
        }
        Task::none()
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        match message {
            Message::Startup => self.load_from_cache_or_refresh_task(),
            Message::UpdateView(value) => {
                // If a previous task errored, we log it
                if let Err(e) = value {
                    self.log.push(e);
                    self.log_open = true;
                    self.progress = None;
                    return Task::none();
                }
                self.progress = None;
                self.update_view()
            }
            Message::Selected(id) => {
                if let Some((rid, _)) = &self.selected {
                    self.selected = if id == *rid {
                        None
                    } else {
                        Some((id, self.view[id].identifier.clone()))
                    }
                } else {
                    self.selected = Some((id, self.view[id].identifier.clone()))
                };
                Task::none()
            }
            Message::Install(plugin) => self.install_task(&plugin),
            Message::Enable(plugin) => {
                let _ = plugin.disable(false);
                self.refresh_local_task()
            }
            Message::Update(plugin) => self.update_task(&plugin),
            Message::Disable(plugin) => {
                let _ = plugin.disable(true);
                self.refresh_local_task()
            }
            Message::Uninstall(plugin) => {
                self.uninstall_task(&plugin, &self.catalog.conf.install_path)
            }
            Message::LinkClicked(url) => {
                if let Err(e) = webbrowser::open(url.as_str()) {
                    warn_err!(e);
                }
                Task::none()
            }
            Message::ProgressNew(progress) => {
                self.progress = Some(progress);
                if let Some(progress) = &self.progress {
                    self.log.push(LogEntry::info(progress.title.clone()));
                    if !progress.message.is_empty() {
                        self.log.push(LogEntry::info(progress.message.clone()));
                    }
                }
                Task::none()
            }
            Message::Progress(value) => {
                if let Some(progress) = &mut self.progress {
                    if let Some(message) = &value.message {
                        progress.message = message.clone();
                        self.log.push(LogEntry::info(message.clone()));
                    }
                    progress.value = value.value;
                }
                Task::none()
            }
            Message::LogResult(result) => {
                if let Err(entry) = result {
                    self.log.push(entry);
                }
                Task::none()
            }
            Message::LogToggle => {
                self.log_open = !self.log_open;
                Task::none()
            }
            /*
            Message::DropDownToggle => {
                self.drop_action = !self.drop_action;
                Task::none()
            }
            */
            Message::RefreshLocal(value) => match value {
                Ok(()) => self.refresh_local_task(),
                Err(e) => {
                    self.log.push(e);
                    Task::none()
                }
            },
            Message::FilterChange(value) => {
                self.filter = value;
                self.update_view()
            }
            Message::Action(DropDownAction::ClearCache) => {
                self.drop_action = false;
                if let Err(e) = fs::remove_dir_all(&self.catalog.conf.catalog_cache) {
                    warn_err!(e);
                }
                if let Err(e) = fs::create_dir_all(&self.catalog.conf.catalog_cache) {
                    warn_err!(e);
                }
                self.refresh_task()
            }
            Message::Action(DropDownAction::Refresh) => {
                self.drop_action = false;
                self.refresh_task()
            }
            Message::Action(DropDownAction::Update) => {
                self.drop_action = false;
                Self::start_task(pgettext("plugins", "Updating"))
                    .chain(Task::batch(self.view.iter().filter_map(|plugin| {
                        if plugin.has_update()
                            && let Some(local) = &plugin.local
                        {
                            Some(Task::sip(
                                Installer::new(&self.catalog.conf.install_path, local).update(),
                                Message::Progress,
                                Message::log_result,
                            ))
                        } else {
                            None
                        }
                    })))
                    .chain(Task::done(Message::RefreshLocal(Ok(()))))
            }
        }
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use iced::Length::{Fill, FillPortion, Shrink};
        use iced::alignment::{Horizontal, Vertical};
        use iced::theme::palette::Pair;
        use widget::{
            column, container, grid, image, mouse_area, row, scrollable, space, text, text_input,
            tooltip,
        };

        let idle = self.progress.is_none();
        let palette = THEME.palette();
        let extended = THEME.extended_palette();

        let bold = |txt| {
            text(txt).font(iced::Font {
                weight: iced::font::Weight::Bold,
                ..Default::default()
            })
        };
        let button = |txt| widget::button(text(txt));
        let badge = |txt, col: Pair| {
            container(text(txt))
                .style(move |theme| {
                    container::rounded_box(theme)
                        .color(col.text)
                        .background(col.color)
                })
                .padding(3.0)
        };

        let plugins = if self.view.is_empty() {
            grid([
                container(text(pgettext("plugins", "No plugins found!")).center())
                    .padding(20)
                    .into(),
            ])
        } else {
            grid(self.view.iter().enumerate().map(|(id, v)| {
                let p = v.plugin();
                let image = image(match &v.image {
                    Some(img) => img.clone(),
                    None => self.default_logo.clone(),
                })
                .width(60)
                .height(60);
                let name = bold(p.name.as_str());
                let badge = match v.state {
                    PluginState::Installed => Some(if v.has_update() {
                        badge(pgettext("plugins", "update"), extended.warning.weak)
                    } else {
                        badge(pgettext("plugins", "installed"), extended.success.weak)
                    }),
                    PluginState::Disabled => Some(badge(
                        pgettext("plugins", "disabled"),
                        extended.background.base,
                    )),
                    PluginState::Available => None,
                };
                // Somewhat like a modal
                let content = column![
                    match badge {
                        Some(badge) => row![name, badge,],
                        None => row![name],
                    }
                    .align_y(Vertical::Center)
                    .spacing(5),
                    text(p.r#abstract.as_str()),
                    text(p.tags.join(", ")),
                ]
                .spacing(5);
                let modal = row![image, content,].spacing(5).align_y(Vertical::Center);
                mouse_area(container(modal).padding(10).style(move |theme| {
                    let extended = theme.extended_palette();
                    let border = if let Some(sel) = &self.selected
                        && id == sel.0
                    {
                        iced::Border {
                            color: palette.primary,
                            width: 3.0,
                            radius: iced::border::Radius::new(2.0),
                        }
                    } else {
                        iced::border::rounded(2)
                    };
                    widget::container::Style {
                        background: Some(extended.background.weakest.color.into()),
                        text_color: Some(extended.background.weakest.text),
                        border,
                        shadow: SHADOW,
                        ..Default::default()
                    }
                }))
                .on_press(Message::Selected(id))
                .into()
            }))
            .fluid(500)
            .spacing(10)
            .height(grid::Sizing::EvenlyDistribute(Shrink))
        };
        let plugins = column![
            text_input(gettext("Filter..."), &self.filter).on_input(Message::FilterChange),
            scrollable(plugins).spacing(10)
        ]
        .spacing(5);

        let tooltip_container = |txt| {
            container(text(txt))
                .padding(10)
                /*
                .style(|theme: &iced::Theme| {
                    let extended = theme.extended_palette();
                    widget::container::Style {
                        background: Some(extended.background.weakest.color.into()),
                        border: iced::border::rounded(2),
                        shadow: SHADOW,
                        ..Default::default()
                    }
                })
                */
                .style(widget::container::dark)
                .max_width(300)
        };
        let (selected, buttons) = if let Some((id, _)) = &self.selected
            && let Some(wrp) = self.view.get(*id)
        {
            let sel = wrp.plugin_prefer_local();
            let info = |txt| text(txt).size(20);
            let tooltip_pos = tooltip::Position::FollowCursor;
            let col = column![
                //bold(pgettext("plugins", "Identifier:")),
                //info(sel.identifier.as_str()),
                bold(pgettext("plugins", "Name:")),
                tooltip(
                    info(sel.name.as_str()),
                    tooltip_container(format!("Identifier: {}", sel.identifier.as_str())),
                    tooltip_pos,
                ),
                bold(pgettext("plugins", "State:")),
                info(gettext(wrp.state.as_str())),
                bold(pgettext("plugins", "Author(s):")),
                info(&sel.author),
                bold(pgettext("plugins", "Plugin Version:")),
                if let Some(local) = &wrp.local
                    && let Some(remote) = &wrp.remote
                    && local.version < remote.version
                {
                    text(
                        formatx!(
                            pgettext("plugins", "{} [{} available]"),
                            &local.version,
                            &remote.version
                        )
                        .unwrap_or(local.version.to_string()),
                    )
                    .size(20)
                    .color(palette.warning)
                } else {
                    text(sel.version.to_string()).size(20)
                },
                bold(pgettext("plugins", "Naev Version:")),
                text(format!(
                    "{}{}",
                    sel.naev_version,
                    match sel.compatible {
                        true => "".to_string(),
                        false => formatx!(
                            pgettext("plugins", " [incompatible with Naev {}]"),
                            &*log::version::VERSION
                        )
                        .unwrap_or(format!(
                            " [incompatible with Naev {}]",
                            *log::version::VERSION
                        )),
                    }
                ))
                .color_maybe(match sel.compatible {
                    true => None,
                    false => Some(palette.danger),
                })
                .size(20),
                bold(pgettext("plugins", "Status:")),
                tooltip(
                    info(gettext(sel.release_status.as_str())).color_maybe(
                        match sel.release_status {
                            ReleaseStatus::Stable => None,
                            _ => Some(palette.warning),
                        }
                    ),
                    tooltip_container(format!(
                        "{}: {}",
                        gettext(sel.release_status.as_str()),
                        gettext(sel.release_status.description())
                    )),
                    tooltip_pos,
                ),
                widget::space::vertical().height(iced::Length::Fixed(5.0)),
                if let Some(md) = &wrp.description_md {
                    widget::markdown::view(md, THEME).map(Message::LinkClicked)
                } else {
                    text(&sel.r#abstract).into()
                }
            ]
            .spacing(5);
            (
                col,
                match wrp.state {
                    PluginState::Installed => {
                        let disable = button(pgettext("plugins", "Disable"))
                            .on_press_maybe(idle.then_some(Message::Disable(sel.clone())));
                        let uninstall = button(pgettext("plugins", "Uninstall"))
                            .on_press_maybe(idle.then_some(Message::Uninstall(sel.clone())));
                        if wrp.has_update() {
                            row![
                                button(pgettext("plugins", "Update"))
                                    .on_press_maybe(idle.then_some(Message::Update(sel.clone()))),
                                disable,
                                uninstall
                            ]
                        } else {
                            row![disable, uninstall]
                        }
                    }
                    PluginState::Disabled => {
                        row![
                            button(pgettext("plugins", "Enable"))
                                .on_press_maybe(idle.then_some(Message::Enable(sel.clone()))),
                            button(pgettext("plugins", "Uninstall"))
                                .on_press_maybe(idle.then_some(Message::Uninstall(sel.clone()))),
                        ]
                    }
                    PluginState::Available => {
                        row![
                            button(pgettext("plugins", "Install"))
                                .on_press_maybe(idle.then_some(Message::Install(sel.clone()))),
                        ]
                    }
                },
            )
        } else {
            (
                column![text("")],
                row![button(pgettext("plugins", "Install"))],
            )
        };
        let buttons = container(
            buttons
                .push(
                    widget::pick_list(
                        [
                            DropDownAction::ClearCache,
                            DropDownAction::Refresh,
                            DropDownAction::Update,
                        ],
                        None::<DropDownAction>,
                        Message::Action,
                    )
                    .placeholder("Action")
                    .style(move |_, _| widget::pick_list::Style {
                        text_color: palette.text,
                        placeholder_color: palette.text,
                        handle_color: palette.text,
                        background: iced::Background::Color(palette.primary),
                        border: Default::default(),
                    }),
                )
                .spacing(10)
                .wrap(),
        )
        .align_right(Fill);
        // Set up the final screen
        let right = column![buttons, selected].spacing(10).width(300);
        let mut main = widget::stack![row![plugins, right].spacing(20).padding(20).height(Fill)];
        if self.log_open {
            let logview = scrollable(widget::Column::with_children(self.log.iter().map(|l| {
                text(&l.message)
                    .color_maybe(match l.ltype {
                        LogType::Info => None,
                        LogType::Error => Some(palette.danger),
                    })
                    .into()
            })))
            .spacing(10)
            .height(Fill)
            .width(Fill)
            .anchor_bottom();
            let over = container(column![
                space().height(FillPortion(2)),
                row![
                    space().width(FillPortion(1)),
                    container(logview)
                        .style(container::rounded_box)
                        .padding(10)
                        .style(widget::container::dark)
                        .width(FillPortion(8)),
                    space().width(FillPortion(1)),
                ]
                .height(FillPortion(1)),
            ])
            .center_x(Fill)
            .align_y(Vertical::Bottom)
            .width(Fill)
            .height(Fill)
            .padding(20);
            main = main.push(over);
        }
        main = main.push(
            container(button(pgettext("plugins", "Logs")).on_press(Message::LogToggle))
                .align_x(Horizontal::Left)
                .align_y(Vertical::Bottom)
                .width(Fill)
                .height(Fill)
                .padding(10),
        );
        if let Some(progress) = &self.progress {
            let over = container(
                container(column![
                    bold(&progress.title),
                    text(&progress.message),
                    widget::progress_bar(0.0..=1.0, progress.value).girth(15.0),
                ])
                .style(|theme| {
                    let extended = theme.extended_palette();
                    widget::container::Style {
                        background: Some(extended.background.weak.color.into()),
                        text_color: Some(extended.background.weak.text),
                        border: iced::border::rounded(2),
                        shadow: SHADOW,
                        ..Default::default()
                    }
                })
                .padding(10)
                .align_y(Vertical::Center)
                .width(400),
            )
            .align_x(Horizontal::Right)
            .align_y(Vertical::Bottom)
            .width(Fill)
            .height(Fill)
            .padding(10);
            main = main.push(over);
        }
        main.into()
    }
}
