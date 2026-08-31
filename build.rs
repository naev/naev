//! The Windows resource linked into the executable: the icon Explorer shows,
//! the version tab in its properties, and the manifest asking for per-monitor
//! DPI awareness. meson compiled this to an object and added it to the link;
//! cargo owns the executable now, so it is built here.

use std::{env, fs, path::PathBuf};

/// Carried in the version resource, as meson carried them.
const COPYRIGHT_YEAR: &str = "2026";
const COPYRIGHT_HOLDER: &str = "Naev Dev Team";

fn main() {
   println!("cargo::rerun-if-changed=extras/windows/resource.rc.in");
   println!("cargo::rerun-if-changed=extras/windows/naev.exe.manifest.in");
   println!("cargo::rerun-if-changed=extras/logos/logo.ico");

   if env::var("CARGO_CFG_TARGET_OS").as_deref() != Ok("windows") {
      return;
   }

   let root = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("cargo sets the manifest dir"));
   let out = PathBuf::from(env::var("OUT_DIR").expect("cargo sets the output directory"));

   let fill = |name: &str| {
      let path = root.join("extras/windows").join(name);
      fs::read_to_string(&path)
         .unwrap_or_else(|err| panic!("reading {}: {err}", path.display()))
         .replace("@VERSION@", env!("CARGO_PKG_VERSION"))
         .replace("@VMAJOR@", env!("CARGO_PKG_VERSION_MAJOR"))
         .replace("@VMINOR@", env!("CARGO_PKG_VERSION_MINOR"))
         .replace("@VREV@", env!("CARGO_PKG_VERSION_PATCH"))
         .replace("@YEAR@", COPYRIGHT_YEAR)
         .replace("@COPYRIGHT@", COPYRIGHT_HOLDER)
   };

   // resource.rc names the manifest and the icon relative to itself, so both
   // are staged beside it.
   let write = |name: &str, text: String| {
      let path = out.join(name);
      fs::write(&path, text).unwrap_or_else(|err| panic!("writing {}: {err}", path.display()));
      path
   };
   write("naev.exe.manifest", fill("naev.exe.manifest.in"));
   let icon = root.join("extras/logos/logo.ico");
   fs::copy(&icon, out.join("logo.ico"))
      .unwrap_or_else(|err| panic!("copying {}: {err}", icon.display()));
   let resource = write("resource.rc", fill("resource.rc.in"));

   embed_resource::compile(&resource, embed_resource::NONE)
      .manifest_required()
      .expect("the windows resource has to reach the executable");
}
