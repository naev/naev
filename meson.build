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
forced_fallbacks = get_option('force_fallback_for')

issue_address = 'https://codeberg.org/naev/naev/issues'
copyright_holder = 'Naev Dev Team'

# Tools
cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments([
   '-Wno-pedantic',
   '-Wshadow',
   '-fno-signed-zeros'
])
# TODO Is there a better way to get rust compiler version?
rustc = find_program('rustc', version: rustc_needed_version, required: false)
if not rustc.found()
   error('Need at least rustc version @0@\nYou can get an up-to-date version by installing rustup and running "rustup default stable" and "rustup update"'.format(rustc_needed_version))
endif
clang = find_program('clang', required: false)
if not clang.found()
   error('Clang is required for bindgen to find the standard library correctly. See issue: https://github.com/servo/gecko-media/issues/71')
endif

if host_machine.system() == 'linux'
   c_args += ['-D_XOPEN_SOURCE=700']
elif host_machine.system() == 'windows'
   # We need SDL_DISABLE_ALLOCA because of
   # https://github.com/libsdl-org/SDL/issues/13358
   c_args += ['-D_USE_MATH_DEFINES', '-DSDL_DISABLE_ALLOCA']
elif host_machine.system() == 'darwin'
   c_args += ['-D_DARWIN_C_SOURCE', '-D_POSIX_C_SOURCE=200809L']
endif

add_project_arguments(c_args, language: 'c')

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
   config_data = configuration_data()
   app_metadata = configuration_data()  # For Info.plist, resource.rc, etc.
   naev_deps = []
   debug = get_option('debug')
   debug_rust = get_option('debug_rust')
   debug_arrays = get_option('debug_arrays')
   tracy = get_option('tracy')
   paranoid = get_option('paranoid')
   # The next three are sometimes expected by GNU tools and headers, sometimes for mere existence. Provide stable values.
   config_data.set_quoted('PACKAGE', meson.project_name())
   config_data.set_quoted('PACKAGE_NAME', meson.project_name())
   config_data.set_quoted('PACKAGE_VERSION', meson.project_version())
   config_data.set_quoted('PKGDATADIR', get_option('prefix') / ndata_path)
   config_data.set_quoted('HOST', host_machine.system() + '-' + host_machine.cpu_family())
   # Cut out the numeric fields from our SemVer <major>.<minor>.<rev>[-pre_release][+build] for OS versioning.
   app_metadata.set('VERSION', meson.project_version())
   app_metadata.set('VMAJOR', meson.project_version().split('.')[0])
   app_metadata.set('VMINOR', meson.project_version().split('.')[1])
   app_metadata.set('VREV', meson.project_version().split('.')[2].split('-')[0].split('+')[0])
   app_metadata.set('YEAR', copyright_year)
   app_metadata.set('COPYRIGHT', copyright_holder)
   config_data.set('DEBUG', debug ? 1 : false)
   config_data.set('DEBUG_ARRAYS', debug_arrays ? 1 : false)
   config_data.set('DEBUGGING', debug ? 1 : false)
   config_data.set('DEBUG_PARANOID', paranoid ? 1 : false)
   summary('Enabled' , debug       , section: 'Debug', bool_yn: true)
   summary('Paranoid', paranoid    , section: 'Debug', bool_yn: true)
   summary('Arrays'  , debug_arrays, section: 'Debug', bool_yn: true)

   has_posix = cc.has_define('__unix__') or cc.has_define('__unix') or host_machine.system()=='darwin'
   config_data.set10('HAS_POSIX', has_posix)
   config_data.set10('HAS_UNIX', has_posix)

   ### Hard deps (required: true)

   naev_deps += cc.find_library('m', required : false)

   enet = dependency('libenet', required: true, version: '>=1.3', fallback: ['enet', 'enet_dep'], static: get_option('steamruntime'))
   sdl = dependency('sdl3', version: '>=3.2.0', required: true)
   libunibreak = dependency('libunibreak', required: true, version: '>=4.0', fallback: ['libunibreak'], static: get_option('steamruntime'))
   cmark = dependency('libcmark', required: true, version: '>=0.31.0', fallback: ['cmark', 'cmark_dep'], static: get_option('steamruntime'))
   opus = dependency( 'opus', required: true )

   libxml2 = dependency('libxml-2.0', required: false)
   if not libxml2.found()
      libxml2 = cc.find_library('xml2', required: true)  # e.g., MacOSX SDK
   endif

   use_system_physfs = ('physfs' not in forced_fallbacks and 'forcefallback' != get_option('wrap_mode'))
   if use_system_physfs
      system_physfs = dependency('physfs', required: false, static: get_option('steamruntime'))
      if not system_physfs.found()
         system_physfs = cc.find_library('physfs', required: false, has_headers: ['physfs.h'])
      endif
      use_system_physfs = system_physfs.found()
   endif
   naev_deps += use_system_physfs ? system_physfs : subproject('physfs').get_variable('physfs_dep')

   naev_deps += [
      cmark,
      enet,
      dependency('freetype2', required: true),
      libunibreak,
      libxml2,
      sdl,
   ]

   # Lua
   useLuaJIT = get_option('luajit')
   lua = dependency('', required: false)
   if useLuaJIT.disabled() == false
      lua = dependency('luajit', fallback: ['luajit', 'luajit_dep'], required: useLuaJIT, static: get_option('steamruntime'), default_options: ['c_std=gnu99', 'luajit=false'])
   endif
   config_data.set10('HAVE_LUAJIT', lua.found())
   summary('LuaJIT', lua.found(), section: 'Features', bool_yn: true)

   if not lua.found()
      lua = dependency('lua51', fallback: ['lua', 'lua_dep'], required: true)
   endif

   naev_deps += [lua]

   # Linear Algebra
   use_system_glpk = ('glpk' not in forced_fallbacks and 'forcefallback' != get_option('wrap_mode'))
   use_system_suitesparse = ('SuiteSparse' not in forced_fallbacks and 'forcefallback' != get_option('wrap_mode'))

   if use_system_glpk
      system_glpk = cc.find_library('glpk', required: false, has_headers: ['glpk.h'])
      use_system_glpk = system_glpk.found()
   endif
   naev_deps += use_system_glpk ? system_glpk : subproject('glpk').get_variable('glpk_dep')

   if use_system_suitesparse
      message('Note: Naev needs the following SuiteSparse libraries: amd, colamd, cholmod, and either csparse or cxsparse. ' +
         'It can use its own copies if you run "meson configure --force-fallback-for=SuiteSparse".')
      foreach csparse_name : ['cxsparse', 'csparse']
         system_csparse = cc.find_library(csparse_name, required: false)
         if system_csparse.found()
            break
         endif
      endforeach
      if system_csparse.found()
         naev_deps += system_csparse
         config_data.set10('HAVE_SUITESPARSE_CS_H', cc.has_header('suitesparse/cs.h'))
      else
         error('Failed to find an installation of c[x]sparse. See above note about SuiteSparse.')
      endif

      system_cholmod = cc.find_library('cholmod', required: false)
      if system_cholmod.found()
         naev_deps += system_cholmod
         config_data.set10('HAVE_SUITESPARSE_CHOLMOD_H', cc.has_header('suitesparse/cholmod.h'))
      else
         error('Failed to find an installation of cholmod. See above note about SuiteSparse.')
      endif

      naev_deps += cc.find_library('amd', required: true)
      naev_deps += cc.find_library('ccolamd', required: false)
      naev_deps += cc.find_library('colamd', required: true)
      naev_deps += cc.find_library('lapack', required: false)
      naev_deps += cc.find_library('metis', required: false)
      naev_deps += cc.find_library('suitesparseconfig', required: true)
   else
      naev_deps += subproject('SuiteSparse').get_variable('SuiteSparse_dep')
   endif

   if get_option('steamruntime')
      blas = cc.find_library('openblas', required: true, static: true)
   elif get_option('blas') == 'Accelerate'
      blas = dependency('Accelerate', required: true)
   else
      blas = cc.find_library(get_option('blas'), required: true)
   endif

   naev_deps += blas

   # libdl can be used for debugging stack traces.
   if host_machine.system()=='windows'
      # On non-Windows platforms, GLAD relies on dlopen().
      # Windows doesn't have dlopen, but we can use dlfcn-win32 instead.
      libdl = dependency('dl', fallback: ['dlfcn-win32', 'dl_dep'], required: true)
   else
      libdl = dependency('dl', required: true)
   endif
   naev_deps += libdl

   ### Soft deps (required: false)

   # Appstream (Used for generating desktop files and verifying metadata)
   ascli_exe = find_program('appstreamcli', version: '>=0.12.9', required: false)

   # Luacheck (linter) wrapper
   luacheck = find_program('luacheck', required: false)
   nluacheck = find_program(files('utils'/'nluacheck.py'))

   # Audio
   openal = dependency('openal', required: true)
   vorbis = dependency('vorbis', required: true)
   vorbisfile = dependency('vorbisfile', required: true)
   ogg = dependency('ogg', required: true)  # Transitive dependency. At least some MSYS2 build envs may miss it.

   naev_deps += [openal, ogg, vorbis, vorbisfile]

   have_tracy = false
   if tracy
      if not debug
         error('tracy requires a debug build!')
      endif
      use_system_tracy = ('tracy' not in forced_fallbacks and 'forcefallback' != get_option('wrap_mode'))
      if use_system_tracy
         system_tracy = cc.find_library('tracy', required: false, has_headers: ['tracy/TracyC.h'] )
         use_system_tracy = system_tracy.found()
      endif
      naev_deps += use_system_tracy ? system_tracy : subproject('tracy').get_variable('tracy_dep')
      config_data.set10('HAVE_TRACY', true)
      config_data.set10('TRACY_ENABLE', true) # Needed for tracy to actually work
      have_tracy = true
   endif
   summary('tracy', have_tracy, section: 'Debug', bool_yn: true )

   # Standard library feature tests
   config_data.set10('HAVE_FEENABLEEXCEPT', cc.has_header_symbol('fenv.h', 'feenableexcept', prefix: '#define _GNU_SOURCE'))
   config_data.set10('HAVE_ALLOCA_H', cc.has_header('alloca.h'))
   config_data.set10('HAVE_FENV_H', cc.has_header('fenv.h'))
   config_data.set10('HAVE_MALLOC_H', cc.has_header('malloc.h'))
   config_data.set10('HAVE_SIGACTION', cc.has_function('sigaction'))
   config_data.set10('HAVE_STRSIGNAL', cc.has_function('strsignal'))
   # BLAS include tests
   cblas_test = '\nint main (void) { double x = 0; return (int)cblas_ddot( 1, &x, 1, &x, 1 ); }'
   config_data.set10('HAVE_ACCELERATE_ACCELERATE_H', cc.links('#include <Accelerate/Accelerate.h>' + cblas_test, dependencies: blas))
   config_data.set10('HAVE_CBLAS_H', cc.links('#include <cblas.h>' + cblas_test, dependencies: blas))
   config_data.set10('HAVE_CBLAS_OPENBLAS_H', cc.links('#include <cblas_openblas.h>' + cblas_test, dependencies: blas))
   config_data.set10('HAVE_CBLAS_HYPHEN_OPENBLAS_H', cc.links('#include <cblas-openblas.h>' + cblas_test, dependencies: blas))
   config_data.set10('HAVE_OPENBLAS_CBLAS_H', cc.links('#include <openblas/cblas.h>' + cblas_test, dependencies: blas))
   config_data.set10('HAVE_F77BLAS_H', cc.has_header('f77blas.h', dependencies: blas))
   config_data.set10('HAVE_OPENBLAS_F77BLAS_H', cc.has_header('openblas/f77blas.h', dependencies: blas))

   ### Generated sources

   # Objects that have to be linked into the executable but aren't part of the
   # engine proper. The C sources themselves are built by naevc's build script.
   if host_machine.system() == 'darwin'
      configure_file(input: 'extras/macos/Info.plist.in', output: 'Info.plist', configuration: app_metadata,
         install: true, install_dir: 'Contents')
      install_data('extras/logos/naev.icns', install_dir: ndata_path)

   elif host_machine.system() == 'windows'
      windows = import('windows')
      icon = files('extras/logos/logo.ico')
      install_data(icon, install_dir: '.')
      res_include = include_directories('extras/logos')
      win_manifest = configure_file(input: 'extras/windows/naev.exe.manifest.in', output: 'naev.exe.manifest', configuration: app_metadata)
      win_rc = configure_file(input: 'extras/windows/resource.rc.in', output: 'resource.rc', configuration: app_metadata)
      # TODO Not linked into the binary yet. Cargo owns the executable now, so
      # this needs rustc-link-arg-bins or the embed-resource crate.
      windows.compile_resources(win_rc, depend_files: [win_manifest, icon], include_directories: res_include)
   endif

# Target we are cross-compiling for
   target_triplet = get_option('target_triplet')

   copydir = find_program(files('utils'/'build'/'copydir.py'))

   # Compiling Naev
   # 1. Cargo compiles the C engine and all the Rust into a static library.
   # 2. meson links the result into the executable.

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
   cargo_env.set( 'NAEV_VERSION', meson.project_version() )
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

subdir('po')

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
