use crate::warn_err;
use anyhow::Result;
use naevc::config;
use std::cmp::Ordering;
use std::ffi::CStr;
use std::os::raw::{c_char, c_int};
use std::sync::LazyLock;

pub static VERSION: LazyLock<semver::Version> =
   LazyLock::new(|| semver::Version::parse(config::PACKAGE_VERSION).unwrap());
pub static VERSION_WITHOUT_PRERELEASE: LazyLock<semver::Version> = LazyLock::new(|| {
   let mut v = VERSION.clone();
   v.pre = semver::Prerelease::EMPTY;
   v
});
pub static VERSION_HUMAN: LazyLock<String> = LazyLock::new(|| {
   format!(
      " {} v{} ({})",
      config::PACKAGE_NAME,
      config::PACKAGE_VERSION,
      config::HOST
   )
});

fn binary_comparison(x: u64, y: u64) -> i32 {
   match x.cmp(&y) {
      Ordering::Equal => 0,
      Ordering::Greater => 1,
      Ordering::Less => -1,
   }
}

pub fn compare_versions(vera: &semver::Version, verb: &semver::Version) -> i32 {
   let res = binary_comparison(vera.major, verb.major);
   if res != 0 {
      return 3 * res;
   }
   let res = binary_comparison(vera.minor, verb.minor);
   if res != 0 {
      return 2 * res;
   }
   binary_comparison(vera.patch, verb.patch)
}

fn parse_cstr(ver: *const c_char) -> Result<semver::Version> {
   if ver.is_null() {
      anyhow::bail!("null pointer");
   }
   let ptr = unsafe { CStr::from_ptr(ver) };
   Ok(semver::Version::parse(&ptr.to_string_lossy())?)
}

// Custom comparator that actually compares prereleases instead of just making it skip all
// comparisons. This makes sense if you use prereleases as something that is not consumed at all,
// but that's not what we do in some of the checks. In particular, we want to be able to do things
// like check if 0.13.0-alpha.1 is >0.12.0 or <0.13.0 which is false in both cases according to the
// spec.
fn version_matches(version: &semver::Version, req: &semver::VersionReq) -> bool {
   // Standard comparison
   if req.matches(version) {
      return true;
   }

   // Fall back to comparator ordering
   req.comparators.iter().all(|cmp| {
      let target = semver::Version {
         major: cmp.major,
         minor: cmp.minor.unwrap_or(0),
         patch: cmp.patch.unwrap_or(0),
         pre: cmp.pre.clone(),
         build: Default::default(),
      };

      match cmp.op {
         semver::Op::Less => version < &target,
         semver::Op::LessEq => version <= &target,
         semver::Op::Greater => version > &target,
         semver::Op::GreaterEq => version >= &target,
         semver::Op::Exact => version == &target,
         // Fallback for complex operators, this is not good, but what are we going to do? :D
         _ => req.matches(version),
      }
   })
}

#[unsafe(no_mangle)]
pub extern "C" fn naev_versionCompare(version: *const c_char) -> c_int {
   let ver = match parse_cstr(version) {
      Ok(v) => v,
      Err(e) => {
         warn_err!(e);
         return 0;
      }
   };
   compare_versions(&VERSION, &ver)
}

#[unsafe(no_mangle)]
pub extern "C" fn naev_versionCompareTarget(
   version: *const c_char,
   target: *const c_char,
) -> c_int {
   let vera = match parse_cstr(version) {
      Ok(v) => v,
      Err(e) => {
         warn_err!(e);
         return 0;
      }
   };
   let verb = match parse_cstr(target) {
      Ok(v) => v,
      Err(e) => {
         warn_err!(e);
         return 0;
      }
   };
   compare_versions(&vera, &verb)
}

#[unsafe(no_mangle)]
pub extern "C" fn naev_versionMatchReq(version: *const c_char, req: *const c_char) -> c_int {
   let vera = match parse_cstr(version) {
      Ok(v) => v,
      Err(e) => {
         warn_err!(e);
         return 0;
      }
   };
   let reqstr = unsafe { CStr::from_ptr(req) };
   let req = match semver::VersionReq::parse(&reqstr.to_string_lossy()) {
      Ok(r) => r,
      Err(e) => {
         warn_err!(e);
         return 0;
      }
   };
   version_matches(&vera, &req) as c_int
}

#[test]
fn test_version() {
   let ver = semver::Version::parse("0.13.0-alpha.2").unwrap();
   let tv = |s| version_matches(&ver, &semver::VersionReq::parse(s).unwrap());
   assert_eq!(tv("<0.14.0"), true);
   assert_eq!(tv(">0.14.0"), false);
   assert_eq!(tv("<0.13.0"), true);
   assert_eq!(tv(">0.13.0"), false);
   assert_eq!(tv(">0.13.1"), false);
   assert_eq!(tv(">0.13.0-alpha.1"), true);
   assert_eq!(tv("<0.13.0-alpha.1"), false);
   assert_eq!(tv(">0.13.0-alpha.3"), false);
   assert_eq!(tv("<0.13.0-alpha.3"), true);
   assert_eq!(tv("<0.13.0-beta.1"), true);
   assert_eq!(tv(">0.13.0-beta.1"), false);
}
