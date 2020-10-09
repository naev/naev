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
      ndata_path = get_option('datadir') / 'naev/ndata'
   endif
   summary('NData Path', ndata_path, section: 'Features')

   debug    = get_option('debug')
   paranoid = get_option('paranoid')
   config_data.set_quoted('PACKAGE', meson.project_name())
   config_data.set_quoted('PACKAGE_NAME', meson.project_name())
   config_data.set_quoted('PACKAGE_VERSION', meson.project_version())
   config_data.set_quoted('NDATA_DEF', get_option('prefix') / ndata_path)
   config_data.set_quoted('HOST', host_machine.system() + '-' + host_machine.cpu_family())
   config_data.set('DEBUG', debug ? 1 : false)
   config_data.set('DEBUGGING', debug ? 1 : false)
   config_data.set('DEBUG_PARANOID', paranoid ? 1 : false)
   summary('Enabled' , debug   , section: 'Debug', bool_yn: true)
   summary('Paranoid', paranoid, section: 'Debug', bool_yn: true)

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
   temp    = version.split('+')
   vmeta   = temp.length() > 1 ? temp[1] : ''
   temp    = temp[0].split('-')
   vcore   = temp[0]
   temp    = temp.length() > 1 ? temp[1].split('.') : ''
   vpre    = temp.length() > 0 ? temp[0] : ''
   vpren   = temp.length() > 1 ? temp[1] : ''
   temp    = vcore.split('.')
   vmajor  = temp[0]
   vminor  = temp[1]
   vrev    = temp[2]
   vhuman  = vcore + (vpre != '' ? ' ' + vpre : '') + (vpren != '' ? ' ' + vpren : '') + (debug ? ' debug' : '')

   config_data.set_quoted('VERSION', version)
   config_data.set_quoted('VMETA'  , vmeta  )
   config_data.set_quoted('VCORE'  , vcore  )
   config_data.set_quoted('VPRE'   , vpre   )
   config_data.set       ('VPREN'  , vpren  )
   config_data.set       ('VMAJOR' , vmajor )
   config_data.set       ('VMINOR' , vminor )
   config_data.set       ('VREV'   , vrev   )
   config_data.set_quoted('VHUMAN' , vhuman )

   # config.h
   configure_file(
      output: 'config.h',
      configuration: config_data
   )
   add_project_arguments('-include', 'config.h', language: 'c')

   # GLAD
   glad = find_program('glad')
   glad_c = custom_target(
      'glad',
      command: [
         glad,
         '--out-path', meson.current_build_dir() / '@OUTDIR@',
         '--local-files',
         '--profile=core',
         '--api=gl=3.1',
         '--generator=c',
         '--spec=gl',
         '--extensions=GL_ARB_texture_filter_anisotropic'],
      output: ['glad.c','glad.h', 'khrplatform.h']
   )
   naev_source += glad_c

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