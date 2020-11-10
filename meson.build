project('naev', 'c',
   version : '0.8.0-beta.3',
   default_options : [
      'warning_level=1',
      'optimization=g',
      'c_std=gnu11',
      'werror=false'
   ],
   meson_version: '>=0.55.0')

issue_address = 'https://github.com/naev/naev/issues'
copyright_holder = 'Naev Dev Team'

# Tools
cc = meson.get_compiler('c')
subdir('utils')

# Version Generation
version_result = run_command(gen_version, meson.project_version())
if version_result.returncode() != 0
   error(version_result.stderr())
else
   version = version_result.stdout().strip()
   meson.add_dist_script(add_to_package, 'dat/VERSION')
   summary('tag', version)
endif

# Initializeation
config_data = configuration_data()
naev_deps = []

# Subdirs
subdir('src')
subdir('docs')
subdir('po')

####
# Naev
####
buildExec = get_option('executable')
if buildExec.disabled() == false

   ndata_path = get_option('ndata_path')
   if ndata_path == ''
      ndata_path = get_option('datadir') / 'naev'
   endif
   summary('NData Path', ndata_path, section: 'Features')

   debug    = get_option('debug')
   paranoid = get_option('paranoid')
   config_data.set_quoted('PACKAGE', meson.project_name())
   config_data.set_quoted('PACKAGE_NAME', meson.project_name())
   config_data.set_quoted('PACKAGE_VERSION', meson.project_version())
   config_data.set_quoted('PKGDATADIR', get_option('prefix') / ndata_path)
   config_data.set_quoted('HOST', host_machine.system() + '-' + host_machine.cpu_family())
   # Cut out the numeric fields from our SemVer <major>.<minor>.<rev>[-pre_release][+build] for OS versioning.
   config_data.set('VMAJOR', meson.project_version().split('.')[0])
   config_data.set('VMINOR', meson.project_version().split('.')[1])
   config_data.set('VREV', meson.project_version().split('.')[2].split('-')[0].split('+')[0])
   config_data.set('DEBUG', debug ? 1 : false)
   config_data.set('DEBUGGING', debug ? 1 : false)
   config_data.set('DEBUG_PARANOID', paranoid ? 1 : false)
   summary('Enabled' , debug   , section: 'Debug', bool_yn: true)
   summary('Paranoid', paranoid, section: 'Debug', bool_yn: true)

   ### Hard deps (required: true)

   csparse = dependency('', required: false)
   if 'SuiteSparse' not in get_option('force_fallback_for')
      csparse_names = ['csparse', 'cxsparse']
      foreach n : csparse_names
         csparse = cc.find_library(n, required: false)
         if csparse.found()
            break
         endif
      endforeach
   endif

   if csparse.found()
      if cc.has_header('suitesparse/cs.h')
         config_data.set10('HAVE_SUITESPARSE_CS_H', true)
      endif
   else
      csparse = dependency(
         'SuiteSparse',
         fallback: ['SuiteSparse', 'CSparse_dep'],
         required: true)
   endif
   naev_deps += csparse

   naev_deps += cc.find_library('m', required : false)

   sdl = dependency('sdl2', required: true)
   naev_deps += [
      dependency('freetype2', required: true),
      sdl,
      declare_dependency(
         dependencies: dependency('libpng', required: true),
         compile_args: '-DNOLOGPRINTFCONSOLE',
         link_args: '-DNOLOGPRINTFCONSOLE'),
      dependency('libxml-2.0', required: true)
   ]

   # Lua
   useLuaJIT = get_option('luajit')
   lua = dependency('', required: false)
   if useLuaJIT.disabled() == false
      lua = dependency('luajit', fallback: ['luajit', 'luajit_dep'], required: useLuaJIT)
   endif
   summary('LuaJIT', lua.found(), section: 'Features', bool_yn: true)

   if not lua.found()
      lua = dependency('lua51', fallback: ['lua', 'lua_dep'], required: true)
   endif

   naev_deps += lua

   ### Soft deps (required: false

   # BFD
   if get_option('debug')
      bfd = cc.find_library('bfd', required: false)
   else
      bfd = dependency('', required: false)
   endif
   summary('BFD', bfd.found(), section: 'Debug', bool_yn: true)

   naev_deps += bfd
   config_data.set10('HAS_BFD', bfd.found())

   # Appstream (Used for generating desktop files and verifying metadata)
   ascli_exe = find_program('appstreamcli', version: '>=0.12.9', required: false)

   # OpenAL
   hasOpenAL = false
   useOpenAL = get_option('openal')
   openal    = dependency('openal', required: useOpenAL)
   if openal.found()
      vorbis = dependency('vorbis', required: useOpenAL)
      if not vorbis.found()
         message('Found OpenAL, but not using it because libvorbis is missing.')
      else
         vorbisfile = dependency('vorbisfile', required: useOpenAL)
         if not vorbisfile.found()
            message('Found OpenAL, but not using it because libvorbisfile is missing.')
         else
            hasOpenAL  = true
            naev_deps += [openal, vorbis, vorbisfile]
         endif
      endif
   endif
   config_data.set10('USE_OPENAL', hasOpenAL)
   summary('OpenAL', hasOpenAL, section: 'Features', bool_yn: true)

   # SDL Mixer
   useSDLMixer = get_option('sdl_mixer')
   sdl_mixer = dependency('SDL2_mixer', required: useSDLMixer)
   naev_deps += sdl_mixer
   config_data.set10('USE_SDLMIX', sdl_mixer.found())
   summary('SDL Mixer', sdl_mixer.found(), section: 'Features', bool_yn: true)

   ### Generated sources

   # VERSION
   config_data.set_quoted('VERSION', version)

   # config.h
   configure_file(
      output: 'config.h',
      configuration: config_data
   )
   add_project_arguments('-include', 'config.h', language: 'c')

   # GLAD
   if not cc.has_header('windows.h')
      naev_deps += cc.find_library('dl', required: true)
   endif

   include_dirs = include_directories(
      'src',
      'src/tk',
      'src/tk/widget'
   )

   if host_machine.system() == 'darwin'
      add_languages('objc')
      naev_source += mac_source
      naev_deps += dependency('Foundation', required: true )
   endif

   if host_machine.system() == 'windows'
      windows = import('windows')
      icon = files('extras/logos/logo.ico')
      win_rc = configure_file(input: 'extras/resource.rc.in', output: 'resource.rc', configuration: config_data)
      naev_source += windows.compile_resources(win_rc, depend_files: icon)
   endif

   shaders_c_gen = executable(
      'shaders_c_gen',
      'src/shaders_c_gen.c',
      install: false,
      native: true)

   shader_source = custom_target(
      'generate_shaders',
      command: shaders_c_gen,
      output: ['shaders.gen.c', 'shaders.gen.h']
   )

   naev_source += shader_source

   naev_bin = executable(
      'naev',
      naev_source,
      include_directories: include_dirs,
      dependencies: naev_deps,
      export_dynamic: bfd.found(),
      install: true)

   install_data(
      'LICENSE',
      'README',
      install_dir: get_option('datadir') / 'doc/naev'
   )

   install_data(
      'naev-confupdate.sh',
      'extras/logos/naev.png',
      install_dir: get_option('datadir') / 'naev'
   )

   install_subdir(
      'dat',
      install_dir: ndata_path,
   )

   if host_machine.system() not in ['windows', 'cygwin', 'emscripten', 'android', 'darwin']        
         metainfo_file = 'org.naev.naev.metainfo.xml'

         if (ascli_exe.found())
         # Create desktop-entry file from metainfo
         custom_target('gen-desktop-entry',
            input : [metainfo_file],
            output : ['org.naev.naev.desktop'],
            command : [ascli_exe, 'make-desktop-file', '@INPUT@', '@OUTPUT@'],
            install: true,
            install_dir: join_paths (get_option ('datadir'), 'applications')
         )
         else
            install_data('org.naev.naev.desktop', install_dir: join_paths (get_option ('datadir'), 'applications'))
         endif

         install_data(metainfo_file, install_dir: get_option('datadir') / 'metainfo')

      install_data(['extras/logos/logo16.png', 'extras/logos/logo32.png', 'extras/logos/logo64.png', 'extras/logos/logo128.png', 'extras/logos/naev.png'],
             rename : ['16x16/apps/naev.png', '32x32/apps/naev.png', '64x64/apps/naev.png', '128x128/apps/naev.png', '256x256/apps/naev.png'],
             install_dir: get_option('datadir') / 'icons/hicolor')
      install_man('naev.6')
      
   endif

   subdir('test')

endif

####
# Soundtrack
####
soundtrackpy = find_program('utils/soundtrack.py')
custom_target(
   'soundtrack',
   command: [
      soundtrackpy,
      '--csv', 'yes',
      '--source-dir', meson.source_root(),
      '--output', '@OUTPUT0@'
   ],
   output: [
      'naev-' + meson.project_version() + '-soundtrack.zip',
      'naev-' + meson.project_version() + '-soundtrack.csv'
   ]
)
