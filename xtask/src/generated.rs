//! Outfits derived from an existing outfit by a script, mirroring the table
//! the old `dat/outfits/generated/meson.build` carried.

/// One derived outfit.
pub struct Derived {
   /// Script in `dat/outfits/generated`.
   pub script: &'static str,
   /// Outfit it reads, relative to `dat/outfits`.
   pub input: &'static str,
   /// File it writes.
   pub output: &'static str,
}

pub const DERIVED: &[Derived] = &[
   Derived {
      script: "neutralizer.py",
      input: "weapons/heavy_ion_cannon.xml",
      output: "neutralizer.xml",
   },
   Derived {
      script: "reaver.py",
      input: "weapons/heavy_ripper_cannon.xml",
      output: "reaver_cannon.xml",
   },
   Derived {
      script: "corsair_systems.py",
      input: "core_system/medium/unicorp_pt200_core_system.xml",
      output: "corsair_systems.xml",
   },
   Derived {
      script: "corsair_hull.py",
      input: "core_hull/medium/nexus_ghost_weave.xml",
      output: "corsair_hull_plating.xml",
   },
   Derived {
      script: "corsair_engine.py",
      input: "core_engine/medium/nexus_arrow_700_engine.xml",
      output: "corsair_engine.xml",
   },
   Derived {
      script: "junker_hull.py",
      input: "core_hull/small/sk_small_cargo_hull.xml",
      output: "junker_plates.xml",
   },
];
