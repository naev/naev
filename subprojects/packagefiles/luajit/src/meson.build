ljver = meson.project_version().split('.')
ljver = '.'.join(ljver[0], ljver[1])
abiver = '5.1'
verdir = 'luajit-' + ljver
jitdir = get_option('jitdir')
if jitdir == ''
   jitdir = get_option('datadir') / verdir / 'jit'
endif

foreach _var, _subdir : {
   'install_cmod': 'libdir',
   'install_lmod': 'datadir',
}
   _val = get_option(_var)
   if _val    = ''
      assert(not fs.is_absolute(_val))
   else
      _val = get_option(_subdir) / 'lua' / abiver
   endif
   set_variable(_var, _val)
endforeach

luajit_h = custom_target(
   'luajit.h',
   capture: true,
   command: [
      'sed',
      '-e', 's/@0@/@1@/'.format(
         'ROLLING',
         meson.project_version().split('.')[-1],
      ),
      '-e', '/^#error /d',
      '@INPUT@',
   ],
   input: 'luajit_rolling.h',
   output: 'luajit.h',
   install: true,
   install_tag: 'devel',
   install_dir: get_option('includedir') / verdir,
)

subdir('host')

lj_vm = custom_target(
   'lj_vm',
   output: host_machine.system() == 'windows' ? 'lj_vm.o' : 'lj_vm.S',
   command: [
      buildvm_exe,
      '-m', LJVM_MODE,
      '-o', '@OUTPUT@',
   ],
)

lib_sources = files(
   'lib_base.c',
   'lib_bit.c',
   'lib_buffer.c',
   'lib_debug.c',
   'lib_ffi.c',
   'lib_io.c',
   'lib_jit.c',
   'lib_math.c',
   'lib_os.c',
   'lib_package.c',
   'lib_string.c',
   'lib_table.c',
)

buildvm_headers = []
foreach _target : [
   {'mode': 'bcdef', 'output': 'lj_bcdef.h', 'input': lib_sources},
   {'mode': 'ffdef', 'output': 'lj_ffdef.h', 'input': lib_sources},
   {'mode': 'folddef', 'output': 'lj_folddef.h', 'input': 'lj_opt_fold.c'},
   {'mode': 'libdef', 'output': 'lj_libdef.h', 'input': lib_sources},
   {'mode': 'recdef', 'output': 'lj_recdef.h', 'input': lib_sources},
]
   buildvm_headers += custom_target(
      _target['output'],
      input: _target['input'],
      output: _target['output'],
      command: [buildvm_exe, '-m', _target['mode'], '-o', '@OUTPUT@', '@INPUT@'],
   )
endforeach

custom_target(
   'vmdef.lua',
   input: lib_sources,
   output: 'vmdef.lua',
   command: [buildvm_exe, '-m', 'vmdef', '-o', '@OUTPUT@', '@INPUT@'],
   install: true,
   install_dir: jitdir,
   install_tag: 'runtime',
)

luajit_lib_flags = [
   TARGET_XCFLAGS,
   '-D_LARGEFILE_SOURCE',
   '-U_FORTIFY_SOURCE',
]

luajit_lib_src = [buildvm_headers, lj_vm, luajit_h]
if get_option('amalgam')
   luajit_lib_src += 'ljamalg.c'
else
   luajit_lib_src += lib_sources
   luajit_lib_src += [
      'lib_aux.c',
      'lib_init.c',
      'lj_alloc.c',
      'lj_api.c',
      'lj_asm.c',
      'lj_assert.c',
      'lj_bc.c',
      'lj_bcread.c',
      'lj_bcwrite.c',
      'lj_buf.c',
      'lj_carith.c',
      'lj_ccall.c',
      'lj_ccallback.c',
      'lj_cconv.c',
      'lj_cdata.c',
      'lj_char.c',
      'lj_clib.c',
      'lj_cparse.c',
      'lj_crecord.c',
      'lj_ctype.c',
      'lj_debug.c',
      'lj_dispatch.c',
      'lj_err.c',
      'lj_ffrecord.c',
      'lj_func.c',
      'lj_gc.c',
      'lj_gdbjit.c',
      'lj_ir.c',
      'lj_lex.c',
      'lj_lib.c',
      'lj_load.c',
      'lj_mcode.c',
      'lj_meta.c',
      'lj_obj.c',
      'lj_opt_dce.c',
      'lj_opt_fold.c',
      'lj_opt_loop.c',
      'lj_opt_mem.c',
      'lj_opt_narrow.c',
      'lj_opt_sink.c',
      'lj_opt_split.c',
      'lj_parse.c',
      'lj_prng.c',
      'lj_profile.c',
      'lj_record.c',
      'lj_serialize.c',
      'lj_snap.c',
      'lj_state.c',
      'lj_str.c',
      'lj_strfmt.c',
      'lj_strfmt_num.c',
      'lj_strscan.c',
      'lj_tab.c',
      'lj_trace.c',
      'lj_udata.c',
      'lj_vmevent.c',
      'lj_vmmath.c',
   ]
endif

luajit_dep_kwargs = {
   'include_directories': include_directories('.'),
   #'sources'            : luajit_h,
   'variables'          : {
      'INSTALL_CMOD'     : install_cmod,
      'INSTALL_LMOD'     : install_lmod,
   },
}

luajit_lib_name = 'luajit-' + abiver
luajit_lib_kwargs = {
   'dependencies'    : [m_dep, dl_dep],
   'install'         : true,
   'override_options': ['b_sanitize=none'],
}

# Shared luajit library. {{{

if get_option('default_library')    = 'static'

   luajit_shared_lib = shared_library(
      luajit_lib_name,
      luajit_lib_src,
      c_args: luajit_lib_flags + (
         host_machine.system() == 'windows' ? '-DLUA_BUILD_AS_DLL' : []
      ),
      kwargs: luajit_lib_kwargs,
      soversion: meson.project_version(),
      # A version number like `2.1.1713773202` breaks clang when compiling for x86_64 on macOS:
      # ```
      # cc  -o […]/libluajit-5.1.2.1.1713773202.dylib […] -compatibility_version 2.1.1713773202 -current_version 2.1.1713773202 -lm -ldl
      # ld: malformed 32-bit x.y.z version number: 2.1.1713773202
      # clang: error: linker command failed with exit code 1 (use -v to see invocation)
      # ````
      # So use the same versions as in the official makefile.
      darwin_versions: [ljver, ljver + '.255'],
   )

   meson.override_dependency(
      'luajit',
      declare_dependency(
         kwargs: luajit_dep_kwargs,
         link_with: luajit_shared_lib,
      ),
      static: false,
   )

endif

# }}}

# Static luajit library. {{{

if get_option('default_library')    = 'shared'

   luajit_static_lib = static_library(
      luajit_lib_name,
      luajit_lib_src,
      c_args: luajit_lib_flags,
      kwargs: luajit_lib_kwargs,
   )

   meson.override_dependency(
      'luajit',
      declare_dependency(
         kwargs: luajit_dep_kwargs,
         link_with: luajit_static_lib,
      ),
      static: true,
   )

endif

if get_option('luajit')

   if get_option('default_library') == 'both'
      # Mixed mode creates a static + dynamic library and a statically linked luajit.
      if host_machine.system() == 'windows'
         error('mixing a static LuaJIT executable with a dynamic LuaJIT library is not supported on Windows')
      endif
      luajit_exe_export_dynamic = true
      luajit_exe_lib = luajit_static_lib
   elif get_option('default_library') == 'static'
      # Static mode creates a static library and a statically linked luajit.
      if host_machine.system() == 'windows'
         warning('static mode doesn\'t work well on Windows: C modules cannot be loaded, because they bind to lua51.dll')
      endif
      luajit_exe_export_dynamic = true
      luajit_exe_lib = luajit_static_lib
   else
      # Dynamic mode creates a dynamic library and a dynamically linked luajit.
      luajit_exe_export_dynamic = false
      luajit_exe_lib = luajit_shared_lib
   endif

   luajit_exe = executable(
      'luajit-' + meson.project_version(),
      [luajit_h, 'luajit.c'],
      export_dynamic: luajit_exe_export_dynamic,
      install: get_option('luajit'),
      link_with: luajit_exe_lib,
   )
   meson.override_find_program('luajit', luajit_exe)

   luajit_exe_basename = fs.name(luajit_exe.full_path())
   luajit_exe_extension = luajit_exe_basename.split(luajit_exe.name())[1]

   install_symlink(
      'luajit' + luajit_exe_extension,
      pointing_to: luajit_exe_basename,
      install_dir: get_option('bindir'),
      install_tag: 'runtime',
   )

endif

licensedir = (
   meson.version().version_compare('>= 1.1.0') ? get_option('licensedir') : ''
)
if licensedir == ''
   licensedir = get_option('datadir') / 'licenses/luajit'
endif
install_data(
   '../COPYRIGHT',
   install_dir: licensedir,
   install_tag: 'doc',
)

install_headers(
   'lauxlib.h',
   'lua.h',
   'lua.hpp',
   'luaconf.h',
   'lualib.h',
   subdir: verdir,
)

install_man('../etc/luajit.1')

install_subdir(
   'jit',
   exclude_files: '.gitignore',
   install_dir: jitdir,
   install_tag: 'runtime',
   strip_directory: true,
)

pkg = import('pkgconfig')
pkg.generate(
   description: 'Just-in-time compiler for Lua',
   filebase: 'luajit',
   libraries: ['-L${libdir}', '-l${libname}'],
   name: 'LuaJIT',
   subdirs: verdir,
   url: 'https://luajit.org',
   variables: [
      'abiver=' + abiver,
      'libname=' + luajit_lib_name,
      'INSTALL_CMOD=' + '${prefix}' / install_cmod,
      'INSTALL_LMOD=' + '${prefix}' / install_lmod,
   ],
)

# vim: foldmethod=marker foldlevel=0
