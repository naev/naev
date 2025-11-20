use crate::plugin::{Plugin, Source};
use anyhow::{Context, Result};
use fs_err as fs;
use log::{info, warn};
use std::path::{Path, PathBuf};

#[derive(Debug, Clone)]
pub struct Progress {
    pub message: Option<String>,
    pub value: f32,
}
impl Into<f32> for Progress {
    fn into(self) -> f32 {
        self.value
    }
}
impl From<f32> for Progress {
    fn from(value: f32) -> Self {
        Progress {
            message: None,
            value,
        }
    }
}

/// Placeholder installer. Wire up real git/zip logic later.
pub struct Installer {
    root: PathBuf,
    plugin: Plugin,
}

impl Installer {
    pub fn new<P: AsRef<Path>>(root: P, plugin: &Plugin) -> Self {
        Self {
            root: root.as_ref().to_owned(),
            plugin: plugin.clone(),
        }
    }

    /// Installs from a plugin from a source
    pub async fn install(self) -> Result<()> {
        match &self.plugin.source {
            Source::Git(url) => self.install_from_git(url),
            Source::Download(url) => self.install_from_zip_url(url).await,
            Source::Local => anyhow::bail!("local plugin"),
        }
    }

    /// Updates from a plugin from a source
    pub async fn update(self) -> Result<()> {
        match &self.plugin.source {
            Source::Git(url) => self.update_git_plugin(url),
            Source::Download(url) => self.update_zip_plugin(url).await,
            Source::Local => anyhow::bail!("local plugin"),
        }
    }

    /// Moves a plugin to another directory
    pub fn move_to<P: AsRef<Path>>(self, to: P) -> Result<()> {
        let from = {
            // Try to use mount point if applicable, for manually installed stuff
            match self.plugin.mountpoint {
                Some(mp) => mp,
                None => self.root.join(&*self.plugin.identifier),
            }
        };
        info!(
            "Moving plugin '{}' from '{}' -> '{}'",
            &self.plugin.identifier,
            from.display(),
            to.as_ref().display()
        );
        fs_extra::dir::move_dir(&from, &to, &Default::default())?;
        info!("Moving completed");
        Ok(())
    }

    fn install_from_git(&self, url: &reqwest::Url) -> Result<()> {
        info!("Installing plugin from git repo '{}'", &url);
        let info = &self.plugin;
        let dest = self.root.join(&*info.identifier);
        if dest.exists() {
            fs::remove_dir_all(&dest).ok();
        }
        // TODO: use git2::Repository::clone_recurse for submodules; for now, basic clone.
        let _repo = git2::Repository::clone(url.as_str(), &dest)
            .with_context(|| format!("Cloning {url} into {}", dest.display()))?;
        info!("Installed '{}' to {}", &info.name, dest.display());
        Ok(())
    }

    /// Checks whether a git-based plugin has updates available upstream.
    pub fn check_git_update(&self, url: &reqwest::Url) -> Result<Option<String>> {
        let info = &self.plugin;
        use git2::Repository;

        let dest = self.root.join(&*info.identifier);

        if !dest.exists() {
            return Ok(None); // not installed yet
        }

        let repo = Repository::open(&dest)
            .with_context(|| format!("Failed to open local repo {}", dest.display()))?;

        // Fetch from origin
        {
            let mut remote = repo
                .find_remote("origin")
                .or_else(|_| repo.remote("origin", url.as_str()))
                .with_context(|| {
                    format!("Could not find or create remote 'origin' for {}", info.name)
                })?;
            remote
                .fetch(&["main", "master"], None, None)
                .with_context(|| format!("Failed to fetch remote for {}", info.name))?;
        }

        // Compare local HEAD vs remote HEAD
        let head = repo.head()?.peel_to_commit()?.id();
        let fetch_head = repo.find_reference("FETCH_HEAD")?.peel_to_commit()?.id();

        if head != fetch_head {
            Ok(Some(format!("{fetch_head:?}"))) // update available
        } else {
            Ok(None)
        }
    }

    /// Performs a git pull if an update is available.
    pub fn update_git_plugin(&self, url: &reqwest::Url) -> Result<()> {
        use git2::{FetchOptions, Repository};

        let info = &self.plugin;
        let dest = self.root.join(&*info.identifier);
        let repo = Repository::open(&dest)
            .with_context(|| format!("Failed to open local repo {}", dest.display()))?;

        // Prepare remote
        let mut remote = repo
            .find_remote("origin")
            .or_else(|_| repo.remote("origin", url.as_str()))
            .with_context(|| {
                format!("Could not find or create remote 'origin' for {}", info.name)
            })?;

        // Fetch refs
        let mut fo = FetchOptions::new();
        remote.fetch(&["refs/heads/*:refs/remotes/origin/*"], Some(&mut fo), None)?;

        // Determine branch name
        let head = repo.head()?.shorthand().unwrap_or("main").to_string();
        let local_branch = format!("refs/heads/{}", head);
        let remote_branch = format!("refs/remotes/origin/{}", head);

        // Look up commits
        let local_oid = repo.refname_to_id(&local_branch)?;
        let remote_oid = repo.refname_to_id(&remote_branch)?;
        if local_oid == remote_oid {
            info!("{} is already up to date.", info.name);
            return Ok(());
        }

        // Convert to annotated commit for analysis
        let annotated = repo.find_annotated_commit(remote_oid)?;
        let (analysis, _) = repo.merge_analysis(&[&annotated])?;

        if analysis.is_fast_forward() {
            let mut ref_heads = repo.find_reference(&local_branch)?;
            ref_heads.set_target(remote_oid, "Fast-forward update")?;
            repo.set_head(&local_branch)?;
            repo.checkout_head(Some(git2::build::CheckoutBuilder::default().force()))?;
            info!("Fast-forwarded {} to latest commit.", info.name);
        } else if analysis.is_up_to_date() {
            info!("{} is already up to date.", info.name);
        } else {
            warn!(
                "{} cannot be fast-forwarded; manual merge required.",
                info.name
            );
        }

        Ok(())
    }

    /// Installs a plugin from a direct zip link by downloading and saving the archive.
    /// The zip file is stored directly in the plugins directory without extraction.
    pub async fn install_from_zip_url(&self, url: &reqwest::Url) -> Result<()> {
        info!("Installing plugin from zip download '{}'", &url);
        let info = &self.plugin;
        let dest_zip = self.root.join(format!("{}.zip", &info.identifier));

        info!("Downloading plugin '{}' from {}", &info.name, url);

        // Download zip into memory
        let bytes = {
            let resp = reqwest::get(url.clone()).await?.error_for_status()?;
            Ok::<bytes::Bytes, anyhow::Error>(resp.bytes().await?)
        }?;

        // Ensure destination directory exists
        fs::create_dir_all(&self.root)?;

        // Save as a single zip file in plugins dir
        fs::write(&dest_zip, &bytes)?;
        info!("Installed '{}' to {}", &info.name, dest_zip.display());
        Ok(())
    }

    /// Updates a zip-based plugin by re-downloading and replacing the archive file.
    pub async fn update_zip_plugin(&self, url: &reqwest::Url) -> Result<()> {
        let info = &self.plugin;
        let dest_zip = self.root.join(format!("{}.zip", &info.identifier));

        // Check version difference (same as before)
        let local = Plugin::from_path(&dest_zip)?;

        if info.version <= local.version {
            info!(
                "{} is already up to date (local v{} >= repo v{})",
                &info.name, info.version, local.version
            );
            return Ok(());
        }

        info!("Updating zip plugin '{}' from {}", &info.name, url);

        // Download new archive
        let bytes = {
            let resp = reqwest::get(url.clone()).await?.error_for_status()?;
            Ok::<bytes::Bytes, anyhow::Error>(resp.bytes().await?)
        }?;

        // Overwrite existing archive
        fs::write(&dest_zip, &bytes)?;
        info!(
            "Successfully updated '{}' to version {:?}",
            &info.name, info.version
        );
        Ok(())
    }

    pub fn uninstall(self) -> Result<()> {
        info!("Uninstalling plugin '{}'", &self.plugin.identifier);
        let target = {
            // Try to uninstall from mount point first
            match self.plugin.mountpoint {
                Some(mp) => mp,
                None => self.root.join(&*self.plugin.identifier),
            }
        };
        if target.exists() {
            fs::remove_dir_all(&target)?;
        }
        Ok(())
    }
}
