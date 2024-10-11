use naevc::config;
use std::sync::LazyLock;

pub const VERSION: LazyLock<semver::Version> =
    LazyLock::new(|| semver::Version::parse(config::package_version).unwrap());
pub const VERSION_HUMAN: LazyLock<String> = LazyLock::new(|| {
    format!(
        " {} v{} ({})",
        config::package_name,
        config::package_version,
        config::host
    )
});
// If we can move the naevc::config info into this crate, we could solve this
//pub const VERSION_HUMAN: String = format!(" {} v{} ({})", config::package_name, config::package_version, config::host );
