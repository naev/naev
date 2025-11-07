use crate::model::PluginInfo;
use anyhow::{Context, Result, bail};
use fs_err as fs;
use std::path::PathBuf;

/// Placeholder installer. Wire up real git/zip logic later.
pub struct Installer {
    pub dest_root: PathBuf,
}

impl Installer {
    pub fn new(dest_root: PathBuf) -> Self {
        Self { dest_root }
    }

    pub fn install(&self, info: &PluginInfo) -> Result<()> {
        match info.source_kind() {
            Some(crate::model::SourceKind::Git) => self.install_from_git(info),
            Some(crate::model::SourceKind::Link) => self.install_from_zip(info),
            None => bail!("Plugin does not specify <git> or <link> source"),
        }
    }

    fn install_from_git(&self, info: &PluginInfo) -> Result<()> {
        let url = info.git.as_deref().context("Missing <git> URL")?;
        let name = &info.name;
        let dest = self.dest_root.join(name);
        if dest.exists() {
            fs::remove_dir_all(&dest).ok();
        }
        // TODO: use git2::Repository::clone_recurse for submodules; for now, basic clone.
        let _repo = git2::Repository::clone(url, &dest)
            .with_context(|| format!("Cloning {url} into {}", dest.display()))?;
        Ok(())
    }

    /// Checks whether a git-based plugin has updates available upstream.
    pub fn check_git_update(&self, info: &PluginInfo) -> Result<Option<String>> {
        use git2::Repository;

        let url = info.git.as_deref().context("Missing <git> URL")?;
        let dest = self.dest_root.join(&info.name);

        if !dest.exists() {
            return Ok(None); // not installed yet
        }

        let repo = Repository::open(&dest)
            .with_context(|| format!("Failed to open local repo {}", dest.display()))?;

        // Fetch from origin
        {
            let mut remote = repo
                .find_remote("origin")
                .or_else(|_| repo.remote("origin", url))
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
    pub fn update_git_plugin(&self, info: &PluginInfo) -> Result<()> {
        use git2::{FetchOptions, Repository};

        let url = info.git.as_deref().context("Missing <git> URL")?;
        let dest = self.dest_root.join(&info.name);
        let repo = Repository::open(&dest)
            .with_context(|| format!("Failed to open local repo {}", dest.display()))?;

        // Prepare remote
        let mut remote = repo
            .find_remote("origin")
            .or_else(|_| repo.remote("origin", url))
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
            tracing::info!("{} is already up to date.", info.name);
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
            tracing::info!("Fast-forwarded {} to latest commit.", info.name);
        } else if analysis.is_up_to_date() {
            tracing::info!("{} is already up to date.", info.name);
        } else {
            tracing::warn!(
                "{} cannot be fast-forwarded; manual merge required.",
                info.name
            );
        }

        Ok(())
    }

    /// Installs a plugin from a direct zip link by downloading and saving the archive.
    /// The zip file is stored directly in the plugins directory without extraction.
    pub fn install_from_zip(&self, info: &PluginInfo) -> Result<()> {
        let url = info.link.as_deref().context("Missing <link> URL")?;
        let name = &info.name;
        let dest_zip = self.dest_root.join(format!("{}.zip", name));

        tracing::info!("Downloading plugin '{}' from {}", name, url);

        // Download zip into memory
        let rt = tokio::runtime::Runtime::new()?;
        let bytes = rt.block_on(async {
            let resp = reqwest::get(url).await?.error_for_status()?;
            Ok::<bytes::Bytes, anyhow::Error>(resp.bytes().await?)
        })?;

        // Ensure destination directory exists
        fs::create_dir_all(&self.dest_root)?;

        // Save as a single zip file in plugins dir
        fs::write(&dest_zip, &bytes)?;
        tracing::info!("Installed '{}' to {}", name, dest_zip.display());
        Ok(())
    }

    /// Updates a zip-based plugin by re-downloading and replacing the archive file.
    pub fn update_zip_plugin(&self, info: &PluginInfo) -> Result<()> {
        let url = info.link.as_deref().context("Missing <link> URL")?;
        let name = &info.name;
        let dest_zip = self.dest_root.join(format!("{}.zip", name));

        // Check version difference (same as before)
        let mut local_version = None;
        if dest_zip.exists() {
            let file = fs::File::open(&dest_zip)?;
            let mut zip = zip::ZipArchive::new(file)?;
            if zip.file_names().any(|n| n == "plugin.xml") {
                use std::io::Read;
                let mut xml_data = String::new();
                {
                    let mut f = zip.by_name("plugin.xml")?;
                    f.read_to_string(&mut xml_data)?;
                }
                if let Ok(local_info) =
                    quick_xml::de::from_str::<crate::model::InstalledPluginInfo>(&xml_data)
                {
                    local_version = local_info.version;
                }
            }
        }

        if let (Some(repo_v), Some(local_v)) = (&info.version, &local_version) {
            if repo_v <= local_v {
                tracing::info!(
                    "{} is already up to date (local v{} >= repo v{})",
                    name,
                    local_v,
                    repo_v
                );
                return Ok(());
            }
        }

        tracing::info!("Updating zip plugin '{}' from {}", name, url);

        // Download new archive
        let rt = tokio::runtime::Runtime::new()?;
        let bytes = rt.block_on(async {
            let resp = reqwest::get(url).await?.error_for_status()?;
            Ok::<bytes::Bytes, anyhow::Error>(resp.bytes().await?)
        })?;

        // Overwrite existing archive
        fs::write(&dest_zip, &bytes)?;
        tracing::info!(
            "Successfully updated '{}' to version {:?}",
            name,
            info.version
        );
        Ok(())
    }

    pub fn uninstall(&self, name: &str) -> Result<()> {
        let target = self.dest_root.join(name);
        if target.exists() {
            fs::remove_dir_all(&target)?;
        }
        Ok(())
    }
}
