use anyhow::Result;
use fs_err as fs;
use iced::{Task, widget};
use log::gettext::{N_, gettext, pgettext};
use log::warn_err;
use pluginmgr::install::Installer;
use pluginmgr::plugin::{Identifier, Plugin};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::io::Write;
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

#[derive(Debug, Clone, Deserialize, Serialize)]
struct Remote {
    url: reqwest::Url,
    mirror: Option<reqwest::Url>,
    branch: String,
}

fn local_plugins_dir() -> PathBuf {
    pluginmgr::local_plugins_dir().unwrap()
}
fn local_plugins_disabled_dir() -> PathBuf {
    pluginmgr::local_plugins_disabled_dir().unwrap()
}
fn catalog_cache_dir() -> PathBuf {
    pluginmgr::cache_dir().unwrap().join("pluginmanager")
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct Conf {
    remotes: Vec<Remote>,
    refresh_interval: chrono::TimeDelta,
    #[serde(skip, default = "local_plugins_dir")]
    install_path: PathBuf,
    #[serde(skip, default = "local_plugins_disabled_dir")]
    disable_path: PathBuf,
    #[serde(skip, default = "catalog_cache_dir")]
    catalog_cache: PathBuf,
}
impl Conf {
    fn new() -> Result<Self> {
        Ok(Self {
            remotes: vec![Remote {
                url: reqwest::Url::parse("https://codeberg.org/naev/naev-plugins")?,
                mirror: None,
                branch: "main".to_string(),
            }],
            install_path: pluginmgr::local_plugins_dir()?,
            disable_path: pluginmgr::local_plugins_disabled_dir()?,
            catalog_cache: pluginmgr::cache_dir()?.join("pluginmanager"),
            refresh_interval: chrono::TimeDelta::days(1),
        })
    }
}

const THEME: iced::Theme = iced::Theme::Dark;

pub fn open() -> Result<()> {
    Ok(iced::application(App::run, App::update, App::view)
        .title(gettext("Naev Plugin Manager"))
        .theme(THEME)
        .centered()
        .run()?)
}

#[derive(Debug, Clone)]
enum Message {
    Startup,
    UpdateView,
    Selected(usize),
    Install(Plugin),
    Enable(Plugin),
    Update(Plugin),
    Disable(Plugin),
    Uninstall(Plugin),
    UninstallDisabled(Plugin),
    Idle,
    DropDownToggle,
    RefreshLocal,
    ActionRefresh,
    ActionUpdate,
}

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

#[derive(Debug, Clone, Deserialize, Serialize)]
struct PluginWrap {
    identifier: Identifier,
    local: Option<Plugin>,
    remote: Option<Plugin>,
    state: PluginState,
    #[serde(skip, default)]
    image: Option<iced::advanced::image::Handle>,
}
impl PluginWrap {
    fn new_local(plugin: &Plugin, state: PluginState) -> Self {
        PluginWrap {
            identifier: plugin.identifier.clone(),
            local: Some(plugin.clone()),
            remote: None,
            state,
            image: None,
        }
    }

    fn new_remote(plugin: &Plugin) -> Self {
        PluginWrap {
            identifier: plugin.identifier.clone(),
            local: Some(plugin.clone()),
            remote: None,
            state: PluginState::Available,
            image: None,
        }
    }

    fn update_remote_if_newer(&mut self, remote: &Plugin) {
        if let Some(dest) = &self.remote {
            if dest.version <= remote.version {
                self.remote = Some(remote.clone());
            }
        } else {
            self.remote = Some(remote.clone());
        }
    }

    fn plugin(&self) -> &Plugin {
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

    fn load_image<P: AsRef<Path>>(&mut self, dir: P) -> Result<()> {
        let plugin = self.plugin();
        if let Some(url) = &plugin.image_url
            && let Some(urlpath) = url.to_file_path().ok()
            && let Some(ext) = urlpath.extension().and_then(|e| e.to_str())
        {
            let path = dir.as_ref().join(format!("{}.{}", plugin.identifier, ext));
            if fs::exists(&path)? {
                self.image = Some(iced::advanced::image::Handle::from_path(&path));
            } else {
                todo!();
            }
        }
        Ok(())
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
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

    async fn refresh(&self) -> Result<()> {
        let mut hm: HashMap<Identifier, Plugin> = HashMap::new();
        for remote in &self.conf.remotes {
            let plugins = {
                match pluginmgr::discover_remote_plugins(remote.url.clone(), &remote.branch).await {
                    Ok(plugins) => Ok(plugins),
                    Err(e) => {
                        if let Some(mirror) = &remote.mirror {
                            pluginmgr::discover_remote_plugins(mirror.clone(), &remote.branch).await
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
        }
        {
            let mut data = self.data.lock().unwrap();
            for (id, remote) in hm.iter() {
                if let Some(wrap) = data.get_mut(id) {
                    wrap.update_remote_if_newer(remote);
                } else {
                    data.insert(id.clone(), PluginWrap::new_remote(remote));
                }
            }
            self.meta.lock().unwrap().last_updated = chrono::Local::now().into();
        }
        self.refresh_local().await
    }

    async fn refresh_local(&self) -> Result<()> {
        let mut data = self.data.lock().unwrap();
        for (_, wrap) in data.iter_mut() {
            wrap.local = None;
            wrap.state = PluginState::Available;
        }
        for plugin in pluginmgr::discover_local_plugins(&self.conf.disable_path)? {
            if let Some(wrap) = data.get_mut(&plugin.identifier) {
                wrap.local = Some(plugin.clone());
                wrap.state = PluginState::Disabled;
            } else {
                data.insert(
                    plugin.identifier.clone(),
                    PluginWrap::new_local(&plugin, PluginState::Disabled),
                );
            }
        }
        for plugin in pluginmgr::discover_local_plugins(&self.conf.install_path)? {
            if let Some(wrap) = data.get_mut(&plugin.identifier) {
                wrap.local = Some(plugin.clone());
                wrap.state = PluginState::Installed;
            } else {
                data.insert(
                    plugin.identifier.clone(),
                    PluginWrap::new_local(&plugin, PluginState::Installed),
                );
            }
        }
        data.retain(|_, wrap| wrap.local.is_some() || wrap.remote.is_some());
        for (_, wrap) in data.iter_mut() {
            if let Err(e) = wrap.load_image(&self.conf.catalog_cache) {
                warn_err!(e);
            }
        }
        drop(data);

        self.save_to_cache()
    }

    fn save_to_cache(&self) -> Result<()> {
        fs::create_dir_all(&self.conf.catalog_cache)?;
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
        let mut file = fs::File::create(self.conf.catalog_cache.join("metadata.toml"))?;
        file.write_all(data.as_bytes())?;
        Ok(())
    }

    async fn load_from_cache(&self) -> Result<()> {
        {
            let mut data = self.data.lock().unwrap();
            *data = fs::read_dir(&self.conf.catalog_cache)?
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
        }
        self.refresh_local().await?;
        Ok(())
    }
}

#[derive(Debug)]
struct App {
    catalog: Arc<Catalog>,
    view: Vec<PluginWrap>,
    has_update: bool,
    selected: Option<(usize, Identifier)>,
    idle: bool,
    drop_action: bool,
    // Some useful data
    default_logo: iced::advanced::image::Handle,
}

impl App {
    fn run() -> (Self, Task<Message>) {
        let app = Self::new().unwrap();
        (app, Task::done(Message::Startup))
    }

    fn new() -> Result<Self> {
        let conf = Conf::new()?;
        fs::create_dir_all(&conf.install_path)?;
        fs::create_dir_all(&conf.disable_path)?;

        // We'll hardcode a logo into the source code for now
        use iced::advanced::image;
        let default_logo = image::Handle::from_bytes(image::Bytes::from_static(include_bytes!(
            "../../../extras/logos/logo64.png"
        )));

        Ok(App {
            catalog: Arc::new(Catalog::new(conf)),
            view: Vec::new(),
            selected: None,
            has_update: false,
            idle: true,
            drop_action: false,
            default_logo,
        })
    }

    fn load_from_cache_or_refresh_task(&mut self) -> Task<Message> {
        async fn wrap(c: Arc<Catalog>) {
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
            if refresh && let Err(e) = c.refresh().await {
                warn_err!(e);
            }
        }
        if let Ok(metacatalog) =
            Catalog::from_path(self.catalog.conf.catalog_cache.join("metadata.toml"))
        {
            self.catalog = Arc::new(metacatalog);
        }
        self.idle = false;
        Task::perform(wrap(self.catalog.clone()), |_| Message::UpdateView)
    }

    fn refresh_task(&mut self) -> Task<Message> {
        async fn wrap(c: Arc<Catalog>) {
            if let Err(e) = c.refresh().await {
                warn_err!(e);
            }
        }
        self.idle = false;
        Task::perform(wrap(self.catalog.clone()), |_| Message::UpdateView)
    }

    fn refresh_local_task(&mut self) -> Task<Message> {
        async fn wrap(c: Arc<Catalog>) {
            if let Err(e) = c.refresh_local().await {
                warn_err!(e);
            }
        }
        self.idle = false;
        Task::perform(wrap(self.catalog.clone()), |_| Message::UpdateView)
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        let task = match message {
            Message::Startup => {
                self.idle = false;
                self.load_from_cache_or_refresh_task()
            }
            Message::UpdateView => {
                self.view = self
                    .catalog
                    .data
                    .lock()
                    .unwrap()
                    .values()
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
                self.idle = true;
                Task::none()
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
            Message::Install(plugin) => {
                self.idle = false;
                async fn install_wrapper(installer: Installer) {
                    match installer.install().await {
                        Ok(_) => (),
                        Err(e) => warn_err!(e),
                    }
                }
                Task::perform(
                    install_wrapper(Installer::new(&self.catalog.conf.install_path, &plugin)),
                    |_| Message::RefreshLocal,
                )
                .chain(Task::done(Message::Idle))
            }
            Message::Enable(plugin) => {
                match Installer::new(&self.catalog.conf.disable_path, &plugin)
                    .move_to(&self.catalog.conf.install_path)
                {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                self.refresh_local_task()
            }
            Message::Update(plugin) => {
                async fn update_wrapper(installer: Installer) {
                    match installer.update().await {
                        Ok(_) => (),
                        Err(e) => warn_err!(e),
                    }
                }
                Task::perform(
                    update_wrapper(Installer::new(&self.catalog.conf.install_path, &plugin)),
                    |_| Message::RefreshLocal,
                )
                .chain(Task::done(Message::Idle))
            }
            Message::Disable(plugin) => {
                match Installer::new(&self.catalog.conf.install_path, &plugin)
                    .move_to(&self.catalog.conf.disable_path)
                {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                self.refresh_local_task()
            }
            Message::Uninstall(plugin) => {
                match Installer::new(&self.catalog.conf.install_path, &plugin).uninstall() {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                self.refresh_local_task()
            }
            Message::UninstallDisabled(plugin) => {
                match Installer::new(&self.catalog.conf.disable_path, &plugin).uninstall() {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                self.refresh_local_task()
            }
            Message::Idle => {
                self.idle = true;
                Task::none()
            }
            Message::DropDownToggle => {
                self.drop_action = !self.drop_action;
                Task::none()
            }
            Message::RefreshLocal => self.refresh_local_task(),
            Message::ActionRefresh => {
                self.drop_action = false;
                self.refresh_task()
            }
            Message::ActionUpdate => {
                self.drop_action = false;
                self.idle = false;
                async fn update_wrapper(installer: Installer) {
                    match installer.update().await {
                        Ok(_) => (),
                        Err(e) => warn_err!(e),
                    }
                }
                Task::batch(self.view.iter().filter_map(|plugin| {
                    if plugin.has_update()
                        && let Some(local) = &plugin.local
                    {
                        Some(
                            Task::perform(
                                update_wrapper(Installer::new(
                                    &self.catalog.conf.install_path,
                                    local,
                                )),
                                |_| Message::RefreshLocal,
                            )
                            .discard(),
                        )
                    } else {
                        None
                    }
                }))
                .chain(Task::done(Message::RefreshLocal))
                .chain(Task::done(Message::Idle))
            }
        };
        // Clear selection if it's not matched anymore
        if let Some((id, identifier)) = &self.selected {
            if let Some(sel) = self.view.get(*id) {
                if sel.identifier != *identifier {
                    self.selected = None; // TODO try to recover selection
                }
            } else {
                self.selected = None;
            }
        }
        task
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use iced::theme::palette::Pair;
        use widget::{column, container, grid, image, mouse_area, row, scrollable, text};

        let idle = self.idle;
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

        let plugins = {
            scrollable(
                grid(self.view.iter().enumerate().map(|(id, v)| {
                    let p = v.plugin();
                    let image = image(&self.default_logo).width(60).height(60);
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
                        .spacing(5),
                        text(p.r#abstract.as_str()),
                        text(p.tags.join(", ")),
                    ]
                    .spacing(5);
                    let modal = row![image, content,]
                        .spacing(5)
                        .align_y(iced::alignment::Vertical::Center);
                    mouse_area(container(modal).padding(10).style(move |theme| {
                        let container = container::rounded_box(theme)
                            .background(iced::Background::Color(extended.background.weakest.color));
                        if let Some(sel) = &self.selected
                            && id == sel.0
                        {
                            container.border(iced::Border {
                                color: palette.primary,
                                width: 3.0,
                                radius: iced::border::Radius::new(2.0),
                            })
                        } else {
                            container
                        }
                    }))
                    .on_press(Message::Selected(id))
                    .into()
                }))
                .fluid(500)
                .spacing(10)
                .height(grid::Sizing::EvenlyDistribute(iced::Length::Shrink)),
            )
            .spacing(10)
        };

        let (selected, buttons) = if let Some((id, _)) = &self.selected
            && let Some(wrp) = self.view.get(*id)
        {
            let sel = wrp.plugin();
            let info = |txt| text(txt).size(20);
            let col = column![
                bold(pgettext("plugins", "Identifier:")),
                info(sel.identifier.as_str()),
                bold(pgettext("plugins", "Name:")),
                info(sel.name.as_str()),
                bold(pgettext("plugins", "State:")),
                info(gettext(wrp.state.as_str())),
                bold(pgettext("plugins", "Author(s):")),
                info(&sel.author),
                bold(pgettext("plugins", "Plugin Version:")),
                text(sel.version.to_string()).size(20),
                bold(pgettext("plugins", "Naev Version:")),
                text(format!(
                    "{}{}",
                    sel.naev_version,
                    match sel.compatible {
                        true => "".to_string(),
                        false => format!(
                            " [{} {}]",
                            gettext("incompatible with Naev "),
                            *log::version::VERSION
                        ),
                    }
                ))
                .color_maybe(match sel.compatible {
                    true => None,
                    false => Some(palette.danger),
                })
                .size(20),
                bold(pgettext("plugins", "Status:")),
                info(gettext(sel.release_status.as_str())),
                bold(pgettext("plugins", "Description")),
                text(sel.description.as_ref().unwrap_or(&sel.r#abstract)),
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
                            button(pgettext("plugins", "Uninstall")).on_press_maybe(
                                idle.then_some(Message::UninstallDisabled(sel.clone()))
                            ),
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
        // Add refresh button and format
        let actions = column![
            button(pgettext("plugins", "Update All"))
                .on_press_maybe((!self.has_update && self.idle).then_some(Message::ActionUpdate)),
            button(pgettext("plugins", "Force Refresh"))
                .on_press_maybe(self.idle.then_some(Message::ActionRefresh)),
        ]
        .spacing(5)
        .align_x(iced::Alignment::End);
        let buttons = container(
            buttons
                .push(
                    iced_aw::widget::DropDown::new(
                        button(pgettext("plugins", "Action")).on_press(Message::DropDownToggle),
                        actions,
                        self.drop_action,
                    )
                    .width(iced::Length::Fill)
                    .on_dismiss(Message::DropDownToggle)
                    .alignment(iced_aw::drop_down::Alignment::Bottom),
                )
                .padding(10)
                .spacing(10)
                .wrap(),
        )
        .align_right(iced::Length::Fill);
        // Set up the final screen
        let right = column![buttons, selected].spacing(10).width(300);
        row![plugins, right].spacing(20).padding(20).into()
    }
}
