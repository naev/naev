use anyhow::Result;
use iced::{Task, widget};
use pluginmgr::plugin::{Identifier, Plugin};
use std::collections::HashMap;

struct Conf {
    plugins_url: String,
    plugins_branch: String,
}
impl Conf {
    fn new() -> Self {
        Default::default()
    }
}
impl Default for Conf {
    fn default() -> Self {
        Self {
            plugins_url: String::from("https://codeberg.org/naev/naev-plugins"),
            plugins_branch: String::from("main"),
        }
    }
}

const THEME: iced::Theme = iced::Theme::Dark;

pub fn open() -> iced::Result {
    iced::application(App::run, App::update, App::view)
        .title("Naev Plugin Manager")
        .theme(THEME)
        .centered()
        .run()
}

#[derive(Debug, Clone, Copy)]
enum Message {
    Close,
    Selected(usize),
    Refresh,
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum PluginState {
    Installed,
    //Disabled,
    Available,
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

struct App {
    conf: Conf,
    remote: Vec<Plugin>,
    local: Vec<Plugin>,
    all: Vec<PluginWrap>,
    selected: Option<(usize, PluginWrap)>,
}

impl App {
    fn run() -> (Self, Task<Message>) {
        let app = Self::new().unwrap();
        (app, Task::none())
    }

    fn refresh(&mut self) -> Result<()> {
        self.remote =
            pluginmgr::discover_remote_plugins(&self.conf.plugins_url, &self.conf.plugins_branch)?;
        self.selected = None; // TODO try to recover selection

        self.local = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;

        Ok(())
    }

    fn new() -> Result<Self> {
        let conf = Conf::new();

        let remote = pluginmgr::discover_remote_plugins(&conf.plugins_url, &conf.plugins_branch)?;
        let local = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;

        let mut all: HashMap<Identifier, PluginWrap> = local
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
        for plugin in remote.iter() {
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

        Ok(App {
            conf,
            remote,
            local,
            all: all.into_values().collect(),
            selected: None,
        })
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        match message {
            Message::Close => iced::exit(),
            Message::Refresh => {
                self.refresh().unwrap();
                Task::none()
            }
            Message::Selected(id) => {
                if let Some((rid, _)) = &self.selected {
                    self.selected = if id == *rid {
                        None
                    } else {
                        Some((id, self.all[id].clone()))
                    }
                } else {
                    self.selected = Some((id, self.all[id].clone()))
                };
                Task::none()
            }
        }
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use widget::{button, column, container, grid, mouse_area, row, scrollable, text};
        let bold = |txt| {
            text(txt).font(iced::Font {
                weight: iced::font::Weight::Bold,
                ..Default::default()
            })
        };
        let plugins = {
            scrollable(
                grid(self.all.iter().enumerate().map(|(id, v)| {
                    let p = v.plugin();
                    let image = text("IMG").width(60).height(60);
                    let content = column![
                        row![
                            bold(p.name.as_str()),
                            text(match v.state {
                                PluginState::Installed => "[installed]",
                                PluginState::Available => "",
                            })
                            .color(THEME.palette().warning)
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

        let selected = if let Some((_, sel)) = &self.selected {
            let sel = sel.plugin();
            let info = |txt| text(txt).size(20);
            column![
                bold("Identifier:"),
                info(sel.identifier.as_str()),
                bold("Name:"),
                info(sel.name.as_str()),
                bold("Author(s):"),
                info(&sel.author),
                bold("Plugin Version:"),
                text(sel.version.to_string()).size(20),
                bold("Naev Version:"),
                text(format!(
                    "{}{}",
                    sel.naev_version,
                    match sel.compatible {
                        true => "".to_string(),
                        false => format!(" [incompatible with Naev {}]", *log::version::VERSION),
                    }
                ))
                .color_maybe(match sel.compatible {
                    true => None,
                    false => Some(THEME.palette().danger),
                })
                .size(20),
                bold("Status:"),
                info(sel.release_status.as_str()),
                bold("Description"),
                text(sel.description.as_ref().unwrap_or(&sel.r#abstract)),
            ]
            .width(300)
            .spacing(5)
        } else {
            column![text("")].width(300)
        };
        let main = row![plugins, selected,].spacing(20).padding(20);
        let buttons = row![
            button("Refresh").on_press(Message::Refresh),
            button("Close").on_press(Message::Close),
        ]
        .padding(20)
        .spacing(10);
        column![main, buttons].into()
    }
}
