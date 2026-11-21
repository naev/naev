use std::path::PathBuf;

// In link order, since a static archive only resolves against what follows it.
const LINK_ONLY: &[&str] = &[
   "glpk",
   "cholmod",
   "amd",
   "camd",
   "colamd",
   "suitesparseconfig",
];

const LINK_ONLY_OPTIONAL: &[&str] = &["ccolamd", "lapack", "metis"];

pub struct Deps {
   pub include_paths: Vec<PathBuf>,
   pub have_luajit: bool,
   pub have_tracy: bool,
}

pub fn probe() -> Deps {
   let mut include_paths = Vec::new();

   for (name, version) in [
      ("sdl3", "3.2.0"),
      ("libenet", "1.3"),
      ("libcmark", "0.31.0"),
   ] {
      include_paths.extend(required(name, Some(version)));
   }
   // libunibreak removed the incremental API we use in 0.8.0
   // https://github.com/adah1972/libunibreak/issues/46
   include_paths.extend(required_range("libunibreak", "4.0", "8.0"));
   // Some MSYS2 environments don't pull in ogg through vorbis.
   for name in ["opus", "freetype2", "openal", "vorbis", "vorbisfile", "ogg"] {
      include_paths.extend(required(name, None));
   }

   // Probing emits link flags, so stop at the first Lua found.
   let luajit = optional("luajit");
   let have_luajit = luajit.is_some();
   let lua = luajit
      .or_else(|| {
         ["lua51", "lua5.1", "lua"]
            .iter()
            .find_map(|name| optional(name))
      })
      .unwrap_or_else(|| missing("luajit or lua5.1", "LuaJIT (preferred) or Lua 5.1"));
   include_paths.extend(lua);

   // These can lack a pkg-config file, like libxml2 in the macOS SDK.
   for (package, library) in [("libxml-2.0", "xml2"), ("physfs", "physfs")] {
      match optional(package) {
         Some(paths) => include_paths.extend(paths),
         None if has_library(library) => link_lib(library),
         None => missing(library, package),
      }
   }

   for name in LINK_ONLY {
      link_lib(name);
   }
   for name in LINK_ONLY_OPTIONAL {
      if has_library(name) {
         link_lib(name);
      }
   }
   link_csparse();
   link_blas();
   link_platform();

   let have_tracy = probe_tracy(&mut include_paths);

   Deps {
      include_paths,
      have_luajit,
      have_tracy,
   }
}

/// c[x]sparse is packaged under either name depending on the distribution.
fn link_csparse() {
   for name in ["cxsparse", "csparse"] {
      if has_library(name) {
         link_lib(name);
         return;
      }
   }
   missing(
      "cxsparse or csparse",
      "SuiteSparse (CXSparse or CSparse component)",
   );
}

fn link_blas() {
   let blas = std::env::var("NAEV_BLAS").unwrap_or_else(|_| "openblas".to_string());
   println!("cargo:rerun-if-env-changed=NAEV_BLAS");

   if blas == "Accelerate" {
      println!("cargo:rustc-link-lib=framework=Accelerate");
   } else {
      if !has_library(&blas) {
         missing(&blas, "a BLAS implementation, or set NAEV_BLAS");
      }
      link_lib(&blas);
   }
}

fn link_lib(name: &str) {
   println!("cargo:rustc-link-lib={name}");
}

fn link_platform() {
   let target_os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
   match target_os.as_str() {
      "macos" => {
         println!("cargo:rustc-link-lib=framework=Foundation");
      }
      "windows" => {
         // GLAD needs dlopen; dlfcn-win32 provides it on Windows.
         println!("cargo:rustc-link-lib=dl");
      }
      _ => {
         println!("cargo:rustc-link-lib=m");
         println!("cargo:rustc-link-lib=dl");
      }
   }
}

fn probe_tracy(include_paths: &mut Vec<PathBuf>) -> bool {
   if std::env::var_os("CARGO_FEATURE_TRACY").is_none() {
      return false;
   }
   match optional("tracy") {
      Some(paths) => {
         include_paths.extend(paths);
         true
      }
      None => missing("tracy", "the tracy profiler"),
   }
}

fn required(name: &str, version: Option<&str>) -> Vec<PathBuf> {
   let mut cfg = pkg_config::Config::new();
   if let Some(version) = version {
      cfg.atleast_version(version);
   }
   match cfg.probe(name) {
      Ok(lib) => lib.include_paths,
      Err(_) => {
         let what = match version {
            Some(v) => format!("{name} >= {v}"),
            None => name.to_string(),
         };
         missing(name, &what)
      }
   }
}

fn required_range(name: &str, min: &str, below: &str) -> Vec<PathBuf> {
   let mut cfg = pkg_config::Config::new();
   cfg.range_version(min..below);
   match cfg.probe(name) {
      Ok(lib) => lib.include_paths,
      Err(_) => missing(name, &format!("{name} >= {min}, < {below}")),
   }
}

fn optional(name: &str) -> Option<Vec<PathBuf>> {
   pkg_config::Config::new()
      .probe(name)
      .ok()
      .map(|lib| lib.include_paths)
}

/// Drives the compiler by hand, since `cc::Build` has no link step.
fn has_library(name: &str) -> bool {
   let out_dir = PathBuf::from(std::env::var("OUT_DIR").expect("cargo sets OUT_DIR"));
   let probe_dir = out_dir.join("libprobe");
   let _ = std::fs::create_dir_all(&probe_dir);
   let src = probe_dir.join(format!("probe_{name}.c"));
   if std::fs::write(&src, "int main(void) { return 0; }\n").is_err() {
      return false;
   }

   let mut cc = cc::Build::new();
   cc.file(&src)
      .out_dir(&probe_dir)
      .warnings(false)
      .cargo_metadata(false)
      .cargo_warnings(false)
      .cargo_output(false);
   let Ok(compiler) = cc.try_get_compiler() else {
      return false;
   };

   let exe = probe_dir.join(format!("probe_{name}"));
   std::process::Command::new(compiler.path())
      .args(compiler.args())
      .arg(&src)
      .arg(format!("-l{name}"))
      .arg("-o")
      .arg(&exe)
      .output()
      .map(|out| out.status.success())
      .unwrap_or(false)
}

fn missing(name: &str, what: &str) -> ! {
   panic!("could not find {what}. Install the {name} development package.")
}
