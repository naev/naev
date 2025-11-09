use crate::warn_err;
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

#[unsafe(no_mangle)]
pub extern "C" fn naev_versionCompare(version: *const c_char) -> c_int {
    let ver = match parse_cstr(version) {
        Ok(v) => v,
        Err(e) => {
            warn_err(e);
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
            warn_err(e);
            return 0;
        }
    };
    let verb = match parse_cstr(target) {
        Ok(v) => v,
        Err(e) => {
            warn_err(e);
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
            warn_err(e);
            return 0;
        }
    };
    let reqstr = unsafe { CStr::from_ptr(req) };
    let req = match semver::VersionReq::parse(&reqstr.to_string_lossy()) {
        Ok(r) => r,
        Err(e) => {
            warn_err(e.into());
            return 0;
        }
    };
    req.matches(&vera) as c_int
}
