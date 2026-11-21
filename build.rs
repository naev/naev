use std::{env, fs, path::PathBuf};

#[path = "extras/metadata.rs"]
mod metadata;

fn main() {
   println!("cargo::rerun-if-changed=Cargo.toml");
   println!("cargo::rerun-if-changed=extras/metadata.rs");
   println!("cargo::rerun-if-env-changed=NAEV_BUILD_VERSION");
   println!("cargo::rerun-if-changed=extras/windows/resource.rc.in");
   println!("cargo::rerun-if-changed=extras/windows/naev.exe.manifest.in");
   println!("cargo::rerun-if-changed=extras/logos/logo.ico");

   if env::var("CARGO_CFG_TARGET_OS").as_deref() != Ok("windows") {
      return;
   }

   let root = PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("cargo sets the manifest dir"));
   let out = PathBuf::from(env::var("OUT_DIR").expect("cargo sets the output directory"));

   let metadata = metadata::Metadata::read(&root).unwrap_or_else(|err| panic!("{err}"));
   // xtask and meson stamp builds with git's description of them.
   let version = env::var("NAEV_BUILD_VERSION").unwrap_or_else(|_| metadata.version.clone());
   let fill = |name: &str| {
      let path = root.join("extras/windows").join(name);
      let template = fs::read_to_string(&path)
         .unwrap_or_else(|err| panic!("reading {}: {err}", path.display()));
      metadata.fill(&template, &version)
   };

   // resource.rc names the manifest and icon relative to itself, so they're staged beside it.
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
