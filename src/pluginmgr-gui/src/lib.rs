use anyhow::Result;
use iced::{Task, widget};
use log::gettext::{N_, gettext, pgettext};
use log::warn_err;
use pluginmgr::install::Installer;
use pluginmgr::plugin::{Identifier, Plugin};
use std::collections::HashMap;
use std::path::PathBuf;

#[derive(Debug, Clone)]
struct Remote {
    url: reqwest::Url,
    mirror: Option<reqwest::Url>,
    branch: String,
}

#[derive(Debug, Clone)]
struct Conf {
    remotes: Vec<Remote>,
    install_path: PathBuf,
    disable_path: PathBuf,
}
impl Conf {
    fn new() -> Result<Self> {
        Ok(Self {
            remotes: vec![Remote {
                // TODO worth using url-macro library for this?
                url: reqwest::Url::parse("https://codeberg.org/naev/naev-plugins")?,
                mirror: None,
                branch: "main".to_string(),
            }],
            install_path: pluginmgr::local_plugins_dir()?,
            disable_path: pluginmgr::local_plugins_disabled_dir()?,
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
    Selected(usize),
    Install(Plugin),
    Enable(Plugin),
    Disable(Plugin),
    Uninstall(Plugin),
    Refresh(()),
    RefreshLocal(()),
    UpdateCatalog(Catalog),
}

#[derive(Debug, Clone, Copy, PartialEq)]
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

#[derive(Debug, Clone)]
struct PluginWrap {
    local: Option<Plugin>,
    remote: Option<Plugin>,
    state: PluginState,
}
impl PluginWrap {
    fn plugin(&self) -> &Plugin {
        if let Some(local) = &self.local {
            local
        } else if let Some(remote) = &self.remote {
            remote
        } else {
            unreachable!();
        }
    }
}

#[derive(Debug, Clone)]
struct Catalog {
    conf: Conf,
    remote: Vec<Plugin>,
    local: Vec<Plugin>,
    disabled: Vec<Plugin>,
    all: Vec<PluginWrap>,
}
impl Catalog {
    fn new(conf: Conf) -> Self {
        Self {
            conf,
            remote: Vec::new(),
            local: Vec::new(),
            disabled: Vec::new(),
            all: Vec::new(),
        }
    }

    fn refresh(&mut self) -> Result<()> {
        let mut hm: HashMap<Identifier, Plugin> = HashMap::new();
        for remote in &self.conf.remotes {
            let plugins = {
                match pluginmgr::discover_remote_plugins(remote.url.clone(), &remote.branch) {
                    Ok(plugins) => Ok(plugins),
                    Err(e) => {
                        if let Some(mirror) = &remote.mirror {
                            pluginmgr::discover_remote_plugins(mirror.clone(), &remote.branch)
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
        self.refresh_local()
    }

    fn refresh_local(&mut self) -> Result<()> {
        self.local = pluginmgr::discover_local_plugins(&self.conf.install_path)?;
        self.disabled = pluginmgr::discover_local_plugins(&self.conf.disable_path)?;

        let mut all: HashMap<Identifier, PluginWrap> = self
            .local
            .iter()
            .map(|p| {
                (
                    p.identifier.clone(),
                    PluginWrap {
                        local: Some(p.clone()),
                        remote: None,
                        state: PluginState::Installed,
                    },
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
                        PluginWrap {
                            local: Some(plugin.clone()),
                            remote: None,
                            state: PluginState::Disabled,
                        },
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
                    all.insert(
                        plugin.identifier.clone(),
                        PluginWrap {
                            local: None,
                            remote: Some(plugin.clone()),
                            state: PluginState::Available,
                        },
                    );
                }
            }
        }
        self.all = all.into_values().collect();

        Ok(())
    }

    async fn refresh_async(mut catalog: Catalog) -> Catalog {
        if let Err(e) = catalog.refresh() {
            dbg!("foo");
            warn_err!(e);
        }
        catalog
    }

    async fn refresh_local_async(mut catalog: Catalog) -> Catalog {
        if let Err(e) = catalog.refresh_local() {
            warn_err!(e);
        }
        catalog
    }
}

#[derive(Debug)]
struct App {
    catalog: Catalog,
    selected: Option<(usize, PluginWrap)>,
    progress: Option<(String, f32)>,
}

impl App {
    fn run() -> (Self, Task<Message>) {
        let app = Self::new().unwrap();
        (app, Task::none())
    }

    fn new() -> Result<Self> {
        let conf = Conf::new()?;
        std::fs::create_dir_all(&conf.install_path)?;
        std::fs::create_dir_all(&conf.disable_path)?;

        let mut app = App {
            catalog: Catalog::new(conf),
            selected: None,
            progress: None,
        };
        app.catalog.refresh()?;

        Ok(app)
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        let task = match message {
            Message::UpdateCatalog(c) => {
                self.catalog = c;
                Task::none()
            }
            Message::RefreshLocal(_) => Task::perform(
                Catalog::refresh_local_async(self.catalog.clone()),
                Message::UpdateCatalog,
            ),
            Message::Refresh(_) => {
                // TODO get async working
                //Task::perform( Catalog::refresh_async( self.catalog.clone() ), Message::UpdateCatalog )
                match self.catalog.refresh() {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
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
                async fn install_wrap(installer: Installer) {
                    match installer.install().await {
                        Ok(_) => (),
                        Err(e) => warn_err!(e),
                    }
                }
                Task::perform(
                    install_wrap(Installer::new(&self.catalog.conf.install_path, &plugin)),
                    Message::RefreshLocal,
                )
            }
            Message::Enable(plugin) => {
                match Installer::new(&self.catalog.conf.disable_path, &plugin)
                    .move_to(&self.catalog.conf.install_path)
                {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                Task::perform(
                    Catalog::refresh_local_async(self.catalog.clone()),
                    Message::UpdateCatalog,
                )
            }
            Message::Disable(plugin) => {
                match Installer::new(&self.catalog.conf.install_path, &plugin)
                    .move_to(&self.catalog.conf.disable_path)
                {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                Task::perform(
                    Catalog::refresh_local_async(self.catalog.clone()),
                    Message::UpdateCatalog,
                )
            }
            Message::Uninstall(plugin) => {
                match Installer::new(&self.catalog.conf.install_path, &plugin).uninstall() {
                    Ok(_) => (),
                    Err(e) => warn_err!(e),
                }
                Task::perform(
                    Catalog::refresh_local_async(self.catalog.clone()),
                    Message::UpdateCatalog,
                )
            }
        };
        // Clear selection if it's not matched anymore
        if let Some((id, plg)) = &self.selected {
            if self.catalog.all[*id].plugin().identifier != plg.plugin().identifier {
                self.selected = None; // TODO try to recover selection
            }
        }
        task
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use widget::{button, column, container, grid, mouse_area, row, scrollable, text};

        let idle = self.progress.is_none();
        let catalog = &self.catalog;

        let bold = |txt| {
            text(txt).font(iced::Font {
                weight: iced::font::Weight::Bold,
                ..Default::default()
            })
        };
        let plugins = {
            scrollable(
                grid(catalog.all.iter().enumerate().map(|(id, v)| {
                    let p = v.plugin();
                    let image = text("IMG").width(60).height(60);
                    let content = column![
                        row![
                            bold(p.name.as_str()),
                            match v.state {
                                PluginState::Installed =>
                                    text(gettext("[installed]")).color(THEME.palette().warning),
                                PluginState::Disabled => text(gettext("[disabled]"))
                                    .color(THEME.extended_palette().secondary.strong.color),
                                PluginState::Available => text(""),
                            },
                        ]
                        .spacing(5),
                        text(p.r#abstract.as_str()),
                        text(p.tags.join(", ")),
                    ]
                    .spacing(5);
                    let modal = row![image, content,]
                        .spacing(5)
                        .align_y(iced::alignment::Vertical::Center);
                    mouse_area(container(modal).padding(10).style(
                        if let Some(sel) = &self.selected
                            && id == sel.0
                        {
                            container::primary
                        } else {
                            container::rounded_box
                        },
                    ))
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
                    false => Some(THEME.palette().danger),
                })
                .size(20),
                bold(pgettext("plugins", "Status:")),
                info(gettext(sel.release_status.as_str())),
                bold(pgettext("plugins", "Description")),
                text(sel.description.as_ref().unwrap_or(&sel.r#abstract)),
            ]
            .width(300)
            .spacing(5);
            (
                col,
                match wrp.state {
                    PluginState::Installed => row![
                        button(pgettext("plugins", "Disable"))
                            .on_press_maybe(idle.then_some(Message::Disable(sel.clone()))),
                        button(pgettext("plugins", "Uninstall"))
                            .on_press_maybe(idle.then_some(Message::Uninstall(sel.clone()))),
                    ],
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
                column![text("")].width(300),
                row![button(pgettext("plugins", "Install"))],
            )
        };
        // Add refresh button and format
        let buttons = buttons
            .push(
                button(pgettext("plugins", "Refresh"))
                    .on_press_maybe(idle.then_some(Message::Refresh(()))),
            )
            .padding(10)
            .spacing(10);
        // Set up the final screen
        let right = column![buttons, selected].spacing(10);
        let main = row![plugins, right].spacing(20).padding(20);
        // Add progress bar at the bottom
        match &self.progress {
            Some(p) => column![main, text(&p.0), widget::progress_bar(0.0..=100.0, p.1)],
            None => column![
                main,
                text(pgettext("plugins", "Idle")),
                widget::progress_bar(0.0..=100.0, 100.0)
            ],
        }
        .into()
    }
}
