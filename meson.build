project('naev', ['c', 'rust'],
   version : '0.14.0-alpha.4',
   default_options : [
      'optimization=g',
      'c_std=c11',
      'werror=false',
      'warning_level=3',
      'rust_std=2024',
      # Default to thin LTO when b_lto is enabled
      # Fat LTO takes quite a while and can appear to look deadlocked
      # Users who specifically want fat can still pass -Db_lto_mode=default
      'b_lto_mode=thin',
   ],
   # Requires meson 1.7.0 for rust_std=2024
   meson_version: '>=1.7.0')

# Dependency requirements
# 1.85 Rust 2024
# 1.87 nalgebra 0.34
# 1.88 iced 0.14.0
# 1.89 symphonia-adapter-libopus
# 1.93 vec::into_raw_parts
rust_version = '1.89' # version for Cargo.toml's 'rust-version' field
rustc_needed_version = '>=@0@.0'.format( rust_version )
cargo_needed_version = '>=1.77.2'

copyright_year = 2026

issue_address = 'https://codeberg.org/naev/naev/issues'
copyright_holder = 'Naev Dev Team'

# Tools
# TODO Is there a better way to get rust compiler version?
rustc = find_program('rustc', version: rustc_needed_version, required: false)
if not rustc.found()
   error('Need at least rustc version @0@\nYou can get an up-to-date version by installing rustup and running "rustup default stable" and "rustup update"'.format(rustc_needed_version))
endif
clang = find_program('clang', required: false)
if not clang.found()
   error('Clang is required for bindgen to find the standard library correctly. See issue: https://github.com/servo/gecko-media/issues/71')
endif

# Filesystem module for copying files
fs = import('fs')

# Python module checks (For AUTHORS generation, soundtrack generation and outfit variant generation)
python_module = import('python')
python = python_module.find_installation('python3', required: true)
pyyaml = python_module.find_installation('python3', modules: ['yaml'], required: true)
mutagen = python_module.find_installation('python3', modules: ['mutagen'], required: false)

subdir('utils')

# Version Generation
version_result = run_command(gen_version, meson.project_version(), check: true)
version = version_result.stdout().strip()
meson.add_dist_script(add_to_package, meson.project_source_root() / 'dat/VERSION', 'dat/VERSION' )
summary('tag', version)

# Initialization
ndata_path = get_option('ndata_path')
if ndata_path == ''
   ndata_path = get_option('datadir') / 'naev'
endif
summary('NData Path', ndata_path, section: 'Features')

# Subdirs
subdir('src')
subdir('dat')
subdir('docs')

####
# Naev
####
buildExec = get_option('executable')
if buildExec.disabled() == false
   app_metadata = configuration_data()  # For Info.plist, resource.rc, etc.
   debug = get_option('debug')
   debug_rust = get_option('debug_rust')
   debug_arrays = get_option('debug_arrays')
   tracy = get_option('tracy')
   paranoid = get_option('paranoid')
   # Cut out the numeric fields from our SemVer <major>.<minor>.<rev>[-pre_release][+build] for OS versioning.
   app_metadata.set('VERSION', meson.project_version())
   app_metadata.set('VMAJOR', meson.project_version().split('.')[0])
   app_metadata.set('VMINOR', meson.project_version().split('.')[1])
   app_metadata.set('VREV', meson.project_version().split('.')[2].split('-')[0].split('+')[0])
   app_metadata.set('YEAR', copyright_year)
   app_metadata.set('COPYRIGHT', copyright_holder)
   summary('Enabled' , debug       , section: 'Debug', bool_yn: true)
   summary('Paranoid', paranoid    , section: 'Debug', bool_yn: true)
   summary('Arrays'  , debug_arrays, section: 'Debug', bool_yn: true)

   ### Soft deps (required: false)

   # Appstream (Used for generating desktop files and verifying metadata)
   ascli_exe = find_program('appstreamcli', version: '>=0.12.9', required: false)

   # Luacheck (linter) wrapper
   luacheck = find_program('luacheck', required: false)
   nluacheck = find_program(files('utils'/'nluacheck.py'))

   summary('tracy', tracy, section: 'Debug', bool_yn: true )

   ### Generated sources

   if host_machine.system() == 'darwin'
      configure_file(input: 'extras/macos/Info.plist.in', output: 'Info.plist', configuration: app_metadata,
         install: true, install_dir: 'Contents')
      install_data('extras/logos/naev.icns', install_dir: ndata_path)

   elif host_machine.system() == 'windows'
      icon = files('extras/logos/logo.ico')
      install_data(icon, install_dir: '.')
      configure_file(input: 'extras/windows/naev.exe.manifest.in', output: 'naev.exe.manifest', configuration: app_metadata)
      # TODO resource.rc is generated but never compiled or linked, since cargo
      # owns the executable. Needs rustc-link-arg-bins or the embed-resource crate.
      configure_file(input: 'extras/windows/resource.rc.in', output: 'resource.rc', configuration: app_metadata)
   endif

# Target we are cross-compiling for
   target_triplet = get_option('target_triplet')

   copydir = find_program(files('utils'/'build'/'copydir.py'))

   # Cargo compiles the C engine, the Rust and the executable. meson arranges
   # the inputs, then installs what comes out.

   # Handle setting up and passing stuff to Cargo
   # Since Rust debug is significantly slower than release, we try to use our
   # custom release-with-debug profile that runs quite fast while still having
   # pretty good debugging properties. Use 'debug_rust' option to explicitly
   # force Rust to be debug mode.
   if get_option('debug_rust')
      # Super annoying that the 'dev' target outputs to 'debug', so we have to split the paths
      # If --artifact-dir stabilizes ( https://github.com/rust-lang/cargo/issues/6790 ),
      # we can simplify this and just use that straight
      rust_target = 'dev'
      rust_artifact_dir = 'debug'
   elif get_option('debug')
      rust_target = 'release-with-debug'
      rust_artifact_dir = 'release-with-debug'
   else
      rust_target = 'release'
      rust_artifact_dir = 'release'
   endif
   cargo = find_program('cargo', version: cargo_needed_version)
   cargo_toml = files('Cargo.toml')
   # flatpak needs Cargo.lock in the source tarball to be able to generate stuff
   # https://github.com/flatpak/flatpak-builder-tools/tree/master/cargo
   meson.add_dist_script( cargo_lock, meson.project_source_root() / 'Cargo.toml' )
   cargo_env = environment()
   cargo_env.set( 'CARGO_HOME', meson.project_build_root() / 'cargo-home' )
   # naevc's build script bakes these into config.h and its config module.
   cargo_env.set( 'NAEV_BUILD_VERSION', version )
   cargo_env.set( 'NAEV_PKGDATADIR', get_option('prefix') / ndata_path )
   cargo_env.set( 'NAEV_BLAS', get_option('blas') )

   # Decide whether to enable linker-plugin-LTO for Rust objects. When
   # b_lto is on, the final link is done with LTO, which means every object
   # file fed to the linker (including those inside libnaev.rlib) must carry
   # bitcode the linker plugin understands. For clang/lld that is LLVM
   # bitcode in a .llvmbc section, which rustc only emits under
   # -Clinker-plugin-lto. That flag additionally requires rustc's LLVM and
   # clang's LLVM to be approximately the same major version, otherwise
   # the final link fails with "Can't find section .llvmbc" or bitcode
   # version mismatch errors.
   # See https://doc.rust-lang.org/rustc/linker-plugin-lto.html
   linker_plugin_lto_opt = get_option('linker_plugin_lto')
   use_rust_lto = false
   if linker_plugin_lto_opt == 'enabled'
      use_rust_lto = true
      message('linker_plugin_lto forced on; skipping toolchain compatibility check')
   elif linker_plugin_lto_opt == 'auto' and get_option('b_lto')
      cc_id = meson.get_compiler('c').get_id()
      if cc_id != 'clang'
         error('b_lto is enabled but the C compiler is "@0@", not clang. Mixed-language LTO across rustc and @0@ is not supported. Re-run with -Db_lto=false or switch to a clang toolchain.'.format(cc_id))
      endif
      clang_llvm = meson.get_compiler('c').version().split('.')[0].to_int()
      rustc_verbose = run_command(rustc, '--version', '--verbose', check: true)
      rustc_llvm = 0
      foreach line : rustc_verbose.stdout().split('\n')
         if line.startswith('LLVM version:')
            rustc_llvm = line.split(':')[1].strip().split('.')[0].to_int()
         endif
      endforeach
      if rustc_llvm == 0
         error('b_lto is enabled but could not parse rustc\'s LLVM version. Re-run with -Db_lto=false or -Dlinker_plugin_lto=disabled.')
      endif
      llvm_skew = rustc_llvm - clang_llvm
      if llvm_skew < 0
         llvm_skew = -llvm_skew
      endif
      if llvm_skew > 1
         error('b_lto is enabled but rustc\'s LLVM (@0@) and clang\'s LLVM (@1@) differ by more than one major version, so linker-plugin-LTO cannot be used. Re-run with -Db_lto=false, install a matching toolchain, or pass -Dlinker_plugin_lto=enabled to override this check at your own risk.'.format(rustc_llvm, clang_llvm))
      endif
      use_rust_lto = true
      message('linker-plugin-LTO enabled (clang LLVM @0@ / rustc LLVM @1@)'.format(clang_llvm, rustc_llvm))
   endif
   if use_rust_lto
      cargo_env.set('RUSTFLAGS', '-Clinker-plugin-lto')
   endif
   cargo_options = [ '--manifest-path', cargo_toml ]
   cargo_options += [ '--profile', rust_target ]
   cargo_options += [ '--target-dir', meson.project_build_root() / 'src' ]
   if target_triplet != ''
      cargo_options += ['--target', target_triplet]
   endif

   # Options that map onto cargo features rather than compiler flags.
   cargo_features = []
   if paranoid
      cargo_features += 'paranoid'
   endif
   if debug_arrays
      cargo_features += 'debug_arrays'
   endif
   if tracy
      if not debug
         error('tracy requires a debug build!')
      endif
      cargo_features += 'tracy'
   endif
   if cargo_features.length() > 0
      cargo_options += [ '--features', ','.join(cargo_features) ]
   endif

   # Cargo builds the C engine, all the Rust and the executable in one go.
   cargo_build = custom_target( 'cargo-build',
      depend_files: source_rust + cargo_toml,
      output: 'src',
      console: true,
      env: cargo_env,
      command: [ cargo, 'build', cargo_options, ] )

   # Cargo picks where its artifacts land, so lift the binary somewhere meson
   # and the dev launcher can find it.
   # TODO Drop the copy once --artifact-dir stabilises.
   # https://github.com/rust-lang/cargo/issues/6790
   naev_bin = custom_target( 'naev',
      depends: [cargo_build],
      depend_files: source_rust + cargo_toml,
      output: 'naev',
      command: [ copydir, meson.project_build_root(), 'src' / target_triplet / rust_artifact_dir / '@OUTPUT@', ],
      install: true,
      install_dir: get_option('bindir') )

   # Outfits, naevpedia pages, tech lists and translations. The dev launcher
   # mounts this directory on top of the tracked data.
   # TODO Nothing installs these any more; xtask needs to grow that.
   generated_data = custom_target( 'generated-data',
      output: 'dat',
      console: true,
      build_by_default: true,
      env: cargo_env,
      command: [
         cargo, 'run', '--quiet', '--release',
         '--manifest-path', cargo_toml, '--package', 'xtask', '--',
         'data', '--output', meson.project_build_root() / 'dat',
      ] )

   # Catalogue maintenance. These rewrite tracked files in po/, so they are
   # run deliberately rather than as part of a build.
   foreach task : ['pot', 'update-po']
      run_target( task,
         env: cargo_env,
         command: [
            cargo, 'run', '--quiet', '--release',
            '--manifest-path', cargo_toml, '--package', 'xtask', '--',
            task, '--build-dir', meson.project_build_root(),
         ] )
   endforeach

   # Naev pluginmgr cli for managing plugins.
   # Inherits the cargo environment above, so should work in cross compilation environments.
   #pluginmgr_cli = custom_target('pluginmgr-cli',
   #   depends: [cargo_build],
   #   console: true,
   #   command: [
   #      'env', cargo_env, cargo,
   #      'build',
   #      cargo_options,
   #      '--package', 'pluginmgr-cli'
   #   ],
   #   output: 'pluginmgr-cli'
   #)

   # Simplifies clippy stuff
   clippy = run_target( 'clippy',
      depends: [cargo_build],
      env: cargo_env,
      command: [ cargo, 'clippy', cargo_options, ] )

   # Run all our Rust tests (if we have them)
   test('cargo',
      cargo,
      env: cargo_env,
      timeout: 300,
      args: ['test', '--workspace'] + cargo_options,
      workdir: meson.project_build_root(),
      is_parallel: false, # too slow
      depends: [cargo_build],
      priority: 1,
   )

   gen_authors = find_program(files('utils'/'build'/'gen_authors.py'))
   authors = custom_target(
      'authors',
      command: [
         gen_authors,
         '--output', '@OUTPUT0@',
         '--preamble', '@INPUT0@',
         '@INPUT1@', '@INPUT2@',
      ],
      input: files('dat/AUTHORS', 'assets/gfx/ARTWORK_LICENSE.yaml', 'assets/snd/SOUND_LICENSE.yaml'),
      output: 'AUTHORS',
      install: true,
      install_dir: ndata_path / 'dat',
   )

   gen_gettext_stats = find_program(files('utils'/'build'/'gen_gettext_stats.py'))
   gettext_stats = custom_target('gettext_stats',
      command: [gen_gettext_stats, '--output', '@OUTPUT@', '@INPUT@'],
      input: files('po/' + meson.project_name() + '.pot'),
      output: meson.project_name() + '.txt',
      install: true,
      install_dir: ndata_path / 'dat/gettext_stats',
   )

   # Need to add other generated data files here as necessary
   gen_zip_overlay = find_program(files('utils'/'build'/'gen_zip_overlay.py'))
   gen_zip_command = [gen_zip_overlay, '@OUTPUT@', authors, '--cd', 'gettext_stats', gettext_stats]
   # Note that generated outfits populate build/dat/ which is included with
   # naev.py, so no need to put them in the overlay on top
   #gen_zip_command += ['--cd', 'outfits']
   #foreach target: gen_outfits
   #    gen_zip_command += target
   #endforeach
   zip_overlay = custom_target('zip_overlay',
      command: gen_zip_command,
      output: 'meson_overlay.zip',
      depends: [authors, gettext_stats], # + gen_outfits,
      build_by_default: true
   )

   naevpy_config = {
      'build_root': meson.current_build_dir(),
      'source_root': meson.project_source_root(),
      'naev_bin' : naev_bin.full_path(),
      'zip_overlay' : zip_overlay.full_path(),
      'debug' : debug.to_string(),
      'debug_paranoid' : paranoid.to_string(),
   }

   naev_py = configure_file(
      input: join_paths('utils','build','naev.py'),
      output: 'naev.py',
      configuration: naevpy_config,
   )

   naev_valgrind_py = configure_file(
      input: join_paths('utils','build','naev_valgrind.py'),
      output: 'naev_valgrind.py',
      configuration: naevpy_config,
   )

   #pluginmgr_cli_py = configure_file(
   #   input: join_paths('utils','build','pluginmgr-cli.py'),
   #   output: 'pluginmgr-cli.py',
   #   configuration: {
   #      'pluginmgr_cli_bin' : meson.project_build_root() / 'src' / target_triplet / rust_artifact_dir / 'pluginmgr-cli',
   #   }
   #)

   gdbinit = configure_file(
      input: join_paths('utils','build','gdbinit.in'),
      output: '.gdbinit',
      configuration: {
         'source_root': meson.project_source_root(),
         'build_root': meson.current_build_dir(),
      }
   )

   lldbinit = configure_file(
      input: join_paths('utils','build','lldbinit.py.in'),
      output: 'lldbinit.py',
      configuration: {
         'source_root': meson.project_source_root(),
      }
   )

   if host_machine.system() not in ['windows', 'darwin']
      install_data(
         'gpl.txt',
         'LICENSE',
         'Readme.md',
         install_dir: get_option('datadir') / 'doc/naev'
      )
   endif
   # TODO: And what if it is 'windows' or 'darwin'?

   install_subdir(
      'dat',
      install_dir: ndata_path,
      # Parts of dat/ are used as inputs to custom build targets, which generate the final installed versions.
      exclude_files: ['AUTHORS', 'outfits/meson.build', 'outfits/bioship/generate.py', 'outfits/bioship/meson.build', 'scripts/meson.build', 'outfits/tech/meson.build' ],
      exclude_directories: ['outfits/py', 'outfits/bioship/templates'],
   )

   install_subdir(
      'assets',
      install_dir: ndata_path / 'dat',
      exclude_directories: '.git',  # That's a marker used by "git submodule".
      strip_directory: true,
   )

   if host_machine.system() in ['linux', 'freebsd', 'openbsd', 'netbsd']
      install_data('org.naev.Naev.desktop', install_dir: join_paths (get_option ('datadir'), 'applications'))
      install_data('org.naev.Naev.metainfo.xml', install_dir: get_option('datadir') / 'metainfo')

      install_data(['extras/logos/logo16.png', 'extras/logos/logo32.png', 'extras/logos/logo64.png', 'extras/logos/logo128.png', 'extras/logos/naev.png'],
         rename : ['16x16/apps/org.naev.Naev.png', '32x32/apps/org.naev.Naev.png', '64x64/apps/org.naev.Naev.png', '128x128/apps/org.naev.Naev.png', '256x256/apps/org.naev.Naev.png'],
         install_dir: get_option('datadir') / 'icons/hicolor')

      install_man('naev.6')
   endif

   subdir('test')

   subdir ('extras')
endif


####
# Soundtrack
####
if (mutagen.found())
   soundtrackpy = find_program(files('utils'/'soundtrack.py'))
   custom_target(
      'soundtrack',
      command: [
         soundtrackpy,
         '--csv', 'yes',
         '--source-dir', meson.project_source_root(),
         '--output', '@OUTPUT0@'
      ],
      output: [
         'naev-' + meson.project_version() + '-soundtrack.zip',
         'naev-' + meson.project_version() + '-soundtrack.csv'
      ]
   )
endif
