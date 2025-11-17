mdbook = find_program('mdbook', required: false)
if mdbook.found()
   manual_build = custom_target('manual',
      output: 'index.html',
      input: ['book.toml', 'src/SUMMARY.md'],
      command: [mdbook, 'build', '--dest-dir', meson.current_build_dir(), '@CURRENT_SOURCE_DIR@'],
      build_always_stale: true,
      install: false )
   summary('Dev Manual', true, section: 'Documentation', bool_yn: true )
else
   message('Note: Compiling the dev manual requires the mdbook binary, which can usually be install with "cargo install mdbook".')
   summary('Dev Manual', 'NO (missing mdbook)', section: 'Documentation')
endif
