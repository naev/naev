use std::{
   collections::{HashMap, HashSet},
   env, fs,
   path::{Path, PathBuf},
   process::Command,
};

#[derive(serde::Deserialize)]
struct Manifest {
   build: Build,
   system: Option<System>,
}

// Set for every library, or as OVERRIDE_PREFIX plus a library's name in capitals for one.
const MODE_VAR: &str = "NAEV_VENDOR";
const OVERRIDE_PREFIX: &str = "NAEV_VENDOR_";

/// How a build chooses between a vendored library and a system copy.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Mode {
   /// A system copy in the accepted versions if there is one, otherwise the vendored library.
   Auto,
   /// The vendored library, as the release packages ship.
   Always,
   /// A system copy, failing the build without one. Under NAEV_VENDOR, libraries with no
   /// stand-in still vendor.
   Never,
}

impl Mode {
   /// The mode an environment variable sets, if it's set.
   fn from_var(name: &str) -> Option<Self> {
      println!("cargo:rerun-if-env-changed={name}");
      let value = env::var(name).ok()?;
      Some(match value.as_str() {
         "auto" => Self::Auto,
         "always" => Self::Always,
         "never" => Self::Never,
         other => panic!("{name} is auto, always or never, not {other}"),
      })
   }
}

/// A system copy that can stand in for the vendored library.
#[derive(serde::Deserialize)]
#[serde(untagged)]
enum System {
   /// Found through pkg-config, from version min up to but not including below.
   PkgConfig {
      #[serde(rename = "pkg-config")]
      name: String,
      min: String,
      below: String,
   },
   /// For a library with no pkg-config file: its header, the preprocessor check its version
   /// macros have to pass, and the library to link.
   Header {
      header: String,
      check: String,
      link: String,
   },
}

impl System {
   /// The include dirs the system copy needs, or None without a suitable one.
   fn probe(&self, library: &str) -> Option<Vec<PathBuf>> {
      match self {
         System::PkgConfig { name, min, below } => pkg_config::Config::new()
            .range_version(min.as_str()..below.as_str())
            .probe(name)
            .ok()
            .map(|lib| lib.include_paths),
         System::Header {
            header,
            check,
            link,
         } => header_probe(library, header, check, link).then(|| {
            println!("cargo:rustc-link-lib={link}");
            Vec::new()
         }),
      }
   }
}

/// Compiles and links a program against the header and library, so a missing, unlinkable or
/// out-of-range copy fails it.
fn header_probe(library: &str, header: &str, check: &str, link: &str) -> bool {
   let out_dir = PathBuf::from(env::var("OUT_DIR").expect("cargo sets OUT_DIR"));
   let probe_dir = out_dir.join("vendor-probe");
   let src = probe_dir.join(format!("{library}.c"));
   let program = format!(
      "#include <{header}>\n#if !({check})\n#error out of range\n#endif\nint main(void) {{ return 0; }}\n"
   );
   if fs::create_dir_all(&probe_dir).is_err() || fs::write(&src, program).is_err() {
      return false;
   }
   let Ok(compiler) = cc::Build::new()
      .warnings(false)
      .cargo_metadata(false)
      .cargo_warnings(false)
      .try_get_compiler()
   else {
      return false;
   };
   Command::new(compiler.path())
      .args(compiler.args())
      .arg(&src)
      .arg(format!("-l{link}"))
      .arg("-o")
      .arg(probe_dir.join(library))
      .output()
      .is_ok_and(|out| out.status.success())
}

// The [build] table of a library's vendor.toml. [upstream] only records provenance.
#[derive(serde::Deserialize)]
struct Build {
   library: String,
   sources: Vec<PathBuf>,
   include: Vec<PathBuf>,
   #[serde(default)]
   defines: Vec<String>,
   std: Option<String>,
   // What the engine and bindgen see, defaulting to all of include.
   export: Option<Vec<PathBuf>>,
   // System libraries it needs, by target OS.
   #[serde(default)]
   link: HashMap<String, Vec<String>>,
}

/// Builds every library under vendor/, or links a system copy where NAEV_VENDOR allows one, and
/// returns the include dirs the engine needs for them.
///
/// Panics on a malformed vendor.toml, a mode variable set to anything but auto, always or never,
/// an override naming no vendored library, or a never with no system copy to use.
pub fn build_all(crate_dir: &Path) -> Vec<PathBuf> {
   let vendor = crate_dir.join("vendor");
   println!("cargo:rerun-if-changed={}", vendor.display());
   let mut libraries: Vec<PathBuf> = listed(&vendor)
      .into_iter()
      .filter(|path| path.is_dir())
      .collect();
   libraries.sort();
   let manifests: Vec<(PathBuf, Manifest)> = libraries
      .into_iter()
      .map(|dir| {
         let path = dir.join("vendor.toml");
         let text = fs::read_to_string(&path)
            .unwrap_or_else(|err| panic!("reading {}: {err}", path.display()));
         let manifest =
            toml::from_str(&text).unwrap_or_else(|err| panic!("parsing {}: {err}", path.display()));
         (dir, manifest)
      })
      .collect();

   // An override for a library that isn't vendored would otherwise be ignored without a word.
   let names: Vec<String> = manifests
      .iter()
      .map(|(_, manifest)| manifest.build.library.to_uppercase())
      .collect();
   let unknown: Vec<String> = env::vars_os()
      .filter_map(|(key, _)| key.into_string().ok())
      .filter(|key| {
         key.strip_prefix(OVERRIDE_PREFIX)
            .is_some_and(|name| !names.iter().any(|known| known == name))
      })
      .collect();
   if !unknown.is_empty() {
      panic!(
         "{} names no vendored library; the overrides are {}",
         unknown.join(", "),
         names
            .iter()
            .map(|name| format!("{OVERRIDE_PREFIX}{name}"))
            .collect::<Vec<_>>()
            .join(", ")
      );
   }

   let global = Mode::from_var(MODE_VAR).unwrap_or(Mode::Auto);
   manifests
      .into_iter()
      .flat_map(|(dir, manifest)| {
         let var = format!("{OVERRIDE_PREFIX}{}", manifest.build.library.to_uppercase());
         let chosen = Mode::from_var(&var);
         build(
            &dir,
            manifest,
            chosen.map_or((global, MODE_VAR), |mode| (mode, var.as_str())),
         )
      })
      .collect()
}

/// Links a system copy or builds the vendored library, as mode, set by the named variable, allows.
fn build(dir: &Path, manifest: Manifest, (mode, var): (Mode, &str)) -> Vec<PathBuf> {
   let build = manifest.build;
   let path = dir.join("vendor.toml");
   match (mode, &manifest.system) {
      (Mode::Always, _) => {}
      // A library with no stand-in still vendors under NAEV_VENDOR=never, but not when asked by name.
      (Mode::Never, None) if var != MODE_VAR => {
         panic!("{var}=never, but {} has no system stand-in", path.display())
      }
      (_, None) => {}
      (Mode::Auto | Mode::Never, Some(system)) => match system.probe(&build.library) {
         Some(include) => return include,
         None if mode == Mode::Never => panic!(
            "{var}=never, but no system {} matches the versions {} accepts",
            build.library,
            path.display()
         ),
         None => {}
      },
   }

   let in_dir = |dirs: &[PathBuf]| dirs.iter().map(|rel| dir.join(rel)).collect::<Vec<_>>();
   let mut cc = cc::Build::new();
   // Upstream's code, so its warnings aren't naev's to fix.
   cc.warnings(false);
   for define in &build.defines {
      cc.define(define, None);
   }
   if let Some(std) = &build.std {
      cc.std(std);
   }
   cc.includes(in_dir(&build.include))
      .files(compiled_sources(dir, &build.sources))
      .compile(&build.library);
   let target_os = std::env::var("CARGO_CFG_TARGET_OS").expect("cargo sets it for build scripts");
   for lib in build.link.get(&target_os).into_iter().flatten() {
      println!("cargo:rustc-link-lib={lib}");
   }
   in_dir(build.export.as_deref().unwrap_or(&build.include))
}

/// Every .c file in the source dirs except those another file includes, which build as part of it.
fn compiled_sources(dir: &Path, source_dirs: &[PathBuf]) -> Vec<PathBuf> {
   let files: Vec<PathBuf> = source_dirs
      .iter()
      .flat_map(|rel| listed(&dir.join(rel)))
      .filter(|path| path.is_file())
      .collect();
   let included: HashSet<String> = files.iter().flat_map(|file| included_c(file)).collect();

   let mut sources: Vec<PathBuf> = files
      .into_iter()
      .filter(|file| file.extension().is_some_and(|ext| ext == "c"))
      .filter(|file| !included.contains(&name(file)))
      .collect();
   sources.sort();
   sources
}

/// The file names of the .c files a source or header pulls in with #include "...".
fn included_c(file: &Path) -> Vec<String> {
   let bytes = fs::read(file).unwrap_or_else(|err| panic!("reading {}: {err}", file.display()));
   String::from_utf8_lossy(&bytes)
      .lines()
      .filter_map(|line| line.trim_start().strip_prefix('#'))
      .filter_map(|line| line.trim_start().strip_prefix("include"))
      .filter_map(|line| line.trim_start().strip_prefix('"'))
      .filter_map(|line| line.split('"').next())
      .filter(|target| target.ends_with(".c"))
      .map(|target| name(Path::new(target)))
      .collect()
}

fn name(path: &Path) -> String {
   path
      .file_name()
      .map(|name| name.to_string_lossy().into_owned())
      .unwrap_or_default()
}

fn listed(dir: &Path) -> Vec<PathBuf> {
   fs::read_dir(dir)
      .unwrap_or_else(|err| panic!("reading {}: {err}", dir.display()))
      .map(|entry| {
         entry
            .unwrap_or_else(|err| panic!("reading {}: {err}", dir.display()))
            .path()
      })
      .collect()
}
