#![cfg_attr(debug_assertions, windows_subsystem = "console")]
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
/* This is just an entry point since we want to do linking with meson, not Cargo. */
fn main () -> naev::anyhow::Result<()>{
    naev::naev()
}
