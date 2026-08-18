#![cfg_attr(debug_assertions, windows_subsystem = "console")]
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
/* Thin entry point; everything of substance lives in the naev library. */
fn main() -> naev::anyhow::Result<()> {
   naev::naev()
}
