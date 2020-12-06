subdir('glcheck')

test('Reaches main menu',
    naev_bin,
    args: [
        '--devunittest',
        meson.source_root() / 'dat'],
    workdir: meson.source_root(),
    protocol: 'exitcode')

if (ascli_exe.found())
    metainfo_test_file = 'org.naev.naev.metainfo.xml'
    test('validate metainfo file',
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
