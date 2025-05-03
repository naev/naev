use anyhow::Result;
use naevc::config;
use std::cmp::Ordering;
use std::ffi::CStr;
use std::os::raw::{c_char, c_int};
use std::sync::LazyLock;

pub static VERSION: LazyLock<semver::Version> =
    LazyLock::new(|| semver::Version::parse(config::PACKAGE_VERSION).unwrap());
pub static VERSION_HUMAN: LazyLock<String> = LazyLock::new(|| {
    format!(
        " {} v{} ({})",
        config::PACKAGE_NAME,
        config::PACKAGE_VERSION,
        config::HOST
    )
});
// If we can move the naevc::config info into this crate, we could solve this
//pub static VERSION_HUMAN: String = format!(" {} v{} ({})", config::PACKAGE_NAME, config::PACKAGE_VERSION, config::HOST );

fn binary_comparison(x: u64, y: u64) -> i32 {
    match x.cmp(&y) {
        Ordering::Equal => 0,
        Ordering::Greater => 1,
        Ordering::Less => -1,
    }
}

fn compare_versions(vera: &semver::Version, verb: &semver::Version) -> i32 {
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
    let ptr = unsafe { CStr::from_ptr(ver) };
    let cstr = ptr.to_str().unwrap();
    Ok(semver::Version::parse(cstr)?)
}

#[unsafe(no_mangle)]
pub extern "C" fn naev_versionCompare(version: *const c_char) -> c_int {
    let ver = match parse_cstr(version) {
        Ok(v) => v,
        _ => return 0,
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
        _ => return 0,
    };
    let verb = match parse_cstr(target) {
        Ok(v) => v,
        _ => return 0,
    };
    compare_versions(&vera, &verb)
}
