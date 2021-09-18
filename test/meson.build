subdir('glcheck')

test('main_menu',
    find_program('watch-for-msg.py'),
    args: [
        naev_bin,
        meson.source_root() / 'dat',
        'Reached main menu'],
    workdir: meson.source_root(),
    protocol: 'exitcode')

if (ascli_exe.found())
    metainfo_test_file = 'org.naev.naev.metainfo.xml'
    test('validate_metainfo',
    ascli_exe,
    args: ['validate',
                '--no-net',
                '--pedantic',
                '--verbose',
                metainfo_test_file
    ],
    workdir: meson.source_root()
    )
endif

if (luacheck_exe.found())
    test('lua_lint',
    luacheck_exe,
    args: [join_paths(meson.source_root(), 'dat'),
                '--no-color'
    ],
    workdir: meson.source_root()
    )
endif
