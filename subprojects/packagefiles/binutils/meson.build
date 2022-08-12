project('binutils', 'c', version: '2.39', meson_version: '>=0.57.0')

mod = import('unstable_external_project')
host = run_command(meson.get_compiler('c'), '-dumpmachine', check: true).stdout().strip()

if get_option('steamruntime')
   # Two build systems are 100% sure of where libiberty.a is going to go. Their answers are not the same. Unless...
   parent = meson.current_build_dir() / 'dist' / get_option('prefix') / 'lib'
   run_command(['mkdir', '-p', parent])
   run_command(['ln', '-s', parent / host, parent / 'lib'])
endif

p = mod.add_project('configure',
   configure_options: [
      '--disable-gprofng',
      '--disable-ld',
      '--disable-libstdcxx',
      '--disable-libquadmath',
      '--disable-multilib',
      '--disable-nls',
      '--disable-shared',
      '--enable-install-libbfd',
      '--enable-install-libiberty',
      '--with-system-zlib',
      get_option('b_lto') ? '--enable-lto' : [],
   ],
   cross_configure_options: ['--host='+host],
)

libbfd_dep = declare_dependency(dependencies: [p.dependency('bfd'), p.dependency('iberty'), dependency('zlib')])
