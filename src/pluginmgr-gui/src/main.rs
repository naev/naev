use anyhow::Result;
use iced::{Task, widget};
use pluginmgr::plugin::Plugin;
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

fn main() -> iced::Result {
    iced::application(App::run, App::update, App::view)
        .title("Naev Plugin Manager")
        .theme(THEME)
        .centered()
        .run()
}

#[derive(Debug, Clone, Copy)]
enum Message {
    Selected(usize),
    Refresh,
}

struct App {
    conf: Conf,
    remote: Vec<Plugin>,
    local: Vec<Plugin>,
    remote_selected: Option<(usize, Plugin)>,
    local_versions: HashMap<String, semver::Version>,
}

impl App {
    fn run() -> (Self, Task<Message>) {
        let app = Self::new().unwrap();
        (app, Task::none())
    }

    fn refresh(&mut self) -> Result<()> {
        self.remote =
            pluginmgr::discover_remote_plugins(&self.conf.plugins_url, &self.conf.plugins_branch)?;
        self.remote_selected = None; // TODO try to recover selection

        self.local = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;

        Ok(())
    }

    fn new() -> Result<Self> {
        let conf = Conf::new();

        let remote = pluginmgr::discover_remote_plugins(&conf.plugins_url, &conf.plugins_branch)?;
        let local = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;

        let local_versions: HashMap<String, semver::Version> = local
            .iter()
            .map(|p| (p.name.to_lowercase(), p.version.clone()))
            .collect();

        Ok(App {
            conf,
            remote,
            local,
            remote_selected: None,
            local_versions,
        })
    }

    fn update(&mut self, message: Message) {
        match message {
            Message::Refresh => self.refresh().unwrap(),
            Message::Selected(id) => {
                if let Some((rid, _)) = &self.remote_selected {
                    self.remote_selected = if id == *rid {
                        None
                    } else {
                        Some((id, self.remote[id].clone()))
                    }
                } else {
                    self.remote_selected = Some((id, self.remote[id].clone()))
                }
            }
        }
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use widget::{column, container, grid, mouse_area, row, scrollable, text};
        let bold = |txt| {
            text(txt).font(iced::Font {
                weight: iced::font::Weight::Bold,
                ..Default::default()
            })
        };
        let plugins = {
            scrollable(
                grid(self.remote.iter().enumerate().map(|(id, v)| {
                    let image = text("IMG").width(60).height(60);
                    let content = column![
                        bold(v.name.as_str()),
                        text(v.r#abstract.as_str()),
                        text(v.tags.join(", ")),
                    ]
                    .spacing(5);
                    let modal = row![image, content,]
                        .spacing(5)
                        .align_y(iced::alignment::Vertical::Center);
                    mouse_area(container(modal).padding(10).style(
                        if let Some(sel) = &self.remote_selected
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
            /*
            let installed = table::column(bold("Status"), |this: &PluginInfo| {
                let localver = self.local_versions.get(&this.name);
                match localver {
                    Some(v) => text("installed"),
                    None => text(""),
                }
            });
            let names = table::column(bold("Name"), |this: &PluginInfo| text(this.name.clone()));
            let authors = table::column(bold("Author"), |this: &PluginInfo| {
                text(this.author.clone())
            });
            let versions = table::column(bold("Version"), |this: &PluginInfo| {
                text(match &this.version {
                    Some(v) => v.clone(),
                    None => "".to_string(),
                })
            });
            table([installed, names, authors, versions], &self.remote).width(iced::Length::Fill)
            */
        };

        let selected = if let Some((_, sel)) = &self.remote_selected {
            let info = |txt| text(txt).size(20);
            widget::column![
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
            widget::column![text("")].width(300)
        };
        widget::row![plugins, selected,]
            .spacing(20)
            .padding(20)
            .into()
    }
}
