use std::{
   collections::HashMap,
   fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};

use super::{Job, run};
use crate::{install::InstallArgs, run::Platform};

/// A tool the AppImage is built with, pinned to a release and to each architecture's checksum.
struct Tool {
   name: &'static str,
   url: &'static str,
   sha256: &'static [(&'static str, &'static str)],
}

const LINUXDEPLOY: Tool = Tool {
   name: "linuxdeploy",
   url: "https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20250213-2/linuxdeploy-{arch}.AppImage",
   sha256: &[
      (
         "x86_64",
         "4648f278ab3ef31f819e67c30d50f462640e5365a77637d7e6f2ad9fd0b4522a",
      ),
      (
         "aarch64",
         "06706ac8189797dccd36bd384105892cb5e6e71f784f4df526cc958adc223cd6",
      ),
   ],
};

const APPIMAGETOOL: Tool = Tool {
   name: "appimagetool",
   url: "https://github.com/AppImage/appimagetool/releases/download/1.9.0/appimagetool-{arch}.AppImage",
   sha256: &[
      (
         "x86_64",
         "46fdd785094c7f6e545b61afcfb0f3d98d8eab243f644b4b17698c01d06083d1",
      ),
      (
         "aarch64",
         "04f45ea45b5aa07bb2b071aed9dbf7a5185d3953b11b47358c1311f11ea94a96",
      ),
   ],
};

// Steam's runtime provides everything else the game links.
const STEAM_LIBS: &[&str] = &["libopenal.so.1", "libluajit-5.1.so.2"];

/// Installs into an AppDir under /usr and packs it with linuxdeploy and appimagetool.
pub(super) fn appimage(job: &Job, output: &Path) -> Result<()> {
   let args = job.args;
   if args.build.platform() != Platform::FreeDesktop {
      bail!("the AppImage only packages a Linux build");
   }
   let arch = std::env::consts::ARCH;
   // The release jobs name the architecture with a dash.
   let suffix = arch.replace('_', "-");

   let work = job.target.join("appimage");
   let appdir = work.join("AppDir");
   // A previous AppDir would carry files this build no longer installs.
   if appdir.exists() {
      fs::remove_dir_all(&appdir).with_context(|| format!("clearing {}", appdir.display()))?;
   }
   let install = InstallArgs::game(Some(appdir.clone()), "/usr".into(), args.build.clone());
   crate::install::install(job.root, job.target, job.data_dir, job.metadata, &install)?;

   // appimagetool only reads the older appdata name.
   let app_id = &job.metadata.app_id;
   let metainfo = appdir.join("usr/share/metainfo");
   fs::rename(
      metainfo.join(format!("{app_id}.metainfo.xml")),
      metainfo.join(format!("{app_id}.appdata.xml")),
   )
   .context("renaming the metainfo file")?;

   let tools = job.target.join("appimage-tools");
   let linuxdeploy = fetch(&LINUXDEPLOY, &tools, arch)?;
   let appimagetool = fetch(&APPIMAGETOOL, &tools, arch)?;

   let mut deploy = Command::new(&linuxdeploy);
   deploy.arg("--appdir").arg(&appdir).current_dir(&work);
   run(deploy, "linuxdeploy")?;

   let version = crate::version::current(job.root)?;
   let out = output.join(format!("naev-linux-{suffix}"));
   fs::create_dir_all(&out).with_context(|| format!("creating {}", out.display()))?;
   let image = out.join(format!("naev-{version}-linux-{suffix}.AppImage"));
   let tag = if args.nightly { "nightly" } else { "latest" };

   let mut pack = Command::new(&appimagetool);
   if args.no_compression {
      pack.args(["--mksquashfs-opt", "-no-compression"]);
   } else {
      pack.args([
         "--comp",
         "zstd",
         "--mksquashfs-opt",
         "-Xcompression-level",
         "--mksquashfs-opt",
         "20",
      ]);
   }
   // The zsync file lands in the working directory, beside the AppImage.
   pack
      .args(["--no-appstream", "-v", "-u"])
      .arg(format!(
         "zsync|{}/releases/download/{tag}/naev-*.AppImage.zsync",
         job.metadata.repository
      ))
      .arg(&appdir)
      .arg(&image)
      .current_dir(&out)
      .env("VERSION", &version);
   run(pack, "appimagetool")?;

   println!("wrote {}", image.display());
   Ok(())
}

/// The binary, the libraries Steam's runtime lacks and the data, each in its own handoff directory.
pub(super) fn steam(job: &Job, output: &Path) -> Result<()> {
   if job.args.build.platform() != Platform::FreeDesktop {
      bail!("the Steam Linux depot only packages a Linux build");
   }

   let stage = job.target.join("steam");
   if stage.exists() {
      fs::remove_dir_all(&stage).with_context(|| format!("clearing {}", stage.display()))?;
   }
   let install = InstallArgs::game(Some(stage.clone()), "/usr".into(), job.args.build.clone());
   crate::install::install(job.root, job.target, job.data_dir, job.metadata, &install)?;
   let binary = stage.join("usr/bin/naev");

   let bin_dir = output.join("naev-steamruntime");
   fs::create_dir_all(&bin_dir).with_context(|| format!("creating {}", bin_dir.display()))?;
   let shipped = bin_dir.join("naev.x64");
   fs::copy(&binary, &shipped).with_context(|| format!("copying {}", binary.display()))?;
   crate::install::set_mode(&shipped, 0o755)?;

   let libs_dir = output.join("naev-linux-libs");
   fs::create_dir_all(&libs_dir).with_context(|| format!("creating {}", libs_dir.display()))?;
   let linked = linked(&binary)?;
   for name in STEAM_LIBS {
      let path = linked
         .get(*name)
         .with_context(|| format!("{} doesn't link {name}", binary.display()))?;
      let copied = libs_dir.join(name);
      fs::copy(path, &copied).with_context(|| format!("copying {}", path.display()))?;
      // As linuxdeploy ships them. The runtime image builds OpenAL with debug info.
      let mut strip = Command::new("strip");
      strip.arg(&copied);
      run(strip, "strip")?;
   }

   let ndata_dir = output.join("naev-ndata");
   fs::create_dir_all(&ndata_dir).with_context(|| format!("creating {}", ndata_dir.display()))?;
   let ndata = ndata_dir.join("naev-ndata.tar.xz");
   crate::archive::tar_xz(&ndata, &stage.join("usr/share/naev"), &["dat"])?;

   println!(
      "wrote {}, {} and {}",
      shipped.display(),
      libs_dir.display(),
      ndata.display()
   );
   Ok(())
}

/// The shared libraries the binary resolves to, by soname, as ldd reports them.
fn linked(binary: &Path) -> Result<HashMap<String, PathBuf>> {
   let out = Command::new("ldd")
      .arg(binary)
      .output()
      .context("failed to run ldd")?;
   if !out.status.success() {
      bail!("ldd failed with {}", out.status);
   }
   Ok(String::from_utf8_lossy(&out.stdout)
      .lines()
      .filter_map(|line| {
         let (name, rest) = line.trim().split_once(" => ")?;
         let path = rest.split_whitespace().next()?;
         path
            .starts_with('/')
            .then(|| (name.to_string(), PathBuf::from(path)))
      })
      .collect())
}

/// Downloads and checks a tool the first time it's needed.
fn fetch(tool: &Tool, dir: &Path, arch: &str) -> Result<PathBuf> {
   let path = dir.join(tool.name);
   if path.is_file() {
      return Ok(path);
   }
   let expected = tool
      .sha256
      .iter()
      .find(|(pinned, _)| *pinned == arch)
      .map(|(_, sum)| *sum)
      .with_context(|| format!("{} has no pinned checksum for {arch}", tool.name))?;
   fs::create_dir_all(dir).with_context(|| format!("creating {}", dir.display()))?;

   let partial = dir.join(format!("{}.part", tool.name));
   let mut curl = Command::new("curl");
   curl
      .args(["-L", "-f", "-s", "-S", "-o"])
      .arg(&partial)
      .arg(tool.url.replace("{arch}", arch));
   run(curl, &format!("downloading {}", tool.name))?;
   let sum = crate::archive::sha256(&partial)?;
   if sum != expected {
      bail!(
         "{} has checksum {sum}, not the pinned {expected}",
         tool.name
      );
   }

   // Zeroing the AppImage magic in the ELF header lets the tools run inside containers.
   // https://github.com/AppImage/AppImageKit/issues/828
   let mut bytes = fs::read(&partial).with_context(|| format!("reading {}", partial.display()))?;
   if let Some(at) = bytes.windows(3).position(|window| window == b"AI\x02") {
      bytes[at..at + 3].fill(0);
   }
   fs::write(&partial, bytes).with_context(|| format!("writing {}", partial.display()))?;
   crate::install::set_mode(&partial, 0o755)?;
   fs::rename(&partial, &path).with_context(|| format!("renaming {}", partial.display()))?;
   Ok(path)
}
