project('naev', 'c',
   version : '0.8.0',
   default_options : [
      'warning_level=1',
      'optimization=g',
      'c_std=gnu11',
      'werror=false'
   ],
   meson_version: '>=0.55.0')

issue_address = 'https://github.com/naev/naev/issues'
copyright_holder = 'Naev Dev Team'

subdir('src')
subdir('docs')
subdir('po')

####
# Naev
####
if get_option('configure_build') == true

   ndata_path = get_option('ndata_path')
   if ndata_path == ''
      ndata_path = get_option('datadir') / 'naev/ndata'
   endif

   config_data = configuration_data({
      'VMAJOR'         : meson.project_version().split('.')[0],
      'VMINOR'         : meson.project_version().split('.')[1],
      'VREV'           : meson.project_version().split('.')[2],
      'HOST'           : '"' + host_machine.system() + '-' + host_machine.cpu_family() + '"',
      'DEBUG'          : get_option('debug')    ? 1 : false,
      'DEBUGGING'      : get_option('debug')    ? 1 : false,
      'DEBUG_PARANOID' : get_option('paranoid') ? 1 : false,
      'PACKAGE'        : meson.project_name(),
      'PACKAGE_VERSION': meson.project_version(),
      'NDATA_DEF'      : '"' + (get_option('prefix') / ndata_path) + '"'
   })

   add_project_arguments('-include', 'config.h', language: 'c')

   cc = meson.get_compiler('c')

   ### Hard deps (required: true)

   if 'SuiteSparse' not in get_option('force_fallback_for')
      csparse_names = ['csparse', 'cxsparse']
      foreach n : csparse_names
         csparse = cc.find_library(n, required: false)
         if csparse.found()
            break
         endif
      endforeach
   endif

   if not is_variable('csparse') or not csparse.found()
      csparse = dependency(
         '',
         fallback: ['SuiteSparse', 'CSparse_dep'],
         required: true)
   endif

   libdl = cc.find_library('dl', required : host_machine.system() not in ['windows', 'cygwin'])

   system_deps = [
      cc.find_library('m', required : true),
      libdl,
      csparse
   ]

   deps = [
      dependency('freetype2', required: true),
      dependency('sdl2', required: true),
      declare_dependency(
         dependencies: dependency('libpng', required: true),
         compile_args: '-DNOLOGPRINTFCONSOLE',
         link_args: '-DNOLOGPRINTFCONSOLE'),
      dependency('libxml-2.0', required: true)
   ]

   # Lua
   lua = dependency('', required: false)
   if get_option('use_luajit')
      lua = dependency('luajit', fallback: ['luajit', 'luajit_dep'], required: false)
   endif

   if not lua.found()
      lua = dependency('lua51', fallback: ['lua', 'lua_dep'], required: true)
   endif

   deps += lua

   ### Soft deps (required: false

   # BFD
   if libdl.found() and get_option('debug')
      bfd = cc.find_library('bfd', required: false)
   else
      bfd = dependency('', required: false)
   endif

   system_deps += bfd
   config_data.set10('HAS_BFD', bfd.found())

   # OpenAL
   openal = dependency('openal', required: false)
   vorbis = dependency('vorbis', required: false)
   vorbisfile = dependency('vorbisfile', required: false)
   config_data.set10('USE_OPENAL', openal.found() and vorbis.found() and vorbisfile.found())
   if openal.found()
      if not vorbis.found()
         message('Found OpenAL, but not using it because libvorbis is missing.')
      elif not vorbisfile.found()
         message('Found OpenAL, but not using it because libvorbisfile is missing.')
      else
         deps += [openal, vorbis, vorbisfile]
      endif
   endif

   # SDL Mixer
   sdl_mixer = dependency('SDL2_mixer', required: false)
   deps += sdl_mixer
   config_data.set10('USE_SDLMIX', sdl_mixer.found())

   # FontConfig
   fontconfig = dependency('fontconfig', required: false)
   deps += fontconfig
   config_data.set10('USE_FONTCONFIG', fontconfig.found())

   configure_file(
      output: 'config.h',
      configuration: config_data
   )

   include_dirs = include_directories(
      'src',
      'src/tk',
      'src/tk/widget'
   )

   naev_source = [
      source,
      lua_source
   ]

   if host_machine.system() == 'darwin'
      add_languages('objc')
      naev_source += mac_source
      deps += dependency('Foundation', required: true )
   endif

   shaders_c_gen = executable(
      'shaders_c_gen',
      'src/shaders_c_gen.c',
      dependencies: dependency('sdl2', required: true),
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
      dependencies: [system_deps, deps],
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

   custom_target(
      'version',
      output: 'VERSION',
      command: ['echo', meson.project_version()],
      capture: true,
      install: true,
      install_dir: ndata_path
   )

   if host_machine.system() not in ['windows', 'cygwin', 'emscripten', 'android']
      install_data('org.naev.Naev.appdata.xml', install_dir: get_option('datadir') / 'appdata/naev')
      install_data('org.naev.Naev.desktop', install_dir: get_option('datadir') / 'applications/naev')
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