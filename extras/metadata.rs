//! The project metadata in the root Cargo.toml, and the templates under extras/ that carry it.
//! The root build script and xtask both include this file.

use std::{fs, path::Path};

/// `[workspace.package]` and `[workspace.metadata.naev]` from the root Cargo.toml.
#[derive(Clone, Debug)]
pub struct Metadata {
   pub version: String,
   pub description: String,
   pub homepage: String,
   pub repository: String,
   pub name: String,
   pub app_id: String,
   pub copyright_first_year: i64,
   pub copyright_year: i64,
   pub copyright_holder: String,
}

impl Metadata {
   /// Reads root/Cargo.toml. Fails naming the file, or the first key that's missing or mistyped.
   pub fn read(root: &Path) -> Result<Self, String> {
      let path = root.join("Cargo.toml");
      let manifest: toml::Table = fs::read_to_string(&path)
         .map_err(|err| format!("reading {}: {err}", path.display()))?
         .parse()
         .map_err(|err| format!("parsing {}: {err}", path.display()))?;
      let workspace = manifest
         .get("workspace")
         .ok_or("Cargo.toml has no [workspace] table")?;
      let package = Table::new(Some(workspace), "package", "[workspace.package]")?;
      let naev = Table::new(
         workspace.get("metadata"),
         "naev",
         "[workspace.metadata.naev]",
      )?;
      Ok(Self {
         version: package.string("version")?,
         description: package.string("description")?,
         homepage: package.string("homepage")?,
         repository: package.string("repository")?,
         name: naev.string("name")?,
         app_id: naev.string("app-id")?,
         copyright_first_year: naev.integer("copyright-first-year")?,
         copyright_year: naev.integer("copyright-year")?,
         copyright_holder: naev.string("copyright-holder")?,
      })
   }

   /// Fills the template's `@KEY@` placeholders. `version` is the one people see, which can carry
   /// git's description of the build. Fields that have to be numeric get the release's numbers.
   pub fn fill(&self, template: &str, version: &str) -> String {
      let mut numbers = self.version.split(['.', '-', '+']);
      let mut number = || numbers.next().unwrap_or("0").to_string();
      let (major, minor, patch) = (number(), number(), number());
      template
         .replace("@VERSION@", version)
         .replace("@VMAJOR@", &major)
         .replace("@VMINOR@", &minor)
         .replace("@VREV@", &patch)
         .replace("@NAME@", &self.name)
         .replace("@DESCRIPTION@", &self.description)
         .replace("@HOMEPAGE@", &self.homepage)
         .replace("@REPOSITORY@", &self.repository)
         .replace("@APPID@", &self.app_id)
         .replace(
            "@YEARS@",
            &format!("{}-{}", self.copyright_first_year, self.copyright_year),
         )
         .replace("@COPYRIGHT@", &self.copyright_holder)
   }
}

struct Table<'a> {
   label: &'static str,
   table: &'a toml::Table,
}

impl<'a> Table<'a> {
   fn new(parent: Option<&'a toml::Value>, key: &str, label: &'static str) -> Result<Self, String> {
      let table = parent
         .and_then(|parent| parent.get(key))
         .and_then(toml::Value::as_table)
         .ok_or_else(|| format!("Cargo.toml has no {label} table"))?;
      Ok(Self { label, table })
   }

   fn string(&self, key: &str) -> Result<String, String> {
      self
         .table
         .get(key)
         .and_then(toml::Value::as_str)
         .map(str::to_string)
         .ok_or_else(|| format!("{key} in {} has to be a string", self.label))
   }

   fn integer(&self, key: &str) -> Result<i64, String> {
      self
         .table
         .get(key)
         .and_then(toml::Value::as_integer)
         .ok_or_else(|| format!("{key} in {} has to be a number", self.label))
   }
}

#[cfg(test)]
mod tests {
   use super::*;

   fn metadata() -> Metadata {
      Metadata {
         version: "0.14.0-alpha.4".into(),
         description: "Naev: Sea of Darkness".into(),
         homepage: "https://naev.org/".into(),
         repository: "https://codeberg.org/naev/naev".into(),
         name: "Naev".into(),
         app_id: "org.naev.Naev".into(),
         copyright_first_year: 2006,
         copyright_year: 2026,
         copyright_holder: "Naev Dev Team".into(),
      }
   }

   #[test]
   fn numeric_fields_take_the_release_numbers() {
      let filled = metadata().fill("@VMAJOR@.@VMINOR@.@VREV@ @VERSION@", "0.14.0-alpha.4+g1a2b");
      assert_eq!(filled, "0.14.0 0.14.0-alpha.4+g1a2b");
   }

   #[test]
   fn copyright_spans_the_years() {
      let filled = metadata().fill("(C) @YEARS@ @COPYRIGHT@", "");
      assert_eq!(filled, "(C) 2006-2026 Naev Dev Team");
   }
}
