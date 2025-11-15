subdir('manual')

genCDoc = get_option('docs_c')
if genCDoc.disabled() == false
   # C documentation
   doxygen = find_program('doxygen', required : genCDoc)
   dot    = find_program('dot', required : false)
   if doxygen.found()
      doxy_output = 'c'
      doxy_in = files('c/Doxyfile.in')
      doxy_conf = configuration_data({
         'HAVE_DOT'           : dot.found() ? 'YES' : 'NO',
         'PROJECT_NAME'       : meson.project_name(),
         'PROJECT_NUMBER'     : meson.project_version(),
         'PROJECT_LOGO'       : meson.project_source_root() / 'extras/logos/logo32.png',
         'INPUT'              : meson.project_source_root() / 'src',
         'OUTPUT_DIRECTORY'   : meson.current_build_dir() / doxy_output
      })

      doxyfile = configure_file(
         input          : doxy_in,
         output         : 'Doxyfile',
         configuration  : doxy_conf,
         install        : false)

      cdoc = custom_target(
         'cdoc',
         input       : [source, nlua_source, mac_source, sdf_source, headers],
         output      : doxy_output,
         command     : [doxygen, doxyfile],
         install     : true,
         install_dir : 'doc/naev')
   endif
endif
summary('C', is_variable('cdoc'), section: 'Documentation', bool_yn: true)

genLDoc = get_option('docs_lua')
if genLDoc.disabled() == false
   # Lua documentation
   ldoc = find_program('ldoc', required: genLDoc)
   if ldoc.found()
      c2luadoc = generator(
         find_program(files('lua'/'c2luadoc.sh')),
         arguments: [doxtractor.full_path(), '@INPUT@', '@OUTPUT@'],
         output   : '@BASENAME@.luadoc',
         depends: [doxtractor],
      )

      # WARNING: half-assed way to ensure the same directory contains the Lua sources we want to document.
      # Using @PLAINNAME@ forces a flat structure, which is no loss yet. (I think the @module directive is what counts,
      # in which case we want to leave the nested structure and collision-proof ourselves.)
      direct = generator(
         find_program('cp', required: true),
         arguments: ['@INPUT@', '@OUTPUT@'],
         output   : '@PLAINNAME@'
      )

      clua_source = c2luadoc.process(nlua_source)
      llua_source = direct.process(lua_source)

      lconfig = files('lua/config.ld')

      ldoc = custom_target(
         'ldoc',
         input: [clua_source, llua_source],
         output: 'lua',
         command: [
         ldoc,
         '-c', lconfig,
         '-d', '@OUTPUT@',
         '-l', meson.project_source_root() / 'docs/lua',
         '-s', meson.project_source_root() / 'docs/lua',
         '@PRIVATE_DIR@'],
         install: true,
         install_dir: 'doc/naev')
   endif
endif
summary('Lua', is_variable('ldoc'), section: 'Documentation', bool_yn: true)

genLore = get_option('docs_lore')
if genLore.disabled() == false
   # Lore website build deps
   bundler = find_program('bundle', required: genLore)
   tidy = find_program('tidy', required: genLore)
   yq = find_program('yq', required: genLore)
   graphicsmagick = find_program('gm', required: genLore)
   #ruby_header = cc.find_library('ruby', required: genLore, has_headers: ['ruby.h'])
   #yaml_header = cc.find_library('yaml', required: genLore, has_headers: ['yaml.h'])

   loremasterpy = find_program(join_paths('lore','loremaster.py'))

   lore = custom_target(
      'lore',
      output: 'lore',  # Specify 'lore' as the output directory
      command: [
         loremasterpy,
         '--source-dir', meson.project_source_root(),
         '--build-dir', meson.build_root()
      ],
   )
endif
summary('Lore', is_variable('lore'), section: 'Documentation', bool_yn: true)
