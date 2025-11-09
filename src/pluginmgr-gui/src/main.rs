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

fn main() -> iced::Result {
    iced::application(App::run, App::update, App::view)
        .title("Naev Plugin Manager")
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
                self.remote_selected = Some((id, self.remote[id].clone()));
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
                        bold(v.name.clone()),
                        text(v.r#abstract.clone()),
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
        let name = match &self.remote_selected {
            Some(rs) => &rs.1.name,
            None => "N/A",
        };
        let author = match &self.remote_selected {
            Some(rs) => &rs.1.author,
            None => "N/A",
        };
        let selected = widget::column![
            bold("Name:".to_string()),
            text(name).size(20),
            bold("Author(s):".to_string()),
            text(author).size(20),
        ]
        .width(300)
        .spacing(5);
        widget::row![plugins, selected,]
            .spacing(20)
            .padding(20)
            .into()
    }
}
