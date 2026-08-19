test('main_menu',
   find_program(files('watch-for-msg.py')),
   args: [
      naev_bin,
      '-d', meson.project_build_root() / 'dat',
      '-d', meson.project_source_root() / 'dat',
      '-d', meson.project_source_root() / 'assets',
      '-d', meson.project_source_root(),
      '--exitmainmenu',
      'Reached main menu'
   ],
   workdir: meson.project_source_root(),
   protocol: 'exitcode',
   timeout: 90,
   depends: [naev_bin, generated_data],
   )

if (ascli_exe.found())
   metainfo_test_file = 'org.naev.Naev.metainfo.xml'
   test('validate_metainfo',
   ascli_exe,
   args: [
      'validate',
      '--no-net',
      '--pedantic',
      '--verbose',
      metainfo_test_file
   ],
   workdir: meson.project_source_root()
   )
endif

if (luacheck.found())
   env = environment()
   env.set( 'LUA_PATH', meson.project_source_root() / '?.lua;;' )
   test('luacheck',
      nluacheck,
      #args: ['dat', 'docs', 'extras', 'utils', '--no-color'],
      args: ['dat', '--no-color'],
      env: env,
      workdir: meson.project_source_root(),
      timeout: 150,
      )
endif

# If this test fails, someone put a standard constant like PRIu64 in a translatable string.
# We can't support that (and you don't want to know how GNU gettext does it), but %.0f is typically good enough.
test('search_for_unsupported_sysdep_strings',
   find_program('grep'),
   args: ['-q', '%<PRI', join_paths(meson.project_source_root(), 'po', 'naev.pot')],
   should_fail: true,
)
