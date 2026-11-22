use std::{
   collections::BTreeMap,
   env, fs,
   path::{Path, PathBuf},
   process::Command,
};

use anyhow::{Context, Result, bail};
use goblin::pe::PE;

use super::{Job, run};
use crate::{
   install::InstallArgs,
   run::{BuildArgs, Platform},
};

// Windows provides these itself. The list is mingw-bundledlls' blacklist, less the C runtimes.
const SYSTEM: &[&str] = &[
   "advapi32.dll",
   "advpack.dll",
   "avicap32.dll",
   "avrt.dll",
   "bcrypt.dll",
   "bcryptprimitives.dll",
   "cfgmgr32.dll",
   "combase.dll",
   "comctl32.dll",
   "comdlg32.dll",
   "crypt32.dll",
   "d2d1.dll",
   "d3d10.dll",
   "d3d10_1.dll",
   "d3d11.dll",
   "d3d8.dll",
   "d3d9.dll",
   "dbghelp.dll",
   "dcomp.dll",
   "dnsapi.dll",
   "dsound.dll",
   "dwmapi.dll",
   "dwrite.dll",
   "dxcore.dll",
   "dxgi.dll",
   "gdi32.dll",
   "gdi32full.dll",
   "gdiplus.dll",
   "glu32.dll",
   "hid.dll",
   "imm32.dll",
   "iphlpapi.dll",
   "kernel32.dll",
   "kernelbase.dll",
   "ktmw32.dll",
   "mf.dll",
   "mfplat.dll",
   "mfreadwrite.dll",
   "mpr.dll",
   "msimg32.dll",
   "mswsock.dll",
   "ncrypt.dll",
   "netapi32.dll",
   "ntdll.dll",
   "ole32.dll",
   "oleaut32.dll",
   "opengl32.dll",
   "powrprof.dll",
   "propsys.dll",
   "psapi.dll",
   "rpcrt4.dll",
   "schannel.dll",
   "sechost.dll",
   "secur32.dll",
   "setupapi.dll",
   "shell32.dll",
   "shlwapi.dll",
   "user32.dll",
   "userenv.dll",
   "usp10.dll",
   "uxtheme.dll",
   "version.dll",
   "vulkan-1.dll",
   "wdmaud.drv",
   "win32u.dll",
   "winhttp.dll",
   "wininet.dll",
   "winmm.dll",
   "winspool.drv",
   "ws2_32.dll",
   "wsock32.dll",
   "wtsapi32.dll",
];

/// The C runtime a MinGW build links. Every DLL in a package has to share one.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
enum Runtime {
   Msvcrt,
   Ucrt,
}

impl std::fmt::Display for Runtime {
   fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
      f.write_str(match self {
         Self::Msvcrt => "msvcrt",
         Self::Ucrt => "UCRT",
      })
   }
}

impl Runtime {
   fn of(dll: &str) -> Option<Self> {
      match dll {
         "msvcrt.dll" => Some(Self::Msvcrt),
         "ucrtbase.dll" => Some(Self::Ucrt),
         _ if dll.starts_with("api-ms-win-crt-") => Some(Self::Ucrt),
         _ => None,
      }
   }
}

/// naev.exe and the DLLs it needs, for the Steam depot.
pub(super) fn tarball(job: &Job, output: &Path) -> Result<()> {
   let stage = stage(job)?;

   let mut files: Vec<String> = fs::read_dir(&stage)
      .with_context(|| format!("reading {}", stage.display()))?
      .map(|entry| entry.map(|entry| entry.file_name().to_string_lossy().into_owned()))
      .collect::<Result<_, _>>()
      .with_context(|| format!("reading {}", stage.display()))?;
   files.retain(|name| name.ends_with(".exe") || name.ends_with(".dll"));
   files.sort();

   let out = output.join("naev-win64");
   fs::create_dir_all(&out).with_context(|| format!("creating {}", out.display()))?;
   let tarball = out.join("naev-windows.tar.xz");
   crate::archive::tar_xz(&tarball, &stage, &files)?;
   println!("wrote {}", tarball.display());
   Ok(())
}

/// The NSIS installer, with the project metadata it shows passed in from Cargo.toml.
pub(super) fn installer(job: &Job, output: &Path) -> Result<()> {
   let stage = stage(job)?;
   let root = job.root;

   // naev.nsi expects its assets, the licences and the icon beside it, and the game under bin/.
   let work = job.target.join("nsis");
   if work.exists() {
      fs::remove_dir_all(&work).with_context(|| format!("clearing {}", work.display()))?;
   }
   crate::install::copy_tree(
      &root.join("extras/windows/installer_assets"),
      &work,
      &[],
      &[],
   )?;
   for (from, to) in [
      (root.join("gpl.txt"), "legal/gpl-3.0.txt"),
      (root.join("LICENSE"), "legal/naev-license.txt"),
      (stage.join("logo.ico"), "logo.ico"),
   ] {
      fs::copy(&from, work.join(to)).with_context(|| format!("copying {}", from.display()))?;
   }
   crate::install::copy_tree(&stage, &work.join("bin"), &[], &[])?;

   let out = output.join("naev-win64");
   fs::create_dir_all(&out).with_context(|| format!("creating {}", out.display()))?;
   let exe = out.join("naev-installer.exe");
   let metadata = job.metadata;
   let mut makensis = Command::new("makensis");
   makensis
      .arg(format!("-XOutFile {}", exe.display()))
      .arg(format!("-DVERSION={}", crate::version::current(root)?))
      .arg(format!("-DNAME={}", metadata.name))
      .arg(format!("-DPUBLISHER={}", metadata.copyright_holder))
      .arg(format!("-DURL={}", metadata.homepage))
      .arg(work.join("naev.nsi"));
   run(makensis, "makensis")?;
   println!("wrote {}", exe.display());
   Ok(())
}

/// Installs the game flat into a fresh directory, with the DLLs it needs beside it, once per run.
fn stage(job: &Job) -> Result<PathBuf> {
   if let Some(stage) = job.windows_stage.get() {
      return Ok(stage.clone());
   }
   let build = &job.args.build;
   if build.platform() != Platform::Windows {
      bail!("the Windows packages need a Windows build; pass --target");
   }
   let stage = job.target.join("windows");
   if stage.exists() {
      fs::remove_dir_all(&stage).with_context(|| format!("clearing {}", stage.display()))?;
   }
   let install = InstallArgs::game(None, stage.clone(), build.clone()).flat();
   crate::install::install(job.root, job.target, job.data_dir, job.metadata, &install)?;

   let exe = stage.join(Platform::Windows.executable());
   let (dlls, runtime) = bundled(&exe, &search_path(build)?)?;
   for dll in &dlls {
      let name = dll.file_name().context("a DLL has a file name")?;
      fs::copy(dll, stage.join(name)).with_context(|| format!("copying {}", dll.display()))?;
   }
   println!("bundled {} DLLs, all on {runtime}", dlls.len());
   Ok(job.windows_stage.get_or_init(|| stage).clone())
}

/// Every DLL the executable loads that Windows doesn't provide, and the C runtime they share.
fn bundled(exe: &Path, search: &[PathBuf]) -> Result<(Vec<PathBuf>, Runtime)> {
   let mut found: BTreeMap<String, PathBuf> = BTreeMap::new();
   let mut runtimes: BTreeMap<Runtime, Vec<String>> = BTreeMap::new();
   let mut pending = vec![exe.to_path_buf()];
   while let Some(file) = pending.pop() {
      let bytes = fs::read(&file).with_context(|| format!("reading {}", file.display()))?;
      let pe = PE::parse(&bytes).with_context(|| format!("parsing {}", file.display()))?;
      let name = file
         .file_name()
         .map(|name| name.to_string_lossy().into_owned());
      for &imported in &pe.libraries {
         // Windows matches DLL names without case, so the lookup and the lists do too.
         let import = imported.to_ascii_lowercase();
         if let Some(runtime) = Runtime::of(&import) {
            runtimes.entry(runtime).or_default().extend(name.clone());
            continue;
         }
         // API sets resolve inside Windows itself.
         if SYSTEM.contains(&import.as_str())
            || import.starts_with("api-ms-win-")
            || found.contains_key(&import)
         {
            continue;
         }
         let path = search
            .iter()
            .flat_map(|dir| [dir.join(imported), dir.join(&import)])
            .find(|path| path.is_file())
            .with_context(|| {
               format!(
                  "{} needs {import}, which isn't in the search path or the system DLL list",
                  file.display()
               )
            })?;
         pending.push(path.clone());
         found.insert(import, path);
      }
   }

   let runtime = shared_runtime(exe, &runtimes)?;
   Ok((found.into_values().collect(), runtime))
}

/// The one C runtime every file in the package links, by the files that link each.
fn shared_runtime(exe: &Path, runtimes: &BTreeMap<Runtime, Vec<String>>) -> Result<Runtime> {
   let mut used = runtimes.keys().copied();
   let runtime = used
      .next()
      .with_context(|| format!("{} links no C runtime", exe.display()))?;
   if used.next().is_some() {
      let mixed: Vec<String> = runtimes
         .iter()
         .map(|(runtime, users)| format!("{runtime}: {}", users.join(", ")))
         .collect();
      bail!("the package mixes C runtimes\n{}", mixed.join("\n"));
   }
   Ok(runtime)
}

/// Where the MinGW DLLs live: the cross sysroot, or PATH on a native build.
fn search_path(build: &BuildArgs) -> Result<Vec<PathBuf>> {
   if cfg!(windows) {
      let path = env::var_os("PATH").context("PATH isn't set")?;
      return Ok(env::split_paths(&path).collect());
   }
   let triple = build
      .target()
      .context("cross-building for Windows needs --target")?;
   let arch = triple.split('-').next().unwrap_or(triple);
   // The same compiler the cc crate picks for this target.
   let compiler = env::var(format!("CC_{}", triple.replace('-', "_")))
      .or_else(|_| env::var(format!("CC_{triple}")))
      .unwrap_or_else(|_| format!("{arch}-w64-mingw32-gcc"));
   let out = Command::new(&compiler)
      .arg("-print-sysroot")
      .output()
      .with_context(|| format!("failed to run {compiler}"))?;
   let sysroot = String::from_utf8_lossy(&out.stdout).trim().to_string();
   if !out.status.success() || sysroot.is_empty() {
      bail!("{compiler} reports no sysroot to find the MinGW DLLs in");
   }
   Ok(vec![Path::new(&sysroot).join("mingw/bin")])
}

#[cfg(test)]
mod tests {
   use super::*;

   #[test]
   fn api_sets_count_as_ucrt() {
      assert_eq!(
         Runtime::of("api-ms-win-crt-runtime-l1-1-0.dll"),
         Some(Runtime::Ucrt)
      );
      assert_eq!(Runtime::of("msvcrt.dll"), Some(Runtime::Msvcrt));
      assert_eq!(Runtime::of("kernel32.dll"), None);
   }

   #[test]
   fn one_runtime_is_shared() {
      let runtimes = BTreeMap::from([(Runtime::Msvcrt, vec!["naev.exe".to_string()])]);
      let runtime = shared_runtime(Path::new("naev.exe"), &runtimes).expect("one runtime");
      assert_eq!(runtime, Runtime::Msvcrt);
   }

   #[test]
   fn mixed_runtimes_are_refused() {
      let runtimes = BTreeMap::from([
         (Runtime::Msvcrt, vec!["naev.exe".to_string()]),
         (Runtime::Ucrt, vec!["SDL3.dll".to_string()]),
      ]);
      let err = shared_runtime(Path::new("naev.exe"), &runtimes).expect_err("mixed runtimes");
      assert!(err.to_string().contains("mixes C runtimes"));
   }

   #[test]
   fn no_runtime_is_refused() {
      assert!(shared_runtime(Path::new("naev.exe"), &BTreeMap::new()).is_err());
   }
}
