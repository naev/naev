subdir('glcheck')

test('main_menu',
    find_program('watch-for-msg.py'),
    args: [
        naev_sh,
        'Reached main menu'
    ],
    env: ['WITHGDB=NO'],
    workdir: meson.source_root(),
    protocol: 'exitcode'
    )

if (ascli_exe.found())
    metainfo_test_file = 'org.naev.Naev.metainfo.xml'
    test('validate_metainfo',
    ascli_exe,
    args: [
        'validate',
        '--no-net',
        '--pedantic',
        '--verbose',
        metainfo_test_file
    ],
    workdir: meson.source_root()
    )
endif

if (luacheck.found())

# If Meson is to be believed Luacheck will fail if any warnings are present due to a non-zero return value. See luacheck return values at:
# https://luacheck.readthedocs.io/en/stable/cli.html#command-line-options

    # This wrapper will return a non-zero value for luacheck exit codes 2-4 as these would be considered critical failures.
    luacheck_wrapper = find_program(meson.source_root() + '/utils/luacheck.sh')

    test('luacheck',
        luacheck_wrapper,
        args: ['dat', 'docs', 'extras', 'utils', '--no-color'],
        workdir: meson.source_root(),
        )
endif

# If this test fails, someone put a standard constant like PRIu64 in a translatable string.
# We can't support that (and you don't want to know how GNU gettext does it), but %.0f is typically good enough.
test('search_for_unsupported_sysdep_strings',
   find_program('grep'),
   args: ['-q', '%<PRI', join_paths(meson.source_root(), 'po', 'naev.pot')],
   should_fail: true,
   )
