use anyhow::{Context, Result};
use iced::{Task, widget};
use pluginmgr::{
    install::Installer, local::LocalManager, model::LocalPlugin, model::PluginInfo,
    model::SourceKind, naev_plugins_dir, repository::Repository,
};

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
    iced::application("Naev Plugin Manager", App::update, App::view)
        .centered()
        .run_with(App::run)
}

#[derive(Debug)]
enum Message {
    Refresh,
}

struct App {
    conf: Conf,
    repo: git2::Repository,
    remote: Vec<PluginInfo>,
    local: Vec<LocalPlugin>,
    remote_selected: Option<PluginInfo>,
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

        let local_repo = Repository::from_local_path(&repo_path);
        self.remote = local_repo.list_plugins()?;

        let lm = LocalManager::discover()?;
        self.local = lm.list_installed()?;

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

        let local_repo = Repository::from_local_path(&repo_path);
        let remote = local_repo.list_plugins()?;

        let lm = LocalManager::discover()?;
        let local = lm.list_installed()?;

        Ok(App {
            conf,
            repo,
            remote,
            local,
            remote_selected: None,
        })
    }

    fn update(&mut self, message: Message) {
        match message {
            Message::Refresh => {
                self.refresh().unwrap();
            }
        }
    }

    fn view(&self) -> iced::Element<'_, Message> {
        use widget::text;
        let bold = iced::Font {
            weight: iced::font::Weight::Bold,
            ..Default::default()
        };
        let list = widget::Column::from_vec(
            self.remote
                .iter()
                .map(|v| text(v.name.clone()).into())
                .collect(),
        )
        .width(iced::Length::Fill)
        .padding(10);
        let name = match &self.remote_selected {
            Some(rs) => &rs.name,
            None => "N/A",
        };
        let author = match &self.remote_selected {
            Some(rs) => &rs.author,
            None => "N/A",
        };
        let selected = widget::column![
            text("Name:").font(bold.clone()),
            text(name).size(30),
            text("Author(s):").font(bold),
            text(author).size(30),
        ]
        .width(300);
        widget::row![list, selected,].spacing(20).padding(20).into()
    }
}
