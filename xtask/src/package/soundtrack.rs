use std::{fs, path::Path, process::Command};

use anyhow::{Context, Result};

use super::{Job, run};

/// The tagged soundtrack zip, then the MP3s and cover the Steam soundtrack depot takes.
pub(super) fn soundtrack(job: &Job, output: &Path) -> Result<()> {
   let metadata = job.metadata;
   let version = crate::version::current(job.root)?;
   let out = output.join("naev-soundtrack");
   fs::create_dir_all(&out).with_context(|| format!("creating {}", out.display()))?;
   // soundtrack.py adds .zip and .csv to the name it's given.
   let name = format!("naev-{version}-soundtrack");
   let stem = out.join(&name);
   let mut tag = Command::new("python3");
   tag.arg(job.root.join("utils/soundtrack.py"))
      .args(["--csv", "yes"])
      .arg("--source-dir")
      .arg(job.root)
      .arg("--output")
      .arg(&stem)
      .arg("--organization")
      .arg(&metadata.copyright_holder)
      .arg("--album")
      .arg(format!("{} Soundtrack", metadata.name));
   run(tag, "soundtrack.py")?;

   let zip = out.join(format!("{name}.zip"));
   let tracks = job.target.join("soundtrack");
   if tracks.exists() {
      fs::remove_dir_all(&tracks).with_context(|| format!("clearing {}", tracks.display()))?;
   }
   zip::ZipArchive::new(
      fs::File::open(&zip).with_context(|| format!("opening {}", zip.display()))?,
   )
   .and_then(|mut archive| archive.extract(&tracks))
   .with_context(|| format!("extracting {}", zip.display()))?;

   let steam = output.join("naev-steam-soundtrack");
   if steam.exists() {
      fs::remove_dir_all(&steam).with_context(|| format!("clearing {}", steam.display()))?;
   }
   fs::create_dir_all(&steam).with_context(|| format!("creating {}", steam.display()))?;
   let mut names: Vec<_> = fs::read_dir(&tracks)
      .with_context(|| format!("reading {}", tracks.display()))?
      .map(|entry| entry.map(|entry| entry.path()))
      .collect::<Result<_, _>>()
      .with_context(|| format!("reading {}", tracks.display()))?;
   names.sort();
   for track in &names {
      let mp3 = steam.join(
         track
            .with_extension("mp3")
            .file_name()
            .context("a track has a name")?,
      );
      // LAME's VBR quality 1 at 44.1 kHz, carrying the tags soundtrack.py wrote over as ID3v2.3.
      let mut ffmpeg = Command::new("ffmpeg");
      ffmpeg
         .args(["-hide_banner", "-loglevel", "error", "-i"])
         .arg(track)
         .args(["-c:a", "libmp3lame", "-q:a", "1", "-ar", "44100"])
         .args([
            "-map_metadata",
            "0",
            "-map_metadata",
            "0:s:0",
            "-id3v2_version",
            "3",
            "-vn",
         ])
         .arg(&mp3);
      run(ffmpeg, "ffmpeg")?;
   }
   let cover = job.root.join("extras/logos/naev_soundtrack_cover.png");
   fs::copy(&cover, steam.join("naev_soundtrack_cover.png"))
      .with_context(|| format!("copying {}", cover.display()))?;

   println!(
      "wrote {} and {} MP3s to {}",
      zip.display(),
      names.len(),
      steam.display()
   );
   Ok(())
}
