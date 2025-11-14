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
    Selected(usize),
    Install(Plugin),
    Enable(Plugin),
    Update(Plugin),
    Disable(Plugin),
    Uninstall(Plugin),
    UninstallDisabled(Plugin),
    Idle,
    RefreshDone,
    RefreshLocal,
    UpdateCatalog(Catalog),
    DropDownToggle,
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

#[derive(Debug, Clone, Serialize, Deserialize)]
struct Catalog {
    meta: Metadata,
    conf: Conf,
    #[serde(skip, default)]
    remote: Vec<Plugin>,
    #[serde(skip, default)]
    local: Vec<Plugin>,
    #[serde(skip, default)]
    disabled: Vec<Plugin>,
    #[serde(skip, default)]
    all: Vec<PluginWrap>,
    #[serde(skip, default)]
    needs_update: Vec<Plugin>,
}
impl Catalog {
    fn new(conf: Conf) -> Self {
        Self {
            meta: Metadata::new(),
            conf,
            remote: Vec::new(),
            local: Vec::new(),
            disabled: Vec::new(),
            all: Vec::new(),
            needs_update: Vec::new(),
        }
    }

    fn from_path<P: AsRef<Path>>(path: P) -> Result<Self> {
        let data = fs::read(&path)?;
        Ok(toml::from_slice(&data)?)
    }

    async fn refresh_async(&mut self) -> Result<()> {
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
        self.remote = hm.into_values().collect();
        self.meta.last_updated = chrono::Local::now().into(); // saved in refresh_local()
        self.refresh_local()
    }

    fn refresh_local(&mut self) -> Result<()> {
        self.local = pluginmgr::discover_local_plugins(&self.conf.install_path)?;
        self.disabled = pluginmgr::discover_local_plugins(&self.conf.disable_path)?;

        let mut all: HashMap<Identifier, PluginWrap> = self
            .local
            .iter()
            .map(|plugin| {
                (
                    plugin.identifier.clone(),
                    PluginWrap::new_local(&plugin, PluginState::Installed),
                )
            })
            .collect();
        for plugin in self.disabled.iter() {
            match all.get(&plugin.identifier) {
                Some(_) => {
                    anyhow::bail!(format!(
                        "plugin '{}' is both installed and disabled",
                        &plugin.name
                    ));
                }
                None => {
                    all.insert(
                        plugin.identifier.clone(),
                        PluginWrap::new_local(&plugin, PluginState::Disabled),
                    );
                }
            }
        }
        for plugin in self.remote.iter() {
            match all.get_mut(&plugin.identifier) {
                Some(pw) => {
                    pw.remote = Some(plugin.clone());
                }
                None => {
                    all.insert(plugin.identifier.clone(), PluginWrap::new_remote(&plugin));
                }
            }
        }
        self.all = all.into_values().collect();

        // Sort by state and then identifier
        // TODO allow the user to sort or whatever
        self.all.sort_by(|a, b| {
            let ord = a.state.cmp(&b.state);
            if ord == std::cmp::Ordering::Equal {
                a.plugin().identifier.cmp(&b.plugin().identifier)
            } else {
                ord
            }
        });

        // Finally check to see if there is an update
        self.needs_update = self
            .all
            .iter()
            .filter_map(|plugin| {
                if let Some(local) = &plugin.local
                    && let Some(remote) = &plugin.remote
                    && local.version < remote.version
                {
                    Some(local.clone())
                } else {
                    None
                }
            })
            .collect();

        self.save_to_cache()
    }

    fn save_to_cache(&self) -> Result<()> {
        fs::create_dir_all(&self.conf.catalog_cache)?;
        for plugin in self.all.iter() {
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

    fn load_from_cache(&mut self) -> Result<()> {
        let metacatalog = Catalog::from_path(self.conf.catalog_cache.join("metadata.toml"))?;
        self.conf = metacatalog.conf;
        self.meta = metacatalog.meta;
        self.all = fs::read_dir(&self.conf.catalog_cache)?
            .filter_map(|entry| {
                let entry = match entry {
                    Ok(entry) => entry,
                    Err(e) => {
                        warn_err!(e);
                        return None;
                    }
                };
                let mut wrap = match PluginWrap::from_path(entry.path().as_path()) {
                    Ok(wrap) => wrap,
                    Err(_) => {
                        return None;
                    }
                };
                let _ = wrap.load_image(&self.conf.catalog_cache);
                Some(wrap)
            })
            .collect();
        Ok(())
    }

    fn load_from_cache_or_refresh_task(self) -> Task<Message> {
        async fn wrapper(mut catalog: Catalog) -> Catalog {
            let refresh = match catalog.load_from_cache() {
                Ok(()) => {
                    chrono::Local::now().signed_duration_since(catalog.meta.last_updated)
                        >= catalog.conf.refresh_interval
                }

                Err(e) => {
                    warn_err!(e);
                    true
                }
            };
            if refresh && let Err(e) = catalog.refresh_async().await {
                warn_err!(e);
            }
            catalog
        }
        Task::perform(wrapper(self), Message::UpdateCatalog).chain(Task::done(Message::Idle))
    }

    fn refresh_task(self) -> Task<Message> {
        async fn refresh_wrapper(mut catalog: Catalog) -> Catalog {
            if let Err(e) = catalog.refresh_async().await {
                warn_err!(e);
            }
            catalog
        }
        Task::perform(refresh_wrapper(self), Message::UpdateCatalog)
    }

    fn refresh_local_task(self) -> Task<Message> {
        async fn refresh_local_wrapper(mut catalog: Catalog) -> Catalog {
            if let Err(e) = catalog.refresh_local() {
                warn_err!(e);
            }
            catalog
        }
        Task::perform(refresh_local_wrapper(self), Message::UpdateCatalog)
    }
}

#[derive(Debug)]
struct App {
    catalog: Catalog,
    selected: Option<(usize, PluginWrap)>,
    can_refresh: bool,
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
            catalog: Catalog::new(conf),
            selected: None,
            can_refresh: true,
            idle: true,
            drop_action: false,
            default_logo,
        })
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        let task = match message {
            Message::Startup => {
                self.idle = false;
                self.catalog.clone().load_from_cache_or_refresh_task()
            }
            Message::UpdateCatalog(c) => {
                self.catalog = c;
                Task::none()
            }
            Message::RefreshLocal => self.catalog.clone().refresh_local_task(),
            Message::RefreshDone => {
                self.can_refresh = true;
                Task::none()
            }
            Message::Selected(id) => {
                if let Some((rid, _)) = &self.selected {
                    self.selected = if id == *rid {
                        None
                    } else {
                        Some((id, self.catalog.all[id].clone()))
                    }
                } else {
                    self.selected = Some((id, self.catalog.all[id].clone()))
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
                self.catalog.clone().refresh_local_task()
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
                self.catalog.clone().refresh_local_task()
            }
            Message::Uninstall(plugin) => {
                match Installer::new(&self.catalog.conf.install_path, &plugin).uninstall() {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                self.catalog.clone().refresh_local_task()
            }
            Message::UninstallDisabled(plugin) => {
                match Installer::new(&self.catalog.conf.disable_path, &plugin).uninstall() {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                self.catalog.clone().refresh_local_task()
            }
            Message::Idle => {
                self.idle = true;
                Task::none()
            }
            Message::DropDownToggle => {
                self.drop_action = !self.drop_action;
                Task::none()
            }
            Message::ActionRefresh => {
                self.drop_action = false;
                self.can_refresh = false;
                self.catalog
                    .clone()
                    .refresh_task()
                    .chain(Task::done(Message::RefreshDone))
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
                Task::batch(self.catalog.needs_update.iter().map(|plugin| {
                    Task::perform(
                        update_wrapper(Installer::new(&self.catalog.conf.install_path, &plugin)),
                        |_| Message::RefreshLocal,
                    )
                    .discard()
                }))
                .chain(Task::done(Message::RefreshLocal))
                .chain(Task::done(Message::Idle))
            }
        };
        // Clear selection if it's not matched anymore
        if let Some((id, plg)) = &self.selected
            && self.catalog.all[*id].plugin().identifier != plg.plugin().identifier
        {
            self.selected = None; // TODO try to recover selection
        }
        task
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use iced::theme::palette::Pair;
        use widget::{column, container, grid, image, mouse_area, row, scrollable, text};

        let catalog = &self.catalog;
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
                grid(catalog.all.iter().enumerate().map(|(id, v)| {
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

        let (selected, buttons) = if let Some((_, wrp)) = &self.selected {
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
            button(pgettext("plugins", "Update All")).on_press_maybe(
                (!self.catalog.needs_update.is_empty() && self.idle)
                    .then_some(Message::ActionUpdate)
            ),
            button(pgettext("plugins", "Force Refresh"))
                .on_press_maybe(self.can_refresh.then_some(Message::ActionRefresh)),
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
