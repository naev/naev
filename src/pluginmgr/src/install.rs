use crate::git;
use crate::plugin::{Plugin, Source};
use anyhow::Error;
use formatx::formatx;
use fs_err as fs;
use iced::task::{Sipper, Straw, sipper};
use log::gettext::pgettext;
use log::info;
use std::path::{Path, PathBuf};

#[derive(Debug, Clone)]
pub struct Progress {
    pub message: Option<String>,
    pub value: f32,
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
    pub fn install(self) -> impl Straw<(), Progress, Error> {
        sipper(async move |mut sender| {
            sender
                .send(Progress {
                    message: formatx!(
                        pgettext("plugins", "Installing plugin '{}'"),
                        &self.plugin.name
                    )
                    .ok(),
                    value: 0.0,
                })
                .await;
            match &self.plugin.source {
                Source::Git(url) => self.install_from_git(url.clone()).run(&sender).await,
                Source::Download(url) => self.install_from_zip_url(url.clone()).run(&sender).await,
                Source::Local => anyhow::bail!("local plugin"),
            }?;
            sender.send(1.0.into()).await;
            Ok(())
        })
    }

    /// Updates from a plugin from a source
    pub fn update(self) -> impl Straw<(), Progress, Error> {
        sipper(async move |mut sender| {
            sender
                .send(Progress {
                    message: formatx!(
                        pgettext("plugins", "Updating plugin '{}'"),
                        &self.plugin.name
                    )
                    .ok(),
                    value: 0.0,
                })
                .await;
            match &self.plugin.source {
                Source::Git(_url) => self.update_git_plugin().run(&sender).await,
                Source::Download(url) => self.update_zip_plugin(url.clone()).run(&sender).await,
                Source::Local => anyhow::bail!("local plugin"),
            }?;
            sender.send(1.0.into()).await;
            Ok(())
        })
    }

    pub fn install_from_git<U: reqwest::IntoUrl>(&self, url: U) -> impl Straw<(), Progress, Error> {
        sipper(async move |sender| {
            let info = &self.plugin;
            let dest = self.root.join(&*info.identifier);
            git::clone(&dest, url).run(&sender).await?;
            Ok(())
        })
    }

    /// Performs a git pull if an update is available.
    pub fn update_git_plugin(&self) -> impl Straw<(), Progress, Error> {
        sipper(async move |sender| {
            let info = &self.plugin;
            let dest = self.root.join(&*info.identifier);
            git::pull(&dest).run(&sender).await?;
            Ok(())
        })
    }

    /// Installs a plugin from a direct zip link by downloading and saving the archive.
    /// The zip file is stored directly in the plugins directory without extraction.
    pub fn install_from_zip_url<U: reqwest::IntoUrl>(
        &self,
        url: U,
    ) -> impl Straw<(), Progress, Error> {
        sipper(async move |mut _sender| {
            info!("Installing plugin from zip download '{}'", url.as_str());
            let info = &self.plugin;
            let dest_zip = self.root.join(format!("{}.zip", &info.identifier));

            info!("Downloading plugin '{}' from {}", &info.name, url.as_str());

            // Download zip into memory
            let bytes = {
                let resp = reqwest::get(url).await?.error_for_status()?;
                Ok::<bytes::Bytes, anyhow::Error>(resp.bytes().await?)
            }?;

            // Ensure destination directory exists
            fs::create_dir_all(&self.root)?;

            // Save as a single zip file in plugins dir
            fs::write(&dest_zip, &bytes)?;
            info!("Installed '{}' to {}", &info.name, dest_zip.display());
            Ok(())
        })
    }

    /// Updates a zip-based plugin by re-downloading and replacing the archive file.
    pub fn update_zip_plugin<U: reqwest::IntoUrl>(
        &self,
        url: U,
    ) -> impl Straw<(), Progress, Error> {
        sipper(async move |mut _sender| {
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

            info!("Updating zip plugin '{}' from {}", &info.name, url.as_str());

            // Download new archive
            let bytes = {
                let resp = reqwest::get(url).await?.error_for_status()?;
                Ok::<bytes::Bytes, anyhow::Error>(resp.bytes().await?)
            }?;

            // Overwrite existing archive
            fs::write(&dest_zip, &bytes)?;
            info!(
                "Successfully updated '{}' to version {:?}",
                &info.name, info.version
            );
            Ok(())
        })
    }

    pub fn uninstall(self) -> impl Straw<(), Progress, Error> {
        sipper(async move |mut sender| {
            sender
                .send(Progress {
                    message: formatx!(
                        pgettext("plugins", "Uninstalling plugin '{}'"),
                        &self.plugin.identifier
                    )
                    .ok(),
                    value: 0.0,
                })
                .await;
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
            sender.send(1.0.into()).await;
            Ok(())
        })
    }
}
