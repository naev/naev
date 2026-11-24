use std::{
   collections::BTreeSet,
   env, fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use goblin::mach::MachO;

use super::{Job, run};
use crate::{install::InstallArgs, run::Platform};

/// The architectures in the universal app, by target triple and by the name Apple's tools use.
const ARCHS: &[(&str, &str)] = &[
   ("x86_64-apple-darwin", "x86_64"),
   ("aarch64-apple-darwin", "arm64"),
];

// The executable finds the bundled dylibs through this rpath.
const FRAMEWORKS_RPATH: &str = "@executable_path/../Frameworks";

/// The universal app, zipped for the Steam depot.
pub(super) fn zip(job: &Job, output: &Path) -> Result<()> {
   let app = app(job)?;
   let out = output.join("naev-macos");
   fs::create_dir_all(&out).with_context(|| format!("creating {}", out.display()))?;
   let zip = out.join("naev-macos.zip");
   let base = app.parent().context("the app has a directory")?;
   crate::archive::zip(&zip, base, &["Naev.app"])?;
   println!("wrote {}", zip.display());
   Ok(())
}

/// Builds, bundles and merges every architecture into one signed Naev.app, once per run.
fn app(job: &Job) -> Result<PathBuf> {
   if let Some(app) = job.macos_app.get() {
      return Ok(app.clone());
   }
   if job.args.build.target().is_some() {
      bail!("the macOS app builds each of its architectures itself, so it takes no --target");
   }
   let work = job.target.join("macos");
   if work.exists() {
      fs::remove_dir_all(&work).with_context(|| format!("clearing {}", work.display()))?;
   }

   let mut thin = Vec::new();
   for (triple, arch) in ARCHS {
      let app = work.join(arch).join("Naev.app");
      let install =
         InstallArgs::game(None, app.clone(), job.args.build.for_target(triple)).app_bundle();
      crate::install::install(job.root, job.target, job.data_dir, job.metadata, &install)?;
      bundle_dylibs(&app, triple)?;
      thin.push(app);
   }

   let universal = work.join("universal/Naev.app");
   merge(&thin, &universal)?;

   let entitlements = job.root.join("extras/macos/entitlements.plist");
   // Ad-hoc, which Apple Silicon needs before it runs anything.
   let mut sign = Command::new("rcodesign");
   sign
      .arg("sign")
      .arg("-e")
      .arg(&entitlements)
      .arg(&universal);
   run(sign, "rcodesign sign")?;

   Ok(job.macos_app.get_or_init(|| universal).clone())
}

fn executable(app: &Path) -> PathBuf {
   app.join("Contents/MacOS")
      .join(Platform::MacOs.executable())
}

/// Copies every dylib the executable loads, other than the system's, into Contents/Frameworks,
/// and points each load at @rpath.
fn bundle_dylibs(app: &Path, triple: &str) -> Result<()> {
   let frameworks = app.join("Contents/Frameworks");
   fs::create_dir_all(&frameworks).with_context(|| format!("creating {}", frameworks.display()))?;
   let exe = executable(app);
   let sysroot = sysroot(triple);
   let install_name_tool = install_name_tool(triple);

   let mut bundled = BTreeSet::new();
   // Each file to rewrite, and the directory its @loader_path rpaths were written against.
   let mut pending = vec![(exe.clone(), exe.parent().map(Path::to_path_buf))];
   while let Some((file, loader)) = pending.pop() {
      let bytes = fs::read(&file).with_context(|| format!("reading {}", file.display()))?;
      let macho = MachO::parse(&bytes, 0).with_context(|| format!("parsing {}", file.display()))?;

      let mut rewrite = Command::new(&install_name_tool);
      if file == exe {
         rewrite.args(["-add_rpath", FRAMEWORKS_RPATH]);
      } else {
         rewrite
            .arg("-id")
            .arg(format!("@rpath/{}", file_name(&file)?));
      }
      // goblin lists the file itself first, as "self".
      for &lib in macho.libs.iter().skip(1) {
         if lib.starts_with("/usr/lib/") || lib.starts_with("/System/") {
            continue;
         }
         let source = locate(lib, &macho.rpaths, loader.as_deref(), sysroot.as_deref())
            .with_context(|| {
               format!(
                  "{} needs {lib}, which isn't on this machine",
                  file.display()
               )
            })?;
         let name = file_name(Path::new(lib))?;
         if bundled.insert(name.clone()) {
            let copy = frameworks.join(&name);
            fs::copy(&source, &copy).with_context(|| format!("copying {}", source.display()))?;
            // MacPorts installs its dylibs read-only.
            crate::install::set_mode(&copy, 0o644)?;
            pending.push((copy, source.parent().map(Path::to_path_buf)));
         }
         rewrite
            .arg("-change")
            .arg(lib)
            .arg(format!("@rpath/{name}"));
      }
      // Rpaths into the build machine mean nothing on a player's.
      for rpath in macho.rpaths.iter().filter(|rpath| !rpath.starts_with('@')) {
         rewrite.arg("-delete_rpath").arg(rpath);
      }
      rewrite.arg(&file);
      run(rewrite, "install_name_tool")?;
   }
   println!("bundled {} dylibs for {triple}", bundled.len());
   Ok(())
}

/// Where a load command's dylib is on this machine.
fn locate(
   lib: &str,
   rpaths: &[&str],
   loader: Option<&Path>,
   sysroot: Option<&Path>,
) -> Option<PathBuf> {
   let candidates: Vec<PathBuf> = if let Some(rest) = lib.strip_prefix("@rpath/") {
      rpaths
         .iter()
         .filter_map(|rpath| match rpath.strip_prefix("@loader_path") {
            Some(tail) => loader.map(|dir| dir.join(tail.trim_start_matches('/'))),
            None => Some(PathBuf::from(rpath)),
         })
         .map(|dir| dir.join(rest))
         .collect()
   } else if lib.starts_with('/') {
      // On a cross host the install names point into the target's sysroot.
      sysroot
         .map(|root| root.join(lib.trim_start_matches('/')))
         .into_iter()
         .chain([PathBuf::from(lib)])
         .collect()
   } else {
      Vec::new()
   };
   candidates.into_iter().find(|path| path.is_file())
}

/// Merges the thin bundles into one, with a universal executable and dylibs.
fn merge(thin: &[PathBuf], universal: &Path) -> Result<()> {
   let first = thin.first().context("there's at least one architecture")?;
   // Info.plist and the data are the same for every architecture.
   crate::install::copy_tree(first, universal, &[], &[])?;

   let exe = executable(Path::new(""));
   let mut machos = BTreeSet::from([exe.clone()]);
   for app in thin {
      let frameworks = app.join("Contents/Frameworks");
      for entry in
         fs::read_dir(&frameworks).with_context(|| format!("reading {}", frameworks.display()))?
      {
         let entry = entry.with_context(|| format!("reading {}", frameworks.display()))?;
         machos.insert(Path::new("Contents/Frameworks").join(entry.file_name()));
      }
   }
   for rel in &machos {
      let slices: Vec<PathBuf> = thin
         .iter()
         .map(|app| app.join(rel))
         .filter(|path| path.is_file())
         .collect();
      fat(&slices, &universal.join(rel))?;
   }
   crate::install::set_mode(&universal.join(&exe), 0o755)
}

/// Writes one file holding every slice, or a copy when only one architecture has it.
fn fat(slices: &[PathBuf], out: &Path) -> Result<()> {
   match slices {
      [] => bail!("no architecture has {}", out.display()),
      [only] => {
         fs::copy(only, out).with_context(|| format!("copying {}", only.display()))?;
      }
      _ => {
         let mut create = Command::new("lipo");
         create.args(slices).arg("-create").arg("-output").arg(out);
         run(create, "lipo")?;
      }
   }
   Ok(())
}

/// install_name_tool by the name the cross toolchain gives it, or its plain name on a Mac. lipo
/// handles every architecture under its plain name on both.
fn install_name_tool(triple: &str) -> String {
   // The compiler the cc crate builds with carries the toolchain's prefix, as in
   // x86_64-apple-darwin23-cc.
   env::var(format!("CC_{}", triple.replace('-', "_")))
      .ok()
      .and_then(|cc| {
         cc.rsplit_once('-')
            .map(|(prefix, _)| format!("{prefix}-install_name_tool"))
      })
      .unwrap_or_else(|| "install_name_tool".to_string())
}

/// The sysroot pkg-config found this target's libraries in, when cross-building.
fn sysroot(triple: &str) -> Option<PathBuf> {
   env::var_os(format!(
      "PKG_CONFIG_SYSROOT_DIR_{}",
      triple.replace('-', "_")
   ))
   .map(PathBuf::from)
}

fn file_name(path: &Path) -> Result<String> {
   Ok(path
      .file_name()
      .with_context(|| format!("{} has no file name", path.display()))?
      .to_string_lossy()
      .into_owned())
}
