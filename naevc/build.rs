use std::{env, fs, path::PathBuf, process::Command};

#[path = "build/headers.rs"]
mod headers;

const INCLUDE_DIRS: &[&str] = &["src", "src/tk", "src/tk/widget"];

// Handle issues with bindgen and the math.h header.
const BLOCKLIST: &[&str] = &[
   "__mingw_ldbl_type_t",
   "FP_INT_.*",
   "FP_SUBNORMAL*",
   "FP_NORMAL*",
   "FP_NAN*",
   "FP_INFINITE*",
   "FP_ZERO*",
   // clang 22's amxintrin.h marks the AMX tile registers as both packed and
   // 64-byte aligned, which bindgen has no way to express together.
   ".*tile1024i.*",
];

// These can longjmp out through Lua.
const UNWIND_FNS: &[&str] = &[
   "nlua_package_.*",
   "cli_print",
   "cli_printRaw",
   "cli_warn",
   "luaopen_utf8",
   "luaopen_cmark",
   "luaopen_enet",
];

fn main() {
   let manifest_dir =
      PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("cargo always sets CARGO_MANIFEST_DIR"));
   let root = manifest_dir
      .parent()
      .expect("the naevc crate always lives one level below the repo root")
      .to_path_buf();
   let out_dir =
      PathBuf::from(env::var("OUT_DIR").expect("cargo always sets OUT_DIR for build scripts"));

   write_config(&out_dir);
   generate_tables(&root, &out_dir);
   generate_bindings(&root, &out_dir);
}

// The headers include the generated tables, so this runs before bindgen.
fn generate_tables(root: &std::path::Path, out_dir: &std::path::Path) {
   for script in ["src/shaders_c_gen.py", "src/colours_c_gen.py"] {
      let path = root.join(script);
      let status = Command::new("python3")
         .arg(&path)
         .current_dir(out_dir)
         .status()
         .unwrap_or_else(|e| panic!("failed to run python3 for {script}: {e}"));
      assert!(status.success(), "{script} failed with {status}");
      println!("cargo:rerun-if-changed={}", path.display());
   }
}

fn write_config(out_dir: &std::path::Path) {
   // Keep meson's spelling of the platform.
   let os = match env::var("CARGO_CFG_TARGET_OS").as_deref() {
      Ok("macos") => "darwin".to_string(),
      Ok(other) => other.to_string(),
      Err(_) => panic!("cargo always sets CARGO_CFG_TARGET_OS"),
   };
   let arch = env::var("CARGO_CFG_TARGET_ARCH").expect("cargo always sets CARGO_CFG_TARGET_ARCH");

   // Used for save and plugin compatibility.
   let version = env::var("NAEV_VERSION")
      .expect("NAEV_VERSION must be set to the engine version by the outer build");

   let pkgdatadir =
      env::var("NAEV_PKGDATADIR").unwrap_or_else(|_| "/usr/local/share/naev".to_string());
   let debug = env::var("DEBUG").is_ok_and(|d| d != "false" && d != "0");
   let paranoid = env::var_os("CARGO_FEATURE_PARANOID").is_some();

   let config = format!(
      "pub const HOST: &str = \"{os}-{arch}\";\n\
       pub const PACKAGE: &str = \"naev\";\n\
       pub const PACKAGE_NAME: &str = \"naev\";\n\
       pub const PACKAGE_VERSION: &str = \"{version}\";\n\
       pub const PKGDATADIR: &str = \"{pkgdatadir}\";\n\
       \n\
       pub const DEBUG: bool = {debug};\n\
       pub const DEBUG_PARANOID: bool = {paranoid};\n"
   );
   fs::write(out_dir.join("config.rs"), config).expect("OUT_DIR is writable");

   println!("cargo:rerun-if-env-changed=NAEV_PKGDATADIR");
   println!("cargo:rerun-if-env-changed=NAEV_VERSION");
}

fn generate_bindings(root: &std::path::Path, out_dir: &std::path::Path) {
   // bindgen wants a single translation unit.
   let merged: String = headers::HEADERS
      .iter()
      .map(|h| format!("#include \"{}\"\n", root.join(h).display()))
      .collect();
   let merged_path = out_dir.join("naevc.h");
   fs::write(&merged_path, merged).expect("OUT_DIR is writable");

   let mut builder = bindgen::Builder::default()
      .header(merged_path.to_string_lossy())
      // https://github.com/rust-lang/rust-bindgen/issues/3147
      .wrap_unsafe_ops(true);

   for dir in INCLUDE_DIRS {
      builder = builder.clang_arg(format!("-I{}", root.join(dir).display()));
   }
   // shaders.gen.h and colours.gen.h land here.
   builder = builder.clang_arg(format!("-I{}", out_dir.display()));
   for dir in probe_include_dirs() {
      builder = builder.clang_arg(format!("-I{}", dir.display()));
   }
   for &item in BLOCKLIST {
      builder = builder.blocklist_item(item);
   }
   for &func in UNWIND_FNS {
      builder = builder.override_abi(bindgen::Abi::CUnwind, func);
   }

   builder
      .generate()
      .expect("bindgen failed to parse the Naev headers")
      .write_to_file(out_dir.join("naevc.rs"))
      .expect("OUT_DIR is writable");

   for header in headers::HEADERS.iter().chain(headers::UNPARSED_HEADERS) {
      println!("cargo:rerun-if-changed={}", root.join(header).display());
   }
   println!("cargo:rerun-if-changed=build/headers.rs");
}

// Linking stays with meson for now.
fn probe_include_dirs() -> Vec<PathBuf> {
   let mut dirs = Vec::new();

   let sdl = pkg_config::Config::new()
      .cargo_metadata(false)
      .atleast_version("3.2.0")
      .probe("sdl3")
      .expect("SDL3 >= 3.2.0 not found; install the SDL3 development package");
   dirs.extend(sdl.include_paths);

   // meson prefers LuaJIT and falls back to plain Lua 5.1.
   let lua = ["luajit", "lua51", "lua5.1", "lua"]
      .iter()
      .find_map(|name| probe(name))
      .expect("neither LuaJIT nor Lua 5.1 found; install one of their development packages");
   dirs.extend(lua);

   // These can lack a pkg-config file, like libxml2 in the macOS SDK.
   for name in ["libxml-2.0", "physfs", "openal"] {
      dirs.extend(probe(name).unwrap_or_default());
   }

   dirs
}

fn probe(name: &str) -> Option<Vec<PathBuf>> {
   pkg_config::Config::new()
      .cargo_metadata(false)
      .probe(name)
      .ok()
      .map(|lib| lib.include_paths)
}
