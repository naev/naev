project('naev', 'c',
   version : '0.8.0',
   default_options : [
      'warning_level=1',
      'optimization=g',
      'c_std=gnu11',
      'werror=false'
   ],
   meson_version: '>=0.55.0')

config_data = configuration_data({
   'VMAJOR': meson.project_version().split('.')[0],
   'VMINOR': meson.project_version().split('.')[1],
   'VREV'  : meson.project_version().split('.')[2],
   'HOST'  : '"x86_64-linux-gnu"',
   'DEBUG'         : get_option('debug') ? 1 : false,
   'DEBUGGING'     : get_option('debug') ? 1 : false,
   'DEBUG_PARANOID': get_option('paranoid') ? 1 : false
})

configure_file(
   output: 'config.h',
   configuration: config_data
)

add_project_arguments('-include', 'config.h', language: 'c')

cc = meson.get_compiler('c')

if 'SuiteSparse' not in get_option('force_fallback_for')
   csparse_names = ['csparse', 'cxsparse', 'libcsparse', 'libcxsparse', 'libcsparse3', 'libcxsparse3']
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

system_deps = [
   cc.find_library('dl', required : true),
   cc.find_library('m', required : true),
   csparse
]

sdl = dependency('sdl2', required: true)
deps = [
   dependency('freetype2', required: true),
   sdl,
   declare_dependency(
      dependencies: dependency('libpng', required: true),
      compile_args: '-DNOLOGPRINTFCONSOLE',
      link_args: '-DNOLOGPRINTFCONSOLE'),
   dependency('libxml-2.0', required: true),
   dependency('lua51', fallback: ['lua', 'lua_dep'], static: true, required: true)
]

include_dirs = include_directories(
   'src',
   'src/tk',
   'src/tk/widget'
)

source = files(
   'src/ai.c',
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
   'src/nlua_vec2.c',
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

shaders_c_gen = executable(
   'shaders_c_gen',
   'src/shaders_c_gen.c',
   dependencies: sdl,
   install: false,
   native: true)

shader_source = custom_target(
   'generate_shaders',
   command: shaders_c_gen,
   output: ['shaders.gen.c', 'shaders.gen.h']
)

executable(
   'naev',
   source,
   shader_source,
   include_directories: include_dirs,
   dependencies: [system_deps, deps],
   default: true,
   install: true)
