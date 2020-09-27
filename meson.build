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

subdir('po')

####
# Source lists
####
source = files(
   'src/array.c',
   'src/background.c',
   'src/board.c',
   'src/camera.c',
   'src/claim.c',
   'src/collision.c',
   'src/colour.c',
   'src/comm.c',
   'src/commodity.c',
   'src/cond.c',
   'src/conf.c',
   'src/console.c',
   'src/damagetype.c',
   'src/debris.c',
   'src/dev.c',
   'src/dev_mapedit.c',
   'src/dev_outfit.c',
   'src/dev_planet.c',
   'src/dev_ship.c',
   'src/dev_sysedit.c',
   'src/dev_system.c',
   'src/dev_uniedit.c',
   'src/dialogue.c',
   'src/economy.c',
   'src/equipment.c',
   'src/escort.c',
   'src/event.c',
   'src/explosion.c',
   'src/faction.c',
   'src/fleet.c',
   'src/font.c',
   'src/glad.c',
   'src/gui.c',
   'src/gui_omsg.c',
   'src/gui_osd.c',
   'src/hook.c',
   'src/info.c',
   'src/input.c',
   'src/intro.c',
   'src/joystick.c',
   'src/land.c',
   'src/land_outfits.c',
   'src/land_shipyard.c',
   'src/land_trade.c',
   'src/load.c',
   'src/log.c',
   'src/map.c',
   'src/map_find.c',
   'src/map_overlay.c',
   'src/map_system.c',
   'src/md5.c',
   'src/menu.c',
   'src/mission.c',
   'src/music.c',
   'src/music_openal.c',
   'src/music_sdlmix.c',
   'src/naev.c',
   'src/ndata.c',
   'src/nebula.c',
   'src/news.c',
   'src/nfile.c',
   'src/nlua.c',
   'src/nmath.c',
   'src/nondata.c',
   'src/nopenal.c',
   'src/npc.c',
   'src/npng.c',
   'src/nstring.c',
   'src/ntime.c',
   'src/nxml.c',
   'src/nxml_lua.c',
   'src/nzip.c',
   'src/opengl.c',
   'src/opengl_matrix.c',
   'src/opengl_render.c',
   'src/opengl_shader.c',
   'src/opengl_tex.c',
   'src/opengl_vbo.c',
   'src/options.c',
   'src/outfit.c',
   'src/pause.c',
   'src/perlin.c',
   'src/physics.c',
   'src/pilot.c',
   'src/pilot_cargo.c',
   'src/pilot_ew.c',
   'src/pilot_heat.c',
   'src/pilot_hook.c',
   'src/pilot_outfit.c',
   'src/pilot_weapon.c',
   'src/plasmaf.c',
   'src/player.c',
   'src/player_autonav.c',
   'src/player_gui.c',
   'src/queue.c',
   'src/rng.c',
   'src/save.c',
   'src/ship.c',
   'src/shiplog.c',
   'src/shipstats.c',
   'src/slots.c',
   'src/sound.c',
   'src/sound_openal.c',
   'src/sound_sdlmix.c',
   'src/space.c',
   'src/spfx.c',
   'src/start.c',
   'src/tech.c',
   'src/threadpool.c',
   'src/toolkit.c',
   'src/unidiff.c',
   'src/utf8.c',
   'src/weapon.c',
   'src/tk/widget/input.c',
   'src/tk/widget/image.c',
   'src/tk/widget/cust.c',
   'src/tk/widget/rect.c',
   'src/tk/widget/tabwin.c',
   'src/tk/widget/fader.c',
   'src/tk/widget/checkbox.c',
   'src/tk/widget/imagearray.c',
   'src/tk/widget/button.c',
   'src/tk/widget/text.c',
   'src/tk/widget/list.c',
)

lua_source = files(
   'src/ai.c',
   'src/nlua_bkg.c',
   'src/nlua_camera.c',
   'src/nlua_cli.c',
   'src/nlua_col.c',
   'src/nlua_commodity.c',
   'src/nlua_diff.c',
   'src/nlua_evt.c',
   'src/nlua_faction.c',
   'src/nlua_gfx.c',
   'src/nlua_gui.c',
   'src/nlua_hook.c',
   'src/nlua_jump.c',
   'src/nlua_misn.c',
   'src/nlua_music.c',
   'src/nlua_naev.c',
   'src/nlua_news.c',
   'src/nlua_outfit.c',
   'src/nlua_pilot.c',
   'src/nlua_planet.c',
   'src/nlua_player.c',
   'src/nlua_rnd.c',
   'src/nlua_ship.c',
   'src/nlua_shiplog.c',
   'src/nlua_spfx.c',
   'src/nlua_system.c',
   'src/nlua_tex.c',
   'src/nlua_time.c',
   'src/nlua_tk.c',
   'src/nlua_var.c',
   'src/nlua_vec2.c'
)

mac_source = files('src/glue_macos.m')

# Headers aren't needed for running the builds,
# as they can all be found in the defined include derectories.
# These are only used to let Meson know that Doxygen needs to be
# re-run when these files change.
headers = files(
   'src/ai.h',
   'src/array.h',
   'src/background.h',
   'src/board.h',
   'src/camera.h',
   'src/claim.h',
   'src/collision.h',
   'src/colour.h',
   'src/comm.h',
   'src/commodity.h',
   'src/cond.h',
   'src/conf.h',
   'src/console.h',
   'src/damagetype.h',
   'src/debris.h',
   'src/dev.h',
   'src/dev_mapedit.h',
   'src/dev_outfit.h',
   'src/dev_planet.h',
   'src/dev_ship.h',
   'src/dev_sysedit.h',
   'src/dev_system.h',
   'src/dev_uniedit.h',
   'src/dialogue.h',
   'src/economy.h',
   'src/equipment.h',
   'src/escort.h',
   'src/event.h',
   'src/explosion.h',
   'src/faction.h',
   'src/fleet.h',
   'src/font.h',
   'src/gettext.h',
   'src/glad.h',
   'src/glue_macos.h',
   'src/gui.h',
   'src/gui_omsg.h',
   'src/gui_osd.h',
   'src/hook.h',
   'src/info.h',
   'src/input.h',
   'src/intro.h',
   'src/joystick.h',
   'src/khrplatform.h',
   'src/land.h',
   'src/land_outfits.h',
   'src/land_shipyard.h',
   'src/land_takeoff.h',
   'src/land_trade.h',
   'src/load.h',
   'src/log.h',
   'src/map.h',
   'src/mapData.h',
   'src/map_find.h',
   'src/map_overlay.h',
   'src/map_system.h',
   'src/md5.h',
   'src/menu.h',
   'src/mission.h',
   'src/music.h',
   'src/music_openal.h',
   'src/music_sdlmix.h',
   'src/naev.h',
   'src/ncompat.h',
   'src/ndata.h',
   'src/nebula.h',
   'src/news.h',
   'src/nfile.h',
   'src/nlua.h',
   'src/nlua_bkg.h',
   'src/nlua_camera.h',
   'src/nlua_cli.h',
   'src/nlua_col.h',
   'src/nlua_commodity.h',
   'src/nlua_diff.h',
   'src/nlua_evt.h',
   'src/nlua_faction.h',
   'src/nlua_gfx.h',
   'src/nlua_gui.h',
   'src/nlua_hook.h',
   'src/nlua_jump.h',
   'src/nlua_misn.h',
   'src/nlua_music.h',
   'src/nlua_naev.h',
   'src/nlua_news.h',
   'src/nlua_outfit.h',
   'src/nlua_pilot.h',
   'src/nlua_planet.h',
   'src/nlua_player.h',
   'src/nlua_rnd.h',
   'src/nlua_ship.h',
   'src/nlua_shiplog.h',
   'src/nlua_spfx.h',
   'src/nlua_system.h',
   'src/nlua_tex.h',
   'src/nlua_time.h',
   'src/nlua_tk.h',
   'src/nlua_var.h',
   'src/nlua_vec2.h',
   'src/nluadef.h',
   'src/nmath.h',
   'src/nopenal.h',
   'src/npc.h',
   'src/npng.h',
   'src/nstd.h',
   'src/nstring.h',
   'src/ntime.h',
   'src/nxml.h',
   'src/nxml_lua.h',
   'src/nzip.h',
   'src/opengl.h',
   'src/opengl_matrix.h',
   'src/opengl_render.h',
   'src/opengl_shader.h',
   'src/opengl_tex.h',
   'src/opengl_vbo.h',
   'src/options.h',
   'src/outfit.h',
   'src/pause.h',
   'src/perlin.h',
   'src/physics.h',
   'src/pilot.h',
   'src/pilot_cargo.h',
   'src/pilot_ew.h',
   'src/pilot_heat.h',
   'src/pilot_hook.h',
   'src/pilot_outfit.h',
   'src/pilot_weapon.h',
   'src/plasmaf.h',
   'src/player.h',
   'src/player_autonav.h',
   'src/player_gui.h',
   'src/queue.h',
   'src/rng.h',
   'src/save.h',
   'src/ship.h',
   'src/shiplog.h',
   'src/shipstats.h',
   'src/slots.h',
   'src/sound.h',
   'src/sound_openal.h',
   'src/sound_priv.h',
   'src/sound_sdlmix.h',
   'src/space.h',
   'src/spfx.h',
   'src/start.h',
   'src/tech.h',
   'src/threadpool.h',
   'src/tk/toolkit_priv.h',
   'src/tk/widget.h',
   'src/tk/widget/button.h',
   'src/tk/widget/checkbox.h',
   'src/tk/widget/cust.h',
   'src/tk/widget/fader.h',
   'src/tk/widget/image.h',
   'src/tk/widget/imagearray.h',
   'src/tk/widget/input.h',
   'src/tk/widget/list.h',
   'src/tk/widget/rect.h',
   'src/tk/widget/tabwin.h',
   'src/tk/widget/text.h',
   'src/toolkit.h',
   'src/unidiff.h',
   'src/utf8.h',
   'src/weapon.h'
)

####
# Naev
####
if get_option('configure_build') == true

   config_data = configuration_data({
      'VMAJOR'         : meson.project_version().split('.')[0],
      'VMINOR'         : meson.project_version().split('.')[1],
      'VREV'           : meson.project_version().split('.')[2],
      'HOST'           : '"' + host_machine.system() + '-' + host_machine.cpu_family() + '"',
      'DEBUG'          : get_option('debug')    ? 1 : false,
      'DEBUGGING'      : get_option('debug')    ? 1 : false,
      'DEBUG_PARANOID' : get_option('paranoid') ? 1 : false,
      'PACKAGE'        : meson.project_name(),
      'PACKAGE_VERSION': meson.project_version()
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

   executable(
      'naev',
      naev_source,
      include_directories: include_dirs,
      dependencies: [system_deps, deps],
      export_dynamic: bfd.found(),
      install: true)

   install_data(
      'AUTHORS',
      'LICENSE',
      'README',
      install_dir: get_option('datadir') / 'doc/naev'
   )

   install_data(
      'naev-confupdate.sh',
      'extras/logos/naev.png',
      install_dir: get_option('datadir') / 'naev'
   )

   install_subdir('dat', install_dir: get_option('datadir') / 'naev')

   if host_machine.system() not in ['windows', 'cygwin', 'emscripten', 'android']
      install_data('naev.appdata.xml', install_dir: get_option('datadir') / 'appdata/naev')
      install_data('naev.desktop', install_dir: get_option('datadir') / 'applications/naev')
      install_man('naev.6')
   endif

endif

subdir('docs')

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