manual_build = custom_target('manual',
   output: 'index.html',
   input: ['book.toml', 'src/SUMMARY.md'],
   command: [find_program('mdbook'), 'build', '--dest-dir', meson.current_build_dir(), '@CURRENT_SOURCE_DIR@'],
   build_always_stale: true,
   install: false )
