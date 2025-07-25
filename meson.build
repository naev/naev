project('naev', ['c', 'rust'],
   version : '0.13.0-alpha.8',
   default_options : [
      'optimization=g',
      'c_std=c11',
      'werror=false',
      'warning_level=3',
      'rust_std=2024',
   ],
   # Requires meson 1.7.0 for rust_std=2024
   meson_version: '>=1.7.0')
# Dependency requirements
rustc_needed_version = '>=1.85.0'
bindgen_needed_version = '>=0.72.0'

copyright_year = 2025
forced_fallbacks = get_option('force_fallback_for')

issue_address = 'https://github.com/naev/naev/issues'
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
   error('Need at least rustc version @0@\nYou can get an up-to-date version by installing rustup and running "rustup defaul stable" and "rustup update"'.format(rustc_needed_version))
endif
bindgen = find_program('bindgen', version: bindgen_needed_version, required: false)
if not bindgen.found()
   error('Need at least bindgen version @0@\nYou can get an up-to-date version by running "cargo install bindgen-cli"'.format(bindgen_needed_version))
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
meson.add_dist_script(add_to_package, 'dat/VERSION')
summary('tag', version)

# Initialization
ndata_path = get_option('ndata_path')
if ndata_path == ''
   ndata_path = get_option('datadir') / 'naev'
endif
summary('NData Path', ndata_path, section: 'Features')

# Subdirs
subdir('src')
subdir('dat/outfits') # Has to be before naevpedia
subdir('dat/naevpedia')
subdir('dat/scripts')
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
   pcre2 = dependency('libpcre2-8', fallback : ['pcre2', 'libpcre2_8'], static: get_option('steamruntime'))
   sdl = dependency('sdl3', version: '>=3.2.0', required: true)
   libunibreak = dependency('libunibreak', required: true, version: '>=4.0', fallback: ['libunibreak'], static: get_option('steamruntime'))
   cmark = dependency('libcmark', required: true, version: '>=0.31.0', fallback: ['cmark', 'cmark_dep'], static: get_option('steamruntime'))

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
      pcre2,
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
   nluacheck = find_program(join_paths('utils','nluacheck.py'))

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

   # Generated headers
   configure_file(output: 'config.h', configuration: config_data)
   configure_file(output: 'naev_build_version.h', configuration: {'VERSION': '"'+version+'"'})
   add_project_arguments('-include', 'config.h', language: 'c')

   include_dirs = [include_directories(
      'src',
      'src/tk',
      'src/tk/widget'
   )]

   libsdf = static_library('sdf', sdf_source, include_directories: include_dirs, override_options: ['optimization=3'])
   naev_deps += declare_dependency(link_with: libsdf)

   if host_machine.system() == 'darwin'
      add_languages('objc', native: false)
      configure_file(input: 'extras/macos/Info.plist.in', output: 'Info.plist', configuration: app_metadata,
         install: true, install_dir: 'Contents')
      install_data('extras/logos/naev.icns', install_dir: ndata_path)
      naev_source += mac_source
      naevlua_source += mac_source
      naev_deps += dependency('Foundation', required: true )

   elif host_machine.system() == 'windows'
      windows = import('windows')
      icon = files('extras/logos/logo.ico')
      install_data(icon, install_dir: '.')
      res_include = include_directories('extras/logos')
      win_manifest = configure_file(input: 'extras/windows/naev.exe.manifest.in', output: 'naev.exe.manifest', configuration: app_metadata)
      win_rc = configure_file(input: 'extras/windows/resource.rc.in', output: 'resource.rc', configuration: app_metadata)
      naev_source += windows.compile_resources(win_rc, depend_files: [win_manifest, icon], include_directories: res_include)
   endif

   shaders_source = custom_target(
      'generate_shaders',
      command: [python, '@INPUT@'],
      input: 'src/shaders_c_gen.py',
      output: ['shaders.gen.c', 'shaders.gen.h']
   )
   naev_source += shaders_source
   naevlua_source += shaders_source
   colours_source = custom_target(
      'generate_colours',
      command: [python, '@INPUT@'],
      input: 'src/colours_c_gen.py',
      output: ['colours.gen.c', 'colours.gen.h']
   )
   naev_source += colours_source
   naevlua_source += colours_source

   # Generate Rust headers from C
   rust = import('rust')
   merge_h = find_program('utils/build/merge_h.py')
   naevch = custom_target(
      'merge headers',
      input: headers,
      command: [ merge_h, '@INPUT@' ],
      capture: true,
      output: 'naevc.h',
   )

bindgen_args = [
   '--wrap-unsafe-ops', # Needed to try to lower warnings with 2024 rust https://github.com/rust-lang/rust-bindgen/issues/3147
   '--override-abi', 'nlua_package_.*=C-unwind',
   '--override-abi', 'cli_print=C-unwind',
   '--override-abi', 'cli_printRaw=C-unwind',
   '--override-abi', 'cli_warn=C-unwind',
   '--override-abi', 'luaopen_utf8=C-unwind',
   '--override-abi', 'luaopen_cmark=C-unwind',
   '--override-abi', 'luaopen_enet=C-unwind',
   '--raw-line', '#![allow(clippy::all)]\n#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals, improper_ctypes, unnecessary_transmutes)]\npub mod config;',

]
# Handle issues with bindgen and the math.h header
bindgen_blocklist_items = [
   '__mingw_ldbl_type_t',
   'FP_INT_.*',
   'FP_SUBNORMAL*',
   'FP_NORMAL*',
   'FP_NAN*',
   'FP_INFINITE*',
   'FP_ZERO*'
]

foreach item : bindgen_blocklist_items
   bindgen_args += ['--blocklist-item', item]
endforeach

# Target we are cross-compiling for
target_triplet = get_option('target_triplet')

# meson doesn't automatically forward the C arguments to bindgen, so we have to do that ourselves...
# Related issue https://github.com/mesonbuild/meson/issues/13591
# We are setting this now in the github actions with BINDGEN_EXTRA_CLANG_ARGS.
#bindgen_cargs = []
#if target_triplet != ''
#  bindgen_cargs += ['--target='+target_triplet]
#endif

# Invoke bindgen to generate Rust bindings
naevcrs = rust.bindgen(
  input: naevch,
  output: 'naevc.rs',
  include_directories: [include_dirs, '.'],
  dependencies: naev_deps,
  language: 'c',
  args: bindgen_args,
  #c_args: bindgen_cargs,
)

   copydir = find_program('utils/build/copydir.py')
   naevc_crate = custom_target(
      'naevc crate',
      input: [ naevcrs, naevc_crate_cargo, configrs ],
      output: 'naevc',
      command: [ copydir, '@OUTPUT@', '@INPUT@' ],
   )

   # Compiling Naev
   # 1. First, compile all the C code as a static library with meson.
   # 2. Next, compile all the main Rust code as a static library with Cargo.
   # 3. Finally, use meson to link it all together in Rust.

   # First create the static library with C using meson
   naev_clib = static_library(
      'naev', # Has to be called 'naev' like the executable, or things fail to compile (why?)
      naev_source,
      include_directories: include_dirs,
      dependencies: naev_deps,
      install: false)

   # Next handle setting up and passing stuff to Cargo
   msrv = '1.77.2'
   # Since Rust debug is significantly slower than release, we try to use our
   # custom release-with-debug profile that runs quite fast while still having
   # pretty good debugging properties. Use 'debug_rust' option to explicitly
   # force Rust to be debug mode.
   if get_option('debug_rust')
      rust_target = 'debug'
   elif get_option('debug')
      rust_target = 'release-with-debug'
   else
      rust_target = 'release'
   endif
   cargo = find_program('cargo', version:'>= @0@'.format(msrv))
   cargo_data = configuration_data()
   cargo_data.set_quoted('NAME', meson.project_name())
   cargo_data.set_quoted('VERSION', meson.project_version())
   cargo_data.set_quoted('PATH', meson.project_source_root() / 'src' / 'naev.rs')
   cargo_data.set_quoted('NAEVC', naevc_crate.full_path() )
   cargo_data.set('BUILD', meson.project_build_root() )
   cargo_data.set('SOURCE', meson.project_source_root() )
   cargo_toml = configure_file( input: 'Cargo.toml.in', output: 'Cargo.toml', configuration: cargo_data )
   buildrs = configure_file( input: 'build.rs.in', output: 'build.rs', configuration: cargo_data )
   cargo_env = [ 'CARGO_HOME=' + meson.project_build_root() / 'cargo-home' ]
   cargo_options = [ '--manifest-path', cargo_toml ]
   cargo_options += [ '--profile', rust_target ]
   cargo_options += [ '--target-dir', meson.project_build_root() / 'src' ]
   if target_triplet != ''
      cargo_options += ['--target', target_triplet]
   endif

   naev_rlib = custom_target( 'cargo-build',
      depends: [naevcrs, naevc_crate, naev_clib],
      depend_files: source_rust + cargo_toml + buildrs,
      output: 'libnaev.rlib',
      console: true,
      command: [ 'env', cargo_env, cargo, 'build', cargo_options, '&&', copydir, meson.project_build_root(), 'src' / target_triplet / rust_target / '@OUTPUT@', ] )
   naev_rlib_lib = library(
      'naev_rlib',
      naev_rlib,
      install : true )
   # We create naev.rs with naev_rlib as a dependency to force rlib to be
   # compiled first, otherwise meson tries to compile both naev.rs and
   # naev_rlib in parallel and it fails.
   naev_rs = custom_target( 'naevrs',
      output: 'naev.rs',
      input: meson.project_source_root() / 'utils' / 'build' / 'naev.rs',
      depends: [naev_rlib],
      command: [ copydir, meson.project_build_root(), '@INPUT@', ] )

   # Lets make sure that we only enable export_dynamic if we aren't using macos or windows
   if host_machine.system() in ['windows', 'darwin']
      use_export_dynamic = false
   else
      use_export_dynamic = get_option('debug')
   endif

   # Add console subsystem linker flag on Windows to show console window
   naev_bin_link_args = []
   if get_option('debug') and host_machine.system() == 'windows'
      naev_bin_link_args += ['-Wl,--subsystem,console']
   endif

   # Finally, build the executable with Rust again, but using meson to link it all together
   naev_bin = executable(
      'naev',
      sources: naev_rs,
      # Have to make sure Rust uses the dependencies from Cargo
      rust_args: ['--extern', 'naev=libnaev.rlib', '-L', 'src' / target_triplet / rust_target / 'deps', '-L', 'src' / rust_target / 'deps'],
      # Also have to pull in the C dependencies
      dependencies: naev_deps,
      link_with: [naev_rlib_lib],
      export_dynamic: use_export_dynamic,
      link_args: naev_bin_link_args,
      install: true)

   # Naevlua is unused and unsupported with rust for now.
   #naevlua_bin = executable(
   #   'naevlua',
   #   naevlua_source,
   #   include_directories: include_dirs,
   #   dependencies: naev_deps,
   #   export_dynamic: get_option('debug'),
   #   override_options: ['-DNOMAIN=1'],
   #   install: false)
   gen_authors = find_program(join_paths('utils','build','gen_authors.py'))
   authors = custom_target(
      'authors',
      command: [
         gen_authors,
         '--output', '@OUTPUT0@',
         '--preamble', '@INPUT0@',
         '@INPUT1@', '@INPUT2@',
      ],
      input: files('dat/AUTHORS', 'artwork/gfx/ARTWORK_LICENSE.yaml', 'artwork/snd/SOUND_LICENSE.yaml'),
      output: 'AUTHORS',
      install: true,
      install_dir: ndata_path / 'dat',
   )

   gen_gettext_stats = find_program(join_paths('utils','build','gen_gettext_stats.py'))
   gettext_stats = custom_target(
      'gettext_stats',
      command: [gen_gettext_stats, '--output', '@OUTPUT@', '@INPUT@'],
      input: files('po/' + meson.project_name() + '.pot'),
      output: meson.project_name() + '.txt',
      install: true,
      install_dir: ndata_path / 'dat/gettext_stats',
   )

   # Need to add other generated data files here as necessary
   gen_zip_overlay = find_program(join_paths('utils','build','gen_zip_overlay.py'))
   gen_zip_command = [gen_zip_overlay, '@OUTPUT@', authors, '--cd', 'gettext_stats', gettext_stats]
   # Note that generated outfits populate build/dat/ which is included with
   # naev.py, so no need to put them in the overlay on top
   #gen_zip_command += ['--cd', 'outfits']
   #foreach target: gen_outfits
   #    gen_zip_command += target
   #endforeach
   zip_overlay = custom_target(
      'zip_overlay',
      command: gen_zip_command,
      output: 'meson_overlay.zip',
      depends: [authors, gettext_stats], # + gen_outfits,
      build_by_default: true
   )

   naev_py = configure_file(
      input: join_paths('utils','build','naev.py'),
      output: 'naev.py',
      configuration: {
         'build_root': meson.current_build_dir(),
         'source_root': meson.project_source_root(),
         'naev_bin' : naev_bin.full_path(),
         'zip_overlay' : zip_overlay.full_path(),
         'debug' : debug,
         'debug_paranoid' : paranoid,
      }
   )

   gdbinit = configure_file(
      input: join_paths('utils','build','gdbinit.in'),
      output: '.gdbinit',
      configuration: {
         'source_root': meson.project_source_root(),
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
      exclude_files: ['AUTHORS', 'outfits/meson.build', 'outfits/bioship/generate.py', 'outfits/bioship/meson.build', 'scripts/meson.build' ],
      exclude_directories: ['outfits/py', 'outfits/bioship/templates'],
   )

   install_subdir(
      'artwork',
      install_dir: ndata_path / 'dat',
      exclude_directories: '.git',  # That's a marker used by "git submodule".
      strip_directory: true,
   )

   if host_machine.system() not in ['windows', 'cygwin', 'emscripten', 'android', 'darwin']
      metainfo_file = 'org.naev.Naev.metainfo.xml'

      if (ascli_exe.found())
         # Create desktop-entry file from metainfo
         custom_target('gen-desktop-entry',
            input : [metainfo_file],
            output : ['org.naev.Naev.desktop'],
            command : [ascli_exe, 'make-desktop-file', '@INPUT@', '@OUTPUT@'],
            install: true,
            install_dir: join_paths (get_option ('datadir'), 'applications')
         )
      else
         install_data('org.naev.Naev.desktop', install_dir: join_paths (get_option ('datadir'), 'applications'))
      endif

      install_data(metainfo_file, install_dir: get_option('datadir') / 'metainfo')

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
   soundtrackpy = find_program(join_paths('utils','soundtrack.py'))
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

####
# luacheckrc
####
luacheckrc_extractor = find_program(join_paths('utils','luacheckrc_extractor.py'))
custom_target(
  'luacheckrc_gen',
  build_by_default: true,
  command: [
    luacheckrc_extractor,
    nlua_source + files('src/nlua_vec2.c'),
    '--output',
    join_paths(meson.current_source_dir(),'utils','@OUTPUT@'),
  ],
  output: [
    'luacheckrc_gen.lua',
  ]
)
