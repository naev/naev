/* This is just an entry point since we want to do linking with meson, not Cargo. */
fn main () -> std::io::Result<()>{
    naev::naev()
}
