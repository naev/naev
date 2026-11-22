// indicatif draws nothing when stderr isn't a terminal, so each task prints its own summary line.

use indicatif::{ProgressBar, ProgressStyle};

pub fn bar(len: usize, message: &'static str) -> ProgressBar {
   let bar = ProgressBar::new(len as u64);
   bar.set_style(
      ProgressStyle::with_template("{msg} [{bar:24}] {pos}/{len}")
         .unwrap_or_else(|_| ProgressStyle::default_bar())
         .progress_chars("=> "),
   );
   bar.set_message(message);
   bar
}

pub fn spinner(message: &'static str) -> ProgressBar {
   let bar = ProgressBar::new_spinner();
   bar.set_style(
      ProgressStyle::with_template("{spinner} {msg} [{elapsed}]")
         .unwrap_or_else(|_| ProgressStyle::default_spinner()),
   );
   bar.set_message(message);
   bar.enable_steady_tick(std::time::Duration::from_millis(120));
   bar
}
