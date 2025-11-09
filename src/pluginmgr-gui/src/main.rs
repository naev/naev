use anyhow::{Context, Result};
use iced::{Task, widget};
use pluginmgr::plugin::{Plugin, PluginStub};
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
    repo: git2::Repository,
    remote: Vec<PluginStub>,
    local: Vec<Plugin>,
    remote_selected: Option<(usize, PluginStub)>,
    local_versions: HashMap<String, semver::Version>,
}

impl App {
    fn run() -> (Self, Task<Message>) {
        let app = Self::new().unwrap();
        (app, Task::none())
    }

    fn refresh_repo(conf: &Conf, repo: &git2::Repository) -> Result<()> {
        repo.find_remote("origin")?
            .fetch(&[&conf.plugins_branch], None, None)?;
        repo.checkout_head(None)?;
        Ok(())
    }

    fn refresh(&mut self) -> Result<()> {
        Self::refresh_repo(&self.conf, &self.repo)?;

        let repo_path = self
            .repo
            .workdir()
            .context("getting plugins repository workdir")?;

        self.remote = pluginmgr::repository(&repo_path)?;
        self.remote_selected = None; // TODO try to recover selection

        self.local = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;

        Ok(())
    }

    fn new() -> Result<Self> {
        let conf = Conf::new();

        let proj_dirs = directories::ProjectDirs::from("org", "naev", "naev")
            .context("getting project directorios")?;
        let cache_dir = proj_dirs.cache_dir();
        let repo_path = cache_dir.join("naev-plugins");

        let repo = if repo_path.exists() {
            let repo = match git2::Repository::open(&repo_path) {
                Ok(repo) => repo,
                Err(e) => anyhow::bail!("failed to open: {}", e),
            };
            Self::refresh_repo(&conf, &repo)?;
            repo
        } else {
            match git2::Repository::clone(&conf.plugins_url, &repo_path) {
                Ok(repo) => repo,
                Err(e) => anyhow::bail!("failed to clone: {}", e),
            }
        };

        let remote = pluginmgr::repository(&repo_path)?;
        let local = pluginmgr::discover_local_plugins(pluginmgr::local_plugins_dir()?)?;

        let local_versions: HashMap<String, semver::Version> = local
            .iter()
            .map(|p| (p.name.to_lowercase(), p.version.clone()))
            .collect();

        Ok(App {
            conf,
            repo,
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
            scrollable( grid(
                self.remote
                .iter()
                .enumerate()
                .map(|(id,v)| {
                    let image = text("IMG").width(60).height(60);
                    let content = column![
                        bold(v.name.clone()),
                        text("The abstract will be allowed to have up to 200 characters, so we should simulate something like that. Maybe a bit more. So this is about to become a fully specced abstract with exactly 200 characters."),
                        text("TAGS"),
                    ].spacing(5);
                    let modal = row![
                        image,
                        content,
                    ].spacing(5)
                        .align_y(iced::alignment::Vertical::Center);
                    mouse_area( container( modal )
                        .padding(10)
                        .style( if let Some(sel) = &self.remote_selected && id==sel.0 {
                            container::primary
                        } else {
                            container::rounded_box
                        }) )
                        .on_press( Message::Selected(id) )
                        .into()
                } )
            ).fluid(500).spacing(10).height( grid::Sizing::EvenlyDistribute( iced::Length::Shrink ) ) ).spacing(10)
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
            Some(rs) => &rs.1.identifier,
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
