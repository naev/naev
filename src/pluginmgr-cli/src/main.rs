use anyhow::Result;
use clap::{Parser, Subcommand};
use pluginmgr::{
   install::Installer, local::LocalManager, model::SourceKind, naev_plugins_dir,
   repository::Repository,
};
use serde::Serialize;
use std::path::PathBuf;
use tracing_subscriber::{EnvFilter, fmt};

/// Helper macro for unified human / JSON output.
/// Example:
///   print_output!(cli.json, &data, {
///       for item in &data { println!("{}", item.name); }
///   });
macro_rules! print_output {
   ($json:expr, $data:expr, $fallback:block) => {
      if $json {
         println!("{}", serde_json::to_string_pretty(&$data)?);
      } else {
         $fallback
      }
   };
}

#[derive(Parser)]
#[command(name = "naev-pluginmgr", version, about = "Naev Plugin Manager CLI")]
struct Cli {
   /// Path to a checked-out plugin repo (containing `plugins/` with XML files)
   #[arg(long)]
   repo_path: Option<PathBuf>,

   /// Output results as JSON (machine-readable)
   #[arg(long, global = true)]
   json: bool,

   #[command(subcommand)]
   cmd: Commands,
}

#[derive(Subcommand)]
enum Commands {
   ListRepo,
   ListLocal,
   Install { name: String },
   Uninstall { name: String },
   Update { name: String },
   UpdateAll,
   Mount { name: String },
   Validate { name: Option<String> },
}

fn main() -> Result<()> {
   let _guard = init_tracing();
   let cli = Cli::parse();

   match cli.cmd {
      Commands::ListRepo => {
         let repo_path = cli.repo_path.expect("--repo-path is required for ListRepo");
         let repo = Repository::from_local_path(repo_path);
         let plugins = repo.list_plugins()?;

         let lm = LocalManager::discover()?;
         let installed = lm.list_installed()?;

         // Build map: name -> version
         use std::collections::HashMap;
         let installed_versions: HashMap<String, Option<String>> = installed
            .iter()
            .map(|p| (p.name.to_lowercase(), p.version.clone()))
            .collect();

         let dest = PathBuf::from(naev_plugins_dir()?.as_str());
         let inst = Installer::new(dest);

         let output: Vec<_> = plugins
            .into_iter()
            .map(|p| {
               let lower = p.name.to_lowercase();
               let mut status = "[Available]".to_string();

               if let Some(local_version) = installed_versions.get(&lower) {
                  status = "[Installed]".to_string();

                  if let (Some(repo_v), Some(local_v)) = (&p.version, local_version) {
                     if repo_v > local_v {
                        status = "[Update Available]".to_string();
                     }
                  } else if let Some(SourceKind::Git) = p.source_kind() {
                     if let Ok(Some(_)) = inst.check_git_update(&p) {
                        status = "[Update Available]".to_string();
                     }
                  }
               }

               #[derive(Serialize)]
               struct RepoEntry {
                  name: String,
                  status: String,
                  author: String,
                  source: String,
               }

               RepoEntry {
                  name: p.name.clone(),
                  status,
                  author: p.author.clone(),
                  source: p
                     .git
                     .as_deref()
                     .or(p.link.as_deref())
                     .unwrap_or("<no source>")
                     .to_string(),
               }
            })
            .collect();

         print_output!(cli.json, &output, {
            for e in &output {
               println!(
                  "{:<25} {:<18} {:<20} {}",
                  e.name, e.status, e.author, e.source
               );
            }
         });
      }
      Commands::ListLocal => {
         let lm = LocalManager::discover()?;
         let plugins = lm.list_installed()?;
         print_output!(cli.json, &plugins, {
            for p in &plugins {
               println!(
                  "{:<25} {:<8} {:<20} {}",
                  p.name,
                  p.version.as_deref().unwrap_or("-"),
                  p.author.as_deref().unwrap_or("<unknown>"),
                  p.path
               );
               if let Some(desc) = &p.description {
                  println!("    {}", desc);
               }
               if let Some(src) = &p.source {
                  println!("    Source: {}", src);
               }
            }
         });
      }
      Commands::Install { name } => {
         let repo = Repository::from_local_path(
            cli.repo_path.expect("--repo-path is required for Install"),
         );
         let info = repo
            .list_plugins()?
            .into_iter()
            .find(|p| p.name == name)
            .expect("Plugin not found in repository");
         let dest = PathBuf::from(naev_plugins_dir()?.as_str());
         let inst = Installer::new(dest);
         inst.install(&info)?;
         println!("Installed {}", info.name);
      }
      Commands::Uninstall { name } => {
         let dest = PathBuf::from(naev_plugins_dir()?.as_str());
         let inst = Installer::new(dest);
         inst.uninstall(&name)?;
         println!("Uninstalled {}", name);
      }
      Commands::Update { name } => {
         let repo =
            Repository::from_local_path(cli.repo_path.expect("--repo-path is required for Update"));
         let info = repo
            .list_plugins()?
            .into_iter()
            .find(|p| p.name == name)
            .unwrap_or_else(|| panic!("Plugin '{}' not found in repository.", name));

         let dest = PathBuf::from(naev_plugins_dir()?.as_str());
         let inst = Installer::new(dest);

         if let Some(SourceKind::Git) = info.source_kind() {
            inst.update_git_plugin(&info)?;
            println!("Updated {}", info.name);
         } else {
            println!(
               "Plugin '{}' is not a git-based plugin and cannot be updated automatically.",
               info.name
            );
         }
      }
      Commands::Mount { name } => {
         let lm = LocalManager::discover()?;
         let mut mount = lm.mount_plugin(&name)?;
         println!("Mounted plugin: {}", mount.source_path().display());
         for f in mount.list_files()? {
            println!("  {}", f);
         }
      }
      Commands::Validate { name } => {
         let lm = LocalManager::discover()?;
         let root = &lm.root;

         if let Some(name) = name {
            // Validate a single plugin
            let path_dir = root.join(&name);
            let path_zip = root.join(format!("{}.zip", name));
            let target = if path_zip.exists() {
               path_zip
            } else {
               path_dir
            };

            let result = pluginmgr::validator::validate_plugin(&target)?;
            if result.valid {
               println!("{} - valid", result.name);
            } else {
               println!("{} - invalid:", result.name);
               for err in result.errors {
                  println!("    • {}", err);
               }
               std::process::exit(1);
            }
         } else {
            // Validate all plugins
            let results = pluginmgr::validate_all(&lm)?;
            let mut valid_count = 0;
            let mut invalid_count = 0;

            if cli.json {
               println!("{}", serde_json::to_string_pretty(&results)?);
               // Return nonzero if any invalid
               if results.iter().any(|r| !r.valid) {
                  std::process::exit(1);
               }
            } else {
               println!("Validating all installed plugins...\n");
               for r in &results {
                  if r.valid {
                     println!("✔ {:<25} valid", r.name);
                     valid_count += 1;
                  } else {
                     println!("✖ {:<25} invalid", r.name);
                     for e in &r.issues {
                        println!("    • {}", e);
                     }
                     invalid_count += 1;
                  }
               }
               println!(
                  "\nSummary: {} valid, {} invalid",
                  valid_count, invalid_count
               );
               if invalid_count > 0 {
                  std::process::exit(1);
               }
            }
         }
      }
      Commands::UpdateAll => {
         let repo_path = cli
            .repo_path
            .expect("--repo-path is required for update-all");
         let repo = Repository::from_local_path(repo_path);
         let lm = LocalManager::discover()?;

         let dest = std::path::PathBuf::from(naev_plugins_dir()?.as_str());
         let inst = Installer::new(dest);
         let results = pluginmgr::update_all(&repo, &lm, &inst)?;
         print_output!(cli.json, &results, {
            println!("Checking all installed plugins for updates...\n");
            for r in &results {
               println!("{:<25} {}", r.name, r.status);
            }
            println!("\nUpdate check complete.");
         });
      }
   }

   Ok(())
}

fn init_tracing() -> tracing_appender::non_blocking::WorkerGuard {
   let filter = EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("info"));
   fmt().with_env_filter(filter).init();
   // dummy guard to keep type; no file appender here
   let (nb, guard) = tracing_appender::non_blocking(std::io::stderr());
   let _ = nb; // silence unused
   guard
}
