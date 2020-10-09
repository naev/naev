genCDoc = get_option('docs_c')
if genCDoc.disabled() == false
    # C documentation
    doxygen = find_program('doxygen', required : genCDoc)
    dot     = find_program('dot', required : false)
    if doxygen.found()
        doxy_output = 'c'
        doxy_in = files('c/Doxyfile.in')
        doxy_conf = configuration_data({
            'HAVE_DOT'           : dot.found() ? 'YES' : 'NO',
            'PROJECT_NAME'       : meson.project_name(),
            'PROJECT_NUMBER'     : meson.project_version(),
            'PROJECT_LOGO'       : meson.source_root() / 'extras/logos/logo32.png',
            'INPUT'              : meson.source_root() / 'src',
            'OUTPUT_DIRECTORY'   : meson.current_build_dir() / doxy_output
        })

        doxyfile = configure_file(
            input        : doxy_in,
            output       : 'Doxyfile',
            configuration: doxy_conf,
            install      : false)

        cdoc = custom_target(
            'cdoc',
            input      : [source, lua_source, mac_source, headers],
            output     : doxy_output,
            command    : [doxygen, doxyfile],
            install    : true,
            install_dir: 'doc/naev')
    endif
endif
summary('C', is_variable('cdoc'), section: 'Documentation', bool_yn: true)

genLDoc = get_option('docs_lua')
if genLDoc.disabled() == false
    # Lua documentation
    ldoc = find_program('ldoc', required: genLDoc)
    if ldoc.found()
        c2luadoc = generator(
            find_program('lua/c2luadoc.sh', required: true),
            arguments: ['@INPUT@', '@OUTPUT@'],
            output   : '@BASENAME@.luadoc'
        )

        ldoc_source = c2luadoc.process(lua_source)

        lconfig = files('lua/config.ld')

        ldoc = custom_target(
            'ldoc',
            input: ldoc_source,
            output : 'lua',
            command: [
            ldoc,
            '-c', lconfig,
            '-d', '@OUTPUT@',
            '-l', meson.source_root() / 'docs/lua',
            '-s', meson.source_root() / 'docs/lua',
            '@PRIVATE_DIR@'],
            install: true,
            install_dir: 'doc/naev')
    endif
endif
summary('Lua', is_variable('ldoc'), section: 'Documentation', bool_yn: true)